/**
 *  AutoComp Server
 *  server.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/28/2018
 */

#include "network/server/server.hpp"

namespace autocomp
{
  namespace net
  {

  const LEVELS Server::ERROR{g3::kWarningValue + 1, {"ERROR"}};

  Server::Server(const unsigned short & port, const unsigned int & nThreads,
                 const std::string & shutdownPipeName)
    : serverSocket(port),
      requestThreadPool(nThreads),
      transmissionThreadPool(nThreads),
      doneServing(true),
      shutdownPipeName(shutdownPipeName),
      decisionTree(constants::DECISION_TREE_FILENAME)
  {}

  Server::Server(const unsigned short & port,
                 const std::string & shutdownPipeName,
                 const unsigned int & nThreads)
    : serverSocket(port),
      requestThreadPool(nThreads),
      transmissionThreadPool(nThreads),
      doneServing(true),
      shutdownPipeName(shutdownPipeName),
      decisionTree(constants::DECISION_TREE_FILENAME)
  {}

  Server::~Server()
  {
    this->shutdown();
  }

  unsigned short Server::getPort() const
  {
    return this->serverSocket.getPort();
  }

  void Server::init()
  {
    if (not this->doneServing) {
      return;
    }

    // ---> Logging <--- //
    this->initLogger();
    this->performanceDataWriter =
      std::make_shared<autocomp::io::PerformanceDataWriter>();

    LOG(INFO) << "Initializing AutoComp server at port "
               << this->serverSocket.getPort();

    // ---> Request processing thread pool initialization <--- //
    LOG(INFO) << "Initializing request thread pool";
    this->requestThreadPool.init();

    // ---> Shutdown named pipe initialization <--- //
    LOG(INFO) << "Initializing shutdown named pipe";
    this->shutdownPipeFileDescriptor =
      ::open(this->shutdownPipeName.c_str(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);

    if (this->shutdownPipeFileDescriptor == -1) {
      std::string errorMessage("Error opening named pipe ");
      errorMessage.append(this->shutdownPipeName)
                  .append(". ")
                  .append(std::strerror(errno))
                  .append(" (errno code: ")
                  .append(std::to_string(errno))
                  .append(")");

      LOG(ERROR) << errorMessage;

      throw exceptions::NetworkError(errorMessage);
    }

    // ---> Poll structures initialization <--- //
    // First: server socket
    this->pollFileDescriptors[0].fd = this->serverSocket.getFileDescriptor();
    this->pollFileDescriptors[0].events = POLLIN;
    // Second: shutdown pipe
    this->pollFileDescriptors[1].fd = this->shutdownPipeFileDescriptor;
    this->pollFileDescriptors[1].events = POLLIN;

    // ---> Server socket initialization <--- //
    LOG(INFO) << "Initialized server socket";
    try {
      this->serverSocket.bind();
      this->serverSocket.listen();
      this->serverSocket.setSendBufferCapacity(12000000);
    }
    catch (exceptions::NetworkError & error) {
      LOG(ERROR) << error.what();
      throw error;
    }

    // Serve
    this->doneServing = false;

    // ---> CPU monitor thread initialization <--- //
    LOG(INFO) << "Initializing CPU monitor thread";
    this->cpuMonitorThread = std::thread(monitorCPU,
                                         std::ref(this->resourceState),
                                         std::ref(this->doneServing));

    LOG(INFO) << "Server initialized";
  }

  void Server::shutdown()
  {
    if (this->doneServing) {
      return;
    }

    LOG(INFO) << "Shutting server down";

    LOG(INFO) << "Closing shutdown named pipe";
    ::close(this->shutdownPipeFileDescriptor);

    LOG(INFO) << "Closing server socket";
    this->serverSocket.close();

    LOG(INFO) << "Shutting request thread pool down";
    this->requestThreadPool.shutdown();

    LOG(INFO) << "Shutting CPU monitor thread down";
    this->doneServing = true;
    this->cpuMonitorThread.join();

    this->performanceDataWriter.reset();
  }

  void Server::initLogger()
  {
    // Adding ERROR logging level
    g3::only_change_at_initialization::addLogLevel(ERROR);

    this->logWorker = g3::LogWorker::createLogWorker();
    auto logHandle= this->logWorker
                        ->addDefaultLogger(constants::LOG_FILE_PREFIX,
                                           constants::LOG_DIR,
                                           constants::LOG_FILE_ID);
    g3::initializeLogging(this->logWorker.get());
    auto logFormatChanging = logHandle->call(&g3::FileSink::overrideLogDetails, 
                                             Server::logFormatter);
    auto headerChanging = logHandle->call(&g3::FileSink::overrideLogHeader, "");

    logFormatChanging.wait();
    headerChanging.wait();
  }

  std::string Server::logFormatter(const g3::LogMessage & logMessage)
  {
    return std::string("[")
               .append(logMessage.timestamp())
               .append("]\t[")
               .append(logMessage.level())
               .append("]\t[")
               .append(logMessage.file())
               .append("->")
               .append(logMessage.function())
               .append(":")
               .append(logMessage.line())
               .append("] ");
  }

  void Server::serve()
  {
    LOG(INFO) << "Starting server at port " << this->serverSocket.getPort();

    while (not this->doneServing) {
      poll(this->pollFileDescriptors, this->nInputFileDescriptors,
           this->infiniteTime);

      for (const pollfd & fileDescriptor : this->pollFileDescriptors) {
        if (fileDescriptor.revents & POLLIN) {
          // User request
          if (fileDescriptor.fd == this->serverSocket.getFileDescriptor()) {

            std::shared_ptr<TCPSocket> clientSocket = this->serverSocket
                                                            .accept();

            this->requestThreadPool.run(Server::processRequest, clientSocket,
                                        std::ref(this->requestThreadPool),
                                        performanceDataWriter,
                                        std::ref(this->resourceState),
                                        std::ref(this->decisionTree));

            LOG(INFO) << "Received incoming connection from "
                      << clientSocket->getHostname() << ":"
                      << clientSocket->getPort()
                      << ". Enqueued to thread pool";
          }
          // Shutdown pipe, so shut down!
          else {
            char buffer[10];
            ::read(fileDescriptor.fd, buffer, 10);
            this->shutdown();
            continue;
          }
        }
      }
    }

    LOG(INFO) << "Server stopped";
  }

  // This is the core, the actual server!
  void Server::processRequest(std::shared_ptr<TCPSocket> clientSocket,
                              ThreadPool & transmissionThreadPool,
                              std::shared_ptr<io::PerformanceDataWriter>
                                performanceDataWriter,
                              ResourceState & resourceState,
                              const DecisionTree & decisionTree)
  {
    SynchronousQueue<Buffer> transmissionQueue;
    bool requestDone = false;
    std::mutex mutex;
    std::condition_variable condition;

    auto sendErrorMessage = 
      [&transmissionQueue, &condition] (std::string && message)
      {
        messaging::ErrorMessage errorMessage;
        errorMessage.set_message(std::move(message));
        Buffer errorMessageBuffer;
        serializeMessage(errorMessage, errorMessageBuffer);

        transmissionQueue.push(std::move(errorMessageBuffer));
        condition.notify_one();
      };

    LOG(INFO) << "Serving request from " << clientSocket->getHostname() << ":"
              << clientSocket->getPort();

    // <--- Receving user request ---> //
    std::vector<char> buffer;
    size_t readBytes;

    try {
      readBytes = clientSocket->receive(buffer);
    }
    catch (exceptions::NetworkError & error) {
      LOG(ERROR) << "Error receiving client's request";
      return;
    }

    // <--- Parsing user request ---> //
    messaging::FileTransmissionRequest fileRequest;
    if (not deserializeMessage(buffer, fileRequest)) {
      LOG(ERROR) << "Error parsing client's request";
      sendErrorMessage("Invalid request message");

      return;
    }

    LOG(INFO) << std::boolalpha << "Client's request {"
              << "path: " << fileRequest.filename() 
              << ", mode: " << FileRequestMode_Name(fileRequest.mode())
              << ", compressor: " << Compressor_Name(fileRequest.compressor())
              << ", compressionLevel: " << fileRequest.compressionlevel()
              << "}";

    // <--- Preparing users file user request ---> //
    std::shared_ptr<FileProcessingStrategy> fileProcessor;

    try {
      fileProcessor = Server::configureFileProcessor(fileRequest,
                                                     resourceState,
                                                     transmissionQueue,
                                                     clientSocket,
                                                     performanceDataWriter,
                                                     decisionTree);
    }
    catch (exceptions::InvalidCompressorError & error) {
      sendErrorMessage(error.what());
      LOG(ERROR) << "Error configuring file processor: " << error.what();
      return;
    }
    catch (exceptions::InvalidCompressionLevelError & error) {
      sendErrorMessage(error.what());
      LOG(ERROR) << "Error configuring file processor: " << error.what();
      return;
    }
    catch (std::runtime_error & error) {
      sendErrorMessage(error.what());
      LOG(ERROR) << "Error configuring file processor: " << error.what();
      return;
    }

    try {
      fileProcessor->preparePath(fileRequest.filename());
    }
    catch (exceptions::IOError & error) {
      sendErrorMessage(error.what());
      return;
    }
    catch (std::bad_alloc & error) {
      sendErrorMessage("An internal error occured and the request could not "
                       "be processed");
      return;
    }

    // <--- Setting up transmission thread ---> //
    LOG(INFO) << "Setting transmission thread up";
    auto transmissionThread =
      transmissionThreadPool.run(Server::transmit, clientSocket,
                                 std::ref(transmissionQueue),
                                 std::ref(requestDone), std::ref(mutex),
                                 std::ref(condition),
                                 std::ref(resourceState));

    Buffer chunk(1.1 * fileProcessor->getChunkSize() * 1024);
    Buffer fileInitialMessageBuffer;
    Buffer chunkHeaderBuffer;

    auto tic = std::chrono::high_resolution_clock::now();

    while(fileProcessor->hasNextFile()) {
      size_t fileSize;

      try {
        fileSize = fileProcessor->openNextFile();
      }
      catch (exceptions::IOError & error) {
        sendErrorMessage(error.what());
        continue;
      }
      catch (exceptions::CompressionError & error) {
        sendErrorMessage(error.what());
        continue;
      }

      messaging::FileInitialMessage fileInitialMessage;
      fileInitialMessage.set_filename(fileProcessor->getCurrentFileName());
      fileInitialMessage.set_filesize(fileSize);
      fileInitialMessage.set_chunksize(fileProcessor->getChunkSize());
      if (not fileProcessor->hasNextFile()) {
        fileInitialMessage.set_lastfile(true);
      }
      serializeMessage(fileInitialMessage, fileInitialMessageBuffer);

      LOG(INFO) << "Sending initial message for file "
                << fileProcessor->getCurrentFileName()
                << ", which is "
                << (fileInitialMessage.lastfile() ? "" : "not ")
                << "the last file";

      transmissionQueue.push(std::move(fileInitialMessageBuffer));
      condition.notify_one();

      uint64_t nChunks = 0;
      Compressor usedCompressor;

      while(fileProcessor->hasNextChunk()) {
        try {
          usedCompressor = fileProcessor->getNextChunk(chunk);
        }
        catch (exceptions::IOError & error) {
          // Should anything be done?
          LOG(ERROR) << "Error retreiving chunk #" << nChunks + 1;
          break;
        }

        // Antes de enviar el chunk, debo enviar el chunk header
        messaging::ChunkHeader chunkHeader;
        chunkHeader.set_compressor(usedCompressor);
        chunkHeader.set_chunkposition(nChunks++);
        if (not fileProcessor->hasNextChunk()) {
          chunkHeader.set_lastchunk(true);
        }
        serializeMessage(chunkHeader, chunkHeaderBuffer);

        LOG(INFO) << "Sending header and chunk #" << nChunks << " with size "
                  << chunk.getSize();

        transmissionQueue.push(std::move(chunkHeaderBuffer));
        transmissionQueue.push(std::move(chunk));
        condition.notify_one();
      }
    }

    auto toc = std::chrono::high_resolution_clock::now();

    std::cout << std::chrono::duration_cast<std::chrono::seconds>(
                    toc - tic).count() << " secs" << std::endl;

    if (fileRequest.mode() == TRAIN) {
      transmissionQueue.clear();
      sendErrorMessage("Done traning");
      sendErrorMessage("Done traning");
    }

    {
      std::unique_lock<std::mutex> guard(mutex);
      requestDone = true;
    }
    condition.notify_one();
    transmissionThread.wait();

    LOG(INFO) << "Finished sending file to client";
  }

  void Server::transmit(std::shared_ptr<TCPSocket> clientSocket,
                        SynchronousQueue<Buffer> & transmissionQueue,
                        bool & requestDone, std::mutex & mutex,
                        std::condition_variable & condition,
                        ResourceState & resourceState)
  {
    LOG(INFO) << "Transmission thread set up"; 

    Buffer buffer;
    bool dequeued;
    bool done = false;
    std::size_t bytesSent = 0, currentBytesInBuffer;
    std::chrono::time_point<std::chrono::high_resolution_clock> baseTime,
                                                                currentTime;
    float elapsedTime;

    baseTime = std::chrono::high_resolution_clock::now();

    auto conditionChecker = [&transmissionQueue, &requestDone] ()
                            {
                              return not transmissionQueue.isEmpty() or
                                     requestDone;
                            };

    while (not done) {   
      {
        std::unique_lock<std::mutex> guard(mutex);
        condition.wait(guard, conditionChecker);

        dequeued = transmissionQueue.pop(buffer);
        done = transmissionQueue.isEmpty() and requestDone;
      }

      if (dequeued) {
        try {
          currentTime = std::chrono::high_resolution_clock::now();
          elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                            currentTime - baseTime
                          ).count();

          if (elapsedTime < 10) {
          //if (elapsedTime == 0) {
            bytesSent += clientSocket->send(buffer);
          }
          else {
            currentBytesInBuffer = clientSocket->getSendBufferSize();
            resourceState.bandwidth.store(8E-3 *
                                          (bytesSent - currentBytesInBuffer) /
                                            elapsedTime);

#ifdef BANDWIDTH_TEST

            std::cout << resourceState.bandwidth << " Mbits/s" << std::endl;

#endif

            bytesSent = currentBytesInBuffer + clientSocket->send(buffer);
            baseTime = std::chrono::high_resolution_clock::now();
          }
        }
        catch (exceptions::NetworkError & error) {
          LOG(ERROR) << "Error sending message to client: " << error.what();
          break;
        }
      }
    }

    resourceState.bandwidth.store(0);

    LOG(INFO) << "Transmission thread finished";
  }

  std::shared_ptr<FileProcessingStrategy> Server::configureFileProcessor(
      const messaging::FileTransmissionRequest & fileRequest,
      const ResourceState & resourceState,
      const SynchronousQueue<Buffer> & transmissionQueue,
      const std::shared_ptr<TCPSocket> & clientSocket,
      std::shared_ptr<io::PerformanceDataWriter> performanceDataWriter,
      const DecisionTree & decisionTree
    )
  {
    if (not fileRequest.IsInitialized()) {
      return nullptr;
    }

    std::shared_ptr<AutomaticCompressionStrategy> compressor;
    std::size_t chunkSize = 64;

    switch (fileRequest.mode()) {
      case NO_COMPRESSION:
        compressor = std::make_shared<Copy>();
        break;

      case AUTOCOMP:
        compressor = std::make_shared<AutoCompCompressor<net::TCPSocket>>(
                        &decisionTree, &resourceState, clientSocket,
                        performanceDataWriter);
        break;

      case COMPRESS:
      {
        auto singleCompressor =
            std::make_shared<SingleCompressor>(performanceDataWriter);
        singleCompressor->setCompressor(fileRequest.has_compressor()
                                          ? fileRequest.compressor()
                                          : (Compressor) 0,
                                        fileRequest.has_compressionlevel()
                                          ? fileRequest.compressionlevel()
                                          : -1);
        compressor = singleCompressor;
        chunkSize = 512;
        break;
      }

      case PRE_COMPRESS:
      {
        auto fileProcessor = std::make_shared<PreCompressingFileProcessor>();
        fileProcessor->setCompressor(fileRequest.has_compressor()
                                      ? fileRequest.compressor()
                                      : (Compressor) 0,
                                     fileRequest.has_compressionlevel()
                                      ? fileRequest.compressionlevel()
                                      : -1);

        return fileProcessor;
      }

      case TRAIN:
      {
        auto trainCompressor =
          std::make_shared<TrainingCompressor>(&resourceState,
                                               &transmissionQueue,
                                               clientSocket,
                                               performanceDataWriter);

        trainCompressor->setCompressor(fileRequest.has_compressor()
                                      ? fileRequest.compressor()
                                      : (Compressor) 0,
                                     fileRequest.has_compressionlevel()
                                      ? fileRequest.compressionlevel()
                                      : -1);
        compressor = trainCompressor;
        break;
      }

      default:
      {
        std::string errorMessage("Invalid file request mode ");
        errorMessage.append(FileRequestMode_Name(fileRequest.mode()));
        throw exceptions::InvalidRequestParameterError(errorMessage);
      }
    }

    return std::make_shared<FileProcessor>(chunkSize, compressor);
  }

  } // namespace net
} // namespace autocomp
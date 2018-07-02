/**
 *  AutoComp Client
 *  client.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/29/2018
 */

#include "network/client/client.hpp"

#include <iostream>

namespace autocomp
{
  namespace net
  {

  Client::Client(const std::string & serverHostname,
                 const unsigned short & serverPort)
    : serverHostname(serverHostname),
      serverPort(serverPort),
      doneReceiving(true),
      preCompression(false),
      bandwidthModulatorPID(-1)
  {
    this->compressors.emplace(
      ZLIB, std::unique_ptr<ZlibCompressor>(new ZlibCompressor()));
    this->compressors.emplace(
      SNAPPY, std::unique_ptr<SnappyCompressor>(new SnappyCompressor()));
    this->compressors.emplace(
      LZO, std::unique_ptr<LZOCompressor>(new LZOCompressor()));
    this->compressors.emplace(
      BZIP2, std::unique_ptr<Bzip2Compressor>(new Bzip2Compressor()));
    this->compressors.emplace(
      LZMA, std::unique_ptr<LZMACompressor>(new LZMACompressor()));
    this->compressors.emplace(
      FPC, std::unique_ptr<FPCCompressor>(new FPCCompressor()));
  }

  Client::~Client()
  {
    this->shutdown();
  }

  void Client::init()
  {
    // ---> Logging <--- //
    this->initLogger();

    LOG(INFO) << "Initializing AutoComp client";

    this->doneReceiving = false;
    this->preCompression = false;

    // ---> Decompression thread <--- //
    LOG(INFO) << "Initializing decompression thread";
    this->decompressionThread = std::thread(&Client::decompress, this);

    // ---> Connecting with server <--- //
    LOG(INFO) << "Connecting to server";
    try {
      this->socket.connect(serverHostname, serverPort);
    }
    catch (exceptions::NetworkError & error) {
      LOG(ERROR) << error.what();
      throw error;
    }
  }

  void Client::shutdown()
  {
    if (this->doneReceiving) {
      return;
    }

    LOG(INFO) << "Shutting client down";

    LOG(INFO) << "Closing socket";
    this->socket.close();

    if (this->bandwidthModulatorPID > 0) {
      LOG(INFO) << "Shutting bandwidth modulador down";
      ::kill(this->bandwidthModulatorPID, SIGTERM);
      bandwidthModulatorPID = -1;
    }

    LOG(INFO) << "Joining decompression thread";
    {
      std::unique_lock<std::mutex> guard(this->mutex);
      this->doneReceiving = true;
    }
    this->condition.notify_all();

    if (this->decompressionThread.joinable()) {
      this->decompressionThread.join();
    }
  }

  void Client::requestFile(const std::string & path,
                           const FileRequestMode & mode,
                           const Compressor * compressor,
                           const int * compressionLevel,
                           const std::string & destinationDirectory)
  {
    LOG(INFO) << std::boolalpha
              << "Requesting file " << path << " with parameters = {"
              << "mode: " << FileRequestMode_Name(mode)
              << ", compressor: " << (compressor
                                        ? Compressor_Name(*compressor)
                                        : "")
              << ", compressionLevel: " << (compressionLevel
                                              ? *compressionLevel
                                              : -1)
              << "} from server "
              << this->serverHostname << ":" << this->serverPort;

    auto deserializeAndCheckMessage =
      [] (std::vector<char> & buffer, auto & message,
          messaging::ErrorMessage & errorMessage)
      {
        if (deserializeMessage(buffer, message)) {
          return true;
        }

        errorMessage.Clear();
        deserializeMessage(buffer, errorMessage);

        return false;
      };

    auto receiveMessage =
      [this] (auto & buffer)
      {
        try {
          return this->socket.receive(buffer);
        }
        catch (exceptions::NetworkError & error) {
          this->shutdown();
          throw error;
        }
      };

    // <--- Serializing and sending file transmission request ---> //
    messaging::FileTransmissionRequest request = 
      this->configureFileRequestMessage(path, mode, compressor, 
                                        compressionLevel);
    std::vector<char> requestMessageBuffer, fileInitialMessageBuffer,
                      chunkHeaderBuffer;
    serializeMessage(request, requestMessageBuffer);

    try {
      this->socket.send(requestMessageBuffer);

      /*
      if (mode == TRAIN) {
        LOG(INFO) << "Launching bandwidth modulador process";
        this->launchBandwidthModulator();
      }
      */
    }
    catch (exceptions::NetworkError & error) {
      LOG(ERROR) << "Error sending file request message: " << error.what();
      this->shutdown();
      throw error;
    }
    catch (std::runtime_error & error) {
      LOG(ERROR) << "Error launching bandwidth modulator: " << error.what();
      this->shutdown();
      throw error;
    }

    // <--- Receiving file(s) ---> //
    bool lastFile = false;
    messaging::ErrorMessage errorMessage;

    this->destinationDirectory = destinationDirectory;
    if (this->destinationDirectory.back() != '/') {
      this->destinationDirectory.push_back('/');
    }
    this->requestedPath = path;

    while (not lastFile) {
      receiveMessage(fileInitialMessageBuffer);

      messaging::FileInitialMessage fileInitialMessage;

      if(not deserializeAndCheckMessage(fileInitialMessageBuffer,
                                        fileInitialMessage, errorMessage)) {
        std::string errorMessageStr(errorMessage.IsInitialized()
                                      ? errorMessage.message()
                                      : "Received invalid file initial message "
                                        "from server");
        LOG(ERROR) << "Error receiving file initial message: "
                   << errorMessageStr;
        this->shutdown();
        throw exceptions::NetworkError(errorMessageStr);
      }

      LOG(INFO) << std::boolalpha
                << "Received file initial message {"
                << "filename: " << fileInitialMessage.filename()
                << ", fileSize: " << fileInitialMessage.filesize()
                << ", chunkSize: " << fileInitialMessage.chunksize()
                << ", lastFile: " << fileInitialMessage.lastfile();

      // <--- Send file info to the decompression thread ---> //
      this->decompressionQueue.push({
                                      fileInitialMessage,
                                      messaging::ChunkHeader(),
                                      Buffer()
                                    });
      this->condition.notify_one();

      // <--- Receive and enqueue chunks ---> //
      Buffer chunk;
      bool lastChunk = false;
      int nChunks = 0;

      while (not lastChunk) {
        // <--- Receive chunk header ---> //
        receiveMessage(chunkHeaderBuffer);
        messaging::ChunkHeader chunkHeader;

        if(not deserializeAndCheckMessage(chunkHeaderBuffer, chunkHeader,
                                          errorMessage)) {
          std::string errorMessageStr(errorMessage.IsInitialized()
                                        ? errorMessage.message()
                                        : "Received invalid chunk header from "
                                          "server");
          LOG(ERROR) << "Error receiving chunk header: " << errorMessageStr;
          this->shutdown();
          throw exceptions::NetworkError(errorMessageStr);
        }

        LOG(INFO) << "Receiving chunk";

        // <--- Receive chunk ---> //
        size_t bytesReceived = receiveMessage(chunk);
        chunk.setSize(bytesReceived);

        LOG(INFO) << "Received chunk #" << ++nChunks << " with size " 
                  << chunk.getSize();

        // <--- Enqueue chunk for decompression ---> //
        this->decompressionQueue.push({
                                        messaging::FileInitialMessage(),
                                        chunkHeader,
                                        std::move(chunk)
                                      });
        this->condition.notify_one();

        // <--- Check and possibly change lastChunk flag ---> //
        if (chunkHeader.has_lastchunk()) {
          lastChunk = chunkHeader.lastchunk();
        }
      }

      // <--- Check and possibly change lastFile flag ---> //
      if (fileInitialMessage.has_lastfile()) {
        lastFile = fileInitialMessage.lastfile();
      }
    }

    LOG(INFO) << "File transfer completed";

    this->shutdown();
  }

  void
  Client::decompress()
  {
    bool done = false;
    bool dequeued;
    Buffer decompressedChunk;
    DecompressionQueueEntry entry;
    std::string currentFileName;
    PreCompressingFileProcessor preCompressingFileProcessor;

    std::ofstream out;
    size_t bytesReceived = 0, currentFileSize = 0;

    char absoluteFilename[constants::MAX_STRING_LENGTH];

    while(not done) {
      {
        std::unique_lock<std::mutex> guard(this->mutex);
        condition.wait(guard,
                       [this]()
                       { 
                          return not this->decompressionQueue.isEmpty() or
                                 this->doneReceiving;
                       });

        dequeued = this->decompressionQueue.pop(entry);

        done = this->decompressionQueue.isEmpty() and this->doneReceiving;
      }

      if (not dequeued) {
        continue;
      }

      // File initial message
      if (entry.fileInitialMessage.IsInitialized()) {
        ::strncpy(absoluteFilename, entry.fileInitialMessage.filename().c_str(),
                  constants::MAX_STRING_LENGTH); 
        char * filename = ::basename(absoluteFilename);
        currentFileName = this->destinationDirectory + filename;

        // Add file extenssion
        if (this->preCompression) {
          currentFileName.append(constants::COMPRESSED_FILE_EXTENSSIONS
                                  .at(this->preCompressingCompressor));
        }

        out.open(currentFileName, std::ofstream::out | std::ofstream::binary |
                                  std::ofstream::trunc);

        if (out.fail()) {
          LOG(ERROR) << "Could not create/open the file "
                     << this->destinationDirectory << filename;
          // What else should I do?
        }

        bytesReceived = 0;
        currentFileSize = entry.fileInitialMessage.filesize();

        decompressedChunk.setData("");
        decompressedChunk.resize(entry.fileInitialMessage.chunksize() * 1024);
      }
      // New chunk of already open file. Decompress
      else {
        // <--- Decompress chunk ---> //
        if (entry.chunkHeader.compressor() != COPY
            and not this->preCompression) {
          try {
            this->compressors.at(entry.chunkHeader.compressor())
                             ->decompress(entry.chunk, decompressedChunk);
          }
          catch (exceptions::DecompressionError & error) {
            // Should anything be done?
            // This should never happen
            LOG(ERROR) << "Error decompressing chunk of file "
                       << currentFileName << ": " << error.what();
          }
        }
        else {
          decompressedChunk.swap(entry.chunk);
        }

        out.write(decompressedChunk.getData(), decompressedChunk.getSize());
        bytesReceived += decompressedChunk.getSize();

        if (entry.chunkHeader.has_lastchunk() and
            entry.chunkHeader.lastchunk()) {
          out.close();

          // Decompress file
          if (this->preCompression) {
            // Decompressed file name
            size_t extenssionIndex = currentFileName.find_last_of('.');
            std::string decompressedFileName(
                currentFileName.substr(0, extenssionIndex)
              );

            try {
              preCompressingFileProcessor.decompressFile(
                                              currentFileName,
                                              decompressedFileName,
                                              this->preCompressingCompressor
                                            );
            }
            catch (exceptions::DecompressionError & error) {
              // Should anything be done?
              // This should never happen
              LOG(ERROR) << "Error decompressing file " << currentFileName
                         << ": " << error.what();
            }

            //if (this->preCompressingCompressor != COPY) {
            //  ::remove(currentFileName.c_str());
            //}
          }
          else if (bytesReceived != currentFileSize) {
            // ERROR: corrupted file
            // What to do?
            LOG(ERROR) << "Corrupted file " << currentFileName
                       << ": its size should be " << currentFileSize
                       << " but actually is " << bytesReceived;
          }
        }
      }
    }
  }

  messaging::FileTransmissionRequest
  Client::configureFileRequestMessage(const std::string & path,
                                      const FileRequestMode & mode,
                                      const Compressor * compressor,
                                      const int * compressionLevel)
  {
    messaging::FileTransmissionRequest message;

    message.set_filename(path);
    message.set_mode(mode);

    if (mode == PRE_COMPRESS) {
      this->preCompression = true;
    }

    this->preCompressingCompressor = (Compressor) 0;

    if (compressor and Compressor_IsValid(*compressor)) {
      message.set_compressor(*compressor);
      this->preCompressingCompressor = *compressor;
    }

    if (compressionLevel and *compressionLevel >= 0) {
      message.set_compressionlevel(*compressionLevel);
    }

    return message;
  }

  void Client::initLogger()
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
                                             Client::logFormatter);
    auto headerChanging = logHandle->call(&g3::FileSink::overrideLogHeader, "");

    logFormatChanging.wait();
    headerChanging.wait();
  }

  std::string Client::logFormatter(const g3::LogMessage & logMessage)
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

  ::pid_t Client::launchBandwidthModulator()
  {
    int errnoValue = 0;

    this->bandwidthModulatorPID = ::vfork();

    if (this->bandwidthModulatorPID == -1) {
      throw std::runtime_error("Could not fork bandwidth modulator process");
    }

    // Child
    if (this->bandwidthModulatorPID == 0) {
      execlp(constants::BANDWIDTH_MODULATOR_SCRIPT.c_str(),
             constants::BANDWIDTH_MODULATOR_SCRIPT.c_str(),
             "--modulate", "--interface", "enp3s0", "1000", "10000", "100000",
             nullptr);
      errnoValue = errno;

      _exit(1);
    }

    // Parent
    // The child exited or exec'ed:
    if (errnoValue) {
      std::string errorMessage("Could not execute bandwidth modulator script ");
      errorMessage.append(constants::BANDWIDTH_MODULATOR_SCRIPT.c_str())
                  .append(" because child (bandwidth modulator) process exited "
                          "after excelp with errno code ")
                  .append(std::to_string(errnoValue))
                  .append(" (")
                  .append(std::strerror(errnoValue))
                  .append(")");

      throw std::runtime_error(errorMessage);
    }

    int childExitStatus;
    ::pid_t waitpidResult = ::waitpid(this->bandwidthModulatorPID,
                                      &childExitStatus, WNOHANG);

    if (waitpidResult == -1) {
      throw std::runtime_error("waitpid() exited with error (-1) when waiting "
                               "for child (bandwidth modulator) process");
    }
    
    if (waitpidResult > 0) {      
      throw std::runtime_error("Could not launch bandwidth modulator process: "
                               "the process exited early");
    }

    return this->bandwidthModulatorPID;
  }

  } // namespace net
} // namespace autocomp
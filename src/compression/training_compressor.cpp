/**
 *  AutoComp "Compression" Strategy for retrieving training data
 *  training_compressor.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 09/04/2018
 */

#include "compression/training_compressor.hpp"

namespace autocomp {

// TrainingCompressor constructor
TrainingCompressor::TrainingCompressor(
    const ResourceState * resourceState,
    const SynchronousQueue<Buffer> * transmissionQueue,
    const std::shared_ptr<net::TCPSocket> & clientSocket,
    std::shared_ptr<io::PerformanceDataWriter> & performanceDataWriter
  )
  : SingleCompressor(performanceDataWriter),
    resourceState(resourceState),
    transmissionQueue(transmissionQueue),
    clientSocket(clientSocket),
    clientSocketSendBufferCapacity(clientSocket->getSendBufferCapacity()),
    cpuModulatorPID(-1)
{
  if (not resourceState) {
    throw std::domain_error("resourceState must not be null");
  }
  
  if (not clientSocket) {
    throw std::domain_error("clientSocket must not be null");
  }

  if (not transmissionQueue) {
    throw std::domain_error("transmissionQueue must not be null");
  }

  /*
  // Insert snappy
  this->compressors.insert(
      std::make_pair(SNAPPY,
                     CompressorType(std::make_shared<SnappyCompressor>(), 0))
    );

  // Insert zlib
  std::vector<int> zlibLevels({1, 5, 8});
  for (int & level : zlibLevels) {
    this->compressors.insert(
        std::make_pair(ZLIB,
                       CompressorType(std::make_shared<ZlibCompressor>(level),
                                      level))
      );
  }

  // Insert lzo
  std::vector<int> lzoLevels({6, 7, 8});
  for (int & level : lzoLevels) {
    this->compressors.insert(
        std::make_pair(LZO,
                       CompressorType(std::make_shared<LZOCompressor>(level),
                                      level))
      );
  }

  // Insert bzip2
  std::vector<int> bzip2Levels({1, 5, 9});
  for (int & level : bzip2Levels) {
    this->compressors.insert(
        std::make_pair(BZIP2,
                       CompressorType(std::make_shared<Bzip2Compressor>(level),
                                      level))
      );
  }

  // Insert lzma
  std::vector<int> lzmaLevels({1, 3, 6});
  for (int & level : lzmaLevels) {
    this->compressors.insert(
        std::make_pair(LZMA,
                       CompressorType(std::make_shared<LZMACompressor>(level),
                                      level))
      );
  }

  //Insert copy
  this->compressors.insert(std::make_pair(COPY, CompressorType(nullptr, 0)));
  */

  //this->launchCPUModulator();
}

TrainingCompressor::~TrainingCompressor()
{
  if (this->cpuModulatorPID > 0) {
    ::kill(this->cpuModulatorPID, SIGTERM);
  }
}

/*
// Compresses the data in the input buffer into the output buffer.
Compressor
TrainingCompressor::compress(const Buffer & inData, Buffer & outData) const
{
  Compressor currentCompressor;
  CompressorType compressorPair;
  CompressorPointer compressor;
  int currentCompressionLevel;

  float compressionRate, compressionRatio, compressionTime;
  Compressor bestCompressor, secondBestCompressor;
  int bestCompressionLevel, secondBestCompressionLevel;
  float bestEfectiveTransmissionRate, secondBestEfectiveTransmissionRate,
        currentEfectiveTransmissionRate;
  Buffer tmpOutData(inData.getSize() * 1.1);

  bestEfectiveTransmissionRate = secondBestEfectiveTransmissionRate = 0;
  bestCompressor = secondBestCompressor = COPY;
  bestCompressionLevel = secondBestCompressionLevel = 0;

  // Get resource state (current opportunity)
  float cpuLoad = this->resourceState->cpuLoad;
  float availableBandwidth = this->resourceState->bandwidth;
  float sendBufferLoad = this->getClientSocketSendBufferLoad();
  int transmissionQueueSize = this->transmissionQueue->getSize();
  int bytecounting = this->getBytecounting(inData);

  for (auto compressorEntry : this->compressors) {
    currentCompressor = compressorEntry.first;
    compressorPair = compressorEntry.second;
    compressor = compressorPair.first;
    currentCompressionLevel = compressorPair.second;

    if (currentCompressor != COPY) {
      // Compressing while measuring compression time
      auto startTime = std::chrono::high_resolution_clock::now();
      compressor->compress(inData, tmpOutData);
      auto endTime = std::chrono::high_resolution_clock::now();

      compressionTime = std::chrono::duration_cast<std::chrono::microseconds>(
                            endTime - startTime
                          ).count();
      // Compression rate in Mbits/s (8e-6 Mbits in 1 byte and 1e-6 sec in 1 us)
      compressionRate = 8 * tmpOutData.getSize() / compressionTime;
      compressionRatio = inData.getSize() / (float) tmpOutData.getSize();
    }
    else {
      compressionRate = std::numeric_limits<float>::max();
      compressionRatio = 1;
    }

    // Is current compressor the best compressor?
    currentEfectiveTransmissionRate =
      this->getEfectiveTransmissionRate(availableBandwidth, compressionRate,
                                        compressionRatio);

    //std::cout << "  " << Compressor_Name(currentCompressor)
    //          << "  " << currentEfectiveTransmissionRate << std::endl;

    if (currentEfectiveTransmissionRate > bestEfectiveTransmissionRate) {
      // The best compressor is now the second best
      secondBestCompressor = bestCompressor;
      secondBestCompressionLevel = bestCompressionLevel;
      secondBestEfectiveTransmissionRate = bestEfectiveTransmissionRate;

      // Update the best compressor
      bestCompressor = currentCompressor;
      bestCompressionLevel = currentCompressionLevel;
      bestEfectiveTransmissionRate = currentEfectiveTransmissionRate;

      if (currentCompressor != COPY) {
        outData.swap(tmpOutData);
      }
    }
    else if (currentEfectiveTransmissionRate >
              secondBestEfectiveTransmissionRate) {
      secondBestCompressor = currentCompressor;
      secondBestCompressionLevel = currentCompressionLevel;
      secondBestEfectiveTransmissionRate = currentEfectiveTransmissionRate;
    }
  }

  // Save data for best and second best compressors and resource state
  this->performanceDataWriter->write(
      {
        std::to_string(cpuLoad),
        std::to_string(availableBandwidth),
        std::to_string(sendBufferLoad),
        std::to_string(transmissionQueueSize),
        std::to_string(bytecounting),
        Compressor_Name(bestCompressor),
        std::to_string(bestCompressionLevel),
        Compressor_Name(secondBestCompressor),
        std::to_string(secondBestCompressionLevel)
      }
    );
  
  //std::cout << cpuLoad                               << ","
  //          << availableBandwidth                    << ","
  //          << sendBufferLoad                        << ","
  //          << transmissionQueueSize                 << ","
  //          << bytecounting                          << ","
  //          << Compressor_Name(bestCompressor)       << ","
  //          << bestCompressionLevel                  << ","
  //          << Compressor_Name(secondBestCompressor) << ","
  //          << secondBestCompressionLevel            << std::endl;
  

  if (bestCompressor == COPY) {
    outData.setSize(0);
  }

  return bestCompressor;
}
*/

Compressor
TrainingCompressor::compress(const Buffer & inData, Buffer & outData) const
{
  /*
  static bool test(false);

  float sendBufferLoad = this->getClientSocketSendBufferLoad();

  if (test and sendBufferLoad < 0.1) {
    return COPY;
  }

  test = not test;
  */

  int bytecounting = this->getBytecounting(inData);

  if (bytecounting > 100) {
    return COPY;
  }

  float compressionRate, compressionRatio, compressionTime;

  // Get resource state (current opportunity)
  float cpuLoad = this->resourceState->cpuLoad;
  float availableBandwidth = this->resourceState->bandwidth;
  int transmissionQueueSize = this->transmissionQueue->getSize();

  if (this->currentCompressor != COPY) {
    auto compressor = this->compressors[this->currentCompressor];

    auto tic = std::chrono::high_resolution_clock::now();
    compressor->compress(inData, outData);
    auto toc = std::chrono::high_resolution_clock::now();

    compressionTime =
      std::chrono::duration_cast<std::chrono::microseconds>(toc - tic).count();
    // Compression rate in Mbits/s (8e-6 Mbits in 1 byte and 1e-6 sec in 1 us)
    compressionRate = 8 * outData.getSize() / compressionTime;
    compressionRatio = inData.getSize() / (float) outData.getSize();
  }
  else {
    compressionRate = availableBandwidth;
    compressionRatio = 1;
  }

  // Save performance data.
  // NOTE: In production, this should be saved in memory.
  this->performanceDataWriter->write(
      Compressor_Name(this->currentCompressor), this->currentCompressionLevel,
      cpuLoad, availableBandwidth, bytecounting, compressionRate, compressionRatio
      //this->getEfectiveTransmissionRate(availableBandwidth, compressionRate,
      //                                  compressionRatio)
    );

  return this->currentCompressor;
}

inline float TrainingCompressor::getClientSocketSendBufferLoad() const
{
  return this->clientSocket->getSendBufferSize() / 
          (float) this->clientSocketSendBufferCapacity;
}

inline float TrainingCompressor::getEfectiveTransmissionRate(
    const float & availableBandwidth,
    const float & compressionRate,
    const float & compressionRatio
  ) const
{
  return std::min(availableBandwidth, compressionRate) * compressionRatio;
}

inline int TrainingCompressor::getBytecounting(const Buffer & inData) const
{
  static const float subChunkProportion = 0.1;
  static const std::vector<float> relativeSubChunkPositions({0.10, 0.45, 0.80});

  int subChunkSize = subChunkProportion * inData.getSize();
  int offset;

  float averageBytecounting = 0;

  for (const float & relativePosition : relativeSubChunkPositions) {
    offset = relativePosition * inData.getSize();
    averageBytecounting += bytecounting(inData.getData() + offset,
                                        subChunkSize);
  }

  return std::round(averageBytecounting /
                      (float) relativeSubChunkPositions.size());
}

::pid_t TrainingCompressor::launchCPUModulator()
{
  int errnoValue = 0;

  this->cpuModulatorPID = ::vfork();

  if (this->cpuModulatorPID == -1) {
    throw std::runtime_error("Could not fork CPU modulator process");
  }

  // Child
  if (this->cpuModulatorPID == 0) {
    execlp(constants::CPU_MODULATOR_SCRIPT.c_str(),
           constants::CPU_MODULATOR_SCRIPT.c_str(), "--modulate", nullptr);
    errnoValue = errno;

    _exit(1);
  }

  // Parent
  // The child exited or exec'ed:
  if (errnoValue) {
    std::string errorMessage("Could not execute CPU modulator script ");
    errorMessage.append(constants::CPU_MODULATOR_SCRIPT.c_str())
                .append(" because child (CPU modulator) process exited after "
                        "excelp with errno code ")
                .append(std::to_string(errnoValue))
                .append(" (")
                .append(std::strerror(errnoValue))
                .append(")");

    throw std::runtime_error(errorMessage);
  }

  int childExitStatus;
  ::pid_t waitpidResult = ::waitpid(this->cpuModulatorPID, &childExitStatus,
                                    WNOHANG);

  if (waitpidResult == -1) {
    throw std::runtime_error("waitpid() exited with error (-1) when waiting "
                             "for child (CPU modulator) process");
  }
  
  if (waitpidResult > 0) {      
    throw std::runtime_error("Could not launch CPU modulator process: "
                             "the process exited early");
  }

  return this->cpuModulatorPID;
}

} // namespace autocomp

/**
 *  AutoComp Automatic and Adaptive Compression Strategy
 *  autocomp_compressor.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 09/20/2018
 */

#ifndef AC_AUTOCOMP_COMPRESSOR_HPP
#define AC_AUTOCOMP_COMPRESSOR_HPP

#include <string>
#include <map>
#include <memory>
#include <cmath>

#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "utils/data_structures.hpp"
#include "utils/functions.hpp"
#include "utils/decision_tree.hpp"
#include "network/socket/tcp_socket.hpp"
#include "messaging/compressor.pb.h"
#include "compression/automatic_compression_strategy.hpp"
#include "compression/zlib_compressor.hpp"
#include "compression/snappy_compressor.hpp"
#include "compression/lzo_compressor.hpp"
#include "compression/bzip2_compressor.hpp"
#include "compression/lzma_compressor.hpp"

namespace autocomp {

/** 
 * AutoComp Compressor class.
 */
template<class SocketType>
class AutoCompCompressor : public AutomaticCompressionStrategy
{
  const ResourceState * resourceState;

  const std::shared_ptr<SocketType> clientSocket;

  const float clientSocketSendBufferCapacity;

  const int bytesToSendUncompressed = 512 * 1024; // 512 KB

  const int bytesToSendBeforeCalculatingAgaing = 512 * 1024;

  // <--- Compressors ---> //

  // CompressorType => Pair<Compressor, Level>
  using CompressorType = std::pair<Compressor, int>;

  using CompressorPointer = std::shared_ptr<CompressionStrategy>;

  std::map<CompressorType, CompressorPointer> compressors;

  const DecisionTree * decisionTree;

public:

  /**
   * AutoCompCompressor constructor
   *
   * @ŧhrows std::bad_alloc On a memory allocation failure.
   */
  AutoCompCompressor(const DecisionTree * decisionTree,
                     const ResourceState * resourceState,
                     const std::shared_ptr<SocketType> & clientSocket,
                     const std::shared_ptr<io::PerformanceDataWriter> &
                        performanceDataWriter = nullptr);

  /**
   * @copydoc autocomp::CompressionStrategy::compress()
   */
  Compressor compress(const Buffer & inData, Buffer & outData) const;

private:

  int getCPULoadLevel(const float & cpuLoad) const;

  int getBandwidthLevel(const float & bandwidth) const;

  int getBytecoutingLevel(const float & bytecounting) const;

  float getClientSocketSendBufferLoad() const;

  int getBytecounting(const Buffer & inData) const;

}; // class AutoCompCompressor

// <--- AutoCompCompressor's methods definition ---> //

template<class SocketType>
AutoCompCompressor<SocketType>::AutoCompCompressor(
    const DecisionTree * decisionTree, const ResourceState * resourceState,
    const std::shared_ptr<SocketType> & clientSocket,
    const std::shared_ptr<io::PerformanceDataWriter> & performanceDataWriter
  )
  : AutomaticCompressionStrategy(performanceDataWriter),
    decisionTree(decisionTree),
    resourceState(resourceState),
    clientSocket(clientSocket),
    clientSocketSendBufferCapacity(clientSocket->getSendBufferCapacity())
{
  if (not decisionTree) {
    throw std::domain_error("decisionTree must not be null");
  }

  if (not resourceState) {
    throw std::domain_error("resourceState must not be null");
  }
  
  if (not clientSocket) {
    throw std::domain_error("clientSocket must not be null");
  }

  // Insert snappy
  this->compressors.insert(
      std::make_pair(std::make_pair(SNAPPY, -1),
      std::make_shared<SnappyCompressor>())
    );

  // Insert zlib level 0
  this->compressors.insert(
      std::make_pair(std::make_pair(ZLIB, 0),
      std::make_shared<ZlibCompressor>(0))
    );

  // Insert zlib, lzo, bzip2 and lzma levels 1..9
  for (int compressionLevel = 1; compressionLevel <= 9; compressionLevel++) {
    this->compressors.insert(
        std::make_pair(std::make_pair(ZLIB, compressionLevel),
        std::make_shared<ZlibCompressor>(compressionLevel))
      );

    this->compressors.insert(
        std::make_pair(std::make_pair(LZO, compressionLevel),
        std::make_shared<LZOCompressor>(compressionLevel))
      );

    this->compressors.insert(
        std::make_pair(std::make_pair(BZIP2, compressionLevel),
        std::make_shared<Bzip2Compressor>(compressionLevel))
      );

    this->compressors.insert(
        std::make_pair(std::make_pair(LZMA, compressionLevel),
        std::make_shared<LZMACompressor>(compressionLevel))
      );
  }
}

template<class SocketType>
Compressor
AutoCompCompressor<SocketType>::compress(const Buffer & inData,
                                         Buffer & outData) const
{
  static int currentBytecounting(0);
  static float currentSendBufferLoad(0.0);
  static int remainingBytesToCalculateAgain(0);
  static int remainingBytesToSendUncompressed(0);
  //static int remainingBytesToSendSnappy(0);

  if (remainingBytesToSendUncompressed > 0) {
    remainingBytesToSendUncompressed -= inData.getSize();

    return COPY;
  }

  //CompressorType compressorType;
  //CompressorPointer compressor;

  if (remainingBytesToCalculateAgain <= 0) {
    //if ((currentSendBufferLoad = this->getClientSocketSendBufferLoad()) < 0.1 or
    //    (currentBytecounting = this->getBytecounting(inData)) > 100) {

    if ((currentBytecounting = this->getBytecounting(inData)) > 100) {
      remainingBytesToSendUncompressed = this->bytesToSendUncompressed;

      return COPY;
    }

    remainingBytesToCalculateAgain = this->bytesToSendBeforeCalculatingAgaing;
  }
  else {
    remainingBytesToCalculateAgain -= inData.getSize();
  }

  if ((currentSendBufferLoad = this->getClientSocketSendBufferLoad()) < 0.05) {
    try {
      auto compressor = this->compressors.at(std::make_pair(ZLIB, 3));
      compressor->compress(inData, outData);
    }
    catch (const std::out_of_range & error) {
      return COPY;
    }

    //remainingBytesToSendSnappy = 512 * 1024;

    return ZLIB;
  }

  /*
  if (this->resourceState->bandwidth <= 2) {
    try {
      auto compressor = this->compressors.at(std::make_pair(ZLIB, 9));
      compressor->compress(inData, outData);
    }
    catch (const std::out_of_range & error) {
      return COPY;
    }

    //remainingBytesToSendSnappy = 512 * 1024;

    return ZLIB;
  }
  */

  /*
  float bandwidth = this->resourceState->bandwidth;
  
  if (bandwidth > 100) {
    try {
      auto compressor = this->compressors.at(std::make_pair(SNAPPY, -1));
      compressor->compress(inData, outData);
    }
    catch (const std::out_of_range & error) {
      return COPY;
    }

    //remainingBytesToSendSnappy = 512 * 1024;

    return SNAPPY;
  }
  */

  auto compressorType = 
    this->decisionTree->classify(
        {
          this->getCPULoadLevel(this->resourceState->cpuLoad),
          this->getBandwidthLevel(this->resourceState->bandwidth),
          this->getBytecoutingLevel(currentBytecounting)
        }
      );

  this->performanceDataWriter->write(Compressor_Name(compressorType.first));  

  if (compressorType.first == COPY) {
    return COPY;
  }

  try {
    auto compressor = this->compressors.at(compressorType);
    compressor->compress(inData, outData);
  }
  catch (const std::out_of_range & error) {
    return COPY;
  }

  return compressorType.first;
}

template<class SocketType>
inline
int AutoCompCompressor<SocketType>::getCPULoadLevel(const float & cpuLoad) const
{
  return (cpuLoad * 100 / 10);
}

template<class SocketType>
inline int 
AutoCompCompressor<SocketType>::getBandwidthLevel(const float & bandwidth) const
{
  if (bandwidth < 100) {
    return (bandwidth / 5);
  }

  if (bandwidth < 1000) {
    return (bandwidth / 100 + 19);
  }

  return 58;
}

template<class SocketType>
inline int
AutoCompCompressor<SocketType>::getBytecoutingLevel(const float & bytecounting)
  const
{
  //return (bytecounting < 100 ? bytecounting / 5 : bytecounting / 5 - 1);

  return bytecounting / 10;
}

template<class SocketType>
inline
float AutoCompCompressor<SocketType>::getClientSocketSendBufferLoad() const
{
  return this->clientSocket->getSendBufferSize() / 
            this->clientSocketSendBufferCapacity;
}

template<class SocketType>
inline
int AutoCompCompressor<SocketType>::getBytecounting(const Buffer & inData) const
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

} // namespace autocomp

#endif // AC_AUTOCOMP_COMPRESSOR_HPP
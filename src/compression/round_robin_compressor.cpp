/**
 *  AutoComp Round Robin Compressor
 *  round_robin_compressor.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/15/2018
 */

#include "compression/round_robin_compressor.hpp"

namespace autocomp {

// RoundRobinCompressor constructor
RoundRobinCompressor::RoundRobinCompressor(
    std::shared_ptr<io::PerformanceDataWriter> & performanceDataWriter)
  : AutomaticCompressionStrategy(performanceDataWriter)
{
  if (not performanceDataWriter) {
    throw std::domain_error("performanceDataWriter must not be null");
  }

  // Zlib
  CompressorPointer zlibCompressor = std::make_shared<ZlibCompressor>();
  // Snappy
  CompressorPointer snappyCompressor = std::make_shared<SnappyCompressor>();
  // LZO
  CompressorPointer lzoCompressor = std::make_shared<LZOCompressor>();
  // Bzip2
  CompressorPointer bzip2Compressor = std::make_shared<Bzip2Compressor>();
  // LZMA
  CompressorPointer lzmaCompressor = std::make_shared<LZMACompressor>();
  // FPC
  CompressorPointer fpcCompressor = std::make_shared<FPCCompressor>();

  // Insert zlib level 0
  this->compressors.insert(std::make_pair(ZLIB,
                                          CompressorType(zlibCompressor, 0)));
  // Insert snappy
  this->compressors.insert(std::make_pair(SNAPPY,
                                          CompressorType(snappyCompressor,
                                                         -1)));

  // Insert zlib, lzo, bzip2 and lzma levels 1..9
  for (int compressionLevel = 1; compressionLevel <= 9; compressionLevel++) {
    this->compressors.insert(std::make_pair(ZLIB,
                                            CompressorType(zlibCompressor,
                                                           compressionLevel)));
    this->compressors.insert(std::make_pair(LZO,
                                            CompressorType(lzoCompressor,
                                                           compressionLevel)));
    this->compressors.insert(std::make_pair(BZIP2,
                                            CompressorType(bzip2Compressor,
                                                           compressionLevel)));
    this->compressors.insert(std::make_pair(LZMA,
                                            CompressorType(lzmaCompressor,
                                                           compressionLevel)));
  }

  /*
  // Insert fpc
  int fpcCompressionLevels[] = {4, 8, 16, 20, 24, 28}
  for (int & compressionLevel : fpcCompressionLevels) {
    this->compressors.insert(std::make_pair(FPC,
                                            std::make_pair(fpcCompressor,
                                                           compressionLevel)));
    this->compressors.insert({FPC, {fpcCompressor, compressionLevel}});
  }
  */

  //Insert copy
  this->compressors.insert(std::make_pair(COPY, CompressorType(nullptr, -1)));

  // Next compressor
  this->currentCompressor = this->compressors.begin();
}

// Compresses the data in the input buffer into the output buffer.
Compressor RoundRobinCompressor::compress(const Buffer & inData,
                                          Buffer & outData) const
{
  Compressor compressorType;
  CompressorType compressorPair;
  CompressorPointer compressor;
  int compressionLevel;

  std::tie(compressorType, compressorPair) = *this->currentCompressor;
  std::tie(compressor, compressionLevel) = compressorPair;

  if (compressorType != COPY) {
    if (compressionLevel >= 0) {
      auto leveledCompressor =
        dynamic_cast<LeveledCompressor *>(compressor.get());

      if (leveledCompressor != nullptr) {
        try {
          leveledCompressor->setCompressionLevel(compressionLevel);
        }
        catch (const exceptions::InvalidCompressionLevelError & error) {
          throw exceptions::CompressionError("RoundRobin",
                                             inData.getSize(),
                                             outData.getCapacity(),
                                             error.what());
        }
      }
      else {
        std::string errorMessage("Invalid pointer to leveled compressor ("
                                 "compressor ");
        errorMessage.append(Compressor_Name(compressorType))
                    .append(", level ")
                    .append(std::to_string(compressionLevel))
                    .append(")");
        throw exceptions::CompressionError("RoundRobin",
                                           inData.getSize(),
                                           outData.getCapacity(),
                                           errorMessage);
      }      
    }
    
    // Compressing while measuring compression time
    auto startTime = std::chrono::high_resolution_clock::now();
    compressor->compress(inData, outData);
    auto endTime = std::chrono::high_resolution_clock::now();

    CompressionPerformanceData performanceData{
      .compressor = compressor->getCompressorName(),
      .compressionLevel = (short) compressionLevel,
      .elapsedTime = 
        std::chrono::duration_cast<std::chrono::microseconds>(endTime 
                                                                - startTime)
                                                             .count(),
      .originalSize = inData.getSize(),
      .finalSize = outData.getSize()
    };

    this->performanceDataWriter->write(performanceData);
  }

  if (++this->currentCompressor == this->compressors.end()) {
    this->currentCompressor = this->compressors.begin();
  }

  return compressorType;
}

} // namespace autocomp

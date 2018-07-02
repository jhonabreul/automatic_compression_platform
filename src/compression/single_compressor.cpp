/**
 *  AutoComp Single Compressor
 *  single_compressor.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/30/2018
 */

#include "compression/single_compressor.hpp"

namespace autocomp {

// SingleCompressor constructor
SingleCompressor::SingleCompressor(std::shared_ptr<io::PerformanceDataWriter> &
                                      performanceDataWriter)
  : AutomaticCompressionStrategy(performanceDataWriter),
    compressors{
      {ZLIB,    std::make_shared<ZlibCompressor>()},
      {SNAPPY,  std::make_shared<SnappyCompressor>()},
      {LZO,     std::make_shared<LZOCompressor>()},
      {BZIP2,   std::make_shared<Bzip2Compressor>()},
      {LZMA,    std::make_shared<LZMACompressor>()},
      {FPC,     std::make_shared<FPCCompressor>()}
    },
    currentCompressor(ZLIB),
    currentCompressionLevel(6)
{
  if (not performanceDataWriter) {
    throw std::domain_error("performanceDataWriter must not be null");
  }
  
  /*
  this->compressors.insert(std::make_pair(ZLIB,
                                          std::make_shared<ZlibCompressor>()));
  this->compressors.insert(std::make_pair(SNAPPY,
                                          std::make_shared<SnappyCompressor>()));
  this->compressors.insert(std::make_pair(LZO,
                                          std::make_shared<LZOCompressor>()));
  this->compressors.insert(std::make_pair(BZIP2,
                                          std::make_shared<Bzip2Compressor>()));
  this->compressors.insert(std::make_pair(LZMA,
                                          std::make_shared<LZMACompressor>()));
  this->compressors.insert(std::make_pair(FPC,
                                          std::make_shared<FPCCompressor>()));
  */
}

Compressor SingleCompressor::getCompressor() const
{
  return this->currentCompressor;
}

int SingleCompressor::getCompressionLevel() const
{
  if (this->currentCompressor == SNAPPY or this->currentCompressor == COPY) {
    return -1;
  }

  auto leveledCompressor = dynamic_cast<LeveledCompressor *>(
                              this->compressors[this->currentCompressor].get()
                           );

  return leveledCompressor->getCompressionLevel();
}

void SingleCompressor::setCompressor(const Compressor & compressor,
                                     const int & compressionLevel)
{
  if (not Compressor_IsValid(compressor)) {
    std::string errorMessage("Maximum value for a compressor is ");
    errorMessage.append(std::to_string(Compressor_MAX))
                .append(" (")
                .append(Compressor_Name(Compressor_MAX))
                .append(")");

    throw exceptions::InvalidCompressorError(compressor, errorMessage);
  }

  if (compressor != SNAPPY and compressor != COPY) {
    auto leveledCompressor = dynamic_cast<LeveledCompressor *>(
                              this->compressors[compressor].get()
                             );

    if (compressionLevel != -1) {
      leveledCompressor->setCompressionLevel(compressionLevel);
    }
    else {
      leveledCompressor->setDefaultCompressionLevel();
    }

    this->currentCompressionLevel = leveledCompressor->getCompressionLevel();
  }
  else {
    this->currentCompressionLevel = -1;
  }

  this->currentCompressor = compressor;
}

// Compresses the data in the input buffer into the output buffer.
Compressor
SingleCompressor::compress(const Buffer & inData, Buffer & outData) const
{
  if (this->currentCompressor != COPY) {
    auto compressor = this->compressors[this->currentCompressor];

    // Compressing while measuring compression time
#ifdef MEASURE_COMPRESSION_TIME

    auto startTime = std::chrono::high_resolution_clock::now();

#endif

    compressor->compress(inData, outData);

#ifdef MEASURE_COMPRESSION_TIME

    auto endTime = std::chrono::high_resolution_clock::now();

    CompressionPerformanceData performanceData{
      .compressor = compressor->getCompressorName(),
      .compressionLevel = (short) this->compressionLevel,
      .elapsedTime = 
        std::chrono::duration_cast<std::chrono::microseconds>(endTime 
                                                                - startTime)
                                                             .count(),
      .originalSize = inData.getSize(),
      .finalSize = outData.getSize()
    };

    this->performanceDataWriter->write(performanceData);

#endif

  }

  return this->currentCompressor;
}

} // namespace autocomp

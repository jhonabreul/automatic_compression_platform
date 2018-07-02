/**
 *  AutoComp zlib Compressor
 *  zlib_compressor.cpp
 *
 *  This class implements the abstract class CompressionStrategy for
 *  compression/decompression using the ZLIB compression library.
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/13/2018
 */

#include "compression/snappy_compressor.hpp"

namespace autocomp {

// SnappyCompressor constructor 
SnappyCompressor::SnappyCompressor()
  : CompressionStrategy(Compressor_Name(SNAPPY))
{}

// Compresses the data in the input buffer into the output buffer.
void SnappyCompressor::compress(const Buffer & inData, Buffer & outData) const
{
  size_t compressedDataSize;

  // RawCompress from snappy
  snappy::RawCompress(inData.getData(), inData.getSize(),
                      outData.getData(), &compressedDataSize);

  try {
    outData.setSize(compressedDataSize);
  }
  catch (std::domain_error & error) {
    throw exceptions::CompressionError(this->compressorName,
                                       inData.getSize(),
                                       outData.getCapacity(),
                                       error.what());
  }
}

// Decompresses the data in the input buffer into the output buffer.
void SnappyCompressor::decompress(const Buffer & inData, Buffer & outData) const
{
  size_t decompressedDataSize;

  if (not snappy::GetUncompressedLength(inData.getData(), inData.getSize(),
                                        &decompressedDataSize)) {
    std::string message = "Parsing error in snappy::GetUncompressedLength";
    throw exceptions::DecompressionError(this->compressorName,
                                         inData.getSize(),
                                         outData.getCapacity(),
                                         message);
  }

  try {
    outData.setSize(decompressedDataSize);
  }
  catch (std::domain_error & error) {
    throw exceptions::DecompressionError(this->compressorName,
                                         inData.getSize(),
                                         outData.getCapacity(),
                                         error.what());
  }

  if (not snappy::RawUncompress(inData.getData(), inData.getSize(),
                                outData.getData())) {
    outData.setSize(0);
    std::string message = "Decompression error";

    throw exceptions::DecompressionError(this->compressorName,
                                         inData.getSize(),
                                         outData.getCapacity(),
                                         message);
  }
}

} // namespace autocomp
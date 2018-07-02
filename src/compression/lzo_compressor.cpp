/**
 *  AutoComp LZO Compressor
 *  zlib_compressor.cpp
 *
 *  This class implements the abstract class LeveledCompressor for
 *  compression/decompression using the LZO compression library.
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/13/2018
 */

#include "compression/lzo_compressor.hpp"

namespace autocomp {

// LZOCompressor constructor 
LZOCompressor::LZOCompressor(const int & compressionLevel)
  : LeveledCompressor(Compressor_Name(LZO), 1, 9, 3),
    workMemorySize((LZO1X_999_MEM_COMPRESS + sizeof(lzo_align_t) - 1)
                        / sizeof(lzo_align_t))
{
  this->setCompressionLevel(compressionLevel);
  this->workMemory = new lzo_align_t[workMemorySize];
}

// Compresses the data in the input buffer into the output buffer.
void LZOCompressor::compress(const Buffer & inData, Buffer & outData) const
{
  const unsigned char * originalData;
  unsigned char * compressedData;
  size_t compressedDataSize;

  originalData = reinterpret_cast<const unsigned char *>(inData.getData());
  compressedData = reinterpret_cast<unsigned char *>(outData.getData());
  compressedDataSize = outData.getCapacity();

  int compressionResultCode;

  compressionResultCode = lzo1x_999_compress_level(originalData,
                                                   inData.getSize(),
                                                   compressedData,
                                                   &compressedDataSize,
                                                   this->workMemory,
                                                   nullptr, 0, nullptr,
                                                   this->compressionLevel);

  if (compressionResultCode != LZO_E_OK) {
    std::string message = "Obtained error code ";
    message.append(std::to_string(compressionResultCode));

    throw exceptions::CompressionError(this->compressorName,
                                       inData.getSize(),
                                       outData.getCapacity(),
                                       message);
  }

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
void LZOCompressor::decompress(const Buffer & inData, Buffer & outData) const
{
  const unsigned char * originalData;
  unsigned char * decompressedData;
  size_t decompressedDataSize;

  originalData = reinterpret_cast<const unsigned char *>(inData.getData());
  decompressedData = reinterpret_cast<unsigned char *>(outData.getData());
  decompressedDataSize = outData.getCapacity();

  int decompressionResultCode;

  // uncompress from zlib
  decompressionResultCode = lzo1x_decompress(originalData, inData.getSize(),
                                             decompressedData,
                                             &decompressedDataSize, nullptr);

  if (decompressionResultCode != LZO_E_OK) {
    std::string message = "Obtained error code ";
    message.append(std::to_string(decompressionResultCode));

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
}

} // namespace autocomp
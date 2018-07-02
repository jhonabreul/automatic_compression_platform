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

#include "compression/zlib_compressor.hpp"

namespace autocomp {

// ZlibCompressor constructor 
ZlibCompressor::ZlibCompressor(const int & compressionLevel)
  : LeveledCompressor(Compressor_Name(ZLIB), 0, 9, 6)
{
  this->setCompressionLevel(compressionLevel);
}

// Compresses the data in the input buffer into the output buffer.
void ZlibCompressor::compress(const Buffer & inData, Buffer & outData) const
{
  const unsigned char * originalData;
  unsigned char * compressedData;
  size_t compressedDataSize;

  originalData = reinterpret_cast<const unsigned char *>(inData.getData());
  compressedData = reinterpret_cast<unsigned char *>(outData.getData());
  compressedDataSize = outData.getCapacity();

  int compressionResultCode;

  // compress2 from zlib
  compressionResultCode = compress2(compressedData, &compressedDataSize, 
                                    originalData, inData.getSize(),
                                    compressionLevel);

  if (compressionResultCode != Z_OK) {
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
void ZlibCompressor::decompress(const Buffer & inData, Buffer & outData) const
{
  const unsigned char * originalData;
  unsigned char * decompressedData;
  size_t decompressedDataSize;

  originalData = reinterpret_cast<const unsigned char *>(inData.getData());
  decompressedData = reinterpret_cast<unsigned char *>(outData.getData());
  decompressedDataSize = outData.getCapacity();

  int decompressionResultCode;

  // uncompress from zlib
  decompressionResultCode = uncompress(decompressedData, &decompressedDataSize, 
                                       originalData, inData.getSize());

  if (decompressionResultCode != Z_OK) {
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
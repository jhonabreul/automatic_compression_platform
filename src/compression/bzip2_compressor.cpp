/**
 *  AutoComp bzip2 Compressor
 *  bzip2_compressor.cpp
 *
 *  This class implements the abstract class LeveledCompressor for
 *  compression/decompression using the bzip2 compression library.
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/13/2018
 */

#include "compression/bzip2_compressor.hpp"

namespace autocomp {

// Bzip2Compressor constructor 
Bzip2Compressor::Bzip2Compressor(const int & compressionLevel)
  : LeveledCompressor(Compressor_Name(BZIP2), 1, 9, 9)
{
  this->setCompressionLevel(compressionLevel);
}

// Compresses the data in the input buffer into the output buffer.
void Bzip2Compressor::compress(const Buffer & inData, Buffer & outData) const
{
  unsigned int compressedDataSize;
  int compressionResultCode;

  compressedDataSize = outData.getCapacity();
  compressionResultCode = BZ2_bzBuffToBuffCompress(outData.getData(),
                                                   &compressedDataSize,
                                                   const_cast<char *>(
                                                      inData.getData()
                                                   ),
                                                   inData.getSize(),
                                                   this->compressionLevel,
                                                   0, 0);

  if (compressionResultCode != BZ_OK) {
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
void Bzip2Compressor::decompress(const Buffer & inData, Buffer & outData) const
{
  unsigned int decompressedDataSize;
  int decompressionResultCode;
  
  decompressedDataSize = outData.getCapacity();
  decompressionResultCode = BZ2_bzBuffToBuffDecompress(outData.getData(),
                                                       &decompressedDataSize,
                                                       const_cast<char *>(
                                                          inData.getData()
                                                       ),
                                                       inData.getSize(),
                                                       0, 0);

  if (decompressionResultCode != BZ_OK) {
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
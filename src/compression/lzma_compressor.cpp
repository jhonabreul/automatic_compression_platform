/**
 *  AutoComp LZMA Compressor
 *  lzma_compressor.cpp
 *
 *  This class implements the abstract class LeveledCompressor for
 *  compression/decompression using the LZMA compression library.
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/14/2018
 */

#include "compression/lzma_compressor.hpp"

namespace autocomp {

// LZMACompressor constructor.
LZMACompressor::LZMACompressor(const int & compressionLevel)
  : LeveledCompressor(Compressor_Name(LZMA), 0, 9, 6),
    BLOCK_SIZE(4096)
{
  this->setCompressionLevel(compressionLevel);
}

// Compresses the data in the input buffer into the output buffer.
void LZMACompressor::compress(const Buffer & inData, Buffer & outData) const
{
  this->_compress(inData, outData);
}

// Decompresses the data in the input buffer into the output buffer.
void LZMACompressor::decompress(const Buffer & inData, Buffer & outData) const
{
  this->_decompress(inData, outData);
}

// Initializes the stream object for compression.
void LZMACompressor::initCompressor() const
{
  this->stream = LZMA_STREAM_INIT;

  lzma_ret initResult = lzma_easy_encoder(&this->stream, this->compressionLevel,
                                          LZMA_CHECK_CRC64);

  // Return successfully if the initialization went fine.
  if (initResult == LZMA_OK) {
    return;
  }

  // Something went wrong.
  std::string errorMessage("LZMA compressor initialization failure. "
                           "Error code is ");
  errorMessage.append(std::to_string(initResult));

  throw exceptions::CompressionError(this->compressorName, 0, 0, errorMessage);
}

// Initializes the stream object for decompression
void LZMACompressor::initDecompressor() const
{
  this->stream = LZMA_STREAM_INIT;

  lzma_ret initResult = lzma_stream_decoder(&this->stream, UINT64_MAX,
                                            LZMA_CONCATENATED);

  // Return successfully if the initialization went fine.
  if (initResult == LZMA_OK) {
    return;
  }

  // Something went wrong.
  std::string errorMessage("LZMA decompressor initialization failure. "
                      "Error code is ");
  errorMessage.append(std::to_string(initResult));

  throw exceptions::DecompressionError(this->compressorName, 0, 0,
                                       errorMessage);
}

// Compresses the data in the input buffer into the output buffer using the
// LZMA compression library
void LZMACompressor::_compress(const Buffer & inData, Buffer & outData) const
{
  try {
    this->initCompressor();
  }
  catch (exceptions::CompressionError & compressionError) {
    compressionError.setBufferInputSize(inData.getSize());
    compressionError.setBufferOutputCapacity(outData.getCapacity());

    throw compressionError;
  }

  this->code<exceptions::CompressionError>(inData, outData);
}

// Decompresses the data in the input buffer into the output buffer using the
// LZMA compression library
void LZMACompressor::_decompress(const Buffer & inData, Buffer & outData) const
{
  try {
    this->initDecompressor();
  }
  catch (exceptions::DecompressionError & decompressionError) {
    decompressionError.setBufferInputSize(inData.getSize());
    decompressionError.setBufferOutputCapacity(outData.getCapacity());

    throw decompressionError;
  }

  this->code<exceptions::DecompressionError>(inData, outData);
}

// Compressed/decompresses the data in the input buffer into the output buffer
// using the LZMA compression library.
template<typename ET>
void LZMACompressor::code(const Buffer & inData, Buffer & outData) const
{
  const unsigned char * inBuffer;
  unsigned char * outBuffer;

  inBuffer = reinterpret_cast<const unsigned char *>(inData.getData());
  outBuffer = reinterpret_cast<unsigned char *>(outData.getData());

  // This will be LZMA_RUN until the end of the input buffer is reached.
  // This tells lzma_code() when there will be no more input.
  lzma_action action = LZMA_RUN;

  // Initialize input and output buffers
  this->stream.next_in = nullptr;
  this->stream.avail_in = 0;
  this->stream.next_out = outBuffer;
  this->stream.avail_out = outData.getCapacity();

  int consumedInputBytes = 0;
  std::string errorMessage;
  bool codingFailed = false;

  // Loop until the input buffer has been successfully code or until
  // an error occurs.
  while (true) {
    // Update the input buffer if it is empty.
    if (this->stream.avail_in == 0) {
      if (consumedInputBytes < inData.getSize()) {
        this->stream.next_in = inBuffer + consumedInputBytes;

        if (inData.getSize() - consumedInputBytes > this->BLOCK_SIZE) {
          this->stream.avail_in = this->BLOCK_SIZE;
        } else {
          this->stream.avail_in = inData.getSize() - consumedInputBytes;
        }

        consumedInputBytes += this->stream.avail_in;
      }
      else {
        action = LZMA_FINISH;
      }          
    }

    // Tell liblzma to do the actual coding
    lzma_ret codingResult = lzma_code(&this->stream, action);

    // The output buffer ran out of available space
    if (this->stream.avail_out == 0 and this->stream.avail_in > 0) {
      codingFailed = true;
      errorMessage = "Output buffer ran out of space";
      break;
    }

    // Normally the return value of lzma_code() will be LZMA_OK until
    // everything has been coded.
    if (codingResult != LZMA_OK) {
      // Once everything has been coded successfully, the return value of
      // lzma_code() will be LZMA_STREAM_END.
      if (codingResult == LZMA_STREAM_END) {
        try {
          outData.setSize(outData.getCapacity() - this->stream.avail_out);
        }
        catch (std::domain_error & error) {
          codingFailed = true;
          errorMessage = error.what();
          break;
        }

        break;
      }

      // It's not LZMA_OK nor LZMA_STREAM_END, so it must be an error code.
      codingFailed = true;
      errorMessage = "Obtained error code ";
      errorMessage.append(std::to_string(codingResult));
      break;
    }
  }

  lzma_end(&this->stream);

  if (codingFailed) {
    throw ET(this->compressorName, inData.getSize(), outData.getCapacity(),
             errorMessage);
  }
}

} // namespace autocomp
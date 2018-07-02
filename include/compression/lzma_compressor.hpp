/**
 *  AutoComp LZMA Compressor
 *  lzma_compressor.hpp
 *
 *  This class implements the abstract class LeveledCompressor for
 *  compression/decompression using the LZMA compression library.
 *
 *  @author Jhonathan Abreu
 *  @version 1.0 07/14/2018
 */

#ifndef AC_LZMA_COMPRESSOR_HPP
#define AC_LZMA_COMPRESSOR_HPP

#include <string>

extern "C" {
  #include "lzma.h"
}

#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "messaging/compressor.pb.h"
#include "compression/leveled_compressor.hpp"

namespace autocomp {

/** 
 * LZMA compressor class.
 *
 * Class for a compression strategy using the LZMA library.
 */
class LZMACompressor : public LeveledCompressor
{
  /**
   * LZMA compresses in blocks of size BLOCK_SIZE
   */
  const unsigned int BLOCK_SIZE;

  /**
   * Stream that the LZMA compressor uses for compression/decompression
   */
  mutable lzma_stream stream;

public:

  /**
   * LZMACompressor constructor 
   *
   * @param compressionLevel Compression level for the LZMA algorithm.
   *
   * @throws InvalidCompressionLevelError When the compression level is < 0
   *                                      or > 9
   */
  LZMACompressor(const int & compressionLevel = 6);

  /**
   * @copydoc autocomp::CompressionStrategy::compress()
   */
  void compress(const Buffer & inData, Buffer & outData) const;

  /**
   * @copydoc autocomp::CompressionStrategy::decompress()
   */
  void decompress(const Buffer & inData, Buffer & outData) const;

private:

  /**
   * Initializes the stream object for compression.
   *
   * @throws CompressionError If any library specific error occurs.
   */
  void initCompressor() const;

  /**
   * Initializes the stream object for decompression.
   *
   * @throws CompressionError If any library specific error occurs.
   */
  void initDecompressor() const;

  /**
   * Compresses the data in the input buffer into the output buffer using the
   * LZMA compression library.
   *
   * @param inData Data to be compressed
   * @param outData Buffer where the compressed data will be stored
   *
   * @throws CompressionError If any compression algorithm specific error occurs
   */
  void _compress(const Buffer & inData, Buffer & outData) const;

  /**
   * Decompresses the data in the input buffer into the output buffer using the
   * LZMA compression library.
   *
   * @param inData Data to be decompressed
   * @param outData Buffer where the decompressed data will be stored
   *
   * @throws DecompressionError If any compression algorithm specific error 
   *                            occurs
   */
  void _decompress(const Buffer & inData, Buffer & outData) const;

  /**
   * Compressed/decompresses the data in the input buffer into the output buffer
   * using the LZMA compression library.
   *
   * @pre The stream member must have been previusly initialized.
   *
   * @tparam ET Exception type to launch on coding error. This must be either
   *            CompressionError or CompressionError
   *
   * @param inData Data to be compressed/decompressed
   * @param outData Buffer where the compressed/decompressed data will be stored
   *
   * @throws CompressionError If any compression algorithm specific error 
   *                          occurs (when it is called to compress)
   * @throws DecompressionError If any compression algorithm specific error 
   *                            occurs (when it is called to decompress)
   */
  template<typename ET>
  void code(const Buffer & inData, Buffer & outData) const;

}; // class LZMACompressor

} // namespace autocomp

#endif // AC_LZMA_COMPRESSOR_HPP
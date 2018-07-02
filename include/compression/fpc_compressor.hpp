/**
 *  AutoComp FPC Compressor
 *  fpc_compressor.hpp
 *
 *  This class implements the abstract class LeveledCompressor for
 *  compression/decompression using the FPC compression library.
 *
 *  @author Jhonathan Abreu
 *  @version 1.0 07/15/2018
 */

#ifndef AC_FPC_COMPRESSOR_HPP
#define AC_FPC_COMPRESSOR_HPP

#include <string>
#include <cassert>
#include <cstring>

#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "messaging/compressor.pb.h"
#include "compression/leveled_compressor.hpp"

namespace autocomp {

/** 
 * FPC compressor class.
 *
 * Class for a compression strategy using the FPC library.
 */
class FPCCompressor : public LeveledCompressor
{
  /**
   * FPC compresses in blocks of size BLOCK_SIZE
   */
  const int BLOCK_SIZE;

  /**
   * A mask used by the FPC algorithm
   */
  const unsigned long long mask[8];

public:

  /**
   * FPCCompressor constructor 
   *
   * @param compressionLevel Compression level for the FPC algorithm.
   *
   * @throws InvalidCompressionLevelError When the compression level is < 1
   *                                      or > 28
   */
  FPCCompressor(const int & compressionLevel = 20);

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
   * Compresses the data in the input buffer into the output buffer using the
   * FPC compression algorithm.
   *
   * @note This code belongs to Martin Burtscher and was adapted for in-memory 
   *       compression
   *
   * @param inData Data to be compressed
   * @param outData Buffer where the compressed data will be stored
   *
   * @throws CompressionError If any compression algorithm specific error occurs
   */
  void _compress(const Buffer & inData, Buffer & outData) const;

  /**
   * Decompresses the data in the input buffer into the output buffer using the
   * FPC compression algorithm.
   *
   * @note This code belongs to Martin Burtscher and was adapted for in-memory 
   *       decompression
   *
   * @param inData Data to be decompressed
   * @param outData Buffer where the decompressed data will be stored
   *
   * @throws DecompressionError If any compression algorithm specific error 
   *                            occurs
   */
  void _decompress(const Buffer & inData, Buffer & outData) const;

}; // class FPCCompressor

} // namespace autocomp

#endif // AC_FPC_COMPRESSOR_HPP
/**
 *  AutoComp bzip2 Compressor
 *  bzip2_compressor.hpp
 *
 *  This class implements the abstract class LeveledCompressor for
 *  compression/decompression using the bzip2 compression library.
 *
 *  @author Jhonathan Abreu
 *  @version 1.0 07/13/2018
 */

#ifndef AC_BZIP2_COMPRESSOR_HPP
#define AC_BZIP2_COMPRESSOR_HPP

#include <string>

extern "C" {
  #include "bzlib.h"
}

#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "messaging/compressor.pb.h"
#include "compression/leveled_compressor.hpp"

namespace autocomp {

/** 
 * bzip2 compressor class.
 *
 * Class for a compression strategy using the bzip2 library.
 */
class Bzip2Compressor : public LeveledCompressor
{
public:

  /**
   * Bzip2Compressor constructor 
   *
   * @param compressionLevel Compression level for the bzip2 algorithm.
   *
   * @throws InvalidCompressionLevelError When the compression level is < 1
   *                                      or > 9
   */
  Bzip2Compressor(const int & compressionLevel = 9);

  /**
   * @copydoc autocomp::CompressionStrategy::compress()
   */
  void compress(const Buffer & inData, Buffer & outData) const;

  /**
   * @copydoc autocomp::CompressionStrategy::decompress()
   */
  void decompress(const Buffer & inData, Buffer & outData) const;

}; // class Bzip2Compressor

} // namespace autocomp

#endif // AC_BZIP2_COMPRESSOR_HPP
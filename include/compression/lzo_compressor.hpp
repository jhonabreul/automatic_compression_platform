/**
 *  AutoComp LZO Compressor
 *  lzo_compressor.hpp
 *
 *  This class implements the abstract class LeveledCompressor for
 *  compression/decompression using the LZO compression library.
 *
 *  @author Jhonathan Abreu
 *  @version 1.0 07/13/2018
 */

#ifndef AC_LZO_COMPRESSOR_HPP
#define AC_LZO_COMPRESSOR_HPP

#include <string>

extern "C" {
  #include "lzo/lzoconf.h"
  #include "lzo/lzo1x.h"
}

#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "messaging/compressor.pb.h"
#include "compression/leveled_compressor.hpp"

namespace autocomp {

/** 
 * LZO compressor class.
 *
 * Class for a compression strategy using the LZO library.
 */
class LZOCompressor : public LeveledCompressor
{
  /**
   * Work memory for the LZO algorithm
   */
  mutable lzo_align_t * workMemory;

  /**
   * Work memory size
   */
  const unsigned long workMemorySize;

public:

  /**
   * LZOCompressor constructor 
   *
   * @param compressionLevel Compression level for the LZO algorithm.
   *
   * @throws InvalidCompressionLevelError When the compression level is < 1
   *                                       or > 9
   * @throws std::bad_alloc If work memory can not be allocated
   */
  LZOCompressor(const int & compressionLevel = 3);

  /**
   * @copydoc autocomp::CompressionStrategy::compress()
   */
  void compress(const Buffer & inData, Buffer & outData) const;

  /**
   * @copydoc autocomp::CompressionStrategy::decompress()
   */
  void decompress(const Buffer & inData, Buffer & outData) const;

}; // class LZOCompressor

} // namespace autocomp

#endif // AC_LZO_COMPRESSOR_HPP
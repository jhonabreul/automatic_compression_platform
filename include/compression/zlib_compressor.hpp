/**
 *  AutoComp zlib Compressor
 *  zlib_compressor.hpp
 *
 *  This class implements the abstract class CompressionStrategy for
 *  compression/decompression using the ZLIB compression library.
 *
 *  @author Jhonathan Abreu
 *  @version 1.0 07/13/2018
 */

#ifndef AC_ZLIB_COMPRESSOR_HPP
#define AC_ZLIB_COMPRESSOR_HPP

#include <string>

extern "C" {
  #include "zlib.h"
}

#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "messaging/compressor.pb.h"
#include "compression/leveled_compressor.hpp"

namespace autocomp {

/** 
 * zlib compressor class.
 *
 * Class for a compression strategy using the zlib library.
 */
class ZlibCompressor : public LeveledCompressor
{
public:

  /**
   * ZlibCompressor constructor 
   *
   * @param compressionLevel Compression level for the Zlib algorithm.
   *
   * @throws InvalidCompressionLevelError When the compression level is < 0
   *                                      or > 9
   */
  ZlibCompressor(const int & compressionLevel = 6);

  /**
   * @copydoc autocomp::CompressionStrategy::compress()
   */
  void compress(const Buffer & inData, Buffer & outData) const;

  /**
   * @copydoc autocomp::CompressionStrategy::decompress()
   */
  void decompress(const Buffer & inData, Buffer & outData) const;

}; // class ZlibCompressor

} // namespace autocomp

#endif // AC_ZLIB_COMPRESSOR_HPP
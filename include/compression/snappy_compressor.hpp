/**
 *  AutoComp Snappy Compressor
 *  snappy_compressor.hpp
 *
 *  This class implements the abstract class CompressionStrategy for
 *  compression/decompression using Google's Snappy compression library.
 *
 *  @author Jhonathan Abreu
 *  @version 1.0 07/13/2018
 */

#ifndef AC_SNAPPY_COMPRESSOR_HPP
#define AC_SNAPPY_COMPRESSOR_HPP

#include <string>
#include "snappy.h"

#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "messaging/compressor.pb.h"
#include "compression/compression_strategy.hpp"

namespace autocomp {

/** 
 * Snappy compressor class.
 *
 * Class for a compression strategy using Google's Snappy library.
 */
class SnappyCompressor : public CompressionStrategy
{
public:

  /**
   * SnappyCompressor constructor
   */
  SnappyCompressor();

  /**
   * @copydoc autocomp::CompressionStrategy::compress()
   */
  void compress(const Buffer & inData, Buffer & outData) const;

  /**
   * @copydoc autocomp::CompressionStrategy::decompress()
   */
  void decompress(const Buffer & inData, Buffer & outData) const;

}; // class SnappyCompressor

} // namespace autocomp

#endif // AC_SNAPPY_COMPRESSOR_HPP
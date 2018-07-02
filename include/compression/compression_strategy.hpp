/**
 *  Compression Strategy Interface
 *  compression_strategy.hpp
 *
 *  Declaration of the compression strategy interface.
 *
 *  @author Jhonathan Abreu
 *  @version 1.0 07/12/2018
 */

#ifndef AC_COMPRESSION_STRATEGY_INTERFACE_HPP
#define AC_COMPRESSION_STRATEGY_INTERFACE_HPP

#include <string>

#include "utils/buffer.hpp"

namespace autocomp {

/**
 * Abstract class for compression strategies.
 */
class CompressionStrategy
{
protected:
  
  /**
   * Compressor's name (intended only for loggin)
   */
  const std::string compressorName;

  /**
   * CompressionStrategy constructor
   *
   * @param compressorName Compressor's name (intended only for loggin)
   */
  CompressionStrategy(const std::string & compressorName)
    : compressorName(compressorName) {}

public:

  /**
   * Gets the compressor name
   *
   * @returns The compressor name
   */
  std::string getCompressorName() const
  {
    return this->compressorName;
  }

  /**
   * Compression method
   *
   * Compresses the data in the input buffer into the output buffer.
   *
   * @param inData Data to be compressed
   * @param outData Buffer where the compressed data will be stored
   *
   * @throws CompressionError If any compression algorithm specific error occurs
   */
  virtual void compress(const Buffer & inData, Buffer & outData) const = 0;

  /**
   * Decompression method
   *
   * Decompresses the data in the input buffer into the output buffer.
   *
   * @param inData Data to be decompressed
   * @param outData Buffer where the decompressed data will be stored
   *
   * @throws DecompressionError If any compression algorithm specific error 
   *                            occurs
   */
  virtual void decompress(const Buffer & inData, Buffer & outData) const = 0;

}; // class CompressionStrategy

} // namespace autocomp

#endif // AC_COMPRESSION_STRATEGY_INTERFACE_HPP
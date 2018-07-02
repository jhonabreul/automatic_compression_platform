/**
 *  Leveled Compressor
 *  leveled_compressor.hpp
 *
 *  Declaration of the interface for leveled compressors.
 *
 *  @author Jhonathan Abreu
 *  @version 1.0 07/13/2018
 */

#ifndef AC_LEVELED_COMPRESSOR_HPP
#define AC_LEVELED_COMPRESSOR_HPP

#include <string>

#include "compression_strategy.hpp"

namespace autocomp {

/** 
 * Abstract class for compression strategies (algorithms) with compression
 * levels.
 */
class LeveledCompressor : public CompressionStrategy
{
protected:

  /** 
   * Compression level to use when compress() method is called
   */
  int compressionLevel;

  const int minCompressionLevel;

  const int maxCompressionLevel;

  const int defaultCompressionLevel;

  /**
   * LeveledCompressor constructor
   *
   * @param compressorName Compressor's name (intended only for loggin)
   */
  LeveledCompressor(const std::string & compressorName,
                    const int & minCompressionLevel,
                    const int & maxCompressionLevel,
                    const int & defaultCompressionLevel)
    : CompressionStrategy(compressorName),
      minCompressionLevel(minCompressionLevel),
      maxCompressionLevel(maxCompressionLevel),
      defaultCompressionLevel(defaultCompressionLevel)
  {}

public:

  /**
   * Gets the current compression level for the compressor
   *
   * @return the current compression level for the compressor
   */
  int getCompressionLevel() const
  {
    return this->compressionLevel;
  }

  int getMinCompressionLevel() const
  {
    return this->minCompressionLevel;
  }

  int getMaxCompressionLevel() const
  {
    return this->maxCompressionLevel;
  }

  int getDefaultCompressionLevel() const
  {
    return this->defaultCompressionLevel;
  }

  void setDefaultCompressionLevel()
  {
    this->compressionLevel = this->defaultCompressionLevel;
  }

  /**
   * Sets the compression level for the compressor
   *
   * @param compressionLevel Compression level for the algorithm
   *
   * @throws InvalidCompressionLevelError When the given level is out of the
   *                                      level domainfor the compressor
   */
  void setCompressionLevel(const int & compressionLevel) {
    if (compressionLevel < this->minCompressionLevel or
        compressionLevel > this->maxCompressionLevel) {
      throw exceptions::InvalidCompressionLevelError(this->compressorName,
                                                     compressionLevel);
    }

    this->compressionLevel = compressionLevel;
  }

}; // class LeveledCompressor : public CompressionStrategy

} // namespace autocomp

#endif // AC_LEVELED_COMPRESSOR_HPP
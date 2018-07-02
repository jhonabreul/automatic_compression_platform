/**
 *  AutoComp Single Compressor
 *  sigle_compressor.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/30/2018
 */

#ifndef AC_SINGLE_COMPRESSOR_HPP
#define AC_SINGLE_COMPRESSOR_HPP

#include <string>
#include <map>
#include <memory>
#include <utility>
#include <tuple>
#include <chrono>

#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "messaging/compressor.pb.h"
#include "compression/automatic_compression_strategy.hpp"
#include "compression/zlib_compressor.hpp"
#include "compression/snappy_compressor.hpp"
#include "compression/lzo_compressor.hpp"
#include "compression/bzip2_compressor.hpp"
#include "compression/lzma_compressor.hpp"
#include "compression/fpc_compressor.hpp"

namespace autocomp {

/** 
 * This "Automatic" compression strategy is intended por compressing using
 * a particular compression algorithm
 */
class SingleCompressor : public AutomaticCompressionStrategy
{

protected:

  /**
   * Alias for a pointer to a CompressionStrategy.
   *
   * An object of this type can point to an object of any class that implements
   * CompressionStrategy.
   */
  using CompressorPointer = std::shared_ptr<CompressionStrategy>;

  /**
   * Alias for the container used to store the compressor objects.
   */
  using CompressorsContainer = std::map<Compressor, CompressorPointer>;

  /**
   * The compressors container
   */
  mutable CompressorsContainer compressors;

  /**
   * Current compressor to use
   */
  Compressor currentCompressor;

  int currentCompressionLevel;

public:

  /**
   * SingleCompressor constructor
   *
   * @ŧhrows std::bad_alloc On a memory allocation failure.
   */
  SingleCompressor(std::shared_ptr<io::PerformanceDataWriter> &
                      performanceDataWriter);

  Compressor getCompressor() const;

  int getCompressionLevel() const;

  void setCompressor(const Compressor & compressor,
                     const int & compressionLevel = -1);

  /**
   * @copydoc autocomp::CompressionStrategy::compress()
   */
  Compressor compress(const Buffer & inData, Buffer & outData) const;

}; // class SingleCompressor

} // namespace autocomp

#endif // AC_SINGLE_COMPRESSOR_HPP
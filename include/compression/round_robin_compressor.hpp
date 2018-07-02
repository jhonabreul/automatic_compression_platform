/**
 *  AutoComp Round Robin Compressor
 *  round_robin_compressor.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/15/2018
 */

#ifndef AC_ROUND_ROBIN_COMPRESSOR_HPP
#define AC_ROUND_ROBIN_COMPRESSOR_HPP

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
 * Round Robin compressor class.
 *
 * Implementation of an automatic compressor that compressed data in a round
 * robin fashion. That is, each call to the compress() method will compress
 * the data using a different compression algorithm, or the same one used in 
 * the last call but with its next available level (if applicable).
 */
class RoundRobinCompressor : public AutomaticCompressionStrategy
{
  /**
   * Alias for a pointer to a CompressionStrategy.
   *
   * An object of this type can point to an object of any class that implements
   * CompressionStrategy.
   */
  using CompressorPointer = std::shared_ptr<CompressionStrategy>;

  /**
   * Type for a compressor with its corresponding level (if applcoable).
   */
  using CompressorType = std::pair<CompressorPointer, int>;

  /**
   * Alias for the container used to store the compressor objects.
   */
  using CompressorsContainer = std::multimap<Compressor, CompressorType>;

  /**
   * The compressors container
   */
  mutable CompressorsContainer compressors;

  /**
   * An iterator to the next compressor to be used in the next call to
   * the compress() method.
   */
  mutable CompressorsContainer::iterator currentCompressor;

public:

  /**
   * RoundRobinCompressor constructor
   *
   * @ŧhrows std::bad_alloc On a memory allocation failure.
   */
  RoundRobinCompressor(std::shared_ptr<io::PerformanceDataWriter> &
                        performanceDataWriter);

  /**
   * @copydoc autocomp::CompressionStrategy::compress()
   */
  Compressor compress(const Buffer & inData, Buffer & outData) const;

}; // class RoundRobinCompressor

} // namespace autocomp

#endif // AC_ROUND_ROBIN_COMPRESSOR_HPP
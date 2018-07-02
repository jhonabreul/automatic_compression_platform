/**
 *  Automatic Compression Strategy Interface
 *  automatic_compression_strategy.hpp
 *
 *  Declaration of the automatic compression strategy interface.
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/15/2018
 */

#ifndef AC_AUTOMATIC_COMPRESSION_STRATEGY_INTERFACE_HPP
#define AC_AUTOMATIC_COMPRESSION_STRATEGY_INTERFACE_HPP

#include <string>

#include "utils/buffer.hpp"
#include "utils/data_structures.hpp"
#include "io/performance_data_writer.hpp"
#include "messaging/compressor.pb.h"


namespace autocomp {

/**
 * Abstract class for automatic compression strategies.
 */
class AutomaticCompressionStrategy
{
protected:
  
  /**
   * Writer for compression performance data
   */
  mutable std::shared_ptr<io::PerformanceDataWriter> performanceDataWriter;

public:

  AutomaticCompressionStrategy(
      const std::shared_ptr<io::PerformanceDataWriter> & performanceDataWriter =
        nullptr
    )
    : performanceDataWriter(performanceDataWriter)
  {}

  /**
   * Compression method
   *
   * Compresses the data in the input buffer into the output buffer.
   *
   * @param inData Data to be compressed
   * @param outData Buffer where the compressed data will be stored
   *
   * @returns The compressor for the compression task
   *
   * @throws CompressionError If any compression algorithm specific error occurs
   */
  virtual Compressor compress(const Buffer & inData,
                              Buffer & outData) const = 0;

}; // class AutomaticCompressionStrategy

} // namespace autocomp

#endif // AC_AUTOMATIC_COMPRESSION_STRATEGY_INTERFACE_HPP
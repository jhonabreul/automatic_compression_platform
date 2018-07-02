/**
 *  AutoComp Copy Compressor
 *  copy_compressor.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/30/2018
 */

#ifndef AC_COPY_COMPRESSOR_HPP
#define AC_COPY_COMPRESSOR_HPP

#include <string>
#include <map>
#include <memory>
#include <utility>
#include <tuple>

#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "messaging/compressor.pb.h"
#include "compression/automatic_compression_strategy.hpp"

namespace autocomp
{

/**
 * Class intented to only copy and do no compression at all
 */
class Copy : public AutomaticCompressionStrategy
{
public:

  Copy(const std::shared_ptr<io::PerformanceDataWriter> &
          performanceDataWriter = nullptr)
    : AutomaticCompressionStrategy(performanceDataWriter)
  {}

  /**
   * @copydoc autocomp::CompressionStrategy::compress()
   */
  Compressor compress(const Buffer & inData, Buffer & outData) const
  {
    return COPY;
  }

}; // class Copy

} // namespace autocomp

#endif // AC_COPY_COMPRESSOR_HPP
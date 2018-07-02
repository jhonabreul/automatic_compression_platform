/**
 *  AutoComp Data Structures
 *  data_structures.hpp
 *
 *  Definition of common data structures for AutoComp system.
 *
 *  @author Jhonathan Abreu
 *  @version 1.0 07/12/2018
*/

#ifndef AC_DATA_STRUCTURES_HPP
#define AC_DATA_STRUCTURES_HPP

#include <mutex>
#include <atomic>

namespace autocomp {

/**
 * Compression/Decompression performance data.
 */
struct CompressionPerformanceData
{
  std::string compressor; /**< Compressor used for compression/
                               decompression. */
  short compressionLevel; /**< Compression level used for compression. */
  long elapsedTime;       /**< Compression/decompression time. */ 
  size_t originalSize;    /**< Size before compression/decompression. */
  size_t finalSize;       /**< Size after compression/decompression. */
};

struct ResourceState
{
  std::atomic<float> cpuLoad;
  std::atomic<float> bandwidth;

  ResourceState()
    : cpuLoad(0), bandwidth(0)
  {}
};

} // namespace autocomp

#endif // AC_DATA_STRUCTURES_HPP
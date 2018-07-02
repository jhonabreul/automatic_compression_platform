/**
 *  AutoComp "Compression" Strategy for retrieving training data
 *  training_compressor.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 09/04/2018
 */

#ifndef AC_TRAINING_COMPRESSOR_HPP
#define AC_TRAINING_COMPRESSOR_HPP

#include <string>
#include <map>
#include <memory>
#include <utility>
#include <chrono>
#include <algorithm>
#include <limits>
#include <cmath>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <csignal>

#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "utils/data_structures.hpp"
#include "utils/functions.hpp"
#include "utils/synchronous_queue.hpp"
#include "network/socket/tcp_socket.hpp"
#include "messaging/compressor.pb.h"
#include "compression/single_compressor.hpp"
#include "compression/zlib_compressor.hpp"
#include "compression/snappy_compressor.hpp"
#include "compression/lzo_compressor.hpp"
#include "compression/bzip2_compressor.hpp"
#include "compression/lzma_compressor.hpp"
#include "compression/fpc_compressor.hpp"

namespace autocomp {

/** 
 * Traning Compressor class.
 *
 * This compressor is intended for retrieving traning data for a classifier
 */
class TrainingCompressor : public SingleCompressor
{
  /**
   * Resources state variable
   */
  const ResourceState * resourceState;

  const SynchronousQueue<Buffer> * transmissionQueue;

  const std::shared_ptr<net::TCPSocket> clientSocket;

  const int clientSocketSendBufferCapacity;

  ::pid_t cpuModulatorPID;

public:

  /**
   * TrainingCompressor constructor
   *
   * @ŧhrows std::bad_alloc On a memory allocation failure.
   */
  TrainingCompressor(const ResourceState * resourceState,
                     const SynchronousQueue<Buffer> * transmissionQueue,
                     const std::shared_ptr<net::TCPSocket> & clientSocket,
                     std::shared_ptr<io::PerformanceDataWriter> &
                        performanceDataWriter);

  ~TrainingCompressor();

  /**
   * @copydoc autocomp::CompressionStrategy::compress()
   */
  Compressor compress(const Buffer & inData, Buffer & outData) const;

private:

  float getClientSocketSendBufferLoad() const;

  float getEfectiveTransmissionRate(const float & availableBandwidth,
                                    const float & compressionRate,
                                    const float & compressionRatio) const;

  int getBytecounting(const Buffer & inData) const;

  ::pid_t launchCPUModulator();

}; // class TrainingCompressor

} // namespace autocomp

#endif // AC_TRAINING_COMPRESSOR_HPP
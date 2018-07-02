/**
 *  AutoComp Client
 *  client.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/29/2018
 */

#ifndef AC_CLIENT_HPP
#define AC_CLIENT_HPP

#include <thread>
#include <string>
#include <cstring>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <fstream>
#include <map>
#include <libgen.h> // basename
#include <cstdio> // remove

#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
#include <g3log/loglevels.hpp>
#include "google/protobuf/message.h"

#include "utils/exceptions.hpp"
#include "utils/constants.hpp"
#include "utils/buffer.hpp"
#include "utils/synchronous_queue.hpp"
#include "utils/thread_pool.hpp"
#include "utils/protobuf_utils.hpp"
#include "messaging/compressor.pb.h"
#include "messaging/file_request_mode.pb.h"
#include "messaging/error_message.pb.h"
#include "messaging/chunk_header.pb.h"
#include "messaging/file_initial_message.pb.h"
#include "messaging/file_transmission_request.pb.h"
#include "network/socket/tcp_socket.hpp"
#include "compression/zlib_compressor.hpp"
#include "compression/snappy_compressor.hpp"
#include "compression/lzo_compressor.hpp"
#include "compression/bzip2_compressor.hpp"
#include "compression/lzma_compressor.hpp"
#include "compression/fpc_compressor.hpp"
#include "compression/pre_compressing_file_processor.hpp"

namespace autocomp
{
  namespace net
  {

  /**
   * AutoComp Client template
   */
  class Client
  {

    struct DecompressionQueueEntry
    {
      messaging::FileInitialMessage fileInitialMessage;
      messaging::ChunkHeader chunkHeader;
      Buffer chunk;
    };

    std::thread decompressionThread;
    SynchronousQueue<DecompressionQueueEntry> decompressionQueue;
    std::mutex mutex;
    std::condition_variable condition;
    bool doneReceiving;
    TCPSocket socket;
    const std::string serverHostname;
    const unsigned short serverPort;

    std::string requestedPath = "";
    std::string destinationDirectory = "";
    bool preCompression;
    Compressor preCompressingCompressor;

    // Compressors
    std::map<Compressor, std::unique_ptr<CompressionStrategy>> compressors;

    // Logging
    std::unique_ptr<g3::LogWorker> logWorker;
    const LEVELS ERROR {g3::kWarningValue + 1, {"ERROR"}};

    ::pid_t bandwidthModulatorPID;
    
  public:
    
    Client(const std::string & serverHostname,
           const unsigned short & serverPort);

    Client(const Client &) = delete;
    Client(Client &&) = delete;
    Client & operator=(const Client &) = delete;
    Client & operator=(Client &&) = delete;

    ~Client();

    void init();

    void requestFile(const std::string & path, const FileRequestMode & mode,
                     const Compressor * compressor,
                     const int * compressionLevel,
                     const std::string & destinationDirectory);

    void shutdown();

  private:

    void decompress(); // This should be decompress

    messaging::FileTransmissionRequest
    configureFileRequestMessage(const std::string & path, 
                                const FileRequestMode & mode,
                                const Compressor * compressor,
                                const int * compressionLevel);

    void initLogger();

    static std::string logFormatter(const g3::LogMessage & logMessage);

    ::pid_t launchBandwidthModulator();

  }; // class Client

  } // namespace net
} // namespace autocomp

#endif // AC_CLIENT_HPP
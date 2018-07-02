/**
 *  AutoComp Server
 *  server.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/28/2018
 */

#ifndef AC_SERVER_HPP
#define AC_SERVER_HPP

#include <thread>
#include <string>
#include <memory>
#include <vector>
#include <sys/poll.h>
#include <sys/types.h>  // mkfifo
#include <sys/stat.h>   // mkfifo
#include <fcntl.h>      // open
#include <chrono>
#include <atomic>

#include "g3log/g3log.hpp"
#include "g3log/logworker.hpp"
#include "g3log/loglevels.hpp"

#include "utils/exceptions.hpp"
#include "utils/constants.hpp"
#include "utils/buffer.hpp"
#include "utils/thread_pool.hpp"
#include "utils/synchronous_queue.hpp"
#include "utils/protobuf_utils.hpp"
#include "utils/decision_tree.hpp"
#include "network/socket/tcp_socket.hpp"
#include "messaging/compressor.pb.h"
#include "messaging/error_message.pb.h"
#include "messaging/chunk_header.pb.h"
#include "messaging/file_initial_message.pb.h"
#include "messaging/file_transmission_request.pb.h"
#include "compression/automatic_compression_strategy.hpp"
#include "compression/round_robin_compressor.hpp"
#include "compression/single_compressor.hpp"
#include "compression/copy_compressor.hpp"
#include "compression/training_compressor.hpp"
#include "compression/autocomp_compressor.hpp"
#include "compression/file_processing_strategy.hpp"
#include "compression/file_processor.hpp"
#include "compression/pre_compressing_file_processor.hpp"
#include "monitors/monitors.hpp" // monitorCPU

namespace autocomp
{
  namespace net
  {

  /**
   * AutoComp Server template
   */
  class Server
  {
    ThreadPool requestThreadPool;
    ThreadPool transmissionThreadPool;
    std::thread cpuMonitorThread;
    TCPSocket serverSocket;

    std::atomic<bool> doneServing;
    const std::string shutdownPipeName;
    int shutdownPipeFileDescriptor;
    
    enum { nInputFileDescriptors = 2 };
    pollfd pollFileDescriptors[nInputFileDescriptors];
    const int infiniteTime = -1;

    ResourceState resourceState;

    // Logging
    std::unique_ptr<g3::LogWorker> logWorker;
    static const LEVELS ERROR;

    std::shared_ptr<io::PerformanceDataWriter> performanceDataWriter;

    DecisionTree decisionTree;

  public:
    
    Server(const unsigned short & port,
           const unsigned int & nThreads = std::thread::hardware_concurrency(),
           const std::string & shutdownPipeName =
              autocomp::constants::SHUTDOWN_PIPE_NAME);

    Server(const unsigned short & port, const std::string & shutdownPipeName,
           const unsigned int & nThreads = std::thread::hardware_concurrency());

    Server(const Server &) = delete;
    Server(Server &&) = delete;
    Server & operator=(const Server &) = delete;
    Server & operator=(Server &&) = delete;

    ~Server();

    unsigned short getPort() const;

    void init();

    void serve();

    void shutdown();

  private:

    static void processRequest(std::shared_ptr<TCPSocket> clientSocket,
                               ThreadPool & transmissionThreadPool,
                               std::shared_ptr<io::PerformanceDataWriter>
                                  performanceDataWriter,
                               ResourceState & resourceState,
                               const DecisionTree & decisionTree);

    static void transmit(std::shared_ptr<TCPSocket> clientSocket,
                         SynchronousQueue<Buffer> & transmissionQueue,
                         bool & requestDone, std::mutex & mutex,
                         std::condition_variable & condition,
                         ResourceState & resourceState);

    static std::shared_ptr<FileProcessingStrategy> configureFileProcessor(
        const messaging::FileTransmissionRequest & fileRequest,
        const ResourceState & resourceState,
        const SynchronousQueue<Buffer> & transmissionQueue,
        const std::shared_ptr<TCPSocket> & clientSocket,
        std::shared_ptr<io::PerformanceDataWriter> performanceDataWriter,
        const DecisionTree & decisionTree
      );

    void initLogger();

    static std::string logFormatter(const g3::LogMessage & logMessage);

  }; // class Server

  } // namespace net
} // namespace autocomp

#endif // AC_SERVER_HPP
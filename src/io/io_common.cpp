/**
 *  AutoComp Common I/O Helper functions and classes.
 *  io_common.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *	@date 07/17/2018
*/

#include "io/io_common.hpp"

namespace autocomp
{
  namespace io
  {

  // Initializes the logger.
  void initLogger(std::unique_ptr<g3::LogWorker> & worker)
  {
    auto handle= worker->addDefaultLogger(constants::LOG_FILE_PREFIX,
                                          constants::LOG_DIR,
                                          constants::LOG_FILE_ID);
    g3::initializeLogging(worker.get());

    auto changeFormatting = handle->call(&g3::FileSink::overrideLogDetails,
                                         &formatLogMessage);
    const std::string newHeader("");
    auto changeHeader = handle->call(&g3::FileSink::overrideLogHeader,
                                     newHeader);

    changeFormatting.wait();
    changeHeader.wait();
  }

  } //namespace io
} //namespace autocomp
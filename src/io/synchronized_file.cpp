/**
 *  AutoComp Synchronized File
 *  synchronized_file.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/17/2018
 */

#include "io/synchronized_file.hpp"

namespace autocomp
{
  namespace io
  {

  // SynchronizedFile constructor
  SynchronizedFile::SynchronizedFile()
    : out(nullptr)
  {
    std::time_t currentTime = std::time(nullptr);
    char timestamp[80];

    std::strftime(timestamp, 80, "%Y%m%d-%H%M%S", std::localtime(&currentTime));

    this->filename.append(constants::LOG_DIR)
                  .append("/")
                  .append(constants::PERFORMANCE_LOG_FILE_NAME_PREFIX)
                  .append(timestamp)
                  .append(constants::CSV_LOG_FILE_EXTENSSION);

    this->outBuffer = std::unique_ptr<AsynchronousBuffer>(
                        new AsynchronousBuffer(this->filename)
                      );
    this->out.rdbuf(this->outBuffer.get());
  }

  SynchronizedFile::~SynchronizedFile()
  {
    this->out.flush();
  }

  // Writes data to the file
  void SynchronizedFile::write(const std::string & data)
  {
    std::unique_lock<std::mutex> lock(this->writingMutex);

    this->out << data << '\n' << std::flush;
  }

  } // namespace io
} // namespace autocomp
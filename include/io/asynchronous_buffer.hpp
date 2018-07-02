/**
 *  AutoComp Asynchronous Buffer
 *  asynchronous_buffer.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/17/2018
 */

#ifndef AC_ASYNCHRONOUS_BUFFER_HPP
#define AC_ASYNCHRONOUS_BUFFER_HPP

#include <string>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <queue>
#include <streambuf>
#include <thread>
#include <vector>

#include "utils/constants.hpp"
#include "utils/exceptions.hpp"

namespace autocomp
{
  namespace io
  {

  /**
   * Asynchronous buffer for asynchronous file I/O
   */
  class AsynchronousBuffer : public std::streambuf
  {
    std::ofstream out;
    std::mutex mutex;
    std::condition_variable condition;
    std::queue<std::vector<char>> queue;
    std::vector<char> buffer;
    bool done;
    std::thread thread;
    const int maxBufferSize;

    void worker();

  public:

    /**
     * @throws exceptions::IOError When an error occurs when opening the file
     */
    AsynchronousBuffer(const std::string & filename);

    ~AsynchronousBuffer();

    int overflow(int c);

    int sync();

  }; // class AsynchronousBuffer

  } // namespace io
} // namespace autocomp

#endif // AC_ASYNCHRONOUS_BUFFER_HPP
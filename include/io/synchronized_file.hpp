/**
 *  AutoComp Synchronized File
 *  synchronized_file.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/17/2018
 */

#ifndef AC_SYNCHRONIZED_FILE_HPP
#define AC_SYNCHRONIZED_FILE_HPP

#include <string>
#include <fstream>
#include <streambuf>
#include <memory>
#include <ctime>
#include <mutex>

#include "utils/constants.hpp"
#include "utils/exceptions.hpp"
#include "io/asynchronous_buffer.hpp"

namespace autocomp
{
  namespace io
  {

  /**
   * A synchronized file for multithreaded performance date writting
   */
  class SynchronizedFile
  {
    /** 
     * Out stream for writting to disk
     */
    std::ostream out;

    /** 
     * 
     */
    std::unique_ptr<AsynchronousBuffer> outBuffer;

    /** 
     * File name
     */
    std::string filename;

    /** 
     * Mutex for writting
     */
    std::mutex writingMutex;

  public:

    /**
     * SynchronizedFile constructor 
     *
     * @throws exceptions::IOError When an error occurs when opening the file
     */
    SynchronizedFile();

    ~SynchronizedFile();
  
    /**
     * Writes data to the file
     *
     * @param data Data to be written to the file
     *
     * @throws exceptions::IOError If the out stream is closed.
     */
    void write(const std::string & data);

    SynchronizedFile(const SynchronizedFile &) = delete;
    SynchronizedFile & operator=(const SynchronizedFile &) = delete;

  }; // class SynchronizedFile

  } // namespace io
} // namespace autocomp

#endif // AC_SYNCHRONIZED_FILE_HPP
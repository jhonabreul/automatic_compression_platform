/**
 *  AutoComp Asynchronous Buffer
 *  asynchronous_buffer.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/17/2018
 */

#include "io/asynchronous_buffer.hpp"

namespace autocomp
{
  namespace io
  {

  AsynchronousBuffer::AsynchronousBuffer(const std::string & filename)
    : maxBufferSize(5 * 1024),
      out(filename),
      buffer(5 * 1024),
      done(false)
  {
    if(this->out.fail()) {          
      std::string errorMessage("Error creating performance log file (");
      errorMessage.append(filename)
                  .append(")");

      std::cout << "ERROOOOOOOOOR: " << errorMessage << std::endl;

      throw exceptions::IOError(errorMessage);
    }

    this->setp(this->buffer.data(),
               this->buffer.data() + this->buffer.size() - 1);

    try {
      this->thread = std::thread(&AsynchronousBuffer::worker, this);
    }
    catch (std::system_error & error) {          
      std::string errorMessage("Error lauching asynchronous buffer thread: ");
      errorMessage.append(error.what());

      throw exceptions::IOError(errorMessage);
    }
  }

  AsynchronousBuffer::~AsynchronousBuffer()
  {
    {
      std::unique_lock<std::mutex> guard(this->mutex);
      this->done = true;
    }
    this->condition.notify_one();
    if (this->thread.joinable()) {
      this->thread.join();
    }
    this->out.close();
  }

  int AsynchronousBuffer::overflow(int c)
  {
    if (c != std::char_traits<char>::eof()) {
      *this->pptr() = std::char_traits<char>::to_char_type(c);
      this->pbump(1);
    }
  
    return this->sync() != -1
            ? std::char_traits<char>::not_eof(c)
            : std::char_traits<char>::eof();
  }

  int AsynchronousBuffer::sync()
  {    
    if (this->pbase() != this->pptr()) {
      this->buffer.resize(std::size_t(this->pptr() - this->pbase()));
      {
        std::unique_lock<std::mutex> guard(this->mutex);
        this->queue.push(std::move(this->buffer));
      }
      this->condition.notify_one();
      this->buffer = std::vector<char>(this->maxBufferSize);
      this->setp(this->buffer.data(),
                 this->buffer.data() + this->buffer.size() - 1);
    }
    
    return 0;
  }

  void AsynchronousBuffer::worker()
  {
    bool endWorker(false);
    std::vector<char> tmpBuffer;

    while (not endWorker) {
      {
        std::unique_lock<std::mutex> guard(this->mutex);
        this->condition.wait(guard,
                             [this]()
                              { 
                                return not this->queue.empty() or this->done;
                              });

        if (not this->queue.empty()) {
          tmpBuffer.swap(this->queue.front());
          this->queue.pop();
        }
        
        endWorker = this->queue.empty() and this->done;
      }
      
      if (not tmpBuffer.empty()) {
        this->out.write(tmpBuffer.data(), std::streamsize(tmpBuffer.size()));
        tmpBuffer.clear();
      }
    }
    
    this->out.flush();
  }

  } // namespace io
} // namespace autocomp
/**
 *  AutoComp Exceptions
 *  buffer.hpp
 *
 *  Declaration of class Buffer, used to store input and output buffers for
 *  compression/decompression.
 *
 *  @author Jhonathan Abreu
 *  @version 1.0 07/12/2018
 */

#ifndef AC_EXCEPTIONS_HPP
#define AC_EXCEPTIONS_HPP

#include <string>
#include <stdexcept>

#include "messaging/compressor.pb.h"

namespace autocomp
{
  namespace exceptions
  {

  /**
   * Exception for any compression error
   */
  class CompressionError : public std::runtime_error
  {
    std::string compressor;
    size_t inputSize;
    size_t outputCapacity;

    mutable std::string errorMessage;

  public:

    CompressionError(const std::string & compressor,
                     const size_t & inputSize,
                     const size_t & outputCapacity,
                     const std::string & message)
      : std::runtime_error(message),
        compressor(compressor),
        inputSize(inputSize),
        outputCapacity(outputCapacity)
    {}

    const char * what() const throw ()
    {
      errorMessage = "Compression error (";

      errorMessage.append("compressor: ")
                  .append(this->compressor)
                  .append(", input buffer size: ")
                  .append(std::to_string(this->inputSize))
                  .append(", output buffer capacity: ")
                  .append(std::to_string(this->outputCapacity))
                  .append("): ")
                  .append(std::runtime_error::what());

      return errorMessage.c_str();
    }

    void setCompressorName(const std::string & compressorName)
    {
      this->compressor = compressorName;
    }

    void setBufferInputSize(const size_t & inputSize)
    {
      this->inputSize = inputSize;
    }

    void setBufferOutputCapacity(const size_t & outputCapacity)
    {
      this->outputCapacity = outputCapacity;
    }

    std::string getCompressorName()
    {
      return this->compressor;
    }

    size_t getBufferInputSize()
    {
      return this->inputSize;
    }

    size_t getBufferOutputCapacity()
    {
      return this->outputCapacity;
    }
  }; // class CompressionError

  /**
   * Exception for any decompression error
   */
  class DecompressionError : public std::runtime_error
  {
    std::string compressor;
    size_t inputSize;
    size_t outputCapacity;

    mutable std::string errorMessage;

  public:

    DecompressionError(const std::string & compressor,
                       const size_t & inputSize,
                       const size_t & outputCapacity,
                       const std::string & message)
      : std::runtime_error(message),
        compressor(compressor),
        inputSize(inputSize),
        outputCapacity(outputCapacity)
    {}

    const char * what() const throw ()
    {
      errorMessage = "Decompression error (";

      errorMessage.append("compressor: ")
                  .append(this->compressor)
                  .append(", input buffer size: ")
                  .append(std::to_string(this->inputSize))
                  .append(", output buffer capacity: ")
                  .append(std::to_string(this->outputCapacity))
                  .append("): ")
                  .append(std::runtime_error::what());

      return errorMessage.c_str();
    }

    void setCompressorName(const std::string & compressorName)
    {
      this->compressor = compressorName;
    }

    void setBufferInputSize(const size_t & inputSize)
    {
      this->inputSize = inputSize;
    }

    void setBufferOutputCapacity(const size_t & outputCapacity)
    {
      this->outputCapacity = outputCapacity;
    }

    std::string getCompressorName()
    {
      return this->compressor;
    }

    size_t getBufferInputSize()
    {
      return this->inputSize;
    }

    size_t getBufferOutputCapacity()
    {
      return this->outputCapacity;
    }
  }; // class DecompressionError

  /**
   * Exception for any decompression error
   */
  class InvalidCompressionLevelError : public std::domain_error
  {
    std::string compressor;
    int compressionLevel;

    mutable std::string errorMessage;

  public:

    InvalidCompressionLevelError(const std::string & compressor,
                                 const int & compressionLevel,
                                 const std::string & message = "")
      : std::domain_error(message),
        compressor(compressor),
        compressionLevel(compressionLevel)
    {}

    const char * what() const throw ()
    {
      errorMessage = "Invalid compression level (";

      errorMessage.append("compressor: ")
                  .append(this->compressor)
                  .append(", compression level: ")
                  .append(std::to_string(this->compressionLevel))
                  .append(") ")
                  .append(std::domain_error::what());

      return errorMessage.c_str();
    }
  }; // class InvalidCompressionLevelError

  /**
   * Exception for compressor enum error
   */
  class InvalidCompressorError : public std::domain_error
  {
    Compressor compressor;

    mutable std::string errorMessage;

  public:

    InvalidCompressorError(const Compressor & compressor,
                           const std::string & message = "")
      : std::domain_error(message),
        compressor(compressor)
    {}

    const char * what() const throw ()
    {
      errorMessage = "Invalid compressor (";

      errorMessage.append("compressor value: ")
                  .append(std::to_string(this->compressor))
                  .append("). ")
                  .append(std::domain_error::what());

      return errorMessage.c_str();
    }
  }; // class InvalidCompressorError

  /**
   * Exception for any I/O error
   */
  class IOError : public std::runtime_error
  {
  public:

    IOError(const std::string & message)
      : std::runtime_error(message)
    {}

    const char * what() const throw ()
    {
      return std::runtime_error::what();
    }
  }; // class IOError

  /**
   * Exception for any network error
   */
  class NetworkError : public std::runtime_error
  {
  public:

    NetworkError(const std::string & message)
      : std::runtime_error(message)
    {}

    const char * what() const throw ()
    {
      return std::runtime_error::what();
    }
  }; // class NetworkError

  /**
   * Exception for any network error
   */
  class InvalidRequestParameterError : public std::runtime_error
  {
  public:

    InvalidRequestParameterError(const std::string & message)
      : std::runtime_error(message)
    {}

    const char * what() const throw ()
    {
      return std::runtime_error::what();
    }
  }; // class InvalidRequestParameterError

  } // namespace exceptions
} // namespace autocomp

#endif // AC_EXCEPTIONS_HPP
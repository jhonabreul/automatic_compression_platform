/**
 *  AutoComp Buffer
 *  buffer.hpp
 *
 *  Declaration of class Buffer, used to store input and output buffers for
 *  compression/decompression.
 *
 *  @author Jhonathan Abreu
 *  @version 1.0 07/12/2018
 */

#ifndef AC_BUFFER_HPP
#define AC_BUFFER_HPP

#include <string>
#include <cstddef>
#include <stdexcept>
#include <algorithm>
#include <vector>

namespace autocomp {

/**
 * Describes a Buffer abstraction to store input and output buffers for
 * compression/decompression.
 */
class Buffer
{
  /** 
   * The buffer itself.
   *
   * Stores the data begin compressed/decompressed.
   */
  std::vector<char> data;

  /** 
   * Buffer's maximum capacity
   */
  std::size_t capacity;

  /** 
   * Buffer's current size.
   *
   * Total size of the actual data stored in the buffer. It must be <= capacity
   */
  std::size_t actualSize;

public:

  /**
   * Buffer constructor 
   *
   * @param capacity The maximum capacity in bytes of the buffer.
   *
   * @throws std::bad_alloc When the internal container can not be initialized
   */
  Buffer(const std::size_t & capacity);

  Buffer();

  Buffer(Buffer && other);

  Buffer & operator=(Buffer && other);

  Buffer(const Buffer &) = delete;
  Buffer & operator=(const Buffer &) = delete;

  ~Buffer();

  /**
   * Gets a pointer to the internal container of the buffer
   *
   * @returns char * pointing to the first element of the buffer
   */
  char * getData();

  /**
   * Gets a pointer to the internal container of the buffer
   *
   * @returns char * pointing to the first element of the buffer
   */
  const char * getData() const;

  const std::vector<char> & getContainer() const;

  std::vector<char> & getContainer();

  /**
   * Sets the buffer's data
   *
   * @param data Buffer's data
   * @param size Buffer's data size
   *
   * @throws std::domain_error If the size of the given data is greater than
   *                           the buffer's capacity
   */
  void setData(const char * data, const int & size);

  /**
   * Sets the buffer's data
   *
   * @param data Buffer's data
   *
   * @throws std::domain_error If the size of the given data is greater than
   *                           the buffer's capacity
   */
  void setData(const std::string & data);

  void setData(const std::vector<char> & data);

  void setData(std::vector<char> && data);

  /**
   * Gets the buffer's capacity
   *
   * @returns The buffer's capacity
   */
  std::size_t getCapacity() const;

  /**
   * Gets the buffer's actual size
   *
   * @return The buffer's actual size
   */
  std::size_t getSize() const;

  /**
   * Sets the buffer's actual size
   *
   * @param size Buffer's actual size
   *
   * @throws std::domain_error When the given size is greater or equal than the
   *                           buffer's capacity
   */
  void setSize(const std::size_t & size);

  /**
   * Assigns more memory to the buffer
   *
   * @param newCapacity Buffer's new capacity
   */
  void resize(const std::size_t & newCapacity);

  /**
   * Swaps the contents of the buffer
   *
   * @param buffer Swapping buffer
   */
  void swap(Buffer & buffer);

  operator bool() const
  {
    return this->capacity != 0;
  }

}; // class Buffer

} // namespace autocomp

#endif // AC_BUFFER_HPP
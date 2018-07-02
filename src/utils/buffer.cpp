/**
 *  AutoComp Buffer
 *  buffer.cpp
 *
 *  Definition of class Buffer methods, used to store input and output buffers
 *  for compression/decompression.
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/12/2018
 */

#include "utils/buffer.hpp"

namespace autocomp {

// Buffer constructor
Buffer::Buffer(const std::size_t & capacity)
  : data(capacity, 0),
    capacity(capacity),
    actualSize(0)
{}

Buffer::Buffer()
  : capacity(0),
    actualSize(0)
{}

Buffer::Buffer(Buffer && other)
  : capacity(0),
    actualSize(0)
{
  *this = std::move(other);
}

Buffer & Buffer::operator=(Buffer && other)
{
  if (this != &other) {
    this->swap(other);
  }

  return *this;
}

// Buffer destructor
Buffer::~Buffer()
{}

// Gets a pointer to the internal container of the buffer
char * Buffer::getData()
{
  return this->data.data();
}

// Gets a pointer to the internal container of the buffer
const char * Buffer::getData() const 
{
  return this->data.data();
}

const std::vector<char> & Buffer::getContainer() const
{
  return this->data;
}

std::vector<char> & Buffer::getContainer()
{
  return this->data;
}

// Sets the buffer's data
void Buffer::setData(const char * data, const int & size)
{
  if (size > this->capacity) {
    throw std::domain_error("Argument data size is greater than the buffer's "
                            "capacity");
  }

  std::copy_n(data, size, this->data.begin());
  //this->data[size] = '\0';
  this->setSize(size);
}

// Sets the buffer's data
void Buffer::setData(const std::string & data)
{
  if (data.size() > this->capacity) {
    throw std::domain_error("Argument data size is greater than the buffer's "
                            "capacity");
  }

  std::copy_n(data.begin(), data.size(), this->data.begin());
  //this->data[size] = '\0';
  this->setSize(data.size());
}

void Buffer::setData(const std::vector<char> & data)
{
  if (data.size() > this->capacity) {
    throw std::domain_error("Argument data size is greater than the buffer's "
                            "capacity");
  }

  std::copy_n(data.begin(), data.size(), this->data.begin());
  //this->data[size] = '\0';
  this->setSize(data.size());
}

void Buffer::setData(std::vector<char> && data)
{
  this->data.swap(data);
  this->capacity = this->data.capacity();
  this->actualSize = this->data.size();
}

// Gets the buffer's capacity
std::size_t Buffer::getCapacity() const
{
  return this->capacity;
}


// Gets the buffer's actual size
std::size_t Buffer::getSize() const
{
  return this->actualSize;
}

// Sets the buffer's actual size
void Buffer::setSize(const std::size_t & size)
{
  if (size > capacity) {
    throw std::domain_error("Size can not be greater than buffer's capacity");
  }

  this->actualSize = size;
}

// Assigns more memory to the buffer
void Buffer::resize(const std::size_t & newCapacity)
{
  if (newCapacity < this->actualSize) {
    return;
  }

  this->data.resize(newCapacity);
  this->capacity = newCapacity;
}

// Swaps the contents of the buffer
void Buffer::swap(Buffer & buffer)
{
  this->data.swap(buffer.data);
  std::swap(this->capacity, buffer.capacity);
  std::swap(this->actualSize, buffer.actualSize);
}

} // namespace autocomp
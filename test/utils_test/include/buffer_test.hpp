#ifndef AC_BUFFER_TEST_HPP
#define AC_BUFFER_TEST_HPP

/* C++ System Headers */
#include <string>
#include <stdexcept>

/* External headers */
#include "gtest/gtest.h"

/* Project headers */
#include "utils/buffer.hpp"

class BufferTest : public ::testing::Test
{
protected:

  autocomp::Buffer * buffer;
  std::size_t bufferCapacity = 512000;

  void SetUp()
  {
    ASSERT_NO_THROW({
      buffer = new autocomp::Buffer(bufferCapacity);
    });
  }

  void TearDown() {
    delete buffer;
  }
}; // class BufferTest


TEST_F(BufferTest, SetValidSize)
{
  size_t validSize = bufferCapacity - 1;

  ASSERT_LT(validSize, bufferCapacity);

  ASSERT_NO_THROW({
    buffer->setSize(validSize);
  });  
}

TEST_F(BufferTest, SetInvalidSize)
{
  size_t invalidSize = bufferCapacity + 1;

  ASSERT_GT(invalidSize, bufferCapacity);

  ASSERT_THROW(
    {
      buffer->setSize(invalidSize);
    },
    std::domain_error);  
}

TEST_F(BufferTest, SetValidStringData)
{
  std::string data(buffer->getCapacity() - 1, 'A');

  ASSERT_NO_THROW({
    buffer->setData(data);
  });

  EXPECT_STREQ(data.data(), buffer->getData());
}

TEST_F(BufferTest, SetValidCStringData)
{
  std::string data(buffer->getCapacity() - 1, 'A');

  ASSERT_NO_THROW({
    buffer->setData(data.c_str(), data.size());
  });

  EXPECT_STREQ(data.data(), buffer->getData());
}

TEST_F(BufferTest, SetInvalidStringData)
{
  std::string data(buffer->getCapacity() + 1, 'A');

  ASSERT_THROW(
    {
      buffer->setData(data);
    },
    std::domain_error);
}

TEST_F(BufferTest, SetInvalidCStringData)
{
  std::string data(buffer->getCapacity() + 1, 'A');

  ASSERT_THROW(
    {
      buffer->setData(data.c_str(), data.size());
    },
    std::domain_error);
}

#endif //AC_BUFFER_TEST_HPP
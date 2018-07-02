#ifndef SNAPPY_TEST_H
#define SNAPPY_TEST_H

/* C++ System Headers */
#include <string>

/* External headers */
#include "gtest/gtest.h"
#include "snappy.h"

/* Project headers */
#include "test_constants.hpp"
#include "common_functions.hpp"

class SnappyTest : public ::testing::Test
{
protected:

  std::string originalData;
  std::string compressedData;
  std::string uncompressedData;

  void SetUp()
  {
    ASSERT_NO_THROW({
      originalData = autocomp::test::getDataFromFile(
          autocomp::test::constants::compressionTestFilename
        );
    });
  }
     
  void TearDown() {}
}; // class SnappyTest


TEST_F(SnappyTest, CompressesAndUncompresses) {
  size_t compressedSize = snappy::Compress(originalData.data(),
                                           originalData.size(),
                                           &compressedData);
  ASSERT_EQ(compressedSize,compressedData.size());

  ASSERT_TRUE(snappy::Uncompress(compressedData.data(), compressedData.size(),
                                 &uncompressedData));

  ASSERT_EQ(originalData.size(), uncompressedData.size());
  ASSERT_EQ(originalData, uncompressedData);
}

#endif //SNAPPY_TEST_H
#ifndef ZLIB_TEST_H
#define ZLIB_TEST_H

/* C++ System Headers */
#include <string>

/* External headers */
#include "gtest/gtest.h"

extern "C" {
  #include "zlib.h"
}

/* Project headers */
#include "test_constants.hpp"
#include "common_functions.hpp"

class ZlibTest : public ::testing::Test
{
protected:

  std::string originalData;
  Bytef * compressedBuffer, * uncompressedBuffer;
  uLong compressedCapacity, uncompressedCapacity;
  uLong compressedSize, uncompressedSize;

  void SetUp()
  {
    ASSERT_NO_THROW({
      originalData = autocomp::test::getDataFromFile(
          autocomp::test::constants::compressionTestFilename
        );
    });

    compressedCapacity = originalData.size() * 1.1 + 12;
    uncompressedCapacity = originalData.size();

    ASSERT_NO_THROW(compressedBuffer = new Bytef[compressedCapacity]);
    ASSERT_NO_THROW(uncompressedBuffer = new Bytef[uncompressedCapacity]);
  }
     
  void TearDown()
  {
    delete[] compressedBuffer;
    delete[] uncompressedBuffer;
  }
}; // class ZlibTest


TEST_F(ZlibTest, CompressesAndUncompresses)
{
  const Bytef * originalBuffer = reinterpret_cast<const Bytef *>(
      originalData.data()
    );

  for (int level = 1; level <= 9; level++) {
    memset(compressedBuffer, 0, compressedCapacity);
    memset(uncompressedBuffer, 0, uncompressedCapacity);
    compressedSize = compressedCapacity;
    uncompressedSize = uncompressedCapacity;

    /* Compression */
    ASSERT_EQ(Z_OK, compress2(compressedBuffer, &compressedSize,
                              originalBuffer, originalData.size(), level));
    EXPECT_LE(compressedSize, compressedCapacity);

    /* Decompression */
    ASSERT_EQ(Z_OK, uncompress(uncompressedBuffer, &uncompressedSize,
                                 compressedBuffer, compressedSize));
    EXPECT_LE(uncompressedSize, uncompressedCapacity);
    EXPECT_LE(originalData.size(), uncompressedSize);

    /* Integrity check */
    ASSERT_EQ(0, memcmp(originalBuffer, uncompressedBuffer,
                        originalData.size()));
  }
}

TEST_F(ZlibTest, LevelZeroDoesNotCompress)
{
  const Bytef * originalBuffer = reinterpret_cast<const Bytef *>(
      originalData.data()
    );

  memset(compressedBuffer, 0, compressedCapacity);
  memset(uncompressedBuffer, 0, uncompressedCapacity);
  compressedSize = compressedCapacity;
  uncompressedSize = uncompressedCapacity;

  /* Compression */
  ASSERT_EQ(Z_OK, compress2(compressedBuffer, &compressedSize,
                            originalBuffer, originalData.size(), 0));
  ASSERT_GT(compressedSize, originalData.size());
    
  /* Decompression */
  ASSERT_EQ(Z_OK, uncompress(uncompressedBuffer, &uncompressedSize,
                             compressedBuffer, compressedSize));
  EXPECT_LE(originalData.size(), uncompressedSize);

  /* Integrity check */
  ASSERT_EQ(0, memcmp(originalBuffer, uncompressedBuffer,
                      originalData.size()));
}

#endif //ZLIB_TEST_H
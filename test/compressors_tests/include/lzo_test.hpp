#ifndef LZO_TEST_H
#define LZO_TEST_H

/* C++ System Headers */
#include <string>

/* External headers */
#include "gtest/gtest.h"

extern "C" {
  #include "lzo/lzoconf.h"
  #include "lzo/lzo1x.h"
}

/* Project headers */
#include "test_constants.hpp"
#include "common_functions.hpp"

class LZOTest : public ::testing::Test
{
protected:

  std::string originalData;
  lzo_bytep compressedBuffer;
  lzo_bytep uncompressedBuffer;
  lzo_align_t * workMemory;
  lzo_uint compressedCapacity, uncompressedCapacity;
  lzo_uint compressedSize, uncompressedSize;
  lzo_uint workMemorySize;

  void SetUp()
  {
    ASSERT_EQ(LZO_E_OK, lzo_init());

    ASSERT_NO_THROW({
      originalData = autocomp::test::getDataFromFile(
          autocomp::test::constants::compressionTestFilename
        );
    });

    compressedCapacity = originalData.size() + originalData.size() / 16
                          + 64 + 3;
    uncompressedCapacity = originalData.size();
    workMemorySize = (LZO1X_999_MEM_COMPRESS + sizeof(lzo_align_t) - 1)
                        / sizeof(lzo_align_t);

    ASSERT_NO_THROW(compressedBuffer = new lzo_byte[compressedCapacity]);
    ASSERT_NO_THROW(uncompressedBuffer = new lzo_byte[uncompressedCapacity]);
    ASSERT_NO_THROW(workMemory = new lzo_align_t[workMemorySize]);
  }
     
  void TearDown()
  {
    delete[] compressedBuffer;
    delete[] uncompressedBuffer;
    delete[] workMemory;
  }
}; // class LZOTest


TEST_F(LZOTest, CompressesAndUncompresses)
{
  const lzo_bytep originalBuffer = reinterpret_cast<const lzo_bytep>(
      originalData.data()
    );

  for (int compressionLevel = 1; compressionLevel <= 9; compressionLevel++) {
    memset(compressedBuffer, 0, compressedCapacity);
    memset(uncompressedBuffer, 0, uncompressedCapacity);
    memset(workMemory, 0, workMemorySize);
    compressedSize = compressedCapacity;
    uncompressedSize = uncompressedCapacity;

    /* Compression */
    ASSERT_EQ(LZO_E_OK,
              lzo1x_999_compress_level(originalBuffer, originalData.size(),
                                       compressedBuffer, &compressedSize,
                                       workMemory, nullptr, 0, nullptr,
                                       compressionLevel));
    EXPECT_LE(compressedSize, compressedCapacity);

    /* Decompression */
    ASSERT_EQ(LZO_E_OK, lzo1x_decompress(compressedBuffer, compressedSize,
                                         uncompressedBuffer, &uncompressedSize,
                                         nullptr));
    EXPECT_LE(uncompressedSize, uncompressedCapacity);
    EXPECT_LE(originalData.size(), uncompressedSize);

    /* Integrity check */
    ASSERT_EQ(0, memcmp(originalBuffer, uncompressedBuffer,
                        originalData.size()));
  }
}

#endif //LZO_TEST_H
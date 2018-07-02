#ifndef BZIP2_TEST_H
#define BZIP2_TEST_H

/* C++ System Headers */
#include <string>

/* External headers */
#include "gtest/gtest.h"

extern "C" {
  #include "bzlib.h"
}

/* Project headers */
#include "test_constants.hpp"
#include "common_functions.hpp"

class Bzip2Test : public ::testing::Test
{
protected:

  std::string originalData;
  char * originalBuffer;
  char * compressedBuffer;
  char * uncompressedBuffer;
  unsigned int compressedCapacity, uncompressedCapacity;
  unsigned int compressedSize, uncompressedSize;

  bz_stream stream;

  void SetUp()
  {
    // Read file contents
    ASSERT_NO_THROW({
      originalData = autocomp::test::getDataFromFile(
          autocomp::test::constants::compressionTestFilename
        );
    });

    compressedCapacity = 2 * originalData.size();
    uncompressedCapacity = originalData.size();

    ASSERT_NO_THROW(originalBuffer = new char[originalData.size()]);
    ASSERT_NO_THROW(compressedBuffer = new char[compressedCapacity]);
    ASSERT_NO_THROW(uncompressedBuffer = new char[uncompressedCapacity]);

    memcpy(originalBuffer, originalData.data(), originalData.size());
  }
     
  void TearDown()
  {
    delete[] originalBuffer;
    delete[] compressedBuffer;
    delete[] uncompressedBuffer;
  }
  
}; // class Bzip2Test

TEST_F(Bzip2Test, CompressesAndUncompresses)
{
  for (int compressionLevel = 1; compressionLevel <= 9; compressionLevel++) {
    memset(compressedBuffer, 0, compressedCapacity);
    memset(uncompressedBuffer, 0, uncompressedCapacity);
    compressedSize = compressedCapacity;
    uncompressedSize = uncompressedCapacity;

    /* Compression */
    ASSERT_EQ(BZ_OK,
              BZ2_bzBuffToBuffCompress(compressedBuffer, &compressedSize,
                                       originalBuffer, originalData.size(),
                                       compressionLevel, 0, 0));
    EXPECT_LE(compressedSize, compressedCapacity);

    /* Decompression */
    ASSERT_EQ(BZ_OK,
              BZ2_bzBuffToBuffDecompress(uncompressedBuffer, &uncompressedSize,
                                         compressedBuffer, compressedSize,
                                         0, 0));
    EXPECT_LE(uncompressedSize, uncompressedCapacity);
    EXPECT_LE(originalData.size(), uncompressedSize);

    /* Integrity check */
    ASSERT_EQ(0, memcmp(originalBuffer, uncompressedBuffer,
                        originalData.size()));
  }
}

#endif //BZIP2_TEST_H
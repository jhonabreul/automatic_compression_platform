#ifndef AC_LZO_COMPRESSOR_TEST_H
#define AC_LZO_COMPRESSOR_TEST_H

/* C++ System Headers */
#include <string>
#include <cstddef>
#include <stdexcept>

/* External headers */
#include "gtest/gtest.h"

/* Project headers */
#include "test_constants.hpp"
#include "common_functions.hpp"
#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "compression/lzo_compressor.hpp"

class LZOCompressorTest : public ::testing::Test
{
protected:

  std::string originalData;
  autocomp::Buffer * originalBuffer, * compressedBuffer, * decompressedBuffer;

  void SetUp()
  {
    ASSERT_NO_THROW({
      originalData = autocomp::test::getDataFromFile(
          autocomp::test::constants::compressionTestFilename
        );
    });

    ASSERT_NO_THROW({
      originalBuffer = new autocomp::Buffer(originalData.size() * 1.1);
    });
    ASSERT_NO_THROW({
      compressedBuffer = new autocomp::Buffer(originalData.size() * 1.2);
    });
    ASSERT_NO_THROW({
      decompressedBuffer = new autocomp::Buffer(originalData.size() * 1.1);
    });

    ASSERT_NO_THROW(originalBuffer->setData(originalData));
  }
     
  void TearDown()
  {
    delete originalBuffer;
    delete compressedBuffer;
    delete decompressedBuffer;
  }
}; // class LZOCompressorTest

TEST_F(LZOCompressorTest, CompressionLevelValidation)
{
  for (int level = 1; level <= 9; level++) {  
    ASSERT_NO_THROW({
      autocomp::LZOCompressor compressor(level);
      compressor.setCompressionLevel(level);
    });
  }

  int validLevel = 3;
  int negativeLevel = 0;
  ASSERT_THROW(
    {
      autocomp::LZOCompressor compressor(negativeLevel);
    },
    autocomp::exceptions::InvalidCompressionLevelError);

  ASSERT_THROW(
    {
      autocomp::LZOCompressor compressor(validLevel);
      compressor.setCompressionLevel(negativeLevel);
    },
    autocomp::exceptions::InvalidCompressionLevelError);

  int outOfBoundLevel = 10;
  ASSERT_THROW(
    {
      autocomp::LZOCompressor compressor(outOfBoundLevel);
    },
    autocomp::exceptions::InvalidCompressionLevelError);

  ASSERT_THROW(
    {
      autocomp::LZOCompressor compressor(validLevel);
      compressor.setCompressionLevel(outOfBoundLevel);
    },
    autocomp::exceptions::InvalidCompressionLevelError);
}

TEST_F(LZOCompressorTest, CompressesAndDecompresses)
{
  autocomp::LZOCompressor compressor(3);

  for (int level = 1; level <= 9; level++) {
    /* Compression */
    ASSERT_NO_THROW({
      compressor.setCompressionLevel(level);
      compressor.compress(*originalBuffer, *compressedBuffer);
    });

    /* Decompression */
    ASSERT_NO_THROW({
      compressor.setCompressionLevel(level);
      compressor.decompress(*compressedBuffer, *decompressedBuffer);
    });

    /* Integrity check */
    ASSERT_EQ(originalBuffer->getSize(), decompressedBuffer->getSize());
    ASSERT_EQ(0, memcmp(originalBuffer->getData(),
                        decompressedBuffer->getData(),
                        originalBuffer->getSize()));
  }
}

#endif //AC_LZO_COMPRESSOR_TEST_H
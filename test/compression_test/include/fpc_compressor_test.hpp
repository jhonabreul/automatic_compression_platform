#ifndef AC_FPC_COMPRESSOR_TEST_H
#define AC_FPC_COMPRESSOR_TEST_H

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
#include "compression/fpc_compressor.hpp"

class FPCCompressorTest : public ::testing::Test
{
protected:

  std::string originalData;
  autocomp::Buffer * originalBuffer, * compressedBuffer, * decompressedBuffer;

  void SetUp()
  {
    ASSERT_NO_THROW({
      originalData = autocomp::test::getDataFromFile(
          autocomp::test::constants::fpcTestFilename
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
}; // class FPCCompressorTest

TEST_F(FPCCompressorTest, CompressionLevelValidation)
{
  int levels[] = {1, 2, 8, 10, 16, 20, 24, 28};

  for (int & level : levels) {  
    ASSERT_NO_THROW({
      autocomp::FPCCompressor compressor(level);
      compressor.setCompressionLevel(level);
    });
  }

  int validLevel = 20;
  int negativeLevel = 0;
  ASSERT_THROW(
    {
      autocomp::FPCCompressor compressor(negativeLevel);
    },
    autocomp::exceptions::InvalidCompressionLevelError);

  ASSERT_THROW(
    {
      autocomp::FPCCompressor compressor(validLevel);
      compressor.setCompressionLevel(negativeLevel);
    },
    autocomp::exceptions::InvalidCompressionLevelError);

  int outOfBoundLevel = 100;
  ASSERT_THROW(
    {
      autocomp::FPCCompressor compressor(outOfBoundLevel);
    },
    autocomp::exceptions::InvalidCompressionLevelError);

  ASSERT_THROW(
    {
      autocomp::FPCCompressor compressor(validLevel);
      compressor.setCompressionLevel(outOfBoundLevel);
    },
    autocomp::exceptions::InvalidCompressionLevelError);
}

TEST_F(FPCCompressorTest, CompressesAndDecompresses)
{
  autocomp::FPCCompressor compressor(20);
  int levels[] = {1, 2, 8, 10, 16, 20, 24, 28};

  for (int & level : levels) {
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

#endif //AC_FPC_COMPRESSOR_TEST_H
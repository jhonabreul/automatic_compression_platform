#ifndef AC_ROUND_ROBIN_COMPRESSOR_TEST_H
#define AC_ROUND_ROBIN_COMPRESSOR_TEST_H

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
#include "compression/round_robin_compressor.hpp"

class RoundRobinCompressorTest : public ::testing::Test
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
}; // class RoundRobinCompressorTest

TEST_F(RoundRobinCompressorTest, RoundRobinFashion)
{
  std::shared_ptr<autocomp::io::PerformanceDataWriter> performanceDataWriter =
    std::make_shared<autocomp::io::PerformanceDataWriter>();
  autocomp::RoundRobinCompressor roundRobinCompressor(performanceDataWriter);

  int nZlibLevels = 10;
  int nSnappyLevels = 1;
  int nLZOLevels = 9;
  int nBzip2Levels = 9;
  int nLZMALevels = 9;
  int nCopys = 1;

  for (int i = 0; i < nZlibLevels; i++) {
    ASSERT_EQ(autocomp::ZLIB, roundRobinCompressor.compress(*originalBuffer,
                                                            *compressedBuffer));
  }

  for (int i = 0; i < nSnappyLevels; i++) {
    ASSERT_EQ(autocomp::SNAPPY,
              roundRobinCompressor.compress(*originalBuffer,
                                            *compressedBuffer));
  }

  for (int i = 0; i < nLZOLevels; i++) {
    ASSERT_EQ(autocomp::LZO, roundRobinCompressor.compress(*originalBuffer,
                                                           *compressedBuffer));
  }

  for (int i = 0; i < nBzip2Levels; i++) {
    ASSERT_EQ(autocomp::BZIP2,
              roundRobinCompressor.compress(*originalBuffer,
                                            *compressedBuffer));
  }

  for (int i = 0; i < nLZMALevels; i++) {
    ASSERT_EQ(autocomp::LZMA, roundRobinCompressor.compress(*originalBuffer,
                                                            *compressedBuffer));
  }

  for (int i = 0; i < nCopys; i++) {
    ASSERT_EQ(autocomp::COPY, roundRobinCompressor.compress(*originalBuffer,
                                                            *compressedBuffer));
  }

  ASSERT_EQ(autocomp::ZLIB, roundRobinCompressor.compress(*originalBuffer,
                                                          *compressedBuffer));
}

TEST_F(RoundRobinCompressorTest, CompressesAndDecompresses)
{
  std::shared_ptr<autocomp::io::PerformanceDataWriter> performanceDataWriter =
    std::make_shared<autocomp::io::PerformanceDataWriter>();
  autocomp::RoundRobinCompressor roundRobinCompressor(performanceDataWriter);
  int nTests = 200;


  for (int i = 0; i < nTests; i++) {
    autocomp::Compressor compressorType;
    ASSERT_NO_THROW({
      compressorType = roundRobinCompressor.compress(*originalBuffer,
                                                     *compressedBuffer);
    });    
    
    switch (compressorType) {
      case autocomp::ZLIB:
        ASSERT_NO_THROW({
          autocomp::ZlibCompressor().decompress(*compressedBuffer,
                                                *decompressedBuffer);
        });
        break;
    
      case autocomp::SNAPPY:
        ASSERT_NO_THROW({
          autocomp::SnappyCompressor().decompress(*compressedBuffer,
                                                  *decompressedBuffer);
        });
        break;

      case autocomp::LZO:
        ASSERT_NO_THROW({
          autocomp::LZOCompressor().decompress(*compressedBuffer,
                                               *decompressedBuffer);
        });
        break;

      case autocomp::BZIP2:
        ASSERT_NO_THROW({
          autocomp::Bzip2Compressor().decompress(*compressedBuffer,
                                                 *decompressedBuffer);
        });
        break;

      case autocomp::LZMA:
        ASSERT_NO_THROW({
          autocomp::LZMACompressor().decompress(*compressedBuffer,
                                                *decompressedBuffer);
        });
        break;

      case autocomp::COPY:
        ASSERT_EQ(0, compressedBuffer->getSize());
        continue;
        
      default:
        FAIL() << "Unknown compressor returned by "
                  "autocomp::RoundRobinCompressor::compress()";
    }

    // Integrity check
    ASSERT_EQ(originalBuffer->getSize(), decompressedBuffer->getSize());
    ASSERT_EQ(0, memcmp(originalBuffer->getData(),
                        decompressedBuffer->getData(),
                        originalBuffer->getSize()));

    compressedBuffer->setSize(0);
    decompressedBuffer->setSize(0);
  }
}

#endif //AC_ROUND_ROBIN_COMPRESSOR_TEST_H
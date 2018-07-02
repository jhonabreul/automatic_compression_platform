#ifndef AC_SINGLE_COMPRESSOR_TEST_H
#define AC_SINGLE_COMPRESSOR_TEST_H

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
#include "messaging/compressor.pb.h"
#include "compression/single_compressor.hpp"

class SingleCompressorTest : public ::testing::Test
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
}; // class SingleCompressorTest

TEST_F(SingleCompressorTest, CompressesAndDecompresses)
{
  std::shared_ptr<autocomp::io::PerformanceDataWriter> performanceDataWriter =
    std::make_shared<autocomp::io::PerformanceDataWriter>();
  autocomp::SingleCompressor singleCompressor(performanceDataWriter);
  
  autocomp::Compressor compressors[] = {autocomp::ZLIB, autocomp::SNAPPY,
                                        autocomp::LZO, autocomp::BZIP2,
                                        autocomp::LZMA, autocomp::COPY};

  for (auto & compressor : compressors) {
    singleCompressor.setCompressor(compressor);
    autocomp::Compressor compressorType;
    
    ASSERT_NO_THROW({
      compressorType = singleCompressor.compress(*originalBuffer,
                                                 *compressedBuffer);
    });
    ASSERT_EQ(compressor, compressorType);
    
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
                  "autocomp::SingleCompressor::compress()";
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

#endif //AC_SINGLE_COMPRESSOR_TEST_H
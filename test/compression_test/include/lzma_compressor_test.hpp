#ifndef AC_LZMA_COMPRESSOR_TEST_H
#define AC_LZMA_COMPRESSOR_TEST_H

/* C++ System Headers */
#include <string>
#include <cstddef>
#include <stdexcept>
#include <fstream>

/* External headers */
#include "gtest/gtest.h"

/* Project headers */
#include "test_constants.hpp"
#include "common_functions.hpp"
#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "compression/lzma_compressor.hpp"

class LZMACompressorTest : public ::testing::Test
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
}; // class LZMACompressorTest

TEST_F(LZMACompressorTest, CompressionLevelValidation)
{
  for (int level = 0; level <= 9; level++) {  
    ASSERT_NO_THROW({
      autocomp::LZMACompressor compressor(level);
      compressor.setCompressionLevel(level);
    });
  }

  int validLevel = 6;
  int negativeLevel = -1;
  ASSERT_THROW(
    {
      autocomp::LZMACompressor compressor(negativeLevel);
    },
    autocomp::exceptions::InvalidCompressionLevelError);

  ASSERT_THROW(
    {
      autocomp::LZMACompressor compressor(validLevel);
      compressor.setCompressionLevel(negativeLevel);
    },
    autocomp::exceptions::InvalidCompressionLevelError);

  int outOfBoundLevel = 10;
  ASSERT_THROW(
    {
      autocomp::LZMACompressor compressor(outOfBoundLevel);
    },
    autocomp::exceptions::InvalidCompressionLevelError);

  ASSERT_THROW(
    {
      autocomp::LZMACompressor compressor(validLevel);
      compressor.setCompressionLevel(outOfBoundLevel);
    },
    autocomp::exceptions::InvalidCompressionLevelError);
}

TEST_F(LZMACompressorTest, CompressesAndDecompresses)
{
  autocomp::LZMACompressor compressor(9);

  for (int level = 0; level <= 9; level++) {
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

TEST_F(LZMACompressorTest, CompressesAndDecompressesInChunks)
{
  autocomp::LZMACompressor compressor(7);
  int chunkSize = 15000; // bytes (15 KB)
  autocomp::Buffer inData(chunkSize);
  autocomp::Buffer compressedData(1.1 * chunkSize);
  autocomp::Buffer decompressedData(chunkSize);

  std::ifstream in(autocomp::test::constants::compressionTestFilename,
                   std::ifstream::in | std::ifstream::binary);

  std::string fileData("");

  int nChunks = 0;

  while (not in.eof()) {
    in.read(inData.getData(), chunkSize);

    nChunks++;

    inData.setSize(in.gcount());

    compressor.compress(inData, compressedData);
    compressor.decompress(compressedData, decompressedData);

    ASSERT_EQ(inData.getSize(), decompressedData.getSize());
    ASSERT_EQ(0, memcmp(inData.getData(), decompressedData.getData(),
                        inData.getSize()));

    fileData.append(decompressedData.getData(), decompressedData.getSize());
  }

  in.close();

  std::string realFileData;

  ASSERT_NO_THROW({
    realFileData = autocomp::test::getDataFromFile(
        autocomp::test::constants::compressionTestFilename
      );
  });

  ASSERT_EQ(realFileData.size(), fileData.size());
  ASSERT_TRUE(realFileData == fileData);
}

#endif //AC_LZMA_COMPRESSOR_TEST_H
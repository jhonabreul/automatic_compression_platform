#ifndef AC_BZIP2_COMPRESSOR_TEST_H
#define AC_BZIP2_COMPRESSOR_TEST_H

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
#include "compression/bzip2_compressor.hpp"

class Bzip2CompressorTest : public ::testing::Test
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
}; // class Bzip2CompressorTest

TEST_F(Bzip2CompressorTest, CompressionLevelValidation)
{
  for (int level = 1; level <= 9; level++) {  
    ASSERT_NO_THROW({
      autocomp::Bzip2Compressor compressor(level);
      compressor.setCompressionLevel(level);
    });
  }

  int validLevel = 9;
  int negativeLevel = 0;
  ASSERT_THROW(
    {
      autocomp::Bzip2Compressor compressor(negativeLevel);
    },
    autocomp::exceptions::InvalidCompressionLevelError);

  ASSERT_THROW(
    {
      autocomp::Bzip2Compressor compressor(validLevel);
      compressor.setCompressionLevel(negativeLevel);
    },
    autocomp::exceptions::InvalidCompressionLevelError);

  int outOfBoundLevel = 10;
  ASSERT_THROW(
    {
      autocomp::Bzip2Compressor compressor(outOfBoundLevel);
    },
    autocomp::exceptions::InvalidCompressionLevelError);

  ASSERT_THROW(
    {
      autocomp::Bzip2Compressor compressor(validLevel);
      compressor.setCompressionLevel(outOfBoundLevel);
    },
    autocomp::exceptions::InvalidCompressionLevelError);
}

TEST_F(Bzip2CompressorTest, CompressesAndDecompresses)
{
  autocomp::Bzip2Compressor compressor(9);

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

TEST_F(Bzip2CompressorTest, CompressionTimeVsCompressionRatio)
{
  std::string data;
  autocomp::Buffer * buffer, * compressed, * decompressed;

  std::vector<std::string> files{
    "alice29.txt", "bib", "book2", "geo", "paper1", "paper3", "paper5", "pic", "progc",
    "progp", "trans", "asyoulik.txt",
    "book1", "cp.html", "lcet10.txt", "news",
    "obj2", "paper2", "paper4", "paper6", "plrabn12.txt", "progl",
    "ptt5", "sum"
  };

  std::string filesPath = "/home/jhonathanabreu/Documentos/ula/tesis/data/compression_time_vs_ratio/";

  autocomp::Bzip2Compressor compressor;

  for (const auto & file : files) {
    ASSERT_NO_THROW(data = autocomp::test::getDataFromFile(filesPath + file));
    ASSERT_NO_THROW(buffer = new autocomp::Buffer(data.size() * 1.1));
    ASSERT_NO_THROW(compressed = new autocomp::Buffer(data.size() * 1.1));
    ASSERT_NO_THROW(decompressed = new autocomp::Buffer(data.size() * 1.1));
    ASSERT_NO_THROW(buffer->setData(data));

    auto tic = std::chrono::high_resolution_clock::now();
    compressor.compress(*buffer, *compressed);
    auto toc = std::chrono::high_resolution_clock::now();
    int compressionTime =
      std::chrono::duration_cast<std::chrono::microseconds>(toc - tic).count();
    float compressionRatio = buffer->getSize() / (float) compressed->getSize();

    std::cout << compressionTime << " " << compressionRatio << std::endl;
  }
}

#endif //AC_BZIP2_COMPRESSOR_TEST_H
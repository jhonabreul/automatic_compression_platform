#ifndef AC_SNAPPY_COMPRESSOR_TEST_H
#define AC_SNAPPY_COMPRESSOR_TEST_H

/* C++ System Headers */
#include <string>
#include <cstddef>
#include <stdexcept>
#include <chrono>

/* External headers */
#include "gtest/gtest.h"
#include "snappy.h"

/* Project headers */
#include "test_constants.hpp"
#include "common_functions.hpp"
#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "compression/snappy_compressor.hpp"

class SnappyCompressorTest : public ::testing::Test
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
      compressedBuffer =
        new autocomp::Buffer(snappy::MaxCompressedLength(originalData.size()));
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
}; // class SnappyCompressorTest

TEST_F(SnappyCompressorTest, CompressesAndDecompresses)
{
  autocomp::SnappyCompressor compressor;

  /* Compression */
  ASSERT_NO_THROW({
    compressor.compress(*originalBuffer, *compressedBuffer);
  });

  /* Decompression */
  ASSERT_NO_THROW({
    compressor.decompress(*compressedBuffer, *decompressedBuffer);
  });

  /* Integrity check */
  ASSERT_EQ(originalBuffer->getSize(), decompressedBuffer->getSize());
  ASSERT_EQ(0, memcmp(originalBuffer->getData(),
                      decompressedBuffer->getData(),
                      originalBuffer->getSize()));
}

TEST_F(SnappyCompressorTest, CompressionTimeVsCompressionRatio)
{
  std::string data;
  autocomp::Buffer * buffer, * compressed, * decompressed;

  /*
  std::vector<std::string> files{
    "alice29.txt", "bib", "book2", "dickens", "geo", "kennedy.xls", "mozilla",
    "nci", "obj1", "ooffice", "paper1", "paper3", "paper5", "pic", "progc",
    "progp", "reymont", "sao", "trans", "xargs.1", "x-ray", "asyoulik.txt",
    "book1", "cp.html", "fields.c", "grammar.lsp", "lcet10.txt", "mr", "news",
    "obj2", "osdb", "paper2", "paper4", "paper6", "plrabn12.txt", "progl",
    "ptt5", "samba", "sum", "webster", "xml"
  };
  */

  std::vector<std::string> files{
    "alice29.txt", "bib", "book2", "geo", "paper1", "paper3", "paper5", "pic", "progc",
    "progp", "trans", "asyoulik.txt",
    "book1", "cp.html", "lcet10.txt", "news",
    "obj2", "paper2", "paper4", "paper6", "plrabn12.txt", "progl",
    "ptt5", "sum"
  };

  std::string filesPath = "/home/jhonathanabreu/Documentos/ula/tesis/data/compression_time_vs_ratio/";

  autocomp::SnappyCompressor compressor;

  for (const auto & file : files) {
    ASSERT_NO_THROW(data = autocomp::test::getDataFromFile(filesPath + file));
    ASSERT_NO_THROW(buffer = new autocomp::Buffer(data.size() * 1.1));
    ASSERT_NO_THROW({
      compressed =
        new autocomp::Buffer(snappy::MaxCompressedLength(data.size()));
    });
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

#endif //AC_SNAPPY_COMPRESSOR_TEST_H
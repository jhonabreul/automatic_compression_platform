#ifndef AC_FILE_PROCESSOR_TEST_H
#define AC_FILE_PROCESSOR_TEST_H

/* C++ System Headers */
#include <string>
#include <cstddef>
#include <stdexcept>
#include <memory>
#include <cmath>

/* External headers */
#include "gtest/gtest.h"

/* Project headers */
#include "test_constants.hpp"
#include "common_functions.hpp"
#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "messaging/compressor.pb.h"
#include "compression/round_robin_compressor.hpp"
#include "compression/file_processor.hpp"

TEST(FileProcessorTest, ProcessesSingleFile)
{
  unsigned int chunkSize = 15;
  autocomp::Buffer compressedData(1.1 * chunkSize * 1024);
  autocomp::Buffer decompressedData(1.1 * chunkSize * 1024);

  std::shared_ptr<autocomp::io::PerformanceDataWriter> performanceDataWriter =
    std::make_shared<autocomp::io::PerformanceDataWriter>();

  autocomp::FileProcessor fileProcessor(
      chunkSize,
      std::make_shared<autocomp::RoundRobinCompressor>(performanceDataWriter)
    );

  ASSERT_NO_THROW({
    fileProcessor.preparePath(autocomp
                              ::test
                              ::constants
                              ::fpcTestFilename);
  });

  int nFiles = 0;

  while(fileProcessor.hasNextFile()) {
    size_t fileSize;
    int nChunks = 0;

    ASSERT_NO_THROW({
      fileSize = fileProcessor.openNextFile();
    });

    std::string fileData("");

    while (fileProcessor.hasNextChunk()) {
      nChunks++;

      compressedData.setData(std::string(""));
      decompressedData.setData(std::string(""));
      autocomp::Compressor usedCompressor;

      ASSERT_NO_THROW({
        usedCompressor = fileProcessor.getNextChunk(compressedData);
      });

      switch (usedCompressor) {
        case autocomp::ZLIB:
          ASSERT_NO_THROW({
            autocomp::ZlibCompressor().decompress(compressedData,
                                                  decompressedData);
          });
          break;
      
        case autocomp::SNAPPY:
          ASSERT_NO_THROW({
            autocomp::SnappyCompressor().decompress(compressedData,
                                                    decompressedData);
          });
          break;

        case autocomp::LZO:
          ASSERT_NO_THROW({
            autocomp::LZOCompressor().decompress(compressedData,
                                                 decompressedData);
          });
          break;

        case autocomp::BZIP2:
          ASSERT_NO_THROW({
            autocomp::Bzip2Compressor().decompress(compressedData,
                                                   decompressedData);
          });
          break;

        case autocomp::LZMA:
          ASSERT_NO_THROW({
            autocomp::LZMACompressor().decompress(compressedData,
                                                  decompressedData);
          });
          break;

        case autocomp::COPY:
          decompressedData.swap(compressedData);
          break;
          
        default:
          FAIL() << "Unknown compressor returned by "
                    "autocomp::FileProcessor::compress()";
      }

      fileData.append(decompressedData.getData(),
                      decompressedData.getSize());
    }

    ASSERT_EQ(ceil(fileSize / (double) (chunkSize * 1024)), nChunks);

    nFiles++;

    // Integrity check
    std::string originalFileData;

    ASSERT_NO_THROW({
      originalFileData = autocomp::test::getDataFromFile(
          fileProcessor.getCurrentFileName()
        );
    });

    ASSERT_EQ(originalFileData.size(), fileData.size());
    ASSERT_TRUE(originalFileData == fileData);
  }

  ASSERT_EQ(1, nFiles);
}

TEST(FileProcessorTest, ProcessesDirectory)
{
  unsigned int chunkSize = 15;
  autocomp::Buffer compressedData(1.1 * chunkSize * 1024);
  autocomp::Buffer decompressedData(1.1 * chunkSize * 1024);

  std::vector<std::string> realFileList{
    "test/test_files/alice29.txt",
    "test/test_files/test_dir/test_file2.txt",
    "test/test_files/test_dir/test_file3.xml",
    "test/test_files/test_dir/test_file.txt",
    "test/test_files/test_dir/test_subdir2/test_file.html",
    "test/test_files/test_dir/test_subdir2/test_subsubdir/test_subsubfile.txt",
    "test/test_files/test_dir/test_subdir/test_subfile.txt",
    "test/test_files/test.trace"
  };

  std::vector<std::string> fileList;

  std::shared_ptr<autocomp::io::PerformanceDataWriter> performanceDataWriter =
    std::make_shared<autocomp::io::PerformanceDataWriter>();
  autocomp::FileProcessor fileProcessor(
      chunkSize,
      std::make_shared<autocomp::RoundRobinCompressor>(performanceDataWriter)
    );

  ASSERT_NO_THROW({
    fileProcessor.preparePath(autocomp::test::constants::testDirectory);
  });

  while(fileProcessor.hasNextFile()) {
    size_t fileSize;
    int nChunks = 0;

    ASSERT_NO_THROW({
      fileSize = fileProcessor.openNextFile();
    });

    std::string fileData("");

    while (fileProcessor.hasNextChunk()) {
       nChunks++;

      compressedData.setData(std::string(""));
      decompressedData.setData(std::string(""));
      autocomp::Compressor usedCompressor;

      ASSERT_NO_THROW({
        usedCompressor = fileProcessor.getNextChunk(compressedData);
      });

      switch (usedCompressor) {
        case autocomp::ZLIB:
          ASSERT_NO_THROW({
            autocomp::ZlibCompressor().decompress(compressedData,
                                                  decompressedData);
          });
          break;
      
        case autocomp::SNAPPY:
          ASSERT_NO_THROW({
            autocomp::SnappyCompressor().decompress(compressedData,
                                                    decompressedData);
          });
          break;

        case autocomp::LZO:
          ASSERT_NO_THROW({
            autocomp::LZOCompressor().decompress(compressedData,
                                                 decompressedData);
          });
          break;

        case autocomp::BZIP2:
          ASSERT_NO_THROW({
            autocomp::Bzip2Compressor().decompress(compressedData,
                                                   decompressedData);
          });
          break;

        case autocomp::LZMA:
          ASSERT_NO_THROW({
            autocomp::LZMACompressor().decompress(compressedData,
                                                  decompressedData);
          });
          break;

        case autocomp::COPY:
          decompressedData.swap(compressedData);
          break;
          
        default:
          FAIL() << "Unknown compressor returned by "
                    "autocomp::FileProcessor::compress()";
      }

      fileData.append(decompressedData.getData(),
                      decompressedData.getSize());
    }

    ASSERT_EQ(ceil(fileSize / (double) (chunkSize * 1024)), nChunks);

    fileList.push_back(fileProcessor.getCurrentFileName());

    // Integrity check
    std::string originalFileData;

    ASSERT_NO_THROW({
      originalFileData = autocomp::test::getDataFromFile(
          fileProcessor.getCurrentFileName()
        );
    });

    ASSERT_EQ(originalFileData.size(), fileData.size());
    ASSERT_TRUE(originalFileData == fileData);
  }

  std::sort(realFileList.begin(), realFileList.end());
  std::sort(fileList.begin(), fileList.end());

  ASSERT_EQ(realFileList, fileList);
}

TEST(FileProcessorTest, DoesNotPreparedUnexistingPath)
{
  std::shared_ptr<autocomp::io::PerformanceDataWriter> performanceDataWriter =
    std::make_shared<autocomp::io::PerformanceDataWriter>();  
  autocomp::FileProcessor fileProcessor(
      15000,
      std::make_shared<autocomp::RoundRobinCompressor>(performanceDataWriter)
    );

  ASSERT_THROW(
    {
      fileProcessor.preparePath("./this/path/is/not/supposed/to/exist");
    },
    autocomp::exceptions::IOError);
}

#endif //AC_FILE_PROCESSOR_TEST_H
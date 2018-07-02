#ifndef AC_PRE_COMPRESSING_FILE_PROCESSOR_TEST_H
#define AC_PRE_COMPRESSING_FILE_PROCESSOR_TEST_H

/* C++ System Headers */
#include <string>
#include <cstddef>
#include <stdexcept>
#include <memory>
#include <cmath>
#include <cstdio>

/* External headers */
#include "gtest/gtest.h"

/* Project headers */
#include "test_constants.hpp"
#include "common_functions.hpp"
#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "messaging/compressor.pb.h"
#include "compression/pre_compressing_file_processor.hpp"

class PreCompressingFileProcessorTest : public ::testing::Test
{
protected:

  std::string filename = autocomp::test::constants::fpcTestFilename;

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
     
  void TearDown()
  {
    this->removeCompressedFiles();
  }

  void removeCompressedFiles()
  {
    std::string extenssions[] = {".gz", ".lzop", ".lzma", ".bz2", ".fpc"};

    for (auto & prefix : this->realFileList) {
      for (auto & extenssion : extenssions) {
        std::string file = prefix + extenssion;
        ::remove(file.c_str());
      }
    }
  }

}; // class PreCompressingFileProcessorTest

TEST_F(PreCompressingFileProcessorTest, ProcessesSingleFile)
{
  autocomp::Compressor compressors[] = {autocomp::Compressor::ZLIB,
                                        autocomp::Compressor::LZO,
                                        autocomp::Compressor::BZIP2,
                                        autocomp::Compressor::LZMA,
                                        autocomp::Compressor::FPC,
                                        autocomp::Compressor::COPY};

  for (auto & compressor : compressors) {
    autocomp::PreCompressingFileProcessor fileProcessor;

    std::shared_ptr<autocomp::Buffer> data;

    try {
      data = std::make_shared<autocomp::Buffer>(fileProcessor.getChunkSize() * 
                                                  1024);
    } catch (std::bad_alloc & e) {
      FAIL() << e.what();
    }

    ASSERT_NO_THROW({
      fileProcessor.preparePath(this->filename);
    });

    int nFiles = 0;

    std::string compressedFileName("/tmp/AutoComp_compressedFile"),
                decompressedFileName("/tmp/AutoComp_decompressedFile");
    std::ofstream out(compressedFileName,
                      std::ofstream::out | std::ofstream::binary |
                      std::ofstream::trunc);

    if (out.fail()) {
      FAIL() << "Could not create the file " << compressedFileName;
    }

    fileProcessor.setCompressor(compressor);

    while(fileProcessor.hasNextFile()) {
      size_t fileSize;
      int nChunks = 0;

      ASSERT_NO_THROW({
        fileSize = fileProcessor.openNextFile();
      });

      std::string fileData("");
      autocomp::Compressor usedCompressor;

      while (fileProcessor.hasNextChunk()) {
        nChunks++;

        data->setData(std::string(""));

        ASSERT_NO_THROW({
          usedCompressor = fileProcessor.getNextChunk(*data);
        });

        ASSERT_EQ(compressor, usedCompressor);

        out.write(data->getData(), data->getSize());
      }

      out.close();
      nFiles++;

      // Decompressing file
      ASSERT_NO_THROW({
        fileProcessor.decompressFile(compressedFileName, decompressedFileName,
                                     usedCompressor);
      });

      if (usedCompressor == autocomp::COPY) {
        decompressedFileName = compressedFileName;
      }

      // Integrity check
      std::string originalFileData, decompressedFileData;

      ASSERT_NO_THROW({
        decompressedFileData = 
          autocomp::test::getDataFromFile(decompressedFileName);
      });

      ASSERT_NO_THROW({
        originalFileData = autocomp::test::getDataFromFile(
            fileProcessor.getCurrentFileName()
          );
      });

      ASSERT_EQ(originalFileData.size(), decompressedFileData.size());
      ASSERT_TRUE(originalFileData == decompressedFileData);
    }

    ASSERT_EQ(1, nFiles);
  }
}

TEST_F(PreCompressingFileProcessorTest, ProcessesDirectory)
{
  autocomp::Compressor compressors[] = {autocomp::Compressor::ZLIB,
                                        autocomp::Compressor::LZO,
                                        autocomp::Compressor::BZIP2,
                                        autocomp::Compressor::LZMA,
                                        //autocomp::Compressor::FPC,
                                        autocomp::Compressor::COPY};

  for (auto & compressor : compressors) {
    autocomp::PreCompressingFileProcessor fileProcessor;
    std::vector<std::string> fileList;

    ASSERT_NO_THROW({
      fileProcessor.preparePath(autocomp::test::constants::testDirectory);
    });

    fileProcessor.setCompressor(compressor);

    while(fileProcessor.hasNextFile()) {
      std::shared_ptr<autocomp::Buffer> data;

      try {
        data = std::make_shared<autocomp::Buffer>(fileProcessor.getChunkSize() * 
                                                    1024);
      } catch (std::bad_alloc & e) {
        FAIL() << e.what();
      }

      size_t fileSize;
      int nChunks = 0;

      ASSERT_NO_THROW({
        fileSize = fileProcessor.openNextFile();
      });

      std::string compressedFileName("/tmp/AutoComp_compressedFile"),
                  decompressedFileName("/tmp/AutoComp_decompressedFile");
      std::ofstream out(compressedFileName,
                        std::ofstream::out | std::ofstream::binary |
                        std::ofstream::trunc);
      if (out.fail()) {
        FAIL() << "Could not create the file " << compressedFileName;
      }

      std::string fileData("");
      autocomp::Compressor usedCompressor;

      while (fileProcessor.hasNextChunk()) {
        nChunks++;

        data->setData(std::string(""));

        ASSERT_NO_THROW({
          usedCompressor = fileProcessor.getNextChunk(*data);
        });

        ASSERT_EQ(compressor, usedCompressor);

        out.write(data->getData(), data->getSize());
      }

      out.close();
      fileList.push_back(fileProcessor.getCurrentFileName());

      // Decompressing file
      ASSERT_NO_THROW({
        fileProcessor.decompressFile(compressedFileName, decompressedFileName,
                                     usedCompressor);
      });

      if (usedCompressor == autocomp::COPY) {
        decompressedFileName = compressedFileName;
      }

      // Integrity check
      std::string originalFileData, decompressedFileData;

      ASSERT_NO_THROW({
        decompressedFileData = 
          autocomp::test::getDataFromFile(decompressedFileName);
      });

      ASSERT_NO_THROW({
        originalFileData = autocomp::test::getDataFromFile(
            fileProcessor.getCurrentFileName()
          );
      });

      ASSERT_EQ(originalFileData.size(), decompressedFileData.size());
      ASSERT_TRUE(originalFileData == decompressedFileData);

      this->removeCompressedFiles();
    }

    std::sort(this->realFileList.begin(), this->realFileList.end());
    std::sort(fileList.begin(), fileList.end());

    ASSERT_EQ(this->realFileList, fileList);
  }
}

TEST_F(PreCompressingFileProcessorTest, DoesNotPreparedUnexistingPath)
{
  autocomp::PreCompressingFileProcessor fileProcessor;

  ASSERT_THROW(
    {
      fileProcessor.preparePath("./this/path/is/not/supposed/to/exist");
    },
    autocomp::exceptions::IOError);
}

#endif //AC_PRE_COMPRESSING_FILE_PROCESSOR_TEST_H
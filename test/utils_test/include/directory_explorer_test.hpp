#ifndef AC_DIRECTORY_EXPLORER_TEST_HPP
#define AC_DIRECTORY_EXPLORER_TEST_HPP

/* C++ System Headers */
#include <string>
#include <stdexcept>
#include <vector>
#include <algorithm> 

/* External headers */
#include "gtest/gtest.h"

/* Project headers */
#include "test_constants.hpp"
#include "io/directory_explorer.hpp"

TEST(DirectoryExplorerTest, ThrowsWhenRootPathDoesNotExist)
{
  ASSERT_THROW({
      autocomp::DirectoryExplorer explorer("./this/path/is/not/supposed/"
                                           "to/exist");
  }, autocomp::exceptions::IOError);
}

TEST(DirectoryExplorerTest, GetsCorrectFileList)
{
  std::vector<std::string> realFileList{
    "test/test_files/alice29.txt",
    "test/test_files/test_dir/test_file2.txt",
    "test/test_files/test_dir/test_file3.xml",
    "test/test_files/test_dir/test_file.txt",
    "test/test_files/test_dir/test_subdir2/test_file.html",
    "test/test_files/test_dir/test_subdir2/test_subsubdir/test_file.txt",
    "test/test_files/test_dir/test_subdir/test_subfile.txt",
    "test/test_files/test.trace"
  };

  std::vector<std::string> fileList;

  autocomp::DirectoryExplorer * explorer;

  ASSERT_NO_THROW({
    explorer =
      new autocomp::DirectoryExplorer(autocomp::test::constants::testDirectory);
  });

  while(explorer->hasNextFile()) {
    fileList.push_back(explorer->getNextFileName());
  }

  std::sort(realFileList.begin(), realFileList.end());
  std::sort(fileList.begin(), fileList.end());

  ASSERT_EQ(realFileList, fileList);
}

TEST(DirectoryExplorerTest, GetsFile)
{
  autocomp::DirectoryExplorer * explorer;

  ASSERT_NO_THROW({
    explorer = new autocomp::DirectoryExplorer(autocomp
                                               ::test
                                               ::constants
                                               ::compressionTestFilename);
  });

  std::vector<std::string> fileList;

  while(explorer->hasNextFile()) {
    fileList.push_back(explorer->getNextFileName());
  }

  ASSERT_EQ(1, fileList.size());
  ASSERT_EQ(autocomp::test::constants::compressionTestFilename,
            fileList.front());
}

#endif //AC_DIRECTORY_EXPLORER_TEST_HPP
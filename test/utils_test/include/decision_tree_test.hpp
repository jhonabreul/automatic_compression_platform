#ifndef AC_DECISION_TREE_TEST_HPP
#define AC_DECISION_TREE_TEST_HPP

/* C++ System Headers */
#include <string>
#include <stdexcept>
#include <memory>

/* External headers */
#include "gtest/gtest.h"

/* Project headers */
#include "utils/decision_tree.hpp"
#include "utils/exceptions.hpp"
#include "test_constants.hpp"

TEST(DecisionTreeTest, FailsOnNonexistentFile)
{
  ASSERT_THROW(autocomp::DecisionTree("./file/not/supposed/to/exist.txt"),
               autocomp::exceptions::IOError);
}

TEST(DecisionTreeTest, FailsOnInvalidFile)
{
  ASSERT_THROW(
      autocomp::DecisionTree(autocomp::test::constants::invalidDecisionTreeFile),
      autocomp::exceptions::IOError
    );
}

TEST(DecisionTreeTest, SucessOnValidFile)
{
  ASSERT_NO_THROW(
      autocomp::DecisionTree(autocomp::test::constants::validDecisionTreeFile)
    );
}

TEST(DecisionTreeTest, Classfies)
{
  std::unique_ptr<autocomp::DecisionTree> decisionTree;

  ASSERT_NO_THROW(
  {
    decisionTree =
      std::unique_ptr<autocomp::DecisionTree>(
          new autocomp::DecisionTree(autocomp::test::constants::validDecisionTreeFile)
        );
  });

  std::vector<std::vector<int>> points{{0,6,14}, {0,6,16}, {0,6,19}, {0,7,0},
                                       {0,7,1}, {8,93,10}, {8,93,11}, {8,93,12},
                                       {8,93,13}, {8,93,15}, {8,94,2}, {8,94,3},
                                       {8,94,4}, {8,94,5}, {8,94,6}, {8,94,7},
                                       {8,94,8}, {8,94,9}, {8,94,10}, {8,94,11},
                                       {8,94,12}, {8,94,13}, {8,94,15}, {8,95,0},
                                       {8,95,2}, {8,95,3}, {8,95,4}, {14,56,1},
                                       {14,56,2}, {14,56,3}, {14,56,4}, {20,74,9},
                                       {20,75,13}, {20,75,16}, {0,7,1}, {0,7,2},
                                       {13,21,12}, {13,21,13}};

  std::vector<std::string> compressors{"zlib_6", "zlib_6", "zlib_6", "copy",
                                       "copy", "zlib_6", "zlib_6", "zlib_6",
                                       "zlib_6", "lzo_8", "lzo_8", "lzo_8",
                                       "bzip2_5", "bzip2_5", "bzip2_5",
                                       "bzip2_5", "lzo_8", "bzip2_5", "bzip2_5",
                                       "zlib_6", "zlib_6", "bzip2_5", "lzo_8",
                                       "lzo_8", "zlib_6", "zlib_6", "bzip2_5",
                                       "snappy", "snappy", "snappy", "snappy",
                                       "snappy", "snappy", "snappy", "copy",
                                       "copy", "copy", "copy"};

  for (int i = 0; i < points.size(); i++) {
    auto compressor = decisionTree->classify(points[i]);
    std::string compressorName(autocomp::Compressor_Name(compressor.first));

    std::transform(compressorName.begin(), compressorName.end(),
                   compressorName.begin(), ::tolower);

    if (compressor.second != -1) {
      compressorName.append("_").append(std::to_string(compressor.second));
    }

    EXPECT_EQ(compressorName, compressors[i]);
  }
}

#endif //AC_DECISION_TREE_TEST_HPP
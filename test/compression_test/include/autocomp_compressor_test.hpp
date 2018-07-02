#ifndef AC_AUTOCOMP_COMPRESSOR_TEST_HPP
#define AC_AUTOCOMP_COMPRESSOR_TEST_HPP

/* C++ System Headers */
#include <string>
#include <cstddef>
#include <stdexcept>
#include <vector>
#include <memory>

/* External headers */
#include "gtest/gtest.h"
#include "gmock/gmock.h"

/* Project headers */
#include "test_constants.hpp"
#include "common_functions.hpp"
#include "utils/buffer.hpp"
#include "utils/exceptions.hpp"
#include "utils/data_structures.hpp"
#include "utils/decision_tree.hpp"
#include "compression/autocomp_compressor.hpp"

namespace mock
{
  class TCPSocket : autocomp::net::TCPSocket
  {
  public:

    MOCK_CONST_METHOD0(getSendBufferCapacity, int());

    MOCK_CONST_METHOD0(getSendBufferSize, int());
  };
}

class AutoCompCompressorTest : public ::testing::Test
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
}; // class AutoCompCompressorTest

TEST_F(AutoCompCompressorTest, CopiesOnAlmostEmptySendBuffer)
{
  autocomp::ResourceState resourceState;
  std::shared_ptr<mock::TCPSocket> pseudoClientSocket =
    std::make_shared<mock::TCPSocket>();
  std::unique_ptr<autocomp::DecisionTree> decisionTree;

  ASSERT_NO_THROW(
  {
    decisionTree =
      std::unique_ptr<autocomp::DecisionTree>(
          new autocomp::DecisionTree(autocomp::test::constants::validDecisionTreeFile)
        );
  });

  EXPECT_CALL(*pseudoClientSocket, getSendBufferCapacity())
    .Times(1)
    .WillOnce(::testing::Return(1000));

  EXPECT_CALL(*pseudoClientSocket, getSendBufferSize())
    .Times(::testing::AtLeast(1))
    .WillRepeatedly(::testing::Return(0));

  autocomp::AutoCompCompressor<mock::TCPSocket> autocompCompressor(
      decisionTree.get(), &resourceState, pseudoClientSocket
    );
  
  int nTests = 200;

  for (int i = 0; i < nTests; i++) {
    autocomp::Compressor usedCompressor;

    ASSERT_NO_THROW({
      usedCompressor = autocompCompressor.compress(*originalBuffer,
                                                   *compressedBuffer);
    });    
    
    ASSERT_EQ(autocomp::COPY, usedCompressor);
  }
}

TEST_F(AutoCompCompressorTest, CompressesAndDecompresses)
{
  autocomp::ResourceState resourceState;
  std::shared_ptr<mock::TCPSocket> pseudoClientSocket =
    std::make_shared<mock::TCPSocket>();
  std::unique_ptr<autocomp::DecisionTree> decisionTree;

  ASSERT_NO_THROW(
  {
    decisionTree =
      std::unique_ptr<autocomp::DecisionTree>(
          new autocomp::DecisionTree(autocomp::test::constants::validDecisionTreeFile)
        );
  });

  EXPECT_CALL(*pseudoClientSocket, getSendBufferCapacity())
    .Times(1)
    .WillOnce(::testing::Return(1000));

  EXPECT_CALL(*pseudoClientSocket, getSendBufferSize())
    .Times(::testing::AtLeast(1))
    .WillRepeatedly(::testing::Return(1000));

  autocomp::AutoCompCompressor<mock::TCPSocket> autocompCompressor(
      decisionTree.get(), &resourceState, pseudoClientSocket
    );
  
  for (float cpuLoad = 0, bandwidth = 0.5; cpuLoad <= 100;
       cpuLoad += 5, bandwidth += 5) {
    resourceState.cpuLoad.store(cpuLoad);
    resourceState.bandwidth.store(bandwidth);

    autocomp::Compressor usedCompressor;

    ASSERT_NO_THROW({
      usedCompressor = autocompCompressor.compress(*originalBuffer,
                                                   *compressedBuffer);
    });
    
    switch (usedCompressor) {
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
                  "autocomp::AutoCompCompressor::compress()";
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

#endif //AC_AUTOCOMP_COMPRESSOR_TEST_HPP
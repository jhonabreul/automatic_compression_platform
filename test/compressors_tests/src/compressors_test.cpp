#include "gtest/gtest.h"

#include "zlib_test.hpp"
#include "snappy_test.hpp"
#include "lzo_test.hpp"
#include "bzip2_test.hpp"
#include "lzma_test.hpp"
#include "fpc_test.hpp"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
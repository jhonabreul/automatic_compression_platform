#include "gtest/gtest.h"

#include "zlib_compressor_test.hpp"
#include "snappy_compressor_test.hpp"
#include "lzo_compressor_test.hpp"
#include "bzip2_compressor_test.hpp"
#include "lzma_compressor_test.hpp"
#include "fpc_compressor_test.hpp"
#include "single_compressor_test.hpp"
#include "round_robin_compressor_test.hpp"
#include "training_compressor_test.hpp"
#include "file_processor_test.hpp"
#include "pre_compressing_file_processor_test.hpp"
#include "autocomp_compressor_test.hpp"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
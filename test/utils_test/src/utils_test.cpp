#include "gtest/gtest.h"

#include "buffer_test.hpp"
#include "directory_explorer_test.hpp"
#include "synchronous_queue_test.hpp"
#include "thread_pool_test.hpp"
#include "decision_tree_test.hpp"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
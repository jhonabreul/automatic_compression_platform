#include "gtest/gtest.h"
#include "addition_test.h"
#include "division_test.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
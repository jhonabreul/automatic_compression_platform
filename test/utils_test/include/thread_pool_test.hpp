#ifndef AC_THREAD_POOL_TEST_HPP
#define AC_THREAD_POOL_TEST_HPP

/* C++ System Headers */
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <memory>
#include <unistd.h>

/* External headers */
#include "gtest/gtest.h"

/* Project headers */
#include "utils/thread_pool.hpp"

class ThreadPoolTest : public ::testing::Test
{
protected:

  std::shared_ptr<autocomp::ThreadPool> threadPool;

  void SetUp()
  {
    this->threadPool = std::make_shared<autocomp::ThreadPool>(3);
    this->threadPool->init();
  }

  void TearDown()
  {
    this->threadPool->shutdown();
  }

protected:

  static void multiply(const int a, const int b, int & out)
  {
    out = a * b;
  }

}; // class ThreadPoolTest


TEST_F(ThreadPoolTest, SimpleTesting)
{
  std::vector<int> operands;

  for (int i = 1; i < 10; i++) {
    operands.push_back(i);
  }

  int resultsSize = operands.size() * operands.size();
  std::vector<int> results(resultsSize, 0);
  std::vector<std::future<void>> futures(resultsSize);

  for (int i = 0; i < operands.size(); i++) {
    for (int j = 0; j < operands.size(); j++) {
      int resultIndex = 9 * i + j;
      futures[resultIndex] =
        this->threadPool->run(ThreadPoolTest::multiply,
                              operands[i], operands[j],
                              std::ref(results[resultIndex]));
    }
  }

  for (auto & future : futures) {
    future.wait();
  }

  for (int i = 0; i < operands.size(); i++) {
    for (int j = 0; j < operands.size(); j++) {
      int resultIndex = 9 * i + j;
      int result = operands[i] * operands[j];
      ASSERT_EQ(result, results[resultIndex]);
    }
  }
}

#endif //AC_THREAD_POOL_TEST_HPP
#ifndef AC_SYNCHRONOUS_QUEUE_TEST_HPP
#define AC_SYNCHRONOUS_QUEUE_TEST_HPP

/* C++ System Headers */
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <set>

/* External headers */
#include "gtest/gtest.h"

/* Project headers */
#include "utils/synchronous_queue.hpp"

class SynchronousQueueTest : public ::testing::Test
{
protected:

  std::vector<int> ids;
  std::mutex idsMutex;
  std::condition_variable condition;
  autocomp::SynchronousQueue<int> queue;

  bool doneProducing = false;

public:

  void produce(int start, int end)
  {
    for (int i = start; i < end; i++) {
      queue.push(i);
      this->condition.notify_one();
    }
  }

  void consume()
  {
    bool done = false;
    int id;

    while(not done) {
      std::unique_lock<std::mutex> guard(this->idsMutex);
      this->condition.wait(guard,
                           [this]()
                            { 
                              return not this->queue.isEmpty() or
                                     this->doneProducing;
                            });

      if (this->queue.pop(id)) {
        this->ids.push_back(id);
      }

      done = this->doneProducing;
    }
  }

}; // class SynchronousQueueTest


TEST_F(SynchronousQueueTest, AccessTest)
{
  int nConsumers = 6, nProducers = 3;
  int nIdsPerProducer = 50;
  std::vector<std::thread> consumers;
  std::vector<std::thread> producers;

  for (int i = 0; i < nConsumers; i++) {
    consumers.emplace_back(&SynchronousQueueTest::consume, this);
  }

  for (int i = 0; i < nProducers; i++) {
    producers.emplace_back(&SynchronousQueueTest::produce, this,
                           i * nIdsPerProducer, (i + 1) * nIdsPerProducer);
  }

  for (auto & producer : producers) {
    producer.join();
  }

  this->doneProducing = true;
  this->condition.notify_all();

  for (auto & consumer : consumers) {
    consumer.join();
  }

  std::set<int> uniqueIds(ids.begin(), ids.end());
  ASSERT_EQ(ids.size(), uniqueIds.size());
}

#endif //AC_SYNCHRONOUS_QUEUE_TEST_HPP
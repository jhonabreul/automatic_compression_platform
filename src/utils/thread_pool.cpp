/**
 *  AutoComp Thread Pool
 *  thread_pool.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/27/2018
 */

#include "utils/thread_pool.hpp"

namespace autocomp {

ThreadPool::ThreadPool(const unsigned int & nThreads)
  : threads(nThreads),
    done(false)
{}

ThreadPool::~ThreadPool()
{
  this->shutdown();
}

void ThreadPool::init()
{
  for (auto & thread : this->threads) {
    thread = std::thread(&ThreadPool::worker, this);
  }
}

void ThreadPool::shutdown()
{
  { 
    std::unique_lock<std::mutex> guard(this->mutex);
    this->done = true;
  }

  this->condition.notify_all();
  
  for (auto & thread : this->threads) {
    if (thread.joinable()) {
      thread.join();
    }
  }
}

// Function that each thread runs, which dequeues a task and executes it
void ThreadPool::worker()
{
  VoidFunction task;
  bool dequeued;
  bool doneWorking = false;

  while (not doneWorking) {
    {
      std::unique_lock<std::mutex> guard(this->mutex);
      this->condition.wait(guard, [this]()
                                  { 
                                    return not this->taskQueue.isEmpty() or
                                           this->done;
                                  });
      
      dequeued = this->taskQueue.pop(task);
      doneWorking = this->taskQueue.isEmpty() and this->done;
    }

    if (dequeued) {
      task();
    }
  }
}

} // namespace autocomp
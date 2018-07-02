/**
 *  AutoComp Thread Pool
 *  thread_pool.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/27/2018
 */

#ifndef AC_THREAD_POOL_HPP
#define AC_THREAD_POOL_HPP

#include <thread>
#include <vector>
#include <mutex>
#include <future>
#include <functional> // std::function
#include <memory>

#include "utils/synchronous_queue.hpp"

namespace autocomp {

/**
 * A thread pool for avoiding creating new threads for certain operations
 */
class ThreadPool
{
  using VoidFunction = std::function<void()>;

  SynchronousQueue<VoidFunction> taskQueue;         //!< Task queue
  std::vector<std::thread> threads;   //!< Thread pool
  std::mutex mutex;                   //!< Mutex for task dequeuing
  std::condition_variable condition;  //!< Condition for task dequeuing
  bool done;                          //!< Shutdown flag

public:

  ThreadPool(const unsigned int & nThreads =
                std::thread::hardware_concurrency());

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool(ThreadPool &&) = delete;
  ThreadPool & operator=(const ThreadPool &) = delete;
  ThreadPool & operator=(ThreadPool &&) = delete;

  ~ThreadPool();

  void init();

  void shutdown();

private:

  /**
   * Function that each thread runs, which dequeues a task and executes it
   */
  void worker();

public:

  template<typename F, typename...Args>
  auto run(F && function, Args && ... arguments)
    -> std::future<decltype(function(arguments...))>
  {
    using ArgumentlessFunction = decltype(function(arguments...))();

    std::function<ArgumentlessFunction> task = 
      std::bind(std::forward<F>(function), std::forward<Args>(arguments)...);

    // Make shared ptr to be able to copy or assign it
    auto taskPtr =
      std::make_shared<std::packaged_task<ArgumentlessFunction>>(task);

    // Wrap task in generic void function
    VoidFunction voidFunctionWrapper = [taskPtr]() {
        (*taskPtr)(); 
      };

    // Enqueue generic wrapper function
    this->taskQueue.push(voidFunctionWrapper);

    // Wake up one thread if its waiting
    this->condition.notify_one();

    // Return future from promise
    return taskPtr->get_future();
  }
  
}; // class ThreadPool

} // namespace autocomp

#endif // AC_THREAD_POOL_HPP
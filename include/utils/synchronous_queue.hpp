/**
 *  AutoComp Synchronous Queue
 *  synchronous_queue.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/27/2018
 */

#ifndef AC_SYNCHRONOUS_QUEUE_HPP
#define AC_SYNCHRONOUS_QUEUE_HPP

#include <queue>
#include <mutex>

namespace autocomp {

/**
 * A synchronous queue for safe use in multi-threaded applications
 *
 * @tparam T type of message stored in the queue
 */
template <typename T>
class SynchronousQueue
{
  std::queue<T> queue;
  mutable std::mutex mutex;

public:

  SynchronousQueue() = default;
  SynchronousQueue(const SynchronousQueue &) = delete;
  SynchronousQueue(SynchronousQueue &&) = delete;
  SynchronousQueue & operator=(const SynchronousQueue &) = delete;
  SynchronousQueue & operator=(SynchronousQueue &&) = delete;

  bool isEmpty() const
  {
    std::unique_lock<std::mutex> guard(this->mutex);

    return this->queue.empty();
  }
  
  int getSize() const
  {
    std::unique_lock<std::mutex> guard(this->mutex);

    return this->queue.size();
  }

  void push(const T & entry)
  {
    std::unique_lock<std::mutex> guard(this->mutex);
    this->queue.push(entry);
  }

  void push(T && entry)
  {
    std::unique_lock<std::mutex> guard(this->mutex);
    this->queue.push(std::move(entry));
  }
  
  bool pop(T & entry)
  {
    std::unique_lock<std::mutex> guard(this->mutex);

    if (this->queue.empty()) {
      return false;
    }

    entry = std::move(this->queue.front());
    this->queue.pop();

    return true;
  }

  void clear()
  {
    std::unique_lock<std::mutex> guard(this->mutex);

    std::queue<T> tmpQueue;
    this->queue.swap(tmpQueue);
  }

}; // class SynchronousQueue

} // namespace autocomp

#endif // AC_SYNCHRONOUS_QUEUE_HPP
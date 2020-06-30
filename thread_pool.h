#ifndef THREAD_POOL_H__
#define THREAD_POOL_H__

#include <list>
#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace yc {
class ThreadPool {
public:
  struct TaskQueue {
    void Add(std::function<void()>);
    std::function<void()> Pop();
    std::list<std::function<void()>> tasks_;
  };

  ThreadPool(unsigned int nums = 0);
  ~ThreadPool();

  void AddTask(std::function<void()>);
  void Start();
  void Stop();
private:
  void DoWork();

  std::vector<std::thread> threads_;
  unsigned int thread_nums_;
  unsigned int free_nums_;
  std::atomic<bool> running_;
  TaskQueue task_queue_;
  unsigned int task_nums_ = 0;
  std::mutex mutex_;
  std::condition_variable cond_;
};
}

#endif

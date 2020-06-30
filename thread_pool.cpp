#include "thread_pool.h"

namespace {
constexpr unsigned int kMinThreadNum = 1;
constexpr unsigned int kMaxThreadNum = 128;
}

namespace yc {
void ThreadPool::TaskQueue::Add(std::function<void()> f) {
  tasks_.emplace_back(std::move(f));
}

std::function<void()> ThreadPool::TaskQueue::Pop() {
  if (tasks_.empty()) return nullptr;
  auto item = std::move(tasks_.front());
  tasks_.pop_front();
  return item;
}

bool ThreadPool::TaskQueue::Empty() {
  return tasks_.empty();
}

ThreadPool::ThreadPool(unsigned int nums)
  : thread_nums_(nums),
    running_(false) {
  thread_nums_ = thread_nums_ < kMinThreadNum ? kMinThreadNum : thread_nums_;
  thread_nums_ = thread_nums_ > kMaxThreadNum ? kMaxThreadNum : thread_nums_;
}

ThreadPool::~ThreadPool() {
  running_ = false;
  cond_.notify_all();
  for (auto && t : threads_) {
    if (t.joinable()) {
      t.join();
    }
  }
}

void ThreadPool::AddTask(std::function<void()> f) {
  std::unique_lock<std::mutex> lock(mutex_);
  task_queue_.Add(std::move(f));
  task_nums_++;
  cond_.notify_one();
}

void ThreadPool::Start() {
  if (running_) return;
  running_ = true;
  for (int i = 0; i < thread_nums_; ++i) {
    threads_.emplace_back(std::thread(&ThreadPool::DoWork, this));
  }
}

void ThreadPool::Stop() {
  running_ = false;
}

void ThreadPool::DoWork() {
  do {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      task = std::move(task_queue_.Pop());
      if (!task) {
        free_nums_++;
        cond_.wait(lock, [&]() {
          return !task_queue_.Empty();
        });
        free_nums_--;
        task = task_queue_.Pop();
      }
      task_nums_--;
    }
    task();
  } while (running_);
}
}

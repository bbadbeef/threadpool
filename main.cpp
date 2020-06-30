#include "thread_pool.h"

#include <iostream>
#include <chrono>
int main() {
  yc::ThreadPool tp(7);
  tp.Start();
  for (size_t i=0; i<50; ++i) {
    tp.AddTask([]() {
      //std::cout << "task------- " << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    });
  }
  return 0;
}
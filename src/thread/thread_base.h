#ifndef _THREAD_BASE_H_
#define _THREAD_BASE_H_

#include <mutex>
#include <thread>

class ThreadBase {
 public:
  ThreadBase();
  ~ThreadBase();

 public:
  void StartThread();
  void StopThread();
  virtual bool Process() = 0;

 private:
  void Run();

 private:
  std::unique_ptr<std::thread> thread_ = nullptr;
  bool start_ = false;
  std::mutex mutex_;
};

#endif
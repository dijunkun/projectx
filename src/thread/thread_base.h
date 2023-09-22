#ifndef _THREAD_BASE_H_
#define _THREAD_BASE_H_

#include <atomic>
#include <thread>

class ThreadBase {
 public:
  ThreadBase();
  ~ThreadBase();

 public:
  void Start();
  void Stop();

  void Pause();
  void Resume();

  virtual bool Process() = 0;

 private:
  void Run();

 private:
  std::unique_ptr<std::thread> thread_ = nullptr;

  std::atomic<bool> stop_{false};
  std::atomic<bool> pause_{false};
};

#endif
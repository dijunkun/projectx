#include "thread_base.h"

#include "log.h"

ThreadBase::ThreadBase() {}

ThreadBase::~ThreadBase() {}

void ThreadBase::StartThread() {
  if (!thread_) {
    thread_ = std::make_unique<std::thread>(&ThreadBase::Run, this);
  }
}

void ThreadBase::StopThread() {
  if (thread_ && thread_->joinable()) {
    thread_->join();
  }
}

void ThreadBase::Run() {
  while (Process()) {
  }
}
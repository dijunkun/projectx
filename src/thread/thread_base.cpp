#include "thread_base.h"

#include "log.h"

ThreadBase::ThreadBase() {}

ThreadBase::~ThreadBase() {}

void ThreadBase::Start() {
  if (!thread_) {
    thread_ = std::make_unique<std::thread>(&ThreadBase::Run, this);
  }

  stop_ = false;
}

void ThreadBase::Stop() {
  stop_ = true;

  if (thread_ && thread_->joinable()) {
    thread_->join();
  }
}

void ThreadBase::Pause() { pause_ = true; }

void ThreadBase::Resume() { pause_ = false; }

void ThreadBase::Run() {
  while (!stop_ && Process()) {
  }
}
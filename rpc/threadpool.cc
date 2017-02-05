// Copyright 2017 <codefever@github.com>
#include "rpc/threadpool.h"

#include <utility>

#include <glog/logging.h>

ThreadPool::ThreadPool(int num_threads)
    : stopped_(false) {
  CHECK_GT(num_threads, 0);
  for (int i = 0; i < num_threads; ++i) {
    threads_.emplace_back(boost::thread(std::bind(&ThreadPool::Loop, this)));
  }
}

ThreadPool::~ThreadPool() {
  Stop();
}

bool ThreadPool::Schedule(WorkFunc work) {
  if (stopped_.load(std::memory_order_relaxed)) {
    return false;
  }
  boost::lock_guard<boost::mutex> lock(mutex_);
  works_.push_back(work);
  cv_.notify_one();
  return true;
}

void ThreadPool::Stop() {
  if (stopped_.load(std::memory_order_relaxed)) {
    return;
  }

  stopped_.store(true, std::memory_order_relaxed);
  mutex_.lock();
  cv_.notify_all();
  mutex_.unlock();
  for (auto& t : threads_) {
    t.join();
  }
  threads_.clear();
}

void ThreadPool::Loop() {
  VLOG(2) << "theadpool start..";
  bool continued = true;
  while (continued) {
    WorkFunc new_work;
    boost::unique_lock<boost::mutex> lock(mutex_);
    while (works_.empty() && !stopped_.load(std::memory_order_relaxed)) {
      cv_.wait(lock);
    }
    if (!works_.empty()) {
      new_work = std::move(works_.front());
      works_.pop_front();
    }
    continued = !works_.empty() || !stopped_.load(std::memory_order_relaxed);
    lock.unlock();

    // run a job
    if (new_work) {
      new_work();
    }
  }
}

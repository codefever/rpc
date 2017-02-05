// Copyright 2017 <codefever@github.com>
#pragma once

#include <atomic>
#include <deque>
#include <functional>
#include <vector>

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

class ThreadPool {
 public:
  typedef std::function<void()> WorkFunc;
  explicit ThreadPool(int num_threads);
  ~ThreadPool();
  bool Schedule(WorkFunc work);
  void Stop();

 private:
  void Loop();

 private:
  boost::mutex mutex_;
  boost::condition_variable cv_;
  std::deque<WorkFunc> works_;
  std::vector<boost::thread> threads_;
  std::atomic<bool> stopped_;
};

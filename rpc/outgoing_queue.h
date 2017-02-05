// Copyright 2017 <codefever@github.com>
#pragma once

#include <deque>

#include <boost/thread/mutex.hpp>

template<class MsgType>
class OutgoingQueue {
 public:
  OutgoingQueue() {}
  ~OutgoingQueue() {
    boost::mutex::scoped_lock lock(mutex_);
    while (!q_.empty()) {
      delete q_.front();
      q_.pop_front();
    }
  }

  MsgType* GetNext() {
    boost::mutex::scoped_lock lock(mutex_);
    return q_.empty() ? nullptr : q_.front();
  }

  MsgType* PopAndGetNext() {
    boost::mutex::scoped_lock lock(mutex_);
    delete q_.front();
    q_.pop_front();
    return q_.empty() ? nullptr : q_.front();
  }
  MsgType* PushAndGetPrev(MsgType* e) {
    boost::mutex::scoped_lock lock(mutex_);
    MsgType* prev = q_.empty() ? nullptr : q_.back();
    q_.push_back(e);
    return prev;
  }

 private:
  boost::mutex mutex_;
  std::deque<MsgType*> q_;
};

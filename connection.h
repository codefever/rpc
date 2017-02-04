// Copyright 2017 <codefever@github.com>
#pragma once

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <utility>

#include <boost/asio.hpp>

#include "dispatcher.h"
#include "pack.h"
#include "service_map.h"

class Connection : public std::enable_shared_from_this<Connection> {
 public:
  Connection(boost::asio::io_service& io_service,  // NOLINT(runtime/references)
             DispatcherCall dispatcher);
  ~Connection();

  boost::asio::ip::tcp::socket& socket() {
    return socket_;
  }

  void Serve();
  void ServeResponses();

 private:
  void DoRecv();
  void DoWrite();
  void DoClose();

  void AddFeedback(RawMessage* msg);
  RawMessage* PeakFeedback();
  bool PopAndCheckIfAnyFeedback();

  void WriteEncodedData(
      std::shared_ptr<RawMessageEncoder::EncodedData> encoded_data);

 private:
  boost::asio::ip::tcp::socket socket_;
  // boost::asio::deadline_timer timer_;  // TODO(who): timeout check
  DispatcherCall dispatcher_;

  std::shared_ptr<RawMessageDecoder> decoder_;
  std::shared_ptr<RawMessageEncoder> encoder_;

  std::atomic<int> pending_requests_;

  // lock-free?
  std::mutex mutex_;
  std::deque<RawMessage*> feedback_;

 private:
  Connection(const Connection&) = delete;
  void operator= (const Connection&) = delete;
};

// Copyright 2017 <codefever@github.com>
#pragma once

#include <atomic>
#include <deque>
#include <memory>
#include <utility>

#include <boost/asio.hpp>

#include "rpc/dispatcher.h"
#include "rpc/outgoing_queue.h"
#include "rpc/pack.h"
#include "rpc/service_map.h"

class Connection : public std::enable_shared_from_this<Connection> {
 public:
  Connection(boost::asio::io_service& io_service,  // NOLINT(runtime/references)
             DispatcherCall dispatcher);
  ~Connection();

  boost::asio::ip::tcp::socket& socket() {
    return socket_;
  }

  void Serve();

 private:
  void DoRecv();
  void DoWrite();
  void DoClose();

  void WriteEncodedData(
      std::shared_ptr<RawMessageEncoder::EncodedData> encoded_data);

 private:
  boost::asio::ip::tcp::socket socket_;
  // boost::asio::deadline_timer timer_;  // TODO(who): timeout check
  DispatcherCall dispatcher_;

  std::shared_ptr<RawMessageDecoder> decoder_;
  std::shared_ptr<RawMessageEncoder> encoder_;

  // std::atomic<int> pending_requests_;
  OutgoingQueue<RawMessage> outgoing_;

 private:
  Connection(const Connection&) = delete;
  void operator= (const Connection&) = delete;
};

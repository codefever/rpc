// Copyright 2017 <codefever@github.com>
#pragma once

#include <atomic>
#include <memory>
#include <utility>

#include <boost/asio.hpp>

#include "buffer.h"
#include "service_map.h"

class Connection : public std::enable_shared_from_this<Connection> {
 public:
  Connection(boost::asio::io_service& io_service,  // NOLINT(runtime/references)
             const ServiceMap* service_map);

  boost::asio::ip::tcp::socket& socket() {
    return socket_;
  }

  void Serve();

 private:
  void DoRecv();
  void DoWrite();

 private:
  boost::asio::ip::tcp::socket socket_;
  boost::asio::deadline_timer timer_;  // TODO(who): timeout check
  const ServiceMap* service_map_;
  Buffer recv_buffer_;

  std::atomic<int> pending_requests_;

 private:
  Connection(const Connection&) = delete;
  void operator= (const Connection&) = delete;
};

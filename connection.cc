#include "connection.h"

#include <glog/logging.h>

Connection::Connection(boost::asio::io_service& io_service,
                       const ServiceMap* service_map)
    : socket_(io_service),
      timer_(io_service),
      service_map_(service_map) {}

void Connection::DoRecv() {
  auto self(shared_from_this());
  auto buffer = recv_buffer_.GetWritableBuffer();
  socket_.async_read_some(
      boost::asio::buffer(buffer.first, buffer.second),
      [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
        if (!ec) {
          recv_buffer_.Commit(bytes_transferred);

          self->DoRecv();
        } else if (ec != boost::asio::error::eof) {
          LOG(WARNING) << "recv fail: " << ec.message();
        }
      });
}

void Connection::DoWrite() {
  auto self(shared_from_this());
}

void Connection::Serve() {
  DoRecv();
}
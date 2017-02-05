// Copyright 2017 <codefever@github.com>
#include "rpc/connection.h"

#include <glog/logging.h>

class SimpleRespondor : public Respondor {
 public:
  void SetDone(std::function<void(RawMessage* response)> done) {
    done_ = done;
  }

  void Finish(RawMessage* response) {
    done_(response);
  }

 private:
  std::function<void(RawMessage* response)> done_;
};

Connection::Connection(boost::asio::io_service& io_service,
                       DispatcherCall dispatcher)
    : socket_(io_service),
      // timer_(io_service),
      dispatcher_(dispatcher),
      decoder_(new SimpleMessageDecoder),
      encoder_(new SimpleMessageEncoder) {}

Connection::~Connection() {
  VLOG(1) << "connection[" << this << "] destroyed.";
}

void Connection::DoRecv() {
  auto self(shared_from_this());
  char* data = nullptr;
  size_t size = 0;
  RawMessageDecoder::DecodeCallback decode_func;
  decoder_->Next(&data, &size, &decode_func);

  socket_.async_read_some(
      boost::asio::buffer(data, size),
      [this, self, decode_func](boost::system::error_code ec,
                   std::size_t bytes_transferred) {
        if (!ec) {
          if (!decode_func(bytes_transferred)) {
            LOG(WARNING) << "decode fail!";
            DoClose();
            return;
          }

          while (auto* req = decoder_->PopDecodedMessage()) {
            std::shared_ptr<RawMessage> request(req);
            std::shared_ptr<SimpleRespondor> respondor(new SimpleRespondor);
            respondor->SetDone([this, self](RawMessage* response) {
              if (outgoing_.PushAndGetPrev(response) == nullptr) {
                ServeResponses();
              }
            });

            // schedule
            dispatcher_(request,
                        std::dynamic_pointer_cast<Respondor>(respondor));
          }

          self->DoRecv();
        } else if (ec != boost::asio::error::eof &&
                   ec != boost::asio::error::operation_aborted) {
          LOG(WARNING) << "recv fail: " << ec.message();
          DoClose();
        }
      });
}

void Connection::WriteEncodedData(
      std::shared_ptr<RawMessageEncoder::EncodedData> encoded_data) {
  auto self(shared_from_this());
  const char* data = nullptr;
  size_t size = 0;
  if (encoded_data->Next(&data, &size)) {
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(data, size),
        [this, self, encoded_data](boost::system::error_code ec,
                     std::size_t bytes_transferred) {
          if (!ec) {
            WriteEncodedData(encoded_data);
          } else if (ec != boost::asio::error::operation_aborted) {
            LOG(WARNING) << "write fail: " << ec.message();
            DoClose();
          }
        });
  } else {
    if (encoded_data->Error()) {
      LOG(WARNING) << "encode fail!";
      DoClose();
    } else {
      RawMessage* next = outgoing_.PopAndGetNext();
      if (next) {
        std::shared_ptr<RawMessageEncoder::EncodedData> next_data(
            encoder_->Encode(next));
        WriteEncodedData(next_data);
      }
    }
  }
}

void Connection::DoWrite() {
  auto self(shared_from_this());
  RawMessage* response = outgoing_.GetNext();
  if (!response) {
    return;
  }

  std::shared_ptr<RawMessageEncoder::EncodedData> encoded_data(
      encoder_->Encode(response));
  WriteEncodedData(encoded_data);
}

void Connection::Serve() {
  // socket_.get_io_service().dispatch(std::bind(&Connection::DoRecv, this));
  VLOG(1) << "connection[" << this << "] attached.";
  DoRecv();
}

void Connection::ServeResponses() {
  auto self(shared_from_this());
  socket_.get_io_service().dispatch([this, self]() {
    if (!socket().is_open()) {
      return;
    }
    self->DoWrite();
  });
}

void Connection::DoClose() {
  boost::system::error_code ignored;
  socket_.close(ignored);
}

// Copyright 2017 <codefever@github.com>
#include "rpc/pack.h"

#include <stdint.h>

#include <glog/logging.h>

struct SimpleHeader {
  uint32_t len;
  uint32_t seq_no;
  uint16_t code;
  uint16_t type;
  uint16_t sid;
  uint16_t mid;
} __attribute__((packed));

SimpleMessageDecoder::SimpleMessageDecoder()
    : state_(State::RECV_HEAD),
      header_(new SimpleHeader()),
      bytes_read_(0),
      curr_(nullptr) {}

SimpleMessageDecoder::~SimpleMessageDecoder() {
  delete header_;
  delete curr_;

  while (!decoded_.empty()) {
    delete decoded_.front();
    decoded_.pop_front();
  }
}

void SimpleMessageDecoder::Next(char** data, size_t* size,
                                DecodeCallback* callback) {
  switch (state_) {
  case State::RECV_HEAD: {
      CHECK_LT(bytes_read_, sizeof(*header_));
      *data = reinterpret_cast<char*>(header_) + bytes_read_;
      *size = sizeof(*header_) - bytes_read_;
    }
    break;
  case State::RECV_PAYLOAD: {
      CHECK_LT(bytes_read_, header_->len);
      *data = const_cast<char*>(curr_->mutable_payload()->data()) + bytes_read_;
      *size = curr_->mutable_payload()->size() - bytes_read_;
    }
    break;
  }
  *callback = std::bind(&SimpleMessageDecoder::TryDecode, this,
                        std::placeholders::_1);
}

RawMessage* SimpleMessageDecoder::PopDecodedMessage() {
  RawMessage* ret = nullptr;
  if (!decoded_.empty()) {
    ret = decoded_.front();
    decoded_.pop_front();
  }
  return ret;
}

bool SimpleMessageDecoder::TryDecode(size_t size) {
  switch (state_) {
  case State::RECV_HEAD: {
      if (bytes_read_ + size >= sizeof(*header_)) {
        CHECK_EQ(bytes_read_ + size, sizeof(*header_));
        VLOG(3) << "decode head, len=[" << header_->len
                << "], seq_no=[" << header_->seq_no
                << "], sid=[" << header_->sid
                << "], mid=[" << header_->mid
                << "], code=[" << header_->code << "]";

        curr_ = new RawMessage;
        curr_->set_sid(static_cast<int>(header_->sid));
        curr_->set_mid(static_cast<int>(header_->mid));
        curr_->set_seq_no(header_->seq_no);
        curr_->set_error_code(static_cast<rpc::RpcErrorCode>(header_->code));
        curr_->set_compress_type(static_cast<rpc::RpcCompressType>(header_->type));  // NOLINT
        curr_->mutable_payload()->resize(header_->len);
        bytes_read_ = 0;

        state_ = State::RECV_PAYLOAD;

        // go ahead...
        return TryDecode(0);
      } else {
        bytes_read_ += size;
      }
    }
    break;
  case State::RECV_PAYLOAD: {
      if (bytes_read_ + size >= header_->len) {
        CHECK_EQ(bytes_read_ + size, header_->len);
        decoded_.push_back(curr_);
        curr_ = nullptr;
        bytes_read_ = 0;
        state_ = State::RECV_HEAD;
      } else {
        bytes_read_ += size;
      }
    }
    break;
  }
  return true;
}

class SimpleEncodedData : public RawMessageEncoder::EncodedData {
 public:
  explicit SimpleEncodedData(const RawMessage* msg)
      : msg_(msg), state_(State::SEND_HEAD) {
    header_.len = msg->payload().size();
    header_.seq_no = msg->seq_no();
    header_.code = msg->error_code();
    header_.type = msg->compress_type();
    header_.sid = msg->sid();
    header_.mid = msg->mid();
  }

  ~SimpleEncodedData() override {}

  bool Next(const char** data, size_t* size) override {
    switch (state_) {
    case State::SEND_HEAD: {
        *data = reinterpret_cast<char*>(&header_);
        *size = sizeof(header_);
        if (msg_->payload().size() > 0) {
          state_ = State::SEND_PAYLOAD;
        } else {
          state_ = State::FINISH;
        }
      }
      break;
    case State::SEND_PAYLOAD: {
        *data = const_cast<char*>(msg_->payload().data());
        *size = msg_->payload().size();
        state_ = State::FINISH;
      }
      break;
    case State::FINISH:
    case State::ERROR: {
        return false;
      }
    }
    return true;
  }

  bool Error() override {
    return state_ == State::ERROR;
  }

 private:
  SimpleHeader header_;
  const RawMessage* msg_;

  enum class State {
    SEND_HEAD,
    SEND_PAYLOAD,
    FINISH,
    ERROR,
  };
  State state_;
};

RawMessageEncoder::EncodedData* SimpleMessageEncoder::Encode(
    const RawMessage* msg) {
  return new SimpleEncodedData(msg);
}

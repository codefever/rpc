// Copyright 2017 <codefever@github.com>
#pragma once

#include <functional>
#include <deque>
#include <string>

#include "rpc/rpc_codes.pb.h"

class RawMessage {
 public:
  RawMessage() {
    Reset();
  }

  inline void Reset() {
    sid_ = mid_ = -1;
    seq_no_ = 0;
    error_code_ = rpc::RpcErrorCode::OK;
    compress_type_ = rpc::RpcCompressType::NONE;
  }

  int sid() const {
    return sid_;
  }
  void set_sid(int sid) {
    sid_ = sid;
  }

  int mid() const {
    return mid_;
  }
  void set_mid(int mid) {
    mid_ = mid;
  }

  uint32_t seq_no() const {
    return seq_no_;
  }
  void set_seq_no(uint32_t seq_no) {
    seq_no_ = seq_no;
  }

  rpc::RpcErrorCode error_code() const {
    return error_code_;
  }
  void set_error_code(rpc::RpcErrorCode error_code) {
    error_code_ = error_code;
  }

  rpc::RpcCompressType compress_type() const {
    return compress_type_;
  }
  void set_compress_type(rpc::RpcCompressType compress_type) {
    compress_type_ = compress_type;
  }

  const std::string& payload() const {
    return payload_;
  }
  std::string* mutable_payload() {
    return &payload_;
  }

 private:
  int sid_;
  int mid_;
  uint32_t seq_no_;
  rpc::RpcErrorCode error_code_;
  rpc::RpcCompressType compress_type_;
  std::string payload_;
};

class RawMessageDecoder {
 public:
  RawMessageDecoder() {}
  virtual ~RawMessageDecoder() {}

  // Tell the decoder how many bytes is filled in buffer.
  typedef std::function<bool(size_t)> DecodeCallback;
  virtual void Next(char** data, size_t* size, DecodeCallback* callback) = 0;

  // Return decoded messages if avaible
  virtual RawMessage* PopDecodedMessage() = 0;
};

class RawMessageEncoder {
 public:
  RawMessageEncoder() {}
  virtual ~RawMessageEncoder() {}

  class EncodedData {
   public:
    EncodedData() {}
    virtual ~EncodedData() {}

    // Returns false if eof or fails
    virtual bool Next(const char** data, size_t* size) = 0;

    virtual bool Error() = 0;
  };

  virtual EncodedData* Encode(const RawMessage* msg) = 0;
};

class Respondor {
 public:
  Respondor() {}
  virtual ~Respondor() {}
  virtual void Finish(RawMessage* response) = 0;
};


// Implementations
struct SimpleHeader;

class SimpleMessageDecoder : public RawMessageDecoder {
 public:
  SimpleMessageDecoder();
  ~SimpleMessageDecoder() override;
  void Next(char** data, size_t* size, DecodeCallback* callback) override;
  RawMessage* PopDecodedMessage() override;

 private:
  bool TryDecode(size_t size);

 private:
  enum class State : int {
    RECV_HEAD = 0,
    RECV_PAYLOAD
  };
  State state_;

  SimpleHeader* header_;
  size_t bytes_read_;

  std::deque<RawMessage*> decoded_;
  RawMessage* curr_;
};

class SimpleMessageEncoder : public RawMessageEncoder {
 public:
  RawMessageEncoder::EncodedData* Encode(const RawMessage* msg) override;
};

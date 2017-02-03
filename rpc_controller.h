// Copyright 2017 <codefever@github.com>
#pragma once

#include <string>

#include <google/protobuf/service.h>

#include "rpc_codes.pb.h"

using rpc::RpcErrorCode;

class RpcController : public google::protobuf::RpcController {
 public:
  RpcController() : error_code_(RpcErrorCode::OK) {}
  virtual ~RpcController() {}

  void Reset() override {
    error_code_ = RpcErrorCode::OK;
  }

  bool Failed() const override {
    return error_code_ != RpcErrorCode::OK;
  }

  std::string ErrorText() const override;
  void SetFailed(const std::string& reason) override;

  // not supported yet.
  void StartCancel() override {}
  bool IsCanceled() const override {
    return false;
  }
  void NotifyOnCancel(google::protobuf::Closure* callback) override {}

  // generic error codes
 public:
  void SetFailed(RpcErrorCode ec) {
    error_code_ = ec;
  }
  RpcErrorCode ErrorCode() const {
    return error_code_;
  }

 private:
  RpcErrorCode error_code_;
};

// Copyright 2017 <codefever@github.com>
#include "rpc_controller.h"

std::string RpcController::ErrorText() const {
  return rpc::RpcErrorCode_Name(error_code_);
}

void RpcController::SetFailed(const std::string& reason) {
  if (!RpcErrorCode_Parse(reason, &error_code_)) {
    error_code_ = RpcErrorCode::UNDEFINED_ERROR;
  }
}

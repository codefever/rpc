#pragma once

#include <memory>

#include <google/protobuf/service.h>

#include "rpc_controller.h"

struct Context {
  RpcController controller;
  google::protobuf::Service* service;
  const google::protobuf::MethodDescriptor* method;
  std::shared_ptr<google::protobuf::Message> request;
  std::shared_ptr<google::protobuf::Response> response;
  google::protobuf::Closure* done;
  inline Context() : service(nullptr), method(nullptr), done(nullptr) {}
};

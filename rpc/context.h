// Copyright 2017 <codefever@github.com>
#pragma once

#include <memory>

#include <google/protobuf/service.h>

#include "rpc/rpc_controller.h"

struct Context {
  RpcController controller;
  google::protobuf::Service* service;
  const google::protobuf::MethodDescriptor* method;
  std::shared_ptr<google::protobuf::Message> request;
  std::shared_ptr<google::protobuf::Message> response;
  google::protobuf::Closure* done;
  inline Context() : service(nullptr), method(nullptr), done(nullptr) {}

  inline void Invoke(google::protobuf::Closure* done) {
    service->CallMethod(method, &controller,
                        request.get(), response.get(),
                        done);
  }
};

#pragma once

#include <stdint.h>
#include <map>
#include <utility>

#include <google/protobuf/service.h>

struct ServiceEntry {
  google::protobuf::Service* service;
  const google::protobuf::MethodDescriptor* method;
  const google::protobuf::Message* request_prototype;
  const google::protobuf::Message* response_prototype;
};

typedef std::pair<int/*sid*/, int/*mid*/> ServiceIdentity;

typedef std::map<ServiceIdentity, ServiceEntry> ServiceMap;
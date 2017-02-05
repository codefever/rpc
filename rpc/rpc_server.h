// Copyright 2017 <codefever@github.com>
#pragma once

#include <memory>

#include <boost/asio.hpp>
#include <google/protobuf/service.h>

#include "rpc/pack.h"
#include "rpc/service_map.h"
#include "rpc/threadpool.h"

class RpcServer {
 public:
  ~RpcServer();
  void Serve();
  bool Died() const {
    return died_;
  }

  class Builder {
   public:
    Builder();
    Builder& AddService(google::protobuf::Service* service);
    Builder& Listen(const boost::asio::ip::tcp::endpoint& endpoint);
    Builder& UseWorkers(int num_workers);
    RpcServer* Build();

   private:
    ServiceMap service_map_;
    boost::asio::ip::tcp::endpoint endpoint_;
    int num_workers_;
    Builder(const Builder&) = delete;
    void operator= (const Builder&) = delete;
  };

 private:
  RpcServer(ServiceMap&& service_map,
            boost::asio::ip::tcp::endpoint endpoint,
            int num_workers);
  void DoAccept();
  void DoAwaitStop();
  void DoDispatch(std::shared_ptr<RawMessage> request,
                  std::shared_ptr<Respondor> respondor);

 private:
  const ServiceMap service_map_;
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
  boost::asio::signal_set signals_;
  bool died_;

  std::shared_ptr<ThreadPool> workers_;

 private:
  RpcServer(const RpcServer&) = delete;
  void operator= (const RpcServer&) = delete;
};

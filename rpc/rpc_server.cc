// Copyright 2017 <codefever@github.com>
#include "rpc/rpc_server.h"

#include <memory>
#include <utility>

#include <glog/logging.h>
#include <google/protobuf/descriptor.pb.h>

#include "rpc/connection.h"
#include "rpc/context.h"
#include "rpc/rpc_options.pb.h"

namespace {

class StdFunctionalClosure : public google::protobuf::Closure {
 public:
  explicit StdFunctionalClosure(std::function<void()> f) : f_(f) {}
  ~StdFunctionalClosure() override {}

  void Run() override {
    f_();
    delete this;
  }

 private:
  std::function<void()> f_;
};

google::protobuf::Closure* NewCallback(std::function<void()> f) {
  return new StdFunctionalClosure(f);
}

}  // namespace

RpcServer::Builder::Builder()
    : endpoint_(boost::asio::ip::tcp::v4(), 8080) {}

RpcServer::Builder& RpcServer::Builder::AddService(
    google::protobuf::Service* service) {
  const google::protobuf::ServiceDescriptor* service_descriptor =
      service->GetDescriptor();
  CHECK(service_descriptor->options().HasExtension(rpc::sid))
      << "service[" << service_descriptor->full_name()
      << "] must have sid extension";
  const int kSid = service_descriptor->options().GetExtension(rpc::sid);
  for (int i = 0; i < service_descriptor->method_count(); ++i) {
    auto method = service_descriptor->method(i);
    CHECK(method->options().HasExtension(rpc::mid))
        << "method[" << method->full_name() << "] must have mid extension";;
    const int kMid = method->options().GetExtension(rpc::mid);
    ServiceIdentity id = std::make_pair(kSid, kMid);
    ServiceEntry entry = {
        .service = service,
        .method = method,
        .request_prototype = &service->GetRequestPrototype(method),
        .response_prototype = &service->GetResponsePrototype(method)
    };
    auto ins = service_map_.insert(std::make_pair(id, entry));
    CHECK(ins.second) << "duplicated method found: " << method->full_name()
                      << ", sid=[" << kSid << "], mid=[" << kMid << "]";
  }
  return *this;
}

RpcServer::Builder& RpcServer::Builder::Listen(
    const boost::asio::ip::tcp::endpoint& endpoint) {
  endpoint_ = endpoint;
  return *this;
}

RpcServer* RpcServer::Builder::Build() {
  if (service_map_.empty()) {
    LOG(ERROR) << "no services registered, build fail.";
    return nullptr;
  }
  return new RpcServer(std::move(service_map_), endpoint_);
}

RpcServer::RpcServer(ServiceMap&& service_map,
                     boost::asio::ip::tcp::endpoint endpoint)
    : service_map_(std::move(service_map)),
      io_service_(),
      acceptor_(io_service_),
      signals_(io_service_),
      died_(false) {
  signals_.add(SIGINT);
  signals_.add(SIGTERM);

  try {
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
  } catch (boost::system::system_error& e) {
    LOG(ERROR) << "listen fail: " << e.what();
    died_ = true;
  }
}

RpcServer::~RpcServer() {}

void RpcServer::Serve() {
  if (Died()) {
    LOG(ERROR) << "cannot launch a dead server";
    return;
  }

  DoAwaitStop();
  DoAccept();

  boost::system::error_code ignored_ec;
  io_service_.run(ignored_ec);
}

void RpcServer::DoAccept() {
  auto incoming = std::make_shared<Connection>(
      io_service_,
      std::bind(&RpcServer::DoDispatch,
                this,
                std::placeholders::_1,
                std::placeholders::_2));

  acceptor_.async_accept(incoming->socket(),
    [this, incoming](boost::system::error_code ec) {
      if (!acceptor_.is_open()) {
        return;
      }
      if (!ec) {
        incoming->Serve();
      } else {
        LOG(ERROR) << "accept fail: " << ec.message();
      }
      DoAccept();
    });
}

void RpcServer::DoAwaitStop() {
  signals_.async_wait(
    [this](boost::system::error_code /*ec*/, int /*signo*/) {
      acceptor_.close();
      io_service_.stop();
    });
}

void RpcServer::DoDispatch(std::shared_ptr<RawMessage> request,
                           std::shared_ptr<Respondor> respondor) {
  RawMessage* response = new RawMessage();

  auto it = service_map_.find(std::make_pair(request->sid(), request->mid()));
  if (it == service_map_.end()) {
    response->set_error_code(rpc::RpcErrorCode::UNSUPPORTED_METHODS);
    respondor->Finish(response);
    return;
  }

  auto& entry = it->second;
  // direct run
  auto ctx = std::make_shared<Context>();
  ctx->service = entry.service;
  ctx->method = entry.method;
  ctx->request.reset(entry.request_prototype->New());
  ctx->response.reset(entry.response_prototype->New());

  if (!ctx->request->ParseFromString(request->payload())) {
    response->set_error_code(rpc::RpcErrorCode::PARSE_ERROR);
    respondor->Finish(response);
    return;
  }

  // fill response and detach request
  response->set_seq_no(request->seq_no());
  request.reset();

  // invoke
  ctx->Invoke(NewCallback([ctx, respondor, response]() {
    if (ctx->controller.Failed()) {
      response->set_error_code(ctx->controller.ErrorCode());
    } else if (!ctx->response->SerializeToString(response->mutable_payload())) {
      response->set_error_code(rpc::RpcErrorCode::SERIALIZE_ERROR);
    }
    respondor->Finish(response);
  }));
}

#include <memory>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "rpc/rpc_server.h"
#include "rpc/rpc_codes.pb.h"
#include "examples/echo.pb.h"

DEFINE_int32(listen_port, 8080, "");

class EchoServiceImpl : public EchoService {
 public:
  void Echo(::google::protobuf::RpcController* controller,
            const ::EchoRequest* request,
            ::EchoResponse* response,
            ::google::protobuf::Closure* done) override {
    if (request->ping().empty()) {
      controller->SetFailed("CALL_ERROR");
    } else {
      response->set_pong(request->ping());
    }
    done->Run();
  }
};

int main(int argc, char* argv[]) {
  ::google::ParseCommandLineFlags(&argc, &argv, true);
  ::google::InitGoogleLogging(argv[0]);

  EchoServiceImpl service;
  RpcServer::Builder builder;
  builder.AddService(&service)
         .Listen(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                 FLAGS_listen_port));
  std::shared_ptr<RpcServer> server(builder.Build());

  LOG(INFO) << "server start!";
  server->Serve();
  LOG(INFO) << "server stop!";
  return 0;
}

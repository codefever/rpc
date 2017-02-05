#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "examples/echo.pb.h"
#include "rpc/pack.h"
#include "rpc/rpc_controller.h"

DEFINE_string(host, "127.0.0.1", "");
DEFINE_int32(port, 8080, "");

void SendRequest(const google::protobuf::Message& request,
                 boost::asio::ip::tcp::socket &socket) {
  RawMessage msg;
  msg.set_sid(100);
  msg.set_mid(1024);
  CHECK(request.SerializeToString(msg.mutable_payload()));
  // LOG(INFO) << "payload: " << msg.payload().size();

  SimpleMessageEncoder encoder;
  auto* encoded_data = encoder.Encode(&msg);
  const char* data = nullptr;
  size_t size = 0;
  while (encoded_data->Next(&data, &size)) {
    boost::system::error_code ec;
    ssize_t n = socket.send(boost::asio::buffer(data, size), 0, ec);
    CHECK(size == n && !ec);
  }
  CHECK(!encoded_data->Error());
  delete encoded_data;
}

void RecvResponse(google::protobuf::Message* response,
                  boost::asio::ip::tcp::socket &socket,
                  rpc::RpcErrorCode* error_code) {
  SimpleMessageDecoder decoder;
  RawMessage* msg = nullptr;
  do {
    char* data = nullptr;
    size_t size = 0;
    RawMessageDecoder::DecodeCallback decode_func;
    decoder.Next(&data, &size, &decode_func);
    boost::system::error_code ec;
    ssize_t n = socket.receive(boost::asio::buffer(data, size), 0, ec);
    CHECK(!ec && decode_func(n));
  } while ((msg = decoder.PopDecodedMessage()) == nullptr);

  // LOG(INFO) << "payload: " << msg->payload().size();
  CHECK(response->ParseFromString(msg->payload()));
  *error_code = msg->error_code();
  delete msg;
}

int main(int argc, char* argv[]) {
  ::google::ParseCommandLineFlags(&argc, &argv, true);
  ::google::InitGoogleLogging(argv[0]);

  boost::asio::io_service io_service;
  boost::asio::ip::tcp::socket socket(io_service);
  boost::asio::ip::tcp::endpoint endpoint(
      boost::asio::ip::address::from_string(FLAGS_host),
      FLAGS_port);

  socket.connect(endpoint);

  std::string line;
  while (true) {
    std::cout << ">>> ping: ";
    if (!std::getline(std::cin, line)) {
      break;
    }
    EchoRequest request;
    EchoResponse response;
    request.set_ping(line);
    rpc::RpcErrorCode error_code;

    SendRequest(request, socket);
    RecvResponse(&response, socket, &error_code);

    if (error_code != rpc::RpcErrorCode::OK) {
      LOG(ERROR) << "rpc fail: " << error_code;
    } else {
      std::cout << "<<< pong: " << response.pong() << std::endl;
    }
  }
  return 0;
}

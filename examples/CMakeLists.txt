include(Protobuf)

file(GLOB echo_PROTOS "*.proto")
add_proto_cpp_library(echo_PROTO_SRCS echo_PROTO_HDRS ${echo_PROTOS})
add_library(echo_proto ${echo_PROTO_SRCS} ${echo_PROTO_HDRS})
target_link_libraries(echo_proto rpc_proto ${PROTOBUF_LIBRARIES})
target_include_directories(echo_proto PUBLIC ${PROJECT_BINARY_DIR} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

set(echo_server_SRCS "echo_server.cc")
add_executable(echo_server ${echo_server_SRCS})
target_link_libraries(echo_server echo_proto rpc)

set(echo_raw_client_SRCS "echo_raw_client.cc")
add_executable(echo_raw_client ${echo_raw_client_SRCS})
target_link_libraries(echo_raw_client echo_proto rpc)

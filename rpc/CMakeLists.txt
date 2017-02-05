include(CppLint)
include(Protobuf)

# rpc_*.proto
file(GLOB rpc_PROTOS "*.proto")
add_proto_cpp_library(rpc_PROTO_SRCS rpc_PROTO_HDRS ${rpc_PROTOS})
add_library(rpc_proto ${rpc_PROTO_SRCS} ${rpc_PROTO_HDRS})
target_link_libraries(rpc_proto ${PROTOBUF_LIBRARIES})
target_include_directories(rpc_proto PUBLIC ${PROJECT_BINARY_DIR} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# lib
find_package(Boost 1.56.0 REQUIRED COMPONENTS system thread)
find_package(GFlags)
find_package(Glog)
#set(Boost_USE_STATIC_LIBS ON)
include_directories(${Boost_INCLUDE_DIRS} ${GFlags_INCLUDE_DIRS} ${Glog_INCLUDE_DIRS})
file(GLOB rpc_HDRS *.h)
file(GLOB rpc_SRCS *.cc)
add_library(rpc ${rpc_SRCS} ${rpc_HDRS})
add_cpplint_check(TARGET rpc FILES ${rpc_SRCS} ${rpc_HDRS})
target_link_libraries(rpc rpc_proto ${Boost_LIBRARIES} ${GFLAGS_LIBRARIES} ${GLOG_LIBRARIES})

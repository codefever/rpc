cmake_minimum_required (VERSION 3.2)
project (rpc)

set(CMAKE_CXX_STANDARD 11)

# ---[ Using cmake scripts and modules
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)
include(Cpplint)

# rpc_*.proto
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
file(GLOB rpc_PROTOS "*.proto")
PROTOBUF_GENERATE_CPP(rpc_PROTO_SRCS rpc_PROTO_HDRS ${rpc_PROTOS})
add_library(rpc_proto ${rpc_PROTO_SRCS} ${rpc_PROTO_HDRS})
target_link_libraries(rpc_proto ${PROTOBUF_LIBRARIES})

# lib
find_package(Boost 1.63.0 REQUIRED COMPONENTS system thread)
find_package(GFlags)
find_package(Glog)
#set(Boost_USE_STATIC_LIBS ON)
include_directories(${Boost_INCLUDE_DIRS} ${GFlags_INCLUDE_DIRS} ${Glog_INCLUDE_DIRS})
file(GLOB rpc_HDRS *.h)
file(GLOB rpc_SRCS *.cc)
add_library(rpc ${rpc_SRCS} ${rpc_HDRS})
add_cpplint_check(TARGET rpc FILES ${rpc_SRCS} ${rpc_HDRS})
target_link_libraries(rpc rpc_proto ${Boost_LIBRARIES} ${GFlags_IBRARIES} ${Glog_LIBRARIES})
#cmake_minimum_required(VERSION 3.10)
#project(my_simple_ros LANGUAGES CXX)
#
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#
## ===== 找到依赖 =====
#find_package(Protobuf CONFIG REQUIRED)
#
## 打印 Protobuf 信息（可选，用于验证是否找到）
#message(STATUS "Protobuf 版本: ${Protobuf_VERSION}")
#message(STATUS "Protobuf 头文件路径: ${Protobuf_INCLUDE_DIRS}")
#message(STATUS "Protobuf 库文件: ${Protobuf_LIBRARIES}")
#message(STATUS "protoc 编译器路径: ${Protobuf_PROTOC_EXECUTABLE}")
#
#target_include_directories(ros_rpc_core PUBLIC
#        ${CMAKE_CURRENT_SOURCE_DIR}/include
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/generated
#        ${Protobuf_INCLUDE_DIRS}
#        ${gRPC_INCLUDE_DIRS}
#        ${EIGEN3_INCLUDE_DIR}
#)
#
cmake_minimum_required(VERSION 3.10)
project(simple_ros_comm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== 找到依赖 =====
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)


message(STATUS "Protobuf include dirs: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf libs: ${Protobuf_LIBRARIES}")
message(STATUS "gRPC include dirs: ${gRPC_INCLUDE_DIRS}")
message(STATUS "gRPC libs: ${gRPC_LIBRARIES}")

# ===== 核心库 =====
add_library(ros_rpc_core
        src/msg_factory.cpp
        src/generated/ros_rpc.pb.cc
        src/generated/ros_rpc.grpc.pb.cc
        src/generated/example.pb.cc
        src/generated/marker.pb.cc
        src/generated/geometry_msgs.pb.cc
)

target_include_directories(ros_rpc_core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/generated
        ${Protobuf_INCLUDE_DIRS}
        ${gRPC_INCLUDE_DIRS}
        ${EIGEN3_INCLUDE_DIR}
)

target_link_libraries(ros_rpc_core
        PUBLIC
        protobuf::libprotobuf
        gRPC::grpc++
        gRPC::grpc++_reflection
        pthread
        muduo_net
        muduo_base
        nlohmann_json::nlohmann_json
)
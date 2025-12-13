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


# ===== 生成 Protobuf 文件 =====
# 定义 proto 文件目录
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
# 定义生成路径
set(GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/generated)

# ===== 核心库 =====
add_library(ros_rpc_core
        src/msg_factory.cpp
        src/poll_manager.cpp
        src/generated/example.pb.cc
        src/generated/ros_rpc.pb.cc
        src/message_graph.cpp
        src/timer.cpp
        src/master_tcp_server.cpp
        src/ros_rpc_client.cpp
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

# ===== 测试程序 =====
add_executable(mytest_msg_factory
        test/mytest_msg_factory.cpp
)

target_include_directories(mytest_msg_factory PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${GENERATED_DIR}
)

target_link_libraries(mytest_msg_factory
        PRIVATE
        ros_rpc_core
)

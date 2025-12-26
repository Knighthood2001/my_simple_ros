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
        src/generated/ros_rpc.grpc.pb.cc
        src/message_graph.cpp
        src/timer.cpp
        src/master_tcp_server.cpp
        src/ros_rpc_client.cpp
        src/ros_rpc_server.cpp
        src/global_init.cpp
        src/node_handle.cpp
        src/publisher.cpp
        src/subscriber.cpp
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
# 1. 列出所有测试源文件（替换你原来逐个写的4个测试程序）
set(MYTEST
        test/mytest_msg_factory.cpp
        test/mytest_SystemManager.cpp
        test/mytest_Timer.cpp
        test/mytest_init.cpp
        # 后续新增测试文件，直接在这里加一行即可
)

# 2. 批量编译所有测试程序
foreach(test_src IN LISTS MYTEST)
    # 提取文件名（不含后缀）作为可执行程序名（比如 mytest_msg_factory.cpp → mytest_msg_factory）
    get_filename_component(test_name ${test_src} NAME_WE)

    # 创建可执行程序（替代原来逐个add_executable）
    add_executable(${test_name} ${test_src})

    # 设置头文件包含目录（和你原有配置完全一致）
    target_include_directories(${test_name} PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${GENERATED_DIR}
    )

    # 链接依赖库（保留你原有ros_rpc_core，若有GTest/线程库可按需加）
    target_link_libraries(${test_name}
            PRIVATE
            ros_rpc_core
    )

    # 设置输出目录（可选，和你示例里一致，放到bin/tests下）
    set_target_properties(${test_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/mytests
    )
endforeach()


# ===== 测试 =====
find_package(GTest REQUIRED)
enable_testing()

# 测试列表
set(TESTS
        test/gtest_init.cpp
)

# 编译测试文件 -> 放到 bin/tests
foreach(test_src IN LISTS TESTS)
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name}
            PRIVATE
            ros_rpc_core
            GTest::GTest
            GTest::Main
            pthread)
    set_target_properties(${test_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
    )
    add_test(NAME ${test_name} COMMAND ${CMAKE_BINARY_DIR}/bin/tests/${test_name})
endforeach()

# ====工具=====
set(TOOLS
        tools/master.cpp
)
# 编译测试文件 -> 放到 bin/tests
foreach(tool_src IN LISTS TOOLS)
    get_filename_component(tool_name ${tool_src} NAME_WE)
    add_executable(${tool_name} ${tool_src})
    target_link_libraries(${tool_name}
            PRIVATE
            ros_rpc_core)
    set_target_properties(${tool_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tools
    )
endforeach()


//
// Created by wu on 25-12-2.
//

#ifndef ROS_RPC_CLIENT_H
#define ROS_RPC_CLIENT_H
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "ros_rpc.grpc.pb.h"

namespace simple_ros{
class RosRpcClient{
  public:
    //显式构造函数
    explicit RosRpcClient(const std::string& server_address);
    ~RosRpcClient();

  private:
    //RosRpcService::Stub 是 gRPC 自动生成的客户端存根类，用于调用服务端定义的 RPC 方法。
    std::unique_ptr<RosRpcService::Stub> stub_;  // 使用 std::unique_ptr 管理 Stub
};
}
#endif //ROS_RPC_CLIENT_H

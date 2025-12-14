//
// Created by wu on 25-12-13.
//

#ifndef ROS_RPC_SERVER_H
#define ROS_RPC_SERVER_H
#include <grpcpp/grpcpp.h>
#include "ros_rpc.grpc.pb.h"
#include "ros_rpc.pb.h"
#include <memory>
#include <string>

namespace simple_ros{
//业务逻辑实现（RosRpcServiceImpl）+ 服务生命周期管理（RosRpcServer）
// ========== 核心1：RPC 业务逻辑实现类 ==========
// final：禁止该类被继承，避免子类重写 RPC 接口导致逻辑混乱
// RosRpcService::Service 是 proto 文件生成的服务基类，所有 RPC 接口都是纯虚函数
class RosRpcServiceImpl final :public RosRpcService::Service{
  public:
    // 重写“订阅话题”的 RPC 接口（必须加 override，保证签名匹配）
    // 接口签名：gRPC 标准 → Status返回值 + 上下文 + 请求指针 + 响应指针
    // 订阅话题服务
    grpc::Status Subscribe(grpc::ServerContext* context, const SubscribeRequest* request, SubscribeResponse* response) override;
    
  private:

};
// ========== 核心2：gRPC 服务端生命周期管理类 ==========
class RosRpcServer{
  public:
  // 构造函数：接收监听地址、TCP服务、节点图（传给内部的service_）
  explicit RosRpcServer(const std::string& server_address);
  ~RosRpcServer();
  
  void Run();
  void Shutdown();
  
  private:
    std::string server_address_;  // 监听地址（ip:port）
    std::unique_ptr<grpc::Server> server_;  // gRPC服务端核心对象（独占所有权，unique_ptr）
    RosRpcServiceImpl service_;  // RPC服务实现实例（绑定到server_上）
};
}
#endif //ROS_RPC_SERVER_H

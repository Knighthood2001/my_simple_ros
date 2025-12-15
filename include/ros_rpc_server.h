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
#include "message_graph.h"
#include <mutex>
#include "master_tcp_server.h"

namespace simple_ros{
//业务逻辑实现（RosRpcServiceImpl）+ 服务生命周期管理（RosRpcServer）
// ========== 核心1：RPC 业务逻辑实现类 ==========
// final：禁止该类被继承，避免子类重写 RPC 接口导致逻辑混乱
// RosRpcService::Service 是 proto 文件生成的服务基类，所有 RPC 接口都是纯虚函数
class RosRpcServiceImpl final :public RosRpcService::Service{
  public:
    RosRpcServiceImpl(std::shared_ptr<MasterTcpServer> tcp_server, std::shared_ptr<MessageGraph> graph) :tcp_server_(tcp_server), graph_(graph){};
    ~RosRpcServiceImpl() override = default;
    // 重写“订阅话题”的 RPC 接口（必须加 override，保证签名匹配）
    // 接口签名：gRPC 标准 → Status返回值 + 上下文 + 请求指针 + 响应指针
    // 订阅话题服务
    grpc::Status Subscribe(grpc::ServerContext* context, const SubscribeRequest* request, SubscribeResponse* response) override;
    grpc::Status RegisterPublisher(grpc::ServerContext* context, const RegisterPublisherRequest* request, RegisterPublisherResponse* response) override;
    grpc::Status Unsubscribe(grpc::ServerContext* context, const UnsubscribeRequest* request, UnsubscribeResponse* response) override;
    grpc::Status UnregisterPublisher(grpc::ServerContext* context, const UnregisterPublisherRequest* request, UnregisterPublisherResponse* response) override;
    // 新增：获取节点列表
    grpc::Status GetNodes(grpc::ServerContext* context, const GetNodesRequest* request, GetNodesResponse* response) override;
    // 新增：获取节点详细信息
    grpc::Status GetNodeInfo(grpc::ServerContext* context, const GetNodeInfoRequest* request, GetNodeInfoResponse* response) override;
    // 新增：获取话题列表
    grpc::Status GetTopics(grpc::ServerContext* context, const GetTopicsRequest* request, GetTopicsResponse* response) override;
    // 新增：获取话题详细信息
    grpc::Status GetTopicInfo(grpc::ServerContext* context, const GetTopicInfoRequest* request, GetTopicInfoResponse* response) override;
      
  private:
        // 使用外部传入的图结构智能指针
    std::shared_ptr<MessageGraph> graph_;
    mutable std::mutex mtx_;
    std::shared_ptr<MasterTcpServer> tcp_server_;
};
// ========== 核心2：gRPC 服务端生命周期管理类 ==========
class RosRpcServer{
  public:
  // 构造函数：接收监听地址、TCP服务、节点图（传给内部的service_）
  RosRpcServer(const std::string& server_address, std::shared_ptr<MasterTcpServer> tcp_server, std::shared_ptr<MessageGraph> graph);
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

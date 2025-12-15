//
// Created by wu on 25-12-13.
//
#include "ros_rpc_server.h"
#include <grpcpp/server_builder.h>
#include <iostream>
#include "message_graph.h"

namespace simple_ros{
grpc::Status RosRpcServiceImpl::Subscribe(grpc::ServerContext* context, const SubscribeRequest* request, SubscribeResponse* response){
  //用「话题名 + 消息类型」拼接成唯一键
  const std::string topic_key = request->topic_name() + "/" + request->msg_type();
  const NodeInfo& node = request->node_info();//node：引用请求中的 NodeInfo（订阅节点的元信息，包含节点名、IP、端口等），避免拷贝，提升效率。

  LOG_INFO << "Received Subscribe request: topic: "<< request->topic_name() << ", msg_type=" << request->msg_type() << ", node_name=" << node.node_name();
  {
    // 步骤1：更新节点拓扑图，添加订阅者关系
    graph_->AddSubscriber(node, {request->topic_name(), request->msg_type()});//把「节点」和「话题 + 消息类型」的订阅关系写入拓扑图；
    LOG_DEBUG << "Addd subscriber "<< node.node_name() << "to topic " << request->topic_name();
    // 通知节点 “话题的目标列表变化”
    simple_ros::TopicTargetsUpdate update;
    update.set_topic(request->topic_name());  // 指定通知对应的话题名；
    auto* add_node = update.add_add_targets();  // 添加 “需要新增的目标节点”（即新订阅者）—— 发布者收到这个通知后，会把该节点加入 “数据发送列表”；
    *add_node = node;  //把新订阅节点的 NodeInfo（IP、端口、节点名）写入通知。

    // 遍历发布者并发送通知
    int count = 0;
    for(auto& pub : graph_->GetPublishersByTopic(request->topic_name())) {// 从拓扑图中获取该话题的所有发布者节点（NodeInfo 列表）
      tcp_server_->SendUpdate(pub.node_name(), update);  //给每个发布者发 “新增订阅者” 的 TCP 通知；
      count++;
    }
    LOG_INFO << "Notified " << count << " publishers about new subscriber " << node.node_name();
  }
  response->set_success(true);
  response->set_message("Subscribe success");
  LOG_INFO << "Subscribe request processed successfully for node " << node.node_name();
  return grpc::Status::OK;
}
grpc::Status RegisterPublisher(grpc::ServerContext* context, const RegisterPublisherRequest* request, RegisterPublisherResponse* response){

}
grpc::Status Unsubscribe(grpc::ServerContext* context, const UnsubscribeRequest* request, UnsubscribeResponse* response){
  const TopicKey k{request->topic_name(),request->msg_type()};
  const NodeInfo& node = request->node_info();
  {
    // 步骤1：从拓扑图中移除该节点的订阅关系
    graph_->RemoveSubscriber(node ,k);

    // 步骤2：构建“移除订阅者”的更新通知（发给发布者）
    simple_ros::TopicTargetsUpdate update;
    update.set_topic(request->topic_name());  // 指定通知对应的话题名
    auto* remove_node = update.add_remove_targets();  // 添加“要移除的目标节点”字段
    *remove_node = node;  // 把取消订阅的节点信息写入通知

    // 步骤3：遍历该话题的所有发布者，逐个发送“移除订阅者”通知
    for (auto& pub : graph_->GetPublishersByTopic(request->topic_name())) {
      tcp_server_->SendUpdate(pub.node_name(), update);
    }
  }
  response->set_success(true);
  response->set_message("Unsubscribe success");
  return grpc::Status::OK;
}
grpc::Status UnregisterPublisher(grpc::ServerContext* context, const UnregisterPublisherRequest* request, UnregisterPublisherResponse* response){

}
// 新增：获取节点列表
grpc::Status GetNodes(grpc::ServerContext* context, const GetNodesRequest* request, GetNodesResponse* response){

}
// 新增：获取节点详细信息
grpc::Status GetNodeInfo(grpc::ServerContext* context, const GetNodeInfoRequest* request, GetNodeInfoResponse* response){

}
// 新增：获取话题列表
grpc::Status GetTopics(grpc::ServerContext* context, const GetTopicsRequest* request, GetTopicsResponse* response){
  
}
// 新增：获取话题详细信息
grpc::Status GetTopicInfo(grpc::ServerContext* context, const GetTopicInfoRequest* request, GetTopicInfoResponse* response){

}
  
RosRpcServer::RosRpcServer(const std::string& server_address): server_address_(server_address){

}

RosRpcServer::~RosRpcServer(){
  Shutdown();
}
void RosRpcServer::Run(){
  // 1. 创建 gRPC 服务构建器：用于配置服务端参数
  grpc::ServerBuilder builder;
  // 2. 绑定监听地址 + 安全凭证（Insecure=不安全，仅测试用）
  builder.AddListeningPort(server_address_, grpc::InsecureServerCredentials());
  // 3. 注册业务逻辑实例：把 service_ 绑定到 Server，让 Server 能转发请求到 service_
  builder.RegisterService(&service_);
  // 4. 构建并启动服务：返回 Server 智能指针，赋值给 server_
  server_ = builder.BuildAndStart();
  std::cout << "Server listening on "<< server_address_ << std::endl;
  // 6. 阻塞等待服务停止：否则启动后立即退出（核心！）
  server_->Wait();
}

// ========== 关闭 gRPC 服务 ==========
void RosRpcServer::Shutdown(){
  if (server_) server_->Shutdown();
}
}
//
// Created by wu on 25-12-13.
//
#include "ros_rpc_server.h"
#include <grpcpp/server_builder.h>
#include <iostream>

namespace simple_ros{
grpc::Status RosRpcServiceImpl::Subscribe(grpc::ServerContext* context, const SubscribeRequest* request, SubscribeResponse* response){
  std::cout << "Received Subscribe request fo topic: "<< request->topic_name() << std::endl;
  response->set_success(true);
  response->set_message("Subscribe success");
  return grpc::Status::OK;
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
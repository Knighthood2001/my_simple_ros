//
// Created by wu on 25-12-2.
//
#include <iostream>
#include "ros_rpc_client.h"

namespace simple_ros{
RosRpcClient::RosRpcClient(const std::string& server_address){
  stub_ = RosRpcService::NewStub(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));
}

bool RosRpcClient::Subscribe(const std::string& topic_name,
                            const std::string& msg_type,
                            const NodeInfo& node_info,
                            SubscribeResponse* response){
  //构造请求对象
  SubscribeRequest request;
  request.set_topic_name(topic_name);
  request.set_msg_type(msg_type);
  *request.mutable_node_info() = node_info;
  //创建一个 grpc::ClientContext 对象，用于管理 RPC 调用的上下文信息。
  grpc::ClientContext context;
  //调用 RPC 方法
  grpc::Status status = stub_->Subscribe(&context, request, response);

  if (!status.ok()) {
    std::cerr << "subscribe RPC failed:" << status.error_message() << std::endl;
    return false;
  }
  return true;
}
bool RosRpcClient::Unsubscribe(const std::string& topic_name,
                              const std::string& msg_type,
                              const NodeInfo& node_info,
                              UnsubscribeResponse* response){
  UnsubscribeRequest request;
  request.set_topic_name(topic_name);
  request.set_msg_type(msg_type);
  *request.mutable_node_info() = node_info;

  grpc::ClientContext context;
  grpc::Status status = stub_->Unsubscribe(&context, request, response);
  if (!status.ok()) {
    std::cerr << "unsubscribe RPC failed:" << status.error_message() << std::endl;
    return false;
  }
  return true;
}

bool RosRpcClient::RegisterPublisher(const std::string& topic_name,
                                    const std::string& msg_type,
                                    const NodeInfo& node_info,
                                    RegisterPublisherResponse* response) {
    RegisterPublisherRequest request;
    request.set_topic_name(topic_name);
    request.set_msg_type(msg_type);
    *request.mutable_node_info() = node_info;
                                        
    grpc::ClientContext context;
    grpc::Status status = stub_->RegisterPublisher(&context, request, response);

    if (!status.ok()) {
        std::cerr << "RegisterPublisher RPC failed: " << status.error_message() << std::endl;
        return false;
    }
    return true;
}

bool RosRpcClient::UnregisterPublisher(const std::string& topic_name,
                                       const std::string& msg_type,
                                       const NodeInfo& node_info,
                                       UnregisterPublisherResponse* response) {
    UnregisterPublisherRequest request;
    request.set_topic_name(topic_name);
    request.set_msg_type(msg_type);
    *request.mutable_node_info() = node_info;

    grpc::ClientContext context;
    grpc::Status status = stub_->UnregisterPublisher(&context, request, response);
    if (!status.ok()) {
        std::cerr << "UnregisterPublisher RPC failed: " << status.error_message() << std::endl;
        return false;
    }
    return true;
}
bool RosRpcClient::GetNodes(const std::string& filter, GetNodesResponse* response){
  GetNodesRequest request;
  request.set_filter(filter);

  grpc::ClientContext context;
  grpc::Status status = stub_->GetNodes(&context, request, response);

  if(!status.ok()){
    std::cerr << "GetNodes RPC failed:" << status.error_message() << std::endl;
    return false;
  }
  return true;
}
bool RosRpcClient::GetNodeInfo(const std::string& node_name, GetNodeInfoResponse* response){
  GetNodeInfoRequest request;
  request.set_node_name(node_name);

  grpc::ClientContext context;
  grpc::Status status = stub_->GetNodeInfo(&context, request, response);

  if(!status.ok()){
    std::cerr << "GetNodeInfo RPC failed:" << status.error_message() << std::endl;
    return false;
  }
  return true;
}

bool RosRpcClient::GetTopics(const std::string& filter, GetTopicsResponse* response){
  GetTopicsRequest request;
  request.set_filter(filter);

  grpc::ClientContext context;
  grpc::Status status = stub_->GetTopics(&context, request, response);

  if(!status.ok()){
    std::cerr << "GetTopics RPC failed:" << status.error_message() << std::endl;
    return false;
  }
  return true;
}

bool RosRpcClient::GetTopicInfo(const std::string& topic_name, GetTopicInfoResponse* response){
  GetTopicInfoRequest request;
  request.set_topic_name(topic_name);

  grpc::ClientContext context;
  grpc::Status status = stub_->GetTopicInfo(&context, request, response);

  if(!status.ok()){
    std::cerr << "GetTopicInfo RPC failed:" << status.error_message() << std::endl;
    return false;
  }
  return true;
}



}
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
grpc::Status RosRpcServiceImpl::RegisterPublisher(grpc::ServerContext* context, const RegisterPublisherRequest* request, RegisterPublisherResponse* response){
  const TopicKey k{request->topic_name(), request->msg_type()};
  const NodeInfo& node = request->node_info();
  
  {
    graph_->AddPublisher(node, k);
    LOG_INFO << "RegisterPublisher request: topic=" << request->topic_name() 
            << ", msg_type=" << request->msg_type() 
            << ", node_name=" << node.node_name();
    simple_ros::TopicTargetsUpdate update;
    update.set_topic(request->topic_name());
    for (auto& pub : graph_->GetSubscribersByTopic(request->topic_name())) {
      auto* add_node = update.add_add_targets();
      *add_node = node;
    }
    tcp_server_->SendUpdate(node.node_name(), update);
  }
  response->set_success(true);
  response->set_message("Register publisher success");
  return grpc::Status::OK;
}
grpc::Status RosRpcServiceImpl::Unsubscribe(grpc::ServerContext* context, const UnsubscribeRequest* request, UnsubscribeResponse* response){
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
grpc::Status RosRpcServiceImpl::UnregisterPublisher(grpc::ServerContext* context, const UnregisterPublisherRequest* request, UnregisterPublisherResponse* response){
  const TopicKey k{request->topic_name(),request->msg_type()};
  const NodeInfo& node = request->node_info();
  
  {
    graph_->RemovePublisher(node ,k);
    LOG_INFO << "UnregisterPublisher request: topic=" << request->topic_name() 
            << ", msg_type=" << request->msg_type() 
            << ", node_name=" << node.node_name();

  }
  response->set_success(true);
  response->set_message("Unregister publisher success");
  return grpc::Status::OK;
}
// 处理客户端 “获取节点列表” 的请求，支持按节点名模糊过滤，返回符合条件的节点列表，并告知请求处理结果。
grpc::Status RosRpcServiceImpl::GetNodes(grpc::ServerContext* context, const GetNodesRequest* request, GetNodesResponse* response){
  std::vector<NodeInfo> nodes = graph_->GetAllNodes();  // 系统中所有节点的完整列表
  
  for (const auto& node : nodes) {
    //// 可以根据filter进行过滤，如果提供了filter参数
    if(!request->filter().empty() && node.node_name().find(request->filter()) == std::string::npos) {
      continue;  // 过滤掉不匹配的节点
    }
    //将单个NodeInfo赋值给响应的新元素
    *response->add_nodes() = node;
  }
  response->set_success(true);
  response->set_message("Get nodes list success");
  return grpc::Status::OK;
}
// 新增：获取节点详细信息
grpc::Status RosRpcServiceImpl::GetNodeInfo(grpc::ServerContext* context, const GetNodeInfoRequest* request, GetNodeInfoResponse* response){
  const std::string& node_name = request->node_name();
  
  if (!graph_->HasNode(node_name)){
    response->set_success(false);
    response->set_message("Node not found" + node_name);
    LOG_WARN << "Node " << node_name << " not found";
    return grpc::Status::OK;
  }
  
  NodeInfo node_info;
  graph_->GetNodeByName(node_name, &node_info);
  *response->mutable_node_info() = node_info;
  
  // 获取并填充节点发布的话题列表
  auto publish_topic_keys = graph_->GetNodePublishTopicKeys(node_name);
  for (const auto& topic_key : publish_topic_keys) {
    TopicInfo* topic_info = response->add_publishes();
    topic_info->set_topic_name(topic_key.topic);
    topic_info->set_msg_type(topic_key.msg_type);  //设置该话题的消息类型（如 std_msgs/String）
  }

  // 获取并填充节点订阅的话题列表
  auto subscribe_topic_keys = graph_->GetNodeSubscribeTopicKeys(node_name);
  for (const auto& topic_key : subscribe_topic_keys) {
    TopicInfo* topic_info = response->add_subscribes();
    topic_info->set_topic_name(topic_key.topic);
    topic_info->set_msg_type(topic_key.msg_type);
  }
  response->set_success(true);
  response->set_message("Get node info success");
  return grpc::Status::OK;
}
// 新增：获取话题列表
grpc::Status RosRpcServiceImpl::GetTopics(grpc::ServerContext* context, const GetTopicsRequest* request, GetTopicsResponse* response){
  std::lock_guard<std::mutex> lock(mtx_);

  try{
    // 获取所有节点的发布和订阅话题
    std::unordered_map<std::string, std::string> topic_msg_types;

    // 遍历所有节点
    std::vector<NodeInfo> all_nodes = graph_->GetAllNodes();
    for (const NodeInfo& node : all_nodes) {
      // 获取节点发布的话题（包含消息类型）
      std::vector<TopicKey> publish_topics = graph_->GetNodePublishTopicKeys(node.node_name());//获取该节点发布的所有话题键（topic_key 是自定义结构体，含 topic（话题名）和 msg_type（消息类型）
      for (const TopicKey& topic_key : publish_topics){
        topic_msg_types[topic_key.topic] = topic_key.msg_type;
      }
      // 获取节点订阅的话题（包含消息类型）
      std::vector<TopicKey> subscribe_topics = graph_-> GetNodeSubscribeTopicKeys(node.node_name());
      for (const TopicKey& topic_key : subscribe_topics){
        topic_msg_types[topic_key.topic] = topic_key.msg_type;
      }
    }

    // 应用过滤条件
    const std::string& filter = request->filter();
    for (const auto& pair : topic_msg_types){  // const std::pair<const std::string, std::string>&    只读 + 引用，无拷贝，效率最优。
      const std::string& topic_name = pair.first;
      const std::string& msg_type = pair.second;
      // 如果没有过滤条件或话题名包含过滤字符串
      if(filter.empty() || topic_name.find(filter) != std::string::npos){
        // 填充 Protobuf 响应
        TopicInfo* topic_info = response->add_topics();
        topic_info->set_topic_name(topic_name);
        topic_info->set_msg_type(msg_type);
      }
    }
    response->set_success(true);
    response->set_message("Get topics success");
    return grpc::Status::OK;
  }catch(const std::exception& e){
    response->set_success(false);
    response->set_message(std::string("get topics failed: ") + e.what());
    return grpc::Status(grpc::StatusCode::INTERNAL, response->message());  // 标记为 服务内部错误
  }
}
// 新增：获取话题详细信息
grpc::Status RosRpcServiceImpl::GetTopicInfo(grpc::ServerContext* context, const GetTopicInfoRequest* request, GetTopicInfoResponse* response){
  std::lock_guard<std::mutex> lock(mtx_);

  try{
    const std::string& topic_name = request->topic_name();

    std::string msg_type;
    bool topic_exists = false;

    std::vector<NodeInfo> all_nodes = graph_->GetAllNodes();
    for (const NodeInfo& node : all_nodes) {
      auto publish_topics = graph_->GetNodePublishTopicKeys(node.node_name());
      for (const TopicKey& topic_key : publish_topics){
        if(topic_key.topic == topic_name){ 
          msg_type = topic_key.msg_type;
          topic_exists = true;
          break;
        }
      }
      if (topic_exists) break;
      
      auto subscribe_topics = graph_->GetNodeSubscribeTopicKeys(node.node_name());
      for (const TopicKey& topic_key : subscribe_topics){
        if(topic_key.topic == topic_name){
          msg_type = topic_key.msg_type;
          topic_exists = true;
          break;
        }
      }
      if (topic_exists) break;
    }
    if (!topic_exists){
      response->set_success(false);
      response->set_message("Topic not found");
      return grpc::Status(grpc::StatusCode::NOT_FOUND, response->message());
    }
    
    std::vector<NodeInfo> publishers = graph_->GetPublishersByTopic(topic_name);
    for (const NodeInfo& publisher : publishers){
      NodeInfo* node_info = response->add_publishers();
      *node_info = publisher;
    }
    
    std::vector<NodeInfo> subscribers = graph_->GetSubscribersByTopic(topic_name);
    for (const NodeInfo& subscriber : subscribers){
      NodeInfo* node_info = response->add_subscribers();
      *node_info = subscriber;
    }
    response->set_success(true);
    response->set_message("Get topic info success");
    response->set_topic_name(topic_name);
    response->set_msg_type(msg_type);
    return grpc::Status::OK;
  }catch(const std::exception& e){
    response->set_success(false);
    response->set_message(std::string("Get topic info failed: ") + e.what());
    return grpc::Status(grpc::StatusCode::INTERNAL, response->message());
  }
}
  
RosRpcServer::RosRpcServer(const std::string& server_address, std::shared_ptr<MasterTcpServer> tcp_server, std::shared_ptr<MessageGraph> graph): server_address_(server_address), service_(tcp_server, graph) {

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
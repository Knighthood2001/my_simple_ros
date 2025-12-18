#include "publisher.h"
#include "poll_manager.h"
#include "global_init.h"
using namespace simple_ros;

template <typename T>
Publisher<T>::Publisher(const std::string& topic): topic_(topic){
  msgType_ = T::descriptor()->full_name();
  LOG_INFO<<"Publisher init: topic=" << topic << "type=" << msgType_;
  updateTargets();
}

template <typename T>
void Publisher<T>::publish(const T& msg){
  std::string msg_data;
  if(!msg.SerializeToString(&msg_data)){
    LOG_ERROR<<"Failed to serialize message";
    return;
  }

  std::string buffer;

  uint16_t topic_len = htons(static_cast<uint16_t>(topic_.size()));
  buffer.append(reinterpret_cast<char*>(&topic_len), sizeof(topic_len));
  buffer.append(topic_);

  uint16_t msg_name_len = htons(static_cast<uint16_t>(msgType_.size()));
  buffer.append(reinterpret_cast<char*>(&msg_name_len), sizeof(msg_name_len));
  buffer.append(msgType_);

  uint32_t msg_data_len = htonl(static_cast<uint32_t>(msg_data.size()));
  buffer.append(reinterpret_cast<char*>(&msg_data_len), sizeof(msg_data_len));
  buffer.append(msg_data);

  LOG_INFO<<"message packed: total size= " << buffer.size();
  for (const auto& conn_pair: connections_){
    if (conn_pair.second && conn_pair.second->connected()){
      conn_pair.second->send(buffer);
      LOG_INFO << "send to " << conn_pair.first << ", size=" << buffer.size();
    }
  }
}

template <typename T>
void Publisher<T>::updateTargets(){
  std::shared_ptr<PollManager> poll_manager = SystemManager::instance().getPollManager();
  if (!poll_manager){
    LOG_ERROR<<"poll manager is null";
    return;
  }
  auto targets_set = poll_manager->getTargets(topic_);
  std::vector<NodeInfo> targets(targets_set.begin(), targets_set.end());

  for(const auto& node_info : targets){
    std::string conn_id = getConnectionId(node_info);
    if (clients_.find(conn_id) == clients_.end()){
      createClient(node_info);
    }
  }
}

template <typename T>
void Publisher<T>::createClient(const NodeInfo& nodeInfo){
  std::string conn_id = getConnectionId(nodeInfo);
  LOG_INFO << "Create TCP client: " << conn_id;
  muduo::net::InetAddress server_addr(nodeInfo.ip().c_str(), nodeInfo.port());
  auto client = std::make_unique<muduo::net::TcpClient>(SystemManager::instance().getEventLoop().get(), server_addr, "PublisherClient");

  client->setConnectionCallback([this, conn_id](const muduo::net::TcpConnectionPtr& conn){
    if(conn->connected()){
      LOG_INFO<< "connected to "<< conn_id;
      connections_[conn_id] = conn;
    }else{
      LOG_INFO<< "disconnected from "<< conn_id;
      connections_.erase(conn_id);
    }
  });
  client->connect();
  clients_[conn_id] = std::move(client);  // 保存客户端实例（管理生命周期）
}

template <typename T>
std::string Publisher<T>::getConnectionId(const NodeInfo& nodeInfo){
  return nodeInfo.ip() + ":" + std::to_string(nodeInfo.port());
}

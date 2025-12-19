#include "node_handle.h"

using namespace simple_ros;

template <typename MsgType>
std::shared_ptr<Subscriber> NodeHandle::subscriber(const std::string& topic,
                                      uint32_t queue_size,
                                      std::function<void(const std::shared_ptr<MsgType>&)> callback){
  std::string msg_type_name = MsgType::descriptor()->full_name();
  LOG_INFO << "Subscriber to topic=" << topic << ", type= " << msg_type_name;
  
  auto subscriber = std::make_shared<Subscriber>(topic, queue_size, callback);
  
  auto rpc_client = SystemManager::instance().getRpcClient();
  if (rpc_client){
    SubscribeResponse response;
    bool success = rpc_client->Subscribe(topic, msg_type_name, nodeInfo_, &response);
    if (success) {
      LOG_INFO << "Subscribed to topic=" << topic << ", type=" << msg_type_name;
    }else{
      LOG_ERROR << "Subscribing to topic=" << topic << ", type=" << msg_type_name;
    }
  }else {
    LOG_ERROR << "Global Rpc client not initialized";
  }
  return subscriber;
}
template <typename MsgType>
std::shared_ptr<Publisher<MsgType>> NodeHandle::advertise(const std::string& topic){
  // 获取消息类型名称
  std::string msg_type_name = MsgType::descriptor()->full_name();
  LOG_INFO << "Advertise topic=" << topic << ", type=" << msg_type_name;

  // 创建发布者实例
  auto publisher = std::make_shared<Publisher<MsgType>>(topic);
  LOG_INFO << "Debug: nodeInfo_ details - node_name: '" << nodeInfo_.node_name() 
            << "', ip: '" << nodeInfo_.ip() 
            << "', port: " << nodeInfo_.port();
  // 调用RPC注册发布者
  auto rpc_client = SystemManager::instance().getRpcClient();
  if (rpc_client) {
      RegisterPublisherResponse response;
      bool success = rpc_client->RegisterPublisher(topic, msg_type_name, nodeInfo_, &response);
      if (success) {
          LOG_INFO << "RegisterPublisher RPC successful for topic: " << topic;
      } else {
          LOG_ERROR << "RegisterPublisher RPC failed for topic: " << topic;
      }
  } else {
      LOG_ERROR << "Global RPC client not initialized";
  }

  return publisher;
}
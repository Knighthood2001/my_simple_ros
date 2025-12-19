#include "subscriber.h"

template <typename MsgType>
Subscriber::Subscriber(const std::string& topic, uint32_t queue_size, std::function<void(const std::shared_ptr<MsgType>&)> typed_callback)
  : topic_(topic), queue_size_(queue_size){
  callback_ = [typed_callback](const std::shared_ptr<google::protobuf::Message>& msg_base){
    auto typed_msg = std::make_shared<MsgType>();
    std::string serialized;
    msg_base->SerializeToString(&serialized);
    if (!typed_msg->ParseFromString(serialized)){
      LOG_ERROR<<"Failed to parse message from string";
      return;
    }
    typed_callback(typed_msg);
  };
  auto msg_queue = SystemManager::instance().getMessageQueue();
  if (!msg_queue){
    LOG_ERROR<<"MessageQueue not initialized";
    return;
  }
  msg_queue_ = msg_queue;
  msg_queue->registerTopic(topic);
  msg_queue->setTopicMaxQueueSize(topic, queue_size);
  msg_queue->addSubscriber(topic, callback_);
  LOG_INFO << "Subscriber created fot topic: " << topic << ", message type: " << MsgType::descriptor()->full_name();
}
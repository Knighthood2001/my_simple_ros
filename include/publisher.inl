#include "publisher.h"
#include "poll_manager.h"

using namespace simple_ros;

template <typename T>
Publisher<T>::Publisher(const std::string& topic): topic_(topic){
  msgType_ = T::descriptor()->full_name();
  LOG_INFO<<"Publisher init: topic=" << topic << "type=" << msgType_;
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

}
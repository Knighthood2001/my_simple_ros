//
// Created by wu on 25-11-6.
//
#include "msg_factory.h"

MsgFactory& MsgFactory::instance(){
  static MsgFactory instance;
  return instance;
}

// 创建 unique_ptr<Message>
std::unique_ptr<google::protobuf::Message> MsgFactory::createMessage(const std::string& name){
  //1
  auto it = factory_.find(name);
  if (it != factory_.end()){
    return std::unique_ptr<google::protobuf::Message>it->second->New();
  }
  //如果消息类型未注册，仍然希望能够动态创建消息。
  //使用 Protobuf 的 DescriptorPool 和 DynamicMessageFactory 实现动态消息创建。
  const google::protobuf::Descriptor* desc = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(name);
  if(!desc){
    return nullptr;
  }
  const google::protobuf::Message* prototype = dynamic_factory_.GetPrototype(desc);
  if(!prototype){
    return nullptr;
  }
  //缓存下来，下次直接使用
  factory_[name] = prototype;

  return std::unique_ptr<google::protobuf::Message>(prototype->New());
}

std::shared_ptr<google::protobuf::Message> MsgFactory::makeSharedMessage(std::unique_ptr<google::protobuf::Message> msg){
  //std::move(msg) 的作用是 触发 unique_ptr 的移动语义，让它持有的 Message 资源能够被安全地转移给 shared_ptr。
  return std::shared_ptr<google::protobuf::Message>(std::move(msg));
}
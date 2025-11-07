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
  return nullptr;

}
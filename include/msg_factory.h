//
// Created by wu on 25-11-6.
//

#ifndef MSG_FACTORY_H
#define MSG_FACTORY_H

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <memory>
#include <string>
#include <unordered_map>

class MsgFactory{
public:
  // 获取单例实例
  static MsgFactory& instance();
  //注册消息类型
  template<typename MsgType>
  void registerMessage(){
    factory_[MsgType::descriptor()->full_name()] = &MsgType::default_instance();
  }
  //创建消息
  std::unique_ptr<google::protobuf::Message> createMessage(const std::string& name);


private:
  MsgFactory()=default;
  ~MsgFactory()=default;
  // 禁止拷贝和赋值
  MsgFactory(const MsgFactory&)=delete;
  MsgFactory& operator=(const MsgFactory&)=delete;
  //缓存消息原型
  std::unordered_map<std::string, const google::protobuf::Message*> factory_;
  google::protobuf::DynamicMessageFactory dynamic_factory_;


}

#endif //MSG_FACTORY_H

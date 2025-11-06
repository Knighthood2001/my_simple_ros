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

private:
  MsgFactory()=default;
  ~MsgFactory()=default;
  // 禁止拷贝和赋值
  MsgFactory(const MsgFactory&)=delete;
  MsgFactory& operator=(const MsgFactory&)=delete;



}

#endif //MSG_FACTORY_H

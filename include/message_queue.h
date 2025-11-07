//
// Created by wu on 25-11-7.
//

#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <string>

class MessageQueue {
public:
  // 推送消息到指定主题的队列
  void push(const std::string& topic, std::shared_ptr<google::protobuf::Message> msg){
    std::lock_guard<std::mutex> lock(mutex_);
    message_queue_[topic].push_back(msg);
  }
  void process(const std::string& topic){
    std::lock_guard<std::mutex> lock(mutex_);
    if(!message_queue_[topic].empty()){
      auto msg = message_queue_[topic].front();
      message_queue_[topic].pop_front();
      //处理消息
    }
  }

private:
  std::mutex mutex_;
  //多主题消息队列
  std::unordered_map<std::string, std::list<std::shared_ptr<google::protobuf::Message>>> message_queue_;

};
#endif //MESSAGE_QUEUE_H

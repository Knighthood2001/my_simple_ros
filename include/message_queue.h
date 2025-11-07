//
// Created by wu on 25-11-7.
//

#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H
#include <list>
#include <memory>
#include <mutex>

class MessageQueue {
public:
  void push(std::shared_ptr<google::protobuf::Message> msg){
    std::lock_guard<std::mutex> lock(mutex_);
    message_queue_.push_back(msg);
  }
  void process(){
    std::lock_guard<std::mutex> lock(mutex_);
    if(!message_queue_.empty()){
      auto msg = message_queue_.front();
      message_queue_.pop_front();
      //处理消息
    }
  }

private:
  std::mutex mutex_;
  std::list<std::shared_ptr<google::protobuf::Message>> message_queue_;

}
#endif //MESSAGE_QUEUE_H

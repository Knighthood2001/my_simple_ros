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
#include <functional>
#include <vector>
#include <google/protobuf/message.h>
#include <unordered_set>
#include <muduo/base/Logging.h>

class MessageQueue {
public:
  using Callback = std::function<void(const std::shared_ptr<google::protobuf::Message>&)>; //回调函数
                                                                                           
  // 构造函数，可以设置默认队列大小
  MessageQueue(uint32_t default_max_queue_size = 1000)
        : default_max_queue_size_(default_max_queue_size) {}
    
  // 设置主题的最大消息队列大小
  void setTopicMaxQueueSize(const std::string& topic, uint32_t max_size){
    std::lock_guard<std::mutex> lock(mutex_);
    topic_max_queue_sizes_[topic] = max_size;
  }

  //添加订阅者
  void addSubscriber(const std::string& topic, Callback cb){
    std::lock_guard<std::mutex> lock(mutex_);
    subscribers_[topic].push_back(cb);
  }

  //注册主题
  void registerTopic(const std::string& topic){
    std::lock_guard<std::mutex> lock(mutex_);
    if (registered_topics_.find(topic) == registered_topics_.end()) {
      registered_topics_.insert(topic);
      LOG_INFO << "Registering topic: " << topic;
    }

  }
  // 推送消息到指定主题的队列
  void push(const std::string& topic, std::shared_ptr<google::protobuf::Message> msg){
    std::lock_guard<std::mutex> lock(mutex_);
    if (registered_topics_.find(topic) == registered_topics_.end()) {
      //主题没有注册
      LOG_WARN << "Received message for unregistered topic: " << topic;
      return;
    }
    //检查队列是否为满，如果满，移除最早的消息
    auto it = topic_max_queue_sizes_.find(topic);
    uint32_t max_size = (it != topic_max_queue_sizes_.end()) ? it -> second : default_max_queue_size_;
    if (message_queue_[topic].size() >= max_size){
      message_queue_[topic].pop_front();
    }
    //添加新消息到队列
    message_queue_[topic].push_back(msg);
  }
  //处理指定主题的消息
  void process(const std::string& topic){
    std::lock_guard<std::mutex> lock(mutex_);
    if(!message_queue_[topic].empty()){
      auto msg = message_queue_[topic].front();
      message_queue_[topic].pop_front();
      //处理消息
      for (const auto& cb : subscribers_[topic]) {
        cb(msg);
      }
    }
  }

private:
  std::mutex mutex_;
  uint32_t default_max_queue_size_;                            // 默认队列大小
  //多主题消息队列
  std::unordered_map<std::string, std::list<std::shared_ptr<google::protobuf::Message>>> message_queue_;
  std::unordered_map<std::string, std::vector<Callback>> subscribers_; //订阅者列表
  std::unordered_map<std::string, uint32_t> topic_max_queue_sizes_; //每个主题的最大消息队列大小
  std::unordered_set<std::string> registered_topics_; //已经注册的主题
};
#endif //MESSAGE_QUEUE_H

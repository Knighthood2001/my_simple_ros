//
// Created by wu on 25-12-19.
//
#include "subscriber.h"
#include "message_queue.h"
#include <muduo/base/Logging.h>

Subscriber::Subscriber(const std::string& topic, uint32_t queue_size, MessageQueue::Callback callback)
    : topic_(topic), queue_size_(queue_size), callback_(std::move(callback)) {
  auto msg_queue = SystemManager::instance().getMessageQueue();
  if (msg_queue) {
    msg_queue_ = msg_queue;
    msg_queue->registerTopic(topic);
    msg_queue->setTopicMaxQueueSize(topic, queue_size);
    msg_queue->addSubscriber(topic, callback_);
  }else {
    LOG_ERROR<<"MessageQueue not initialized for topic: "<< topic;
  }
}

Subscriber::~Subscriber(){
  auto msg_queue = msg_queue_.lock();
  if (msg_queue) {
    msg_queue->removeSubscriber(topic_);
    LOG_INFO<<"Subscriber removed";
  }
}
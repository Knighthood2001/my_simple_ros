//
// Created by wu on 25-12-19.
//

#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H
#include <string>
#include <memory>
#include <functional>
#include <google/protobuf/message.h>
#include "global_init.h"
#include "message_queue.h"

class SystemManager;
class MessageQueue;

class Subscriber {
  public:
    Subscriber(const std::string& topic, uint32_t queue_size, MessageQueue::Callback callback);
    ~Subscriber();

    template <typename MsgType>
    Subscriber(const std::string& topic, uint32_t queue_size, std::function<void(const std::shared_ptr<MsgType>&)> typed_callback);
    Subscriber(const Subscriber&) = delete;
    Subscriber& operator=(const Subscriber&) = delete;

    Subscriber(Subscriber&&) noexcept = delete;
    Subscriber& operator=(Subscriber&&) noexcept = delete;
  private:
    std::string topic_;
    uint32_t queue_size_;
    MessageQueue::Callback callback_;
    std::weak_ptr<MessageQueue> msg_queue_;
};
#include "subscriber.inl"
#endif //SUBSCRIBER_H

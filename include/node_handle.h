//
// Created by wu on 25-12-19.
//

#ifndef NODE_HANDLE_H
#define NODE_HANDLE_H
#include <string>
#include <memory>
#include <functional>
#include "subscriber.h"
#include "publisher.h"
#include "ros_rpc.pb.h"
#include "timer.h"

using namespace simple_ros;

class NodeHandle{
  public:
    NodeHandle();
    ~NodeHandle();
    NodeHandle(const NodeHandle&) = delete;
    NodeHandle& operator=(const NodeHandle&) = delete;

    NodeHandle(NodeHandle&&) noexcept;
    NodeHandle& operator=(NodeHandle&&) noexcept;

    template <typename MsgType>
    std::shared_ptr<Subscriber> subscriber(const std::string& topic,
                                          uint32_t queue_size,
                                          std::function<void(const std::shared_ptr<MsgType>&)> callback);
    std::shared_ptr<Subscriber> subscribe(const std::string& topic,
                                         uint32_t queue_size, 
                                         const std::string& msg_type_name, 
                                         MessageQueue::Callback callback);
    template <typename MsgType>
    std::shared_ptr<Publisher<MsgType>> advertise(const std::string& topic);

    std::shared_ptr<Timer> createTimer(double period, const TimerCallback& callback, bool oneshot = false);
  private:
    NodeInfo nodeInfo_;
};
#include "node_handle.inl"
#endif //NODE_HANDLE_H

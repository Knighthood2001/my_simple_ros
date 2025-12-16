//
// Created by wu on 25-12-16.
//

#ifndef GLOBAL_INIT_H
#define GLOBAL_INIT_H
#include <memory>
#include <thread>
#include <muduo/net/EventLoop.h>
#include "poll_manager.h"
#include "message_queue.h"

class SystemManager{
  public:
    static SystemManager& instance();
    SystemManager(const SystemManager&) = delete;
    SystemManager& operator=(const SystemManager&) = delete;

    void init();
    void init(int port);

    void spin();
    void spinOnce();
    // 获取消息队列指针（供外部添加消息）
    std::shared_ptr<MessageQueue> getMessageQueue() const {return messageQueue_;}

  private:
    std::shared_ptr<muduo::net::EventLoop> eventLoop_;  // 事件循环
    std::shared_ptr<PollManager> pollManager_;  // 网络连接管理器
    std::thread eventThread_;  // 运行事件循环的后台线程

    std::shared_ptr<MessageQueue> messageQueue_;  // 消息队列
    bool running_ = true;  // 控制 spin 循环的标志

    // 私有构造/析构，确保只能通过 instance() 获取实例
    SystemManager() = default;
    ~SystemManager() = default;

    
};
#endif //GLOBAL_INIT_H

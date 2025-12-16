//
// Created by wu on 25-12-16.
//
#include <muduo/base/Logging.h>
#include "global_init.h"

SystemManager& SystemManager::instance(){
  static SystemManager instance_;
  return instance_;
}

void SystemManager::init(){
  init(12345);
}

//启动一个后台线程，在该线程中初始化网络事件循环（EventLoop）和网络连接管理器（PollManager），并运行事件循环以处理网络 IO 事件，从而避免阻塞主线程。
void SystemManager::init(int port){
  //启动后台线程运行事件循环
  eventThread_ = std::thread([this, port](){
    eventLoop_ = std::make_shared<muduo::net::EventLoop>(); // 创建事件循环实例
    
    muduo::net::InetAddress listenAddr("127.0.0.1", port);  // 创建监听地址
    // 网络连接管理器
    pollManager_ = std::make_shared<PollManager>(eventLoop_.get(), listenAddr);
    // 启动网络监听
    pollManager_->start();
    LOG_INFO<< "PollManager started on port: " << port;
    /*
    EventLoop::loop() 是一个阻塞函数，进入事件循环的主循环：
      1.不断检测注册的 IO 事件（如 socket 可读 / 可写）、定时器事件等。
      2.当事件发生时，调用对应的回调函数（例如 PollManager 注册的 “新连接回调”“数据读取回调”）。
      3.循环会一直运行，直到调用 eventLoop_->quit() 才会退出（通常在 shutdown() 中触发）。
    */
    eventLoop_->loop();
    
    //事件循环退出后清理资源,手动重置 pollManager_ 和 eventLoop_ 的智能指针，释放它们管理的资源，避免内存泄漏。
    pollManager_.reset();
    eventLoop_.reset();
  });
}
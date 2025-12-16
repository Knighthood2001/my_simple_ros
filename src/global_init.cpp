//
// Created by wu on 25-12-16.
//
#include <muduo/base/Logging.h>
#include <chrono>
#include "global_init.h"
#include "ros_rpc_client.h"

using namespace simple_ros;

SystemManager& SystemManager::instance(){
  static SystemManager instance_;
  return instance_;
}

void SystemManager::init(){
  init(12345);
}

//启动一个后台线程，在该线程中初始化网络事件循环（EventLoop）和网络连接管理器（PollManager），并运行事件循环以处理网络 IO 事件，从而避免阻塞主线程。
void SystemManager::init(int port){
  // 先初始化消息队列
  if(!messageQueue_){
    messageQueue_ = std::make_shared<MessageQueue>();
  }

  // 初始化全局RPC客户端，连接主服务器
  rpcClient_ = std::make_shared<RosRpcClient>("localhost:50051");
  LOG_INFO << "global RosRpcClient initialized";

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

void SystemManager::init(int port, std::string node_name){
  nodeInfo_.set_node_name(node_name);
  nodeInfo_.set_ip("127.0.0.1");
  nodeInfo_.set_port(port);
  LOG_INFO<< "NodeInfo initialized: name: " << node_name << " port: " << port;

  init(port);
}

void SystemManager::init(const std::string& node_name){
  int port = findAvailablePort();  // 自动找端口
  if (port<0){
    LOG_WARN<< "no available port found";
    throw std::runtime_error("No available port found");
  }
  init(port, node_name);
}
void SystemManager::spin(){
  while(running_){
    if(messageQueue_){
      // 队列中存储的是 “待执行的回调函数”，processCallbacks() 的作用是遍历消息队列，
      // 逐个执行这些回调函数—— 而这些回调函数的逻辑，正是 “处理对应的消息”（比如解析网络数据、响应 ROS 话题、执行异步任务）。
      messageQueue_->processCallbacks();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));   //// 避免占用过高CPU
  }
}
void SystemManager::spinOnce(){
  if(messageQueue_){
    messageQueue_->processCallbacks();// 单次处理
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

int SystemManager::findAvailablePort(int start_port, int end_port){
  for (int port = start_port; port <= end_port; ++port){
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) continue;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    // 绑定成功说明端口可用
    int bind_result = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    close(sock);
    if (bind_result == 0) return port;
  }
  return -1;
}

SystemManager::~SystemManager(){
  shutdown();
}

void SystemManager::shutdown(){
  // 先停业务 → 停事件循环 → 等线程退出 → 清理资源，确保
    //自旋循环（spin()）正常退出；
    //后台事件循环线程（eventThread_）安全停止，不触发崩溃；
    //所有核心资源（消息队列、事件循环、网络管理器）正确释放。
  running_ = false;  //这一步是 “停业务层”，先让主线程的自旋循环停止，避免关闭过程中还在处理新消息。
  if (eventLoop_){
    eventLoop_->runInLoop([this]{eventLoop_->quit();});
  }
  // 3. 等待后台事件线程完全退出
  if (eventThread_.joinable()){//如果不判断直接 join()，可能触发崩溃（比如线程未启动 / 已 join 过）
    eventThread_.join();
  }
  // 4. 显式清理消息队列资源
  messageQueue_.reset();
  LOG_INFO << "SystemManager shutdown";
}
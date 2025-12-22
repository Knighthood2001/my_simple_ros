//
// Created by wu on 25-12-20.
//
#include "global_init.h"
#include "message_queue.h"
#include "ros_rpc_client.h"
#include <iostream>
#include <thread>
#include "ros_rpc.pb.h"


int main(){
  try {
    // 1初始化systemmanager
    std::string node_name = "example_node";
    SystemManager& systemManager = SystemManager::instance();
    systemManager.init(node_name);

    NodeInfo nodeInfo = systemManager.getNodeInfo();
    std::cout << "Node initialized with name: " << nodeInfo.node_name() << ", IP: " << nodeInfo.ip() << ", Port: " << nodeInfo.port() << std::endl;

    auto rpcClient = systemManager.getRpcClient();
    if (rpcClient) {
      std::cout << "RPC client initialized" << std::endl;
    }

    std::shared_ptr<MessageQueue> messageQueue = systemManager.getMessageQueue();
    if (messageQueue) {
      std::cout << "Message queue initialized" << std::endl;
    }
    // 启动事件循环
    std::thread spinThread([&]() {
      std::cout << "starting spin loop ..." << std::endl;
      systemManager.spin();
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));
    // 这里可以插入一些内容，比如说模拟发布消息

    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "Shutting down system ..." << std::endl;
    systemManager.shutdown();

    if (spinThread.joinable()) {
      spinThread.join();
    }

    std::cout << "system shutdown complete" << std::endl;



  }catch (std::exception& e) {
    std::cerr << "Exception caught: "<< e.what() << std::endl;
    return -1;
  }
  return 0;
}
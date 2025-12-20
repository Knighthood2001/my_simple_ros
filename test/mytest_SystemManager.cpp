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
    std::string node_name = "example_node";
    SystemManager& systemManager = SystemManager::instance();
    systemManager.init(node_name);

    NodeInfo nodeInfo = systemManager.getNodeInfo();

  }catch (std::exception& e) {
    std::cerr << "Exception caught: "<< e.what() << std::endl;
    return -1;
  }
  return 0;
}
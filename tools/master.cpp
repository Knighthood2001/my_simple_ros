//
// Created by wu on 25-12-26.
//
#include <iostream>
#include <memory>
#include <thread>
#include <grpcpp/grpcpp.h>
#include "ros_rpc_server.h"
#include "master_tcp_server.h"

int main(){
  muduo::net::EventLoop loop;

  std::shared_ptr<simple_ros::MessageGraph> graph = std::make_shared<simple_ros::MessageGraph>();

  auto tcp_server = std::make_shared<simple_ros::MasterTcpServer>(&loop, graph);
  tcp_server->Start();

  std::string server_address("0.0.0.0:50051");
  simple_ros::RosRpcServer server(server_address, tcp_server, graph);

  std::thread server_thread(&simple_ros::RosRpcServer::Run, &server);

  loop.loop();

  server.Shutdown();
  if (server_thread.joinable()){
    server_thread.join();
  }
  return 0;
}
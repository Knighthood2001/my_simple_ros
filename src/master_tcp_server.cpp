//
// Created by wu on 25-12-11.
//
#include <muduo/base/Logging.h>
#include "master_tcp_server.h"

namespace simple_ros{
MasterTcpServer::MasterTcpServer(muduo::net::EventLoop* loop): loop_(loop), server_(loop, muduo::net::InetAddress(50052), "MasterTcpServer"){
  LOG_INFO << "MasterTcpServer initialized";
}
MasterTcpServer::~MasterTcpServer(){
  Stop();
  LOG_INFO << "MasterTcpServer destroyed";
}
void MasterTcpServer::Start(){
  server_.start();
  LOG_INFO << "MasterTcpServer started";
}

void MasterTcpServer::Stop(){
  server_.stop();
  LOG_INFO << "MasterTcpServer stopped";
}

}
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
void MasterTcpServer::OnConnection(const muduo::net::TcpConnectionPtr& conn){
  LOG_INFO << "Connection to " << conn->peerAddress().toIpPort() << "is" << (conn->connected() ? "UP" : "DOWN");
  std::lock_guard<std::mutex> lock(clients_mutex_);
  if (conn->connected()){
    active_clients_[conn->peerAddress().toIpPort()] = conn;
  }else{
    active_clients_.erase(conn->peerAddress().toIpPort());
  }
}
void MasterTcpServer::OnWriteComplete(const muduo::net::TcpConnectionPtr& conn){
  LOG_INFO << "Write complete to " << conn->peerAddress().toIpPort();
  // 发送完成后主动断开连接
  conn->shutdown();
}
}
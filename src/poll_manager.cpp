//
// Created by wu on 25-11-7.
//
#include "poll_manager.h"
#include <muduo/base/Logging.h>

PollManager::PollManager(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr)
    : server_(loop, listenAddr, "PollManager"){  //设置服务器名称 "PollManager"
  server_.setConnectionCallback(std::bind(&PollManager::onConnection, this, std::placeholders::_1));

}

void PollManager::start(){
  server_.start();
}

void PollManager::onConnection(const muduo::net::TcpConnectionPtr& conn){
  if (conn->isConnected()) { // 判断连接状态：是否是新建立的连接
    LOG_INFO << "new connection: " << conn->peerAddress().toIpPort();
  } else {
    LOG_INFO << "connection closed: " << conn->name();
  }
}
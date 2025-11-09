//
// Created by wu on 25-11-7.
//
#include "poll_manager.h"
#include <muduo/base/Logging.h>

PollManager::PollManager(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr)
    : server_(loop, listenAddr, "PollManager"){  //设置服务器名称 "PollManager"
  // 连接回调
  server_.setConnectionCallback(std::bind(&PollManager::onConnection, this, std::placeholders::_1));
  // 消息回调：绑定成员函数 onMessage（必须注册，否则数据不会被传递）
  server_.setMessageCallback(std::bind(&PollManager::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
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

void PollManager::setMessageCallback(std::function<void(const std::string&, const std::string&)> cb){
  messageCallback_ = std::move(cb);
}

void PollManager::onMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp){
  //后续进行优化
  while (buf->readableBytes() > 0){
    std::string msg(buf->retrieveAllAsString());
    if (messgaeCallback_) {
      messageCallBack_(conn->name(), msg);
    }
  }
}
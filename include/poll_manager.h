//
// Created by wu on 25-11-7.
//

#ifndef POLL_MANAGER_H
#define POLL_MANAGER_H
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <string>

class PollManager{
public:
  PollManager(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr);
  void start();

private:
  muduo::net::TcpServer server_;  //TCP服务器
  void onConnection(const muduo::net::TcpConnectionPtr& conn); // 处理连接事件


};

#endif //POLL_MANAGER_H

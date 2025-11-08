//
// Created by wu on 25-11-7.
//

#ifndef POLL_MANAGER_H
#define POLL_MANAGER_H
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <string>
/*
| 成员                       | 作用                 |
| ------------------------ | ------------------ |
| `EventLoop* loop`        | Muduo 的事件循环，IO复用核心 |
| `InetAddress listenAddr` | 服务器监听的 IP + 端口     |
| `TcpServer server_`      | Muduo 的 TCP 服务器对象  |
| `onConnection()`         | 连接事件回调函数           |
 */
class PollManager{
public:
  PollManager(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr);
  void start();

private:
  muduo::net::TcpServer server_;  //TCP服务器
  void onConnection(const muduo::net::TcpConnectionPtr& conn); // 处理连接事件


};

#endif //POLL_MANAGER_H

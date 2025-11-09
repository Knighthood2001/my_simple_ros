//
// Created by wu on 25-11-7.
//

#ifndef POLL_MANAGER_H
#define POLL_MANAGER_H
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <string>
#include <functional>
#include <muduo/net/Buffer.h>
#include <muduo/base/Timestamp.h>
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
  // 设置消息回调：参数 (connectionId, message)
  void setMessageCallback(std::function<void(const std::string&, const std::string&)> cb);

private:
  muduo::net::TcpServer server_;  //TCP服务器
  // Muduo 要求的回调签名：onConnection 和 onMessage
  void onConnection(const muduo::net::TcpConnectionPtr& conn); // 处理连接事件
  // 处理客户端发来的消息，从buf中读取数据并调用handleMessage进行处理
  void onMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp time);
  // 存储用户提供的回调
  // 回调函数，用于处理接收到的消息
  std::function<void(const std::string&, const std::string&)> messageCallback_;


};

#endif //POLL_MANAGER_H

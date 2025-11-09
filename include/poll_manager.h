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
#include <unordered_set>
#include <unordered_map>
#include <ros_rpc.pb.h>

using namespace simple_ros;
/*
| 成员                       | 作用                 |
| ------------------------ | ------------------ |
| `EventLoop* loop`        | Muduo 的事件循环，IO复用核心 |
| `InetAddress listenAddr` | 服务器监听的 IP + 端口     |
| `TcpServer server_`      | Muduo 的 TCP 服务器对象  |
| `onConnection()`         | 连接事件回调函数           |
 */
// 为 NodeInfo 对象生成哈希值，用于在 unordered_set 中快速查找(结合 IP 地址和端口号生成唯一标识)
struct NodeInfoHash{
  size_t operator()(const NodeInfo& n)const{
    return std::hash<std::string>()(n.ip())^(std::hash<int>()(n.port()) << 1);
  }
};

struct NodeInfoEqual{
  bool operator()(const NodeInfo& a, const NodeInfo& b)const{
    return a.ip() == b.ip() && a.port() == b.port();
  }
};
class PollManager{
public:
  PollManager(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr);
  void start();
  // 设置消息回调：参数 (connectionId, message)
  void setMessageCallback(std::function<void(const std::string&, const std::string&)> cb);
  //std::unordered_set<NodeInfo, NodeInfoHash, NodeInfoEqual> 用于存储订阅某个 Topic 的节点集合，保证节点 不重复，并支持 O(1) 高效插入、删除和查找。
  // 根据主题名称获取目标节点集合，允许外部调用者查询某个主题的目标节点，用于消息分发
  std::unordered_set<NodeInfo, NodeInfoHash, NodeInfoEqual> getTargets(const std::string& topic) const;

private:
  muduo::net::TcpServer server_;  //TCP服务器
  // Muduo 要求的回调签名：onConnection 和 onMessage
  void onConnection(const muduo::net::TcpConnectionPtr& conn); // 处理连接事件
  // 处理客户端发来的消息，从buf中读取数据并调用handleMessage进行处理
  void onMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp time);
  //解析消息内容，根据主题分发给目标节点
  void handleMessage(const std::string& topic, const std::string& msg_name, const std::string& data);
  // 存储用户提供的回调
  // 回调函数，用于处理接收到的消息
  std::function<void(const std::string&, const std::string&)> messageCallback_;
  // 键：话题名称（string）
  // 值：订阅该话题的所有节点集合
  // 优势：快速查找特定话题的所有订阅者，便于消息广播。
  std::unordered_map<std::string, std::unordered_set<NodeInfo, NodeInfoHash, NodeInfoEqual>> topic_targets_;


};

#endif //POLL_MANAGER_H

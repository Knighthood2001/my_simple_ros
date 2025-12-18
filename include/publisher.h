//
// Created by wu on 25-12-18.
//

#ifndef PUBLISHER_H
#define PUBLISHER_H
#include <string>
#include <google/protobuf/message.h>
#include <arpa/inet.h>
#include <muduo/base/Logging.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include "ros_rpc.pb.h"
#include <muduo/net/TcpClient.h>

using namespace simple_ros;

template <typename T>
class Publisher {
  public:
    Publisher(const std::string& topic);
    ~Publisher();
    void publish(const T& msg);
    void unregister();
    void updateTargets();  // 更新订阅节点
    void createClient(const NodeInfo& nodeInfo);  // 创建TCP客户端
    std::string getConnectionId(const NodeInfo& nodeInfo);  // 生成连接ID,比如127.0.0.1:8000
  private:
    std::string topic_; //话题名
    std::string msgType_;  // 消息类型全名
    NodeInfo nodeInfo_;  // 节点信息
    std::unordered_map<std::string, std::unique_ptr<muduo::net::TcpClient>> clients_;
    std::unordered_map<std::string, muduo::net::TcpConnectionPtr> connections_;

};

#include "publisher.inl"
#endif //PUBLISHER_H

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
    void publish(const T& msg);
    void updateTargets();
    void createClient(const NodeInfo& nodeInfo);
    std::string getConnectionId(const NodeInfo& nodeInfo);
  private:
    std::string topic_; //话题名
    std::string msgType_;  // 消息类型全名

    std::unordered_map<std::string, std::unique_ptr<muduo::net::TcpClient>> clients_;
    std::unordered_map<std::string, muduo::net::TcpConnectionPtr> connections_;

};

#include "publisher.inl"
#endif //PUBLISHER_H

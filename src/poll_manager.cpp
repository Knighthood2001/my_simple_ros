//
// Created by wu on 25-11-7.
//
#include "poll_manager.h"
#include <muduo/base/Logging.h>
#include "msg_factory.h"

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
  if (conn->connected()) { // 判断连接状态：是否是新建立的连接
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
    if (messageCallback_) {
      messageCallback_(conn->name(), msg);
    }
  }
}

std::unordered_set<NodeInfo, NodeInfoHash, NodeInfoEqual> PollManager::getTargets(const std::string& topic) const {
  auto it = topic_targets_.find(topic);
  if (it == topic_targets_.end()) {
    return {};
  }
  return it->second;
}

void PollManager::handleMessage(const std::string& topic,
                                const std::string& msg_name,
                                const std::string& data){
  
  // 2. 处理其他类型消息：通过消息工厂创建对应类型的消息对象并解析
  // 调用 MsgFactory 单例的 createMessage 方法，根据 msg_name 创建对应的消息对象
  auto msg = MsgFactory::instance().createMessage(msg_name);
  // 若消息对象创建失败，或解析原始数据失败（Protobuf反序列化），则记录警告并返回
  if (!msg || !msg->ParseFromString(data)){
    LOG_WARN << "Failed to create/parse message: " << msg_name;
    return;
  }
}
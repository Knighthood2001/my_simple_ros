//
// Created by wu on 25-12-11.
//

#ifndef MASTER_TCP_SERVER_H
#define MASTER_TCP_SERVER_H
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <mutex>
#include <unordered_map>
#include <muduo/net/TcpConnection.h>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "ros_rpc.pb.h"
#include "message_graph.h"

namespace simple_ros{

// 添加结构体来保存待发送的更新数据
struct PendingUpdate{
  NodeInfo node_info;
  TopicTargetsUpdate update;
};
class MasterTcpServer {
  public:
    MasterTcpServer(muduo::net::EventLoop* loop);
    ~MasterTcpServer();

    void Start();
    void Stop();
    // 发送更新给指定节点
    bool SendUpdate(const std::string& node_name, const TopicTargetsUpdate& update);

  private:

    //创建临时客户端连接并且发送更新
    void SendUpdateToNode(const NodeInfo& node_info, const TopicTargetsUpdate& update);
    
    // TCP客户端回调
    void OnConnection(const muduo::net::TcpConnectionPtr& conn);
    void OnWriteComplete(const muduo::net::TcpConnectionPtr& conn);

    muduo::net::EventLoop* loop_;
    muduo::net::TcpServer server_;
    
    std::unordered_map<std::string, muduo::net::TcpConnectionPtr> active_clients_;
    std::unordered_map<std::string, PendingUpdate> pending_updates_;
    std::mutex clients_mutex_;
    std::shared_ptr<MessageGraph> graph_;  // 使用shared_ptr
};

}
#endif //MASTER_TCP_SERVER_H

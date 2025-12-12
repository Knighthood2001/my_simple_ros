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
// muduo 的 TcpClient::connect() 是异步操作（调用后不会立刻建立连接），因此需要先把 “目标节点信息（node_info）” 和 “要发送的更新（update）” 暂存起来；
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
    // 对外暴露的核心接口：按节点名发送更新
    // 参数：node_name=目标节点名，update=要发送的更新数据
    // 返回值：bool=是否成功触发发送逻辑（非是否发送成功）
    bool SendUpdate(const std::string& node_name, const TopicTargetsUpdate& update);

  private:
    // 核心实现：创建临时 TCP 客户端，给指定节点发更新
    // 被 SendUpdate 调用，封装底层 TCP 逻辑
    void SendUpdateToNode(const NodeInfo& node_info, const TopicTargetsUpdate& update);
    
    // muduo 回调函数：处理 TCP 客户端的连接事件（成功/失败）
    void OnConnection(const muduo::net::TcpConnectionPtr& conn);
    // muduo 回调函数：处理数据写完成事件（发送完成后清理资源）
    void OnWriteComplete(const muduo::net::TcpConnectionPtr& conn);

    // 核心：muduo 事件循环（IO 线程的核心，所有网络操作绑定到该循环）
    muduo::net::EventLoop* loop_;
    // Master 自身的 TCP 服务端：监听端口，接收节点的连接/请求
    muduo::net::TcpServer server_;
    
    // 管理活跃的临时 TCP 客户端：key=ip:port（节点网络唯一标识），value=客户端智能指针
    // 作用：防止客户端提前析构（异步连接过程中客户端对象不能销毁）
    std::unordered_map<std::string, muduo::net::TcpConnectionPtr> active_clients_;
    // 缓存待发送的更新数据：key=ip:port，value=待发送的节点信息+更新数据
    std::unordered_map<std::string, PendingUpdate> pending_updates_;
    std::mutex clients_mutex_;
    // 节点拓扑图：管理所有节点的信息（节点名→NodeInfo），用于根据节点名查 IP/端口
    std::shared_ptr<MessageGraph> graph_;  // 使用shared_ptr：便于共享和生命周期管理
};

}
#endif //MASTER_TCP_SERVER_H

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

namespace simple_ros{
class MasterTcpServer {
  public:
    MasterTcpServer(muduo::net::EventLoop* loop);
    ~MasterTcpServer();

    void Start();
    void Stop();

  private:
    
    // TCP客户端回调
    void OnConnection(const muduo::net::TcpConnectionPtr& conn);
        void OnWriteComplete(const muduo::net::TcpConnectionPtr& conn);

    muduo::net::EventLoop* loop_;
    muduo::net::TcpServer server_;
    
    std::unordered_map<std::string, muduo::net::TcpConnectionPtr> active_clients_;
    
    std::mutex clients_mutex_;
};

}
#endif //MASTER_TCP_SERVER_H

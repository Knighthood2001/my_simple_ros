//
// Created by wu on 25-12-11.
//

#ifndef MASTER_TCP_SERVER_H
#define MASTER_TCP_SERVER_H
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

namespace simple_ros{
class MasterTcpServer {
  public:
    MasterTcpServer(muduo::net::EventLoop* loop);
    ~MasterTcpServer();

    void Start();
    void Stop();

  private:
    muduo::net::EventLoop* loop_;
    muduo::net::TcpServer server_;

};

}
#endif //MASTER_TCP_SERVER_H

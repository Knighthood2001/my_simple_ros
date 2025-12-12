//
// Created by wu on 25-12-11.
//
#include "master_tcp_server.h"
#include <iostream>
#include <arpa/inet.h> // htons htonl
#include <muduo/base/Logging.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <boost/bind/bind.hpp>
#include <sstream>
#include "master_tcp_server.h"
#include "ros_rpc.pb.h"
#include <muduo/base/Logging.h>
#include <memory>
namespace simple_ros{
MasterTcpServer::MasterTcpServer(muduo::net::EventLoop* loop): loop_(loop), server_(loop, muduo::net::InetAddress(50052), "MasterTcpServer"){
  LOG_INFO << "MasterTcpServer initialized";
}
MasterTcpServer::~MasterTcpServer(){
  Stop();
  LOG_INFO << "MasterTcpServer destroyed";
}
void MasterTcpServer::Start(){
  server_.start();
  LOG_INFO << "MasterTcpServer started";
}

void MasterTcpServer::Stop(){
  server_.stop();
  LOG_INFO << "MasterTcpServer stopped";
}
void MasterTcpServer::OnConnection(const muduo::net::TcpConnectionPtr& conn){
  LOG_INFO << "Connection to " << conn->peerAddress().toIpPort() << "is" << (conn->connected() ? "UP" : "DOWN");
  std::lock_guard<std::mutex> lock(clients_mutex_);
  if (conn->connected()){
    active_clients_[conn->peerAddress().toIpPort()] = conn;
  }else{
    active_clients_.erase(conn->peerAddress().toIpPort());
  }
}
void MasterTcpServer::OnWriteComplete(const muduo::net::TcpConnectionPtr& conn){
  LOG_INFO << "Write complete to " << conn->peerAddress().toIpPort();
  // 发送完成后主动断开连接
  conn->shutdown();
}

bool MasterTcpServer::SendUpdate(const std::string& node_name, const TopicTargetsUpdate& update){
  // 第一步：创建一个空的NodeInfo变量（“空容器”）
  NodeInfo node_info;

  // 第二步：调用graph_->GetNodeByName，核心逻辑：
  // - 输入：node_name（要找的节点名）
  // - 输出：&node_info（函数把查到的节点信息，写入这个变量）
  // - 返回值：bool（true=查到了，false=没查到）
  if (!graph_->GetNodeByName(node_name, &node_info)){
    LOG_WARN <<"Failed to send update: node not found -" << node_name;
  }
  // 查到了并且写入node_info
  // 将 SendUpdateToNode 这个 “给节点发更新” 的函数，封装成一个可执行的任务，
  // 交给 loop_（muduo 库的 EventLoop，即事件循环），由 EventLoop 所在的 IO 线程去执行这个任务 —— 而不是在当前调用 SendUpdate 的线程里立即执行。
  loop_->runInLoop(boost::bind(&MasterTcpServer::SendUpdateToNode, this, node_info, update));
}

void MasterTcpServer::SendUpdateToNode(const NodeInfo& node_info, const TopicTargetsUpdate& update){
  // 临时 TCP 连接创建 → 待发送数据缓存 → 异步连接 → 回调绑定
  if (node_info.ip().empty() || node_info.port() <= 0){
    LOG_WARN << "Invalid node address - ip: " << node_info.ip() <<", port: " << node_info.port();
    return;
  }
  muduo::net::InetAddress peer_addr(node_info.ip(), static_cast<uint16_t>(node_info.port()));//把「IP 字符串 + 端口号」转换成底层可识别的 socket 地址结构（如 sockaddr_in）。
  std::string peer_key = peer_addr.toIpPort();//生成 ip:port 格式的字符串（比如 192.168.1.100:8080），作为后续管理的唯一键。

  LOG_INFO << "Creating temporary connection to node: " << node_info.node_name() 
           << " at " << peer_key << " for topic: " << update.topic();
  // muduo 的 TcpClient::connect() 是异步操作—— 调用后不会立刻建立连接，而是由 IO 线程异步处理连接请求，连接结果会在 OnConnection 回调中通知。
  // 因此，必须先把 “要发送的更新数据” 存起来，等连接成功后再取出来发送。
  {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    pending_updates_[peer_key] = {node_info, update};
  }
  // muduo 封装的 TCP 客户端类，用于主动发起 TCP 连接
  // TcpClient 是普通对象，若不用智能指针，函数执行完后 client 会析构 —— 但此时异步连接还在进行中，析构会导致 socket 资源异常释放，程序崩溃。
  // shared_ptr 能保证：只要连接未完成 / 未销毁，TcpClient 就不会被析构。
  auto client = std::make_shared<muduo::net::TcpClient>(loop_, peer_addr, "MasterTcpClient-"+node_info.node_name());
  //muduo 的 TcpClient 依赖「回调机制」处理异步事件，这里绑定两个核心回调：
  //连接回调（OnConnection）：
    //当 TCP 连接 “建立成功” 或 “连接失败 / 断开” 时触发，参数 boost::placeholders::_1 是 muduo 传入的 TcpConnectionPtr（TCP 连接的智能指针）。
    //核心作用：连接成功后，从 pending_updates_ 取出数据，通过该连接发送；连接失败则清理缓存数据。
  //写完成回调（OnWriteComplete）：
    //当数据成功写入 socket 缓冲区（注意：不是对方已接收，是本地发送完成）时触发。
    //核心作用：发送完成后，清理临时连接和缓存数据，避免资源泄漏。
  client->setConnectionCallback(boost::bind(&MasterTcpServer::OnConnection, this, boost::placeholders::_1));
  client->setWriteCompleteCallback(boost::bind(&MasterTcpServer::OnWriteComplete, this, boost::placeholders::_1));
  // 保存 Client 指针：防止提前析构
  {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    active_clients_[peer_key] = client;
  }
  // 告诉 muduo 的 IO 线程 “发起对 peer_addr 的 TCP 连接”，调用后立刻返回（异步），不会阻塞当前线程。
  client->connect();
}
}
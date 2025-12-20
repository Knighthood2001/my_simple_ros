//
// Created by wu on 25-12-2.
//

#ifndef ROS_RPC_CLIENT_H
#define ROS_RPC_CLIENT_H
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "ros_rpc.grpc.pb.h"

namespace simple_ros{
class RosRpcClient{
  public:
    //显式构造函数
    explicit RosRpcClient(const std::string& server_address);
    ~RosRpcClient() = default;
    bool Subscribe(const std::string& topic_name,
                  const std::string& msg_type,
                  const NodeInfo& node_info,
                  SubscribeResponse* response);

    bool Unsubscribe(const std::string& topic_name,
                    const std::string& msg_type,
                    const NodeInfo& node_info,
                    UnsubscribeResponse* response);
    // 调用RegisterPublisher RPC
    bool RegisterPublisher(const std::string& topic_name,
                           const std::string& msg_type,
                           const NodeInfo& node_info,
                           RegisterPublisherResponse* response);

    bool UnregisterPublisher(const std::string& topic_name,
                             const std::string& msg_type,
                             const NodeInfo& node_info,
                             UnregisterPublisherResponse* response);

    //添加查询功能:支持查询节点和话题的信息。
    // 获取节点列表
    bool GetNodes(const std::string& filter, GetNodesResponse* response);

    bool GetNodeInfo(const std::string& node_name, GetNodeInfoResponse* response);

    bool GetTopics(const std::string& filter, GetTopicsResponse* response);

    bool GetTopicInfo(const std::string& topic_name, GetTopicInfoResponse* response);
  private:
    //RosRpcService::Stub 是 gRPC 自动生成的客户端存根类，用于调用服务端定义的 RPC 方法。
    std::unique_ptr<RosRpcService::Stub> stub_;  // 使用 std::unique_ptr 管理 Stub
};
}
#endif //ROS_RPC_CLIENT_H

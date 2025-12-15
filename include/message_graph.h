//
// Created by wu on 25-11-11.
//

#ifndef MESSAGE_GRAPH_H
#define MESSAGE_GRAPH_H
#include <memory>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <grpcpp/grpcpp.h>
#include <ros_rpc.grpc.pb.h>

namespace simple_ros{
  struct TopicKey{
    std::string topic;
    std::string msg_type;
    bool operator==(const TopicKey& other)const noexcept {
      return topic == other.topic && msg_type == other.msg_type;
    }
  };
  struct TopicKeyHash{
    size_t operator()(const TopicKey& key)const noexcept {
      return std::hash<std::string>()(key.topic)^(std::hash<std::string>()(key.msg_type)<<1);
    }
  };
  struct Edge{
    std::string src_node; // 发布者
    std::string dst_node; // 订阅者
    TopicKey key;
    bool operator==(const Edge& other) const noexcept {
      return src_node == other.src_node && dst_node == other.dst_node &&
             key.topic == other.key.topic && key.msg_type == other.key.msg_type;
    }
  };
  struct EdgeHash{
    size_t operator()(const Edge& edge) const noexcept {
      size_t h1 = std::hash<std::string>()(edge.src_node);
      size_t h2 = std::hash<std::string>()(edge.dst_node);
      size_t h3 = std::hash<std::string>()(edge.key.topic);
      size_t h4 = std::hash<std::string>()(edge.key.msg_type);
      return (((h1^(h2<<1))^(h3<<1))^(h4<<1));
    }
  };
  //每个节点可能同时是发布者和订阅者，因此需要存储它发布和订阅的话题。
  struct NodeVertex{
    NodeInfo info;//ip port node_name
    //集合无序，不会重复，存放该节点发布/订阅的topic，msg
    std::unordered_set<TopicKey, TopicKeyHash> publishes;
    std::unordered_set<TopicKey, TopicKeyHash> subscribes;
  };
  /*
  节点管理：
    UpsertNode：新增或更新节点信息。
    GetNodeByName：通过节点名称获取节点信息。
  发布/订阅关系管理：
    AddPublisher：添加发布者。
    AddSubscriber：添加订阅者。
    RemovePublisher：移除发布者。
    RemoveSubscriber：移除订阅者。
  查询功能：
    GetSubscribersByTopic：获取某个话题的订阅者。
    GetPublishersByTopic：获取某个话题的发布者。
  导出功能：
    ToReadableString：导出为可读字符串。
    ToDOT：导出为 Graphviz 的 DOT 格式。
    ToJSON：导出为 JSON 格式。
  */
  class MessageGraph {
  public:
    void UpsertNode(const NodeInfo &info);
    bool GetNodeByName(const std::string& node_name, NodeInfo *node_info);

    // 维护 topic/msg 到 发布者/订阅者 的索引，并即时建边
    void AddPublisher(const NodeInfo &node, const TopicKey &k);
    void AddSubscriber(const NodeInfo &node, const TopicKey &k);
    // 删除发布/订阅关系，并相应删边；必要时清理孤立点
    void RemovePublisher(const NodeInfo &node, const TopicKey &k);
    void RemoveSubscriber(const NodeInfo &node, const TopicKey &k);

    //导出
    std::string ToReadableString() const;
    std::string ToDOT() const;
    std::string ToJSON() const;

    std::vector<NodeInfo> GetAllNodes() const;
    //是否存在node_name的Node
    bool HasNode(const std::string& node_name) const;

    std::vector<NodeInfo> GetSubscribersByTopic(const std::string &topic) const;
    std::vector<NodeInfo> GetPublishersByTopic(const std::string &topic) const;
    // 获取节点发布/订阅的所有话题
    std::vector<std::string> GetNodePublishTopics(const std::string& node_name) const;
    std::vector<std::string> GetNodeSubscribeTopics(const std::string& node_name) const;
    // 获取节点发布/订阅的所有话题（包含消息类型）
    std::vector<TopicKey> GetNodePublishTopicKeys(const std::string &node_name) const;
    std::vector<TopicKey> GetNodeSubscribeTopicKeys(const std::string &node_name) const;

  private:
    //以 “节点名” 为键，存储所有节点的完整信息
    std::unordered_map<std::string, NodeVertex> nodes_;
    //以 “话题键（TopicKey）” 为键，分别存储该话题的所有发布者节点名、订阅者节点名。
    //节点名集合用unordered_set存储，确保去重（同一节点不会重复添加到同一话题的发布者 / 订阅者中），且插入 / 删除 / 查找效率为 O (1)。
    std::unordered_map<TopicKey, std::unordered_set<std::string>, TopicKeyHash> publishers_by_topic_;
    std::unordered_map<TopicKey, std::unordered_set<std::string>, TopicKeyHash> subscribers_by_topic_;

    //存储所有 “发布者 - 订阅者” 之间的边（Edge），代表节点间的消息流向关系。
    std::unordered_set<Edge, EdgeHash> edges_;

    //新增发布者时，需要关联已存在的订阅者；
    //新增订阅者时，需要关联已存在的发布者。
    void ConnectPublisherToSubscribers(const std::string& pub_node, const TopicKey& k);
    void ConnectPublishersToSubscriber(const std::string& sub_node, const TopicKey& k);

    //从消息图中移除与特定节点和主题相关的边（连接关系）
    void RemoveEdgesBy(const std::string& node, const TopicKey& k, bool node_is_publisher);
    
    // 可选：清理完全没有发布/订阅与边的孤立节点（避免积累）
    void CleanupIsolatedNodeIfAny(const std::string &node_name);


  };

};

#endif //MESSAGE_GRAPH_H

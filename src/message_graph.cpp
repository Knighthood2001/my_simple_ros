//
// Created by wu on 25-11-11.
//
#include <sstream>

#include "message_graph.h"

namespace simple_ros {
  void MessageGraph::UpsertNode(const NodeInfo &info){
    auto& v = nodes_[info.node_name()];
    v.info = info;// 不用去管发布者和订阅者
  }
  bool MessageGraph::GetNodeByName(const std::string& node_name, NodeInfo *node_info){
    auto it = nodes_.find(node_name);
    if (it == nodes_.end()){return false;}
    // 通过解引用，我们才能将it->second.info的值赋值给node_info指针所指向的外部NodeInfo对象。
    *node_info = it->second.info;  // NodeInfo info;//ip port node_name
    return true;
  }

  //根据话题（TopicKey）将相关的发布者和订阅者通过 “边” 关联起来
  void MessageGraph::ConnectPublisherToSubscribers(const std::string& pub_node, const TopicKey& k){
    // 步骤1：查找订阅了话题k的所有订阅者节点
    auto it = subscribers_by_topic_.find(k);
    // 如果没有订阅者，直接返回（无需建边）
    if (it == subscribers_by_topic_.end()) return;
    // 步骤2：遍历所有订阅者，为每个订阅者与当前发布者建立边
    for (const auto& sub : it->second) {
      edges_.insert(Edge{pub_node, sub, k});
    }
  }

  void MessageGraph::ConnectPublishersToSubscriber(const std::string& sub_node, const TopicKey& k){
    auto it = publishers_by_topic_.find(k);
    if (it == publishers_by_topic_.end()) return;
    for (const auto& pub : it->second) {
      edges_.insert(Edge{pub, sub_node, k});
    }
  }

  void MessageGraph::AddPublisher(const NodeInfo& node, const TopicKey& k){
    // node是ip port node_name,k是topic msg_type
    UpsertNode(node);

    nodes_[node.node_name()].publishes.insert(k);

    publishers_by_topic_[k].insert(node.node_name());

    ConnectPublisherToSubscribers(node.node_name(), k);

  }

  void MessageGraph::AddSubscriber(const NodeInfo& node, const TopicKey& k){
    UpsertNode(node);
    nodes_[node.node_name()].subscribes.insert(k);
    subscribers_by_topic_[k].insert(node.node_name());
    ConnectPublishersToSubscriber(node.node_name(), k);
  }

  void MessageGraph::RemoveEdgesBy(const std::string& node, const TopicKey& k, bool node_is_publisher){
    // 由于 edges_ 是 unordered_set，只能线性扫描；但边规模通常 << 节点规模，且仅在注销/退订时发生。
    std::vector<Edge> to_erase;  // src_node dst_node 以及TopicKey key
    to_erase.reserve(16);// 预留16个
    /*
    对于每条边，检查：
        边的主题和消息类型是否与参数匹配
        根据 node_is_publisher 判断是移除该节点作为源（发布者）还是目标（订阅者）的边
    */
    for (const auto& e : edges_){  //std::unordered_set<Edge, EdgeHash> edges_;
      if (e.key.topic == k.topic && e.key.msg_type == k.msg_type){
        //节点取消发布某个主题时（node_is_publisher = true）
        if (node_is_publisher && e.src_node == node){
          to_erase.push_back(e);
        }
        //节点取消订阅某个主题时（node_is_publisher = false）
        if(!node_is_publisher && e.dst_node == node){
          to_erase.push_back(e);
        }
      }
    }
    for (auto& e : to_erase){
      edges_.erase(e);
    }
  }
  //从消息图中移除一个发布者（publisher）节点及其相关连接。
  void MessageGraph::RemovePublisher(const NodeInfo& node, const TopicKey& k){
    auto itn = nodes_.find(node.node_name());  //std::unordered_map<std::string, NodeVertex> nodes_
    if (itn != nodes_.end()){
      itn->second.publishes.erase(k);       // 从节点的发布列表中移除该主题
    }
    auto itp = publishers_by_topic_.find(k);//std::unordered_map<TopicKey, std::unordered_set<std::string>, TopicKeyHash> publishers_by_topic_;
    if (itp != publishers_by_topic_.end()){
      itp->second.erase(node.node_name());  // 从该主题的发布者列表中移除节点
      if (itp->second.empty()){
        publishers_by_topic_.erase(itp);    // 如果列表为空则移除整个主题条目
      }
    }
    RemoveEdgesBy(node.node_name(), k, true);
    CleanupIsolatedNodeIfAny(node.node_name());
  }
  void MessageGraph::RemoveSubscriber(const NodeInfo &node, const TopicKey &k){
    auto itn = nodes_.find(node.node_name());
    if (itn != nodes_.end()){
      itn->second.subscribes.erase(k);
    }
    auto its = subscribers_by_topic_.find(k);
    if (its != subscribers_by_topic_.end()){
      its->second.erase(node.node_name());
      if (its->second.empty()){
        subscribers_by_topic_.erase(its);
      }
    }
    RemoveEdgesBy(node.node_name(), k, false);
    CleanupIsolatedNodeIfAny(node.node_name());
  }
  void MessageGraph::CleanupIsolatedNodeIfAny(const std::string &node_name){
    //查找节点
    auto it = nodes_.find(node_name);
    if (it == nodes_.end()) return;

    //检查发布和订阅关系,如果节点有任一关系（ has_pub 或 has_sub 为 true ），说明不是孤立节点，直接返回
    bool has_pub = !(it->second.publishes.empty());
    bool has_sub = !(it->second.subscribes.empty());
    if (has_pub || has_sub) return;
    
    //检查边的连接.遍历所有边（ edges_ ），检查是否有边以该节点为起点（ src_node ）或终点（ dst_node ）。
    //如果存在这样的边，说明节点仍与其他节点有连接，直接返回。
    for (const auto& e : edges_){
      if (e.src_node == node_name || e.dst_node == node_name) return;
    }
    nodes_.erase(it);
  }

  /*
    ==== Message Graph ====
    Nodes: 3, Edges: 2

    [Nodes]
    - sensor_camera (ip=192.168.1.10, port=8080)
        publishes:
          - image_raw : sensor_msgs/Image
    - process_image (ip=192.168.1.11, port=8081)
        publishes:
          - image_processed : sensor_msgs/CompressedImage
        subscribes:
          - image_raw : sensor_msgs/Image
    - display_screen (ip=192.168.1.12, port=8082)
        subscribes:
          - image_processed : sensor_msgs/CompressedImage

    [Edges]
    - sensor_camera -> process_image  [image_raw : sensor_msgs/Image]
    - process_image -> display_screen  [image_processed : sensor_msgs/CompressedImage]
  */
  std::string MessageGraph::ToReadableString() const {
    std::ostringstream oss;
    oss << "==== Message Graph ====\n";
    oss << "Nodes: " << nodes_.size() << ", Edges: " << edges_.size() << "\n\n";
    oss << "[Nodes]\n";
    for (const auto& [name, v] : nodes_) {
        oss << " - " << name << " (ip=" << v.info.ip() << ", port=" << v.info.port() << ")\n";
        if (!v.publishes.empty()) {
            oss << "    publishes:\n";
            for (const auto& k : v.publishes) oss << "      - " << k.topic << " : " << k.msg_type << "\n";
        }
        if (!v.subscribes.empty()) {
            oss << "    subscribes:\n";
            for (const auto& k : v.subscribes) oss << "      - " << k.topic << " : " << k.msg_type << "\n";
        }
    }
    oss << "\n[Edges]\n";
    for (const auto& e : edges_) {
        oss << " - " << e.src_node << " -> " << e.dst_node
            << "  [" << e.key.topic << " : " << e.key.msg_type << "]\n";
    }
    return oss.str();
  }
  
  std::vector<NodeInfo> MessageGraph::GetAllNodes() const {
    std::vector<NodeInfo> result;
    for (const auto& [name, v] : nodes_) {
      result.push_back(v.info);
    }
    return result;
  }
  bool MessageGraph::HasNode(const std::string& node_name) const {
    auto it = nodes_.find(node_name);
    if (it == nodes_.end()) return false;
    return true;
  }

  std::vector<std::string> MessageGraph::GetNodePublishTopics(const std::string& node_name) const {
    std::vector<std::string> result;
    auto it = nodes_.find(node_name);
    if (it != nodes_.end()){
      for (const auto& topic_key : it->second.publishes) {
        result.push_back(topic_key.topic);
      }
    }
    return result;
  }
  
  std::vector<std::string> MessageGraph::GetNodeSubscribeTopics(const std::string& node_name) const {
    std::vector<std::string> result;
    auto it = nodes_.find(node_name);
    if (it != nodes_.end()){
      for (const auto& topic_key : it->second.subscribes) {
        result.push_back(topic_key.topic);
      }
    }
    return result;
  }

  std::vector<TopicKey> MessageGraph::GetNodePublishTopicKeys(const std::string &node_name) const{
    std::vector<TopicKey> result;
    auto it = nodes_.find(node_name);
    if (it != nodes_.end()){
      for (const auto& topic_key : it->second.publishes){
        result.push_back(topic_key);
      }
    }
    return result;
  }
  std::vector<TopicKey> MessageGraph::GetNodeSubscribeTopicKeys(const std::string &node_name) const {
    std::vector<TopicKey> result;
    auto it = nodes_.find(node_name);
    if (it != nodes_.end()){
      for (const auto& topic_key : it->second.subscribes){
        result.push_back(topic_key);
      }
    }
    return result;
  }

}
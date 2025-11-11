//
// Created by wu on 25-11-11.
//
#include "message_graph.h"
namespace simple_ros {
  void MessageGraph::UpsertNode(const NodeInfo &info){
    auto& v = nodes_[info.node_name()];
    v.info = info;// 不用去管发布者和订阅者
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



}
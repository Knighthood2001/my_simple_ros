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
};
#endif //MESSAGE_GRAPH_H

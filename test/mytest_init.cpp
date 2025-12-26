//
// Created by wu on 25-12-26.
//
#include "global_init.h"
#include "msg_factory.h"
#include "example.pb.h"
#include <gtest/gtest.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/InetAddress.h>
#include <muduo/base/Logging.h>
#include <thread>
#include <chrono>
#include <memory>
#include <atomic>
#include "node_handle.h"

int main(){
  SystemManager& sys = SystemManager::instance();
  sys.init();
  LOG_INFO << "System initialized successfully";
  // std::this_thread::sleep_for(std::chrono::milliseconds(200));

  MsgFactory::instance().registerMessage<example::SensorData>();

  std::atomic<bool> message_received(false);
  std::atomic<int32_t> received_sensor_id(-1);
  std::atomic<float> received_value(-1.0f);

  // 注册topic
  const std::string topic_name = "test_topic";

  NodeHandle nh;
  std::shared_ptr<Subscriber> test_sub = nh.subscribe<example::SensorData>(topic_name, 10,
    [&](const std::shared_ptr<example::SensorData>& sensor_data) {
        LOG_INFO << "Received message on topic '" << topic_name << "'";
        message_received = true;
        received_sensor_id = sensor_data->sensor_id();
        received_value = sensor_data->value();
        LOG_INFO << "Sensor ID: " << received_sensor_id
                 << ", Value: " << received_value;
  });
  example::SensorData sensor;
  sensor.set_sensor_id(100);
  sensor.set_value(2.718f);

  std::string data;

  if (!sensor.SerializeToString(&data)) {
    LOG_ERROR<<"Failed to serialize message";
  }
  std::string msg_name = "example.SensorData";
  uint16_t topic_len = htons(static_cast<uint16_t>(topic_name.size()));
  uint16_t name_len = htons(static_cast<uint16_t>(msg_name.size()));
  uint32_t msg_len = htonl(static_cast<uint32_t>(data.size()));

  std::string buffer;
  buffer.append(reinterpret_cast<const char*>(&topic_len), sizeof(topic_len));
  buffer.append(topic_name);
  buffer.append(reinterpret_cast<const char*>(&name_len), sizeof(name_len));
  buffer.append(msg_name);
  buffer.append(reinterpret_cast<const char*>(&msg_len), sizeof(msg_len));
  buffer.append(data);

  muduo::net::InetAddress serverAddr("127.0.0.1", 12345);
  muduo::net::EventLoop clientLoop;
  muduo::net::TcpClient client(&clientLoop, serverAddr,"InitTestClient");

  bool client_connected = false;
  client.setConnectionCallback([&](const muduo::net::TcpConnectionPtr& conn) {
    if (conn->connected()) {
      LOG_INFO << "Client connected to server: " << conn->peerAddress().toIpPort();
      client_connected = true;
      conn->send(buffer);
    } else {
      LOG_WARN << "Client disconnected: " << conn->name();
    }
  });
  client.setWriteCompleteCallback([&clientLoop](const muduo::net::TcpConnectionPtr& conn){
      LOG_INFO << "Message sent successfully: " << conn->name();
      clientLoop.quit(); // 发送完成后退出客户端 EventLoop
  });
  client.connect();
  clientLoop.loop();

  LOG_INFO << "Message begin: ";
  sys.spinOnce();
  LOG_INFO << "Message end: " ;

  LOG_INFO<< "Message received: "<< message_received;
  LOG_INFO << "received_sensor_id: " << received_sensor_id;
  LOG_INFO << "received_value: " << received_value;


  LOG_INFO << "Test completed";

  sys.shutdown();




}
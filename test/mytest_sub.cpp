//
// Created by wu on 25-12-26.
//
#include "global_init.h"
#include "example.pb.h"
#include "node_handle.h"

int main(){
  SystemManager& sys = SystemManager::instance();
  sys.init(12345, "subscriber_node");

  NodeHandle nh;
  auto pub = nh.advertise<example::SensorData>("/echo_topic");

  auto sub = nh.subscribe<example::SensorData>("/test_topic", 10,
    [&](const std::shared_ptr<example::SensorData>& msg){
    LOG_INFO << "Echo received message: sensor_id= "<< msg->sensor_id() << ", value= "<< msg->value();

    example::SensorData reply;
    reply.set_sensor_id(msg->sensor_id());
    reply.set_value(msg->value());
    pub->publish(reply);
    LOG_INFO << "Echo published message back: sensor_id= " << msg->sensor_id() << ", value= " << msg->value();
  });

  while(true){
    sys.spinOnce();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  sys.shutdown();
  return 0;
}
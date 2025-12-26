//
// Created by wu on 25-12-26.
//
#include "node_handle.h"
#include "example.pb.h"
#include <iostream>
int main(){
  SystemManager& systemManager = SystemManager::instance();
  systemManager.init("publisher_node");

  NodeHandle nh;

  auto publisher = nh.advertise<example::SensorData>("/sensor_data");

  example::SensorData sensor_data;
  sensor_data.set_sensor_id(1);
  sensor_data.set_value(3.14f);
  publisher->publish(sensor_data);
  // std::cout << "第一次发送消息" << std::endl;

  // auto timer = nh.createTimer(2.0, [&publisher](const simple_ros::TimerEvent& event) {
  //   example::SensorData sensor_data2;
  //   sensor_data2.set_sensor_id(2);
  //   sensor_data2.set_value(4.0f);
  //   publisher->publish(sensor_data2);
  //   std::cout << "第二次发送消息" << std::endl;
  // }, true);

  systemManager.spin();
  return 0;
}
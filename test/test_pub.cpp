//
// Created by wu on 26-1-9.
//
#include <thread>
#include <chrono>
#include <memory>
#include <muduo/base/Logging.h>
#include "global_init.h"
#include "example.pb.h"
#include "node_handle.h"

class PublisherNode {
  public:
    PublisherNode(int port, const std::string& nodeName):port_(port), nodeName_(nodeName), counter_(0) {}

    void initialize(){
      auto& sys = SystemManager::instance();
      sys.init(port_, nodeName_);
      std::this_thread::sleep_for(std::chrono::milliseconds(200));

      nh_ = std::make_shared<NodeHandle>();
      pub_ = nh_->advertise<example::SensorData>("test_topic");

      sub_ = nh_->subscribe<example::SensorData>("echo_topic", 10, std::bind(&PublisherNode::echoCallback, this, std::placeholders::_1));

      timer_ = nh_->createTimer(1.0, std::bind(&PublisherNode::timerCallback, this,std::placeholders::_1), false);
    }

    void run(){
      auto& sys = SystemManager::instance();
      sys.spin();
    }
    void shutdown(){
      auto& sys = SystemManager::instance();
      sys.shutdown();
    }
  private:
    int port_;
    std::string nodeName_;
    int counter_;
    std::shared_ptr<NodeHandle> nh_;
    std::shared_ptr<Publisher<example::SensorData>> pub_;
    std::shared_ptr<Subscriber> sub_;
    std::shared_ptr<Timer> timer_;
    void echoCallback(const std::shared_ptr<example::SensorData>& msg){
      LOG_INFO << "Echo received message: sensor_id="<< msg->sensor_id() << ", value=" <<msg->value();
    }
    void timerCallback(const TimerEvent& event){
      example::SensorData sensor;
      sensor.set_sensor_id(100+counter_);
      sensor.set_value(3.14f+ counter_);
      pub_->publish(sensor);
      LOG_INFO << "Published message: sensor_id=" << sensor.sensor_id() << ", value=" <<sensor.value()<< ", counter="  << counter_;
      counter_++;
    }
};

int main(){
  PublisherNode node(12346, "publisher_node");
  node.initialize();

  node.run();

  node.shutdown();
  return 0;
}
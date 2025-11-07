//
// Created by wu on 25-11-7.
//

#include "msg_factory.h"
#include "example.pb.h"
#include <iostream>

int main() {

  MsgFactory::instance().registerMessage<example::SensorData>();
  MsgFactory::instance().registerMessage<example::ControlCommand>();
  MsgFactory::instance().registerMessage<example::Heartbeat>();

  auto sensorMsg = MsgFactory::instance().createMessage("example.SensorData");
  if (sensorMsg) {
    auto* sensorData = dynamic_cast<example::SensorData*>(sensorMsg.get());
    if (sensorData) {
      sensorData->set_sensor_id(1);
      sensorData->set_value(42.5);
      sensorData->set_timestamp(123456789);
      
      std::cout << "SensorData:" << std::endl;
      std::cout << "  Sensor ID: " << sensorData->sensor_id() << std::endl;
      std::cout << "  Value: " << sensorData->value() << std::endl;
      std::cout << "  Timestamp: " << sensorData->timestamp() << std::endl;
    }
  } else {
        std::cout << "Failed to create SensorData message!" << std::endl;
  }
  // 动态创建 ControlCommand 消息
  auto controlMsg = MsgFactory::instance().createMessage("example.ControlCommand");
  if (controlMsg) {
      auto* controlCommand = dynamic_cast<example::ControlCommand*>(controlMsg.get());
      if (controlCommand) {
          controlCommand->set_cmd_id(1);
          controlCommand->set_cmd("START");

          std::cout << "\nControlCommand:" << std::endl;
          std::cout << "  Command ID: " << controlCommand->cmd_id() << std::endl;
          std::cout << "  Command: " << controlCommand->cmd() << std::endl;
      }
  } else {
      std::cout << "Failed to create ControlCommand message!" << std::endl;
  }

  // 动态创建 Heartbeat 消息
  auto heartbeatMsg = MsgFactory::instance().createMessage("example.Heartbeat");
  if (heartbeatMsg) {
      auto* heartbeat = dynamic_cast<example::Heartbeat*>(heartbeatMsg.get());
      if (heartbeat) {
          heartbeat->set_timestamp(1672531200000);

          std::cout << "\nHeartbeat:" << std::endl;
          std::cout << "  Timestamp: " << heartbeat->timestamp() << std::endl;
      }
  } else {
      std::cout << "Failed to create Heartbeat message!" << std::endl;
  }
  return 0;
}
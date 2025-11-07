//
// Created by wu on 25-11-7.
//

#include "msg_factory.h"
#include "example.pb.h"
#include <iostream>

int main() {

  MsgFactory::instance().registerMessage<example::SensorData>();
  return 0;
}
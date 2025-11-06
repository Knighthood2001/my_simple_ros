//
// Created by wu on 25-11-6.
//
#include "msg_factory.h"

MsgFactory& MsgFactory::instance(){
  static MsgFactory instance;
  return instance;
}
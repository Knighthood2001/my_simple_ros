//
// Created by wu on 25-12-16.
//
#include "global_init.h"

SystemManager& SystemManager::instance(){
  static SystemManager instance_;
  return instance_;
}

void SystemManager::init(){
  init(12345);
}

void SystemManager::init(int port){
  // 临时打印，标记初始化完成
  printf("System initialized with port: %d\n", port);
}
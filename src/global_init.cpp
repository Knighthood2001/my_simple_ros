//
// Created by wu on 25-12-16.
//
#include "global_init.h"

SystemManager& SystemManager::instance(){
  static SystemManager instance_;
  return instance_;
}
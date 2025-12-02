//
// Created by wu on 25-11-18.
//

#include "timer.h"

namespace simple_ros {

Timer::Timer(muduo::net::EventLoop* loop, double period, const TimerCallback& callback)
  : loop_(loop), period_(period), callback_(callback) {}

Timer::~Timer(){
  stop();
}

void Timer::start(){
  if (isRunning_) return;
  isRunning_ = true;
  timerId_ = loop_->runEvery(period_, callback_);
}

void Timer::stop(){
  if (!isRunning_) return;
  loop_->cancel(timerId_);
  isRunning_ = false;
}
}


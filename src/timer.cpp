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

  if (isOneShot_) {
    timerId_ = loop_->runAfter(period_, callback_); //单次定时器
  }
  else{
    timerId_ = loop_->runEvery(period_, callback_); //重复定时器
  }
}

void Timer::stop(){
  if (!isRunning_) return;
  loop_->cancel(timerId_);
  isRunning_ = false;
}

void Timer::setOneShot(bool oneshot){
  isOneShot_= oneshot;
}

void Timer::pause(){
  if (!isRunning_ || isPaused_) return;
  loop_->cancel(timerId_);
  isRunning_ = false;
  isPaused_ = true;

}
//恢复一个被暂停的计时器
void Timer::resume(){
  if (!isRunning_ || isPaused_) return;
  isRunning_ = true;
  isPaused_ = false;

  double currentTime = muduo::Timestamp::now().secondsSinceEpoch();
  double elapsed = currentTime - lastEvent_.current_real;
  double remaining = period_ - std::fmod(elapsed, period_);

  if(remaining < 0) remaining = 0;

  if(isOneShot_){
    timerId_ = loop_->runAfter(remaining, std::bind(&Timer::internalCallback, this));
  }else{
    timerId_ = loop_->runEvery(remaining, std::bind(&Timer::internalCallback, this));
  }
}
}


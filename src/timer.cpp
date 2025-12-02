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
    timerId_ = loop_->runAfter(period_, std::bind(&Timer::internalCallback, this)); //单次定时器
  }
  else{
    timerId_ = loop_->runEvery(period_, std::bind(&Timer::internalCallback, this)); //重复定时器
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

void Timer::internalCallback(){
  double startTime = muduo::Timestamp::now().secondsSinceEpoch();

  //更新事件信息
  TimerEvent event;
  event.current_real = startTime;
  event.last_real = lastEvent_.current_real;
  event.expected_real = lastEvent_.expected_real + period_;
  event.last_duration = lastEvent_.last_duration;

  if (callback_){
    callback_(event);
  }
  double endTime = muduo::Timestamp::now().secondsSinceEpoch();
  lastEvent_.last_duration = static_cast<int32_t>((endTime-startTime)*1000);
  lastEvent_.current_real = startTime;
  lastEvent_.expected_real = event.expected_real;

  // 对于一次性定时器，执行后停止
  if(isOneShot_){
    isRunning_ = false;
  }
}

void Timer::setPeriod(double period){
  bool wasRunning  = isRunning_;
  if(wasRunning){
    stop();
  }
  period_ = period;
  if(wasRunning){
    start();
  }
}

double Timer::getPeriod() const {
   return period_;
}

}


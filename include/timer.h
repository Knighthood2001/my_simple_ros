//
// Created by wu on 25-11-18.
//

#ifndef TIMER_H
#define TIMER_H
#include <functional>
#include "muduo/net/EventLoop.h"
#include "muduo/net/TimerId.h"
#include <cmath>

namespace simple_ros {

struct TimerEvent{
  double current_real;
  double last_real;
  double expected_real;
  int32_t last_duration;
};

// 将std::function<void()>这个类型，起一个新的别名TimerCallback。
typedef std::function<void(const TimerEvent&)> TimerCallback;

class Timer {
public:
  Timer(muduo::net::EventLoop* loop, double period, const TimerCallback& callback);
  ~Timer();

  //禁止拷贝和移动
  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;
  Timer(Timer&&) = delete;
  Timer& operator=(Timer&&) = delete;

  void start(); //启动定时器
  void stop();  //停止定时器

  void setOneShot(bool oneshot); // 设置是否为单次计时器

  void pause();   // 暂停定时器
  void resume();  // 恢复定时器
  
  void setPeriod(double period); //设置定时器周期
  double getPeriod() const;      //获取定时器周期

private:
/*
具体流程:
  用户创建 Timer 时，会指定 period_（周期）和 callback_（回调函数），并绑定 loop_（事件循环）。
  调用 “启动” 方法后，Timer 会通过 loop_ 注册到事件循环，得到 timerId_（唯一标识），并把 isRunning_ 设为 true。
  loop_ 按照 period_ 周期性检查，到点后调用 callback_，执行用户任务。
  若用户调用 “停止” 方法，Timer 会通过 timerId_ 告诉 loop_ 取消当前定时器，同时把 isRunning_ 设为 false。
*/
  muduo::net::EventLoop* loop_;
  double period_;
  TimerCallback callback_;
  muduo::net::TimerId timerId_;
  bool isRunning_;
  bool isOneShot_; //提供一个标志位 isOneShot_，用于区分周期性定时器和一次性定时器。
  bool isPaused_;
  double remainingTime_;
  TimerEvent lastEvent_;           // 上一次事件信息
  
  //内部回调函数，会调用用户提供的回调函数
  void internalCallback();
};

}
#endif //TIMER_H

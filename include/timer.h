//
// Created by wu on 25-11-18.
//

#ifndef TIMER_H
#define TIMER_H
#include <functional>
#include "muduo/net/EventLoop.h"
#include "muduo/net/TimerId.h"

namespace simple_ros {
// 将std::function<void()>这个类型，起一个新的别名TimerCallback。
typedef std::function<void()> TimerCallback;

class Timer {
public:
  Timer(muduo::net::EventLoop* loop, double period, const TimerCallback& callback);
  ~Timer();

  void start();
  void stop();

  void setOneShot(bool oneshot);

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
};

}
#endif //TIMER_H

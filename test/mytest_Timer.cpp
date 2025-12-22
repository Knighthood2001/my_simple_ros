//
// Created by wu on 25-12-22.
//
#include <iostream>
#include <memory>

#include "muduo/net/EventLoop.h"
#include "timer.h"

using namespace simple_ros;

int main() {
    muduo::net::EventLoop loop;

    Timer timer(&loop, 1.0, [](const TimerEvent& e) {
        std::cout << "ROS-like timer tick" << std::endl;
    });

    timer.start();

    // 模拟 ros::shutdown()
    loop.runAfter(5.0, [&loop]() {
        std::cout << "Shutdown" << std::endl;
        loop.quit();
    });

    loop.loop();
    return 0;
}


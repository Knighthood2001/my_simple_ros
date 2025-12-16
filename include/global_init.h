//
// Created by wu on 25-12-16.
//

#ifndef GLOBAL_INIT_H
#define GLOBAL_INIT_H
#include <memory>

class SystemManager{
  public:
    static SystemManager& instance();
    SystemManager(const SystemManager&) = delete;
    SystemManager& operator=(const SystemManager&) = delete;

    void init();
    void init(int port);

  private:
    // 私有构造/析构，确保只能通过 instance() 获取实例

    SystemManager() = default;
    ~SystemManager() = default;
    
};
#endif //GLOBAL_INIT_H

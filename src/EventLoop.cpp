#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include <vector>

EventLoop::EventLoop(): ep(nullptr), quit(false)
{
    // 创建Epoll实例（事件驱动核心）
    ep = new Epoll();
}

EventLoop::~EventLoop()
{
    delete ep;
}

void EventLoop::loop()
{
    // 持续运行直到quit=true
    while (!quit)
    {
        std::vector<Channel*> chs;
        // 获取活跃事件列表（阻塞调用）
        chs = ep->poll();
        // 遍历处理所有就绪事件
        for (auto it = chs.begin(); it != chs.end(); ++it) {
            (*it)->handleEvent();
        }
        
    }
    
}

// 代理方法，将 Channel 的注册/更新操作转发给 Epoll 实例。

void EventLoop::updateChannel(Channel *ch){
    // 委托给Epoll更新Channel状态
    ep->updateChannel(ch);
}
#include "Epoll.h"
#include "util.h"
#include <unistd.h>
#include <string.h>


#define MAX_EVENTS 1024

// Epoll 构造函数
Epoll::Epoll():epfd(-1), events(nullptr)
{
    // int epfd = epoll_create(1024);  //参数表示监听事件的大小，如超过内核会自动调整，已经被舍弃，无实际意义，传入一个大于0的数即可
    epfd = epoll_create1(0); // 参数是一个flag，一般设为0，详细参考man epoll
    errif(epfd == -1, "epoll create error");

    // 描述监控事件的核心数据结构
    /*
    struct epoll_event {
        uint32_t     events;    // 监控的事件标志位（EPOLLIN等）
        epoll_data_t data;      // 用户数据（通常存储文件描述符）
    };
    */
    events = new epoll_event[MAX_EVENTS]; // 动态分配数组
    bzero(events, sizeof(*events) * MAX_EVENTS); // 正确清零
}

Epoll::~Epoll()
{
    if (epfd != -1){
        close(epfd);
        epfd = -1;
    }
    delete []events; // 数组资源释放
}

// 将文件描述符（如socket）添加到epoll的监控列表
// 并指定需要监听的事件类型（可读、可写等）
void Epoll::addFd(int fd, uint32_t op)
{
    // 1. 创建epoll事件结构体
    struct epoll_event  ev;
    // 2. 清零初始化（避免脏数据）
    bzero(&ev, sizeof(ev));
    // 3. 设置监控的文件描述符
    ev.data.fd = fd;
    // 4. 设置监听的事件类型
    ev.events = op;
    // 5. 调用epoll_ctl添加到监控列表
    errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add event error");
}

// 等待并返回epoll监控的文件描述符上的就绪事件
std::vector<epoll_event> Epoll::poll(int timeout)
{
    // 1. 准备返回的就绪事件集合
    std::vector<epoll_event> activeEvents;
    // 2. 调用epoll_wait等待事件
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
    errif(nfds == -1, "epoll wait error");
    // 3. 遍历就绪事件并存入vector
    for(int i = 0; i < nfds; ++i) {
        activeEvents.push_back(events[i]);
    }
    // 4. 返回就绪事件集合
    return activeEvents;
}


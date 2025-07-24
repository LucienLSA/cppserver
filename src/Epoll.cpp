#include "Epoll.h"
#include "util.h"
#include "Channel.h"
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
// std::vector<epoll_event> Epoll::poll(int timeout)
// {
//     // 1. 准备返回的就绪事件集合
//     std::vector<epoll_event> activeEvents;
//     // 2. 调用epoll_wait等待事件
//     int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
//     errif(nfds == -1, "epoll wait error");
//     // 3. 遍历就绪事件并存入vector
//     for(int i = 0; i < nfds; ++i) {
//         activeEvents.push_back(events[i]);
//     }
//     // 4. 返回就绪事件集合
//     return activeEvents;
// }

// epoll_wait 系统调用 获取当前就绪的I/O事件，
// 并将这些事件关联到对应的 Channel 对象
// 最终返回就绪的 Channel 指针列表
std::vector<Channel*> Epoll::poll(int timeout) {
    // 1. 准备返回的就绪Channel列表
    std::vector<Channel*> activeChannels;
    // 2. 调用epoll_wait等待事件（核心系统调用）
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, timeout);
    errif(nfds == -1, "epoll wait error");
    // 3. 遍历就绪事件并处理
    for (int i = 0; i < nfds; ++i) {
        // 3.1 从epoll_event中提取关联的Channel对象
        Channel *ch = (Channel*) events[i].data.ptr;
        // 3.2 更新Channel的触发事件状态
        ch->setRevents(events[i].events);
        // 3.3 加入就绪列表
        activeChannels.push_back(ch);
    }
    // 4. 返回就绪Channel集合
    return activeChannels;
}

// 更新Channel在epoll中的状态（添加或修改监听事件
void Epoll::updateChannel(Channel *channel)
{
    // 获取Channel对应的文件描述符
    int fd = channel->getFd();
    // 创建epoll事件结构体并初始化
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    // 设置epoll事件参数：
    ev.data.ptr = channel; // 将Channel指针保存在事件数据中（便于事件触发时获取）
    ev.events =channel->getEvents();  // 设置要监听的事件类型
    // 判断Channel是否已在epoll中注册
    if (!channel->getInEpoll()) {
        // 未注册则执行添加操作
        errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
    } else {
        // 已注册则执行修改操作
        errif(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll modify error");
    }

}

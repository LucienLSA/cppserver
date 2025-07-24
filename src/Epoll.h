#pragma once
#include <sys/epoll.h>
#include <vector>

class Channel;
class Epoll
{
private:
    int epfd; // epoll实例的文件描述符
    struct epoll_event *events; // 存储就绪事件的数组
public:
    Epoll();
    ~Epoll();
    // 添加文件描述符到epoll监控
    void addFd(int fd, uint32_t op);
    //
    void updateChannel(Channel*);

    // 事件轮询 poll() 等待并返回就绪事件
    // vector存储就绪事件
    // std::vector<epoll_event> poll(int timeout = -1);
    std::vector<Channel*> poll(int timeout = -1);
    
};
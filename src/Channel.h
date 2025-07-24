#pragma once
#include <sys/epoll.h>

/*文件描述符（如socket）与epoll事件的关联关系*/
// 管理文件描述符在epoll中的注册状态
// 记录关注的事件和实际发生的事件
// 提供事件状态查询和修改接口

class Epoll;
class Channel {
private:
    Epoll *ep; //每个文件描述符会被分发到一个Epoll类，用一个ep指针来指向
    int fd;
    uint32_t events; // 希望监听这个文件描述符的哪些事件
    uint32_t revents; // 在epoll返回该Channel时文件描述符正在发生的事件
    bool inEpoll; // inEpoll表示当前Channel是否已经在epoll红黑树中
public:
    Channel(Epoll *_ep, int _fd); // 初始化时必须关联Epoll实例和文件描述符
    ~Channel();
    
    // 如果我们希望监听该Channel上发生的读事件
    // 调用这个函数后，如Channel不在epoll红黑树中，则添加，否则直接更新Channel、打开允许读事件
    void enableReading();

    int getFd();

    uint32_t getEvents(); // 获取关注的事件
    uint32_t getRevents(); // 获取触发的事件
    bool getInEpoll(); // 检查是否在epoll中
    void setInEpoll(); // 标记注册状态 Epoll::updateChannel()在注册成功后调用

    void setRevents(uint32_t); // Epoll::poll()返回事件后调用
};
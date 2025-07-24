#include "Channel.h"
#include "EventLoop.h"

// 有参构造函数初始化
Channel::Channel(EventLoop*_loop, int _fd):loop(_loop), fd(_fd), events(0), revents(0), inEpoll(false) 
{
}

Channel::~Channel()
{
}

// 如果我们希望监听该Channel上发生的读事件
// 调用这个函数后，如Channel不在epoll红黑树中，则添加，否则直接更新Channel、打开允许读事件
void Channel::enableReading()
{
    events = EPOLLIN | EPOLLET;
    // 
    loop->updateChannel(this);
}


int Channel::getFd(){
    return fd;
}

uint32_t Channel::getEvents()
{
    return events;
}

uint32_t Channel::getRevents()
{
    return revents;
}

bool Channel::getInEpoll()
{
    return inEpoll;
}

void Channel::setInEpoll()
{
    inEpoll = true;
}

void Channel::setRevents(uint32_t _ev){
    revents = _ev;
}


// 当 epoll_wait 检测到 fd 就绪时，由 EventLoop::loop() 触发。
// 直接调用用户通过 setCallback() 注册的函数（如处理读/写事件的业务逻辑）。
void Channel::handleEvent(){
    // 执行绑定的回调函数
    callback();
}

void Channel::setCallback(std::function<void()> _cb){
    // 绑定用户自定义事件处理逻辑
    callback = _cb;
}
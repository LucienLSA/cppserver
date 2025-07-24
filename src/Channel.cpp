#include "Channel.h"
#include "Epoll.h"

// 有参构造函数初始化
Channel::Channel(Epoll *_ep, int _fd):ep(_ep), fd(_fd), events(0), revents(0), inEpoll(false) 
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
    ep->updateChannel(this);
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
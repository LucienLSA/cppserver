#pragma once

class EventLoop;
class Socket;
// 基于 Reactor 模式 的 TCP 服务器实现
class Server
{
private:
    EventLoop *loop;
public:
    Server(EventLoop*);
    ~Server();

    // 处理客户端数据
    void handleReadEvent(int);
    // 监听新连接
    void newConnection(Socket *serv_sock);
};

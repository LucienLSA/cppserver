#pragma once


class InetAddress; // 前置声明
class Socket
{
private:
    int fd;
public:
    Socket();
    Socket(int);
    ~Socket();

    void bind(InetAddress*); // socket地址与文件描述符绑定
    void listen(); // 监听socket端口
    void setnonblocking(); // 将文件描述符（如套接字）设置为非阻塞模式

    int accept(InetAddress*); // 用于与当前连接的客户端通信的套接字

    int getFd(); // 获取当前文件描述符
};
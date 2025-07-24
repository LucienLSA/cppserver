#include "Socket.h"
#include "InetAddress.h"
#include "util.h"
#include <fcntl.h>      // fcntl() 函数管理文件/套接字的属性
#include <sys/socket.h>
#include <unistd.h>     //  POSIX系统调用 提供 open()、close() 等基础IO

// 创建默认的TCP套接字
Socket::Socket():fd(-1) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    errif(fd==-1, "socket create error");
}
// 现有文件描述符初始化
Socket::Socket(int _fd):fd(_fd) {
    errif(fd==-1, "socket create error");
}

// 释放资源，清除文件描述符
Socket::~Socket() {
    if (fd != -1 ) {
        close(fd);
        fd = -1;
    }
}

// 返回clnt_sockfd 客户端套接字描述符  用于与当前连接的客户端通信的套接字
int Socket::accept(InetAddress *addr)
{
    int clnt_sockfd = ::accept(fd, (sockaddr *)&addr->addr, &addr->addr_len);
    errif(clnt_sockfd == -1, "socket accept error");
    return clnt_sockfd;
}

// // 监听这个socket端口
void Socket::listen()
{
    errif(::listen(fd, SOMAXCONN) == -1,"socket listen error" );
}

// // 将文件描述符（如套接字）设置为非阻塞模式;当I/O操作（如read/write）无法立即完成时，
// 立即返回错误（EAGAIN或EWOULDBLOCK），而不是阻塞等待。
void Socket::setnonblocking()
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int Socket::getFd()
{
    return fd;
}

// 实现文件描述符绑定地址
void Socket::bind(InetAddress *addr)
{
    errif(::bind(fd,(sockaddr*)&addr->addr, addr->addr_len)==-1, "socket bind error");
}



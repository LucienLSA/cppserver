#include <stdio.h>      // C标准IO
#include <sys/socket.h> // 核心网络操作
#include <arpa/inet.h>  // IP地址转换
#include <string.h>     // C 内存/字符串操作
#include <unistd.h>     //  POSIX系统调用 提供 open()、close() 等基础IO
#include <fcntl.h>      // fcntl() 函数管理文件/套接字的属性
#include <errno.h>      // 定义系统调用错误码，全局变量 errno 存储最近一次错误
#include <sys/epoll.h>  // epoll 事件通知机制
#include "Epoll.h"
#include "InetAddress.h"
#include "Socket.h"
#include "util.h"

#define MAX_EVENTS 1024  // 最大事件大小
#define READ_BUFFER 1024 // 读取数据的缓存大小

// 将文件描述符（如套接字）设置为非阻塞模式;当I/O操作（如read/write）无法立即完成时，
// 立即返回错误（EAGAIN或EWOULDBLOCK），而不是阻塞等待。
// void setnonblocking(int fd)
// {
//     fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
// }
void handleReadEvent(int);

int main()
{
    // 服务器初始化
    Socket *serv_sock = new Socket(); // TCP套接字操作
    InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888); // IPv4地址结构（IP+端口）
    serv_sock->bind(serv_addr); // 绑定地址
    serv_sock->listen(); // 监听

    // Epoll事件监控设置
    Epoll *ep = new Epoll(); // epoll系统调用（事件监听与分发）
    serv_sock->setnonblocking(); // 必须设为非阻塞
    ep->addFd(serv_sock->getFd(), EPOLLIN | EPOLLET); // 边缘触发模式

    // 事件循环
    while(true) {
        // 调用 ep->poll() 方法，阻塞等待直到以下情况之一发生：
        // 有文件描述符就绪（可读/可写等）
        // 达到指定的超时时间
        // 被信号中断
        // // 等待并返回epoll监控的文件描述符上的就绪事件
        std::vector<epoll_event> events = ep->poll();
        int nfds = events.size();
        for (int i = 0; i < nfds; ++i) {
            // 发生事件的fd是服务器socket fd，表示有新客户端连接
            if (events[i].data.fd == serv_sock->getFd()) {
                /*客户端信息 */
                // 接收连接时也需要保存客户端的socket地址信息
                InetAddress *clnt_addr = new InetAddress(); // 会发生内存泄露！没有delete
                // 用于与当前连接的客户端通信的套接字
                Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr)); // //会发生内存泄露！没有delete
                printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
                ep->addFd(clnt_sock->getFd(), EPOLLIN | EPOLLET) ;
                // ET需要搭配非阻塞式socket使用
                clnt_sock->setnonblocking();
            } else if (events[i].events & EPOLLIN) {// 发生事件的是客户端，并且是可读事件（EPOLLIN）
                handleReadEvent(events[i].data.fd);
            } else {
                printf("something else happend\n");
            }
        }
    }
    delete serv_sock;
    delete serv_addr;
    return 0;
}


// 非阻塞读取处理函数
void handleReadEvent(int sockfd) {
    // 定义缓冲区
    char buf[READ_BUFFER];
    /*read和write来进行网络接口的数据读写操作 */
    while(true) 
    {
        // 从客户端socket读到缓冲区，返回已读数据大小
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            printf("message from client fd %d: %s\n", sockfd, buf);
            // 将相同的数据写回到客户端
            write(sockfd, buf, sizeof(buf));
        } else if (read_bytes == 0) {
            printf("EOF, client fd %d disconnected\n", sockfd);
            close(sockfd);
            break;
        } else if (read_bytes == -1 && errno == EINTR) {//客户端正常中断、继续读取
            printf("continue reading");
            continue;
        } else if(read_bytes == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){//非阻塞IO，这个条件表示数据全部读取完毕
            printf("finish reading once, errno: %d\n", errno);
            break;
        }
}
}
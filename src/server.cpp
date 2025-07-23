#include <stdio.h>      // C标准IO
#include <sys/socket.h> // 核心网络操作
#include <arpa/inet.h>  // IP地址转换
#include <string.h>     // C 内存/字符串操作
#include <unistd.h>     //  POSIX系统调用 提供 open()、close() 等基础IO
#include <fcntl.h>      // fcntl() 函数管理文件/套接字的属性
#include <errno.h>      // 定义系统调用错误码，全局变量 errno 存储最近一次错误
#include <sys/epoll.h>  // epoll 事件通知机制

#include "util.h"

#define MAX_EVENTS 1024  // 最大事件大小
#define READ_BUFFER 1024 // 读取数据的缓存大小

// 将文件描述符（如套接字）设置为非阻塞模式;当I/O操作（如read/write）无法立即完成时，
// 立即返回错误（EAGAIN或EWOULDBLOCK），而不是阻塞等待。
void setnonblocking(int fd)
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int main()
{
    /*服务端初始化与监听*/
    // 建立一个socket套接字，对外提供一个网络通信接口, 文件描述符
    // 第一个参数: AF_INET表示使用IPv4 第二个参数：数据传输方式
    //  第三个参数：协议，0表示根据前面的两个参数自动推导协议类型
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    // 初始化sockaddr_in结构体
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));

    // 套接字绑定到一个IP地址和端口 设置地址族、IP地址和端口
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8888);

    // socket地址与文件描述符绑定
    // 使用专用socket地址（sockaddr_in）而绑定的时候要转化为通用socket地址（sockaddr）
    // 第二个参数传入serv_addr的大小
    errif(bind(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) == -1, "socket bind error");

    // 监听这个socket端口
    errif(listen(sockfd, SOMAXCONN) == -1, "scoket listen error");

    /*epoll*/
    // int epfd = epoll_create(1024);  //参数表示监听事件的大小，如超过内核会自动调整，已经被舍弃，无实际意义，传入一个大于0的数即可
    int epfd = epoll_create1(0); // 参数是一个flag，一般设为0，详细参考man epoll
    errif(epfd == -1, "epoll create error");

    // 描述监控事件的核心数据结构
    /*
    struct epoll_event {
        uint32_t     events;    // 监控的事件标志位（EPOLLIN等）
        epoll_data_t data;      // 用户数据（通常存储文件描述符）
    };
    */
    struct epoll_event events[MAX_EVENTS], ev;
    bzero(&events, sizeof(events));
    bzero(&ev, sizeof(ev));
    // 该IO口为服务器socket fd
    ev.data.fd = sockfd;
    // 服务端如果使用了ET模式，且未处理错误，后续进行修复，实际上服务端接收连接最好不要用ET模式
    ev.events = EPOLLIN | EPOLLET;
    // 非阻塞模式
    setnonblocking(sockfd);
    // 将服务器socket fd添加到epoll
    errif(epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev) == -1, "server socket fd add epoll error");

    // 不断监听epoll上的事件并处理
    while (true)
    {
        // epoll_wait获取有事件发生的fd
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        errif(nfds == -1, "epoll wait error");
        // 处理这nfds个事件
        for (int i = 0; i < nfds; i++)
        {
            // 发生事件的fd是服务器socket fd，表示有新客户端连接
            if (events[i].data.fd == sockfd)
            {
                /*客户端信息 */
                // 接收连接时也需要保存客户端的socket地址信息
                struct sockaddr_in clnt_addr;

                // 第二个参数写入客户端socket长度
                socklen_t clnt_addr_len = sizeof(clnt_addr);
                // 初始化
                bzero(&clnt_addr, sizeof(clnt_addr));
                // 返回clnt_sockfd 客户端套接字描述符  用于与当前连接的客户端通信的套接字
                int clnt_sockfd = accept(sockfd, (sockaddr *)&clnt_addr, &clnt_addr_len);
                errif(clnt_sockfd == -1, "socket accept error");

                // 打印连接的客户端信息
                printf("new client fd %d! ip:%s port:%d\n", clnt_sockfd, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

                // 客户端事件
                bzero(&ev, sizeof(ev));
                ev.data.fd = clnt_sockfd;
                // 对于客户端连接，使用ET模式，可以让epoll更加高效，支持更多并发
                ev.events = EPOLLIN | EPOLLET;
                // ET需要搭配非阻塞式socket使用
                setnonblocking(clnt_sockfd);
                errif(epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sockfd, &ev) == -1, "client socket fd add epoll error"); // 将该客户端的socket fd添加到epol
            }
            else if (events[i].events & EPOLLIN)
            { // 发生事件的是客户端，并且是可读事件（EPOLLIN）
                // 定义缓冲区
                char buf[READ_BUFFER];
                /*read和write来进行网络接口的数据读写操作 */
                while (true)
                {
                    //由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
                    // 初始化缓冲区
                    bzero(&buf, sizeof(buf));
                    // 从客户端socket读到缓冲区，返回已读数据大小
                    ssize_t read_bytes = read(events[i].data.fd, buf, sizeof(buf));
                    // 读到数据
                    // 文件描述符理论上是有限的，在使用完一个fd之后
                    if (read_bytes > 0)
                    {
                        // 服务端打印输出客户端信息
                        printf("message from client fd %d:%s\n", events[i].data.fd, buf);
                        // 将相同的数据写回到客户端
                        write(events[i].data.fd, buf, sizeof(buf));
                    }
                    else if (read_bytes == 0)
                    {
                        printf("client fd %d disconnected\n", events[i].data.fd);
                        // 关闭客户端文件描述符
                        close(events[i].data.fd);
                        break;
                    }
                    else if (read_bytes == -1 && errno == EINTR) //客户端正常中断、继续读取
                    {
                        printf("continue reading");
                        continue;
                    } 
                    else if (read_bytes == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
                    { //非阻塞IO，这个条件表示数据全部读取完毕
                        printf("finish reading once, errno: %d\n", errno);
                        break;
                    }
                }
            } else {
                printf("something else happened\n");
            }
        }
    }
    close(sockfd);
    return 0;
}

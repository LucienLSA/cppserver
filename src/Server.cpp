#include "Server.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include <functional>
#include <ostream>
#include <string.h>
#include <unistd.h>

#define READ_BUFFER 1024 // 读取数据的缓存大小

Server::Server(EventLoop *_loop):loop(_loop)
{
    // 1. 创建服务器 socket 并绑定地址
    // 服务器初始化
    Socket *serv_sock = new Socket(); // TCP套接字操作
    InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888); // IPv4地址结构（IP+端口）
    serv_sock->bind(serv_addr); // 绑定地址
    serv_sock->listen(); // 监听
    serv_sock->setnonblocking(); // 必须设为非阻塞

    // 2. 创建 Channel 并注册到 EventLoop
    Channel *servChannel = new Channel(loop, serv_sock->getFd());
    // 绑定 newConnection 回调，用于处理新连接。
    std::function<void()> cb = std::bind(&Server::newConnection, this, serv_sock);
    servChannel->setCallback(cb);
    // enableReading() 注册到 epoll，监听 EPOLLIN 事件
    servChannel->enableReading();  
}

Server::~Server()
{
}

// 处理客户端数据
void Server::handleReadEvent(int sockfd)
{
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
    // 边缘触发需循环读取，直到无数据
}

// 处理新连接 
void Server::newConnection(Socket *serv_sock)
{
    /*客户端信息 */
    // 1. 接受客户端连接
    // 接收连接时也需要保存客户端的socket地址信息
    InetAddress *clnt_addr = new InetAddress(); // 会发生内存泄露！没有delete
    // 用于与当前连接的客户端通信的套接字
    Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr)); // //会发生内存泄露！没有delete
    // 2. 打印客户端信息
    printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
    // 3. 设置非阻塞并注册到 EventLoop
    // ET需要搭配非阻塞式socket使用
    clnt_sock->setnonblocking();
    Channel *clntChannel = new Channel(loop, clnt_sock->getFd());
    // 绑定 handleReadEvent 回调
    std::function<void()> cb = std::bind(&Server::handleReadEvent, this, clnt_sock->getFd());
    clntChannel->setCallback(cb);
    clntChannel->enableReading();
}

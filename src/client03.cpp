#include <sys/socket.h>
#include <iostream>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

int main() {
    /*客户端连接*/
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8888);

    // 客户端不进行bind操作
    errif(connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr))==-1, "socket connect error");
    
    while (true)
    {
        //定义缓冲区
        char buf[1024];
        //初始化缓冲区
        bzero(&buf,sizeof(buf));
        // 从键盘输入要传到服务器的数据
        scanf("%s", buf);
        //发送缓冲区中的数据到服务器socket，返回已发送数据大小
        ssize_t write_bytes = write(sockfd, buf, sizeof(buf));
        if (write_bytes == -1) {
            printf("socket already disconnected, can't write any more!\n");
            break;
        }
        bzero(&buf, sizeof(buf));       //清空缓冲区
        //从服务器socket读到缓冲区，返回已读数据大小
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        // 文件描述符理论上是有限的，在使用完一个fd之后
        if (read_bytes > 0)  {
            // 客户端打印输出信息
            printf("message from server fd %s\n", buf);
        } else if (read_bytes == 0) {
            // //read返回0，表示EOF，通常是服务器断开链接，等会儿进行测试
            printf("client fd %d disconnected\n", sockfd);
            break;
        } else if (read_bytes == -1){
            // read返回-1，表示发生错误
            close(sockfd);
            errif(true, "socket read error");
        }
    }

    return 0;
}
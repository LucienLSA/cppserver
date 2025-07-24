#pragma once
#include <arpa/inet.h>


// 网络地址和端口类
class InetAddress
{
public:
    struct sockaddr_in addr;// 存储IP地址和端口（二进制格式
    socklen_t addr_len; // 地址结构体的长度
    InetAddress(); // 初始化空地址
    InetAddress(const char* ip, uint16_t port); // 通过字符串IP和端口号初始化地址
    ~InetAddress();
};
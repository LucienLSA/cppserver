#include "InetAddress.h"
#include <string.h>

// 初始化列表设置 addr_len
InetAddress::InetAddress():addr_len(sizeof(addr)) {
    // 使用 bzero 清零地址结构
    bzero(&addr, sizeof(addr));
}

InetAddress::InetAddress(const char* ip, uint16_t port):addr_len(sizeof(addr)) {
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port=htons(port);
    addr_len = sizeof(addr);
}

InetAddress::~InetAddress(){
    
}
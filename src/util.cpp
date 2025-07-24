#include "util.h"
#include <stdio.h>
#include <stdlib.h>

// 实现 封装一个错误处理函数
// 打印出errno的实际意义
// 第二个参数 打印出我们传入的字符串
// 当一个系统调用返回-1，说明有error发生
void errif(bool condition, const char *errmsg) {
    if (condition) {
        perror(errmsg);
        // exit函数让程序退出并返回一个预定义常量EXIT_FAILURE
        exit(EXIT_FAILURE);
    }
}

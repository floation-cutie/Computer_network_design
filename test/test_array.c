#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 REMIND : 当使用 malloc 动态分配内存时，标准库并不提供直接的方法来获取已分配内存的大小。
 */

int main(int argc, char const *argv[]) {
    unsigned char *buffer = (unsigned char *)malloc(sizeof(unsigned char) * 4);
    buffer[0] = 0x00;
    buffer[1] = 0x01;
    buffer[2] = 0x02;
    buffer[3] = 0x03;
    buffer[3] = '\0';
    printf("buffer: %d %d %d %d\n", buffer[0], buffer[1], buffer[2], buffer[3]);
    printf("sizeof buffer: %d\n", sizeof(*buffer) / sizeof(unsigned char));
    return 0;
}

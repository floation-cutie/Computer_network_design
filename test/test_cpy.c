#include <stdio.h>
#include <string.h>

int main() {
    // 使用memcpy复制内存块
    char src[] = "Hello, world!";
    char dest[20];
    memcpy(dest, src, strlen(src) + 1); // 复制包括字符串终止符'\0'在内的所有字符
    printf("memcpy result: %s\n", dest);

    // 使用strncpy复制字符串
    char src2[] = "Hello, world!";
    char dest2[20];
    strncpy(dest2, src2, 5); // 复制前5个字符
    dest2[5] = '\0';         // 手动添加字符串终止符
    printf("strncpy result: %s\n", dest2);

    return 0;
}

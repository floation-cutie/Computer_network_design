#include <stdio.h>
#include <stdlib.h>

void getName(unsigned char *qname, const unsigned char *bytestream,
             unsigned short *offset) {
    unsigned short initial_offset = *offset; // 保存初始偏移量
    int count = 0;
    while (*(bytestream + *offset) != 0) {
        if (((*(bytestream + *offset) >> 6) & 3) == 3) { // 检测压缩标签（前两位为11）
            unsigned short new_offset =
                ntohs(*(unsigned short *)(bytestream + *offset)) & 0x3fff; // 获取新的偏移量
            printf("Compressed label detected. Jumping to offset: %u\n", new_offset);
            getName(qname, bytestream, &new_offset); // 递归解析压缩标签指向的部分
            (*offset) += 2;                          // 跳过压缩标签的两个字节
            return;
        }
        if (*(bytestream + *offset) <= 0x40) {
            count = *(bytestream + *offset); // 获取下一个标签的长度
            printf("Label length: %u\n", count);
            (*offset)++;
        }
        for (int i = 0; i < count; i++) {
            *qname = *(bytestream + *offset); // 复制标签到qname
            printf("Copying byte: %c\n", *qname);
            qname++;
            (*offset)++;
        }
        if (*(bytestream + *offset) != 0) {
            *qname = '.'; // 添加标签分隔符
            printf("Adding label separator\n");
            qname++;
        }
    }
    (*offset)++;
    *qname = '\0'; // 终止字符串
    printf("Finished parsing name. Final offset: %u, Initial offset: %u\n",
           *offset, initial_offset);
}
// doh服务
int main() {
    unsigned char bytestream[] = {
        // 示例 DNS 报文数据
        0x03, 'w', 'w', 'w', 0x07, 'e', 'x', 'a', 'm', 'p', 'l', 'e', 0x03, 'c', 'o', 'm', 0x00, 0x04,
        'h', 't', 't', 'p', 0xc0, 0x00 // 压缩标签，指向偏移量0x00处的"www.example.com"
    };
    unsigned char qname[256];
    unsigned short offset = 0x10; // 这里假设偏移量0x10是压缩标签的开始位置

    getName(qname, bytestream, &offset);
    printf("Parsed name: %s\n", qname); // 输出"www.example.com"

    return 0;
}

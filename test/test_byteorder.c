#include <arpa/inet.h>
#include <stdio.h>

void print_bytes(unsigned char *bytes, int length) {
    for (int i = 0; i < length; i++) {
        printf("%02X ", bytes[i]);
    }
    printf("\n");
}

int main() {
    unsigned short host_port = 0x1234;
    unsigned long host_addr = 0x12345678;

    unsigned short net_port = htons(host_port);
    unsigned long net_addr = htonl(host_addr);

    printf("Host byte order port: 0x%04X\n", host_port);
    printf("Network byte order port: 0x%04X\n", net_port);
    printf("Host byte order address: 0x%08lX\n", host_addr);
    printf("Network byte order address: 0x%08lX\n", net_addr);

    // 打印字节序列
    printf("Host byte order port bytes: ");
    print_bytes((unsigned char *)&host_port, sizeof(host_port));

    printf("Network byte order port bytes: ");
    print_bytes((unsigned char *)&net_port, sizeof(net_port));

    printf("Host byte order address bytes: ");
    print_bytes((unsigned char *)&host_addr, sizeof(host_addr));

    printf("Network byte order address bytes: ");
    print_bytes((unsigned char *)&net_addr, sizeof(net_addr));

    // 转回主机字节序以验证
    unsigned short host_port_converted = ntohs(net_port);
    unsigned long host_addr_converted = ntohl(net_addr);

    printf("Converted back to host byte order port: 0x%04X\n", host_port_converted);
    printf("Converted back to host byte order address: 0x%08lX\n", host_addr_converted);

    return 0;
}

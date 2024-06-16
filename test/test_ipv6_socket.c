#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define BUFFER_SIZE 512

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    WSADATA wsaData;
    int sockfd;
    struct sockaddr_in6 serverAddr, clientAddr;
    int addr_len = sizeof(struct sockaddr_in6);
    unsigned char buffer[BUFFER_SIZE];
    int recv_len;

    // 初始化 Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        exit(1);
    }

    // 配置监听的端口
    const int port = 5353;

    // 创建 IPv6 socket
    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create socket. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        exit(1);
    }

    // 允许 IPv4 和 IPv6 兼容
    int no = 0;
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&no, sizeof(no)) == SOCKET_ERROR) {
        fprintf(stderr, "Failed to set socket options. Error Code: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        exit(1);
    }

    // 配置 serverAddr 结构体
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin6_family = AF_INET6;
    serverAddr.sin6_port = htons(port);
    serverAddr.sin6_addr = in6addr_any; // 绑定到所有可用的网络接口

    // 绑定 socket 到指定的 IP 和端口
    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "Failed to bind socket. Error Code: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        exit(1);
    }

    printf("Server listening on port %d\n", port);

    while (1) {
        // 接收来自客户端的 UDP 数据
        recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &addr_len);
        if (recv_len == SOCKET_ERROR) {
            fprintf(stderr, "recvfrom() failed. Error Code: %d\n", WSAGetLastError());
            closesocket(sockfd);
            WSACleanup();
            exit(1);
        }

        // 判断是 IPv4 还是 IPv6 客户端
        if (clientAddr.sin6_family == AF_INET6) {
            char addr_str[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &clientAddr.sin6_addr, addr_str, sizeof(addr_str));
            printf("Received packet from IPv6 address %s:%d\n", addr_str, ntohs(clientAddr.sin6_port));
        } else {
            struct sockaddr_in *clientAddr4 = (struct sockaddr_in *)&clientAddr;
            char addr_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr4->sin_addr, addr_str, sizeof(addr_str));
            printf("Received packet from IPv4 address %s:%d\n", addr_str, ntohs(clientAddr4->sin_port));
        }

        // 这里可以添加处理和响应 UDP 数据的代码
    }

    // 关闭 socket
    closesocket(sockfd);

    // 清理 Winsock
    WSACleanup();

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define DNS_PORT 53

typedef struct {
    SOCKET sockfd;
    struct sockaddr_in addr;
} DNS_Server;

void cleanup_socket(DNS_Server *server) {
    closesocket(server->sockfd);
    WSACleanup();
}

int main() {
    WSADATA wsa;
    DNS_Server rs;
    int recv_len;
    char buf[512];
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    // 初始化 Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }

    // 创建套接字
    if ((rs.sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return EXIT_FAILURE;
    }

    // 配置地址结构
    rs.addr.sin_family = AF_INET;
    rs.addr.sin_addr.s_addr = INADDR_ANY;
    rs.addr.sin_port = htons(DNS_PORT);

    // 绑定套接字
    if (bind(rs.sockfd, (struct sockaddr *)&rs.addr, sizeof(rs.addr)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        cleanup_socket(&rs);
        return EXIT_FAILURE;
    }

    printf("DNS Server is listening on port %d...\n", DNS_PORT);

    // 循环接收数据
    while (1) {
        memset(buf, 0, sizeof(buf));
        recv_len = recvfrom(rs.sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (recv_len == SOCKET_ERROR) {
            printf("recvfrom() failed. Error Code: %d\n", WSAGetLastError());
            continue;
        }

        printf("Received DNS request from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // 这里可以处理接收到的 DNS 请求并发送响应
        // 示例：发送一个简单的响应（假设响应长度为 recv_len）
        if (sendto(rs.sockfd, buf, recv_len, 0, (struct sockaddr *)&client_addr, client_addr_len) == SOCKET_ERROR) {
            printf("sendto() failed. Error Code: %d\n", WSAGetLastError());
        }
    }

    cleanup_socket(&rs);
    return EXIT_SUCCESS;
}

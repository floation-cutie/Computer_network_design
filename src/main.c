#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define DNS_PORT 53
// 没有EDNS0：
// UDP上最大DNS消息大小：512字节。
// 如果DNS更大时,要求必须通过TCP连接
// 有EDNS0：
// 客户端可以请求更大的缓冲区大小，通常最大可达4096字节
// 但这取决于客户端和服务器的配置以及网络状况。
#define BUFFER_SIZE 512

typedef struct {
    SOCKET sockfd;
    // 表示IPv4地址
    struct sockaddr_in addr;
} RAII_Socket;

// 初始化Winsock
void initialize_winsock() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
}

// 清理Winsock
void cleanup_winsock() {
    WSACleanup();
}

// 初始化RAII_Socket
RAII_Socket create_socket() {
    RAII_Socket rs;
    rs.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (rs.sockfd == INVALID_SOCKET) {
        printf("Could not create socket. Error Code: %d\n", WSAGetLastError());
        cleanup_winsock();
        exit(EXIT_FAILURE);
    }

    rs.addr.sin_family = AF_INET;
    rs.addr.sin_addr.s_addr = INADDR_ANY;
    rs.addr.sin_port = htons(DNS_PORT);

    if (bind(rs.sockfd, (struct sockaddr *)&rs.addr, sizeof(rs.addr)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(rs.sockfd);
        cleanup_winsock();
        exit(EXIT_FAILURE);
    }

    return rs;
}

// 清理RAII_Socket
void cleanup_socket(RAII_Socket *rs) {
    closesocket(rs->sockfd);
}

int main() {
    // 初始化Winsock
    initialize_winsock();

    // 创建RAII_Socket
    RAII_Socket rs = create_socket();

    printf("Socket created and bound to port %d.\n", DNS_PORT);

    // 模拟接收DNS报文
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client;
    int client_len = sizeof(client);

    printf("Waiting for DNS queries...\n");
    while (1) {
        int recv_len = recvfrom(rs.sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client, &client_len);
        if (recv_len == SOCKET_ERROR) {
            printf("recvfrom() failed. Error Code: %d\n", WSAGetLastError());
            break;
        }

        printf("Received DNS query from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        printf("Query length: %d bytes\n", recv_len);

        // 打印接收到的DNS查询报文
        for (int i = 0; i < recv_len; i++) {
            // 格式控制: 以十六进制输出,2为指定的输出字段的宽度.如果位数小于2,则左端补0
            printf("%02x ", (unsigned char)buffer[i]);
            if ((i + 1) % 16 == 0) {
                printf("\n");
            }
        }
        printf("\n");
    }

    // 清理RAII_Socket
    cleanup_socket(&rs);

    // 清理Winsock
    cleanup_winsock();

    return 0;
}

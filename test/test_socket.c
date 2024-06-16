#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 512

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr, forward_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    unsigned char buffer[BUFFER_SIZE];
    ssize_t recv_len, sent_len;

    // 配置监听的 IP 和端口
    const char *LISTEN_IP = "0.0.0.0";
    const int LISTEN_PORT = 5353;

    // 配置转发的 DNS 服务器 IP 和端口
    const char *FORWARD_IP = "8.8.8.8";
    const int FORWARD_PORT = 53;

    // 创建 socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        error("Failed to create socket");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(LISTEN_IP);
    server_addr.sin_port = htons(LISTEN_PORT);

    // 绑定 socket 到指定的 IP 和端口
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error("Failed to bind socket");
    }

    printf("DNS server listening on %s:%d\n", LISTEN_IP, LISTEN_PORT);

    while (1) {
        // 接收来自客户端的 DNS 查询
        recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (recv_len < 0) {
            error("Failed to receive data");
        }

        printf("Received DNS query from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // 配置转发地址
        memset(&forward_addr, 0, sizeof(forward_addr));
        forward_addr.sin_family = AF_INET;
        forward_addr.sin_addr.s_addr = inet_addr(FORWARD_IP);
        forward_addr.sin_port = htons(FORWARD_PORT);

        // 转发 DNS 查询到实际的 DNS 服务器
        sent_len = sendto(sockfd, buffer, recv_len, 0, (struct sockaddr *)&forward_addr, sizeof(forward_addr));
        if (sent_len < 0) {
            error("Failed to forward data");
        }

        // 接收来自 DNS 服务器的响应
        recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (recv_len < 0) {
            error("Failed to receive response");
        }

        // 将响应发送回原始客户端
        sent_len = sendto(sockfd, buffer, recv_len, 0, (struct sockaddr *)&client_addr, addr_len);
        if (sent_len < 0) {
            error("Failed to send response");
        }

        printf("Sent DNS response to %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }

    close(sockfd);
    return 0;
}

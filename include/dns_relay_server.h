#pragma once

#include "debug_info.h"
#include "dns_msg.h"
#include "socket.h"

#define THREAD_POOL_SIZE 10
int block_mode;          // 阻塞/非阻塞模式
RAII_Socket server_sock; // 客户端socket

address_t client_addr;
socklen_t addr_len;
uint8_t buffer[BUFFER_SIZE]; // 用于DNS消息的缓冲区
char *remote_dns;            // 远程主机（BUPT的DNS服务器）

// typedef struct {
//     int sock;
//     struct sockaddr_in client_addr;
//     socklen_t addr_len;
//     char buffer[BUF_SIZE];
//     unsigned short original_id;
// } client_request_t;

// void *handle_request(void *arg);
// void process_dns_request(client_request_t *req);
// void *handle_request(void *arg) {
//     int sockfd = *(int *)arg;
//     struct sockaddr_in client_addr;
//     socklen_t addr_len = sizeof(client_addr);
//     char buffer[BUF_SIZE];

//     while (1) {
//         int len = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
//         if (len < 0) {
//             perror("recvfrom failed");
//             continue;
//         }

//         client_request_t *req = (client_request_t *)malloc(sizeof(client_request_t));
//         req->sock = sockfd;
//         req->client_addr = client_addr;
//         req->addr_len = addr_len;
//         memcpy(req->buffer, buffer, len);
//         req->original_id = ntohs(*(unsigned short *)buffer); // 保存原始ID

//         process_dns_request(req);
//         free(req);
//     }
//     return NULL;
// }

// void process_dns_request(client_request_t *req) {
//     // 生成唯一的中继ID
//     unsigned short relay_id = generate_unique_id();
//     store_id_mapping(req->original_id, relay_id, req->client_addr, req->addr_len);

//     // 替换请求中的ID为中继ID
//     *(unsigned short *)req->buffer = htons(relay_id);

//     // 解析请求，并向上游DNS服务器查询（简化）
//     struct sockaddr_in dns_server_addr;
//     char dns_server_ip[] = "8.8.8.8"; // 使用Google DNS服务器
//     int dns_sock;
//     char response[BUF_SIZE];

//     if ((dns_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
//         perror("socket failed");
//         return;
//     }

//     memset(&dns_server_addr, 0, sizeof(dns_server_addr));
//     dns_server_addr.sin_family = AF_INET;
//     dns_server_addr.sin_addr.s_addr = inet_addr(dns_server_ip);
//     dns_server_addr.sin_port = htons(PORT);

//     if (sendto(dns_sock, req->buffer, BUF_SIZE, 0, (struct sockaddr *)&dns_server_addr, sizeof(dns_server_addr)) < 0) {
//         perror("sendto failed");
//         close(dns_sock);
//         return;
//     }

//     int len = recvfrom(dns_sock, response, BUF_SIZE, 0, NULL, NULL);
//     if (len < 0) {
//         perror("recvfrom failed");
//         close(dns_sock);
//         return;
//     }

//     // 将响应中的ID替换回原始ID
//     struct sockaddr_in client_addr;
//     socklen_t addr_len;
//     unsigned short original_id = get_original_id(ntohs(*(unsigned short *)response), &client_addr, &addr_len);
//     *(unsigned short *)response = htons(original_id);

//     // 将响应发送回客户端
//     if (sendto(req->sock, response, len, 0, (struct sockaddr *)&client_addr, addr_len) < 0) {
//         perror("sendto failed");
//     }

//     close(dns_sock);
// }

void receive_from_client();
void receive_from_server();
void send_dns_response(RAII_Socket sock, DNS_MSG *msg, address_t clientAddr);
void forward_dns_request(RAII_Socket sock, unsigned char *buf, int len);
void forward_dns_response(RAII_Socket sock, unsigned char *buf, int len, address_t clientAddr);
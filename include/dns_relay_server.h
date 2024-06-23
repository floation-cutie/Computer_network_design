#pragma once

#include "cache.h"
#include "debug_info.h"
#include "dns_msg.h"
#include "socket.h"
#include "trie.h"
#define THREAD_POOL_SIZE 10
int block_mode;          // 阻塞/非阻塞模式
RAII_Socket server_sock; // 客户端socket

address_t client_addr;
socklen_t addr_len;
uint8_t buffer[BUFFER_SIZE]; // 用于DNS消息的缓冲区

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

void receive_from_client();
void receive_from_server();
void send_dns_response(RAII_Socket sock, DNS_MSG *msg, address_t clientAddr);
unsigned char *find_ip_in_cache(struct Trie *trie, struct Cache *cache, const unsigned char *domain);
void forward_dns_request(RAII_Socket sock, unsigned char *buf, int len);
void forward_dns_response(RAII_Socket sock, unsigned char *buf, int len, address_t clientAddr);
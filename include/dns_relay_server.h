#pragma once

#include "debug_info.h"
#include "dns_msg.h"
#include "socket.h"
int block_mode;          // 阻塞/非阻塞模式
RAII_Socket server_sock; // 客户端socket

address_t client_addr;
socklen_t addr_len;
uint8_t buffer[BUFFER_SIZE]; // 用于DNS消息的缓冲区
char *remote_dns;            // 远程主机（BUPT的DNS服务器）

void receive_from_client();
void receive_from_server();
void send_dns_response(RAII_Socket sock, DNS_MSG *msg, address_t clientAddr);
void forward_dns_request(RAII_Socket sock, unsigned char *buf, int len);
void forward_dns_response(RAII_Socket sock, unsigned char *buf, int len, address_t clientAddr);
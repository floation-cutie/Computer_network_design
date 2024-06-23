#pragma once

#include "cache.h"
#include "debug_info.h"
#include "dns_msg.h"
#include "socket.h"
#include "trie.h"
#define THREAD_POOL_SIZE 10
#define CACHE_TTL 600    // 对于字典树交给一级缓存的记录，缓存时间为10分钟
int block_mode;          // 阻塞/非阻塞模式
RAII_Socket server_sock; // 客户端socket
struct Trie *trie;
struct Cache *cache;
address_t client_addr;
socklen_t addr_len;
uint8_t buffer[BUFFER_SIZE]; // 用于DNS消息的缓冲区

void handle_client_request(RAII_Socket sock, address_t clientAddr, DNS_MSG *msg, int len);
void handle_server_response(RAII_Socket sock, address_t clientAddr, DNS_MSG *msg, int len);
void send_dns_response(RAII_Socket sock, DNS_MSG *msg, address_t clientAddr);
unsigned char *find_ip_in_cache(const unsigned char *domain, int *IP_type);
void forward_dns_request(RAII_Socket sock, unsigned char *buf, int len);
void forward_dns_response(RAII_Socket sock, unsigned char *buf, int len, address_t clientAddr);
#ifndef DNS_RELAY_SERVER_H
#define DNS_RELAY_SERVER_H

#include "cache.h" // 缓存表
#include "debug_info.h"
#include "dns_function.h"
#include "dns_msg.h" // DNS报文
#include "id_converter.h"
#include "msg_convert.h" // 报文转换
#include "trie.h"        // 本地字典树

#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#define CACHE_TTL 600         // 缓存超时时间
#define MAX_DOMAIN_LENGTH 512 // 域名最大长度

// 处理DNS请求的函数,增加了与用户端通信的socket参数
void handle_dns_request(struct Trie *trie, struct Cache *cache, SOCKET sock, struct sockaddr_in clientAddr);

// 查找域名对应的IP地址的函数,增加了与用户端通信的socket参数
unsigned char *findIpAddress(struct Trie *trie, struct Cache *cache, unsigned char domain[MAX_DOMAIN_LENGTH], int *ip_type);

// 发送DNS响应报文的函数
void send_dns_response(int sock, Dns_Msg *msg, unsigned char *buf, struct sockaddr_in clientAddr);

// 转发DNS请求报文的函数
void forward_dns_request(int sock, unsigned char *buf, int len);

// 转发DNS响应报文的函数
void forward_dns_response(int sock, unsigned char *buf, int len, struct sockaddr_in clientAddr, unsigned short id);

#endif
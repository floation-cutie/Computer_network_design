#pragma once
#include <stdint.h>
#include <stdio.h>
// 统一socket的接口,后续考虑跨平台统一

#include <winsock2.h>
#include <ws2tcpip.h>

typedef struct sockaddr_in address_t;

#define DNS_PORT 53 // DNS服务器默认端口号
/*
 没有EDNS0：
 UDP上最大DNS消息大小：512字节。
 如果DNS更大时,要求必须通过TCP连接
 有EDNS0：
 客户端可以请求更大的缓冲区大小，通常最大可达4096字节
 但这取决于客户端和服务器的配置以及网络状况。
*/
#define BUFFER_SIZE 512

typedef struct {
    SOCKET sockfd;
    address_t addr; // 用于绑定服务器的地址
} RAII_Socket;

void initialize_winsock();

RAII_Socket create_socket();

void cleanup_socket(RAII_Socket *rs);

int socket_recv(RAII_Socket s, address_t *from, void *buffer, int maxlength, int *len);

int socket_send(RAII_Socket s, const address_t *to, const void *buffer, int len);

// // IPv4-mapped IPv6
// int resolve_address(address_t *addr, const char *host);

// // 自动判断并返回ipv4/v6地址的字符串形式
// // 考虑多线程下能否使用静态缓冲区
// char *address_host(address_t addr);
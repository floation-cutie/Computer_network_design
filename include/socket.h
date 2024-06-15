#pragma once
#include <stdint.h>

// 统一socket的接口

#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;

typedef struct sockaddr_in6 address_t;

int socket_init(socket_t *s, const char *host, uint16_t port);

void socket_close(socket_t s);

int socket_recv(socket_t s, address_t *from, void *buffer, int maxlength, int *len);

int socket_send(socket_t s, const address_t *to, const void *buffer, int len);

int resolve_address(address_t *addr, const char *host, uint16_t port);

// 自动判断并返回ipv4/v6地址的字符串形式
// 注意该函数的输出为一个固定缓冲区，后续调用会覆盖前一次的结果
char *address_host(address_t addr);
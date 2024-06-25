#include "config.h"
#include "dns_relay_server.h"
#include "log.h"
#include "thread_pool.h"
#include <process.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define DEFAULT_PORT 53          // DNS服务器默认端口号
#define MAX_DNS_PACKET_SIZE 1024 // DNS请求报文最大长度
CRITICAL_SECTION threadPoolCS;   // 线程池临界区
HANDLE semaphore;                // 信号量，用于线程池和等待队列之间的同步

int main(int argc, char *argv[]) {
    // 默认监听端口号
    int port = DEFAULT_PORT;
    host_path = "../src/dnsrelay.txt"; // 静态缓存文件
    log_path = "../bin/logfile.txt";
    remote_dns = "144.144.144.144";
    debug_mode = 0;
    log_mode = 1; // 启用日志模式

    // 创建字典树和缓存表
    struct Trie *trie = (struct Trie *)malloc(sizeof(struct Trie));
    struct Cache *cache = (struct Cache *)malloc(sizeof(struct Cache));
    init(argc, argv, cache, trie);
    // 初始化线程池和等待队列
    struct ThreadPool threadPool;
    init_thread_pool(&threadPool);

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        log_message("Init Windows Socket Failed");
        return 1;
    }

    // 创建socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        log_message("Create socket failed");
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        log_message("Bind socket %d failed", port);
        closesocket(sock);
        return 1;
    }

    log_message("The DNS relay server is running on port %d...", port);

    while (true) {
        // 接收DNS请求
        char buf[MAX_DNS_PACKET_SIZE];
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        int recvLen = recvfrom(sock, buf, MAX_DNS_PACKET_SIZE, 0, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (recvLen == SOCKET_ERROR) {
            log_message("Receive DNS request failed");
            continue;
        }

        log_message("Receving message from %s:%d, the length is %d bytes.", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), recvLen);

        // 从线程池中取出一个线程，如果线程池已满，则将请求放入等待队列中
        WaitForSingleObject(semaphore, INFINITE);
        EnterCriticalSection(&threadPoolCS);
        struct ThreadParam *param = NULL;
        if (threadPool.count > 0) {
            param = threadPool.params[--threadPool.count];
        }
        LeaveCriticalSection(&threadPoolCS);

        if (param->trie == NULL) {
            // 如果线程池中有空闲线程，则将请求分配给该线程处理
            param->trie = trie;
            param->cache = cache;
            param->sock = sock;
            param->clientAddr = clientAddr;
            param->clientAddrLen = clientAddrLen;
            _beginthreadex(NULL, 0, threadProc, param, 0, NULL);
        } else {
            // 如果线程池已满，则将请求放入等待队列中
            param = (struct ThreadParam *)malloc(sizeof(struct ThreadParam));
            param->trie = trie;
            param->cache = cache;
            param->sock = sock;
            param->clientAddr = clientAddr;
            param->clientAddrLen = clientAddrLen;

            EnterCriticalSection(&threadPoolCS);
            add_to_pool(&threadPool, param);
            LeaveCriticalSection(&threadPoolCS);
        }
    }

    // 关闭socket和清理资源
    closesocket(sock);
    free(trie);
    clearCache(cache);
    close_log();
    free(cache);
    // 销毁线程池和等待队列
    destroy_thread_pool(&threadPool);
    WSACleanup();

    return 0;
}
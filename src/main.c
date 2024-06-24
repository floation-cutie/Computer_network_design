#include "config.h"
#include "dns_relay_server.h"
// #include "thread_pool.h"
// CRITICAL_SECTION threadPoolCS; // 线程池临界区
// HANDLE semaphore;              // 信号量，用于线程池和等待队列之间的同步

int main(int argc, char *argv[]) {
    // 创建字典树和缓存表
    trie = (struct Trie *)malloc(sizeof(struct Trie));
    cache = (struct Cache *)malloc(sizeof(struct Cache));
    host_path = "../src/dnsrelay.txt";
    remote_dns = "10.3.9.5";
    debug_mode = 0;
    log_mode = 0;
    init(argc, argv, cache, trie);
    // // 初始化线程池和等待队列
    // struct ThreadPool threadPool;
    // init_thread_pool(&threadPool);

    server_sock = create_socket();
    int loop = 0;
    // 从单客户端 --> 多客户端
    for (;;) {
        printf("----------------------\nStarting loop %d\n--------------------\n", loop);
        int nbytes = -1;
        memset(buffer, 0, sizeof(buffer));
        address_t sender_addr;
        if (socket_recv(server_sock, &sender_addr, buffer, sizeof(buffer), &nbytes) != 0) {
            printf("Have some problem");
            exit(EXIT_FAILURE);
        }
        if (nbytes == -1) continue;
        printf("received from %s\n", inet_ntoa(sender_addr.sin_addr));
        if (debug_mode)
            printf("Received %d bytes from sender\n", nbytes);
        unsigned short offset = 0;
        // 解析DNS消息
        DNS_MSG *msg = bytestream_to_dnsmsg(buffer, &offset);
        if (msg == NULL) {
            printf("Failed to parse DNS message\n");
            exit(EXIT_FAILURE);
        }
        if (msg->header->qr == HEADER_QR_QUERY && msg->header->opcode == HEADER_OPCODE_QUERY) {
            // 内部细分多线程处理
            // 如果请求是标准查询请求,且个数为1,则直接处理
            if (msg->header->qdcount != 1)
                printf("Unsupported: query qdcount is %d instead of 1, discarded\n", msg->header->qdcount);
            else {
                // // 开启多线程处理
                // // 从线程池中取出一个线程，如果线程池已满，则将请求放入等待队列中
                // WaitForSingleObject(semaphore, INFINITE);
                // EnterCriticalSection(&threadPoolCS);
                // struct ThreadParam *param = NULL;
                // if (threadPool.count > 0) {
                //     param = threadPool.params[--threadPool.count];
                // }
                // LeaveCriticalSection(&threadPoolCS);

                // if (param->clientAddrLen == 0) {
                //     // 如果线程池中有空闲线程，则将请求分配给该线程处理
                //     param->clientAddr = sender_addr;
                //     param->clientAddrLen = sizeof(sender_addr);
                //     param->msg = msg;
                //     param->len = nbytes;
                //     _beginthreadex(NULL, 0, threadProc, param, 0, NULL);
                // } else {
                //     // 如果线程池已满，则将请求放入等待队列中
                //     param = (struct ThreadParam *)malloc(sizeof(struct ThreadParam));
                //     param->clientAddr = sender_addr;
                //     param->clientAddrLen = sizeof(sender_addr);
                //     param->msg = msg;
                //     param->len = nbytes;
                //     EnterCriticalSection(&threadPoolCS);
                //     add_to_pool(&threadPool, param);
                //     LeaveCriticalSection(&threadPoolCS);
                // }
                handle_client_request(server_sock, sender_addr, msg, nbytes);
            }

        } else if (msg->header->qr == HEADER_QR_ANSWER && sender_addr.sin_addr.s_addr == inet_addr(remote_dns)) {
            address_t client_addr;
            handle_server_response(server_sock, client_addr, msg, sizeof(buffer));
        }
        releaseMsg(msg);
    }
    cleanup_id_mapping();
    clearCache(cache);
    free(cache);
    cleanup_socket(&server_sock);
    // trie树的内存静态分配
    free(trie);

    return 0;
}
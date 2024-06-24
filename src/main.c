#include "config.h"
#include "dns_relay_server.h"

void signal_handler(int signal) {
    fprintf(stderr, "Received signal %d\n", signal);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    // 创建字典树和缓存表
    trie = (struct Trie *)malloc(sizeof(struct Trie));
    cache = (struct Cache *)malloc(sizeof(struct Cache));
    host_path = "../src/dnsrelay.txt";
    remote_dns = "10.3.9.5";
    debug_mode = 0;
    log_mode = 0;
    init(argc, argv, cache, trie);
    signal(SIGSEGV, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
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
            printf("Received %d bytes from sender\n Send from %d port\n", nbytes, ntohs(sender_addr.sin_port));
        unsigned short offset = 0;
        // 解析DNS消息
        DNS_MSG *msg = bytestream_to_dnsmsg(buffer, &offset);
        if (msg == NULL) {
            printf("Failed to parse DNS message\n");
            exit(EXIT_FAILURE);
        }
        if (msg->header->qr == HEADER_QR_QUERY && msg->header->opcode == HEADER_OPCODE_QUERY) {
            // 如果请求是标准查询请求,且个数为1,则直接处理
            if (msg->header->qdcount != 1)
                printf("Unsupported: query qdcount is %d instead of 1, discarded\n", msg->header->qdcount);
            else {
                handle_client_request(server_sock, sender_addr, msg, nbytes);
            }

        } else if (msg->header->qr == HEADER_QR_ANSWER && sender_addr.sin_addr.s_addr == inet_addr(remote_dns)) {
            address_t client_addr;
            handle_server_response(server_sock, client_addr, msg, sizeof(buffer));
        } else if (msg->header->qr == HEADER_QR_QUERY) {
            // 构造ID映射,直接转发请求到远程DNS服务器
            unsigned short original_id = msg->header->id;
            unsigned short relay_id = generate_unique_id();
            // 将ID映射关系添加到映射表中
            store_id_mapping(original_id, relay_id, sender_addr, sizeof(sender_addr));
            // 修改DNS消息头部
            msg->header->id = relay_id;
            // 将DNS消息转换为字节流
            unsigned char *buf = dnsmsg_to_bytestream(msg);
            printf("The id mapping is %d -> %d, client address is %s\n", original_id, relay_id, inet_ntoa(sender_addr.sin_addr));
            // 转发DNS请求
            forward_dns_request(server_sock, buf, nbytes);
            free(buf);
        }
        releaseMsg(msg);
        loop++;
    }

    printf("Server is shutting down\n");

    cleanup_id_mapping();
    clearCache(cache);
    free(cache);
    cleanup_socket(&server_sock);
    // trie树的内存静态分配
    free(trie);

    return 0;
}
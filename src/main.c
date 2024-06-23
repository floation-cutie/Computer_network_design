#include "config.h"
#include "dns_relay_server.h"

int main(int argc, char *argv[]) {
    // 创建字典树和缓存表
    trie = (struct Trie *)malloc(sizeof(struct Trie));
    cache = (struct Cache *)malloc(sizeof(struct Cache));
    host_path = "./dnsrelay.txt";
    remote_dns = "10.3.9.5";
    debug_mode = 0;
    log_mode = 0;
    init(argc, argv, cache, trie);

    server_sock = create_socket();
    // 从单客户端 --> 多客户端
    for (;;) {
        int nbytes = -1;
        memset(buffer, 0, sizeof(buffer));
        address_t sender_addr;
        if (socket_recv(server_sock, &sender_addr, buffer, sizeof(buffer), &nbytes) != 0) {
            return;
        }
        if (debug_mode)
            printf("Received %d bytes from sender\n", nbytes);
        unsigned short offset = 0;
        // 解析DNS消息
        DNS_MSG *msg = bytestream_to_dnsmsg(buffer, &offset);
        if (msg == NULL) {
            printf("Failed to parse DNS message\n");
            return;
        }
        if (msg->header->qr == HEADER_QR_QUERY && msg->header->opcode == HEADER_OPCODE_QUERY) {
            // 内部细分多线程处理
            // 如果请求是标准查询请求,且个数为1,则直接处理
            if (msg->header->qdcount != 1)
                printf("Unsupported: query qdcount is %d instead of 1, discarded\n", msg->header->qdcount);
            else
                handle_client_request(server_sock, sender_addr, msg, nbytes);
        } else if (msg->header->qr == HEADER_QR_ANSWER && sender_addr.sin_addr.s_addr == inet_addr(remote_dns)) {
            handle_server_response(server_sock, sender_addr, msg, nbytes);
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
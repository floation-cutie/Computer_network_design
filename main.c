#include "config.h"
#include "dns_relay_server.h"
#include "log.h"

int debug_mode = 0; // 全局调试模式开关

void signal_handler(int signal) {
    log_message("ERROR: Received signal %d", signal);
    close_log(); // 确保关闭日志文件
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    init_log(); // 初始化日志文件并清空旧内容
    log_message("INFO: 程序启动");

    // 设置调试模式
    if (argc > 1 && strcmp(argv[1], "--debug") == 0) {
        debug_mode = 1;
    }

    trie = (struct Trie *)malloc(sizeof(struct Trie));
    cache = (struct Cache *)malloc(sizeof(struct Cache));
    host_path = "../src/dnsrelay.txt";
    remote_dns = "10.3.9.5";
    log_mode = 0;
    init(argc, argv, cache, trie);
    signal(SIGSEGV, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    server_sock = create_socket();

    int loop = 0;
    for (;;) {
        log_message("INFO: ----------------------\nStarting loop %d\n--------------------", loop);
        int nbytes = -1;
        memset(buffer, 0, sizeof(buffer));
        address_t sender_addr;
        if (socket_recv(server_sock, &sender_addr, buffer, sizeof(buffer), &nbytes) != 0) {
            log_message("ERROR: Have some problem");
            close_log(); // 确保关闭日志文件
            exit(EXIT_FAILURE);
        }
        if (nbytes == -1) continue;
        log_message("INFO: received from %s", inet_ntoa(sender_addr.sin_addr));
        if (debug_mode)
            log_message("DEBUG: Received %d bytes from sender, Send from %d port", nbytes, ntohs(sender_addr.sin_port));
        unsigned short offset = 0;
        DNS_MSG *msg = bytestream_to_dnsmsg(buffer, &offset);
        if (msg == NULL) {
            log_message("ERROR: Failed to parse DNS message");
            close_log(); // 确保关闭日志文件
            exit(EXIT_FAILURE);
        }
        if (msg->header->qr == HEADER_QR_QUERY && msg->header->opcode == HEADER_OPCODE_QUERY) {
            if (msg->header->qdcount != 1)
                log_message("WARNING: Unsupported: query qdcount is %d instead of 1, discarded", msg->header->qdcount);
            else {
                handle_client_request(server_sock, sender_addr, msg, nbytes);
            }
        } else if (msg->header->qr == HEADER_QR_ANSWER && sender_addr.sin_addr.s_addr == inet_addr(remote_dns)) {
            address_t client_addr;
            handle_server_response(server_sock, client_addr, msg, sizeof(buffer));
        } else if (msg->header->qr == HEADER_QR_QUERY) {
            unsigned short original_id = msg->header->id;
            unsigned short relay_id = generate_unique_id();
            store_id_mapping(original_id, relay_id, sender_addr, sizeof(sender_addr));
            msg->header->id = relay_id;
            unsigned char *buf = dnsmsg_to_bytestream(msg);
            log_message("INFO: The id mapping is %d -> %d, client address is %s", original_id, relay_id, inet_ntoa(sender_addr.sin_addr));
            forward_dns_request(server_sock, buf, nbytes);
            free(buf);
        }
        releaseMsg(msg);
        loop++;
    }

    log_message("INFO: Server is shutting down");

    cleanup_id_mapping();
    clearCache(cache);
    free(cache);
    cleanup_socket(&server_sock);
    free(trie);

    close_log(); // 确保关闭日志文件

    return 0;
}

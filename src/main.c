#include "config.h"
#include "dns_relay_server.h"

int main(int argc, char *argv[]) {
    // 创建字典树和缓存表
    struct Trie *trie = (struct Trie *)malloc(sizeof(struct Trie));

    struct Cache *cache = (struct Cache *)malloc(sizeof(struct Cache));
    host_path = "./dnsrelay.txt";
    remote_dns = "10.3.9.5";
    debug_mode = 0;
    log_mode = 0;
    init(argc, argv, cache, trie);

    server_sock = create_socket();

    while (1) {
        receive_from_client();
        receive_from_server();
    }

    cleanup_id_mapping();
    clearCache(cache);
    free(cache);
    cleanup_socket(&server_sock);

    free(trie);

    return 0;
}
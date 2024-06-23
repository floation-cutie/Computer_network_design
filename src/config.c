#include "config.h"

void init(int argc, char *argv[], struct Cache *cache, struct Trie *trie) {
    /* 获取程序运行参数 */
    get_config(argc, argv);

    /* 初始化socket */
    initialize_winsock();

    /* 初始化ID映射表 */
    init_id_mapping();

    /* 初始化缓存 */
    initCache(cache);

    /* 初始化二级缓存 */
    initTrie(trie);
    loadLocalTable(trie);
}

void print_help_info() {
    printf("-------------------------------------------------------------------------------\n");
    printf("|                       Welcome to use DNS relay!                             |\n");
    printf("| Please submit your query by terminal, and watch the answer in your terminal.|\n");
    printf("|                  Example: nslookup www.baidu.com 127.0.0.1                  |\n");
    printf("|     Arguments: -h:                  print basic information                 |\n");
    printf("|                -d:                  print debug information                 |\n");
    printf("|                -p [relay_file_path]:set 2-level cache path                  |\n");
    printf("|                -l [log_file_path]:  save log                                |\n");
    printf("|                -s [server_address]: set remote DNS server                   |\n");
    printf("-------------------------------------------------------------------------------\n");
}

/* 读取程序命令参数 */
void get_config(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "hdp:l:s:")) != -1) {
        switch (opt) {
        case 'h':
            print_help_info();
            exit(0);
        case 'd':
            debug_mode = 1;
            break;
        case 'p':
            host_path = optarg;
            break;
        case 'l':
            log_path = optarg;
            log_mode = 1;
            break;
        case 's':
            remote_dns = optarg;
            break;
        default:
            break;
        }
    }
}
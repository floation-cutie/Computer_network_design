#include "config.h"

int vaild_num(char *arg) {
    char *endptr;
    errno = 0; // 重置errno

    // 使用strtol函数进行转换
    long val = strtol(arg, &endptr, 10);

    // 检查转换结果是否有效
    if (errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) {
        printf("For timer: Out of range error occurred\n");
    } else if (errno != 0) {
        printf("For timer: Conversion error occurred\n");
    } else if (endptr == arg) {
        printf("For timer: No digits were found\n");
    } else if (*endptr != '\0') {
        printf("For timer: Further characters after number: %s\n", endptr);
    } else {
        return val;
    }
    return -1;
}

void init(int argc, char *argv[], struct Cache *cache, struct Trie *trie) {
    /* 获取程序运行参数 */
    get_config(argc, argv);

    /* 初始化缓存 */
    initCache(cache);

    /* 初始化二级缓存 */
    initTrie(trie);
    loadLocalTable(trie, host_path);
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
    printf("|                -t [ack timer]: set expire time between relay and remote DNS |\n");
    printf("-------------------------------------------------------------------------------\n");
}

/* 读取程序命令参数 */
void get_config(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "hdp:l:s:t:")) != -1) {
        switch (opt) {
        case 'h':
            print_help_info();
            exit(0);
        case 'd':
            debug_mode = 1;
            puts("Set debug mode");
            break;
        case 'p':
            host_path = optarg;
            break;
        case 'l':
            log_path = optarg;
            log_mode = 1;
            puts("Set log mode");
            break;
        case 's':
            remote_dns = optarg;
            break;
        case 't':
            timer = vaild_num(optarg);
            if (timer == -1)
                exit(0);
            break;
        default:
            puts("Dont't support the specified parameters");
            print_help_info();
            exit(0);
            break;
        }
    }
}
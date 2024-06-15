#include "config.h"

host_path = "./dnsrelay.txt";
int debug_mode = 0;
int log_mode = 0;

void init(int argc, char *argv[]) {
    /* 获取程序运行参数 */
    get_config(argc, argv);

    /* 初始化socket */
    init_socket();

    /* 初始化ID映射表 */
    init_ID_list();

    /* 初始化缓存 */
    init_cache();

    /* 初始化HOST文件 */
    read_host();
}

void print_help_info() {

    printf("-------------------------------------------------------------------------------\n");
    printf("|                       Welcome to use DNS relay!                              |\n");
    printf("| Please submit your query by terminal, and watch the answer in your terminal.|\n");
    printf("|                  Example: nslookup www.baidu.com 127.0.0.1                  |\n");
    printf("|     Arguments: -b:                  print basic information                 |\n");
    printf("|                -d:                  print debug information                 |\n");
    printf("|                -l:                  print log                               |\n");
    printf("|                -s [server_address]: set remote DNS server                   |\n");
    printf("|                -m [mode]: set mode, 0: nonblock, 1: poll                    |\n");
    printf("-------------------------------------------------------------------------------\n");
}

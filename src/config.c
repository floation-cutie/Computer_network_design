// #include "config.h"

// host_path = "./dnsrelay.txt";
// int debug_mode = 0;
// int log_mode = 0;

// void init(int argc, char *argv[]) {
//     /* 获取程序运行参数 */
//     get_config(argc, argv);

//     /* 初始化socket */
//     init_socket();

//     /* 初始化ID映射表 */
//     init_ID_list();

//     /* 初始化缓存 */
//     init_cache();

//     /* 初始化HOST文件 */
//     read_host();
// }

// void print_help_info() {

//     printf("-------------------------------------------------------------------------------\n");
//     printf("|                       Welcome to use DNS relay!                              |\n");
//     printf("| Please submit your query by terminal, and watch the answer in your terminal.|\n");
//     printf("|                  Example: nslookup www.baidu.com 127.0.0.1                  |\n");
//     printf("|     Arguments: -b:                  print basic information                 |\n");
//     printf("|                -d:                  print debug information                 |\n");
//     printf("|                -l:                  save log                                |\n");
//     printf("|                -s [server_address]: set remote DNS server                   |\n");
//     printf("|                -m [mode]: set mode, 0: nonblock, 1: poll                    |\n");
//     printf("-------------------------------------------------------------------------------\n");
// }

// /* 读取程序命令参数 */
// void get_config(int argc, char *argv[]) {

//     print_help_info();

//     for (int index = 1; index < argc; index++) {
//         /* 调试模式 */
//         if (strcmp(argv[index], "-d") == 0) {
//             debug_mode = 1;
//         }

//         /* 日志模式 */
//         if (strcmp(argv[index], "-l") == 0) {
//             log_mode = 1;
//         }

//         /* 输出系统基本信息 */
//         else if (strcmp(argv[index], "-i") == 0) {
//             printf("Hosts path: %s\n", host_path);
//             printf("Remote DNS server address: %s (default: 10.3.9.45, BUPT DNS) \n", remote_dns);
//             printf("mode: ");
//             printf(mode == 0 ? "nonblock\n" : "poll\n");
//         }

//         /* 设置远程DNS服务器 */
//         else if (strcmp(argv[index], "-s") == 0) {
//             char *addr = malloc(16);
//             memset(addr, 0, 16);
//             index++;
//             memcpy(addr, argv[index], strlen(argv[index]) + 1);
//             remote_dns = addr;
//         }

//         else if (strcmp(argv[index], "-m") == 0) {
//             index++;
//             if (strcmp(argv[index], "0") == 0) {
//                 mode = 0;
//             } else if (strcmp(argv[index], "1") == 0) {
//                 mode = 1;
//             }
//         }
//     }
// }
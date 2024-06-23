#include <getopt.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "bd:lsm:")) != -1) {
        switch (opt) {
        case 'b':
            printf("print basic information\n");
            break;
        case 'd':
            printf("print debug information\n");
            printf("optarg: %s\n", optarg);
            break;
        case 'l':
            printf("save log\n");
            break;
        case 's':
            printf("set remote DNS server\n");
            break;
        case 'm':
            printf("set mode\n");
            printf("optarg: %s\n", optarg);
            break;
        default:
            break;
        }
    }
    return 0;
}
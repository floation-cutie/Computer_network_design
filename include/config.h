#pragma once

#include "debug_info.h"
#include "dns_msg.h"
#include "socket.h"
#include <stdio.h>

char *host_path; // 配置文件
char *log_path;  // 日志文件

int debug_mode;
int log_mode;

void init(int argc, char *argv[]);
void get_config();
void print_help_info();
void read_host();
void get_host_info(FILE *ptr);
void write_log(char *domain, uint8_t *ip_addr);
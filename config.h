#pragma once
#include "log.h"

extern char *log_path;  // 声明日志文件路径变量

#include "cache.h"
#include "debug_info.h"
#include "dns_msg.h"
#include "getopt.h"
#include "trie.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
char *host_path; // 静态缓存文件
char *log_path;  // 日志文件
char *remote_dns;
int debug_mode;
int log_mode;
long timer;
void init(int argc, char *argv[], struct Cache *cache, struct Trie *trie);
void get_config(int argc, char *argv[]);
void print_help_info();
int vaild_num(char *arg);
#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

extern FILE *logfile;
extern char *log_path; // 声明日志文件路径变量

void init_log();
void log_message(const char *format, ...);
void close_log();

#endif // LOG_H
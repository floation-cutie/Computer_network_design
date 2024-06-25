#include "log.h"
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

FILE *logfile = NULL;

void init_log() {
    logfile = fopen("C:/Users/Admin/Desktop/new_version/Computer_network_design/src/logfile.txt", "w"); // 使用写模式清空文件
    if (logfile == NULL) {
        fprintf(stderr, "无法打开日志文件: %s\n", strerror(errno)); // 输出详细错误信息
    } else {
        fclose(logfile); // 立即关闭，后续以追加模式打开
        logfile = NULL;
    }
}

void log_message(const char *format, ...) {
    if (logfile == NULL) {
        logfile = fopen("C:/Users/Admin/Desktop/new_version/Computer_network_design/src/logfile.txt", "a"); // 使用追加模式
        if (logfile == NULL) {
            fprintf(stderr, "无法打开日志文件: %s\n", strerror(errno)); // 输出详细错误信息
            return;
        }
    }

    va_list args;
    va_start(args, format);

    time_t now = time(NULL);
    char timestr[20];
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(logfile, "[%s] ", timestr);

    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);

    for (char *p = buffer; *p; p++) {
        if (isprint((unsigned char)*p) || isspace((unsigned char)*p)) {
            fputc(*p, logfile);
        } else {
            fputc('.', logfile); // 将不可打印字符替换为'.'
        }
    }

    fprintf(logfile, "\n");

    va_end(args);

    fflush(logfile); // 确保实时写入文件
}

void close_log() {
    if (logfile != NULL) {
        fclose(logfile);
        logfile = NULL;
    }
}

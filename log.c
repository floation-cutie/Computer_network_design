#include "log.h"
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

FILE *logfile = NULL;
extern char *log_path;  // 确保从其他地方引入 log_path 变量

void init_log() {
    if (log_path == NULL) {
        fprintf(stderr, "日志文件路径未指定\n");
        return;
    }

    logfile = fopen(log_path, "w"); // 使用写模式清空文件
    if (logfile == NULL) {
        fprintf(stderr, "无法打开日志文件: %s\n", strerror(errno)); // 输出详细错误信息
    } else {
        // 保持日志文件打开，后续以追加模式写入
        fclose(logfile); // 立即关闭，后续以追加模式打开
        logfile = NULL;
    }
}

void log_message(const char *format, ...) {
    va_list args;
    va_start(args, format);

    // 输出到控制台
    vprintf(format, args);
    printf("\n");

    // 输出到日志文件
    if (logfile == NULL) {
        logfile = fopen(log_path, "a"); // 使用追加模式
        if (logfile == NULL) {
            fprintf(stderr, "无法打开日志文件: %s\n", strerror(errno)); // 输出详细错误信息
            va_end(args);
            return;
        }
    }

    time_t now = time(NULL);
    char timestr[20];
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(logfile, "[%s] ", timestr);
    vfprintf(logfile, format, args);
    fprintf(logfile, "\n");
    fflush(logfile); // 确保实时写入文件

    va_end(args);
}

void log_byte_stream(const unsigned char *data, int length) {
    // 输出到控制台
    printf("Byte stream: ");
    for (int i = 0; i < length; i++) {
        printf("%02x ", data[i]);
    }
    printf("\n");

    // 输出到日志文件
    if (logfile == NULL) {
        logfile = fopen(log_path, "a"); // 使用追加模式
        if (logfile == NULL) {
            fprintf(stderr, "无法打开日志文件: %s\n", strerror(errno)); // 输出详细错误信息
            return;
        }
    }

    time_t now = time(NULL);
    char timestr[20];
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(logfile, "[%s] Byte stream: ", timestr);
    for (int i = 0; i < length; i++) {
        fprintf(logfile, "%02x ", data[i]);
    }
    fprintf(logfile, "\n");
    fflush(logfile); // 确保实时写入文件
}

void close_log() {
    if (logfile != NULL) {
        fclose(logfile);
        logfile = NULL;
    }
}

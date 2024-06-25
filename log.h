#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

void log_message(const char *format, ...);
void close_log();

#endif // LOG_H

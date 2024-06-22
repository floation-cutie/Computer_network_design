#include <stdio.h>
#include <time.h>

int main() {
    time_t current_time = time(NULL);
    struct tm *local_time = localtime(&current_time);

    char time_str[20]; // YYYY-MM-DD HH:MM:SS\0
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);

    printf("Current time: %s\n", time_str);
    return 0;
}

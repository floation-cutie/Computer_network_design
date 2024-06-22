#include <stdio.h>
#include <time.h>

int main() {
    time_t currentTime = time(NULL);
    // 输出当前时间,以秒为单位
    printf("Current time: %ld\n", currentTime);
    return 0;
}
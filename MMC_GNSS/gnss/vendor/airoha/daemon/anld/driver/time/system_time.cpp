#include "system_time.h"
#include <time.h>

int64_t airoha::getSystemRealTimeInMs() {
    int64_t timeMs;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    timeMs = (int64_t)ts.tv_sec * 1000 + (int64_t)ts.tv_nsec / 1000000;
    return timeMs;
}
int64_t airoha::getSystemRealTimeInNs() {
    int64_t timeNs;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    timeNs = (int64_t)ts.tv_sec * 1000000000 + (int64_t)ts.tv_nsec;
    return timeNs;
}
int64_t getSystemMonoTimeMs() {
    int64_t timeMs;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    timeMs = (int64_t)ts.tv_sec * 1000 + (int64_t)ts.tv_nsec / 1000000;
    return timeMs;
}
int64_t getSystemMonoTimeNs() {
    int64_t timeNs;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    timeNs = (int64_t)ts.tv_sec * 1000000000 + (int64_t)ts.tv_nsec;
    return timeNs;
}

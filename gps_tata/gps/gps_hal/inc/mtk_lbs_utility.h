#ifndef __MTK_LBS_UTILITY_H__
#define __MTK_LBS_UTILITY_H__

#include <time.h>
#include <stdint.h>
#include "mtk_auto_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************
* Basic Utilities
**************************************************/
#define  MTK_GPS_NVRAM  0

#if defined(__ANDROID_OS__)
#define  ANDROID_MNLD_PROP_SUPPORT 1
#elif defined(__LINUX_OS__)
#define  ANDROID_MNLD_PROP_SUPPORT 0
#endif

void msleep(int interval);

#define MNLD_STRNCPY(dst,src,size) do{\
                                       strncpy((char *)(dst), (src), (size - 1));\
                                      (dst)[size - 1] = '\0';\
                                     }while(0)

/*************************************************
* Timer
**************************************************/
typedef void (* timer_callback)();


// in millisecond
time_t get_tick();

// in millisecond
time_t get_time_in_millisecond();

/*************************************************
* Epoll
**************************************************/
// -1 means failure
int epoll_add_fd(int epfd, int fd);

// -1 failed
int epoll_add_fd2(int epfd, int fd, uint32_t events);

// -1 failed
int epoll_del_fd(int epfd, int fd);

int epoll_mod_fd(int epfd, int fd, uint32_t events);

/*************************************************
* Local UDP Socket
**************************************************/
// -1 means failure
int socket_bind_udp(const char* path);

// -1 means failure
int socket_set_blocking(int fd, int blocking);

// -1 means failure
int safe_sendto(const char* path, const char* buff, int len);

// -1 means failure
int safe_recvfrom(int fd, char* buff, int len);

// -1 means failure
int start_timer(timer_t timerid, int milliseconds);

// -1 means failure
int stop_timer(timer_t timerid);

// -1 means failure
timer_t init_timer(timer_callback cb);

// -1 means failure
int deinit_timer(timer_t timerid);

#ifdef __cplusplus
}
#endif

#endif

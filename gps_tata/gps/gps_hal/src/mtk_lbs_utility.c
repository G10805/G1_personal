#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <stddef.h>  // offsetof
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>  // usleep
#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>  // inet_addr
#include <sys/un.h>  // struct sockaddr_un
#include <pthread.h>
#include <sys/epoll.h>
#include <signal.h>
#include <semaphore.h>

#include "mtk_lbs_utility.h"
#include "mtk_auto_log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "mtk_lbs_utility"
#endif

void msleep(int interval) {
    usleep(interval * 1000);
}

// in millisecond
time_t get_tick() {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        LOGE("clock_gettime failed reason=[%s]", strerror(errno));
        return -1;
    }
    return (ts.tv_sec*1000) + (ts.tv_nsec/1000000);
}

// in millisecond
time_t get_time_in_millisecond() {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        LOGE("get_time_in_millisecond  failed reason=[%s]", strerror(errno));
        return -1;
    }
    return ((long long)ts.tv_sec*1000) + ((long long)ts.tv_nsec/1000000);
}
#ifdef __LINUX_OS__ 
/*************************************************
* Timer
**************************************************/
// -1 means failure
timer_t init_timer_id(timer_callback cb, int id) {
    struct sigevent sevp;
    timer_t timerid;

    memset(&sevp, 0, sizeof(sevp));
    sevp.sigev_value.sival_int = id;
    sevp.sigev_notify = SIGEV_THREAD;
    sevp.sigev_notify_function = cb;

    if (timer_create(CLOCK_MONOTONIC, &sevp, &timerid) == -1) {
        LOGE("timer_create  failed reason=[%s]", strerror(errno));
        return (timer_t)-1;
    }
    return timerid;
}

// -1 means failure
timer_t init_timer(timer_callback cb) {
    return init_timer_id(cb, 0);
}

// -1 means failure
int start_timer(timer_t timerid, int milliseconds) {
    struct itimerspec expire;
    expire.it_interval.tv_sec = 0;
    expire.it_interval.tv_nsec = 0;
    expire.it_value.tv_sec = milliseconds/1000;
    expire.it_value.tv_nsec = (milliseconds%1000)*1000000;
    return timer_settime(timerid, 0, &expire, NULL);
}

// -1 means failure
int stop_timer(timer_t timerid) {
    return start_timer(timerid, 0);
}

// -1 means failure
int deinit_timer(timer_t timerid) {
    if (timer_delete(timerid) == -1) {
        // errno
        return -1;
    }
    return 0;
}

#endif
/*************************************************
* Epoll
**************************************************/
// -1 means failure
int epoll_add_fd(int epfd, int fd) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    // don't set the fd to edge trigger
    // the some event like accept may be lost if two or more clients are connecting to server at the same time
    // level trigger is preferred to avoid event lost
    // do not set EPOLLOUT due to it will always trigger when write is available
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        LOGE("epoll_add_fd() epoll_ctl() failed reason=[%s]%d epfd=%d fd=%d",
            strerror(errno), errno, epfd, fd);
        return -1;
    }
    return 0;
}

// -1 failed
int epoll_add_fd2(int epfd, int fd, uint32_t events) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.fd = fd;
    ev.events = events;
    // don't set the fd to edge trigger
    // the some event like accept may be lost if two or more clients are connecting to server at the same time
    // level trigger is preferred to avoid event lost
    // do not set EPOLLOUT due to it will always trigger when write is available
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        LOGE("epoll_add_fd2() epoll_ctl() failed reason=[%s]%d epfd=%d fd=%d",
            strerror(errno), errno, epfd, fd);
        return -1;
    }
    return 0;
}

int epoll_del_fd(int epfd, int fd) {
    struct epoll_event  ev;
    int                 ret;

    if (epfd == -1)
        return -1;

    ev.events  = EPOLLIN;
    ev.data.fd = fd;
    do {
        ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
    } while (ret < 0 && errno == EINTR);
    return ret;
}

// -1 failed
int epoll_mod_fd(int epfd, int fd, uint32_t events) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.fd = fd;
    ev.events = events;
    if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
        LOGE("epoll_mod_fd() epoll_ctl() failed reason=[%s]%d epfd=%d fd=%d",
            strerror(errno), errno, epfd, fd);
        return -1;
    }
    return 0;
}

/*************************************************
* Local UDP Socket
**************************************************/
// -1 means failure
int socket_bind_udp(const char* path) {
    int fd;
    struct sockaddr_un addr;

    fd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (fd < 0) {
        LOGE("socket_bind_udp() socket() failed reason=[%s]%d",
            strerror(errno), errno);
        return -1;
    }
    LOGD("fd=%d,path=%s\n", fd, path);

    memset(&addr, 0, sizeof(addr));
    addr.sun_path[0] = 0;
    MNLD_STRNCPY(addr.sun_path + 1, path,sizeof(addr.sun_path) - 1);
    addr.sun_family = AF_UNIX;
    unlink(path);

    if (bind(fd, (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
        LOGE("socket_bind_udp() bind() failed path=[%s] reason=[%s]%d",
            addr.sun_path+1, strerror(errno), errno);
        close(fd);
        return -1;
    }else
        LOGI("bind ok path=[%s]", addr.sun_path+1);
    return fd;
}

// -1 means failure
int socket_set_blocking(int fd, int blocking) {
    if (fd < 0) {
        LOGE("socket_set_blocking() invalid fd=%d", fd);
        return -1;
    }

    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        LOGE("socket_set_blocking() fcntl() failed invalid flags=%d reason=[%s]%d",
            flags, strerror(errno), errno);
        return -1;
    }

    flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);
    return (fcntl(fd, F_SETFL, flags) == 0) ? 0 : -1;
}

// -1 means failure
int safe_sendto(const char* path, const char* buff, int len) {
    int ret = 0;
    struct sockaddr_un addr;
    int retry = 10;
    int fd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (fd < 0) {
        LOGE("safe_sendto() socket() failed reason=[%s]%d",
            strerror(errno), errno);
        return -1;
    }

    int flags = fcntl(fd, F_GETFL, 0);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1){
        LOGE("fcntl failed reason=[%s]%d",
                    strerror(errno), errno);

        close(fd);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_path[0] = 0;
    MNLD_STRNCPY(addr.sun_path + 1, path,sizeof(addr.sun_path) - 1);
    addr.sun_family = AF_UNIX;

    while ((ret = sendto(fd, buff, len, 0,
        (const struct sockaddr *)&addr, sizeof(addr))) == -1) {
        if (errno == EINTR) {
            LOGE("errno==EINTR\n");
            continue;
        }
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                LOGE("errno==EAGAIN\n");
                continue;
            }
        }
        LOGE("safe_sendto() sendto() failed path=[%s] ret=%d reason=[%s]%d",
            path, ret, strerror(errno), errno);
        break;
    }

    close(fd);
    return ret;
}

// -1 means failure
int safe_recvfrom(int fd, char* buff, int len) {
    int ret = 0;
    int retry = 10;
    int flags = fcntl(fd, F_GETFL, 0);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1){
        LOGE("fcntl failed reason=[%s]%d",
                    strerror(errno), errno);

        close(fd);
        return -1;
    }

    while ((ret = recvfrom(fd, buff, len, 0,
         NULL, NULL)) == -1) {
        LOGW("safe_recvfrom() ret=%d len=%d", ret, len);
        if (errno == EINTR) continue;
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
        }
        LOGE("safe_recvfrom() recvfrom() failed reason=[%s]%d",
            strerror(errno), errno);
        break;
    }
    return ret;
}

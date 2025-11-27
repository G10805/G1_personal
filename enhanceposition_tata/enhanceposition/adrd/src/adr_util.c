/*
* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2017. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <errno.h>
#include <sys/un.h>  /*struct sockaddr_un*/
#include <sys/stat.h>

#include "adr_util.h"
#include "config-parser.h"

extern struct adr_config *config;
extern char *log_path;

static int rtc_fd = -1;
#define RTC_DEV_PATH    "/dev/rtc0"

void
module_list_init(struct module_list *list)
{
    list->prev = list;
    list->next = list;
}

void
module_list_insert(struct module_list *list, struct module_list *elm)
{
    elm->prev = list;
    elm->next = list->next;
    list->next = elm;
    elm->next->prev = elm;
}

void
module_list_remove(struct module_list *elm)
{
    elm->prev->next = elm->next;
    elm->next->prev = elm->prev;
    elm->next = NULL;
    elm->prev = NULL;
}

int
module_list_length(const struct module_list *list)
{
    struct module_list *e;
    int count = 0;

    e = list->next;
    while (e != list) {
        e = e->next;
        count++;
    }
    return count;
}

int
module_list_empty(const struct module_list *list)
{
    return list->next == list;
}

void
module_list_insert_list(struct module_list *list, struct module_list *other)
{
    if (module_list_empty(other))
        return;

    other->next->prev = list;
    other->prev->next = list->next;
    list->next->prev = other->prev;
    list->next = other->next;
}

unsigned char sentence_checksum_calc(const char *sentence,int len)
{
    unsigned char checksum = 0;
    while (*sentence && len)
    {
        checksum ^= (unsigned char)*sentence++;
        len--;
    }
    return checksum;
}

int set_rtc_time( struct rtc_time *tm)
{
    int ret, fd = rtc_fd;
    if (fd < 0) {
        LOG_ERROR("set rtc time error fd < 0");
        return -1;
    }
    ret = ioctl(fd, RTC_SET_TIME, tm);
    if (ret < 0) {
        LOG_ERROR("set rtc time failed!!!!!");
        return -1;
    }
    return 0;
}

int get_rtc_time(struct rtc_time *tm)
{
    int ret,fd = rtc_fd;
    if (fd < 0) {
        LOG_ERROR("get rtc time error fd < 0");
        return -1;
    }
    ret = ioctl(fd, RTC_RD_TIME, tm);
    if (ret < 0) {
        LOG_ERROR("read rtc data failed");
        return -1;
    }

    return 0;
}

void open_rtc_dev()
{
    int fd = open(RTC_DEV_PATH, O_RDWR);
    if (fd < 0) {
        LOG_ERROR("rtctest: Failed to open %s", RTC_DEV_PATH);
        return;
    }
    rtc_fd = fd;
}

void close_rtc_dev()
{
    int fd = rtc_fd;
    if (fd >= 0)
        close(fd);
}

void get_boot_time(struct timespec* boottime)
{
    struct timespec time;

    time.tv_sec = time.tv_nsec = 0;
    syscall(SYS_clock_gettime, CLOCK_BOOTTIME, &time);
    boottime->tv_sec = time.tv_sec;
    boottime->tv_nsec = time.tv_nsec;
}

void diff_tm_timespec (struct tm *param1,
        struct timespec* param2, struct timespec* out)
{
    if (param2->tv_nsec) {
        out->tv_sec = mktime(param1) - param2->tv_sec - 1;
        out->tv_nsec = 1000000000 - param2->tv_nsec;
    } else
        out->tv_sec = mktime(param1) - param2->tv_sec;
}

void diff_timespec_timespec (struct timespec *param1,
        struct timespec* param2, struct timespec* out)
{
    if (param1->tv_nsec < param2->tv_nsec) {
        out->tv_sec = param1->tv_sec - param2->tv_sec - 1;
        out->tv_nsec = param1->tv_nsec + 1000000000 - param2->tv_nsec;
    } else {
        out->tv_sec = param1->tv_sec - param2->tv_sec;
        out->tv_nsec = param1->tv_nsec - param2->tv_nsec;
    }
}


double str2float(const char* p, const char* end)
{
    int   len    = end - p;
    char  temp[16];

    if (len >= (int)sizeof(temp))
        return 0.;

    memcpy(temp, p, len);
    temp[len] = 0;
    return strtod(temp, NULL);
}

int str2int(const char* p, const char* end)
{
    int result = 0;
    int len    = end - p;
    int sign = 1;
    if (*p == '-')
    {
        sign = -1;
        p++;
        len = end - p;
    }
    for (; len > 0; len--, p++)
    {
        int  c;
        if (p >= end)
            return -1;
        c = *p - '0';
        if ((unsigned)c >= 10)
            return -1;
        result = result*10 + c;
    }

    return  sign*result;
}

char* get_log_path()
{
    char *pPath = log_path;
    if (access(pPath, F_OK) < 0) {
        LOG_INFO("not exist the log path %s, errno(%s)%d,now creating ",
            pPath, strerror(errno), errno);
        if(mkdir(pPath, 0777) == 0)
            LOG_INFO("create the log path %s successfully", pPath);
        else{
            LOG_ERROR("create the log path %s failed, errno(%s)%d",
                pPath, strerror(errno), errno);
            return NULL;
        }
    }
    return log_path;
}

/*if app is killed, then should recycle resource*/
int close_log_file(FILE *fp)
{
    if (fp) {
        fflush(fp);//flush userspace to kernel cache
        fsync(fileno(fp));//flush kernel to disk
        fseek(fp, 0L, SEEK_SET);//set begin of file
        if (fgetc(fp) == EOF)
            LOG_ERROR("SD log file is empty");
  
        fclose(fp);
    }
    fp = NULL;
    return 0;
}

/*create a file to store data, should be called when app launch*/
int open_log_file(char *suffix, FILE **fp)
{
    char logname[PATH_LEN];
    char *p = logname;
    char *end = p + sizeof(logname);
    time_t cur_time;
    struct tm local_tm;
    memset(&local_tm, 0, sizeof(local_tm));
    char *pPath = NULL;

    pPath = get_log_path();
    if (pPath == NULL) {
        LOG_ERROR("get log path failed");
        return -1;
    }
    p += snprintf(p, end -p, "%s", pPath);
    cur_time = time(NULL);
    if (cur_time < 0) {
        LOG_ERROR("get time failed, errno(%s):%d", strerror(errno), errno);
        return -1;
    }

    if (localtime_r(&cur_time, &local_tm) == NULL) {
        LOG_ERROR("localtime_r failed, errno(%s):%d", strerror(errno), errno);
        return -1;
    }

    p += snprintf(p, end -p, "/_%04d%02d%02d_%02d_%02d_%02d_",
                  (local_tm.tm_year  + 1900), local_tm.tm_mon + 1, local_tm.tm_mday,
                  local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);

    p += snprintf(p, end -p, "%s", suffix);

    LOG_INFO("debug: file's complete path is %s", logname);

    /* fopen should be done at begining of app launch */
    *fp = fopen(logname, "a+");
    if (!(*fp)) {
        LOG_ERROR("file %s open failed", logname);
        return -1;
    }

    return 0;
}

int safe_recvfrom(int fd, char* buf, int len) {
    int ret = 0;
    int retry = 10;

    while ((ret = recvfrom(fd, buf, len, 0, NULL, NULL)) == -1) {
        if (errno == EINTR) continue;
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
        }
        LOG_ERROR("reason=[%s]\n", strerror(errno));
        break;
    }
    return ret;
}

// -1 means failure
int safe_sendto(int fd, const struct sockaddr_un *addr, const char* buff, int len) {
    int ret = 0;
    int retry = 10;

    while ((ret = sendto(fd, buff, len, 0, (const struct sockaddr *)addr, sizeof(*addr))) == -1) {
        if (errno == EINTR) {
            LOG_ERROR("errno==EINTR\n");
            continue;
        }
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                LOG_ERROR("errno==EAGAIN\n");
                continue;
            }
        }
        LOG_ERROR("sendto() failed: path=[%s] ret=%d reason=[%s]%d, len[%d]",
            addr->sun_path + 1, ret, strerror(errno), errno, len);
        break;
    }

    return ret;
}

int set_fcntl(int fd, int mode) {
    int flag;

    if((flag = fcntl(fd, F_GETFL, 0)) < 0) {
        LOG_ERROR("fcntl F_GETFL failed, errno[%d]:%s",errno, strerror(errno));
        return -1;
    }
    if(fcntl(fd, F_SETFL, flag | mode) < 0) {
        LOG_ERROR("fcntl F_SETFL failed, errno[%d]:%s",errno, strerror(errno));
        return -1;
    }
    return 0;
}

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
        LOG_ERROR("epoll_add_fd() epoll_ctl() failed reason=[%s]%d epfd=%d fd=%d",
            strerror(errno), errno, epfd, fd);
        return -1;
    }
    return 0;
}


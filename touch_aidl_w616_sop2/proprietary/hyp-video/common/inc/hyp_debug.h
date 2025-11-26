/*========================================================================

*//** @file hyp_debug.h

@par FILE SERVICES:
      Hypervisor video debug header


@par EXTERNALIZED FUNCTIONS:
      See below.

Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */

/*========================================================================
                             Edit History
$Header: $

when       who     what, where, why
--------   ---     -------------------------------------------------------
06/30/24    bf     Fix DEBUG_PRINT_CTL definition under linux
03/04/19    sm     Fix logging from video FE
02/05/19    rz     Bringup changes for 8155
07/25/17    sm     Fix LA logging
06/28/17    aw     Unify and update all logs in hyp-video
05/08/17    sm     Update for new hyp-video architecture
========================================================================== */

/* =======================================================================

INCLUDE FILES FOR MODULE

========================================================================== */

#ifndef __HYP_DEBUG_H__
#define __HYP_DEBUG_H__

#include <unistd.h>
#include <string.h>

extern int debug_level;

enum {
    HYP_PRIO_ERROR=0x1,
    HYP_PRIO_HIGH=0x2,
    HYP_PRIO_LOW=0x4,
    HYP_PRIO_INFO=0x8
};

#define HYP_VIDEO_TEST_RESULT(test_name, ret)   \
                printf("%-30s %s\n", test_name, ret ? "FAIL":"PASS")
#define HYP_VIDEO_TEST_TITLE(msg_fmt)   \
                printf("\n\n\t\t" msg_fmt"\n")

#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#ifdef _LINUX_    // LV
#include <syslog.h>
#include <sys/syscall.h>

#define gettid() syscall(SYS_gettid)
#define getpid() syscall(SYS_getpid)
#define DEBUG_PRINT_CTL(level, syslog_prio, fmt, args...)   \
      do {                             \
           if (level & debug_level)           \
               syslog(syslog_prio, "[%ld:%ld]:[%s:%s] " fmt " \n", getpid(), \
                   gettid(), __FILENAME__, __FUNCTION__, ##args); \
      } while(0)

#define HYP_VIDEO_MSG_INFO( msg_fmt,args...)   \
                DEBUG_PRINT_CTL(HYP_PRIO_INFO, LOG_NOTICE, msg_fmt, ##args )
#define HYP_VIDEO_MSG_LOW( msg_fmt,args...)   \
                DEBUG_PRINT_CTL(HYP_PRIO_LOW, LOG_NOTICE, msg_fmt, ##args )
#define HYP_VIDEO_MSG_HIGH( msg_fmt,args...)   \
                DEBUG_PRINT_CTL(HYP_PRIO_HIGH, LOG_NOTICE, msg_fmt, ##args )
#define HYP_VIDEO_MSG_ERROR( msg_fmt,args...)   \
                DEBUG_PRINT_CTL(HYP_PRIO_ERROR, LOG_ERR, msg_fmt, ##args )

#elif defined(__QNXNTO__) || defined(WIN32)    // QNX or WIN32
#include "MMDebugMsg.h"
#define HYP_VIDEO_MSG_INFO( msg_fmt,...)   \
                MM_MSG_PRIO_EX(MM_VIDEO_TASK, MM_PRIO_LOW, msg_fmt, ## __VA_ARGS__ )
#define HYP_VIDEO_MSG_LOW( msg_fmt,...)   \
                MM_MSG_PRIO_EX(MM_VIDEO_TASK, MM_PRIO_MEDIUM, msg_fmt, ## __VA_ARGS__ )
#define HYP_VIDEO_MSG_HIGH( msg_fmt,...)   \
                MM_MSG_PRIO_EX(MM_VIDEO_TASK, MM_PRIO_HIGH, msg_fmt, ## __VA_ARGS__ )
#define HYP_VIDEO_MSG_ERROR( msg_fmt,...)   \
                MM_MSG_PRIO_EX(MM_VIDEO_TASK, MM_PRIO_ERROR, msg_fmt, ## __VA_ARGS__ )

#elif defined(_ANDROID_)    // LA
#ifdef LOG_TAG
#undef LOG_TAG
#endif

#ifdef LOG_NDEBUG
#undef LOG_NDEBUG
#endif

#define LOG_NDEBUG 0

#define LOG_TAG "HYP_VIDEO"
#include "android/log.h"

#define HYP_VIDEO_MSG_INFO(fmt, args...)  ({if(debug_level & HYP_PRIO_INFO) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "[%d:%d]:[%s:%s]" fmt "",  \
                                            getpid(), gettid(), __FILENAME__, __FUNCTION__, ##args);})
#define HYP_VIDEO_MSG_LOW(fmt, args...)   ({if(debug_level & HYP_PRIO_LOW) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "[%d:%d]:[%s:%s]" fmt "",    \
                                            getpid(), gettid(), __FILENAME__, __FUNCTION__, ##args);})
#define HYP_VIDEO_MSG_HIGH(fmt, args...)  ({if(debug_level & HYP_PRIO_HIGH) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "[%d:%d]:[%s:%s]" fmt "",   \
                                            getpid(), gettid(), __FILENAME__, __FUNCTION__, ##args);})
#define HYP_VIDEO_MSG_ERROR(fmt, args...) ({if(debug_level & HYP_PRIO_ERROR) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "[%d:%d]:[%s:%s]" fmt "", \
                                            getpid(), gettid(), __FILENAME__, __FUNCTION__, ##args);})
#else    // Other cases
#define HYP_VIDEO_MSG_INFO(fmt, args...) ({if(debug_level & HYP_PRIO_INFO) printf(fmt,##args);})
#define HYP_VIDEO_MSG_LOW(fmt, args...)  ({if(debug_level & HYP_PRIO_LOW) printf(fmt,##args);})
#define HYP_VIDEO_MSG_HIGH(fmt, args...) ({if(debug_level & HYP_PRIO_HIGH) printf(fmt,##args);})
#define HYP_VIDEO_MSG_ERROR(fmt, args...) ({if(debug_level & HYP_PRIO_ERROR) printf(fmt,##args);})

#endif

#endif

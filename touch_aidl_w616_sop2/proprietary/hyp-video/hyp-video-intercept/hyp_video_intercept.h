/*========================================================================

*//** @file hyp_video_intercept.h

@par DESCRIPTION:
Linux video decoder hypervisor front-end implementation header

@par FILE SERVICES:

@par EXTERNALIZED FUNCTIONS:
See below.

Copyright (c) 2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/
/*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---     -------------------------------------------------------
08/21/20   sh      Add initial version of the file
========================================================================== */

/* =======================================================================

INCLUDE FILES FOR MODULE

========================================================================== */
#ifndef __HYP_VIDEO_INTERCEPT_H__
#define __HYP_VIDEO_INTERCEPT_H__

#include <poll.h>

enum {
    HYP_PRIO_ERROR = 0x1,
    HYP_PRIO_HIGH  = 0x2,
    HYP_PRIO_LOW   = 0x4,
    HYP_PRIO_INFO  = 0x8
};

#if defined(_ANDROID_)

#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#ifdef LOG_NDEBUG
#undef LOG_NDEBUG
#endif

#define LOG_NDEBUG 0

#define LOG_HYP_TAG "HYP_VIDEO_INTERCEPT"
#include "android/log.h"

#define HYP_VIDEO_MSG_INFO(fmt, args...)  ({if(debug_level & HYP_PRIO_INFO) \
                                               __android_log_print(ANDROID_LOG_DEBUG, LOG_HYP_TAG, "[%d:%d]:[%s:%s]" fmt "",  \
                                               getpid(), gettid(), __FILENAME__, __FUNCTION__, ##args);})
#define HYP_VIDEO_MSG_LOW(fmt, args...)   ({if(debug_level & HYP_PRIO_LOW) \
                                               __android_log_print(ANDROID_LOG_VERBOSE, LOG_HYP_TAG, "[%d:%d]:[%s:%s]" fmt "", \
                                               getpid(), gettid(), __FILENAME__, __FUNCTION__, ##args);})
#define HYP_VIDEO_MSG_HIGH(fmt, args...)  ({if(debug_level & HYP_PRIO_HIGH) \
                                               __android_log_print(ANDROID_LOG_INFO, LOG_HYP_TAG, "[%d:%d]:[%s:%s]" fmt "",   \
                                               getpid(), gettid(), __FILENAME__, __FUNCTION__, ##args);})
#define HYP_VIDEO_MSG_ERROR(fmt, args...) ({if(debug_level & HYP_PRIO_ERROR) \
                                               __android_log_print(ANDROID_LOG_ERROR, LOG_HYP_TAG, "[%d:%d]:[%s:%s]" fmt "", \
                                               getpid(), gettid(), __FILENAME__, __FUNCTION__, ##args);})
#endif

int hyp_video_open(const char *str, int flag);
int hyp_video_ioctl(int fd, int cmd, void *data);
int hyp_video_poll(struct pollfd *fds, nfds_t nfds, int timeout);
int hyp_video_close(int fd);

#endif

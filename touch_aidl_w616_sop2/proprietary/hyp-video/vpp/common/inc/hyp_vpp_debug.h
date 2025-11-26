/*========================================================================

*//** @file hyp_vpp_debug.h
Hypervisor VPP debug header

Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */

#ifndef __HYP_VPP_DEBUG_H__
#define __HYP_VPP_DEBUG_H__

#include <unistd.h>
#include <string.h>
#include <inttypes.h>

extern int vpp_debug_level;

enum {
    HYP_PRIO_ERROR = 0x1,
    HYP_PRIO_WARN  = 0x2,
    HYP_PRIO_HIGH  = 0x4,
    HYP_PRIO_LOW   = 0x8,
    HYP_PRIO_INFO  = 0x10,
    HYP_PRIO_PERF  = 0x20
};

#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#ifdef _LINUX_    // LV
#include <syslog.h>
#include <sys/syscall.h>

#define gettid() syscall(SYS_gettid)
#define getpid() syscall(SYS_getpid)
#define DEBUG_PRINT_CTL(debug_level, sys_level, fmt, args...)               \
      do {                                                                  \
           if (vpp_debug_level & debug_level)                               \
             syslog(sys_level, "[%ld:%ld]:[%s:%s:%d] " fmt " \n", getpid(), \
               gettid(), __FILENAME__, __FUNCTION__, __LINE__, ##args);     \
      } while(0)

#define HYP_VPP_MSG_PERF(msg_fmt,args...)   \
                DEBUG_PRINT_CTL(HYP_PRIO_PERF, LOG_DEBUG, msg_fmt, ##args )
#define HYP_VPP_MSG_INFO(msg_fmt,args...)   \
                DEBUG_PRINT_CTL(HYP_PRIO_INFO, LOG_INFO, msg_fmt, ##args )
#define HYP_VPP_MSG_LOW(msg_fmt,args...)   \
                DEBUG_PRINT_CTL(HYP_PRIO_LOW, LOG_INFO, msg_fmt, ##args )
#define HYP_VPP_MSG_HIGH(msg_fmt,args...)   \
                DEBUG_PRINT_CTL(HYP_PRIO_HIGH, LOG_NOTICE, msg_fmt, ##args )
#define HYP_VPP_MSG_WARN(msg_fmt,args...)   \
                DEBUG_PRINT_CTL(HYP_PRIO_WARN, LOG_WARNING, msg_fmt, ##args )
#define HYP_VPP_MSG_ERROR(msg_fmt,args...)   \
                DEBUG_PRINT_CTL(HYP_PRIO_ERROR, LOG_ERR, msg_fmt, ##args )

#elif defined(_ANDROID_)    // LA
#ifdef LOG_TAG
#undef LOG_TAG
#endif

#ifdef LOG_NDEBUG
#undef LOG_NDEBUG
#endif

#define LOG_NDEBUG 0

#define LOG_TAG "HYP_VPP"
#include "android/log.h"

#define HYP_VPP_MSG_PERF(fmt, args...)  ({if(vpp_debug_level & HYP_PRIO_PERF) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "[%d:%d]:[%s:%s] " fmt "",  \
                                            getpid(), gettid(), __FILENAME__, __FUNCTION__, ##args);})
#define HYP_VPP_MSG_INFO(fmt, args...)  ({if(vpp_debug_level & HYP_PRIO_INFO) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "[%d:%d]:[%s:%s] " fmt "",  \
                                            getpid(), gettid(), __FILENAME__, __FUNCTION__, ##args);})
#define HYP_VPP_MSG_LOW(fmt, args...)   ({if(vpp_debug_level & HYP_PRIO_LOW) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "[%d:%d]:[%s:%s] " fmt "", \
                                            getpid(), gettid(), __FILENAME__, __FUNCTION__, ##args);})
#define HYP_VPP_MSG_HIGH(fmt, args...)  ({if(vpp_debug_level & HYP_PRIO_HIGH) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "[%d:%d]:[%s:%s] " fmt "",   \
                                            getpid(), gettid(), __FILENAME__, __FUNCTION__, ##args);})
#define HYP_VPP_MSG_WARN(fmt, args...)  ({if(vpp_debug_level & HYP_PRIO_WARN) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, "[%d:%d]:[%s:%s] " fmt "",   \
                                            getpid(), gettid(), __FILENAME__, __FUNCTION__, ##args);})
#define HYP_VPP_MSG_ERROR(fmt, args...) ({if(vpp_debug_level & HYP_PRIO_ERROR) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "[%d:%d]:[%s:%s] " fmt "", \
                                            getpid(), gettid(), __FILENAME__, __FUNCTION__, ##args);})
#else    // Other cases
#define HYP_VPP_MSG_PERF(fmt, args...)  ({if(vpp_debug_level & HYP_PRIO_PERF) printf(fmt,##args);})
#define HYP_VPP_MSG_INFO(fmt, args...)  ({if(vpp_debug_level & HYP_PRIO_INFO) printf(fmt,##args);})
#define HYP_VPP_MSG_LOW(fmt, args...)   ({if(vpp_debug_level & HYP_PRIO_LOW) printf(fmt,##args);})
#define HYP_VPP_MSG_HIGH(fmt, args...)  ({if(vpp_debug_level & HYP_PRIO_HIGH) printf(fmt,##args);})
#define HYP_VPP_MSG_WARN(fmt, args...)  ({if(vpp_debug_level & HYP_PRIO_WARN) printf(fmt,##args);})
#define HYP_VPP_MSG_ERROR(fmt, args...) ({if(vpp_debug_level & HYP_PRIO_ERROR) printf(fmt,##args);})

#endif

// dump VPP control
#define LOG_CADE(pcade)      HYP_VPP_MSG_INFO("->cade:{mode=%d, cade_level=%u, contrast=%d, saturation=%d}", \
                                (pcade)->mode, (pcade)->cade_level, (pcade)->contrast, (pcade)->saturation);
#define LOG_DI(pdi)          HYP_VPP_MSG_INFO("->di:{mode=%d}", (pdi)->mode);
#define LOG_TNR(ptnr)        HYP_VPP_MSG_INFO("->tnr:{mode=%d, level=%u}", (ptnr)->mode, (ptnr)->level);
#define LOG_CNR(pcnr)        HYP_VPP_MSG_INFO("->cnr:{mode=%d, level=%u}", (pcnr)->mode, (pcnr)->level);
#define LOG_AIE(paie)        HYP_VPP_MSG_INFO("->aie:{mode=%d, hue_mode=%d, cade_level=%u, ltm_level=%u, sat_gain=%u, sat_off=%u, ace_str=%u, ace_bl=%u, ace_bh=%u}", \
                                (paie)->mode, (paie)->hue_mode, (paie)->cade_level, (paie)->ltm_level, \
                                (paie)->ltm_sat_gain, (paie)->ltm_sat_offset, (paie)->ltm_ace_str, \
                                (paie)->ltm_ace_bright_l, (paie)->ltm_ace_bright_h);
#define LOG_FRC(pfrc)        do { \
                                if ((pfrc)->segments) \
                                   for (uint32_t i = 0; i < (pfrc)->num_segments; i++) \
                                      HYP_VPP_MSG_INFO("->frc[%u]:{mode=%d, level=%d, interp=%d, ts_start=%" PRIu64 \
                                         ", frame_copy_on_fallback=%u, frame_copy_input=%u, smart_fallback=%u}", \
                                         i, (pfrc)->segments[i].mode, (pfrc)->segments[i].level, (pfrc)->segments[i].interp, \
                                         (pfrc)->segments[i].ts_start, (pfrc)->segments[i].frame_copy_on_fallback, \
                                         (pfrc)->segments[i].frame_copy_input, (pfrc)->segments[i].smart_fallback); \
                                } while (0)
#define LOG_EAR(pear)        HYP_VPP_MSG_INFO("->ear:{mode=%d}", (pear)->mode);
#define LOG_QBR(pqbr)        HYP_VPP_MSG_INFO("->qbr:{mode=%d}", (pqbr)->mode);
#define LOG_AIS(pais)        HYP_VPP_MSG_INFO("->ais:{mode=%d, roi:(enable=%u, x=%u, y=%u, width=%u, height=%u), classification=%u}", \
                                (pais)->mode, (pais)->roi.enable, (pais)->roi.x_start, (pais)->roi.y_start, \
                                (pais)->roi.width, (pais)->roi.height, (pais)->classification);


#endif /* __HYP_VPP_DEBUG_H__ */

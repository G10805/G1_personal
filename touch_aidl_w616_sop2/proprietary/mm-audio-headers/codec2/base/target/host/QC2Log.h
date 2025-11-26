/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_LOG_H_
#define _QC2AUDIO_LOG_H_

#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>

#include <sys/syscall.h>
#ifndef gettid
#define gettid() syscall(SYS_gettid)
#endif

namespace qc2audio {

extern uint32_t gC2LogLevel;
void updateLogLevel();

/*
 * Change logging-level at runtime by setting env var DEBUG_LEVEL
 *
 * DEBUG_LEVEL     QLOGV          QLOGD        QLOGI
 * ---------------------------------------------------
 * 0x0            silent          silent      silent
 * 0x1            silent          silent      printed
 * 0x3            silent          printed     printed
 * 0x7            printed         printed     printed
 *
 * QLOGW/E are printed always
 */


#ifndef LOG_TAG
#define LOG_TAG ""
#endif

#define _QLOG(enable, VTAG, format, args...)                                                    \
    if (enable) {                                                                               \
        struct timeval tv;                                                                      \
        gettimeofday(&tv, NULL);                                                                \
        printf("%02ld:%02ld.%03ld %d %ld" VTAG LOG_TAG ": " format "\n",                        \
                (tv.tv_sec/60)%60, (tv.tv_sec)%60, tv.tv_usec/1000, getpid(), gettid(), ##args);\
    }

#define QLOGV(format, args...) _QLOG((gC2LogLevel & 0x4), " V ", format, ##args)
#define QLOGD(format, args...) _QLOG((gC2LogLevel & 0x2), " D ", format, ##args)
#define QLOGI(format, args...) _QLOG((gC2LogLevel & 0x1), " I ", format, ##args)
#define QLOGW(format, args...) _QLOG(true,                " W ", format, ##args)
#define QLOGE(format, args...) _QLOG(true,                " E ", format, ##args)

}; // namespace qc2audio

#endif // _QC2AUDIO_LOG_H_



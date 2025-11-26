/*
 **************************************************************************************************
 * Copyright (c) 2018,2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_LOG_H_
#define _QC2_LOG_H_

#include <cstdint>
#include <sys/types.h>

#include <unistd.h>
#include <sys/time.h>

#include <sys/syscall.h>
#include <syslog.h>
#ifndef gettid
#define gettid() syscall(SYS_gettid)
#endif

namespace qc2 {
extern uint32_t gC2LogBuffers;
extern uint32_t gC2FileLogLevel;
extern uint32_t gC2ProfileBypass;

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

#define _QLOG(level, enable, VTAG, format, args...)                                             \
    if (enable) {                                                                               \
        struct timeval tv;                                                                      \
        gettimeofday(&tv, NULL);                                                                \
        syslog(level, "%02ld:%02ld.%03ld %d %ld" VTAG LOG_TAG ": " format "\n",                 \
                (tv.tv_sec/60)%60, (tv.tv_sec)%60, tv.tv_usec/1000, getpid(), gettid(), ##args);\
    }

#define QLOGV(format, args...) _QLOG(LOG_INFO, (gC2LogLevel & 0x4), " V ", format, ##args)
#define QLOGD(format, args...) _QLOG(LOG_INFO, (gC2LogLevel & 0x2), " D ", format, ##args)
#define QLOGI(format, args...) _QLOG(LOG_INFO, (gC2LogLevel & 0x1), " I ", format, ##args)
#define QLOGW(format, args...) _QLOG(LOG_WARNING, true,                " W ", format, ##args)
#define QLOGE(format, args...) _QLOG(LOG_ERR, true,                " E ", format, ##args)

enum QC2LogBufferLevel : uint32_t {
    QC2_LOGBUF_DEC_IN          = 0x00000001,    ///< log decoder input (bitstream)
    QC2_LOGBUF_DEC_OUT         = 0x00000002,    ///< log decoder output (yuv)
    QC2_LOGBUF_DEC_POST_CONV   = 0x00000004,    ///< log decoder output after conversion (yuv)
    QC2_LOGBUF_DEC_OUT_MISR    = 0x00000008,    ///< log decoder output MISR (text)

    QC2_LOGBUF_ENC_IN          = 0x00000010,    ///< log encoder input (yuv)
    QC2_LOGBUF_ENC_OUT         = 0x00000020,    ///< log encoder output (bitstream)
    QC2_LOGBUF_ENC_PRE_CONV    = 0x00000040,    ///< log encoder input before conversion (rgb/yuv)

    QC2_LOGBUF_FORCE_LINEAR    = 0x00001000,    ///< force log compressed buffers as uncompressed
};

#define QTRACEV_ENABLED
#define QTRACED_ENABLED
#define QTRACEV(msg)
#define QTRACED(msg)
#define QTRACEV_INT64(instName, valName, val)
#define QTRACED_INT64(instName, valName, val)
#define QTRACEV_ASYNC_BEGIN(msg, cookie)
#define QTRACEV_ASYNC_END(msg, cookie)
#define QTRACED_ASYNC_BEGIN(msg, cookie)
#define QTRACED_ASYNC_END(msg, cookie)

enum QC2LogLevel : uint32_t {
    QC2_MSGLEVEL_DEBUG   = 0x01,      ///< debug logs
    QC2_MSGLEVEL_VERBOSE = 0x02,      ///< very detailed logs
    QC2_MSGLEVEL_STATISTICS = 0x04,   ///< statistics
    QC2_MSGLEVEL_BUFFER_FLOW = 0x08,  ///< track buffer flow
};

enum QC2LogModuleId : uint32_t {
    QC2_DOMAIN_GENERAL = 0,
    QC2_DOMAIN_CORE = 1,
    QC2_DOMAIN_PIPELINE = 2,
    QC2_DOMAIN_V4L2 = 3,
    QC2_DOMAIN_VPP = 4,
};

#define QC2_IS_ALL_DOMAINS_ENABLED(logDomain) (!(gC2LogLevel & 0xff000000))

#define QC2_IS_LOGID_ENABLED_FOR_DOMAIN(logDomain) \
     ((gC2LogLevel & 0xff000000) ? ((0x1 << logDomain) & ((gC2LogLevel & 0xff000000) >> 24)) : 0)

#define QC2_IS_LOG_ID_ENABLED(logDomain) \
    (QC2_IS_ALL_DOMAINS_ENABLED(logDomain) | QC2_IS_LOGID_ENABLED_FOR_DOMAIN(logDomain))
#define QC2GETLOGLEVEL(logDomain) (QC2_IS_LOG_ID_ENABLED(logDomain) ? (gC2LogLevel & 0xff) : 0)

#define QC2GETPVTLOGLEVEL(logDomain) \
    (QC2_IS_LOGID_ENABLED_FOR_DOMAIN(logDomain) ? \
        ((gC2LogLevel >> ((1 + logDomain) * 4)) & 0xf) : 0)


#define QLOGP(logBit, format, args...) QLOGD(format, ##args)
#define QLOGB(format, args...) QLOGD(format, ##args)
#define QLOGS(format, args...) QLOGD(format, ##args)


// Check if buffer tracking is enabled. If buffer tracking is enabled, then log
// the movement of QC2Buffer across code segments. This must not be enabled in
// production release.
#define QLOG_IS_BUFFER_TRACKING_ENABLED (QC2GETLOGLEVEL(QC2_LOG_DOMAIN) & QC2_MSGLEVEL_BUFFER_FLOW)

// Check if statistics logging is enabled. This must not be enabled in
// production release.
#define QLOG_IS_STATS_ENABLED (QC2GETLOGLEVEL(QC2_LOG_DOMAIN) & QC2_MSGLEVEL_STATISTICS)

// Qualify instance-name to the messages logged from an instance (of anything) that
// contains an instance variable 'std::string mInstName'
#define QLOGB_INST(format, args...) QLOGB("[%s] " format, mInstName.c_str(), ##args)
#define QLOGS_INST(format, args...) QLOGS("[%s] " format, mInstName.c_str(), ##args)
#define QLOGV_INST(format, args...) QLOGV("[%s] " format, mInstName.c_str(), ##args)
#define QLOGD_INST(format, args...) QLOGD("[%s] " format, mInstName.c_str(), ##args)
#define QLOGI_INST(format, args...) QLOGI("[%s] " format, mInstName.c_str(), ##args)
#define QLOGW_INST(format, args...) QLOGW("[%s] " format, mInstName.c_str(), ##args)
#define QLOGE_INST(format, args...) QLOGE("[%s] " format, mInstName.c_str(), ##args)
#define QLOGE_PIPE_INST(format, args...) QLOGE("[%s] " format, mPipelineId2CodecNameMap[nIdentifier].c_str(), ##args)
#define QLOGI_PIPE_INST(format, args...) QLOGI("[%s] " format, mPipelineId2CodecNameMap[nIdentifier].c_str(), ##args)
#define QLOGV_PIPE_INST(format, args...) QLOGV("[%s] " format, mPipelineId2CodecNameMap[nIdentifier].c_str(), ##args)

//-------------------------------------------------------------------------------------------------
// D I A G N O S T I C   L O G G I N G
//-------------------------------------------------------------------------------------------------

/// @addtogroup diag Diagnostic Logs
/// @{

enum QC2FileLogLevel : uint32_t {
    QC2_FILE_LOG_INFO          = 0x01,    ///< log informational diagnostic messages to file
    QC2_FILE_LOG_DEBUG         = 0x02,    ///< log debug messages to file
};

/// @}

//-------------------------------------------------------------------------------------------------
// P R O F I L I N G
//-------------------------------------------------------------------------------------------------

/// @addtogroup profile Profiling
/// @{

/**
 *
 * @brief <b> "vendor.qc2.profile.bypass" </b> skips input processing
 *
 * <pre>
 *
 * <b> Eg: adb shell setprop vendor.qc2.profile.bypass 0x1 </b>
 *
 * This will skips frame processing and reverts input back to framework
 * in queue_nb itself.
 *
 * Helps to triage encoder/decoder framedrop issues.
 *
 * </pre>
 */
enum QC2ProfileLevel : uint32_t {
    QC2_PROFILE_BYPASS          = 0x01,    ///< Profile Encoder
};

static inline bool ShouldBypassWork() {
    return !!(gC2ProfileBypass & QC2_PROFILE_BYPASS);
}

};  // namespace qc2

#endif  // _QC2_LOG_H_



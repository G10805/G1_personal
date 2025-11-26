/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_LOG_H_
#define _QC2_LOG_H_

#include <utils/Log.h>
#include <utils/Trace.h>

namespace qc2 {

/// @addtogroup debug Debugging
/// @{

/// @private
void updateLogLevel();

//-------------------------------------------------------------------------------------------------
// L O G     M E S S A G E S
//-------------------------------------------------------------------------------------------------

/// @addtogroup logs Log Messages
/// @{

/**
 *
 * @brief <b> "vendor.qc2.log.msg" </b> controls logging debug messages
 *
 * <pre>
 *
 * <b> Eg: adb shell setprop vendor.qc2.log.msg 0x1 </b>
 *
 * 0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1
 *|                      |    vpp    |    v4l2   | pipeline  |   core     |       common         |
 *|      domain          |   private |  private  | private   |  private   |       logmask        |
 *|                      |   logmask |  logmask  | logmask   |  logmask   |                      |
 *
 * logdomains:
 *  ALL: 0xff (or 0 for backward compatibility)
 *  GENERAL  0x01
 *  CORE     0x02
 *  PIPELINE 0x04
 *  V4L2     0x08
 *  VPP      0x10
 *  VIDTXR   0x20
 *
 * common logmask:
 *  DEBUG       0x1
 *  VERBOSE     0x2
 *  STATISTICS  0x4
 *  BUFFER FLOW 0x8
 *
 *  Above (logdomain and common logmask) are suffcient for most use cases
 *
 *  Examples:
 *  ----------------------------------------------------
 *      LogDomain         LogLevel        LogMask
 *  ----------------------------------------------------
 *  (1) all             debug|verbose       0x00000003
 *  (2) v4l2            verbose             0x08000002
 *  (3) pipeline        statistics          0x04000004
 *  (4) pipeline|vpp    bufferflow          0x14000008
 *
 *  Private logmasks
 *  In addition, it is possible to have log masks private to the owner of
 *  a domainid. Eg, a team may want to add logs for debugging associated with
 *  a certain submodule. Private log masks faciliate this.
 *
 *  QLOGP(QC2_DOMAIN_PIPELINE, 0x1, "...");
 *  Logmask = 0x04000100
 *

 * </pre>
 */
enum QC2LogLevel : uint32_t {
    QC2_MSGLEVEL_DEBUG   = 0x01,      ///< debug logs
    QC2_MSGLEVEL_VERBOSE = 0x02,      ///< very detailed logs
    QC2_MSGLEVEL_STATISTICS = 0x04,   ///< statistics
    QC2_MSGLEVEL_BUFFER_FLOW = 0x08,  ///< track buffer flow
};

/// @cond HIDE
static const char *kDebugLogsLevelProperty = "vendor.qc2.log.msg";
extern uint32_t gC2LogLevel;

enum QC2LogModuleId : uint32_t {
    QC2_DOMAIN_GENERAL = 0,
    QC2_DOMAIN_CORE = 1,
    QC2_DOMAIN_PIPELINE = 2,
    QC2_DOMAIN_V4L2 = 3,
    QC2_DOMAIN_VPP = 4,
    QC2_DOMAIN_VIDTXR = 5,
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

// Default log domain
#ifndef QC2_LOG_DOMAIN
#define QC2_LOG_DOMAIN QC2_DOMAIN_GENERAL
#endif

// Check if buffer tracking is enabled. If buffer tracking is enabled, then log
// the movement of QC2Buffer across code segments. This must not be enabled in
// production release.
#define QLOG_IS_BUFFER_TRACKING_ENABLED (QC2GETLOGLEVEL(QC2_LOG_DOMAIN) & QC2_MSGLEVEL_BUFFER_FLOW)

// Check if statistics logging is enabled. This must not be enabled in
// production release.
#define QLOG_IS_STATS_ENABLED (QC2GETLOGLEVEL(QC2_LOG_DOMAIN) & QC2_MSGLEVEL_STATISTICS)

#define QLOGP(logBit, format, args...) \
    ALOGI_IF((QC2GETPVTLOGLEVEL(QC2_LOG_DOMAIN)&logBit), format, ##args)
#define QLOGB(format, args...) \
    ALOGD_IF((QC2GETLOGLEVEL(QC2_LOG_DOMAIN) & QC2_MSGLEVEL_BUFFER_FLOW), format, ##args)
#define QLOGS(format, args...) \
    ALOGD_IF((QC2GETLOGLEVEL(QC2_LOG_DOMAIN) & QC2_MSGLEVEL_STATISTICS), format, ##args)
#define QLOGV(format, args...) \
    ALOGD_IF((QC2GETLOGLEVEL(QC2_LOG_DOMAIN) & QC2_MSGLEVEL_VERBOSE), format, ##args)
#define QLOGD(format, args...) \
    ALOGD_IF((QC2GETLOGLEVEL(QC2_LOG_DOMAIN) & QC2_MSGLEVEL_DEBUG), format, ##args)
#define QLOGI(format, args...) ALOGI(format, ##args)
#define QLOGW(format, args...) ALOGW(format, ##args)
#define QLOGE(format, args...) ALOGE(format, ##args)

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

/// @endcond HIDE

/// @}

//-------------------------------------------------------------------------------------------------
// B U F F E R   L O G G I N G
//-------------------------------------------------------------------------------------------------

/// @addtogroup buflogs Buffer Logs
/// @{

/**
 * @brief <b> "vendor.qc2.log.buffers" </b> property controls logging buffers to file             \n
 *
 * <pre>
 * <b> A combination of following masks can be used to log decoder buffers: </b>
 *   0x01  - dump input buffer (queued to the codec)
 *   0x02  - dump output buffer (received from the codec)
 *   0x04  - dump color-converted output for decoder component (if present)
 * <b> A combination of following masks for logging encoder buffers: </b>
 *   0x1x  - dump input buffer (queued to the codec)
 *   0x2x  - dump output buffer (received from the codec)
 *   0x4x  - dump pre-color-converted input to encoder component (if present)
 * <b> Following bit forces logging of compressed format as linear globally: </b>
 *   0x1xxx - force de-compression of UBWC buffers while logging. NOTE: this does inline-conversion
 *
 * <b>
 *   Eg:
 *     adb shell setprop vendor.qc2.log.buffers 0x01  => log decoder input
 *     adb shell setprop vendor.qc2.log.buffers 0x11  => log decoder and encoder input
 *     adb shell setprop vendor.qc2.log.buffers 0x77  => log all types of buffers
 * </b>
 *
 * <b> Dump file will be saved in /data/vendor/media/qc2/ </b>
 *     @note The location '/data/vendor/media/qc2/' must exist on the device before enabling logging
 * <b> File name has following format </b>
 *
 *           YYMMDD_HHMMSS_<id>_<prefix>_<wxh>_<format>_<seq-count>.<extn>
 *                 ^         ^      ^     ^      ^          ^         ^
 *                 |         |      |     |      |          |         |
 *             Date/time     |      |     |      |          |       extn (.yuv/.rgb/.h264/.ivf...)
 *                           |      |     |      |          |
 *                component-id      |     |      |          frame-number at sequence start
 *                                  |     |      |
 *                        buffer-type     |      color format (graphic-only)
 *           (input/output/converted)     |
 *                                        width x height (graphic-only)
 *
 * @note For Graphic buffers, when resolution/format changes, logging will switch over to a new
 *       file and the <seq-count> will be incremented.
 * </pre>
 */
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

/// @cond HIDE
static const char *kDebugLogBuffersProperty = "vendor.qc2.log.buffers";
extern uint32_t gC2LogBuffers;
/// @endcond HIDE

/// @}

//-------------------------------------------------------------------------------------------------
// D I A G N O S T I C   L O G G I N G
//-------------------------------------------------------------------------------------------------

/// @addtogroup diag Diagnostic Logs
/// @{

/**
 * @brief <b> "vendor.qc2.log.diag" </b> property controls logging diagnostic messages to file  \n
 *
 * <pre>
 * <b> Following verbosity levels can be enabled </b>
 *   0x01  - Informational logs
 *   0x02  - Debug logs
 *
 *   Eg:
 *     adb shell setprop vendor.qc2.log.diag 0x03  => enable info and debug diagnostic logs
 *
 * <b> File name has following format </b>
 *
 *             pid_nnnn_[codec][D|E].txt
 *              ^    ^     ^     ^
 *              |    |     |     |
 *     c2 hal PID    |     |    [D]ecoder or [E]ncoder
 *         instance ID     |
 *                 codec (avc, hevc, avc_low_latency..)
 *
 * </pre>
 */
enum QC2FileLogLevel : uint32_t {
    QC2_FILE_LOG_INFO          = 0x01,    ///< log informational diagnostic messages to file
    QC2_FILE_LOG_DEBUG         = 0x02,    ///< log debug messages to file
};

/// @cond HIDE
static const char *kDebugFileLogProperty = "vendor.qc2.log.diag";
extern uint32_t gC2FileLogLevel;
/// @endcond HIDE

/// @}


//-------------------------------------------------------------------------------------------------
// T R A C I N G
//-------------------------------------------------------------------------------------------------

/// @addtogroup trace Tracing
/// @{

/**
 *
 * @brief <b> "vendor.qc2.log.trace" </b> controls logging debug messages
 *
 * <pre>
 *
 * <b> Eg: adb shell setprop vendor.qc2.log.trace 0x11 </b>
 *
 *    LEVELS
 *    .....................................................
 *      level       QTRACEV_{DOMAIN}       QTRACED_{DOMAIN}
 *    .....................................................
 *      0xX0         silent                silent (default)
 *      0xX1         silent                logged
 *      0xX3         logged                logged
 *    .....................................................
 *
 *    DOMAINS
 *
 * </pre>
 */
enum QC2TraceEnable : uint32_t {
    QC2_TRACE_LEVEL_DEBUG   = 0x01,     ///< high-level debug traces
    QC2_TRACE_LEVEL_VERBOSE = 0x02,     ///< very detailed traces

    QC2_TRACE_DOMAIN_CORE   = 0x10,     ///< enable traces at component-level
    QC2_TRACE_DOMAIN_CODEC  = 0x20,     ///< enable traces at codec-level
    QC2_TRACE_DOMAIN_EVENT  = 0x40,     ///< trace events/event-Queue
};

/// @cond HIDE
static const char *kDebugTraceLevelProperty = "vendor.qc2.log.trace";
extern uint32_t gC2TraceLevel;

class QC2AutoTracer {
 public:
    explicit QC2AutoTracer(uint32_t prio, const char *msg)
        : mPrio(prio) {
        if (gC2TraceLevel & mPrio) {
            atrace_begin(ATRACE_TAG_VIDEO, msg);
        }
    }
    ~QC2AutoTracer() {
        if (gC2TraceLevel & mPrio) {
            atrace_end(ATRACE_TAG_VIDEO);
        }
    }
 private:
    uint32_t mPrio;
};

#define QTRACEV_ENABLED \
    ((*(&gC2TraceLevel) & QC2_TRACE_LEVEL_VERBOSE) && (*(&gC2TraceLevel) & QC2_TRACE_DOMAIN))
#define QTRACED_ENABLED \
    ((*(&gC2TraceLevel) & QC2_TRACE_LEVEL_DEBUG) && (*(&gC2TraceLevel) & QC2_TRACE_DOMAIN))

#define QTRACEV(msg) QC2AutoTracer _tracer(QC2_TRACE_LEVEL_VERBOSE | QC2_TRACE_DOMAIN, msg);

#define QTRACED(msg) QC2AutoTracer _tracer(QC2_TRACE_LEVEL_DEBUG | QC2_TRACE_DOMAIN, msg);

#define QTRACEV_INT64(instName, valName, val)                                               \
    if (QTRACEV_ENABLED) {                                                                  \
        char msg[32] = {0};                                                                 \
        snprintf(msg, sizeof(msg), "%s-%s", instName, valName);                             \
        atrace_int64(ATRACE_TAG_VIDEO, msg, val);                                           \
    }                                                                                       \

#define QTRACED_INT64(instName, valName, val)                                               \
    if (QTRACED_ENABLED) {                                                                  \
        char msg[32] = {0};                                                                 \
        snprintf(msg, sizeof(msg), "%s-%s", instName, valName);                             \
        atrace_int64(ATRACE_TAG_VIDEO, msg, val);                                           \
    }                                                                                       \

#define QTRACEV_ASYNC_BEGIN(msg, cookie)                                                    \
    if (QTRACEV_ENABLED) {                                                                  \
        atrace_async_begin(ATRACE_TAG_VIDEO, msg, cookie);                                  \
    }                                                                                       \

#define QTRACEV_ASYNC_END(msg, cookie)                                                      \
    if (QTRACEV_ENABLED) {                                                                  \
        atrace_async_end(ATRACE_TAG_VIDEO, msg, cookie);                                    \
    }                                                                                       \

#define QTRACED_ASYNC_BEGIN(msg, cookie)                                                    \
    if (QTRACED_ENABLED) {                                                                  \
        atrace_async_begin(ATRACE_TAG_VIDEO, msg, cookie);                                  \
    }                                                                                       \

#define QTRACED_ASYNC_END(msg, cookie)                                                      \
    if (QTRACED_ENABLED) {                                                                  \
        atrace_async_end(ATRACE_TAG_VIDEO, msg, cookie);                                    \
    }                                                                                       \
/// @endcond HIDE


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

/// @cond HIDE
static const char *kDebugProfileBypassProperty = "vendor.qc2.profile.bypass";
extern uint32_t gC2ProfileBypass;
/// @endcond HIDE

static inline bool ShouldBypassWork() {
    return !!(gC2ProfileBypass & QC2_PROFILE_BYPASS);
}

/// @}


/// @}

};  // namespace qc2

#endif  // _QC2_LOG_H_

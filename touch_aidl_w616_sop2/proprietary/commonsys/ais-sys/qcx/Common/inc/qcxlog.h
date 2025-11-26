#ifndef QCX_LOG_H
#define QCX_LOG_H

/* ===========================================================================
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @file qcxlog.h
 * @brief Logging API to be used by camera driver.
 *
=========================================================================== */

#ifdef __QNXNTO__
    #ifdef LINUX_LRH
    #include <stdio.h>
    #include <string.h>
    #include <stdarg.h>
    #include <syslog.h>
    #else
    #include <sys/slog2.h>
    #include <sys/trace.h>
    #include <stdio.h>
    #endif
#else
#include <stdio.h>
#include <string.h>
#include <errno.h>
#endif

#include "qcxstatus.h"


#ifdef __cplusplus
extern "C" {
#endif


/**================================================================================================
                      DEFINITIONS AND DECLARATIONS
================================================================================================**/

/**================================================================================================
** Constant and Macros
================================================================================================**/

/**================================================================================================
** Declarations
================================================================================================**/


/**================================================================================================
** Typedefs
================================================================================================**/

/*************************************************************************************************
@brief
List of logging components withn the safe camera driver

WARNING: BEFORE EDITING THE ENUM READ THIS

ANY MODIFICATIONS IN THIS ENUM MUST BE REFLECTED IN THE g_OSALComponentStringLUT and
g_OSALComponentLogLevels in qcxlog.c


**************************************************************************************************/
typedef enum
{
    QCX_LOG_MODULE_CLIENT_MGR = 0,
    QCX_LOG_MODULE_CONFIG_MGR,
    QCX_LOG_MODULE_STREAM_MGR,
    QCX_LOG_MODULE_SENSOR_MGR,
    QCX_LOG_MODULE_DEV_MGR,
    QCX_LOG_MODULE_MSG_DPR,
    QCX_LOG_MODULE_ISP,
    QCX_LOG_MODULE_IFE,
    QCX_LOG_MODULE_CSIPHY,
    QCX_LOG_MODULE_SENSOR,
    QCX_LOG_MODULE_CDM,
    QCX_LOG_MODULE_EXTHW,
    QCX_LOG_MODULE_PLATFORM,
    QCX_LOG_MODULE_DIAG_MGR,
    QCX_LOG_MODULE_OSAL,
    QCX_LOG_MODULE_HTM,
    QCX_LOG_MODULE_SERVICE_INIT,
    QCX_LOG_MODULE_MESSAGE,
    QCX_LOG_MODULE_TEST_APPLICATION,
    QCX_LOG_MODULE_ICP,
    QCX_LOG_MODULE_CAMX,
    QCX_LOG_MODULE_TPG,
    QCX_LOG_MODULE_DRV_COMMON,
    QCX_LOG_MODULE_LINK_MGR,
    QCX_LOG_MODULE_FUSA_AGGR,

    QCX_LOG_NUM_LOGGING_COMPONENTS

} QcxLogModule_e;

/*************************************************************************************************
@brief
Verbosity level of the log
**************************************************************************************************/
typedef enum
{
    QCX_LOG_LVL_ALWAYS = 0,             /* always log the message */
    QCX_LOG_LVL_FATAL,                  /* fatal error message */
    QCX_LOG_LVL_ERROR,                  /* error message */
    QCX_LOG_LVL_WARN,                   /* warning level message */
    QCX_LOG_LVL_HIGH,                   /* high verbosity level message */
    QCX_LOG_LVL_MEDIUM,                 /* medium verbosity level message */
    QCX_LOG_LVL_LOW,                    /* low verbosity level message */
    QCX_LOG_LVL_DEBUG,                  /* debug message */
    QCX_LOG_LVL_MAX_NUM,

    QCX_LOG_LVL_NONE,                   /* Don't log this component */
    QCX_LOG_LVL_MAX = 0x7FFFFFFF
} QcxLogLevel_e;

#ifdef LINUX_LRH
static const int msg_id[] = {
    LOG_EMERG,                         /* QCX_LOG_LVL_ALWAYS = 0 */
    LOG_ALERT,                         /* QCX_LOG_LVL_FATAL = 1 */
    LOG_ERR,                           /* QCX_LOG_LVL_ERROR = 2 */
    LOG_WARNING,                       /* QCX_LOG_LVL_WARN = 3 */
    LOG_NOTICE,                        /* QCX_LOG_LVL_HIGH = 4 */
    LOG_INFO,                          /* QCX_LOG_LVL_MEDIUM = 5 */
    LOG_INFO,                          /* QCX_LOG_LVL_LOW = 6 */
    LOG_DEBUG,                         /* QCX_LOG_LVL_DEBUG = 7 */
};

static int log2syslog(int id)
{
    if(id > QCX_LOG_LVL_DEBUG) {
        return msg_id[7];
    } else {
        return msg_id[id];
    }
}
#endif

/**================================================================================================
** Variables
================================================================================================**/
extern const char* g_OSALComponentStringLUT[];
extern QcxLogLevel_e g_OSALComponentLogLevels[];
extern const char* g_OSALLevelStringLUT[];
extern const uint8_t g_OSALLogLevelLut[];

#if defined(__QNXNTO__) && !defined(LINUX_LRH)
extern slog2_buffer_t g_OSALSlogBuffer;
#endif

/**================================================================================================
 FUNCTION Declarations
 ================================================================================================**/

/**************************************************************************************************
 * @brief
 * Initialize the logging service
 *
 * @return
 * Error code defined in CamStatus_e
 **************************************************************************************************/
CamStatus_e QcxLogInit(void);


#if defined(FEAT_LOG_CONFIG)
/**
 * @brief
 * Parses the logging config file
 *
 * @return
 */
CamStatus_e LogConfig(void);
#endif

/**================================================================================================
** Logging Macros
================================================================================================**/

#ifdef __QNXNTO__

    #ifdef LINUX_LRH

    #undef __FILENAME__
    #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

    #define CAM_TRACE_LOG(moduleId, fmt, ...)                                                   \
    do{                                                                                         \
        if (g_OSALComponentLogLevels[(QCX_LOG_MODULE_ ## moduleId)] == QCX_LOG_LVL_DEBUG)       \
        {                                                                                       \
            syslog(log2syslog(QCX_LOG_LVL_DEBUG),"[Debug ][%s] %s:%s:%d: " fmt "\n",            \
                                g_OSALComponentStringLUT[(QCX_LOG_MODULE_ ## moduleId)],        \
                                __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__);           \
        }                                                                                       \
    } while(0)

    #else

    #define QCX_LOG_OS_ID 666

    //QNX def present in QXA.QA.6.0/qnx_ap/qnx_bins/prebuilt_QOS220/target/qnx7/usr/include/sys/trace.h
    #define _NTO_TRACE_USERFIRST        (0x00000000u)

    /** Hardcoding the CAMERA_TRACE_EVENT_ID to 16 */
    #define CAMERA_TRACE_EVENT_ID _NTO_TRACE_USERFIRST+16

    //trace_logf is qnx call
    #define CAM_TRACE_LOG(moduleId, fmt, ...) \
            trace_logf((QCX_LOG_MODULE_ ## moduleId) + CAMERA_TRACE_EVENT_ID,"[%s][%s][%d] " fmt, \
            __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

    #endif
#else
    #define CAM_TRACE_LOG(moduleId, fmt, ...)
#endif



#if defined (__QNXNTO__)

    #if defined (LINUX_LRH)
    /*
    * Use this MACRO to log usual info, filtering will be done based on component ID
    * This MACRO dos not provide interrupt safe logging
    */
    #define QCX_LOG(moduleId, level, fmt, ...)                                              \
    do {                                                                                    \
        if ((QCX_LOG_LVL_ ## level) <=                                                      \
                g_OSALComponentLogLevels[(QCX_LOG_MODULE_ ## moduleId)])                    \
        {                                                                                   \
            syslog(log2syslog((QCX_LOG_LVL_ ## level)), "[%s][%s] %s:%s:%d: " fmt "\n",     \
                            g_OSALLevelStringLUT[(QCX_LOG_LVL_ ## level)],                  \
                            g_OSALComponentStringLUT[(QCX_LOG_MODULE_ ## moduleId)],        \
                            __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__);           \
        }                                                                                   \
    } while(0)

    /*
    * The QCX_LOG_FAST_X Macros are faster than QCX_LOG, use these to achieve less impact on
    * system timing
    * Can only take U32 as params
    */
    #define QCX_LOG_FAST_0(moduleId, level, fmt, ...)                              \
            QCX_LOG(moduleId, level, fmt, ##__VA_ARGS__)


    #define QCX_LOG_FAST(moduleId, level, fmt, ...)


    #define QCX_LOG_FAST_1(moduleId, level, fmt, p0)                               \
            QCX_LOG_FAST(moduleId, level, fmt, p0)


    #define QCX_LOG_FAST_2(moduleId, level, fmt, p0, p1)                           \
            QCX_LOG_FAST(moduleId, level, fmt, p0)


    #define QCX_LOG_FAST_3(moduleId, level, fmt, p0, p1, p2)                       \
            QCX_LOG_FAST(moduleId, level, fmt, p0)


    #define QCX_LOG_FAST_4(moduleId, level, fmt, p0, p1, p2, p3)                   \
            QCX_LOG_FAST(moduleId, level, fmt, p0)

    #else

    /*
    * Use this MACRO to log usual info, filtering will be done based on component ID
    * This MACRO dos not provide interrupt safe logging
    */
    #define QCX_LOG(moduleId, level, fmt, ...)                                     \
    do {                                                                           \
        if ((QCX_LOG_LVL_ ## level) <=                                             \
                g_OSALComponentLogLevels[(QCX_LOG_MODULE_ ## moduleId)])           \
        {                                                                          \
            (void)slog2f(g_OSALSlogBuffer,                                         \
                QCX_LOG_OS_ID, g_OSALLogLevelLut[(QCX_LOG_LVL_ ## level)],         \
                "[%s][%s] %s:%d: " fmt,                                            \
                g_OSALLevelStringLUT[(QCX_LOG_LVL_ ## level)],                     \
                g_OSALComponentStringLUT[(QCX_LOG_MODULE_ ## moduleId)],           \
                __FILENAME__, __LINE__, ##__VA_ARGS__);                            \
        }                                                                          \
    } while(0)


    /*
    * system timing
    * Can only take U32 as params
    */
    #define QCX_LOG_FAST_0(moduleId, level, fmt)                                   \
    do {                                                                           \
        if ((QCX_LOG_LVL_ ## level) <=                                             \
                g_OSALComponentLogLevels[(QCX_LOG_MODULE_ ## moduleId)])           \
        {                                                                          \
            (void)slog2fa(g_OSALSlogBuffer,                                        \
                QCX_LOG_OS_ID,                                                   \
                g_OSALLogLevelLut[(QCX_LOG_LVL_ ## level)],                      \
                "[%s][%s] %s:%d: " fmt,                                          \
                SLOG2_FA_STRING(g_OSALLevelStringLUT[(QCX_LOG_LVL_ ## level)]),  \
                SLOG2_FA_STRING(g_OSALComponentStringLUT[(QCX_LOG_MODULE_ ## moduleId)]), \
                SLOG2_FA_STRING(__FILENAME__),                                   \
                SLOG2_FA_SIGNED(__LINE__),                                       \
                SLOG2_FA_END);                                                   \
        }                                                                          \
    } while(0)


    #define QCX_LOG_FAST_1(moduleId, level, fmt, p0)                               \
    do {                                                                           \
        if ((QCX_LOG_LVL_ ## level) <=                                             \
                g_OSALComponentLogLevels[(QCX_LOG_MODULE_ ## moduleId)])           \
        {                                                                          \
            (void)slog2fa(g_OSALSlogBuffer,                                        \
                QCX_LOG_OS_ID,                                                   \
                g_OSALLogLevelLut[(QCX_LOG_LVL_ ## level)],                      \
                "[%s][%s] %s:%d: " fmt,                                          \
                SLOG2_FA_STRING(g_OSALLevelStringLUT[(QCX_LOG_LVL_ ## level)]),  \
                SLOG2_FA_STRING(g_OSALComponentStringLUT[(QCX_LOG_MODULE_ ## moduleId)]), \
                SLOG2_FA_STRING(__FILENAME__),                                   \
                SLOG2_FA_SIGNED(__LINE__),                                       \
                SLOG2_FA_UNSIGNED((p0)),                                         \
                SLOG2_FA_END);                                                   \
        }                                                                          \
    } while(0)


    #define QCX_LOG_FAST_2(moduleId, level, fmt, p0, p1)                           \
    do {                                                                           \
        if ((QCX_LOG_LVL_ ## level) <=                                             \
                g_OSALComponentLogLevels[(QCX_LOG_MODULE_ ## moduleId)])           \
        {                                                                          \
            (void)slog2fa(g_OSALSlogBuffer,                                        \
                QCX_LOG_OS_ID,                                                    \
                g_OSALLogLevelLut[(QCX_LOG_LVL_ ## level)],                       \
                "[%s][%s] %s:%d: " fmt,                                           \
                SLOG2_FA_STRING(g_OSALLevelStringLUT[(QCX_LOG_LVL_ ## level)]),   \
                SLOG2_FA_STRING(g_OSALComponentStringLUT[(QCX_LOG_MODULE_ ## moduleId)]), \
                SLOG2_FA_STRING(__FILENAME__),                                    \
                SLOG2_FA_SIGNED(__LINE__),                                        \
                SLOG2_FA_UNSIGNED((p0)),                                          \
                SLOG2_FA_UNSIGNED((p1)),                                          \
                SLOG2_FA_END);                                                    \
        }                                                                          \
    } while(0)


    #define QCX_LOG_FAST_3(moduleId, level, fmt, p0, p1, p2)                       \
    do {                                                                           \
        if ((QCX_LOG_LVL_ ## level) <=                                             \
                g_OSALComponentLogLevels[(QCX_LOG_MODULE_ ## moduleId)])           \
        {                                                                          \
            (void)slog2fa(g_OSALSlogBuffer,                                        \
                QCX_LOG_OS_ID,                                                    \
                g_OSALLogLevelLut[(QCX_LOG_LVL_ ## level)],                       \
                "[%s][%s] %s:%d: " fmt,                                           \
                SLOG2_FA_STRING(g_OSALLevelStringLUT[(QCX_LOG_LVL_ ## level)]),   \
                SLOG2_FA_STRING(g_OSALComponentStringLUT[(QCX_LOG_MODULE_ ## moduleId)]),    \
                SLOG2_FA_STRING(__FILENAME__),                                    \
                SLOG2_FA_SIGNED(__LINE__),                                        \
                SLOG2_FA_UNSIGNED((p0)),                                          \
                SLOG2_FA_UNSIGNED((p1)),                                          \
                SLOG2_FA_UNSIGNED((p2)),                                          \
                SLOG2_FA_END);                                                    \
        }                                                                          \
    } while(0)


    #define QCX_LOG_FAST_4(moduleId, level, fmt, p0, p1, p2, p3)                   \
    do {                                                                           \
        if ((QCX_LOG_LVL_ ## level) <=                                             \
            g_OSALComponentLogLevels[(QCX_LOG_MODULE_ ## moduleId)])               \
        {                                                                          \
            (void)slog2fa(g_OSALSlogBuffer,                                        \
                QCX_LOG_OS_ID,                                                     \
                g_OSALLogLevelLut[(QCX_LOG_LVL_ ## level)],                        \
                "[%s][%s] %s:%d: " fmt,                                            \
                SLOG2_FA_STRING(g_OSALLevelStringLUT[(QCX_LOG_LVL_ ## level)]),    \
                SLOG2_FA_STRING(g_OSALComponentStringLUT[(QCX_LOG_MODULE_ ## moduleId)]),    \
                SLOG2_FA_STRING(__FILENAME__),                                     \
                SLOG2_FA_SIGNED(__LINE__),                                         \
                SLOG2_FA_UNSIGNED((p0)),                                           \
                SLOG2_FA_UNSIGNED((p1)),                                           \
                SLOG2_FA_UNSIGNED((p2)),                                           \
                SLOG2_FA_UNSIGNED((p3)),                                           \
                SLOG2_FA_END);                                                     \
        }                                                                          \
    } while(0)
    #endif


#elif defined(CAMERA_UNITTEST) || defined(__ANDROID__) || defined(__AGL__)

/**================================================================================================
 FUNCTION Declarations
 ================================================================================================**/

/**************************************************************************************************
 * @brief
 * Initialize the logging service
 *
 * @return
 * Error code defined in CamStatus_e
 **************************************************************************************************/
CamStatus_e QcxLogInit(void);

void QcxLog(FILE *fp, QcxLogLevel_e level, const char *fmt, ...);

/*
 * Use this MACRO to log usual info, filtering will be done based on component ID
 * This MACRO dos not provide interrupt safe logging
 */
#define QCX_LOG(moduleId, level, fmt, ...)                                     \
do {                                                                           \
    if ((QCX_LOG_LVL_ ## level) <=                                             \
            g_OSALComponentLogLevels[(QCX_LOG_MODULE_ ## moduleId)])           \
    {                                                                          \
        (void)QcxLog(stderr,                                                   \
              (QCX_LOG_LVL_ ## level),                                         \
              "[%s][%s] %s:%d: " fmt "\n",                                     \
              g_OSALLevelStringLUT[(QCX_LOG_LVL_ ## level)],                   \
              g_OSALComponentStringLUT[(QCX_LOG_MODULE_ ## moduleId)],         \
              __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__);                   \
    }                                                                          \
} while(0)

#define QCX_LOG_FAST_0 QCX_LOG
#define QCX_LOG_FAST_1 QCX_LOG
#define QCX_LOG_FAST_2 QCX_LOG
#define QCX_LOG_FAST_3 QCX_LOG
#define QCX_LOG_FAST_4 QCX_LOG

#else
#error "Unsupported configuration"
#endif //__QNXNTO__

#ifdef CAMERA_UNITTEST
    #define CAM_TRACE_LOG(moduleId, fmt, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* QCX_LOG_H */

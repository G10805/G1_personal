/**================================================================================================

 @file
 qcxlog.c

 @brief
 This file implements logging functionality.

 Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.

 ================================================================================================**/

/**================================================================================================
 INCLUDE FILES FOR MODULE
 ================================================================================================**/
#include <stdio.h>
#if defined (CAMERA_UNITTEST) || defined(__ANDROID__) || defined(__AGL__)
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdarg.h>

#if defined(__ANDROID__)
#include <sys/types.h>
#include <android/log.h>
#include <fcntl.h>
#endif
#endif

#include <stdlib.h>


#ifdef LINUX_LRH
#include "aosal_debug_msg.h"
#include "qcxutils.h"
#endif

#include "qcxlog.h"

/**================================================================================================
 ** Constant and Macros
 ================================================================================================**/
#if __QNXNTO__
//This will be the overall verbosity level for the camera component
//Modify only for debugging purposes
#define DEFAULT_OS_LOG_VERBOSITY SLOG2_DEBUG2


// Based on recommendation from the Platform team, the maximum number of pages should be restricted
// to 62. The buffer size is therefore defined as 248KB (248/4 = 62) instead of 256KB.
#define LOG_BUFFER_SIZE_KB       248
#define QCX_LOG_NUM_BUFF_PAGES   (LOG_BUFFER_SIZE_KB / 4)
#define MAX_NUM_STREAM           16U

extern char * __progname;

#ifndef LINUX_LRH
// Used to translate internal logging levels to QNX levels
const uint8_t g_OSALLogLevelLut[QCX_LOG_LVL_MAX_NUM] = {
    SLOG2_CRITICAL,     //QCX_LOG_LVL_ALWAYS
    SLOG2_CRITICAL,     //QCX_LOG_LVL_FATAL
    SLOG2_ERROR,        //QCX_LOG_LVL_ERROR
    SLOG2_WARNING,      //QCX_LOG_LVL_WARN
    SLOG2_NOTICE,       //QCX_LOG_LVL_HIGH
    SLOG2_INFO,         //QCX_LOG_LVL_MEDIUM
    SLOG2_DEBUG1,       //QCX_LOG_LVL_LOW
    SLOG2_DEBUG2,       //QCX_LOG_LVL_DEBUG
};
#endif

#endif //__QNXNTO__

// When printing, this will be used to MAP enum to a text
/*WARNING: BEFORE EDITING THE VARIABLE BELOW READ THIS

ANY MODIFICATIONS IN THIS CONST MUST BE REFLECTED IN THE LoggingComponents_e in qcxlog.h
and g_OSALComponentLogLevels below*/

const char *g_OSALComponentStringLUT[QCX_LOG_NUM_LOGGING_COMPONENTS] = {
    "CLIENT_MGR",               // Client Manager
    "CONFIG_MGR",               // Config Manager
    "STREAM_MGR",               // Stream Manager
    "SENSOR_MGR",               // Senstor Manager
    "DEV_MGR",                  // Camera Device Manager
    "MSG_DPR",                  // Message Dispatcher
    "ISP",                      // ISP Device
    "IFE",
    "CSIPHY",
    "SENSOR",                   // SENSOR
    "CDM",                      // CDM
    "EXTHW",
    "PLM",
    "DIAG_MGR",
    "OSAL",
    "HTM",
    "SI",                       // Service Initializer
    "MESSAGE",
    "TEST_APP",
    "ICP",
    "CAMX",
    "TPG",
    "DRV_COMMON",
    "LINK_MGR",
    "FUSA_AGGR"
};

//
// We rarely use critical messages; so, space
// pading up to the next longest, most frequently
// use level - Medium.  Padding is done here to
// avoid hardcoding magic number in "%-6s" when
// used for args.
//
// Used to print the level string as part of trace
const char *g_OSALLevelStringLUT[QCX_LOG_LVL_MAX_NUM] = {
    "Always",           //QCX_LOG_LVL_ALWAYS
    "Fatal ",           //QCX_LOG_LVL_FATAL
    "Error ",           //QCX_LOG_LVL_ERROR
    "Warn  ",           //QCX_LOG_LVL_WARN
    "High  ",           //QCX_LOG_LVL_HIGH
    "Medium",           //QCX_LOG_LVL_MEDIUM
    "Low   ",           //QCX_LOG_LVL_LOW
    "Debug ",           //QCX_LOG_LVL_DEBUG
};


/**================================================================================================
 ** Typedefs
 ================================================================================================**/

/**================================================================================================
 ** Variables
 ================================================================================================**/
// Default logging level for each component
QcxLogLevel_e g_OSALComponentLogLevels[QCX_LOG_NUM_LOGGING_COMPONENTS] = {
    QCX_LOG_LVL_WARN,           //CLIENT_MGR
    QCX_LOG_LVL_WARN,           //CONFIG_MGR
    QCX_LOG_LVL_WARN,           //STREAM_MGR
    QCX_LOG_LVL_WARN,           //SENSOR_MGR
    QCX_LOG_LVL_WARN,           //DEV_MGR
    QCX_LOG_LVL_WARN,           //DPR
    QCX_LOG_LVL_WARN,           //ISP
    QCX_LOG_LVL_WARN,           //IFE
    QCX_LOG_LVL_WARN,           //CSIPHY
    QCX_LOG_LVL_WARN,           //SENSOR
    QCX_LOG_LVL_WARN,           //CDM
    QCX_LOG_LVL_WARN,           //EXTHW
    QCX_LOG_LVL_WARN,           //PLM
    QCX_LOG_LVL_WARN,           //DIAG_MGR
    QCX_LOG_LVL_WARN,           //OSAL
    QCX_LOG_LVL_WARN,           //HTM
    QCX_LOG_LVL_WARN,           //SI
    QCX_LOG_LVL_WARN,           //MESSAGE
    QCX_LOG_LVL_WARN,           //TEST_APPL
    QCX_LOG_LVL_WARN,           //ICP
    QCX_LOG_LVL_WARN,           //CAMX
    QCX_LOG_LVL_WARN,           //TPG
    QCX_LOG_LVL_WARN,           //DRV COMMON
    QCX_LOG_LVL_WARN,           //LINK MGR
    QCX_LOG_LVL_WARN,           //FUSA AGGR
};

#if defined(__QNXNTO__) && !defined(LINUX_LRH)
// QNX Logging buffers
#define NUM_SLOG_BUFFERS             2
static slog2_buffer_t sg_slogBuffers[NUM_SLOG_BUFFERS];

slog2_buffer_t g_OSALSlogBuffer = NULL;
#endif //__QNXNTO__

#if defined(CAMERA_UNITTEST) || defined(__ANDROID__) || defined(__AGL__)

static uint64_t sg_startTime = 0;
#endif

/**================================================================================================
 FUNCTION Declarations
 ================================================================================================**/

/**================================================================================================
 FUNCTION DEFINITIONS
 ================================================================================================**/
/**************************************************************************************************
 * @brief
 * Initialize the logging service
 *
 * @return
 * Error code defined in CamStatus_e
 **************************************************************************************************/
CamStatus_e QcxLogInit(void)
{
    CamStatus_e status = CAMERA_EFAILED;

#ifdef __QNXNTO__
    #ifdef LINUX_LRH

    AOErrorCode_e aosalStatus = AO_ERROR_FAILURE;

    //Also initialize AOSAL logging
    aosalStatus = AODebugMsgInitialize();
    status = OSAL_MapError(aosalStatus);
    if(CAMERA_SUCCESS != status)
    {
        // Not really something for which we need to fail camera init
        QCX_LOG(OSAL, ERROR,"AOSAL Logging Init Failed with Error %d", status);
    }

    #else
    slog2_buffer_set_config_t bufferConfig =
    {
        .num_buffers = 2,
        .buffer_set_name = __progname,
        .verbosity_level = DEFAULT_OS_LOG_VERBOSITY,
        .buffer_config =
        {
                {
                    .buffer_name = "QCX",
                    .num_pages = QCX_LOG_NUM_BUFF_PAGES
                },
                {
                    .buffer_name = "CAMX",
                    .num_pages = QCX_LOG_NUM_BUFF_PAGES
                }
        }
    };

    if (0 != slog2_register(&bufferConfig,
                            sg_slogBuffers,
                            0U))
    {
        (void)fprintf(stderr, "Log buffer initialize failed\n");
    }
    else
    {
        g_OSALSlogBuffer = sg_slogBuffers[0];

        // set buffer_config 1 as default for CAMX logs
        (void)slog2_set_default_buffer(sg_slogBuffers[1]);

        status = CAMERA_SUCCESS;
    }
    #endif
#elif defined(CAMERA_UNITTEST) || defined(__ANDROID__) || defined(__AGL__)
    struct timespec ts = {};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    sg_startTime = (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    status = CAMERA_SUCCESS;
#endif

#if defined(FEAT_LOG_CONFIG)
    if(CAMERA_SUCCESS == status)
    {
        status =  LogConfig();
    }
#endif

#if defined(FEAT_LOG_ENVCONFIG)
    //check env
    char *env_var = getenv("CAM_DEBUG");
    if (env_var != NULL)
    {
        int32_t configLevel = atoi(env_var);
        for (uint32_t compIdx = 0; compIdx < (uint32_t) QCX_LOG_NUM_LOGGING_COMPONENTS; compIdx++)
        {
            g_OSALComponentLogLevels[compIdx] = (QcxLogLevel_e) configLevel;
        }

    }
#endif
    return status;

}

#if (defined (CAMERA_UNITTEST) || defined(__AGL__)) && !defined (__ANDROID__)
#ifndef LINUX_LRH
/**
 * @brief prints formatted log into buffer
 *
 * @param fp points to file pointer
 * @param level log level
 * @param fmt points to a format string
 * @param va variadic pointer
 *
 * @return void
 */
void QcxLog(FILE *fp, QcxLogLevel_e level, const char *fmt, ...)
{
    struct timespec ts;
    long long curr;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    curr = (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#ifndef __AGL__
    curr -= sg_startTime;
#endif

    fprintf(fp, "%03lld.%03lld [%u:%u] ", curr/1000, curr%1000,
            (unsigned int)getpid(), (unsigned int)(uintptr_t)pthread_self());

    va_list va;
    va_start(va, fmt);

    (void)vfprintf(fp, fmt, va);

    va_end(va);

    if (level <= QCX_LOG_LVL_WARN)
    {
        fflush(fp);
    }
}
#endif
#endif

#if defined (__ANDROID__)

/**
 * @brief prints formatted log into buffer
 *
 * @param fp points to file pointer
 * @param level log level
 * @param fmt points to a format string
 * @param va variadic pointer
 *
 * @return void
 */
void QcxLog(FILE *fp, QcxLogLevel_e level, const char *fmt, ...)
{
    int rc;
    struct timespec ts;
    long long curr;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    curr = (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    curr -= sg_startTime;

//    fprintf(fp, "%03lld.%03lld [%u:%u] ", curr/1000, curr%1000,
//            (unsigned int)getpid(), (unsigned int)(uintptr_t)pthread_self());
    (void)fp;

    va_list va;
    va_start(va, fmt);

    rc = __android_log_vprint(level <= QCX_LOG_LVL_WARN ? ANDROID_LOG_ERROR : ANDROID_LOG_INFO,
                                "QcxLog", fmt, va);
    va_end(va);
}
#endif

#ifndef __CAMERAEVENTLOG_H_
#define __CAMERAEVENTLOG_H_

/**
 * @file CameraEventLog.h
 *
 * @brief Declarations needed for logging key camera events
 *
 * Copyright (c) 2009-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

/* ===========================================================================
**                      INCLUDE FILES
** ======================================================================== */
#include "CameraResult.h"
#include "CameraOSServices.h"


#ifdef CAMERA_FEATURE_ENABLE_TRACE_EVENT
#ifdef _LINUX
#define ATRACE_TAG (ATRACE_TAG_CAMERA | ATRACE_TAG_HAL)
#include <cutils/trace.h>
#endif

#endif // CAMERA_FEATURE_ENABLE_TRACE_EVENT


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/* ===========================================================================
**                      DATA DECLARATIONS
** ======================================================================== */

#if defined(_LINUX) && defined(CAMERA_FEATURE_ENABLE_TRACE_EVENT)
extern uint32 g_trace_groupsEnable;
static const uint32 MaxTraceStringLength = 512;     ///< AIS internal trace message length limit
#endif

/* ---------------------------------------------------------------------------
** Constant / Define Declarations
** ------------------------------------------------------------------------ */

/* ---------------------------------------------------------------------------
** Type Definitions
** ------------------------------------------------------------------------ */

/// This enumerates all key camera core and client events
typedef enum
{
    // Invalid event - intial value
    CAMERA_EVENT_INVALID,

    // Engine
    CAMERA_ENGINE_EVENT_INITIALIZE_START = 100,
    CAMERA_ENGINE_EVENT_INITIALIZE_DONE,
    CAMERA_ENGINE_EVENT_USRCTXT_START,
    CAMERA_ENGINE_EVENT_POWER_SUSPEND_START,
    CAMERA_ENGINE_EVENT_POWER_SUSPEND_DONE,
    CAMERA_ENGINE_EVENT_POWER_RESUME_START,
    CAMERA_ENGINE_EVENT_POWER_RESUME_DONE,
    CAMERA_ENGINE_EVENT_CONFIGURERS_INIT_START,
    CAMERA_ENGINE_EVENT_CONFIGURERS_INIT_DONE,
    CAMERA_ENGINE_EVENT_CONFIGURERS_CONFIG,
    CAMERA_ENGINE_EVENT_CONFIGURERS_CONFIG_DONE,
    CAMERA_ENGINE_EVENT_CONFIGURERS_START,
    CAMERA_ENGINE_EVENT_CONFIGURERS_START_DONE,
    CAMERA_ENGINE_EVENT_INPUT_PROBE,
    CAMERA_ENGINE_EVENT_INPUT_PROBE_DONE,
    CAMERA_ENGINE_EVENT_QUEUE_BUFFERS,
    CAMERA_ENGINE_EVENT_FRAME_DONE,
    CAMERA_ENGINE_EVENT_FRAME_DONE_SEND,

    // Sensor
    CAMERA_SENSOR_EVENT_PROBED = 200,
    CAMERA_SENSOR_EVENT_INITIALIZE_START,
    CAMERA_SENSOR_EVENT_INITIALIZE_DONE,
    CAMERA_SENSOR_EVENT_STREAM_START,
    CAMERA_SENSOR_EVENT_STREAM_STOP,

    // CameraQueue Events
    CAMERA_CORE_EVENT_CAMERAQUEUECREATE = 300,
    CAMERA_CORE_EVENT_CAMERAQUEUEDESTROY,
    CAMERA_CORE_EVENT_CAMERAQUEUECLEAR,
    CAMERA_CORE_EVENT_CAMERAQUEUEENQUEUE,
    CAMERA_CORE_EVENT_CAMERAQUEUEDEQUEUE,
    CAMERA_CORE_EVENT_CAMERAQUEUEISEMPTY,
    CAMERA_CORE_EVENT_CAMERAQUEUEDROPHEAD,

    CAMERA_PLATFORM_EVENT_VFE_DEQUEUE_EVENT = 400,
    CAMERA_PLATFORM_EVENT_VFE_POLL,

    // Error event
    CAMERA_EVENT_ERROR = 500,

    // Next event, indicating that the previous entry was the most recent
    // event that occured
    CAMERA_EVENT_NEXT = 1000
} CameraEventType;

/// This structure contains camera event information
typedef struct
{
    CameraEventType eCameraEvent;
    uint64 nPayload;
    uint64 nPayload2;
    uint64 timestamp;
} CameraEventInfoType;

static const uint32 AISTraceGroupIFE = 0x1 << 1;
static const uint32 AISTraceGroupEngine = 0x1 << 2;


/* ===========================================================================
**                      MACROS
** ======================================================================== */

/* ===========================================================================
**                      FUNCTION DECLARATIONS
** ======================================================================== */
#if defined(CAMERA_FEATURE_ENABLE_LOG_EVENT)
/**
 * This function is used to initialize Camera event log
 *
 * @param[] NONE
 */
CameraResult CameraEventLogInitialize(void);

/**
 * This function is used to uninitialize Camera event log
 *
 * @param[] NONE
 */
CameraResult CameraEventLogUninitialize(void);

/**
 * This function is used to log key camera events.
 *
 * @param[in] event this is specifying the
 * name of the core or client event to be logged
 * @param[in] nEventPayload this is specifying the
 * event payload to be logged
 * @param[in] result this is specifying the result
 * at the event to be logged
 */
CAM_API void CameraLogEvent(CameraEventType eCameraEvent, uint64 nPayload, uint64 nPayload2);

/**
 * Retrieves the size of the buffer needed to hold the camera event log buffer in bytes.
 * A buffer of this size should be provided to the GetCameraEventLogBuffer method.
 * @return Size of the tuning data set in bytes.
*/
uint32 GetCameraEventLogBufferSizeReq(void);

/**
 * Retrieves the camera event log buffer to a client-provided buffer. This data
 * should be written out with the JPEG header to support debug efforts.
 * @param[out] pEventLogBuffer Pointer to caller allocated memory where
 *             the debug trace buffer can be stored. This buffer size should
 *             be greater or equal to the number of bytes returned by
 *             GetDebugTraceBufferSizeReq.
 * @param[in]  nEventLogBufferLen Length of the caller allocated buffer passed
 *             in as pEventLogBuffer.
 * @param[out] pnEventLogBufferLenReq Number of bytes actually written to
 *             pEventLogBuffer. This may be different than the value
 *             returned by GetCameraEventLogBufferSizeReq.
 * @return CAMERA_SUCCESS if OK.
*/
CameraResult GetCameraEventLogBuffer(
    byte* pEventLogBuffer,
    uint32 nEventLogBufferLen,
    uint32* pnEventLogBufferLenReq);

/**
 * Flushes log buffer to file
 */
void CameraLogDumpBuffer(void);

#else

#define CameraEventLogInitialize(...) CAMERA_SUCCESS
#define CameraEventLogUninitialize(...)
#define CameraLogEvent(...)
#define GetCameraEventLogBufferSizeReq(...)
#define GetCameraEventLogBuffer(...)
#define CameraLogDumpBuffer(...)

#endif /* CAMERA_FEATURE_ENABLE_LOG_EVENT */


#if defined(CAMERA_FEATURE_ENABLE_TRACE_EVENT) && defined(_LINUX)

CAM_API void set_trace_groupsEnable(uint32 groupsEnable);

CAM_API void ais_trace_snprintf(char* str, uint32 size, const char *fmt, ...);

#define AIS_TRACE_SYNC_BEGIN_F(group, ...)                                                                                   \
    if (g_trace_groupsEnable & group)                                                                               \
    {                                                                                                                         \
        char traceNameRandomString[MaxTraceStringLength];                                                               \
        ais_trace_snprintf(traceNameRandomString, sizeof(traceNameRandomString), ##__VA_ARGS__);                         \
        ATRACE_BEGIN(traceNameRandomString);                                                                                  \
    }

#define AIS_TRACE_ASYNC_BEGIN_F(group, id, ...)                                                                              \
    if (g_trace_groupsEnable & group)                                                                               \
    {                                                                                                                         \
        char traceNameRandomString[MaxTraceStringLength];                                                               \
        ais_trace_snprintf(traceNameRandomString, sizeof(traceNameRandomString), ##__VA_ARGS__);                         \
        ATRACE_ASYNC_BEGIN(traceNameRandomString, static_cast<UINT32>(id));                                                   \
    }

#define AIS_TRACE_ASYNC_END_F(group, id, ...)                                                                                \
    if (g_trace_groupsEnable & group)                                                                               \
    {                                                                                                                         \
        char traceNameRandomString[MaxTraceStringLength];                                                               \
        ais_trace_snprintf(traceNameRandomString, sizeof(traceNameRandomString), ##__VA_ARGS__);                         \
        ATRACE_ASYNC_END(traceNameRandomString, static_cast<UINT32>(id));                                                     \
    }

#define AIS_TRACE_INT32_F(group, value, ...)                                                                                 \
    if (g_trace_groupsEnable & group)                                                                               \
    {                                                                                                                         \
        char traceNameRandomString[MaxTraceStringLength];                                                               \
        ais_trace_snprintf(traceNameRandomString, sizeof(traceNameRandomString), ##__VA_ARGS__);                         \
        ATRACE_INT(traceNameRandomString, value);                                                                             \
    }

#define AIS_TRACE_MESSAGE_F(group, ...)                                                                                      \
    if (g_trace_groupsEnable & group)                                                                               \
    {                                                                                                                         \
        char traceNameRandomString[MaxTraceStringLength];                                                               \
        ais_trace_snprintf(traceNameRandomString, sizeof(traceNameRandomString), ##__VA_ARGS__);                           \
        ATRACE_BEGIN(traceNameRandomString);                                                                                  \
        ATRACE_END();                                                                                                         \
    }

#define AIS_TRACE_SYNC_BEGIN(group, pName)                                                                                   \
    if (g_trace_groupsEnable & group)                                                                               \
    {                                                                                                                         \
        ATRACE_BEGIN(pName);                                                                                                  \
    }

#define AIS_TRACE_SYNC_END(group)                                                                                            \
    if (g_trace_groupsEnable & group)                                                                               \
    {                                                                                                                         \
        ATRACE_END();                                                                                                         \
    }

#define AIS_TRACE_MESSAGE(group, pString)                                                                                    \
    if (g_trace_groupsEnable & group)                                                                               \
    {                                                                                                                         \
        ATRACE_BEGIN(pString);                                                                                                \
        ATRACE_END();                                                                                                         \
    }

#else

#define AIS_TRACE_SYNC_BEGIN_F(...)
#define AIS_TRACE_ASYNC_BEGIN_F(...)
#define AIS_TRACE_ASYNC_END_F(...)
#define AIS_TRACE_INT32_F(...)
#define AIS_TRACE_MESSAGE_F(...)
#define AIS_TRACE_SYNC_BEGIN(...)
#define AIS_TRACE_SYNC_END(...)
#define AIS_TRACE_MESSAGE(...)

#endif /* CAMERA_FEATURE_ENABLE_LOG_EVENT */


#ifdef __cplusplus
} // extern "C"
#endif  // __cplusplus

#endif // __CAMERAEVENTLOG_H_

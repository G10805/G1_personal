/**
 * @file CameraEventLog.cpp
 *
 * @brief Implement logging API for key camera events
 *
 * Copyright (c) 2009-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

/* ===========================================================================
                        INCLUDE FILES FOR MODULE
=========================================================================== */
#include "AEEstd.h"

#include "CameraEventLog.h"
#include "CameraOSServices.h"
 #include <stdio.h>

#ifdef CAMERA_FEATURE_ENABLE_TRACE_EVENT

#if defined(__QNXNTO__)
#include <sys/neutrino.h>
#include <sys/trace.h>
#endif

#endif

extern "C"
{
/*NOTE: this feature will require additional changes to enable it with multiple engines
 as they would both be logging to same trace buffer */
#ifdef CAMERA_FEATURE_ENABLE_LOG_EVENT
/* ===========================================================================
**                 DECLARATIONS AND DEFINITIONS FOR MODULE
** ======================================================================== */

/* --------------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define MAX_NUMBER_OF_EVENTS 10000
/* --------------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */

// This stores the most recent MAX_NUMBER_OF_EVENTS events
static CameraEventInfoType* s_CameraEvents = NULL;

// This points to the next event
static volatile uint32 s_nNextCameraEvent = 0;
static volatile uint32 s_nFirstCameraEvent = 0;

// mutex protection for multithreads
static CameraMutex s_DeviceMutex;

static boolean s_initialized = FALSE;

/* --------------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* ===========================================================================
**                      MACRO DEFINITIONS
** ======================================================================== */

/* ===========================================================================
**                      FUNCTION DEFINITIONS
** ======================================================================== */
/**
 * CameraEventLogInitialize
 */
CameraResult CameraEventLogInitialize(void)
{
    CameraResult result;

    CAM_MSG(MEDIUM, "Initializing CameraEventLog");

    if (s_initialized)
    {
        CAM_MSG(ERROR, "CameraEventLog already initialized");
        return CAMERA_SUCCESS;
    }

    FILE* fp = fopen("/data/misc/camera/CameraEventLog.csv", "w");
    if (fp)
    {
        fclose(fp);
    }

    s_CameraEvents = (CameraEventInfoType*)CameraAllocate(CAMERA_ALLOCATE_ID_UNASSIGNED, MAX_NUMBER_OF_EVENTS * sizeof(CameraEventInfoType));
    if (!s_CameraEvents)
    {
        CAM_MSG(ERROR, "Failed to allocate camera events");
        return CAMERA_ENOMEMORY;
    }

    result = CameraCreateMutex(&s_DeviceMutex);
    if (CAMERA_SUCCESS == result)
    {

        // Initialize all camera events
        (void)std_memset(s_CameraEvents, 0, sizeof(s_CameraEvents));

        // Initialize the next camera event index
        s_nNextCameraEvent = 0;
        s_nFirstCameraEvent = 0;

        s_initialized = TRUE;
    }
    else
    {
        CameraFree(CAMERA_ALLOCATE_ID_UNASSIGNED, s_CameraEvents);
    }

    return result;
}

/**
 * CameraEventLogUninitialize
 */
CameraResult CameraEventLogUninitialize(void)
{
    CameraFree(CAMERA_ALLOCATE_ID_UNASSIGNED, s_CameraEvents);
    CameraDestroyMutex(s_DeviceMutex);
    s_DeviceMutex = NULL;
    s_initialized = FALSE;

    return CAMERA_SUCCESS;
}

/**
 * CameraLogEvent
 */
CAM_API void CameraLogEvent(CameraEventType eCameraEvent, uint64 nPayload, uint64 nPayload2)
{
    if (!s_initialized)
    {
        CAM_MSG(ERROR, "CameraLogEvent is not initialized");
        return;
    }

    uint32 nNextCameraEvent;

    CameraLockMutex(s_DeviceMutex);

    nNextCameraEvent = s_nNextCameraEvent++;
    if (s_nNextCameraEvent >= MAX_NUMBER_OF_EVENTS)
    {
        //append to output file when full
        CameraLogDumpBuffer();

        s_nNextCameraEvent = 0;
    }

    CameraUnlockMutex(s_DeviceMutex);

    s_CameraEvents[nNextCameraEvent].eCameraEvent = eCameraEvent;
    s_CameraEvents[nNextCameraEvent].nPayload = nPayload;
    s_CameraEvents[nNextCameraEvent].nPayload2 = nPayload2;
    CameraGetTime(&s_CameraEvents[nNextCameraEvent].timestamp);
}

uint32 GetCameraEventLogBufferSizeReq(void)
{
    return (uint32)sizeof(s_CameraEvents);
}

CameraResult GetCameraEventLogBuffer(
    byte* pEventLogBuffer,
    uint32 nEventLogBufferLen,
    uint32* pnEventLogBufferLenReq)
{
    CameraResult result = CAMERA_EFAILED;

    uint32 nSizeEventLog = sizeof(s_CameraEvents);

    if (NULL != pnEventLogBufferLenReq)
    {

        *pnEventLogBufferLenReq = nSizeEventLog;

        if ((NULL != pEventLogBuffer) &&
            (nEventLogBufferLen >= nSizeEventLog))
        {
            // Copy the event log
#ifdef __ANDROID__
            memcpy(pEventLogBuffer, s_CameraEvents, nSizeEventLog);
#else
            std_memcpy_s(pEventLogBuffer, nEventLogBufferLen, s_CameraEvents, nSizeEventLog);
#endif
            result = CAMERA_SUCCESS;
        }
        else if (NULL == pEventLogBuffer && 0 == nEventLogBufferLen)
        {
            // If no input buffer provided it is not an error. Just report the length requried.
            result = CAMERA_SUCCESS;
        }
    }

    return result;
}

/**
 * Flushes log buffer to file
 */
void CameraLogDumpBuffer(void)
{
    if (!s_initialized)
    {
        CAM_MSG(ERROR, "CameraLogEvent is not initialized");
        return;
    }

    CameraLockMutex(s_DeviceMutex);

    FILE* fp = fopen("/data/misc/camera/CameraEventLog.csv", "a");
    if (!fp)
    {
        CAM_MSG(ERROR, "CameraEventLog failed to open log file");
    }
    else
    {
        int idx = s_nFirstCameraEvent;
        int lastItem = s_nNextCameraEvent;

        CAM_MSG(ERROR, "Dumping CameraEventLog from %d to %d", idx, lastItem);

        while (idx != lastItem)
        {
            if (idx == MAX_NUMBER_OF_EVENTS)
            {
                idx = 0;
            }

            fprintf(fp, "%llu,%d,%llu,%llu\n",
                s_CameraEvents[idx].timestamp,
                s_CameraEvents[idx].eCameraEvent,
                s_CameraEvents[idx].nPayload,
                s_CameraEvents[idx].nPayload2);

            idx++;
        }

        fclose(fp);

        /* flush and reset indices */
        s_nFirstCameraEvent = 0;
        s_nNextCameraEvent = 0;
    }

    CameraUnlockMutex(s_DeviceMutex);
}

#endif /* CAMERA_FEATURE_ENABLE_LOG_EVENT */


#if defined(CAMERA_FEATURE_ENABLE_TRACE_EVENT)

#ifdef _LINUX
uint32 g_trace_groupsEnable = 0xFF;

CAM_API void set_trace_groupsEnable(uint32 groupsEnable)
{
    g_trace_groupsEnable = groupsEnable;
}

CAM_API void ais_trace_snprintf(char* str, uint32 size, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsnprintf(str, size, fmt, args);
    va_end(args);
}

#endif

#endif /* CAMERA_FEATURE_ENABLE_TRACE_EVENT */

} // extern "C"




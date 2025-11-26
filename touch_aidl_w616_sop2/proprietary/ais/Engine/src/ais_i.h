#ifndef _AIS_I_H_
#define _AIS_I_H_

/* ===========================================================================
 * Copyright (c) 2016,2018-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @file ais_i.h
 * @brief AIS internal definition header
 *
=========================================================================== */

#include <string.h>
#include <list>
#include "AEEstd.h"
#include "CameraResult.h"
#include "CameraCommonTypes.h"
#include "CameraOSServices.h"
#include "CameraQueue.h"
#include "CameraConfig.h"
#include "ais_bufferlist.h"

#define AIS_MAX_WM_PER_OUTPUT 3
#define AIS_USER_CTXT_MAX_STREAMS MAX_CHANNEL_INPUT_SRCS
#define AIS_USER_CTXT_MAX_ISP_INSTANCES MAX_CHANNEL_ISP_INSTANCES

#define AIS_PPROC_MAX_BUFLISTS  3

#define AIS_FIELD_BUFFER_SIZE 6
#define MAX_CSI_STATUS_SIZE 16

#if defined(__ANDROID__)
#define AIS_DEFAULT_DUMP_LOCATION "/data/vendor/camera/"
#elif defined(__INTEGRITY)
#define AIS_DEFAULT_DUMP_LOCATION "/ghs_test/Camera/frameDump/"
#else
#define AIS_DEFAULT_DUMP_LOCATION "/tmp/"
#endif

class AisEngine;
class AisUsrCtxt;

typedef std::list<AisUsrCtxt*> AisUsrCtxtList;

/**
 * This structure describes the IFE resources
 * used for a specific IFE stream.
 */
typedef struct
{
    IfeCoreType       ifeCore;      /**<IFE core */
    IfeInterfaceType  ifeInterf;    /**< IFE interface */
    IfeOutputPathType ifeOutput;    /**< IFE output path */
    uint32            bufferListIdx; /**< idx of buffer list associated with this path */
} AisIfeStreamType;

/**
 * AIS CSI information required for CSI configuration
 */
typedef struct
{
    uint32 num_lanes;       /**< number of lanes for input */
    uint32 lane_mask;       /**< lane assignment for input */
    uint32 lane_assign;     /**< lane map assignment for input */
    uint32 settle_count;    /**< settle count for input */
    uint8  is_csi_3phase;   /**< is input 3 phase */
    uint8  is_secure;       /**< secure CSI */
    uint8  vcx_mode;        /**< VC extension enabled */
    uint32 sensor_num_frame_skip;  /**< skip frame numbers after start stream */
    uint8 forceHSmode;      /**< force csi enter HS mode */
    uint64 mipi_rate;       /**< mipi rate for input */
}AisInputCsiParamsType;

/**
 * AIS input mode information that is required by CSI/IFE
 */
typedef struct
{
    uint32 width;   /**< width of input mode */
    uint32 height;  /**< height of input mode */
    uint32 fps;     /**< fps of input mode */
    uint32 vc;      /**< vc of input mode */
    uint32 dt;      /**< dt of input mode */
    Camera_Rectangle crop_info; /**< crop info */
    qcarcam_color_fmt_t fmt;    /**< color format */
    CameraInterlacedType interlaced;  /**< interlaced or progressive input mode, and how to determine field type */
}AisInputModeInfoType;



/**
 * AIS Internal Event IDs
 */
typedef enum
{
    AIS_EVENT_RAW_FRAME_DONE = 0,
    AIS_EVENT_FRAME_DONE,
    AIS_EVENT_STATS_DONE,
    AIS_EVENT_SOF,
    AIS_EVENT_INPUT_STATUS,
    AIS_EVENT_INPUT_FATAL_ERROR,
    AIS_EVENT_INPUT_FRAME_FREEZE,
    AIS_EVENT_CSIPHY_WARNING,
    AIS_EVENT_CSID_WARNING,
    AIS_EVENT_CSID_FATAL_ERROR,
    AIS_EVENT_IFE_OUTPUT_WARNING,
    AIS_EVENT_IFE_OUTPUT_ERROR,
    AIS_EVENT_PPROC_JOB,
    AIS_EVENT_PPROC_JOB_DONE,
    AIS_EVENT_PPROC_JOB_FAIL,
    AIS_EVENT_APPLY_PARAM,
    AIS_EVENT_DEFER_INPUT_DETECT,
    AIS_EVENT_DELAYED_SUSPEND,
    AIS_EVENT_VENDOR,
    AIS_EVENT_USER_NOTIFICATION,
    AIS_EVENT_NUM
}AisEventId;

/**
 * AIS internal frame done event
 */
typedef struct
{
    IfeCoreType         ifeCore;
    IfeOutputPathType   ifeOutput;
    qcarcam_frame_info_v2_t frameInfo;
}AisEventFrameDoneType;

/**
 * AIS internal input status event
 */
typedef struct
{
    uint32 devId;  /*input device ID*/
    uint32 srcId;  /*input source ID*/
    qcarcam_input_signal_t status; /*signal status*/
}AisEventInputSignalType;

/**
 * AIS internal frame freeze event
 */
typedef struct
{
    uint32 devId;
    uint32 srcId;
    uint64 hStream;
    qcarcam_frame_freeze_t status;
}AisEventInputFrameFreezeType;

/**
 * AIS internal sof event
 */
typedef struct
{
    uint64  timestamp;
    uint64  hwTimestamp;
    IfeCoreType       ifeCore;
    IfeInterfaceType  ifeInput;
    uint32  frameId;
}AisEventSofType;

/**
 * AIS ife overflow event message
 */
typedef struct
{
    uint64 timestamp;
    IfeCoreType        ifeCore;
    IfeOutputPathType  ifeOutput;
    uint32 bufIdx;
    uint32 errorStatus;
}AisEventIfeErrorType;

/**
 * AIS internal postprocessing job event
 */
typedef struct
{
    uint64 jobId;

    AisUsrCtxt* pUsrCtxt;
    const struct AisProcChainType* pProcChain;
    uint32 currentHop;

    uint32 bufInIdx[AIS_PPROC_MAX_BUFLISTS];
    uint32 bufOutIdx[AIS_PPROC_MAX_BUFLISTS];

    CameraResult status;
    boolean      bDumpBuffers; /*dump buffers for this job*/

    uint32 streamIdx;
    qcarcam_frame_info_v2_t frameInfo;
    qcarcam_event_error_t error;
}AisEventPProcJobType;

typedef struct
{
    qcarcam_param_t param;
    qcarcam_param_value_t val;
    AisUsrCtxt* pUsrCtxt;
}AisEventApplyParamType;

typedef struct
{
    uint32 dev_id;
    uint32 src_id;
    qcarcam_vendor_param_t vendor_param;
}AisEventVendorType;

typedef struct
{
    qcarcam_param_t paramVal;
    AisUsrCtxt* pUsrCtxt;
}AisEventNotifyParamType;

typedef struct
{
    uint32 csiphyId;
    uint32 statusMsgSize;
    uint32 status[MAX_CSI_STATUS_SIZE];
}AisEventCsiStatusType;

typedef struct
{
    AisUsrCtxt* pUsrCtxt;
}AisEventNotifyRecoveryType;

/**
 * AIS internal event message
 */
typedef struct
{
    AisEventId eventId;

    union {
        AisEventFrameDoneType   ifeFrameDone;
        AisEventIfeErrorType    ifeError;
        AisEventInputSignalType inputStatus;
        AisEventInputFrameFreezeType inputFrameFreeze;
        AisEventSofType         sofInfo;
        AisEventPProcJobType    pprocJob;
        AisEventApplyParamType  applyParam;
        AisEventVendorType      vendorEvent;
        AisEventNotifyParamType notifyParam;
        AisEventCsiStatusType   csiStatus;
        AisEventNotifyRecoveryType notifyRecovery;
    }payload;
}AisEventMsgType;


#endif /* _AIS_I_H_ */


/**
 * @file ife_drv_api.h
 *
 * Copyright (c) 2010-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _IFE_DRV_API_H
#define _IFE_DRV_API_H

#include "CameraCommonTypes.h"

/**
*  @brief IFE driver command ID list
*/
extern const char ifeCommandIDtoString[][80];

typedef enum IFE_CMD_ID
{
    // X-Macro technique: redefining macros so that we can interpret the command ID's as identifiers or strings as needed.
    // This way the order of the strings in ifeCommandIDtoString will always be correct.
    #define CMD(x) x,
    #define CMD_CLONE(x,y) x = y,   // Defines a new command that has the same value (ID) as another command
    #include "ife_drv_cmds.h"
    #undef CMD
    #undef CMD_CLONE
    IFE_CMD_ID_MAX      // The last IFE command ID
}IFE_CMD_ID;

/**
*  @brief IFE output mode
*/
typedef enum
{
    IFE_OUTPUT_MODE_LINE_BASED = 0,
    IFE_OUTPUT_MODE_FRAME_BASED
}IfeOutputModeType;

/**
 * @brief MIPI supported Decode Type
 */
typedef enum
{
    IFE_DECODE_FMT_RAW6            = 0x0,
    IFE_DECODE_FMT_RAW8            = 0x1,
    IFE_DECODE_FMT_RAW10           = 0x2,
    IFE_DECODE_FMT_RAW12           = 0x3,
    IFE_DECODE_FMT_RAW14           = 0x4,
    IFE_DECODE_FMT_RAW16           = 0x5,
    IFE_DECODE_FMT_RAW20           = 0x6,
    IFE_DECODE_FMT_RAW             = 0xF
}IfeDecodeFmtType;

/**
 * @brief Pack type if decode is not RAW
 */
typedef enum
{
    IFE_PACK_TYPE_PLAIN8,
    IFE_PACK_TYPE_PLAIN16,
    IFE_PACK_TYPE_PLAIN32
}IfePackType;

/**
 * @brief CSI Info
 *
 * @csiphyId : CSI PHY core
 * @vc : CSI Virtual Channel
 * @dt : CSI Data Type
 * @dt_id : CSI Data Type ID
 * @numLanes : CSI number of lanes
 * @laneAssign : CSI lane mapping
 * @is3Phase : dphy(0) or cphy(1)
 * @vcxMode : CSI vc extension enabled(1) or disabled(0)
 */
typedef struct
{
    uint32 csiphyId;
    uint32 vc;
    uint32 dt;
    uint32 dt_id;
    uint32 numLanes;
    uint32 laneAssign;
    uint8  is3Phase;
    uint8  vcxMode;
    uint64 mipiRate;
}IfeCsiInfoType;

/**
 * @brief CSID Config flags
 */
typedef enum
{
    IFE_CSID_CONFIG_ENABLE_TIMESTAMP = 1 << 0,
    IFE_CSID_CONFIG_ENABLE_BYTE_CNTR = 1 << 1,
    IFE_CSID_CONFIG_ENABLE_CSID_TPG  = 1 << 2
}IfeCsidConfigEnableFlagsType;

/**
 * @brief CSID Config
 *
 * @param colorFmt: color format
 * @param decodeFmt: IfeDecodeFmtType
 * @param plainFm: IfePackType
 * @param width
 * @param height
 * @param isCropEnable: disabled(0) enabled(1)
 * @param cropTop: top line
 * @param cropBottom: bottom line
 * @param cropLeft: left pixel
 * @param cropRight: right pixel
 * @param flags: bitfield of IfeCsidConfigEnableFlagsType
 * @param init_frame_drop: number of frame to drop
 * @param isVerticalDropEnable: enables dropping on lines on frame
 * @param verticalDropPattern: pattern for line dropping on frame
 * @param verticalDropPeriod: period for verticalDropPattern
 * @param isHorizontalDropEnable: enables dropping select pixels on lines of frame
 * @param horizontalDropPattern: pattern for pixel dropping on lines
 * @param horizontalDropPeriod: period for horizontalDropPattern
 * @param isChromaDownscaleEnable: enables the plane 1 (uv) to subsample between two lines
 * @param isChromaConversionComponentSwap: swaps the uv plane pixels to create vu plane
 */
typedef struct
{
    uint32 colorFmt;
    uint32 decodeFmt;
    uint32 plainFmt;
    uint32 width;
    uint32 height;
    uint32 isCropEnable;
    uint32 cropTop;
    uint32 cropBottom;
    uint32 cropLeft;
    uint32 cropRight;
    uint32 initFrameDrop;
    uint32 flags;
    uint32 isVerticalDropEnable;
    uint32 verticalDropPattern;
    uint32 verticalDropPeriod;
    uint32 isHorizontalDropEnable;
    uint32 horizontalDropPattern;
    uint32 horizontalDropPeriod;
    uint32 isChromaDownscaleEnable;
    uint32 isChromaConversionComponentSwap;
}IfeCsidConfigType;

typedef enum
{
    IFE_BATCH_MODE_DEFAULT = 0,
}IfeBatchModeType;

/**
 * @brief Batch config
 *
 * @param batchMode   batch mode
 * @param numBatchFrames  number of frames in batch
 * @param frameIncrement  bytes from first pixel of frame N to N+1
 * @param detectFirstPhaseTimer  time gap to detect first phase of batch
 */
typedef struct
{
    IfeBatchModeType batchMode;
    uint32 numBatchFrames;
    uint32 frameIncrement;
    uint32 detectFirstPhaseTimer;
}IfeBatchConfigType;

/**
 * @brief Output Config
 *
 * @param colorFmt: color format
 * @param isSecureMode: secure mode disabled(0) enabled(1)
 * @param mode: IfeOutputModeType
 * @param width: width in bytes
 * @param height: number of lines
 * @param stride: stride in bytes
 * @param frameIncrement: in bytes
 * @param framedropPattern: Each bit of the framedrop identifies if a frame from
                      the bus master is kept (1) or dropped (0). The pattern
                      starts from bit 0 and progresses to bit 31.
 * @param framedropPeriod: This field indicates the period of framedrop pattern
                      for the bus master. A period of N is programmed as N-1.
 * @param flags: unused for now
 */
typedef struct
{
    uint32 colorFmt;
    uint32 isSecureMode;
    uint32 mode;
    uint32 width;
    uint32 height;
    uint32 stride;
    uint32 frameIncrement;

    uint32 framedropPattern;
    uint32 framedropPeriod;
    uint32 flags;
    IfeBatchConfigType batchConfig;
}IfeOutputConfigType;

/**
 * @brief Buffer Enqueue for output paths
 *
 * @outputPath : output path
 * @pBuffer : Image buffer
 * @offset : offset into image buffer
 * @pBufferHeader : Frame Header buffer
 */
typedef struct
{
    IfeOutputPathType    outputPath;
    AisBuffer*           pBuffer;
    uint32               bufferOffset;
    AisBuffer*           pBufferHeader;
}IfeCmdEnqOutputBuffer;

/// @brief IFE Messages
/**
 * @brief SOF Event Message
 *
 * @hwTimestamp : SOF HW timestamp
 * @frameId : frame count
 * */
typedef struct {
    uint64  hwTimestamp;
    uint32  frameId;
}IfeSofEventMsg;

/**
 * @brief Error Event Message
 *
 * @reserved : payload
 * */
typedef struct {
    uint32  reserved;
}IfeErrorEventMsg;


/**
 * @brief Frame Event Message
 *
 * @hwTimestamp : SOF HW timestamp per batched frame
 * @timestamp : SOF monotonic timestamp
 * @frameId : frame count per batched frame
 * @bufIdx : buffer idx that is complete
 * @numBatchFrames : number of batched frames
 * */
typedef struct {
    uint64  hwTimestamp[4];
    uint64  timestamp;
    uint32  frameId[4];
    uint32  bufIdx;
    uint32  numBatchFrames;
}IfeFrameEventMsg;

/**
 * @brief IFE Message Types
 */
typedef enum {
    IFE_MSG_ID_SOF,
    IFE_MSG_ID_FRAME_DONE,
    IFE_MSG_ID_OUPUT_WARNING,
    IFE_MSG_ID_OUPUT_ERROR,
    IFE_MSG_ID_CSID_WARNING,
    IFE_MSG_ID_CSID_ERROR
}IfeMsgIdType;

/**********************************************************
 ** IFE Driver Information Structures
 *********************************************************/
/**
 * @brief IFE HW Type
 */
typedef enum
{
    IFE_HW_FULL,
    IFE_HW_LITE
}IfeHwType;

/**
 * @brief IFE HW Capabilities
 *
 * @hwType : IfeHwType
 * @hwVersion : HW version
 * @numRdi : number of RDI paths
 * @numPix : number of PIX paths
 */
typedef struct
{
    IfeHwType hwType;
    uint32 hwVersion;
    uint32 numRdi;
    uint32 numPix;
    uint8 ifeCore;
}IfeHwCapabilitiesType;

/**
 * @brief RDI Config command
 *
 * @outputPath : Output path
 * @csiInfo : CSI Info
 * @inputConfig : Input Config
 * @outputConfig : Output Config
 */
typedef struct
{
    IfeOutputPathType   outputPath;
    IfeCsiInfoType      csiInfo;
    IfeCsidConfigType   inputConfig;
    IfeOutputConfigType outputConfig;
}IfeCmdRdiConfig;

/**
 * @brief AddrSync Config command
 *
 * @rdiStreams: bitmask of rdistreams in use
 */
typedef struct
{
    uint32 rdiStreams;
}IfeCmdAddrSyncConfig;

/**
 * @brief Start command
 *
 * @outputPath : Output path
 */
typedef struct
{
    IfeOutputPathType  outputPath;
}IfeCmdStart;

/**
 * @brief Stop command
 *
 * @outputPath : Output path
 */
typedef struct
{
    IfeOutputPathType  outputPath;
}IfeCmdStop;

/**
 * @brief Diag Info command
 *
 * @pktsRcvd : CSID packets received
 */
typedef struct
{
    uint32 pktsRcvd;
}IfeDiagInfo;

/**
 *  @brief Union of event specific payloads
 *         MUST NOT be larger that v4l2_event reserved array (64bytes)
 */
typedef union
{
    IfeSofEventMsg sofMsg;
    IfeErrorEventMsg errMsg;
    IfeFrameEventMsg frameMsg;
}IfeEventMsgUnionType;

/**
 * @brief IFE Event Message
 *
 * @timestamp : event monotonic timestamp
 * @type : IfeMsgIdType
 * @idx : IFE ID
 * @path : input/output path(s)
 * @u : event specific message
 */
typedef struct {
    uint64  timestamp;
    uint8   type;
    uint8   idx;
    uint8   path;

    IfeEventMsgUnionType u;
}IfeEventMsgType;

#endif   /* _IFE_DRV_API_H */

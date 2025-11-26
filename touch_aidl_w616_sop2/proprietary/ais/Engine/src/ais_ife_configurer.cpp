/*!
 * Copyright (c) 2016-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ais_ife_configurer.h"
#include "ife_drv_api.h"
#include "CameraEventLog.h"


//////////////////////////////////////////////////////////////////////////////////
/// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////////
///@brief AisIFEConfigurer singleton
AisIFEConfigurer* AisIFEConfigurer::m_pIfeConfigurerInstance = nullptr;

//////////////////////////////////////////////////////////////////////////////////
/// FORWARD DECLARE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////
static uint32 GetFmtComponentPP(qcarcam_color_fmt_t fmt);
static uint32 GetBytesPerLine(uint32 width, qcarcam_color_fmt_t fmt);

//////////////////////////////////////////////////////////////////////////////////
/// FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////
/**
 * GetFmtComponentPP
 *
 * @brief Helper function that returns number of channels components per pixel
 * for a qcarcam color format.
 *
 * @param fmt - qcarcam_color_fmt_t
 *
 * @return number of color channels per pixel
 */
static uint32 GetFmtComponentPP(qcarcam_color_fmt_t fmt)
{
    qcarcam_color_pattern_t pattern = (qcarcam_color_pattern_t)QCARCAM_COLOR_GET_PATTERN(fmt);
    uint32 component_pp = 1;

    switch(pattern)
    {
        case QCARCAM_RAW:
        case QCARCAM_BAYER_GBRG:
        case QCARCAM_BAYER_GRBG:
        case QCARCAM_BAYER_RGGB:
        case QCARCAM_BAYER_BGGR:
        case QCARCAM_YUV_NV12:
        case QCARCAM_YUV_NV21:
        case QCARCAM_YUV_YU12:
        case QCARCAM_YUV_YV12:
        {
            component_pp = 1;
            break;
        }
        case QCARCAM_YUV_YUYV:
        case QCARCAM_YUV_YVYU:
        case QCARCAM_YUV_UYVY:
        case QCARCAM_YUV_VYUY:
        {
            component_pp = 2;
            break;
        }
        // For RGB, treated as BPP
        case QCARCAM_RGB_RGB888:
        case QCARCAM_RGB_BGR888:
        {
            component_pp = 3;
            break;
        }
        case QCARCAM_RGB_RGB565:
        {
            component_pp = 2;
            break;
        }
        case QCARCAM_RGB_RGBX8888:
        case QCARCAM_RGB_BGRX8888:
        case QCARCAM_RGB_RGBX1010102:
        case QCARCAM_RGB_BGRX1010102:
        {
            component_pp = 4;
            break;
        }
        default:
            AIS_LOG(ENGINE, ERROR, "unsupported color format 0x%x use default 1 byte pp", fmt);
            break;
    }

    return component_pp;
}

/**
 * GetBytesPerLine
 *
 * @brief Number of bytes per line for a qcarcam color format and width.
 *
 * @param width
 * @param fmt - qcarcam_color_fmt_t
 *
 * @return number of bytes per line
 */
static uint32 GetBytesPerLine(uint32 width, qcarcam_color_fmt_t fmt)
{
    qcarcam_color_bitdepth_t bitdepth = (qcarcam_color_bitdepth_t)QCARCAM_COLOR_GET_BITDEPTH(fmt);
    qcarcam_color_pack_t pack = QCARCAM_COLOR_GET_PACK(fmt);

    uint32 bytesPerLine = 0;

    if (QCARCAM_PACK_MIPI == pack || QCARCAM_PACK_FOURCC == pack)
    {
        uint32 M, N;
        switch(bitdepth)
        {
        case QCARCAM_BITDEPTH_8:
        {
            M = 1;
            N = 1;
            break;
        }
        case QCARCAM_BITDEPTH_10:
        {
            M = 5;
            N = 4;
            break;
        }
        case QCARCAM_BITDEPTH_12:
        {
            M = 3;
            N = 2;
            break;
        }
        case QCARCAM_BITDEPTH_14:
        {
            M = 7;
            N = 4;
            break;
        }
        case QCARCAM_BITDEPTH_16:
        {
            M = 2;
            N = 1;
            break;
        }
        case QCARCAM_BITDEPTH_20:
        {
            M = 5;
            N = 2;
            break;
        }
        case QCARCAM_BITDEPTH_24:
        {
            M = 3;
            N = 1;
            break;
        }
        case QCARCAM_BITDEPTH_32:
        {
            M = 4;
            N = 1;
            break;
        }
        default:
            AIS_LOG(ENGINE, ERROR, "unsupported color format 0x%x use default MIPI8", fmt);
            M = 1;
            N = 1;
            break;
        }

        qcarcam_color_pattern_t pattern = QCARCAM_COLOR_GET_PATTERN(fmt);
        if (pattern >= QCARCAM_RGB)
        {
            return width * bitdepth / 8;
        }
        else
        {
            bytesPerLine = (width * GetFmtComponentPP(fmt) * M + (N-1)) / N;
        }
    }
    else if (QCARCAM_PACK_PLAIN8 == pack)
    {
        bytesPerLine = width * GetFmtComponentPP(fmt);
    }
    else if (QCARCAM_PACK_PLAIN16 == pack)
    {
        //2Bytes per pixel
        bytesPerLine = width * GetFmtComponentPP(fmt) * 2;
    }
    else if (QCARCAM_PACK_PLAIN32 == pack)
    {
        //4Bytes per pixel
        bytesPerLine = width * GetFmtComponentPP(fmt) * 4;
    }
    else
    {
        AIS_LOG(ENGINE, HIGH, "Unsupported color format 0x%x. Will treat as 1 bpp", fmt);
        bytesPerLine = width * GetFmtComponentPP(fmt);
    }

    return bytesPerLine;
}

/**
 * GetRepackParams
 *
 * @brief Get decodeFmt and Plain packing settings for repacking. Fills info into IfeCmdRdiConfig based on
 *        input and output color format.
 *
 * @param pRdiCfg
 *
 * @return CameraResult
 */
static CameraResult GetRepackParams(IfeCmdRdiConfig* pRdiCfg)
{
    CameraResult result = CAMERA_SUCCESS;
    IfeDecodeFmtType decodeFmt = IFE_DECODE_FMT_RAW;
    IfePackType      plainFmt = IFE_PACK_TYPE_PLAIN8;
    qcarcam_color_pack_t outPackType = QCARCAM_COLOR_GET_PACK(pRdiCfg->outputConfig.colorFmt);
    qcarcam_color_bitdepth_t bitDepth = QCARCAM_COLOR_GET_BITDEPTH(pRdiCfg->inputConfig.colorFmt);

    if (pRdiCfg->inputConfig.colorFmt != pRdiCfg->outputConfig.colorFmt)
    {
        qcarcam_color_pack_t inPackType = QCARCAM_COLOR_GET_PACK(pRdiCfg->inputConfig.colorFmt);

        /*Only repack if output is plain packed and input is mipi packed*/
        if (QCARCAM_PACK_MIPI == inPackType &&
            (QCARCAM_PACK_PLAIN8 == outPackType ||
             QCARCAM_PACK_PLAIN16 == outPackType ||
             QCARCAM_PACK_PLAIN32 == outPackType))
        {
            plainFmt = (IfePackType)(outPackType - QCARCAM_PACK_PLAIN8);

            switch(bitDepth)
            {
            case QCARCAM_BITDEPTH_8:
                decodeFmt = IFE_DECODE_FMT_RAW8;
                break;
            case QCARCAM_BITDEPTH_10:
                decodeFmt = IFE_DECODE_FMT_RAW10;
                break;
            case QCARCAM_BITDEPTH_12:
                decodeFmt = IFE_DECODE_FMT_RAW12;
                break;
            case QCARCAM_BITDEPTH_14:
                decodeFmt = IFE_DECODE_FMT_RAW14;
                break;
            case QCARCAM_BITDEPTH_16:
                decodeFmt = IFE_DECODE_FMT_RAW16;
                break;
            case QCARCAM_BITDEPTH_20:
                decodeFmt = IFE_DECODE_FMT_RAW20;
                break;
            default:
                AIS_LOG(ENGINE, ERROR, "Unsupported bitdepth %d for repacking", bitDepth);
                result = CAMERA_EBADPARM;
                break;
            }
        }
    }
    else if (pRdiCfg->inputConfig.isCropEnable)
    {
        qcarcam_color_pattern_t pattern = QCARCAM_COLOR_GET_PATTERN(pRdiCfg->inputConfig.colorFmt);

        //force decode to RAW8 and Plain pack if possible
        decodeFmt = IFE_DECODE_FMT_RAW8;

        if (pattern >= QCARCAM_RGB)
        {
            //treat RGB as RAW8
            plainFmt = IFE_PACK_TYPE_PLAIN8;
        }
        else
        {
            switch(bitDepth)
            {
            case QCARCAM_BITDEPTH_8:
                plainFmt = IFE_PACK_TYPE_PLAIN8;
                break;
            case QCARCAM_BITDEPTH_16:
                plainFmt = IFE_PACK_TYPE_PLAIN16;
                break;
            default:
                AIS_LOG(ENGINE, ERROR, "Unsupported bitdepth %d for crop only", bitDepth);
                result = CAMERA_EBADPARM;
                break;
            }
        }
    }

    pRdiCfg->inputConfig.decodeFmt = decodeFmt;
    pRdiCfg->inputConfig.plainFmt = plainFmt;

    return result;
}

static CameraResult GetYUVPlanarRepackParams(uint32 streamIdx, IfeCmdRdiConfig* pRdiCfg)
{
    CameraResult result = CAMERA_SUCCESS;
    IfeDecodeFmtType decodeFmt = IFE_DECODE_FMT_RAW8;
    IfePackType      plainFmt = IFE_PACK_TYPE_PLAIN8;
    qcarcam_color_pattern_t outputPattern = QCARCAM_COLOR_GET_PATTERN(pRdiCfg->outputConfig.colorFmt);
    qcarcam_color_pattern_t inputPattern = QCARCAM_COLOR_GET_PATTERN(pRdiCfg->inputConfig.colorFmt);

    if (QCARCAM_YUV_UYVY == inputPattern && QCARCAM_YUV_NV12 == outputPattern)
    {
        if (0 == streamIdx)
        {
            pRdiCfg->inputConfig.isHorizontalDropEnable = TRUE;
            pRdiCfg->inputConfig.horizontalDropPattern = 0x5555;
            pRdiCfg->inputConfig.horizontalDropPeriod = 0xF;
        }
        else if (1 == streamIdx)
        {
            pRdiCfg->inputConfig.isHorizontalDropEnable = TRUE;
            pRdiCfg->inputConfig.horizontalDropPattern = 0xAAAA;
            pRdiCfg->inputConfig.horizontalDropPeriod = 0xF;
            pRdiCfg->inputConfig.isChromaDownscaleEnable = TRUE;
            pRdiCfg->outputConfig.height /= 2;
        }
        pRdiCfg->inputConfig.decodeFmt = decodeFmt;
        pRdiCfg->inputConfig.plainFmt = plainFmt;
        return result;
    }
    else if (QCARCAM_YUV_UYVY == inputPattern && QCARCAM_YUV_NV21 == outputPattern)
    {
        if (0 == streamIdx)
        {
            pRdiCfg->inputConfig.isHorizontalDropEnable = TRUE;
            pRdiCfg->inputConfig.horizontalDropPattern = 0x5555;
            pRdiCfg->inputConfig.horizontalDropPeriod = 0xF;
        }
        else if (1 == streamIdx)
        {
            pRdiCfg->inputConfig.isHorizontalDropEnable = TRUE;
            pRdiCfg->inputConfig.horizontalDropPattern = 0xAAAA;
            pRdiCfg->inputConfig.horizontalDropPeriod = 0xF;
            pRdiCfg->inputConfig.isChromaDownscaleEnable = TRUE;
            pRdiCfg->inputConfig.isChromaConversionComponentSwap = TRUE;
            pRdiCfg->outputConfig.height /= 2;
        }
        pRdiCfg->inputConfig.decodeFmt = decodeFmt;
        pRdiCfg->inputConfig.plainFmt = plainFmt;
        return result;
    }
    else
    {
        return CAMERA_EBADPARM;
    }
}

/**
 * CalculateFramedropParam
 *
 * @brief get framedrop period and pattern values
 *
 * @param pFrameRateConfig - qcarcam_frame_rate_t
 *
 * @param pFramedropPeriod - output 32bit period value pointer
 * @param pFramedropPattern - output 32bit pattern value pointer
 */
static void CalculateFramedropParam(qcarcam_frame_rate_t *pFrameRateConfig,
        uint32 *pFramedropPeriod, uint32 *pFramedropPattern)
{
    switch (pFrameRateConfig->frame_drop_mode) {
        case QCARCAM_KEEP_EVERY_2FRAMES:
        case QCARCAM_KEEP_EVERY_3FRAMES:
        case QCARCAM_KEEP_EVERY_4FRAMES:
        {
            *pFramedropPeriod = pFrameRateConfig->frame_drop_mode;
            *pFramedropPattern = 1;
            break;
        }
        case QCARCAM_DROP_ALL_FRAMES:
        {
            *pFramedropPeriod = 0;
            *pFramedropPattern = 0;
            break;
        }
        case QCARCAM_FRAMEDROP_MANUAL:
        {
            *pFramedropPeriod = pFrameRateConfig->frame_drop_period;
            *pFramedropPattern = pFrameRateConfig->frame_drop_pattern;
            break;
        }
        case QCARCAM_KEEP_ALL_FRAMES:
        default:
        {
            *pFramedropPeriod = 0;
            *pFramedropPattern = 1;
            break;
        }
    }

    AIS_LOG(ENGINE, MED, "framedrop %d [%d %d]",
        pFrameRateConfig->frame_drop_mode,
        *pFramedropPeriod,
        *pFramedropPattern);
}


/**
 * AisIFEConfigurer::GetIfeCtxt
 *
 * @brief Gets the pointer to IFECoreCtxtType of the corresponding ifeCore
 */
IFECoreCtxtType* AisIFEConfigurer::GetIfeCtxt(uint8 ifeCore)
{
    if (m_IFEmappingTable.find(ifeCore) == m_IFEmappingTable.end())
        return NULL;
    else
        return m_IFEmappingTable.at(ifeCore);
}

/**
 * AisIFEConfigurer::GetIfeDiagInfo
 *
 * @brief Gets the csid pkts rcvd on csid HW
 *
 * @param ifeDevId ife/csid device id on which pkts count queried
 *
 * @param size of diagInfo needs to fill
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::GetIfeDiagInfo(uint32 ifeDevId, void* ifeDiagInfo,
        uint32 diagInfoSize)
{
    CameraResult rc = CAMERA_SUCCESS;

    if ((ifeDevId < m_nDevices) && (m_IFECoreCtxt[ifeDevId].refcnt != 0))
    {
        rc = CameraDeviceControl(m_IFECoreCtxt[ifeDevId].hIfeHandle, IFE_CMD_ID_DIAG_INFO,
                     NULL, 0, ifeDiagInfo, diagInfoSize, NULL);
    }

    return rc;
}

/**
 * AisIFEConfigurer::CreateInstance
 *
 * @brief Creates singleton instance for AisIFEConfigurer
 */
AisIFEConfigurer* AisIFEConfigurer::CreateInstance()
{
    if(m_pIfeConfigurerInstance == nullptr)
    {
        m_pIfeConfigurerInstance = new AisIFEConfigurer();
        if (m_pIfeConfigurerInstance)
        {
            CameraResult rc = m_pIfeConfigurerInstance->Init();
            if (rc)
            {
                DestroyInstance();
            }
        }
    }

    return m_pIfeConfigurerInstance;
}

/**
 * AisIFEConfigurer::GetInstance
 *
 * @brief Gets the singleton instance for AisIFEConfigurer
 */
AisIFEConfigurer* AisIFEConfigurer::GetInstance()
{
    return m_pIfeConfigurerInstance;
}

/**
 * AisCSIConfigurer::AisIFEConfigurer
 *
 * @brief Destroy the singleton instance of the AisIFEConfigurer class
 */
void AisIFEConfigurer::DestroyInstance()
{
    if(m_pIfeConfigurerInstance != nullptr)
    {
        m_pIfeConfigurerInstance->Deinit();

        delete m_pIfeConfigurerInstance;
        m_pIfeConfigurerInstance = nullptr;
    }
}

/**
 * IfeDeviceCallback
 *
 * @brief IFE Device Callback function
 *
 * @param pClientData
 * @param uidEvent
 * @param nEventDataLen
 * @param pEventData
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::IfeDeviceCallback(void* pClientData,
        uint32 uidEvent, int nEventDataLen, void* pEventData)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisIFEConfigurer* pIfeCtxt = (AisIFEConfigurer*)pClientData;

    AIS_LOG(ENGINE, DBG, "Received IFE %p callback %d", pIfeCtxt, uidEvent);

    IfeEventMsgType* pPayload = ((IfeEventMsgType*)pEventData);
    IfeCoreType ifeId;

    if (nEventDataLen != sizeof(IfeEventMsgType))
    {
        AIS_LOG(ENGINE, ERROR, "Received IFE %p callback %d with wrong size", pClientData, uidEvent);
        return CAMERA_EBADPARM;
    }
    else if (!pIfeCtxt || !pPayload)
    {
        AIS_LOG(ENGINE, ERROR, "Received IFE %p callback %d with no payload", pClientData, uidEvent);
        return CAMERA_EMEMPTR;
    }

    ifeId = (IfeCoreType)pPayload->idx;

    if (ifeId >= IFE_CORE_MAX)
    {
        AIS_LOG(ENGINE, ERROR, "IFE Core %d exceeds MAX %d", ifeId, IFE_CORE_MAX);
        return CAMERA_EBADPARM;
    }

    switch (uidEvent)
    {
    case IFE_MSG_ID_FRAME_DONE:
    {
        IfeFrameEventMsg* pRawMsg = &pPayload->u.frameMsg;
        AisEventMsgType msg = {};

        msg.eventId = AIS_EVENT_RAW_FRAME_DONE;

        msg.payload.ifeFrameDone.ifeCore = ifeId;
        msg.payload.ifeFrameDone.ifeOutput = (IfeOutputPathType)pPayload->path;

        msg.payload.ifeFrameDone.frameInfo.idx = pRawMsg->bufIdx;
        msg.payload.ifeFrameDone.frameInfo.seq_no[0] = pRawMsg->frameId[0];
        msg.payload.ifeFrameDone.frameInfo.timestamp = pRawMsg->timestamp;
        msg.payload.ifeFrameDone.frameInfo.sof_qtimestamp[0] =  pRawMsg->hwTimestamp[0];
        msg.payload.ifeFrameDone.frameInfo.timestamp_system = pPayload->timestamp;

        AIS_LOG(ENGINE, MED, "IFE %d:%d Frame Done %d (%u:%llu:%llu)",
                msg.payload.ifeFrameDone.ifeCore,
                msg.payload.ifeFrameDone.ifeOutput,
                msg.payload.ifeFrameDone.frameInfo.idx,
                msg.payload.ifeFrameDone.frameInfo.seq_no[0],
                msg.payload.ifeFrameDone.frameInfo.timestamp,
                msg.payload.ifeFrameDone.frameInfo.sof_qtimestamp[0]);

        if (pRawMsg->numBatchFrames > 1)
        {
            for(uint32 i = 1; i < pRawMsg->numBatchFrames; i++)
            {
                msg.payload.ifeFrameDone.frameInfo.seq_no[i] = pRawMsg->frameId[i];
                msg.payload.ifeFrameDone.frameInfo.sof_qtimestamp[i] = pRawMsg->hwTimestamp[i];

                AIS_LOG(ENGINE, DBG, "IFE %d:%d Frame Done %d (batch %d) (%u:%llu)",
                    msg.payload.ifeFrameDone.ifeCore,
                    msg.payload.ifeFrameDone.ifeOutput,
                    msg.payload.ifeFrameDone.frameInfo.idx,
                    i,
                    msg.payload.ifeFrameDone.frameInfo.seq_no[i],
                    msg.payload.ifeFrameDone.frameInfo.sof_qtimestamp[i]);
            }
        }

        AIS_TRACE_MESSAGE_F(AISTraceGroupIFE, "IFE %d:%d idx %d_%d fd",
            msg.payload.ifeFrameDone.ifeCore,
            msg.payload.ifeFrameDone.ifeOutput,
            msg.payload.ifeFrameDone.frameInfo.idx,
            msg.payload.ifeFrameDone.frameInfo.seq_no[0]);

        AisEngine::GetInstance()->QueueEvent(&msg);

        break;
    }
    case IFE_MSG_ID_SOF:
    {
        IfeSofEventMsg* pSofMsg = &pPayload->u.sofMsg;
        AisEventMsgType msg = {};

        msg.eventId = AIS_EVENT_SOF;

        msg.payload.sofInfo.ifeCore = ifeId;
        msg.payload.sofInfo.ifeInput = (IfeInterfaceType)pPayload->path;
        msg.payload.sofInfo.timestamp = pPayload->timestamp;
        msg.payload.sofInfo.hwTimestamp = pSofMsg->hwTimestamp;
        msg.payload.sofInfo.frameId = pSofMsg->frameId;

        AIS_LOG(ENGINE, MED, "IFE %d Input %d Target frame id %d TS %llu",
            msg.payload.sofInfo.ifeCore, msg.payload.sofInfo.ifeInput,
            msg.payload.sofInfo.frameId, msg.payload.sofInfo.hwTimestamp);

        AIS_TRACE_MESSAGE_F(AISTraceGroupIFE, "IFE %d:%d:%d SOF",
            msg.payload.ifeFrameDone.ifeCore,
            msg.payload.ifeFrameDone.ifeOutput,
            msg.payload.sofInfo.frameId);

        rc = AisEngine::GetInstance()->QueueEvent(&msg);

        break;
    }
    case IFE_MSG_ID_CSID_ERROR:
    {
        AisEventMsgType msg = {};

        msg.eventId = AIS_EVENT_CSID_FATAL_ERROR;

        msg.payload.ifeError.ifeCore = ifeId;
        msg.payload.ifeError.timestamp = pPayload->timestamp;
        msg.payload.ifeError.errorStatus = pPayload->u.errMsg.reserved;

        AIS_LOG(ENGINE, ERROR, "IFE %d Received CSID ERROR for CSI Status = 0x%x", ifeId, pPayload->u.errMsg.reserved);

        rc = AisEngine::GetInstance()->QueueEvent(&msg);

        break;
    }
    case IFE_MSG_ID_OUPUT_WARNING:
    {
        AisEventMsgType msg = {};

        msg.eventId = AIS_EVENT_IFE_OUTPUT_WARNING;

        msg.payload.ifeError.ifeCore = ifeId;
        msg.payload.ifeError.ifeOutput = (IfeOutputPathType)pPayload->path;
        msg.payload.ifeError.timestamp = pPayload->timestamp;
        msg.payload.ifeError.errorStatus = pPayload->u.errMsg.reserved;
        AisEngine::GetInstance()->UpdateDiagErrInfo((void*)&msg.payload.ifeError, AIS_EVENT_IFE_OUTPUT_WARNING, NULL);
        AIS_LOG(ENGINE, ERROR, "IFE %d Received Output%d WARNING %d", ifeId, pPayload->path, pPayload->u.errMsg.reserved);

        rc = AisEngine::GetInstance()->QueueEvent(&msg);

        break;
    }
    case IFE_MSG_ID_OUPUT_ERROR:
    {
        AisEventMsgType msg = {};

        msg.eventId = AIS_EVENT_IFE_OUTPUT_ERROR;

        msg.payload.ifeError.ifeCore = ifeId;
        msg.payload.ifeError.ifeOutput = (IfeOutputPathType)pPayload->path;
        msg.payload.ifeError.timestamp = pPayload->timestamp;
        msg.payload.ifeError.errorStatus = pPayload->u.errMsg.reserved;

        AIS_LOG(ENGINE, ERROR, "IFE %d Received Output%d ERROR", ifeId, pPayload->path);

        rc = AisEngine::GetInstance()->QueueEvent(&msg);

        break;
    }
    case IFE_MSG_ID_CSID_WARNING:
    {
        AisEventMsgType msg = {};

        msg.eventId = AIS_EVENT_CSID_WARNING;

        msg.payload.ifeError.ifeCore = ifeId;
        msg.payload.ifeError.timestamp = pPayload->timestamp;
        msg.payload.ifeError.errorStatus = pPayload->u.errMsg.reserved;
        AisEngine::GetInstance()->UpdateDiagErrInfo((void*)&msg.payload.ifeError, AIS_EVENT_CSID_WARNING, NULL);
        AIS_LOG(ENGINE, ERROR, "IFE %d Received Warning for RDI paths 0x%x ", ifeId, pPayload->path);
        break;
    }
    default:
        break;
    }

    return rc;
}

/**
 * AisIFEConfigurer::Init
 *
 * @brief IFE Configurer Init.
 *      Queries IFE devices and initializes them
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::Init(void)
{
    CameraResult rc = CAMERA_SUCCESS;
    CameraDeviceInfoType IfeDeviceInfo[IFE_CORE_MAX] = {};
    uint32 nIfeDeviceInfoLen;
    uint32 nDevice;

    // Get Multi SOC Environment
    m_isMultiSoc = CameraPlatform_GetMultiSocEnv();

    rc = mDeviceManagerContext->GetAvailableDevices(
        CAMERA_DEVICE_CATEGORY_IFE,
        IfeDeviceInfo,
        STD_ARRAY_SIZE(IfeDeviceInfo),
        &nIfeDeviceInfoLen);
    if (CAMERA_SUCCESS != rc)
    {
        AIS_LOG(ENGINE, ERROR, "Failed to get available ife devices");
        goto end;
    }

    if (nIfeDeviceInfoLen > IFE_CORE_MAX)
    {
        AIS_LOG(ENGINE, ERROR, "Queried more IFE (%d) than maximum (%d)",
                nIfeDeviceInfoLen, IFE_CORE_MAX);
        rc = CAMERA_ENEEDMORE;
        goto end;
    }

    m_nDevices = nIfeDeviceInfoLen;

    for (nDevice = 0; nDevice < m_nDevices && CAMERA_SUCCESS == rc; ++nDevice)
    {
        rc = mDeviceManagerContext->DeviceOpen(IfeDeviceInfo[nDevice].deviceID,
                        &m_IFECoreCtxt[nDevice].hIfeHandle);

        AIS_LOG(ENGINE, HIGH, "open ifeHandle %p", m_IFECoreCtxt[nDevice].hIfeHandle);
        AIS_LOG_ON_ERR(ENGINE, rc, "Open ife%d failed with rc %d ", nDevice, rc);

        if (CAMERA_SUCCESS == rc)
        {
            rc = CameraDeviceRegisterCallback(m_IFECoreCtxt[nDevice].hIfeHandle, IfeDeviceCallback, this);
            AIS_LOG_ON_ERR(ENGINE, rc, "Registercallback ife%d failed with rc %d ", nDevice, rc);
        }

        if (CAMERA_SUCCESS == rc)
        {
            IfeHwCapabilitiesType ifeHwCaps;

            rc = CameraDeviceControl(m_IFECoreCtxt[nDevice].hIfeHandle, IFE_CMD_ID_GET_HW_CAPABILITIES,
                NULL, 0, &ifeHwCaps, sizeof(ifeHwCaps), NULL);
            AIS_LOG_ON_ERR(ENGINE, rc, "QueryCaps ife%d failed with rc %d ", nDevice, rc);

            if (CAMERA_SUCCESS == rc)
            {
                m_IFECoreCtxt[nDevice].ifeCore = ifeHwCaps.ifeCore;
                m_IFECoreCtxt[nDevice].capsInfo.numRdi = ifeHwCaps.numRdi;
                m_IFECoreCtxt[nDevice].capsInfo.numPix = ifeHwCaps.numPix;
                m_IFEmappingTable.insert({m_IFECoreCtxt[nDevice].ifeCore, &m_IFECoreCtxt[nDevice]});
            }
        }

        if (CAMERA_SUCCESS == rc)
        {
            rc = CameraDeviceControl(m_IFECoreCtxt[nDevice].hIfeHandle, IFE_CMD_ID_POWER_ON,
                NULL, 0, NULL, 0, NULL);
            AIS_LOG_ON_ERR(ENGINE, rc, "PowerON ife%d failed with rc %d ", nDevice, rc);
        }

        if (CAMERA_SUCCESS == rc)
        {
            rc = CameraDeviceControl(m_IFECoreCtxt[nDevice].hIfeHandle, IFE_CMD_ID_RESET,
                    NULL, 0, NULL, 0, NULL);
            AIS_LOG_ON_ERR(ENGINE, rc, "Reset ife%d failed with rc %d ", nDevice, rc);
        }

        m_IFECoreCtxt[nDevice].refcnt = 0;
        m_IFECoreCtxt[nDevice].interfState = IFE_INTERF_INIT;
    }

end:
    return rc;
}

/**
 * AisIFEConfigurer::Deinit
 *
 * @brief IFE Configurer Deinit.
 *      Releases IFE devices
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::Deinit(void)
{
    uint32 nDevice;

    for (nDevice = 0; nDevice < m_nDevices; nDevice++)
    {
        if (m_IFECoreCtxt[nDevice].hIfeHandle)
        {
            (void)CameraDeviceControl(m_IFECoreCtxt[nDevice].hIfeHandle, IFE_CMD_ID_POWER_OFF,
                NULL,0, NULL, 0, NULL);

            (void)CameraDeviceClose(m_IFECoreCtxt[nDevice].hIfeHandle);

            AIS_LOG(ENGINE, HIGH, "close ifeHandle %p", m_IFECoreCtxt[nDevice].hIfeHandle);

            m_IFECoreCtxt[nDevice].hIfeHandle = NULL;
        }
    }

    return CAMERA_SUCCESS;
}

/**
 * AisIFEConfigurer::PowerSuspend
 *
 * @param AisUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::PowerSuspend(AisUsrCtxt* pUsrCtxt, boolean bGranular, CameraPowerEventType powerEventId)
{
    CAM_UNUSED(pUsrCtxt);
    CAM_UNUSED(bGranular);
    CAM_UNUSED(powerEventId);

    return CAMERA_SUCCESS;
}

/**
 * AisIFEConfigurer::PowerResume
 *
 * @param AisUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::PowerResume(AisUsrCtxt* pUsrCtxt, boolean bGranular, CameraPowerEventType powerEventId)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 nDevice;
    CAM_UNUSED(powerEventId);

    if (!bGranular)
    {
        for (nDevice = 0; nDevice < m_nDevices; ++nDevice)
        {
            rc = CameraDeviceControl(m_IFECoreCtxt[nDevice].hIfeHandle, IFE_CMD_ID_RESET,
                                     NULL, 0, NULL, 0, NULL);
            AIS_LOG_ON_ERR(ENGINE, rc, "Reset ife%d failed with rc %d ", nDevice, rc);

            if (rc != CAMERA_SUCCESS)
            {
                break;
            }
        }
    }
    else
    {
        uint32 streamIdx;

        if (NULL == pUsrCtxt)
        {
            AIS_LOG(ENGINE, ERROR, "input ctxt param is NULL");
            rc = CAMERA_EBADPARM;
            return rc;
        }

        for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == rc; streamIdx++)
        {
            AisIfeStreamType* pIfePath = &pUsrCtxt->m_streams[streamIdx].resources.ifeStream;
            IFECoreCtxtType* pIfeCtxt = GetIfeCtxt(pIfePath->ifeCore);

            if (!pIfeCtxt)
            {
                AIS_LOG(ENGINE, ERROR, "Failed to access invalid IFE:%d resource", pIfePath->ifeCore);
                return CAMERA_EFAILED;
            }

            AIS_LOG(ENGINE, HIGH, "refcnt %d", pIfeCtxt->refcnt);
            if (pIfeCtxt->refcnt == 0)
            {
                rc = CameraDeviceControl(pIfeCtxt->hIfeHandle, IFE_CMD_ID_RESET,
                        NULL, 0, NULL, 0, NULL);
                AIS_LOG_ON_ERR(ENGINE, rc, "Reset ife%d failed with rc %d ",pIfePath->ifeCore, rc);
            }
        }
    }

    return rc;
}

/**
 * AisIFEConfigurer::Config
 *
 * @param AisUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::Config(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 streamIdx;

    for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == rc; streamIdx++)
    {
        AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];

        if (pStream->type != AIS_STREAM_TYPE_IFE)
        {
            continue;
        }

        rc = ConfigStream(pUsrCtxt, pStream, streamIdx);
        if (CAMERA_SUCCESS != rc)
        {
            //clean up by stopping any configured streams
            while (streamIdx)
            {
                AisIfeStreamType* pIfePath;
                IfeCmdStop sStopCmd = {};

                streamIdx--;
                pStream = &pUsrCtxt->m_streams[streamIdx];
                pIfePath = &pStream->resources.ifeStream;
                sStopCmd.outputPath = pIfePath->ifeOutput;
                IFECoreCtxtType* pIfeCtxt = GetIfeCtxt(pIfePath->ifeCore);

                if (!pIfeCtxt)
                {
                    AIS_LOG(ENGINE, ERROR, "Failed to access invalid IFE:%d resource", pIfePath->ifeCore);
                    return CAMERA_EFAILED;
                }

                (void)CameraDeviceControl(pIfeCtxt->hIfeHandle, IFE_CMD_ID_STOP,
                        &sStopCmd, sizeof(sStopCmd), NULL, 0, NULL);
            }
            break;
        }
    }

    return rc;
}

/**
 * AisIFEConfigurer::ConfigStream
 *
 * @param AisUsrCtxt
 * @param AisIfeStreamType
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::ConfigStream(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream, uint32 streamIdx)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisIfeStreamType* pIfePath = &pStream->resources.ifeStream;
    IFECoreCtxtType* pIfeCtxt = GetIfeCtxt(pIfePath->ifeCore);

    if (!pIfeCtxt)
    {
        AIS_LOG(ENGINE, ERROR, "Failed to access invalid IFE:%d resource", pIfePath->ifeCore);
        return CAMERA_EFAILED;
    }

    if (pIfePath->bufferListIdx >= AIS_BUFLIST_MAX || !pUsrCtxt->m_bufferList[pIfePath->bufferListIdx])
    {
        AIS_LOG(ENGINE, ERROR, "Invalid buffer_index_list %d!", pIfePath->bufferListIdx);
        rc = CAMERA_EBADPARM;
    }
    else
    {
        AisInputModeInfoType* pModeInfo = &pStream->inputCfg.inputModeInfo;
        qcarcam_frame_rate_t *pFrameRateConfig = &pUsrCtxt->m_usrSettings.frame_rate_config;
        AisBufferList* pBufferList = pUsrCtxt->m_bufferList[pIfePath->bufferListIdx];

        /*
         * Must align output width to 16bytes (128bits) as that is HW requirement for RDI.
         * RDI will pad output if necessary. We will also check stride for this requirement.
         *
         * We use output bufferlist format as we may do repacking.
         */
        uint32 widthInBytes = CAM_ALIGN_SIZE(GetBytesPerLine(pModeInfo->width, pBufferList->GetColorFmt()), 16);

        AIS_LOG(ENGINE, HIGH, "Buffer fmt 0x%x width %d height %d stride %d minstride %d",
                pBufferList->GetColorFmt(),
                pModeInfo->width,
                pModeInfo->height,
                pBufferList->m_pBuffers[0].bufferInfo.planes[0].stride,
                widthInBytes);


        if (pBufferList->m_nBuffers)
        {
            if (pBufferList->GetWidth() < pModeInfo->width ||
                    pBufferList->GetHeight() < pModeInfo->height)
            {
                AIS_LOG(ENGINE, ERROR, "Buffer size doesn't match usecase %dx%d smaller than %dx%d",
                        pBufferList->GetWidth(), pBufferList->GetHeight(), pModeInfo->width, pModeInfo->height);
                rc = CAMERA_EBADPARM;
            }
            else if (pBufferList->m_pBuffers[0].bufferInfo.planes[0].stride < widthInBytes)
            {
                AIS_LOG(ENGINE, ERROR, "Buffer stride (%d bytes) too short (must be at least %d)",
                        pBufferList->m_pBuffers[0].bufferInfo.planes[0].stride,
                        widthInBytes);
                rc = CAMERA_EBADPARM;
            }
            else if (pBufferList->m_pBuffers[0].bufferInfo.planes[0].stride % 16)
            {
                AIS_LOG(ENGINE, ERROR, "Buffer stride (%d bytes) must be aligned to 16bytes (128bit)",
                        pBufferList->m_pBuffers[0].bufferInfo.planes[0].stride);
                rc = CAMERA_EBADPARM;
            }
        }

        if (CAMERA_SUCCESS == rc)
        {
            IfeCmdRdiConfig sRdiCfg = {};

            sRdiCfg.outputPath = pIfePath->ifeOutput;

            sRdiCfg.csiInfo.csiphyId = (CsiphyCoreType)pStream->resources.csiphy;
            sRdiCfg.csiInfo.vc = pModeInfo->vc;
            sRdiCfg.csiInfo.dt = pModeInfo->dt;
            sRdiCfg.csiInfo.dt_id = pStream->resources.cid & 0x3;
            sRdiCfg.csiInfo.numLanes = pStream->inputCfg.csiInfo.num_lanes;
            sRdiCfg.csiInfo.mipiRate = pStream->inputCfg.csiInfo.mipi_rate;
            sRdiCfg.csiInfo.laneAssign = pStream->inputCfg.csiInfo.lane_assign;
            sRdiCfg.csiInfo.is3Phase = pStream->inputCfg.csiInfo.is_csi_3phase;
            sRdiCfg.csiInfo.vcxMode = pStream->inputCfg.csiInfo.vcx_mode;

            //output configuration
#ifdef AIS_DEFAULT_FRAMEBASED
            sRdiCfg.outputConfig.mode = IFE_OUTPUT_MODE_FRAME_BASED;
#else
            /*Enable line based as default mode.*/
            sRdiCfg.outputConfig.mode = IFE_OUTPUT_MODE_LINE_BASED;
#endif
            sRdiCfg.outputConfig.width = widthInBytes;
            sRdiCfg.outputConfig.stride = pBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;
            sRdiCfg.outputConfig.height = pModeInfo->height;
            sRdiCfg.outputConfig.frameIncrement = sRdiCfg.outputConfig.stride * sRdiCfg.outputConfig.height;
            sRdiCfg.outputConfig.colorFmt = pBufferList->GetColorFmt();
            CalculateFramedropParam(pFrameRateConfig,
                    &sRdiCfg.outputConfig.framedropPeriod,
                    &sRdiCfg.outputConfig.framedropPattern);

            sRdiCfg.outputConfig.isSecureMode = pUsrCtxt->m_secureMode;
            sRdiCfg.outputConfig.batchConfig.batchMode = (IfeBatchModeType)pUsrCtxt->m_usrSettings.batch_config.batch_mode;
            sRdiCfg.outputConfig.batchConfig.numBatchFrames = pUsrCtxt->m_usrSettings.batch_config.num_batch_frames;
            sRdiCfg.outputConfig.batchConfig.detectFirstPhaseTimer = pUsrCtxt->m_usrSettings.batch_config.detect_first_phase_timer;

            if (pUsrCtxt->m_usrSettings.batch_config.frame_increment == 0)
            {
                sRdiCfg.outputConfig.batchConfig.frameIncrement= sRdiCfg.outputConfig.frameIncrement;
            }
            else if (pUsrCtxt->m_usrSettings.batch_config.frame_increment <=
                     pBufferList->m_pBuffers[0].bufferInfo.planes[0].size / pUsrCtxt->m_usrSettings.batch_config.num_batch_frames)
            {
                sRdiCfg.outputConfig.batchConfig.frameIncrement = pUsrCtxt->m_usrSettings.batch_config.frame_increment;
            }
            else
            {
                AIS_LOG(ENGINE, ERROR, "frameIncrement %d exceeds maximum buffer size", pUsrCtxt->m_usrSettings.batch_config.frame_increment);
                return CAMERA_EBADPARM;
            }

            //input configuration
            sRdiCfg.inputConfig.colorFmt = pModeInfo->fmt;

            sRdiCfg.inputConfig.flags = IFE_CSID_CONFIG_ENABLE_TIMESTAMP | IFE_CSID_CONFIG_ENABLE_BYTE_CNTR;
            if (QCARCAM_INPUT_TYPE_TESTPATTERN == pUsrCtxt->m_inputId)
            {
                sRdiCfg.inputConfig.flags |= IFE_CSID_CONFIG_ENABLE_CSID_TPG;
            }

            sRdiCfg.inputConfig.initFrameDrop =
                pStream->inputCfg.csiInfo.sensor_num_frame_skip > pUsrCtxt->m_usrSettings.init_frame_drop ?
                pStream->inputCfg.csiInfo.sensor_num_frame_skip : pUsrCtxt->m_usrSettings.init_frame_drop;

            if(pModeInfo->crop_info.right > 0 && pModeInfo->crop_info.bottom > 0)
            {
                sRdiCfg.inputConfig.isCropEnable = TRUE;
                sRdiCfg.inputConfig.cropLeft = pModeInfo->crop_info.left * GetFmtComponentPP(pModeInfo->fmt);
                sRdiCfg.inputConfig.cropTop = pModeInfo->crop_info.top;
                sRdiCfg.inputConfig.cropRight = (pModeInfo->crop_info.right + 1) * GetFmtComponentPP(pModeInfo->fmt) - 1;
                sRdiCfg.inputConfig.cropBottom = pModeInfo->crop_info.bottom;
                AIS_LOG(ENGINE, MED, "fill the crop info [%d,%d %dx%d]",
                        sRdiCfg.inputConfig.cropLeft, sRdiCfg.inputConfig.cropTop,
                        sRdiCfg.inputConfig.cropRight, sRdiCfg.inputConfig.cropBottom);
            }
            else
            {
                sRdiCfg.inputConfig.isCropEnable = FALSE;
                //Set the crop information to full resolution as the kernel driver may indirectly enable the cropper
                sRdiCfg.inputConfig.cropLeft = 0;
                sRdiCfg.inputConfig.cropTop = 0;
                sRdiCfg.inputConfig.cropRight = (pModeInfo->width + 1) * GetFmtComponentPP(pModeInfo->fmt) - 1;
                sRdiCfg.inputConfig.cropBottom = pModeInfo->height;
            }

            if (QCARCAM_OPMODE_RDI_CONVERSION == pUsrCtxt->m_opMode)
            {
                rc = GetYUVPlanarRepackParams(streamIdx,&sRdiCfg);
            }
            else
            {
                rc = GetRepackParams(&sRdiCfg);
            }
            if (CAMERA_SUCCESS != rc)
            {
                AIS_LOG(ENGINE, ERROR, "RDI%d Cannot repack based on configuration (%x %x)",
                        sRdiCfg.outputPath, sRdiCfg.inputConfig.colorFmt, sRdiCfg.outputConfig.colorFmt);
            }
            AIS_LOG(ENGINE, HIGH, "RDI_CONFIG 0x%x to 0x%x (%d, %d) batch:%d drop:%d",
                    sRdiCfg.inputConfig.colorFmt, sRdiCfg.outputConfig.colorFmt,
                    sRdiCfg.inputConfig.decodeFmt, sRdiCfg.inputConfig.plainFmt,
                    sRdiCfg.outputConfig.batchConfig.numBatchFrames,
                    sRdiCfg.inputConfig.initFrameDrop);

            rc = CameraDeviceControl(pIfeCtxt->hIfeHandle, IFE_CMD_ID_RDI_CONFIG,
                    &sRdiCfg, sizeof(sRdiCfg), NULL, 0, NULL);
            AIS_LOG_ON_ERR(ENGINE, rc, "Config ife%d failed with rc %d", pIfePath->ifeCore, rc);

            if (CAMERA_SUCCESS == rc && QCARCAM_OPMODE_RDI_CONVERSION == pUsrCtxt->m_opMode && 1 == streamIdx
                    && pIfePath->ifeCore == pUsrCtxt->m_streams[0].resources.ifeStream.ifeCore)
            {
                /*Same ife core for both streams */
                IfeCmdAddrSyncConfig addrSyncCfg = { (uint32) 1 << pUsrCtxt->m_streams[0].resources.ifeStream.ifeOutput | (uint32) 1 << pIfePath->ifeOutput};
                rc = CameraDeviceControl(pIfeCtxt->hIfeHandle, IFE_CMD_ID_ADDR_SYNC_CONFIG,
                    &addrSyncCfg, sizeof(addrSyncCfg), NULL, 0, NULL);
            }
            if (CAMERA_SUCCESS == rc)
            {
                pIfeCtxt->interfState = IFE_INTERF_CONFIG;
            }
        }
    }

    return rc;
}

/**
 * AisIFEConfigurer::GlobalConfig
 *
 * @brief Keep all devices at halt state. Used for Multi SOC feature.
 *
 * @param AisGlobalCtxt
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::GlobalConfig(AisGlobalCtxt* pGlobalCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;

    CAM_UNUSED(pGlobalCtxt);

    if (TRUE == m_isMultiSoc)
    {
        uint32 nDevice;

        for (nDevice = 0; nDevice < m_nDevices; nDevice++)
        {
            if (m_IFECoreCtxt[nDevice].hIfeHandle)
            {
                rc = CameraDeviceControl(m_IFECoreCtxt[nDevice].hIfeHandle, IFE_CMD_ID_GLOBAL_CSID_HALT,
                        NULL,0, NULL, 0, NULL);

                AIS_LOG_ON_ERR(ENGINE, rc, "Global Config ife%d failed with rc %d", nDevice, rc);
                if (!rc)
                {
                    m_IFECoreCtxt[nDevice].interfState = IFE_INTERF_CONFIG;
                }
            }
        }
    }

    return rc;
}

/**
 * AisIFEConfigurer::Start
 *
 * @param AisUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::Start(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 streamIdx;

    for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == rc; streamIdx++)
    {
        AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];

        if (pStream->type != AIS_STREAM_TYPE_IFE)
        {
            continue;
        }

        rc = StartStream(pUsrCtxt, pStream);
        if (CAMERA_SUCCESS != rc)
        {
            //clean up by stopping any configured streams
            while (streamIdx)
            {
                streamIdx--;
                pStream = &pUsrCtxt->m_streams[streamIdx];

                (void)StopStream(pUsrCtxt, pStream);
            }
            break;
        }
    }

    return rc;
}

/**
 * AisIFEConfigurer::StartStream
 *
 * @param AisUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::StartStream(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisIfeStreamType* pIfePath = &pStream->resources.ifeStream;
    IfeCmdStart sStartCmd = {};
    IFECoreCtxtType* pIfeCtxt = GetIfeCtxt(pIfePath->ifeCore);

    if (!pIfeCtxt)
    {
        AIS_LOG(ENGINE, ERROR, "Failed to access invalid IFE:%d resource", pIfePath->ifeCore);
        return CAMERA_EFAILED;
    }

    sStartCmd.outputPath = pIfePath->ifeOutput;

    rc = CameraDeviceControl(pIfeCtxt->hIfeHandle, IFE_CMD_ID_START,
            &sStartCmd, sizeof(sStartCmd), NULL, 0, NULL);
    AIS_LOG_ON_ERR(ENGINE, rc, "Start ife%d failed with rc %d ", 0, rc);

    if (CAMERA_SUCCESS == rc)
    {
        pIfeCtxt->refcnt++;
        pIfeCtxt->interfState = IFE_INTERF_STREAMING;
    }

    return rc;
}

/**
 * AisIFEConfigurer::Stop
 *
 * @param AisUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::Stop(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 streamIdx;

    for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams; streamIdx++)
    {
        CameraResult tmp_rc;
        AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];

        if (pStream->type != AIS_STREAM_TYPE_IFE)
        {
            continue;
        }

        CAMERA_LOG_FIRST_ERROR(StopStream(pUsrCtxt, pStream), rc, tmp_rc);
    }

    return rc;
}

/**
 * AisIFEConfigurer::StopStream
 *
 * @param AisUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::StopStream(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisIfeStreamType* pIfePath = &pStream->resources.ifeStream;
    IfeCmdStop sStopCmd = {};
    IFECoreCtxtType* pIfeCtxt = GetIfeCtxt(pIfePath->ifeCore);

    if (!pIfeCtxt)
    {
        AIS_LOG(ENGINE, ERROR, "Failed to access invalid IFE:%d resource", pIfePath->ifeCore);
        return CAMERA_EFAILED;
    }

    sStopCmd.outputPath = pIfePath->ifeOutput;

    rc = CameraDeviceControl(pIfeCtxt->hIfeHandle, IFE_CMD_ID_STOP,
            &sStopCmd, sizeof(sStopCmd), NULL, 0, NULL);
    AIS_LOG_ON_ERR(ENGINE, rc, "Stop ife%d failed with rc %d ", 0, rc);

    pIfeCtxt->refcnt--;
    pIfeCtxt->interfState = IFE_INTERF_INIT;

    return rc;
}

/**
 * AisIFEConfigurer::Resume
 *
 * @param AisUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::Resume(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 streamIdx;

    for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == rc; streamIdx++)
    {
        AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];

        if (pStream->type != AIS_STREAM_TYPE_IFE)
        {
            continue;
        }

        rc = ResumeStream(pUsrCtxt, pStream);
    }

    return rc;
}

/**
 * AisIFEConfigurer::ResumeStream
 *
 * @param AisUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::ResumeStream(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisIfeStreamType* pIfePath = &pStream->resources.ifeStream;
    IfeCmdStart sStartCmd = {};
    IFECoreCtxtType* pIfeCtxt = GetIfeCtxt(pIfePath->ifeCore);

    if (!pIfeCtxt)
    {
        AIS_LOG(ENGINE, ERROR, "Failed to access invalid IFE:%d resource", pIfePath->ifeCore);
        return CAMERA_EFAILED;
    }

    sStartCmd.outputPath = pIfePath->ifeOutput;

    rc = CameraDeviceControl(pIfeCtxt->hIfeHandle, IFE_CMD_ID_RESUME,
            &sStartCmd, sizeof(sStartCmd), NULL, 0, NULL);
    AIS_LOG_ON_ERR(ENGINE, rc, "Resume ife%d failed with rc %d ", 0, rc);

    return rc;
}

/**
 * AisIFEConfigurer::Pause
 *
 * @param AisUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::Pause(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 streamIdx;

    for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams; streamIdx++)
    {
        CameraResult tmp_rc;
        AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];

        if (pStream->type != AIS_STREAM_TYPE_IFE)
        {
            continue;
        }

        CAMERA_LOG_FIRST_ERROR(PauseStream(pUsrCtxt, pStream), rc, tmp_rc);
    }

    return rc;
}

/**
 * AisIFEConfigurer::PauseStream
 *
 * @param AisUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::PauseStream(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisIfeStreamType* pIfePath = &pStream->resources.ifeStream;
    IfeCmdStop sStopCmd = {};
    IFECoreCtxtType* pIfeCtxt = GetIfeCtxt(pIfePath->ifeCore);

    if (!pIfeCtxt)
    {
        AIS_LOG(ENGINE, ERROR, "Failed to access invalid IFE:%d resource", pIfePath->ifeCore);
        return CAMERA_EFAILED;
    }

    sStopCmd.outputPath = pIfePath->ifeOutput;

    rc = CameraDeviceControl(pIfeCtxt->hIfeHandle, IFE_CMD_ID_PAUSE,
            &sStopCmd, sizeof(sStopCmd), NULL, 0, NULL);
    AIS_LOG_ON_ERR(ENGINE, rc, "Pause ife%d failed with rc %d ", 0, rc);

    return rc;
}

/**
 * AisIFEConfigurer::SetParam
 *
 * @param AisUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::SetParam(AisUsrCtxt* pUsrCtxt, uint32 nCtrl, void *param)
{
    return CAMERA_EUNSUPPORTED;
}

/**
 * AisIFEConfigurer::GetParam
 *
 * @param AisUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::GetParam(AisUsrCtxt* pUsrCtxt, uint32 nCtrl, void *param)
{
    return CAMERA_EUNSUPPORTED;
}

/**
 * AisIFEConfigurer::QueueBuffers
 *
 * @param AisUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::QueueBuffers(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 streamIdx;

    for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == rc; streamIdx++)
    {
        AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];

        if (pStream->type != AIS_STREAM_TYPE_IFE)
        {
            continue;
        }

        rc = QueueStreamBuffers(pUsrCtxt, streamIdx);
    }

    return rc;
}

/**
 * AisIFEConfigurer::QueueStreamBuffers
 *
 * @param AisUsrCtxt
 * @param AisUsrCtxtStreamType
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::QueueStreamBuffers(AisUsrCtxt* pUsrCtxt, uint32 streamIdx)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];
    int nListIdx = pStream->resources.ifeStream.bufferListIdx;
    uint32 i=0;
    uint32 num = 0;

    if (nListIdx >= AIS_BUFLIST_MAX || !pUsrCtxt->m_bufferList[nListIdx])
    {
        AIS_LOG(ENGINE, ERROR, "Invalid buffer_index_list %d!", nListIdx);
        rc = CAMERA_EBADPARM;
    }
    else
    {
        AisBufferList* pBufferList = pUsrCtxt->m_bufferList[nListIdx];

        for (i=0; i < pBufferList->m_nBuffers; i++)
        {
            rc = QueueStreamBuffer(pUsrCtxt, streamIdx, pBufferList->GetBuffer(i));
            if (rc != CAMERA_SUCCESS)
            {
                break;
            }
            ++num;
        }

        if (num == 0)
        {
            AIS_LOG(ENGINE, ERROR, "Usrctxt %p : no avail buffer, please release", pUsrCtxt);
        }
    }

    return rc;
}

/**
 * AisIFEConfigurer::QueueBuffer
 *
 * @param AisUsrCtxt
 * @param buflistId
 * @param pBuffer
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::QueueBuffer(AisUsrCtxt* pUsrCtxt, AisBuflistIdType buflistId, AisBuffer* pBuffer)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisUsrCtxtStreamType* pStream = NULL;
    uint32 streamIdx;

    for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == rc; streamIdx++)
    {
        pStream = &pUsrCtxt->m_streams[streamIdx];
        if (pStream->resources.ifeStream.bufferListIdx == buflistId)
        {
            rc = QueueStreamBuffer(pUsrCtxt, streamIdx, pBuffer);
        }
    }

    return rc;
}

/**
 * AisIFEConfigurer::QueueStreamBuffer
 *
 * @param AisUsrCtxt
 * @param pStream
 * @param pBuffer
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::QueueStreamBuffer(AisUsrCtxt* pUsrCtxt, uint32 streamIdx, AisBuffer* pBuffer)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];
    AisIfeStreamType* pIfePath = &pStream->resources.ifeStream;
    IFECoreCtxtType* pIfeCtxt = GetIfeCtxt(pIfePath->ifeCore);

    if (!pIfeCtxt)
    {
        AIS_LOG(ENGINE, ERROR, "Failed to access invalid IFE:%d resource", pIfePath->ifeCore);
        return CAMERA_EFAILED;
    }
    IfeCmdEnqOutputBuffer sBufferEnqCmd = {};
    pIfePath = &pStream->resources.ifeStream;
    sBufferEnqCmd.outputPath = pIfePath->ifeOutput;
    sBufferEnqCmd.pBuffer = pBuffer;

    if (QCARCAM_OPMODE_RDI_CONVERSION == pUsrCtxt->m_opMode)
    {
        //changes buffer offset for multiple planes on 1 buffer
        sBufferEnqCmd.bufferOffset = pBuffer->bufferInfo.planes[streamIdx].offset;
    }

    #ifndef AIS_PAIRED_INPUT_GPU_SYNC
        // In the case of paired input use case where user context
        // has 2 streams and stream #1 should populate offset field by width of 1st stream.
        // This will tell IFE driver to configure proper address to HW.
        if ((QCARCAM_OPMODE_PAIRED_INPUT == pUsrCtxt->m_opMode) && (1 == streamIdx))
        {
            uint32 bytesPerLine = GetBytesPerLine(pUsrCtxt->m_streams[0].inputCfg.inputModeInfo.width,
                    pBuffer->colorFmt);
            if (bytesPerLine)
            {
                // In the case of paired input user buffer is divided into halves
                // so the offset is configured to be half of bytes per line.
                sBufferEnqCmd.bufferOffset = bytesPerLine;
            }
            else
            {
                AIS_LOG(ENGINE, ERROR,
                    "Usrctxt %p : failed to get bytesPerLine %d for width %d format %",
                    pUsrCtxt, pBuffer->bufferInfo.planes[0].width, pBuffer->colorFmt);
                return CAMERA_EFAILED;
            }
        }
    #endif
    rc = CameraDeviceControl(pIfeCtxt->hIfeHandle, IFE_CMD_ID_OUTPUT_BUFFER_ENQUEUE,
             &sBufferEnqCmd, sizeof(sBufferEnqCmd), NULL, 0, NULL);

    if (rc == CAMERA_SUCCESS)
    {
        pBuffer->state = AIS_IFE_OUTPUT_QUEUE;
    }
    AIS_LOG_ON_ERR(ENGINE, rc, "IFE_CMD_ID_OUTPUT_BUFFER_ENQUEUE failed ife%d rc %d", pIfePath->ifeCore, rc);

    return rc;
}

/**
 * AisIFEConfigurer::QueueInputBuffer
 *
 * @param AisUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisIFEConfigurer::QueueInputBuffer(AisUsrCtxt* pUsrCtxt,
    AisBufferList* pBufferList, uint32 nIdx)
{
    return CAMERA_SUCCESS;
}

/**
 * AisIFEConfigurer::GetIfeCapabilitiesInfo
 *
 * @param nIfeCore
 *
 * @return IFECapabilitiesType*   ptr to core's capabilities. NULL if invalid core.
 */
const IFECapabilitiesType* AisIFEConfigurer::GetIfeCapabilitiesInfo(IfeCoreType nIfeCore)
{
    const IFECapabilitiesType *ret;

    ret = (m_IFEmappingTable.find(nIfeCore) == m_IFEmappingTable.end()) ? NULL : &(m_IFEmappingTable.at(nIfeCore)->capsInfo);

    return ret;
}

boolean AisIFEConfigurer::IsIFEStarted(AisUsrCtxt* pUsrCtxt)
{
    boolean rc = FALSE;
    uint32 streamIdx;

    for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == rc; streamIdx++)
    {
        AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];

        AisIfeStreamType* pIfePath = &pStream->resources.ifeStream;
        IFECoreCtxtType* pIfeCtxt = GetIfeCtxt(pIfePath->ifeCore);

        if (!pIfeCtxt)
        {
            AIS_LOG(ENGINE, ERROR, "Failed to access invalid IFE:%d resource", pIfePath->ifeCore);
            return rc;
        }

        rc = pIfeCtxt->interfState == IFE_INTERF_STREAMING ? TRUE : FALSE;
        if (rc)
        {
            break;
        }
    }

    return rc;
}

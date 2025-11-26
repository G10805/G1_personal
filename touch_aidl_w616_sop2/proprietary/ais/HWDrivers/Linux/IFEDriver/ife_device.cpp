/**
 * @file ife_device.cpp
 *
 * IFE device driver interface.
 *
 * Copyright (c) 2009-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* ===========================================================================
                        INCLUDE FILES FOR MODULE
=========================================================================== */
#include "AEEStdDef.h"
#include "AEEstd.h"
#include <stdio.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include <stdlib.h>

#include "CameraPlatform.h"
#include "CameraPlatformLinux.h"
#include "CameraOSServices.h"
#include <media/ais_isp.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "ife_device.h"
#include "ife_drv_i.h"

/*-----------------------------------------------------------------------------
* Preprocessor Definitions and Constants
*---------------------------------------------------------------------------*/

/*===========================================================================
                 DEFINITIONS AND DECLARATIONS FOR MODULE
=========================================================================== */
/* ---------------------------------------------------------------------------
** Constant / Define Declarations
** ------------------------------------------------------------------------ */

/* ---------------------------------------------------------------------------
** Global Object Definitions
** -------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------
** Local Object Definitions
** ------------------------------------------------------------------------ */
typedef struct
{
    uint64  timestamp;
    uint32  frameId;
    uint8   type;
    uint8   idx;
    uint8   path;
    uint8   reserved;
} IfeEventCommonPayload;

/**
 * @brief SOF Event Kernel Message
 *
 * @hwTimestamp : SOF HW timestamp
 * */
typedef struct {
    uint64  hwTimestamp;
}IfeSofEventKernelMsg;

/**
 * @brief Frame Event Kernel Message
 *
 * @hwTimestamp : SOF HW timestamp per batched frame
 * @timestamp : SOF monotonic timestamp
 * @bufIdx : buffer idx that is complete
 * @numBatchFrames : number of batched frames
 * */
typedef struct {
    uint64  hwTimestamp[4];
    uint64  timestamp;
    uint32  bufIdx;
    uint32  numBatchFrames;
}IfeFrameEventKernelMsg;

/**
 *  @brief Union of event specific payloads
 */
typedef union
{
    IfeSofEventKernelMsg sof_msg;
    IfeErrorEventMsg err_msg;
    IfeFrameEventKernelMsg frame_msg;
}IfeEventKernelMsgUnionType;

/**
 * @brief IFE Event Message from linux kernel
 *
 * @msg : common payload for all IFE events
 * @u   : event specific message
 */
typedef struct {
    IfeEventCommonPayload msg;
    IfeEventKernelMsgUnionType u;
}IfeEventKernelMsgType;


/* ------------------------------------------------------------------------
** Forward Declarations
** ------------------------------------------------------------------------ */

/**************************************************************************
 ** Component Constructor/Destructor
 *************************************************************************/
IFEDevice::IFEDevice(uint8 ifeId, IfeHwType hwType)
{
    m_ifeId = ifeId;
    m_hwCaps.hwType = hwType;
    m_hwCaps.ifeCore = ifeId;

    m_v4l2EentThreadExit = FALSE;
    m_v4l2EentThread = NULL;

    m_pfnCallback = NULL;
    m_pClientData = NULL;

    memset(m_pipeFd, 0x0, sizeof(m_pipeFd));

    if (m_hwCaps.hwType == IFE_HW_LITE)
    {
        m_hwCaps.numRdi = 4;
        m_hwCaps.numPix = 0;
    }
    else
    {
        m_hwCaps.numRdi = 3;
        m_hwCaps.numPix = 1;
    }
}

int IFEDevice::IfeV4L2EventThread(void* arg)

{
    int pollStatus = -1;
    int exit_thread = 0;
    struct pollfd pollFds[2];
    int pollNumFds;
    CameraResult result = CAMERA_SUCCESS;
    IFEDevice* pIfeCtxt = (IFEDevice*)arg;

    pollFds[0].fd     = pIfeCtxt->m_pipeFd[0];
    pollFds[0].events = POLLIN|POLLRDNORM;
    pollFds[1].fd = pIfeCtxt->m_ifeKmdFd;
    pollFds[1].events = POLLIN|POLLRDNORM|POLLPRI;
    pollNumFds        = 2;

    while(!pIfeCtxt->m_v4l2EentThreadExit)
    {
        pollStatus = poll(pollFds, pollNumFds, -1);
        if(0 < pollStatus)
        {
            result = pIfeCtxt->ProcessV4L2Event();
        }
        else
        {
            AIS_LOG(IFE_DRV, ERROR, "IFE%d poll failed", pIfeCtxt->m_ifeId);
        }
    }
    return 0;
}

CameraResult IFEDevice::SendMessage(IfeEventMsgType* pIfeMsg)
{
    CameraResult result = CAMERA_SUCCESS;

    if (m_pfnCallback)
    {
        result = m_pfnCallback(m_pClientData, (uint32)(pIfeMsg->type), sizeof(*pIfeMsg), (void*)(pIfeMsg));
        if (result != CAMERA_SUCCESS)
        {
            AIS_LOG(IFE_DRV, ERROR, "VFE%d: Failed to send message %u", m_ifeId, pIfeMsg->type);
        }
    }
    else
    {
        AIS_LOG(IFE_DRV, ERROR, "VFE%d::SendMessage(%u) Null VFE callback function", m_ifeId, pIfeMsg->type);
        result = CAMERA_EMEMPTR;
    }

    return result;
}

CameraResult IFEDevice::ProcessV4L2Event()
{
    CameraResult result = CAMERA_SUCCESS;
    struct v4l2_event v4l2Event = {};
    int rc = -1;

    rc = ioctl(m_ifeKmdFd, VIDIOC_DQEVENT, &v4l2Event);
    if (rc >= 0)
    {
        if (V4L_EVENT_TYPE_AIS_IFE != v4l2Event.type)
        {
            AIS_LOG(IFE_DRV, ERROR, "IFE%d incorrect v4l2 type %d", m_ifeId, v4l2Event.type);
            result = CAMERA_EFAILED;
        }
        else if (v4l2Event.id != V4L_EVENT_ID_AIS_IFE)
        {
            AIS_LOG(IFE_DRV, ERROR, "IFE%d incorrect v4l2 ID %d", m_ifeId, v4l2Event.id);
            result =  CAMERA_EFAILED;
        }
        else
        {
            IfeEventKernelMsgType* pKernelMessage = reinterpret_cast<IfeEventKernelMsgType*>(v4l2Event.u.data);
            if (pKernelMessage->msg.reserved != sizeof(IfeEventKernelMsgType))
            {
                AIS_LOG(IFE_DRV, ERROR, "IFE event struct size mismatch(%d != %d)! Incompatible kernel version",
                        pKernelMessage->msg.reserved, sizeof(IfeEventKernelMsgType));
                result =  CAMERA_EFAILED;
            }
            else
            {
                AIS_LOG(IFE_DRV, LOW, "IFE%d Received Event %d %llu",
                        m_ifeId, pKernelMessage->msg.type, pKernelMessage->msg.timestamp);

                //convert kernel IFE event msg struct to UMD IFE event msg struct IfeEventMsgType
                IfeEventMsgType message = {};
                message.timestamp = pKernelMessage->msg.timestamp;
                message.type = pKernelMessage->msg.type;
                message.idx = pKernelMessage->msg.idx;
                message.path = pKernelMessage->msg.path;

                switch(message.type)
                {
                    case IFE_MSG_ID_SOF:
                    {
                        message.u.sofMsg.hwTimestamp = pKernelMessage->u.sof_msg.hwTimestamp;
                        message.u.sofMsg.frameId = pKernelMessage->msg.frameId;
                        AIS_LOG(IFE_DRV, LOW, "IFE%d RDI%d SOF %d",
                                m_ifeId, message.path, message.u.sofMsg.frameId);
                        result = SendMessage(&message);
                    }
                    break;

                    case IFE_MSG_ID_OUPUT_WARNING:
                    {
                        message.u.errMsg.reserved = pKernelMessage->u.err_msg.reserved;
                        AIS_LOG(IFE_DRV, WARN, "IFE%d RDI%d WARNING %d",
                                                    m_ifeId, message.path,
                                                    message.u.errMsg.reserved);
                        result = SendMessage(&message);
                    }
                    break;

                    case IFE_MSG_ID_OUPUT_ERROR:
                    case IFE_MSG_ID_CSID_ERROR:
                    {
                        message.u.errMsg.reserved = pKernelMessage->u.err_msg.reserved;
                        AIS_LOG(IFE_DRV, ERROR, "IFE%d Received error event [%u %u]",
                                m_ifeId, message.path, message.u.errMsg.reserved);
                        result = SendMessage(&message);
                    }
                    break;

                    case IFE_MSG_ID_FRAME_DONE:
                    {
                        for (int i = 0; i < pKernelMessage->u.frame_msg.numBatchFrames; i++)
                        {
                            message.u.frameMsg.hwTimestamp[i] = pKernelMessage->u.frame_msg.hwTimestamp[i];
                            message.u.frameMsg.frameId[i] = pKernelMessage->msg.frameId + i;
                        }
                        message.u.frameMsg.timestamp = pKernelMessage->u.frame_msg.timestamp;
                        message.u.frameMsg.bufIdx = pKernelMessage->u.frame_msg.bufIdx;
                        message.u.frameMsg.numBatchFrames = pKernelMessage->u.frame_msg.numBatchFrames;
                        AIS_LOG(IFE_DRV, LOW, "IFE%d RDI%d Frame Done %d buf %d [%llu]",
                                m_ifeId, message.path,message.u.frameMsg.frameId[0],
                                message.u.frameMsg.bufIdx, message.u.frameMsg.hwTimestamp[0]);
                        result = SendMessage(&message);
                    }
                    break;

                    default:
                        AIS_LOG(IFE_DRV, HIGH, "Received Unknown v4l2 event %d", message.type);
                        return CAMERA_EFAILED;
                }
            }
        }
    }
    else
    {
        AIS_LOG(IFE_DRV, ERROR, "IFE%d VIDIOC_DQEVENT failed", m_ifeId);
        result = CAMERA_EFAILED;
    }

    return result;
}

CameraResult IFEDevice::SubscribeEvents(uint32 type, uint32 id)
{
    CameraResult result = CAMERA_SUCCESS;
    struct v4l2_event_subscription sub = {};
    int rc = -1;

    sub.type = type;
    sub.id   = id;

    AIS_LOG(IFE_DRV, LOW, "Subscribe events id 0x%x type 0x%x",sub.id,  sub.type);

    rc = ioctl(m_ifeKmdFd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    if (0 != rc)
    {
        AIS_LOG(IFE_DRV, ERROR, "Subscribe event %d %d failed %d", type, id, rc);
        result = CAMERA_EFAILED;
    }

    return result;
}

CameraResult IFEDevice::SendIoctl(const char* name, uint32 opCode, uint32 size, void* handle)
{
    int rc = 0;

    struct cam_control ioctlCmd;
    ioctlCmd.op_code     = opCode;
    ioctlCmd.size        = size;
    ioctlCmd.handle_type = AIS_IFE_CMD_TYPE;
    ioctlCmd.reserved    = 0;
    ioctlCmd.handle      = (uint64)(handle);

    rc = ioctl(m_ifeKmdFd, VIDIOC_CAM_CONTROL, &ioctlCmd);
    if (rc < 0) {
        AIS_LOG(IFE_DRV, ERROR, "%s failed with rc %d", name, rc);
    }

    return rc ? CAMERA_EFAILED : CAMERA_SUCCESS;
}

CameraResult IFEDevice::Init()
{
    CameraResult result = CAMERA_SUCCESS;

    //Only 1 KMD IFE Manager
    m_ifeKmdFd = CameraPlatformGetFd(AIS_SUBDEV_IFE, m_ifeId);

    result = pipe(m_pipeFd);
    if (result < 0)
    {
        CAM_MSG(ERROR, "Failed to create ife pipe %d");
        result = CAMERA_EFAILED;
    }

    if (0 == m_ifeKmdFd)
    {
        AIS_LOG(IFE_DRV, ERROR,   "Failed to get the Camera AIS_SUBDEV_IFE");
        result = CAMERA_EFAILED;
    }
    else
    {
        //Create event thread that polls
        result = SubscribeEvents(V4L_EVENT_TYPE_AIS_IFE, V4L_EVENT_ID_AIS_IFE);
        if (CAMERA_SUCCESS == result)
        {
            char name[64];
            snprintf(name, sizeof(name), "ife%d_v4l2", m_ifeId);

            m_v4l2EentThreadExit = FALSE;
            result = CameraCreateThread(CAMERA_THREAD_PRIO_HIGH_REALTIME,
                    0, IFEDevice::IfeV4L2EventThread, this, 0x8000, name, &m_v4l2EentThread);
            if(CAMERA_SUCCESS != result)
            {
                AIS_LOG(IFE_DRV, ERROR, "Create IfeEvent thread failed for VFE");
            }
        }
    }

    return result;
}

CameraResult IFEDevice::DeInit()
{
    CameraResult result = CAMERA_SUCCESS;
    m_ifeKmdFd = 0;

    if (m_v4l2EentThread)
    {
        m_v4l2EentThreadExit = TRUE;
        write(m_pipeFd[1], &m_v4l2EentThreadExit, sizeof(m_v4l2EentThreadExit));
        CameraJoinThread(m_v4l2EentThread, NULL);
        CameraReleaseThread(m_v4l2EentThread);
        m_v4l2EentThread = NULL;
    }

    if (m_pipeFd[0])
        close(m_pipeFd[0]);
    if (m_pipeFd[1])
        close(m_pipeFd[1]);

    return result;
}

/**************************************************************************
 ** Ife Device Interface
 *************************************************************************/
/*===========================================================================
=========================================================================== */
CameraResult ife_device_open(CameraDeviceHandleType** ppDeviceHandle,
        CameraDeviceIDType deviceId)
{
    return IFEDevice::IfeDeviceOpen(ppDeviceHandle, deviceId, IFE_HW_FULL);
}

CameraResult ifelite_device_open(CameraDeviceHandleType** ppDeviceHandle,
        CameraDeviceIDType deviceId)
{
    return IFEDevice::IfeDeviceOpen(ppDeviceHandle, deviceId, IFE_HW_LITE);
}

CameraResult IFEDevice::IfeDeviceOpen(CameraDeviceHandleType** ppDeviceHandle,
        CameraDeviceIDType deviceId, IfeHwType hwType)
{
    CameraResult result = CAMERA_SUCCESS;

    if (NULL == ppDeviceHandle)
    {
        AIS_LOG(IFE_DRV, ERROR, "Invalid ptr passed in");
        return CAMERA_EMEMPTR;
    }

    uint8 ifeId = (deviceId & 0xF);

    IFEDevice* pIfeCtx = IFEOpen(ifeId, hwType);
    if (pIfeCtx)
    {
        /* no error detected */
        *ppDeviceHandle = static_cast<CameraDeviceHandleType*>(pIfeCtx);

        AIS_LOG(IFE_DRV, MED, "IFE%d: IFE device 0x%x opened!", ifeId, deviceId);
    }
    else
    {
        //If init is NOT successful, so tear down whatever has been created.
        AIS_LOG(IFE_DRV, ERROR, "IFE%d: IFE open 0x%x failed", ifeId, deviceId);

        *ppDeviceHandle = NULL;
        result = CAMERA_EFAILED;
    }

    return result;
}

IFEDevice* IFEDevice::IFEOpen(uint8 ifeId, IfeHwType hwType)
{
    CameraResult result = CAMERA_SUCCESS;

    IFEDevice* pIfeCtx = NULL;

    if (ifeId >= IFE_CORE_MAX)
    {
        result = CAMERA_EBADPARM;
    }
    else
    {
        //---------------------------------------------------------------------
        pIfeCtx = new IFEDevice(ifeId, hwType);
        if (pIfeCtx)
        {
            result = pIfeCtx->Init();

            /* This has to be is the last block */
            if (CAMERA_SUCCESS != result)
            {
                pIfeCtx->DeInit();
                delete pIfeCtx;
                pIfeCtx = NULL;
            }
        }
        else
        {
            AIS_LOG(IFE_DRV, ERROR, "IFE%d: Failed to create new IFEDevice", ifeId);
        }
    }

    return pIfeCtx;
}


/************************** IFE Wrapper ****************************\
* ife_device_register_callback()
*
* Description:  Register a client callback to ife device
*
* Parameters:
*               pDevice     - handle to Ife device
*               pfnCallback - function pointer for callback
*
* Returns:      CAMERA_SUCCESS
*               CAMERA_EMEMPTR
*
* Note:         Synchronous call.
*
\**************************************************************************/
CameraResult IFEDevice::RegisterCallback(CameraDeviceCallbackType pfnCallback, void *pClientData)
{
    CameraResult result = CAMERA_SUCCESS;

    //Pass the call back info to Sync Manager and Request Manager
    if (pfnCallback)
    {
        m_pfnCallback = pfnCallback;
        m_pClientData = pClientData;
    }

    return result;
}

/* ===========================================================================
=========================================================================== */
CameraResult IFEDevice::Close(void)
{
    CameraResult result = CAMERA_SUCCESS;

    result = DeInit();

    delete this;

    return result;
}

/* ===========================================================================
                     ife_device_control
=========================================================================== */
const char ifeCommandIDtoString[][80] =
{
    #define CMD(x) #x,      // Stringize parameter x
    #define CMD_CLONE(x,y)  // Don't stringize CMD_CLONE since its not a new value
    #include "ife_drv_cmds.h"
    #undef CMD
    #undef CMD_CLONE
    "IFE_CMD_ID_MAX"        // The last VFE command ID
};


CameraResult IFEDevice::Control(uint32 uidControl,
        const void* pIn, int nInLen,
        void* pOut, int nOutLen, int* pnOutLenReq)
{
    CameraResult result = CAMERA_SUCCESS;

    // pIn = NULL, nInLen !=0 ---> not OK
    // pIn = NULL, nInLen = 0 ---> OK
    // pIn !=NULL, nInLen !=0 ---> OK
    // pIn !=NULL, nInLen = 0 ---> not OK
    if (NULL == pIn)
    {
        if (nInLen > 0)
        {
            return CAMERA_EMEMPTR;
        }
    }

    if (NULL == pOut)
    {
        if (nOutLen > 0)
        {
            return CAMERA_EMEMPTR;
        }
    }

    if (NULL != pnOutLenReq)
    {
        *pnOutLenReq = 0;
    }


    AIS_LOG(IFE_DRV, LOW, "IFE%d: IFE Driver Processing %s command", m_ifeId,  ifeCommandIDtoString[uidControl]);

    switch (uidControl)
    {

        case IFE_CMD_ID_GET_HW_CAPABILITIES:
        {
            if (NULL != pnOutLenReq)
            {
                *pnOutLenReq = sizeof(IfeHwCapabilitiesType);
            }

            if (pOut)
            {
                IfeHwCapabilitiesType* pCaps = (IfeHwCapabilitiesType*)pOut;

                if (nOutLen != sizeof(IfeHwCapabilitiesType))
                {
                    return CAMERA_EBADPARM;
                }

                *pCaps = m_hwCaps;
            }
            break;
        }

        case IFE_CMD_ID_POWER_ON:
        {
            break;
        }

        case IFE_CMD_ID_POWER_OFF:
        {
            break;
        }
        case IFE_CMD_ID_RESET:
        {
            result = SendIoctl("reset", AIS_IFE_RESET, 0, NULL);
            break;
        }
        case IFE_CMD_ID_RDI_CONFIG:
        {
            if (nInLen != sizeof(IfeCmdRdiConfig))
            {
                return CAMERA_EBADPARM;
            }

            IfeCmdRdiConfig* pRdiCfg = (IfeCmdRdiConfig*)pIn;
            uint32 rdi_index = pRdiCfg->outputPath - IFE_OUTPUT_PATH_RDI0;

            if (rdi_index >= m_hwCaps.numRdi)
            {
                AIS_LOG(IFE_DRV, ERROR, "IFE%d: invalid rdi %d", m_ifeId, pRdiCfg->outputPath);
                result = CAMERA_EBADPARM;
            }
            else
            {
                result = SendIoctl("config", AIS_IFE_RESERVE, sizeof(*pRdiCfg), pRdiCfg);
            }

            break;
        }
        case IFE_CMD_ID_START:
        {
            if (nInLen != sizeof(IfeCmdStart))
            {
                return CAMERA_EBADPARM;
            }

            IfeCmdStart* pStartCmd = (IfeCmdStart*)pIn;

            AIS_LOG(IFE_DRV, HIGH, "IFE%d: IFE start rdi%d", m_ifeId,
                    pStartCmd->outputPath);

            result = SendIoctl("start", AIS_IFE_START, sizeof(*pStartCmd), pStartCmd);

            break;
        }
        case IFE_CMD_ID_STOP:
        {
            CameraResult tmpResult;

            if (nInLen != sizeof(IfeCmdStop))
            {
                return CAMERA_EBADPARM;
            }

            IfeCmdStop* pStopCmd = (IfeCmdStop*)pIn;

            AIS_LOG(IFE_DRV, HIGH, "IFE%d: IFE stop rdi%d", m_ifeId,
                    pStopCmd->outputPath);

            CAMERA_LOG_FIRST_ERROR(SendIoctl("stop", AIS_IFE_STOP, sizeof(*pStopCmd), pStopCmd), result, tmpResult);
            CAMERA_LOG_FIRST_ERROR(SendIoctl("release", AIS_IFE_RELEASE, sizeof(*pStopCmd), pStopCmd), result, tmpResult);

            break;
        }

    case IFE_CMD_ID_RESUME:
        {
            if (nInLen != sizeof(IfeCmdStart))
            {
                return CAMERA_EBADPARM;
            }

            IfeCmdStart* pStartCmd = (IfeCmdStart*)pIn;

            result = SendIoctl("resume", AIS_IFE_RESUME, sizeof(*pStartCmd), pStartCmd);

            break;
        }
        case IFE_CMD_ID_PAUSE:
        {
            if (nInLen != sizeof(IfeCmdStop))
            {
                return CAMERA_EBADPARM;
            }

            IfeCmdStop* pStopCmd = (IfeCmdStop*)pIn;

            result = SendIoctl("pause", AIS_IFE_PAUSE, sizeof(*pStopCmd), pStopCmd);

            break;
        }
        case IFE_CMD_ID_OUTPUT_BUFFER_ENQUEUE:
        {
            IfeBufferEnqueueType bufEnqCmd = {};

            if (nInLen != sizeof(IfeCmdEnqOutputBuffer))
            {
                return CAMERA_EBADPARM;
            }
            IfeCmdEnqOutputBuffer* pBuf = (IfeCmdEnqOutputBuffer*)pIn;

            bufEnqCmd.outputPath = pBuf->outputPath;
            bufEnqCmd.buffer.idx = pBuf->pBuffer->idx;
            bufEnqCmd.buffer.mem_handle = (int32_t)pBuf->pBuffer->pDa;
            bufEnqCmd.buffer.offset = pBuf->bufferOffset;

            result = SendIoctl("bufEnq", AIS_IFE_BUFFER_ENQ, sizeof(bufEnqCmd), &bufEnqCmd);

            break;
        }
        case IFE_CMD_ID_DIAG_INFO:
        {
            if (NULL != pnOutLenReq)
            {
                *pnOutLenReq = sizeof(IfeDiagInfo);
            }

            if (pOut)
            {
                IfeDiagInfo* pIfeDiagInfo = (IfeDiagInfo*)pOut;

                if (nOutLen != sizeof(IfeDiagInfo))
                {
                    return CAMERA_EBADPARM;
                }
                result = SendIoctl("diag", AIS_IFE_DIAG_INFO, sizeof(*pIfeDiagInfo ), pIfeDiagInfo);
            }

            break;
        }
        default:
        {
            AIS_LOG(IFE_DRV, ERROR, "IFE%d: Unexpected command ID %d", m_ifeId, uidControl);
            result = CAMERA_EUNSUPPORTED;
            break;
        }
    }

    return result;
}

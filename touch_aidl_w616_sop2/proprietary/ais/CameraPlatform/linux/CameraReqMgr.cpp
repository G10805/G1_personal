/**
 * @file CameraReqMgr.cpp
 *
 * Cam Req manager driver interface.
 *
 * Copyright (c) 2010-2020 Qualcomm Technologies, Inc.
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

#include "CameraPlatform.h"
#include "CameraPlatformLinux.h"
#include "CameraOSServices.h"
#include <media/cam_defs.h>
#include <media/cam_req_mgr.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "CameraReqMgr.h"

CamReqManager* CamReqManager::m_CamReqManagerInstance = nullptr;

/* ===========================================================================
=========================================================================== */
CameraResult CamReqManager::RegisterPowerCallback(CameraPowerEventCallable pfnCallback, void* pUsrData)
{
    CameraResult result = CAMERA_EFAILED;

    if (pfnCallback && pUsrData)
    {
        m_pfnPowerCallback = pfnCallback;
        m_pCallbackUsrData = pUsrData;
        result = CAMERA_SUCCESS;
    }
    else
    {
        CAM_MSG(ERROR, "PowerEvent callback pfnCallback pointer is NULL");
        result = CAMERA_EBADPARM;
    }

    return result;
}

CameraResult CamReqManager::SubscribeEvents(int fd, uint32 id, uint32 type)
{
    CameraResult result = CAMERA_SUCCESS;
    struct v4l2_event_subscription sub = {};
    int returnCode = -1;

    sub.id   = id;
    sub.type = type;

    CAM_MSG(MEDIUM, "Subscribe events id 0x%x type 0x%x",sub.id,  sub.type);

    returnCode = ioctl(fd, VIDIOC_SUBSCRIBE_EVENT, &sub);

    if (-1 == returnCode)
    {
        CAM_MSG(ERROR, "Subscribe events failed %d", returnCode);
        result = CAMERA_EFAILED;
    }
    return result;
}

int CamReqManager::CRMEventThreadProc(void* arg)
{
    int pollStatus = -1;
    struct pollfd pollFds[2];
    int pollNumFds;
    CamReqManager* ReqMgr = (CamReqManager*)arg;

    pollFds[0].fd     = ReqMgr->m_pipeFd[0];
    pollFds[0].events = POLLIN|POLLRDNORM;
    pollFds[1].fd     = ReqMgr->m_camReqMgrKmdFd;
    pollFds[1].events = POLLIN|POLLRDNORM|POLLPRI;
    pollNumFds        = 2;

    while(!ReqMgr->m_camReqMgrThreadExit)
    {
        pollStatus = poll(pollFds, pollNumFds, -1);
        if(0 < pollStatus)
        {
            if ((POLLIN == (pollFds[0].revents & POLLIN)) &&
                    (POLLRDNORM == (pollFds[0].revents & POLLRDNORM)))
            {
                CAM_MSG(HIGH, "Received exit msg");
            }
            if ((POLLPRI == (pollFds[1].revents & POLLPRI)) ||
                    (POLLIN == (pollFds[1].revents & POLLIN)) ||
                    (POLLRDNORM == (pollFds[1].revents & POLLRDNORM)))
            {
                ReqMgr->CRMProcessEvent();
            }
        }
        else
        {
            CAM_MSG(ERROR, "poll failed (%d)", pollStatus);
        }
    }

    return 0;
}

CameraResult CamReqManager::Init()
{
    CameraResult result = CAMERA_SUCCESS;

    m_camReqMgrKmdFd = CameraPlatformGetFd(AIS_SUBDEV_REQMGR, 0);
    if (0 == m_camReqMgrKmdFd)
    {
        CAM_MSG(ERROR, "Failed to get the Camera AIS_SUBDEV_AISMGR");
        result = CAMERA_EFAILED;
    }

    if (CAMERA_SUCCESS == result)
    {
        int rc = pipe(m_pipeFd);
        if (rc < 0)
        {
            CAM_MSG(ERROR, "Failed to create pipe %d");
            result = CAMERA_EFAILED;
        }
    }

    // Add subscribe events of event queue from KMD
    if (CAMERA_SUCCESS == result)
    {
        result = SubscribeEvents(m_camReqMgrKmdFd, V4L_EVENT_CAM_REQ_MGR_SOF,
            V4L_EVENT_CAM_REQ_MGR_EVENT);
        if (CAMERA_SUCCESS == result)
        {
            result = SubscribeEvents(m_camReqMgrKmdFd, V4L_EVENT_CAM_REQ_MGR_ERROR,
                V4L_EVENT_CAM_REQ_MGR_EVENT);
            if (CAMERA_SUCCESS != result)
            {
                CAM_MSG(ERROR, "Error event subscription failed");
                result = CAMERA_EFAILED;
            }
        }
        else
        {
            CAM_MSG(ERROR, "SOF Event subscription failed");
            result = CAMERA_EFAILED;
        }
    }

    if (CAMERA_SUCCESS == result)
    {
        result = SubscribeEvents(m_camReqMgrKmdFd, V4L_EVENT_CAM_REQ_MGR_S2R_SUSPEND,
                V4L_EVENT_CAM_REQ_MGR_EVENT);
        if (CAMERA_SUCCESS != result)
        {
            CAM_MSG(ERROR, "Error req_mgr event - S2R suspend subscription failed");
        }

        result = SubscribeEvents(m_camReqMgrKmdFd, V4L_EVENT_CAM_REQ_MGR_S2R_RESUME,
                V4L_EVENT_CAM_REQ_MGR_EVENT);
        if (CAMERA_SUCCESS != result)
        {
            CAM_MSG(ERROR, "Error req_mgr event - S2R resume subscription failed");
        }

        result = SubscribeEvents(m_camReqMgrKmdFd, V4L_EVENT_CAM_REQ_MGR_HIBERNATION_SUSPEND,
                V4L_EVENT_CAM_REQ_MGR_EVENT);
        if (CAMERA_SUCCESS != result)
        {
            CAM_MSG(ERROR, "Error req_mgr event - hibernation thaw subscription failed");
        }

        result = SubscribeEvents(m_camReqMgrKmdFd, V4L_EVENT_CAM_REQ_MGR_HIBERNATION_RESUME,
            V4L_EVENT_CAM_REQ_MGR_EVENT);
        if (CAMERA_SUCCESS != result)
        {
            CAM_MSG(ERROR, "Error req_mgr event - hibernation restore subscription failed");
        }
    }

    //Create event thread that polls
    if (CAMERA_SUCCESS == result)
    {
        char name[64];
        snprintf(name, sizeof(name), "cam_reqmgr_thread");

        result = CameraCreateThread(CAMERA_THREAD_PRIO_HIGH_REALTIME, 0,
                CamReqManager::CRMEventThreadProc, this, 0x8000, name, &m_camReqMgrThreadHandle);
        if(CAMERA_SUCCESS != result)
        {
            CAM_MSG(ERROR, "Create IfeEvent thread failed for VFE");
        }
    }
    return result;
}

CameraResult CamReqManager::CRMProcessEvent()
{
    CameraResult result = CAMERA_SUCCESS;
    struct v4l2_event v4l2_event = {};
    struct cam_req_mgr_message* pMessage = NULL;
    int rc = -1;

    rc = ioctl(m_camReqMgrKmdFd, VIDIOC_DQEVENT, &v4l2_event);
    if (rc >= 0)
    {
        if (V4L_EVENT_CAM_REQ_MGR_EVENT == v4l2_event.type)
        {
            CAM_MSG(MEDIUM, "Received an V4l2 event");
            pMessage = reinterpret_cast<struct cam_req_mgr_message*>(v4l2_event.u.data);

            if (NULL == pMessage)
            {
                CAM_MSG(MEDIUM, "Received an V4l2 event with enpty data!!");
                return CAMERA_EFAILED;
            }

            if (NULL == m_pfnPowerCallback)
            {
                CAM_MSG(MEDIUM, "not register power event callback!!");
                return CAMERA_EFAILED;
            }

            switch(v4l2_event.id)
            {
                case V4L_EVENT_CAM_REQ_MGR_S2R_SUSPEND:
                {
                    CAM_MSG(HIGH, "Received event V4L_EVENT_CAM_REQ_MGR_S2R_SUSPEND");
                    result = m_pfnPowerCallback(CAMERA_POWER_SUSPEND, m_pCallbackUsrData);
                    break;
                }

                case V4L_EVENT_CAM_REQ_MGR_S2R_RESUME:
                {
                    CAM_MSG(HIGH, "Received event V4L_EVENT_CAM_REQ_MGR_S2R_RESUME");
                    result = m_pfnPowerCallback(CAMERA_POWER_POST_LPM, m_pCallbackUsrData);
                    break;
                }

                case V4L_EVENT_CAM_REQ_MGR_HIBERNATION_SUSPEND:
                {
                    CAM_MSG(HIGH, "Received event V4L_EVENT_CAM_REQ_MGR_HIBERNATION_THAW");
                    result = m_pfnPowerCallback(CAMERA_POWER_DOWN, m_pCallbackUsrData);
                    break;
                }

                case V4L_EVENT_CAM_REQ_MGR_HIBERNATION_RESUME:
                {
                    CAM_MSG(HIGH, "Received event V4L_EVENT_CAM_REQ_MGR_HIBERNATION_RESTORE");
                    result = m_pfnPowerCallback(CAMERA_POWER_POST_HIBERNATION, m_pCallbackUsrData);
                    break;
                }

                default:
                    CAM_MSG(HIGH, "Received Unknown v4l2 event %d", v4l2_event.id);
                    result = CAMERA_EFAILED;
                    break;
            }
        }
        else
        {
            CAM_MSG(ERROR, "Received non v4l2 event = %d", v4l2_event.type);
            result = CAMERA_EFAILED;
        }
    }
    else
    {
        CAM_MSG(ERROR, "VIDIOC_DQEVENT failed on FD %d", m_camReqMgrKmdFd);
        result = CAMERA_EFAILED;
    }

    return result;
}

CameraResult CamReqManager::Ioctl2(uint32 opcode, void* pArg, uint32 hType, uint32 size)
{
    CameraResult       result = CAMERA_SUCCESS;
    struct cam_control ioctlCmd;
    int returnCode;

    CAM_UNUSED(hType);

    CAM_MSG(MEDIUM, "Entering Ioctl %d", opcode);

    ioctlCmd.op_code         = opcode;
    ioctlCmd.size            = size;
    ioctlCmd.handle_type     = CAM_HANDLE_USER_POINTER;
    ioctlCmd.reserved        = 0;
    ioctlCmd.handle          = VoidPtrToUINT64(pArg);

    returnCode = ioctl(m_camReqMgrKmdFd, VIDIOC_CAM_CONTROL, &ioctlCmd);
    if (0 != returnCode)
    {
        CAM_MSG(ERROR, "IFE: ioctl[%d] failed with return %d", opcode, returnCode);
        result = CAMERA_EFAILED;
    }
    else
    {
        CAM_MSG(MEDIUM, "Return Ioctl %d success!", opcode);
    }

    return result;
}

CamReqManager::CamReqManager()
{
    m_camReqMgrKmdFd = 0;
    m_camReqMgrThreadExit = FALSE;
    m_camReqMgrThreadHandle = NULL;
    memset(m_pipeFd, 0x0, sizeof(m_pipeFd));
}

CameraResult CamReqManager::DeInit()
{
    CameraResult result = CAMERA_SUCCESS;

    m_camReqMgrKmdFd = 0;

    if (m_camReqMgrThreadHandle)
    {
        m_camReqMgrThreadExit = TRUE;
        write(m_pipeFd[1], &m_camReqMgrThreadExit, sizeof(m_camReqMgrThreadExit));

        CameraJoinThread(m_camReqMgrThreadHandle, NULL);
        CameraReleaseThread(m_camReqMgrThreadHandle);
        m_camReqMgrThreadHandle = NULL;
    }

    if (m_pipeFd[0])
        close(m_pipeFd[0]);
    if (m_pipeFd[1])
        close(m_pipeFd[1]);

    return result;
}

CamReqManager* CamReqManager::CreateInstance()
{
    m_CamReqManagerInstance = new CamReqManager();
    if (m_CamReqManagerInstance)
    {
        if (m_CamReqManagerInstance->Init())
        {
            delete m_CamReqManagerInstance;
            m_CamReqManagerInstance = nullptr;
        }
    }

    return m_CamReqManagerInstance;
}

CamReqManager* CamReqManager::GetInstance()
{
    return m_CamReqManagerInstance;
}

void CamReqManager::DestroyInstance()
{
    if (m_CamReqManagerInstance != nullptr)
    {
        m_CamReqManagerInstance->DeInit();
        delete m_CamReqManagerInstance;
        m_CamReqManagerInstance = nullptr;
    }
}

/**
 * @file CameraReqMgr.h
 *
 * Copyright (c) 2010-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _CAMERA_REQ_MGR_H
#define _CAMERA_REQ_MGR_H

#include <map>

#include "CameraDevice.h"
#include "CameraOSServices.h"
#include "CameraQueue.h"

#include "ife_drv_api.h"
#include <media/cam_isp.h>

#define MMU_HANDLE_INVALID -1

/// @brief Handle to a device within the CAMSS that is exposed to the UMD
typedef int32 CamDeviceHandle;

/// @brief Type to hold addresses mapped to the camera HW.
typedef uint32 CSLDeviceAddress;
typedef int32 CSLMemHandle;
typedef int32 CSLFence;


static void* VoidPtrInc(
    void*    pVoid,
    size_t   numBytes)
{
    return (static_cast<char*>(pVoid) + numBytes);
}

static uint64 VoidPtrToUINT64(
    void* pVoid)
{
    return static_cast<uint64>(reinterpret_cast<size_t>(pVoid));
}

typedef struct
{
    CameraDeviceCallbackType pfnCallBack;
    void*                    pClientData;
}CamReqSessionPayload;

typedef std::map<int32, CamReqSessionPayload> CamReqSessionMap;

class CamReqManager
{
public:
    static CamReqManager* CreateInstance();
    static CamReqManager* GetInstance();
    static void DestroyInstance();

    CameraResult RegisterPowerCallback(CameraPowerEventCallable pfnCallback, void* pUsrData);
    CameraResult Ioctl2(uint32 opcode, void* pArg, uint32 hType, uint32 size);

private:
    CamReqManager();

    CameraResult Init();
    CameraResult DeInit();

    CameraResult SubscribeEvents(
        int fd,
        uint32 id,
        uint32 type);

    CameraResult CRMProcessEvent();

    static int CRMEventThreadProc(void* arg);


    static CamReqManager* m_CamReqManagerInstance;

    //The file descriptor of the camera request manager
    int m_camReqMgrKmdFd;

    //Callback for Camera Power Events
    CameraPowerEventCallable m_pfnPowerCallback;
    void* m_pCallbackUsrData;

    //V4L2 Polling thread
    int     m_pipeFd[2]; ///< CSLHw pipeFd  to interact with poll thread
    boolean m_camReqMgrThreadExit;
    CameraThread       m_camReqMgrThreadHandle;
};

#endif /* _CAMERA_REQ_MGR_H */

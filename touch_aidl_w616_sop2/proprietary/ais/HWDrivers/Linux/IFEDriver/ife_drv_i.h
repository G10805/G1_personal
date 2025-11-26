/**
 * @file ife_drv_i.h
 *
 * Copyright (c) 2010-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _IFE_DRV_I_H
#define _IFE_DRV_I_H

#include "CameraDevice.h"
#include "CameraOSServices.h"
#include "CameraQueue.h"
#include "CameraReqMgr.h"

#include "ife_drv_api.h"

#include <media/ais_isp.h>

/*-----------------------------------------------------------------------------
 * Type Declarations
 *---------------------------------------------------------------------------*/
/// @brief Buffer Definition
typedef struct {
    int32_t mem_handle;
    uint32_t idx;
    uint32_t offset;
}IfeBufferType;

/// @brief Buffer Enqueue
typedef struct
{
    IfeOutputPathType outputPath;
    IfeBufferType buffer;
    IfeBufferType bufferHeader;
}IfeBufferEnqueueType;

class IFEDevice : public CameraDeviceBase
{
public:
    /** Control. Camera drivers implement this method.
     * @see CameraDeviceControl
     */
    virtual CameraResult Control(uint32 uidControl,
            const void* pIn, int nInLen, void* pOut, int nOutLen, int* pnOutLenReq);

    /**
     * Close function pointer. Camera drivers implement this method.
     * @see CameraDeviceClose
     */
    virtual CameraResult Close(void);

    /**
     * RegisterCallback. Camera drivers implement this method.
     * @see CameraDeviceRegisterCallback
     */
    virtual CameraResult RegisterCallback(CameraDeviceCallbackType pfnCallback, void *pClientData);

    static CameraResult IfeDeviceOpen(CameraDeviceHandleType** ppDeviceHandle,
            CameraDeviceIDType deviceId, IfeHwType hwType);

private:
    static int IfeV4L2EventThread(void* arg);
    CameraResult ProcessV4L2Event();
    CameraResult SendMessage(IfeEventMsgType* ifeMsg);
    CameraResult SendIoctl(const char* name, uint32 opCode, uint32 size, void* handle);
    static IFEDevice* IFEOpen(uint8 ifeId, IfeHwType hwType);
    IFEDevice(uint8 ifeId, IfeHwType hwType);
    CameraResult Init();
    CameraResult DeInit();

    CameraResult SubscribeEvents(uint32 type, uint32 id);


    uint8 m_ifeId;
    IfeHwCapabilitiesType m_hwCaps;
    int m_ifeKmdFd = 0; //The file descriptor of the ife hw mgr node

    //V4L2 Polling thread
    int m_pipeFd[2]; // pipeFd to interact with poll thread

    boolean m_v4l2EentThreadExit;
    CameraThread m_v4l2EentThread;

    CameraDeviceCallbackType m_pfnCallback;
    void* m_pClientData;
};

#endif /* IFE_DRV_I_H */

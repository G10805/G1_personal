#ifndef _AIS_IFE_CONFIGURER_H_
#define _AIS_IFE_CONFIGURER_H_

/*!
 * Copyright (c) 2016-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ais_configurer.h"
#include "CameraDevice.h"
#include "CameraDeviceManager.h"
#include "CameraPlatform.h"
#include <map>

typedef struct
{
    uint32 numRdi;
    uint32 numPix;
}IFECapabilitiesType;

typedef enum
{
    IFE_INTERF_INIT = 0,
    IFE_INTERF_CONFIG,
    IFE_INTERF_STREAMING,
}IFEInterfaceState;

typedef struct
{
    CameraDeviceHandle hIfeHandle;
    IFECapabilitiesType capsInfo;

    IFEInterfaceState interfState;
    uint8 refcnt; //Track used rdi count
    uint8 ifeCore; //original IFE core id
}IFECoreCtxtType;

class AisIFEConfigurer : public AisEngineConfigurer
{
public:
    static AisIFEConfigurer* CreateInstance();
    static AisIFEConfigurer* GetInstance();
    static void       DestroyInstance();

    virtual CameraResult PowerSuspend(AisUsrCtxt* pUsrCtxt, boolean bGranular, CameraPowerEventType powerEventId);
    virtual CameraResult PowerResume(AisUsrCtxt* pUsrCtxt, boolean bGranular, CameraPowerEventType powerEventId);
    virtual CameraResult Config(AisUsrCtxt* pUsrCtxt);
    virtual CameraResult GlobalConfig(AisGlobalCtxt* pGlobalCtxt);
    virtual CameraResult Start(AisUsrCtxt* pUsrCtxt);
    virtual CameraResult Stop(AisUsrCtxt* pUsrCtxt);
    virtual CameraResult Resume(AisUsrCtxt* pUsrCtxt);
    virtual CameraResult Pause(AisUsrCtxt* pUsrCtxt);
    virtual CameraResult SetParam(AisUsrCtxt* pUsrCtxt, uint32 nCtrl, void *param);
    virtual CameraResult GetParam(AisUsrCtxt* pUsrCtxt, uint32 nCtrl, void *param);
    boolean IsIFEStarted(AisUsrCtxt* pUsrCtxt);
    /**
     * QueueBuffers
     *
     * @brief Queue user buffers to IFE
     *
     * @param pUsrCtxt
     *
     * @return CameraResult
     */
    CameraResult QueueBuffers(AisUsrCtxt* pUsrCtxt);

    /**
     * QueueBuffer
     *
     * @brief Queue user buffer to IFE
     *
     * @param pUsrCtxt
     * @param pBuffer
     *
     * @return CameraResult
     */
    CameraResult QueueBuffer(
            AisUsrCtxt* pUsrCtxt,
            AisBuflistIdType buflistId,
            AisBuffer* pBuffer);

    /**
     * QueueInputBuffer
     *
     * @brief Queue input user buffer to IFE
     *
     * @param pUsrCtxt
     * @param pBufferList
     * @param nIdx - index of buffer in buffer list
     *
     * @return CameraResult
     */
    CameraResult QueueInputBuffer(
            AisUsrCtxt* pUsrCtxt,
            AisBufferList* pBufferList,
            uint32 nIdx);

    /**
     * GetIfeCapabilitiesInfo
     *
     * @param nIfeCore
     *
     * @return IFECapabilitiesType*   ptr to core's capabilities. NULL if invalid core.
     */
    const IFECapabilitiesType* GetIfeCapabilitiesInfo(IfeCoreType nIfeCore);
    CameraResult GetIfeDiagInfo(uint32 ifeDevId, void* ifeDiagInfo, uint32 diagInfoSize);

private:
    static AisIFEConfigurer* m_pIfeConfigurerInstance;

    AisIFEConfigurer()
    {
        mDeviceManagerContext = CameraDeviceManager::GetInstance();

        m_nDevices = 0;
        m_isMultiSoc = FALSE;
        memset(m_IFECoreCtxt, 0x0, sizeof(m_IFECoreCtxt));
    }

    ~AisIFEConfigurer()
    {
        // Implement the destructor here
    }

    CameraResult Init(void);
    CameraResult Deinit(void);

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
    static CameraResult IfeDeviceCallback(void* pClientData,
            uint32 uidEvent, int nEventDataLen, void* pEventData);


    CameraResult ConfigStream(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream, uint32 streamIdx);
    CameraResult StartStream(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream);
    CameraResult StopStream(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream);
    CameraResult PauseStream(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream);
    CameraResult ResumeStream(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream);
    CameraResult QueueStreamBuffers(AisUsrCtxt* pUsrCtxt, uint32 streamIdx);
    CameraResult QueueStreamBuffer(AisUsrCtxt* pUsrCtxt, uint32 streamIdx, AisBuffer* pBuffer);
    IFECoreCtxtType* GetIfeCtxt(uint8 ifecore);

    CameraDeviceManager* mDeviceManagerContext;

    IFECoreCtxtType m_IFECoreCtxt[IFE_CORE_MAX];
    std::map <uint8, IFECoreCtxtType*> m_IFEmappingTable;  //holds mapping of ifeCore - pointer to corresponding IFECoreCtxtType
    uint32 m_nDevices;
    boolean m_isMultiSoc; /**< Multi SOC Envionment */
};


#endif /* _AIS_IFE_CONFIGURER_H_ */

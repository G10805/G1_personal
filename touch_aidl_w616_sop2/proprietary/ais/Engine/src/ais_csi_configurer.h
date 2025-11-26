#ifndef _AIS_CSI_CONFIGURER_H_
#define _AIS_CSI_CONFIGURER_H_

/*!
 * Copyright (c) 2016-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ais_configurer.h"
#include "CameraDevice.h"
#include "CameraDeviceManager.h"
#include "CameraPlatform.h"

//CSIPHY device state
typedef enum
{
    CSI_INTERF_INIT = 0,
    CSI_INTERF_CONFIG,
    CSI_INTERF_STREAMING,
}CSIInterfaceState;

//CSIPHY device context
typedef struct
{
    CameraDeviceHandle hCsiPhyHandle; //CameraDevice handle
    CSIInterfaceState eState; //state
    uint32 nRefCount;         //refcount to track number of streams in use
}CSICoreCtxtType;


class AisCSIConfigurer : public AisEngineConfigurer
{
public:
    static AisCSIConfigurer* CreateInstance();
    static AisCSIConfigurer* GetInstance();
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
    CameraResult GetCsiDiagInfo(uint32 csiId, void* diagInfo, uint32 diagSize);
    boolean IsCSIStarted(AisUsrCtxt* pUsrCtxt);

    CameraResult GlobalStart(void);
    CameraResult GlobalStop(void);

private:
    static AisCSIConfigurer* m_pCsiPhyConfigurerInstance;

    AisCSIConfigurer()
    {
        mDeviceManagerContext = CameraDeviceManager::GetInstance();

        memset(m_CSIPHYCoreCtxt, 0x0, sizeof(m_CSIPHYCoreCtxt));
        m_nDevices = 0;
        m_isMultiSoc = FALSE;
    }

    ~AisCSIConfigurer()
    {
        // Implement the destructor here
    }

    CameraResult Init(void);
    CameraResult Deinit(void);

    /**
     * CsiPhyDeviceCallback
     *
     * @brief CSIPHY Device Callback function
     *
     * @param pClientData
     * @param uidEvent
     * @param nEventDataLen
     * @param pEventData
     *
     * @return CameraResult
     */
    static CameraResult CsiPhyDeviceCallback(void* pClientData,
            uint32 uidEvent, int nEventDataLen, void* pEventData);

    /**
     * StopStream
     *
     * @brief Stop CSIPHY used by a stream
     *
     * @param AisUsrCtxtStreamType
     *
     * @return CameraResult
     */
    CameraResult StopStream(AisUsrCtxtStreamType* pStream);


    //handle to CameraDeviceManager
    CameraDeviceManager* mDeviceManagerContext;

    CSICoreCtxtType m_CSIPHYCoreCtxt[CSIPHY_CORE_MAX]; //contexts of all CSIPHY devices available
    uint32 m_nDevices; //number of CSIPHY devices available
    boolean m_isMultiSoc; /**< Multi SOC Envionment */
};


#endif /* _AIS_CSI_CONFIGURER_H_ */

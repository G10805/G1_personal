/*!
 * Copyright (c) 2016-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ais_csi_configurer.h"

#include "CameraMIPICSI2Types.h"

//////////////////////////////////////////////////////////////////////////////////
/// MACRO DEFINITIONS
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
/// TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
/// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////////
///@brief AisCSIConfigurer singleton
AisCSIConfigurer* AisCSIConfigurer::m_pCsiPhyConfigurerInstance = nullptr;

//////////////////////////////////////////////////////////////////////////////////
/// FORWARD DECLARE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
/// FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////

/**
 * AisCSIConfigurer::CreateInstance
 *
 * @brief Creates the singleton instance for AisCSIConfigurer
 */
AisCSIConfigurer* AisCSIConfigurer::CreateInstance()
{
    if(m_pCsiPhyConfigurerInstance == nullptr)
    {
        m_pCsiPhyConfigurerInstance = new AisCSIConfigurer();
        if (m_pCsiPhyConfigurerInstance)
        {
            CameraResult rc = m_pCsiPhyConfigurerInstance->Init();
            if (rc)
            {
                DestroyInstance();
            }
        }
    }

    return m_pCsiPhyConfigurerInstance;
}

/**
 * AisCSIConfigurer::GetInstance
 *
 * @brief Gets the singleton instance for AisCSIConfigurer
 */
AisCSIConfigurer* AisCSIConfigurer::GetInstance()
{
    return m_pCsiPhyConfigurerInstance;
}

/**
 * AisCSIConfigurer::DestroyInstance
 *
 * @brief Destroy the singleton instance of the AisCSIConfigurer class
 */
void AisCSIConfigurer::DestroyInstance()
{
    if(m_pCsiPhyConfigurerInstance != nullptr)
    {
        m_pCsiPhyConfigurerInstance->Deinit();

        delete m_pCsiPhyConfigurerInstance;
        m_pCsiPhyConfigurerInstance = nullptr;
    }
}

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
CameraResult AisCSIConfigurer::CsiPhyDeviceCallback(void* pClientData,
        uint32 uidEvent, int nEventDataLen, void* pEventData)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisCSIConfigurer* pCsiCtxt = (AisCSIConfigurer*)pClientData;

    AIS_LOG(ENGINE, MED, "Received CSIPHY callback %d", uidEvent);

    CsiEventMsgType* pPayload = ((CsiEventMsgType*)pEventData);

    if (nEventDataLen != sizeof(CsiEventMsgType))
    {
        AIS_LOG(ENGINE, ERROR, "Received CSIPHY %p callback %d with wrong size", pClientData, uidEvent);
        return CAMERA_EBADPARM;
    }
    else if (!pCsiCtxt || !pPayload)
    {
        AIS_LOG(ENGINE, ERROR, "Received CSIPHY %p callback %d with no payload", pClientData, uidEvent);
        return CAMERA_EMEMPTR;
    }

    CsiphyCoreType csiId = (CsiphyCoreType)pPayload->idx;

    if (csiId >= CSIPHY_CORE_MAX)
    {
        AIS_LOG(ENGINE, ERROR, "CSIPHY Core %d exceeds MAX %d", csiId, CSIPHY_CORE_MAX);
        return CAMERA_EBADPARM;
    }

    switch(uidEvent)
    {
        case MIPICSI_MSG_ID_WARNING:
        {
            AIS_LOG(ENGINE, HIGH, "Received CSIPHY%d WARNING", csiId);

            AisEventMsgType msg = {};
            msg.eventId = AIS_EVENT_CSIPHY_WARNING;
            msg.payload.csiStatus.csiphyId = csiId;
            memcpy(msg.payload.csiStatus.status, pPayload->statusMsg, sizeof(pPayload->statusMsg));
            msg.payload.csiStatus.statusMsgSize = sizeof(pPayload->statusMsg);
            AisEngine::GetInstance()->UpdateDiagErrInfo((void*)&msg.payload.csiStatus, AIS_EVENT_CSIPHY_WARNING, NULL);
            break;
        }
        default:
            break;
    }

    return rc;
}

/**
 * Init
 *
 * @brief Initialize the Configurer.
 *
 * @return CameraResult
 */
CameraResult AisCSIConfigurer::Init(void)
{
    CameraResult rc = CAMERA_SUCCESS;
    CameraDeviceInfoType cameraDeviceInfo[CSIPHY_CORE_MAX];
    uint32 nCameraDeviceInfoLenReq;
    uint32 nDevice;

    // Get Multi SOC Environment
    m_isMultiSoc = CameraPlatform_GetMultiSocEnv();

    //Get handle to all CSIPHYs
    memset(cameraDeviceInfo, 0, sizeof(cameraDeviceInfo));
    rc = mDeviceManagerContext->GetAvailableDevices(
        CAMERA_DEVICE_CATEGORY_CSIPHY,
        &cameraDeviceInfo[0],
        CSIPHY_CORE_MAX,
        &nCameraDeviceInfoLenReq);

    if (CAMERA_SUCCESS != rc)
    {
        AIS_LOG(ENGINE, ERROR, "Failed to get available csiphy devices");
        goto end;
    }
    else if (nCameraDeviceInfoLenReq > CSIPHY_CORE_MAX)
    {
        AIS_LOG(ENGINE, ERROR, "Queried more csiphy (%d) than maximum (%d)",
                nCameraDeviceInfoLenReq, CSIPHY_CORE_MAX);
        rc = CAMERA_ENEEDMORE;
        goto end;
    }

    m_nDevices = nCameraDeviceInfoLenReq;

    //Open, set callback and reset all CSIPHYs
    for (nDevice = 0; nDevice < m_nDevices && CAMERA_SUCCESS == rc; ++nDevice)
    {
        rc = mDeviceManagerContext->DeviceOpen(cameraDeviceInfo[nDevice].deviceID,
                        &m_CSIPHYCoreCtxt[nDevice].hCsiPhyHandle);
        AIS_LOG(ENGINE, HIGH, "open CsiPhyHandle %p", m_CSIPHYCoreCtxt[nDevice].hCsiPhyHandle);
        AIS_LOG_ON_ERR(ENGINE, rc, "Open CsiPhy%d failed with rc %d ", nDevice, rc);

        if (CAMERA_SUCCESS == rc)
        {
            rc = CameraDeviceRegisterCallback(m_CSIPHYCoreCtxt[nDevice].hCsiPhyHandle, CsiPhyDeviceCallback, this);
            AIS_LOG_ON_ERR(ENGINE, rc, "Registercallback CsiPhy%d failed with rc %d ", nDevice, rc);
        }

        if (CAMERA_SUCCESS == rc)
        {
            rc = CameraDeviceControl(m_CSIPHYCoreCtxt[nDevice].hCsiPhyHandle, CSIPHY_CMD_ID_RESET,
                    NULL, 0, NULL, 0, NULL);
            AIS_LOG_ON_ERR(ENGINE, rc, "PowerON CsiPhy%d failed with rc %d ", nDevice, rc);
        }
    }

end:
    return rc;
}

/**
 * Deinit
 *
 * @brief Deinitialize the Configurer. Needs to clean up and destroy instance.
 *
 * @return CameraResult
 */
CameraResult AisCSIConfigurer::Deinit(void)
{
    uint32 i;

    //Close all devices
    for (i = 0; i < m_nDevices; i++)
    {
        if (m_CSIPHYCoreCtxt[i].hCsiPhyHandle)
        {
            CameraDeviceClose(m_CSIPHYCoreCtxt[i].hCsiPhyHandle);
            m_CSIPHYCoreCtxt[i].hCsiPhyHandle = NULL;
        }
    }

    return CAMERA_SUCCESS;
}

CameraResult AisCSIConfigurer::PowerSuspend(AisUsrCtxt* pUsrCtxt, boolean bGranular, CameraPowerEventType powerEventId)
{
    CAM_UNUSED(pUsrCtxt);
    CAM_UNUSED(bGranular);
    CAM_UNUSED(powerEventId);

    return CAMERA_SUCCESS;
}

CameraResult AisCSIConfigurer::PowerResume(AisUsrCtxt* pUsrCtxt, boolean bGranular, CameraPowerEventType powerEventId)
{
    CAM_UNUSED(pUsrCtxt);
    CAM_UNUSED(bGranular);
    CAM_UNUSED(powerEventId);

    return CAMERA_SUCCESS;
}

/**
 * Config
 *
 * @brief Configure according to user context streams.
 *
 * @param AisUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisCSIConfigurer::Config(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;

    // Config is called only for single SOC environment
    if (FALSE == m_isMultiSoc)
    {
        uint32 streamIdx;

        for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams; streamIdx++)
        {
            AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];

            if (pStream->type != AIS_STREAM_TYPE_IFE)
            {
                continue;
            }

            if (CSI_INTERF_INIT == m_CSIPHYCoreCtxt[pStream->resources.csiphy].eState)
            {
                MIPICsiPhyConfig_t sMipiCsiCfg;
                AisInputCsiParamsType* csiParams = &pStream->inputCfg.csiInfo;

                memset(&sMipiCsiCfg, 0x0, sizeof(sMipiCsiCfg));
                sMipiCsiCfg.eMode = MIPICsiPhyModeDefault;

                sMipiCsiCfg.ePhase = csiParams->is_csi_3phase ? MIPICsiPhy_ThreePhase : MIPICsiPhy_TwoPhase;
                sMipiCsiCfg.nSettleCount = csiParams->settle_count;
                sMipiCsiCfg.nNumOfDataLanes = csiParams->num_lanes;
                sMipiCsiCfg.nLaneMask = csiParams->lane_mask;
                sMipiCsiCfg.nLaneAssign = csiParams->lane_assign;
                sMipiCsiCfg.forceHSmode = csiParams->forceHSmode;

                rc = CameraDeviceControl(m_CSIPHYCoreCtxt[pStream->resources.csiphy].hCsiPhyHandle, CSIPHY_CMD_ID_CONFIG,
                        &sMipiCsiCfg, sizeof(sMipiCsiCfg), NULL, 0, NULL);
                AIS_LOG_ON_ERR(ENGINE, rc, "Config CsiPhy%d with handler 0x%x failed with rc %d ", pStream->resources.csiphy,
                               m_CSIPHYCoreCtxt[pStream->resources.csiphy].hCsiPhyHandle, rc);
            }
        }
    }

    return rc;
}

/**
 * Global Config
 *
 * @brief Configure all devices. Used for Multi SOC feature.
 *
 * @param AisGlobalCtxt
 *
 * @return CameraResult
 */
CameraResult AisCSIConfigurer::GlobalConfig(AisGlobalCtxt* pGlobalCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (TRUE == m_isMultiSoc)
    {
        uint32 streamIdx, idx;

        for (idx = 0; idx < MAX_CAMERA_INPUT_CHANNELS && pGlobalCtxt->m_inputId[idx] != QCARCAM_INPUT_MAX; idx++)
        {
            for (streamIdx = 0; streamIdx < pGlobalCtxt->m_numStreams[idx]; streamIdx++)
            {
                AisUsrCtxtStreamType* pStream = &pGlobalCtxt->m_streams[idx][streamIdx];

                if (CSI_INTERF_INIT == m_CSIPHYCoreCtxt[pStream->resources.csiphy].eState)
                {
                    MIPICsiPhyConfig_t sMipiCsiCfg;
                    AisInputCsiParamsType* csiParams = &pStream->inputCfg.csiInfo;

                    memset(&sMipiCsiCfg, 0x0, sizeof(sMipiCsiCfg));
                    sMipiCsiCfg.eMode = MIPICsiPhyModeDefault;

                    sMipiCsiCfg.ePhase = csiParams->is_csi_3phase ? MIPICsiPhy_ThreePhase : MIPICsiPhy_TwoPhase;
                    sMipiCsiCfg.nSettleCount = csiParams->settle_count;
                    sMipiCsiCfg.nNumOfDataLanes = csiParams->num_lanes;
                    sMipiCsiCfg.nLaneMask = csiParams->lane_mask;
                    sMipiCsiCfg.nLaneAssign = csiParams->lane_assign;

                    rc = CameraDeviceControl(m_CSIPHYCoreCtxt[pStream->resources.csiphy].hCsiPhyHandle, CSIPHY_CMD_ID_CONFIG,
                            &sMipiCsiCfg, sizeof(sMipiCsiCfg), NULL, 0, NULL);
                    AIS_LOG_ON_ERR(ENGINE, rc, "Config CsiPhy%d with handler 0x%x failed with rc %d ",pStream->resources.csiphy,
                                   m_CSIPHYCoreCtxt[pStream->resources.csiphy].hCsiPhyHandle, rc);
                    if (CAMERA_SUCCESS == rc)
                    {
                        m_CSIPHYCoreCtxt[pStream->resources.csiphy].eState = CSI_INTERF_CONFIG;
                    }
                }
            }
        }
    }

    return rc;
}

/**
 * Start
 *
 * @brief Start user context streams.
 *
 * @param AisUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisCSIConfigurer::Start(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;

    // Start is called only for single SOC environment
    if (FALSE == m_isMultiSoc)
    {
        uint32 streamIdx;

        for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == rc; streamIdx++)
        {
            AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];

            if (pStream->type != AIS_STREAM_TYPE_IFE)
            {
                continue;
            }

            //start only if not already streaming
            if (CSI_INTERF_INIT == m_CSIPHYCoreCtxt[pStream->resources.csiphy].eState)
            {
                rc = CameraDeviceControl(m_CSIPHYCoreCtxt[pStream->resources.csiphy].hCsiPhyHandle, CSIPHY_CMD_ID_START,
                        NULL, 0, NULL, 0, NULL);
                AIS_LOG_ON_ERR(ENGINE, rc, "PowerON CsiPhy%d with handler 0x%x failed with rc %d ", pStream->resources.csiphy,
                               m_CSIPHYCoreCtxt[pStream->resources.csiphy].hCsiPhyHandle, rc);
            }

            //Set to streaming state and update refcount
            if (CAMERA_SUCCESS == rc)
            {
                m_CSIPHYCoreCtxt[pStream->resources.csiphy].eState = CSI_INTERF_STREAMING;
                m_CSIPHYCoreCtxt[pStream->resources.csiphy].nRefCount++;
            }
            else
            {
                //clean up by stopping any started streams
                while (streamIdx)
                {
                    streamIdx--;
                    pStream = &pUsrCtxt->m_streams[streamIdx];
                    (void)StopStream(pStream);
                }
                break;
            }
        }
    }

    return rc;
}

/**
 * Global Start
 *
 * @brief Start all devices. Used for Multi SOC feature.
 *
 * @return CameraResult
 */
CameraResult AisCSIConfigurer::GlobalStart(void)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (TRUE == m_isMultiSoc)
    {
        uint32 nDevice = 0;
        for (nDevice = 0; nDevice < m_nDevices; ++nDevice)
        {
            if (CSI_INTERF_CONFIG == m_CSIPHYCoreCtxt[nDevice].eState)
            {
                rc = CameraDeviceControl(m_CSIPHYCoreCtxt[nDevice].hCsiPhyHandle, CSIPHY_CMD_ID_START,
                        NULL, 0, NULL, 0, NULL);

                AIS_LOG_ON_ERR(ENGINE, rc, "Start CsiPhy%d failed with rc %d ", nDevice, rc);
                if (CAMERA_SUCCESS == rc)
                {
                    m_CSIPHYCoreCtxt[nDevice].eState = CSI_INTERF_STREAMING;
                }
            }
        }
    }

    return rc;
}

/**
 * Stop
 *
 * @brief Stop user context streams.
 *
 * @param AisUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisCSIConfigurer::Stop(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;

    // Stop is called only for single SOC environment
    if (FALSE == m_isMultiSoc)
    {
        uint32 streamIdx;

        //Stop all streams
        for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams; streamIdx++)
        {
            CameraResult tmp;
            AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];

            if (pStream->type != AIS_STREAM_TYPE_IFE)
            {
                continue;
            }

            CAMERA_LOG_FIRST_ERROR(StopStream(pStream), rc, tmp);
        }
    }

    return rc;
}

/**
 * Global Stop
 *
 * @brief Stop all devices. Used for Multi SOC feature.
 *
 * @return CameraResult
 */
CameraResult AisCSIConfigurer::GlobalStop(void)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (TRUE == m_isMultiSoc)
    {
        uint32 nDevice = 0;
	    
        for (nDevice = 0; nDevice < m_nDevices && CAMERA_SUCCESS == rc && CSI_INTERF_STREAMING == m_CSIPHYCoreCtxt[nDevice].eState; ++nDevice)
        {
            rc = CameraDeviceControl(m_CSIPHYCoreCtxt[nDevice].hCsiPhyHandle, CSIPHY_CMD_ID_STOP,
                    NULL, 0, NULL, 0, NULL);
	    
            AIS_LOG_ON_ERR(ENGINE, rc, "Stop CsiPhy%d failed with rc %d ", nDevice, rc);
            if (CAMERA_SUCCESS == rc)
            {
                m_CSIPHYCoreCtxt[nDevice].eState = CSI_INTERF_CONFIG;
            }
        }
    }

    return rc;
}

/**
 * StopStream
 *
 * @brief Stop CSIPHY used by a stream
 *
 * @param AisUsrCtxtStreamType
 *
 * @return CameraResult
 */
CameraResult AisCSIConfigurer::StopStream(AisUsrCtxtStreamType* pStream)
{
    CameraResult rc = CAMERA_SUCCESS;

    //Only stop if streaming and refcount reaches 0
    if (m_CSIPHYCoreCtxt[pStream->resources.csiphy].eState == CSI_INTERF_STREAMING)
    {
        m_CSIPHYCoreCtxt[pStream->resources.csiphy].nRefCount--;
        if (0 == m_CSIPHYCoreCtxt[pStream->resources.csiphy].nRefCount)
        {
            rc = CameraDeviceControl(m_CSIPHYCoreCtxt[pStream->resources.csiphy].hCsiPhyHandle, CSIPHY_CMD_ID_STOP,
                    NULL, 0, NULL, 0, NULL);
            AIS_LOG_ON_ERR(ENGINE, rc, "PowerON CsiPhy%d with handler 0x%x failed with rc %d ", pStream->resources.csiphy,
                           m_CSIPHYCoreCtxt[pStream->resources.csiphy].hCsiPhyHandle, rc);
            m_CSIPHYCoreCtxt[pStream->resources.csiphy].eState = CSI_INTERF_INIT;
        }

    }
    else
    {
        rc = CAMERA_EBADSTATE;
    }

    return rc;
}

CameraResult AisCSIConfigurer::Resume(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;

    return rc;
}

CameraResult AisCSIConfigurer::Pause(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;

    return rc;
}


CameraResult AisCSIConfigurer::SetParam(AisUsrCtxt* pUsrCtxt, uint32 nCtrl, void *pParam)
{
    return CAMERA_SUCCESS;
}

CameraResult AisCSIConfigurer::GetParam(AisUsrCtxt* pUsrCtxt, uint32 nCtrl, void *pParam)
{
    return CAMERA_SUCCESS;
}

CameraResult AisCSIConfigurer::GetCsiDiagInfo(uint32 csiId, void* diagInfo, uint32 diagSize)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (csiId >= m_nDevices)
    {
        rc = CAMERA_EOUTOFBOUND;
    }
    else if (m_CSIPHYCoreCtxt[csiId].eState == CSI_INTERF_STREAMING)
    {
        //Only get the status if CSIPHY state is streaming

        rc = CameraDeviceControl(m_CSIPHYCoreCtxt[csiId].hCsiPhyHandle, CSIPHY_CMD_ID_DIAG_INFO,
                        NULL, 0, diagInfo, diagSize, NULL);
        AIS_LOG_ON_ERR(ENGINE, rc, "Getting CSI PHY status failed:%d ",rc);		
    }
    else
    {
        AIS_LOG(ENGINE, LOW, "CSIPHY is in invalid state");
        rc = CAMERA_EBADSTATE;
    }
    return rc;
}

boolean AisCSIConfigurer::IsCSIStarted(AisUsrCtxt* pUsrCtxt)
{
    boolean rc = FALSE;
    uint32 streamIdx;

    for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == rc; streamIdx++)
    {
        AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];
        CameraCsiInfo csiInfo = {};

        CameraPlatformGetCsiIfeMap(pStream->resources.csiphy, &csiInfo);

        if (csiInfo.numIfeMap == 1)
        {
            AIS_LOG(ENGINE, LOW, "CsiPhy%d just map to 1 ife, no need further check");
            return rc;
        }

        if (CSI_INTERF_STREAMING == m_CSIPHYCoreCtxt[pStream->resources.csiphy].eState)
        {
            AIS_LOG(ENGINE, MED, "CsiPhy already started");
            rc = TRUE;
            break;
        }
    }

    return rc;
}

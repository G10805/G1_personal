/*!
 * Copyright (c) 2016-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ais_input_configurer.h"

/*
* timeout value of delay suspend in ms
* suspend only could be executed in case of timeout.
*/
#define AIS_INPUT_DELAY_SUSPEND_TIMER 1500
#define WAIT_DETECT_DONE_TIMEOUT 2000

/**
 * SuspendTimerHandle
 *
 * @brief  Timer handler to put input device into suspend mode if it expires
 */
void AisInputConfigurer::SuspendTimerHandle(void* pUsrData)
{
    CameraResult result = CAMERA_SUCCESS;
    InputDeviceType* pInputDevice = (InputDeviceType*)pUsrData;
    CameraPowerEventType powerEventId = CAMERA_POWER_DOWN;

    AIS_LOG(ENGINE, HIGH, "CameraTimerHandle devId 0x%x", pInputDevice->devId);

    if (!pInputDevice->hDevice)
    {
        AIS_LOG(ENGINE, ERROR, "DevId 0x%x hDevice is Null", pInputDevice->devId);
        return;
    }

    CameraLockMutex(pInputDevice->m_mutex);

    if (pInputDevice->delay_suspend_flag == TRUE)
    {
        result = CameraDeviceControl(pInputDevice->hDevice,
                 Camera_Sensor_AEEUID_CTL_STATE_POWER_SUSPEND,
                 &powerEventId, sizeof(powerEventId), NULL, 0, NULL);

       if (CAMERA_SUCCESS != result)
       {
           AIS_LOG(ENGINE, ERROR, "PowerSuspend failed for sensor ID 0x%x",
                   pInputDevice->sDeviceInfo.deviceID);
           CameraUnlockMutex(pInputDevice->m_mutex);
           return;
       }
       pInputDevice->state = AIS_INPUT_STATE_SUSPEND;

       pInputDevice->delay_suspend_flag = FALSE;
    }

    CameraUnlockMutex(pInputDevice->m_mutex);
}

///@brief AisEngine singleton
AisInputConfigurer* AisInputConfigurer::m_pInputConfigurerInstance = nullptr;


/**
 * AisIFEConfigurer::CreateInstance
 *
 * @brief Creates singleton instance for AisInputConfigurer
 */
AisInputConfigurer* AisInputConfigurer::CreateInstance()
{
    if(m_pInputConfigurerInstance == nullptr)
    {
        m_pInputConfigurerInstance = new AisInputConfigurer();
        if (m_pInputConfigurerInstance)
        {
            CameraResult rc = m_pInputConfigurerInstance->Init();
            if (rc)
            {
                DestroyInstance();
            }
        }
    }

    return m_pInputConfigurerInstance;
}

/**
 * AisIFEConfigurer::GetInstance
 *
 * @brief Gets the singleton instance for AisInputConfigurer
 */
AisInputConfigurer* AisInputConfigurer::GetInstance()
{
    return m_pInputConfigurerInstance;
}

/**
 * AisCSIConfigurer::AisIFEConfigurer
 *
 * @brief Destroy the singleton instance of the AisInputConfigurer class
 */
void AisInputConfigurer::DestroyInstance()
{
    if(m_pInputConfigurerInstance != nullptr)
    {
        m_pInputConfigurerInstance->Deinit();

        delete m_pInputConfigurerInstance;
        m_pInputConfigurerInstance = nullptr;
    }
}

/**
 * AisInputConfigurer::Init
 *
 * @brief Input Configurer Init.
 *      Queries Input devices and sets up detection threads
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::Init(void)
{
    CameraResult result = CAMERA_SUCCESS;
    CameraDeviceInfoType cameraDeviceInfo[MAX_CAMERA_INPUT_DEVICES] = {};
    uint32 devIdx;
    uint32 nCameraDeviceInfoLenReq = 0;
    CameraQueueCreateParamType sCreateParams = {};
    char name[64];
    const CameraChannelInfoType* pCameraDataChannel = NULL;
    int numChannels = 0;

    // Get Multi SOC Environment
    m_isMultiSoc = CameraPlatform_GetMultiSocEnv();

    result = CameraPlatformGetChannelData(&pCameraDataChannel, &numChannels);
    //Get platform channel mapping info
    if (CAMERA_SUCCESS == result)
    {
        m_nInputMapping = 0;

        for (int i = 0; i < numChannels; i++)
        {
            CameraResult tmpResult;

            m_InputMappingTable[m_nInputMapping].sInfo = pCameraDataChannel[i];

            tmpResult = AisProcChainManager::GetNumStreams(m_InputMappingTable[m_nInputMapping].sInfo.opMode,
                                                           &m_InputMappingTable[m_nInputMapping].numStreams);
            if (CAMERA_SUCCESS != tmpResult)
            {
                AIS_LOG(ENGINE, ERROR, "Failed to get numStreams for inputId %d (%d) - skipping it",
                        m_InputMappingTable[m_nInputMapping].sInfo.aisInputId, tmpResult);
            }
            else
            {
                m_nInputMapping++;
            }
        }
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "GetChannelData failed rc %d", result);
        goto error;
    }

    // Get the list of available sensor device drivers from the
    // CameraDeviceManager
    result = mDeviceManagerContext->GetAvailableDevices(
        CAMERA_DEVICE_CATEGORY_SENSOR,
        &cameraDeviceInfo[0],
        MAX_CAMERA_INPUT_DEVICES,
        &nCameraDeviceInfoLenReq);

    if (CAMERA_SUCCESS != result)
    {
        AIS_LOG(ENGINE, ERROR, "Failed to get available input devices");
        goto error;
    }
    else if (nCameraDeviceInfoLenReq > MAX_CAMERA_INPUT_DEVICES)
    {
        AIS_LOG(ENGINE, ERROR, "Queried more inputs (%d) than can handle (%d)",
                nCameraDeviceInfoLenReq, MAX_CAMERA_INPUT_DEVICES);
        result = CAMERA_ENEEDMORE;
        goto error;
    }

    m_nInputDevices = nCameraDeviceInfoLenReq;

    sCreateParams.nCapacity = 8;
    sCreateParams.nDataSizeInBytes = sizeof(InputEventMsgType);
    sCreateParams.eLockType = CAMERAQUEUE_LOCK_THREAD;

    m_detectHandlerIsExit = FALSE;
    CameraCreateMutex(&m_detect_mutex);
    CameraCreateSignal(&m_detectDone);

    for (int i = 0; i < MAX_DETECT_THREAD_IDX; i++)
    {
        InputDetectHandlerType* pDetectHndlr = &m_detectHandler[i];

        result = CameraQueueCreate(&pDetectHndlr->detectQ, &sCreateParams);
        AIS_LOG_ON_ERR(ENGINE, result, "Failed to create detect event queue: %d", result);

        result = CameraCreateSignal(&pDetectHndlr->signal);
        AIS_LOG_ON_ERR(ENGINE, result, "Failed to create signal: %d", result);

        snprintf(name, sizeof(name), "INPUT_detect_hndlr_%d", i);

        if (CAMERA_SUCCESS == result)
        {
            if (0 != CameraCreateThread(CAMERA_THREAD_PRIO_NORMAL,
                    0,
                    AisInputConfigurer::DetectHandlerThread,
                    pDetectHndlr,
                    0x8000,
                    name,
                    &pDetectHndlr->threadId))
            {
                AIS_LOG(ENGINE, ERROR, "CameraCreateThread failed");
                result = CAMERA_EFAILED;
                break;
            }
        }
    }

    // Search list of sensor device drivers to determine which ones are
    // actually available for use
    for (devIdx = 0;
         devIdx < m_nInputDevices;
         ++devIdx)
    {
        unsigned int i;
        InputDeviceType* pInputDev = &m_InputDevices[devIdx];
        CameraCreateMutex(&pInputDev->m_mutex);
        pInputDev->delay_suspend_flag = FALSE;
        pInputDev->sDeviceInfo = cameraDeviceInfo[devIdx];
        pInputDev->devId = cameraDeviceInfo[devIdx].deviceID;

        for(i = 0; i < m_nInputMapping; i++)
        {
            uint32 srcIdx;
            for (srcIdx = 0; srcIdx < m_InputMappingTable[i].numStreams; srcIdx++)
            {
                if (m_InputMappingTable[i].sInfo.inputSrc[srcIdx].devId == pInputDev->devId)
                {
                    m_InputMappingTable[i].devIdx[srcIdx] = devIdx;
                }
            }
        }

        AIS_LOG(ENGINE, HIGH, "Open sensor with ID: 0x%x ...",
                pInputDev->sDeviceInfo.deviceID);

        result = CameraPlatformGetInputDeviceInterface(pInputDev->devId, pInputDev->csiInfo);
        if (result != CAMERA_SUCCESS)
        {
            AIS_LOG(ENGINE, ERROR, "Could not get sensor interface info for device ID : 0x%x",
                    pInputDev->sDeviceInfo.deviceID);
            continue;
        }

        result = mDeviceManagerContext->DeviceOpen(pInputDev->sDeviceInfo.deviceID,
                &pInputDev->hDevice);

        // If the sensor driver did not open successfully, ignore it and move
        // on. Otherwise, if the sensor driver was successfully opened, then
        // add this sensor to the list of available ones.
        if (CAMERA_SUCCESS != result)
        {
            pInputDev->hDevice = NULL;
            AIS_LOG(ENGINE, ERROR, "Could not open sensor with ID: 0x%x",
                    pInputDev->sDeviceInfo.deviceID);
            continue;
        }

        result = CameraDeviceRegisterCallback(pInputDev->hDevice,
            AisInputConfigurer::InputDeviceCallback, this);
        if (CAMERA_SUCCESS != result)
        {
            AIS_LOG(ENGINE, ERROR, "Could not set callback for sensor with ID: 0x%x",
                    pInputDev->sDeviceInfo.deviceID);
            continue;
        }
    }

    return CAMERA_SUCCESS;

error:
    return result;
}

/**
 * AisInputConfigurer::Deinit
 *
 * @brief Input Configurer Deinit.
 *      Releases Input devices
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::Deinit(void)
{
    CameraResult result = CAMERA_SUCCESS;
    uint32 device;

    CameraSetSignal(m_detectDone);
    m_detectHandlerIsExit = TRUE;
    for (int i = 0; i < MAX_DETECT_THREAD_IDX; i++)
    {
        InputDetectHandlerType* pDetectHndlr = &m_detectHandler[i];

        if (pDetectHndlr->signal)
        {
            CameraSetSignal(pDetectHndlr->signal);

            if (pDetectHndlr->threadId)
            {
                CameraJoinThread(pDetectHndlr->threadId, NULL);
                CameraReleaseThread(pDetectHndlr->threadId);
            }

            CameraDestroySignal(pDetectHndlr->signal);
        }

        if (pDetectHndlr->detectQ)
        {
            CameraQueueDestroy(pDetectHndlr->detectQ);
        }
    }
    CameraDestroySignal(m_detectDone);
    m_detectDone = NULL;

    for (device = 0;
         device < m_nInputDevices;
         ++device)
    {
        if (m_InputDevices[device].hDevice)
        {
            result |= CameraDeviceClose(m_InputDevices[device].hDevice);
        }

        m_InputDevices[device].delay_suspend_flag = FALSE;
        CameraReleaseTimer(m_InputDevices[device].pTimer);

        CameraDestroyMutex(m_InputDevices[device].m_mutex);
    }
    CameraDestroyMutex(m_detect_mutex);

    return CAMERA_SUCCESS;
}

/**
 * AisInputConfigurer::QueueDetectEvent
 *
 * @brief Queue detection event to specific detection thread handler
 *
 * @param pDetectHndlr   detection thread handler
 * @param pMsg           detection payload of which input dev to detect
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::QueueDetectEvent(InputDetectHandlerType* pDetectHndlr, InputEventMsgType* pMsg)
{
    CameraResult result;

    AIS_LOG(ENGINE, HIGH, "q_event %d", pMsg->eventId);

    result = CameraQueueEnqueue(pDetectHndlr->detectQ, pMsg);

    if (result == CAMERA_SUCCESS)
    {
        result = CameraSetSignal(pDetectHndlr->signal);
    }
    else
    {
        AIS_LOG(ENGINE, ERROR,"Failed to enqueue event %d (%d)", pMsg->eventId, result);
    }

    return result;
}

/**
 * AisInputConfigurer::ProcessDetectEvent
 *
 * @brief Dequeues detection event from Q and performs sensor detection
 *
 * @param pDetectHndlr   detection thread handler
 *
 * @return CameraResult
 */
int AisInputConfigurer::ProcessDetectEvent(InputDetectHandlerType* pDetectHndlr)
{
    CameraResult result;
    InputEventMsgType msg = {};

    result = CameraQueueDequeue(pDetectHndlr->detectQ, &msg);

    if (CAMERA_SUCCESS != result)
    {
       if (CAMERA_ENOMORE != result)
       {
           AIS_LOG(ENGINE, ERROR, "Failed to dequeue event (%d)", result);
       }
       return 0;
    }

    switch (msg.eventId)
    {
    case INPUT_EVENT_DEFER_DETECT:
    {
        result = AisInputConfigurer::GetInstance()->DetectInput(msg.devIdx);
        AIS_LOG_ON_ERR(ENGINE, result, "Failed to detect devIdx=%d", msg.devIdx);
        break;
    }
    default:
       break;
    }

    return 1;
}

/**
 * AisInputConfigurer::DetectHandlerThread
 *
 * @brief Detection Thread handler. Waits for available job and processes it
 *
 * @param pDetectHndlr   detection thread handler
 *
 * @return CameraResult
 */
int AisInputConfigurer::DetectHandlerThread(void *pArg)
{
    CameraResult rc = CAMERA_SUCCESS;
    InputDetectHandlerType* pDetectHndlr = (InputDetectHandlerType*)pArg;
    AisInputConfigurer* pCtxt = GetInstance();

    if (pDetectHndlr)
    {
        while (!pCtxt->m_detectHandlerIsExit)
        {
            AIS_LOG(ENGINE, LOW, "Awake; ready to work");

            (void)pCtxt->ProcessDetectEvent(pDetectHndlr);

            AIS_LOG(ENGINE, LOW, "Going to wait");

            CameraWaitOnSignal(pDetectHndlr->signal, CAM_SIGNAL_WAIT_NO_TIMEOUT);
        }

        AIS_LOG(ENGINE, HIGH, "Terminating (%d) ...", rc);
    }

    return 0;
}

/**
 * AisInputConfigurer::PowerSuspend
 *
 * @param AisUsrCtxt
 * @param bGranular
 * @param powerEventId
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::PowerSuspend(AisUsrCtxt* pUsrCtxt, boolean bGranular, CameraPowerEventType powerEventId)
{
    CameraResult result = CAMERA_SUCCESS;
    CameraResult tmpResult = CAMERA_SUCCESS;
    uint32 devIdx;
    uint32 delayTimeout;

    const CameraEngineSettings* engineSettings = CameraPlatformGetEngineSettings();
    if (engineSettings->powerManagementPolicy == CAMERA_PM_POLICY_NO_LPM)
    {
        delayTimeout = AIS_INPUT_DELAY_SUSPEND_TIMER;
    }
    else
    {
        delayTimeout = 0; // no need to delay suspend in LPM mode
    }

    if (!bGranular)
    {
        for (devIdx = 0; devIdx < m_nInputDevices; ++devIdx)
        {
            InputDeviceType* pInputDev = &m_InputDevices[devIdx];

            if (!pInputDev->hDevice ||
                pInputDev->state == AIS_INPUT_STATE_UNAVAILABLE)
            {
                AIS_LOG(ENGINE, LOW, "devIdx: 0x%x is not attached", devIdx);
                continue;
            }

            if (pInputDev->state == AIS_INPUT_STATE_SUSPEND)
            {
                AIS_LOG(ENGINE, LOW, "devIdx: 0x%x already power suspend", devIdx);
                continue;
            }

            if (pInputDev->delay_suspend_flag)
            {
                CameraStopTimer(pInputDev->pTimer);
            }

            tmpResult = CameraDeviceControl(pInputDev->hDevice,
                                            Camera_Sensor_AEEUID_CTL_STATE_POWER_SUSPEND,
                                            &powerEventId, sizeof(powerEventId), NULL, 0, NULL);
            if (CAMERA_SUCCESS != tmpResult)
            {
                AIS_LOG(ENGINE, ERROR, "PowerSuspend failed for sensor ID 0x%x",
                        pInputDev->sDeviceInfo.deviceID);
                result = tmpResult;
                continue;
            }
            pInputDev->state = AIS_INPUT_STATE_SUSPEND;
        }
    }
    else
    {
        uint32 streamIdx;

        if (NULL == pUsrCtxt)
        {
            AIS_LOG(ENGINE, ERROR, "input ctxt param is NULL");
            return CAMERA_EBADPARM;
        }

        for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams; streamIdx++)
        {
            uint32 devId = pUsrCtxt->m_streams[streamIdx].inputCfg.devId;
            InputDeviceType* pInputDev = &m_InputDevices[devId];

            if (!pInputDev->hDevice ||
                    pInputDev->state == AIS_INPUT_STATE_UNAVAILABLE)
            {
                AIS_LOG(ENGINE, ERROR, "devId: 0x%x is not attached", pInputDev->devId);
                return CAMERA_EFAILED;
            }

            if (pInputDev->state == AIS_INPUT_STATE_SUSPEND)
            {
                AIS_LOG(ENGINE, LOW, "devId: 0x%x already power suspend", pInputDev->devId);
                return result;
            }

            AIS_LOG(ENGINE, HIGH, "devId: 0x%x, refcnt: %d", devId, pInputDev->refcnt);
            if (pInputDev->refcnt == 0)
            {
                if (delayTimeout == 0)
                {
                    result = CameraDeviceControl(m_InputDevices[devId].hDevice,
                            Camera_Sensor_AEEUID_CTL_STATE_POWER_SUSPEND,
                            &powerEventId, sizeof(powerEventId), NULL, 0, NULL);
                    if (CAMERA_SUCCESS != result)
                    {
                        AIS_LOG(ENGINE, ERROR, "PowerSuspend failed for sensor ID 0x%x",
                                m_InputDevices[devId].sDeviceInfo.deviceID);
                    }
                    m_InputDevices[devId].state = AIS_INPUT_STATE_SUSPEND;
                }
                else
                {
                    pInputDev->delay_suspend_flag = TRUE;

                    if (pInputDev->pTimer == NULL)
                    {
                        AIS_LOG(ENGINE, HIGH, "CameraCreateTimer devId 0x%x", pInputDev->devId);
                        CameraCreateTimer(delayTimeout, 0, AisInputConfigurer::SuspendTimerHandle,
                                pInputDev, &pInputDev->pTimer);
                    }
                    else
                    {
                        AIS_LOG(ENGINE, HIGH, "CameraUpdateTimer devId 0x%x", pInputDev->devId);
                        CameraUpdateTimer(pInputDev->pTimer, delayTimeout);
                    }
                }
            }
        }
    }

    return CAMERA_SUCCESS;
}

/**
 * AisInputConfigurer::PowerResume
 *
 * @param AisUsrCtxt
 * @param bGranular
 * @param powerEventId
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::PowerResume(AisUsrCtxt* pUsrCtxt, boolean bGranular, CameraPowerEventType powerEventId)
{
    CameraResult result = CAMERA_SUCCESS;
    CameraResult tmpResult = CAMERA_SUCCESS;
    uint32 devIdx;

    if (!bGranular)
    {
        for (devIdx = 0; devIdx < m_nInputDevices; ++devIdx)
        {
            InputDeviceType* pInputDev = &m_InputDevices[devIdx];

            if (!pInputDev->hDevice ||
                pInputDev->state == AIS_INPUT_STATE_UNAVAILABLE)
            {
                AIS_LOG(ENGINE, LOW, "devIdx: 0x%x is not attached", devIdx);
                continue;
            }

            /* If powerEventId is CAMERA_POWER_POST_HIBERNATION or CAMERA_POWER_POST_LPM,
               proceed with PowerResume irrespective of the pInputDev state*/

            if ((pInputDev->state != AIS_INPUT_STATE_SUSPEND)
                && (pInputDev->state != AIS_INPUT_STATE_OFF)
                && (powerEventId != CAMERA_POWER_POST_HIBERNATION)
                && (powerEventId != CAMERA_POWER_POST_LPM))
            {
                AIS_LOG(ENGINE, HIGH, "devIdx: 0x%x already power resume", devIdx);
                continue;
            }

            tmpResult = CameraDeviceControl(pInputDev->hDevice,
                                            Camera_Sensor_AEEUID_CTL_STATE_POWER_RESUME,
                                            &powerEventId, sizeof(powerEventId), NULL, 0, NULL);
            if (CAMERA_SUCCESS != tmpResult)
            {
                AIS_LOG(ENGINE, ERROR, "PowerResume failed for sensor ID 0x%x",
                        pInputDev->sDeviceInfo.deviceID);
                result = tmpResult;
                continue;
            }
            pInputDev->state = AIS_INPUT_STATE_RESUME;
        }
    }
    else
    {
        uint32 streamIdx;

        if (NULL == pUsrCtxt)
        {
            AIS_LOG(ENGINE, ERROR, "input ctxt param is NULL");
            return CAMERA_EBADPARM;
        }

        for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams; streamIdx++)
        {
            uint32 devId = pUsrCtxt->m_streams[streamIdx].inputCfg.devId;
            InputDeviceType* pInputDev = &m_InputDevices[devId];

            if (!pInputDev->hDevice ||
                    pInputDev->state == AIS_INPUT_STATE_UNAVAILABLE)
            {
                AIS_LOG(ENGINE, ERROR, "devIdx: 0x%x is not attached", devId);
                return CAMERA_EFAILED;
            }
            AIS_LOG(ENGINE, HIGH, "enter device ID: 0x%x, devId %d, state %d",
                    pInputDev->sDeviceInfo.deviceID, devId, pInputDev->state);

            CameraLockMutex(pInputDev->m_mutex);
            if (pInputDev->delay_suspend_flag == TRUE)
            {
               pInputDev->delay_suspend_flag = FALSE;
               CameraStopTimer(pInputDev->pTimer);
            }

            if ((pInputDev->state != AIS_INPUT_STATE_SUSPEND)
                    && (pInputDev->state != AIS_INPUT_STATE_OFF))
            {
                CameraUnlockMutex(pInputDev->m_mutex);
                AIS_LOG(ENGINE, HIGH, "devId: 0x%x already power resume", devId);
                return CAMERA_SUCCESS;
            }

            result = CameraDeviceControl(pInputDev->hDevice,
                    Camera_Sensor_AEEUID_CTL_STATE_POWER_RESUME,
                    &powerEventId, sizeof(powerEventId), NULL, 0, NULL);
            if (CAMERA_SUCCESS != result)
            {
                CameraUnlockMutex(pInputDev->m_mutex);

                AIS_LOG(ENGINE, ERROR, "PowerResume failed for sensor ID 0x%x",
                        pInputDev->sDeviceInfo.deviceID);
                return result;
            }

            pInputDev->state = AIS_INPUT_STATE_RESUME;

            CameraUnlockMutex(pInputDev->m_mutex);

            AIS_LOG(ENGINE, HIGH, "exit: devId=%d, state=%d", devId, m_InputDevices[devId].state);
        }
    }
    return CAMERA_SUCCESS;
}

/**
 * AisInputConfigurer::FillStreamCSIParams
 *
 * @param pUsrCtxt
 * @param pStream
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::FillStreamCSIParams(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream)
{
    CameraResult rc = CAMERA_SUCCESS;
    Camera_Sensor_MipiCsiInfoType csiInfo = {};
    uint32 devId = pStream->inputCfg.devId;
    uint32 srcId = pStream->inputCfg.srcId;

    AisInputCsiParamsType* pCsiParams = &pStream->inputCfg.csiInfo;

    rc = CameraDeviceControl(m_InputDevices[devId].hDevice,
            Camera_Sensor_AEEUID_CTL_CSI_INFO_PARAMS,
            NULL, 0, &csiInfo, sizeof(csiInfo), NULL);

    if (CAMERA_SUCCESS == rc)
    {
        pCsiParams->num_lanes = csiInfo.MipiCsiInfo[pStream->inputCfg.csiIdx].num_lanes; // TODO check against m_InputDevices[devId].numLanes;
        pCsiParams->lane_mask = csiInfo.MipiCsiInfo[pStream->inputCfg.csiIdx].lane_mask;
        pCsiParams->lane_assign = m_InputDevices[devId].csiInfo[pStream->inputCfg.csiIdx].laneAssign;
        pCsiParams->mipi_rate = csiInfo.MipiCsiInfo[pStream->inputCfg.csiIdx].mipi_rate;
        pCsiParams->is_secure = m_InputDevices[devId].csiInfo[pStream->inputCfg.csiIdx].isSecure;
        pCsiParams->forceHSmode = m_InputDevices[devId].csiInfo[pStream->inputCfg.csiIdx].forceHSmode;
        pCsiParams->settle_count = csiInfo.MipiCsiInfo[pStream->inputCfg.csiIdx].settle_count;
        pCsiParams->is_csi_3phase = csiInfo.MipiCsiInfo[pStream->inputCfg.csiIdx].is_csi_3phase;
        pCsiParams->vcx_mode = csiInfo.MipiCsiInfo[pStream->inputCfg.csiIdx].vcx_mode;
        pCsiParams->sensor_num_frame_skip = csiInfo.sensor_num_frame_skip;
    }

    AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != rc),
            "Error retrieving csi params for devId %d srcId %d result=%d",
            devId, srcId, rc);

    return rc;
}

/**
 * AisInputConfigurer::FillStreamInputModeInfo
 *
 * @param pUsrCtxt
 * @param pStream
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::FillStreamInputModeInfo(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream)
{
    CameraResult rc = CAMERA_SUCCESS;

    AisInputModeInfoType* pModeInfo = &pStream->inputCfg.inputModeInfo;
    uint32 devId = pStream->inputCfg.devId;
    uint32 srcId = pStream->inputCfg.srcId;
    uint32 currentMode = 0;  //Todo: This needs to be set and queried

    rc = CameraDeviceControl(m_InputDevices[devId].hDevice,
            Camera_Sensor_AEEUID_CTL_INFO_SUBCHANNELS,
            NULL, 0,
            &m_InputDevices[devId].subchannelsInfo,
            sizeof(m_InputDevices[devId].subchannelsInfo), NULL);

    if (CAMERA_SUCCESS == rc)
    {
        rc = GetModeInfo(pUsrCtxt, pStream, currentMode, pModeInfo);
    }

    AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != rc),
            "Error retrieving input mode info for devId %d srcId %d result=%d",
            devId, srcId, rc);

    return rc;
}

/**
 * AisInputConfigurer::Config
 *
 * @param pUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::Config(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 streamIdx;

    rc = ValidateInputId(pUsrCtxt);
    if (CAMERA_SUCCESS != rc)
    {
        AIS_LOG(ENGINE, ERROR, "Input %d not connected",(int)pUsrCtxt->m_inputId);
        return rc;
    }

    if (CAMERA_SUCCESS == rc)
    {
        rc = SetParam(pUsrCtxt, AIS_INPUT_CTRL_MODE, NULL);
    }

    if (CAMERA_SUCCESS == rc)
    {
        rc = SetParam(pUsrCtxt, AIS_INPUT_CTRL_CCI_SYNC, NULL);
        AIS_LOG(ENGINE, HIGH, "AIS_INPUT_CTRL_CCI_SYNC set to input%d",(int)pUsrCtxt->m_inputId);
    }

    if(CAMERA_SUCCESS == rc && (pUsrCtxt->m_usrSettings.bitmask & (1 << QCARCAM_PARAM_RESOLUTION)))
    {
        qcarcam_res_t resInfo;
        resInfo.width = pUsrCtxt->m_usrSettings.width;
        resInfo.height = pUsrCtxt->m_usrSettings.height;

        rc = SetParam(pUsrCtxt,
                AIS_INPUT_CTRL_RESOLUTION,
                &resInfo);

        AIS_LOG(ENGINE, HIGH, "QCARCAM_PARAM_RESOLUTION is set to %dx%d",
                resInfo.width,resInfo.height);
    }

    for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams; streamIdx++)
    {
        AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];

        if (pStream->type != AIS_STREAM_TYPE_IFE)
        {
            continue;
        }

        if (CAMERA_SUCCESS == rc)
        {
            rc = FillStreamCSIParams(pUsrCtxt, pStream);

            AIS_LOG(ENGINE, HIGH, "settle %d lanes %d mask 0x%x assign 0x%x",
                    pStream->inputCfg.csiInfo.settle_count,
                    pStream->inputCfg.csiInfo.num_lanes,
                    pStream->inputCfg.csiInfo.lane_mask,
                    pStream->inputCfg.csiInfo.lane_assign);
        }

        if (CAMERA_SUCCESS == rc)
        {
            rc = FillStreamInputModeInfo(pUsrCtxt, pStream);
            AIS_LOG(ENGINE, HIGH, "width %d height %d fps %d vc %d dt %d interlaced %d",
                    pStream->inputCfg.inputModeInfo.width, pStream->inputCfg.inputModeInfo.height,
                    pStream->inputCfg.inputModeInfo.fps, pStream->inputCfg.inputModeInfo.vc,
                    pStream->inputCfg.inputModeInfo.dt, pStream->inputCfg.inputModeInfo.interlaced);
        }
    }

    return rc;
}

/**
 * GlobalConfig
 *
 * @brief Config global Ctxt. Used for Multi SOC feature.
 *
 * @param AisGlobalCtxt
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::GlobalConfig(AisGlobalCtxt* pGlobalCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (TRUE == m_isMultiSoc)
    {
        uint32 streamIdx;
        uint32 idx, devId;
        AisUsrCtxt *ptr = NULL;

        for (idx = 0; idx < m_nInputMapping; idx++)
        {
            pGlobalCtxt->m_inputId[idx] = (qcarcam_input_desc_t)m_InputMappingTable[idx].sInfo.aisInputId;
            pGlobalCtxt->m_numStreams[idx] = m_InputMappingTable[idx].numStreams;

            for (streamIdx = 0; streamIdx < pGlobalCtxt->m_numStreams[idx] && CAMERA_SUCCESS == rc; streamIdx++)
            {
                AisUsrCtxtStreamType* pStream = &pGlobalCtxt->m_streams[idx][streamIdx];
                devId = m_InputMappingTable[idx].devIdx[streamIdx];
                pStream->resources.csiphy = m_InputDevices[devId].csiInfo[m_InputMappingTable[idx].sInfo.inputSrc[streamIdx].csiIdx].csiId;
                pStream->inputCfg.devId = m_InputMappingTable[idx].devIdx[streamIdx];
                pStream->inputCfg.srcId = m_InputMappingTable[idx].sInfo.inputSrc[streamIdx].srcId;

                rc = FillStreamCSIParams(ptr, pStream);

                AIS_LOG(ENGINE, HIGH, "settle %d lanes %d mask 0x%x assign 0x%x",
                        pStream->inputCfg.csiInfo.settle_count,
                        pStream->inputCfg.csiInfo.num_lanes,
                        pStream->inputCfg.csiInfo.lane_mask,
                        pStream->inputCfg.csiInfo.lane_assign);
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
CameraResult AisInputConfigurer::Start(AisUsrCtxt* pUsrCtxt)
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
 * StartStream
 *
 * @brief Start Input Dev used by a stream
 *
 * @param AisUsrCtxt
 * @param AisUsrCtxtStreamType
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::StartStream(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 devId = pStream->inputCfg.devId;
    uint32 srcId = pStream->inputCfg.srcId;
    uint32 src_id_mask = 1 << srcId;

    AIS_LOG(ENGINE, HIGH, "devId=%d, srcId=%d, state=%d", devId,srcId, m_InputDevices[devId].state);

    if (m_InputDevices[devId].streamRefCnt[srcId] == 0)
    {
        rc = CameraDeviceControl(m_InputDevices[devId].hDevice,
                Camera_Sensor_AEEUID_CTL_STATE_FRAME_OUTPUT_START,
                &src_id_mask, sizeof(uint32), NULL, 0, NULL);
        AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != rc), "Failed to start input device %d rc=%d", devId, rc);
        if (CAMERA_SUCCESS == rc)
        {
            m_InputDevices[devId].state = AIS_INPUT_STATE_STREAMING;
        }
    }
    else if (AIS_INPUT_STATE_STREAMING != m_InputDevices[devId].state)
    {
        rc = CAMERA_EBADSTATE;
    }

    //If success, increment refcount
    if (CAMERA_SUCCESS == rc)
    {
        m_InputDevices[devId].refcnt++;
        m_InputDevices[devId].streamRefCnt[srcId]++;
        AIS_LOG(ENGINE, MED, "input start dev %d src %d refcnt %d stream refcnt %d", devId, srcId,
                m_InputDevices[devId].refcnt, m_InputDevices[devId].streamRefCnt[srcId]);
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
CameraResult AisInputConfigurer::Stop(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;

    // Stop is called only for single SOC environment
    if (FALSE == m_isMultiSoc)
    {
        uint32 streamIdx;

        for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams; streamIdx++)
        {
            CameraResult tmp_rc;
            AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];

            if (pStream->type != AIS_STREAM_TYPE_IFE)
            {
                continue;
            }

            CAMERA_LOG_FIRST_ERROR(StopStream(pUsrCtxt, pStream), rc, tmp_rc);;
        }
    }

    return rc;
}

/**
 * StopStream
 *
 * @brief Stop Input Dev used by a stream
 *
 * @param AisUsrCtxtStreamType
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::StopStream(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 devId = pStream->inputCfg.devId;
    uint32 srcId = pStream->inputCfg.srcId;
    uint32 src_id_mask = 1 << srcId;

    //Check to ensure refcount valid
    if (m_InputDevices[devId].refcnt)
    {
        m_InputDevices[devId].refcnt--;
        m_InputDevices[devId].streamRefCnt[srcId]--;

        AIS_LOG(ENGINE, MED, "Input stop dev: %d src %d refcount %d stream refcnt %d",
                devId, srcId, m_InputDevices[devId].refcnt, m_InputDevices[devId].streamRefCnt[srcId]);

        if (m_InputDevices[devId].streamRefCnt[srcId] == 0)
        {
            AIS_LOG(ENGINE, HIGH, "No active users. Stop device %d", devId);
            rc = CameraDeviceControl(m_InputDevices[devId].hDevice,
                    Camera_Sensor_AEEUID_CTL_STATE_FRAME_OUTPUT_STOP,
                    &src_id_mask, sizeof(uint32), NULL, 0, NULL);
            AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != rc), "Failed to stop input device %d rc=%d", devId, rc);
        }
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Trying to stop Input dev: %d but refcount %d", devId, m_InputDevices[devId].refcnt);
        rc = CAMERA_EBADSTATE;
    }

    return rc;
}

/**
 * GlobalStop
 *
 * @brief Stop all running device. Used for Multi SOC feature.
 *
 * @param AisGlobalCtxt
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::GlobalStop(void)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (TRUE == m_isMultiSoc)
    {
        uint32 devIdx;

        for (devIdx = 0;
             devIdx < m_nInputDevices;
             ++devIdx)
        {
            //only stop if streaming and no more users
            if (AIS_INPUT_STATE_STREAMING == m_InputDevices[devIdx].state)
            {
                AIS_LOG(ENGINE, HIGH, "No active users. Stop device %d", devIdx);
                rc = CameraDeviceControl(m_InputDevices[devIdx].hDevice,
                        Camera_Sensor_AEEUID_CTL_STATE_FRAME_OUTPUT_STOP,
                        NULL, 0, NULL, 0, NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != rc), "Failed to stop input device %d rc=%d", devIdx, rc);
                m_InputDevices[devIdx].state = AIS_INPUT_STATE_ON;
            }
        }
    }

    return rc;
}

CameraResult AisInputConfigurer::Resume(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;

    return rc;
}

CameraResult AisInputConfigurer::Pause(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;

    return rc;
}


CameraResult AisInputConfigurer::SetParam(AisUsrCtxt* pUsrCtxt, uint32 nCtrl, void *param)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 streamIdx;

    for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == rc; streamIdx++)
    {
        AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];
        uint32 devId = pStream->inputCfg.devId;
        uint32 srcId = pStream->inputCfg.srcId;

        if (pStream->type != AIS_STREAM_TYPE_IFE)
        {
            continue;
        }

        switch (nCtrl)
        {
            case AIS_INPUT_CTRL_MODE:
            {
                if (m_InputDevices[devId].streamRefCnt[srcId] == 0)
                {
                    Camera_Sensor_ParamConfigType sSensorConfig;
                    sSensorConfig.paramId = QCARCAM_PARAM_INPUT_MODE;
                    sSensorConfig.srcId = srcId;
                    sSensorConfig.param.uVal = pUsrCtxt->m_usrSettings.sensormode;
                    rc = CameraDeviceControl(m_InputDevices[devId].hDevice,
                            Camera_Sensor_AEEUID_CTL_CONFIG_MODE,
                            &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL, 0, NULL);
                }
                else
                {
                    AIS_LOG(ENGINE, ERROR, "Not supporting to set sensor mode during streaming");
                }
                break;
            }
            case AIS_INPUT_CTRL_RESOLUTION:
            {
                qcarcam_res_t* pRes = (qcarcam_res_t*)param;
                Camera_Size pResConfig;
                pResConfig.width = pRes->width;
                pResConfig.height = pRes->height;
                rc = CameraDeviceControl(m_InputDevices[devId].hDevice,
                        Camera_Sensor_AEEUID_CTL_CONFIG_RESOLUTION,
                        &pResConfig, sizeof(Camera_Size), NULL, 0, NULL);
                break;
            }
            case AIS_INPUT_CTRL_EXPOSURE_CONFIG:
            {
                Camera_Sensor_ParamConfigType sSensorConfig;
                sSensorConfig.paramId = QCARCAM_PARAM_EXPOSURE;
                sSensorConfig.srcId = srcId;
                sSensorConfig.param.pExposureConfig = &pUsrCtxt->m_usrSettings.exposure_params;

                rc = CameraDeviceControl(m_InputDevices[devId].hDevice,
                        Camera_Sensor_AEEUID_CTL_CONFIG_SENSOR_PARAMS,
                        &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL, 0, NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != rc),
                        "Error setting  AIS_INPUT_EXPOSURE_CONFIG for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, rc);
                break;
            }
            case AIS_INPUT_CTRL_HDR_EXPOSURE_CONFIG:
            {
                Camera_Sensor_ParamConfigType sSensorConfig;
                sSensorConfig.paramId = QCARCAM_PARAM_HDR_EXPOSURE;
                sSensorConfig.srcId = srcId;
                sSensorConfig.param.pHdrExposureConfig = &pUsrCtxt->m_usrSettings.hdr_exposure;

                rc = CameraDeviceControl(m_InputDevices[devId].hDevice,
                        Camera_Sensor_AEEUID_CTL_CONFIG_SENSOR_PARAMS,
                        &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL, 0, NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != rc),
                        "Error setting  AIS_INPUT_EXPOSURE_CONFIG for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, rc);
                break;
            }
            case AIS_INPUT_CTRL_SATURATION_CONFIG:
            {
                Camera_Sensor_ParamConfigType sSensorConfig;
                sSensorConfig.paramId = QCARCAM_PARAM_SATURATION;
                sSensorConfig.srcId = srcId;
                sSensorConfig.param.fVal = pUsrCtxt->m_usrSettings.saturation_param;

                rc = CameraDeviceControl(m_InputDevices[devId].hDevice,
                        Camera_Sensor_AEEUID_CTL_CONFIG_SENSOR_PARAMS,
                        &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL, 0, NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != rc),
                        "Error setting  AIS_INPUT_CTRL_SATURATION_CONFIG for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, rc);
                break;
            }
            case AIS_INPUT_CTRL_HUE_CONFIG:
            {
                Camera_Sensor_ParamConfigType sSensorConfig;
                sSensorConfig.paramId = QCARCAM_PARAM_HUE;
                sSensorConfig.srcId = srcId;
                sSensorConfig.param.fVal = pUsrCtxt->m_usrSettings.hue_param;

                rc = CameraDeviceControl(m_InputDevices[devId].hDevice,
                        Camera_Sensor_AEEUID_CTL_CONFIG_SENSOR_PARAMS,
                        &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL, 0, NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != rc),
                        "Error setting  AIS_INPUT_CTRL_HUE_CONFIG for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, rc);
                break;
            }
            case AIS_INPUT_CTRL_GAMMA_CONFIG:
            {
                Camera_Sensor_ParamConfigType sSensorConfig;
                qcarcam_gamma_config_t gammaConfig = {};
                gammaConfig.config_type = pUsrCtxt->m_usrSettings.gamma_params.config_type;
                if (gammaConfig.config_type == QCARCAM_GAMMA_EXPONENT)
                {
                    gammaConfig.gamma.f_value = pUsrCtxt->m_usrSettings.gamma_params.f_value;
                }
                else
                {
                    gammaConfig.gamma.table.length = pUsrCtxt->m_usrSettings.gamma_params.length;
                    gammaConfig.gamma.table.p_value = pUsrCtxt->m_usrSettings.gamma_params.table;
                }

                sSensorConfig.paramId = QCARCAM_PARAM_GAMMA;
                sSensorConfig.srcId = srcId;
                sSensorConfig.param.pGammaConfig = &gammaConfig;

                rc = CameraDeviceControl(m_InputDevices[devId].hDevice,
                        Camera_Sensor_AEEUID_CTL_CONFIG_SENSOR_PARAMS,
                        &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL, 0, NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != rc),
                        "Error setting  AIS_INPUT_CTL_GAMMA_CONFIG for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, rc);
                break;
            }
            case AIS_INPUT_CTRL_VENDOR_PARAM:
            {
                Camera_Sensor_ParamConfigType sSensorConfig;
                sSensorConfig.paramId = QCARCAM_PARAM_VENDOR;
                sSensorConfig.srcId = srcId;
                sSensorConfig.param.pVendorParam = (qcarcam_vendor_param_t*)param;

                rc = CameraDeviceControl(m_InputDevices[devId].hDevice,
                        Camera_Sensor_AEEUID_CTL_CONFIG_SENSOR_PARAMS,
                        &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL, 0, NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != rc),
                        "Error setting  AIS_INPUT_CTRL_VENDOR_PARAM for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, rc);

                break;
            }
            case AIS_INPUT_CTRL_BRIGHTNESS:
            {
                Camera_Sensor_ParamConfigType sSensorConfig;
                sSensorConfig.paramId = QCARCAM_PARAM_BRIGHTNESS;
                sSensorConfig.srcId = srcId;
                sSensorConfig.param.fVal = pUsrCtxt->m_usrSettings.brightness_param;

                rc = CameraDeviceControl(m_InputDevices[devId].hDevice,
                        Camera_Sensor_AEEUID_CTL_CONFIG_SENSOR_PARAMS,
                        &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL, 0, NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != rc),
                        "Error setting  AIS_INPUT_CTRL_BRIGHTNESS for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, rc);
                break;
            }
            case AIS_INPUT_CTRL_CONTRAST:
            {
                Camera_Sensor_ParamConfigType sSensorConfig;
                sSensorConfig.paramId = QCARCAM_PARAM_CONTRAST;
                sSensorConfig.srcId = srcId;
                sSensorConfig.param.fVal = pUsrCtxt->m_usrSettings.contrast_param;

                rc = CameraDeviceControl(m_InputDevices[devId].hDevice,
                        Camera_Sensor_AEEUID_CTL_CONFIG_SENSOR_PARAMS,
                        &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL, 0, NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != rc),
                        "Error setting  AIS_INPUT_CTRL_CONTRAST for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, rc);
                break;
            }
            case AIS_INPUT_CTRL_MIRROR_H:
            {
                Camera_Sensor_ParamConfigType sSensorConfig;
                sSensorConfig.paramId = QCARCAM_PARAM_MIRROR_H;
                sSensorConfig.srcId = srcId;
                sSensorConfig.param.uVal = pUsrCtxt->m_usrSettings.mirror_h_param;

                rc = CameraDeviceControl(m_InputDevices[devId].hDevice,
                        Camera_Sensor_AEEUID_CTL_CONFIG_SENSOR_PARAMS,
                        &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL, 0, NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != rc),
                        "Error setting  AIS_INPUT_CTRL_MIRROR_H for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, rc);
                break;
            }
            case AIS_INPUT_CTRL_MIRROR_V:
            {
                Camera_Sensor_ParamConfigType sSensorConfig;
                sSensorConfig.paramId = QCARCAM_PARAM_MIRROR_V;
                sSensorConfig.srcId = srcId;
                sSensorConfig.param.uVal = pUsrCtxt->m_usrSettings.mirror_v_param;

                rc = CameraDeviceControl(m_InputDevices[devId].hDevice,
                        Camera_Sensor_AEEUID_CTL_CONFIG_SENSOR_PARAMS,
                        &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL, 0, NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != rc),
                        "Error setting  AIS_INPUT_CTRL_MIRROR_V for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, rc);
                break;
            }
            case AIS_INPUT_CTRL_CCI_SYNC:
            {
                 Camera_Sensor_ParamConfigType sSensorConfig;
                 sSensorConfig.param.CCISyncParam.cid = pStream->resources.cid;
                 sSensorConfig.param.CCISyncParam.csid = pStream->resources.csid;
                 sSensorConfig.param.CCISyncParam.line = 0;
                 sSensorConfig.param.CCISyncParam.delay = 0;

                 AIS_LOG(ENGINE, DBG, "cid %d csid %d",pStream->resources.cid,pStream->resources.csid);

                 rc = CameraDeviceControl(m_InputDevices[devId].hDevice,
                        Camera_Sensor_AEEUID_CTL_CONFIG_CCI_SYNC_PARAM,
                        &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL, 0, NULL);
                break;
            }
            default:
            {
                AIS_LOG(ENGINE, ERROR, "Unsupported CTRL_ID %d", nCtrl);
                rc = CAMERA_EBADPARM;
                break;
            }
        }
    }

    return rc;
}

CameraResult AisInputConfigurer::GetParam(AisUsrCtxt* pUsrCtxt, uint32 nCtrl, void *param)
{
    CameraResult result = CAMERA_SUCCESS;

    switch (nCtrl)
    {
        case AIS_INPUT_CTRL_INPUT_INTERF:
        {
            for (uint32 idx = 0; idx < m_nInputMapping; idx++)
            {
                if (pUsrCtxt->m_inputId == m_InputMappingTable[idx].sInfo.aisInputId)
                {
                    pUsrCtxt->m_numStreams = m_InputMappingTable[idx].numStreams;
                    pUsrCtxt->m_opMode = m_InputMappingTable[idx].sInfo.opMode;

                    for (uint32 ispIdx = 0; ispIdx < m_InputMappingTable[idx].sInfo.numIspInstances; ispIdx++)
                    {
                        uint32 instanceId = m_InputMappingTable[idx].sInfo.ispInstance[ispIdx].id;
                        if (instanceId < MAX_CHANNEL_ISP_INSTANCES)
                        {
                            pUsrCtxt->m_ispInstance[instanceId].cameraId = m_InputMappingTable[idx].sInfo.ispInstance[ispIdx].cameraId;
                            pUsrCtxt->m_ispInstance[instanceId].useCase = m_InputMappingTable[idx].sInfo.ispInstance[ispIdx].useCase;
                        }
                        else
                        {
                            AIS_LOG(ENGINE, ERROR, "ISP instance id %u exceeds maximum ISP instances %d", instanceId, MAX_CHANNEL_ISP_INSTANCES);
                            result =  CAMERA_EFAILED;
                        }
                    }

                    for (uint32 streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams; streamIdx++)
                    {
                        AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];
                        uint32 secureMode = 0;

                        pStream->inputCfg.devId = m_InputMappingTable[idx].devIdx[streamIdx];
                        pStream->inputCfg.srcId = m_InputMappingTable[idx].sInfo.inputSrc[streamIdx].srcId;

                        for (uint32 i = 0; i < m_InputDevices[pStream->inputCfg.devId].subchannelsInfo.num_subchannels; i++)
                        {
                            if (m_InputDevices[pStream->inputCfg.devId].subchannelsInfo.subchannels[i].src_id == pStream->inputCfg.srcId)
                            {
                                pStream->inputCfg.csiIdx = m_InputDevices[pStream->inputCfg.devId].subchannelsInfo.subchannels[i].csi_idx;
                            }
                        }

                        secureMode = m_InputDevices[pStream->inputCfg.devId].csiInfo[pStream->inputCfg.csiIdx].isSecure;

                        if (0 == streamIdx)
                        {
                            pUsrCtxt->m_secureMode = secureMode;
                        }
                        else if (pUsrCtxt->m_secureMode != secureMode)
                        {
                            AIS_LOG(ENGINE, ERROR, "Input %d has mismatched multistream secure modes", pUsrCtxt->m_inputId);
                            result =  CAMERA_EFAILED;
                        }
                    }

                    break;
                }
            }
        }
        break;
        case AIS_INPUT_CTRL_FIELD_TYPE:
        {
            AisFieldInfoType* pFieldInfo = (AisFieldInfoType*)param;
            uint32 devId = pUsrCtxt->m_streams[0].inputCfg.devId;
            Camera_Sensor_FieldType fieldInfo = {};

            result = CameraDeviceControl(m_InputDevices[devId].hDevice,
                Camera_Sensor_AEEUID_CTL_QUERY_FIELD,
                NULL, 0, &fieldInfo, sizeof(fieldInfo), NULL);
            if (CAMERA_SUCCESS == result)
            {
                pFieldInfo->timestamp = fieldInfo.timestamp;
                pFieldInfo->fieldType = fieldInfo.fieldType;
            }

            break;
        }
        case AIS_INPUT_CTRL_VENDOR_PARAM:
        {
            for (uint32 streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == result; streamIdx++)
            {
                AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];
                uint32 devId = pStream->inputCfg.devId;
                uint32 srcId = pStream->inputCfg.srcId;

                Camera_Sensor_ParamConfigType sSensorConfig = {};
                sSensorConfig.paramId = QCARCAM_PARAM_VENDOR;
                sSensorConfig.srcId = srcId;
                sSensorConfig.param.pVendorParam = (qcarcam_vendor_param_t*)param;

                result = CameraDeviceControl(m_InputDevices[devId].hDevice,
                    Camera_Sensor_AEEUID_CTL_GET_VENDOR_PARAM,
                    NULL, 0,
                    &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL);

                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != result),
                    "Error getting  Camera_Sensor_AEEUID_CTL_GET_VENDOR_PARAM for devID =%d, inputid=%d, result=%d",
                    devId, pUsrCtxt->m_inputId, result);
            }
            break;
        }
        case AIS_INPUT_CTRL_EXPOSURE_CONFIG:
        {
            for (uint32 streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == result; streamIdx++)
            {
                AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];
                uint32 devId = pStream->inputCfg.devId;
                uint32 srcId = pStream->inputCfg.srcId;

                Camera_Sensor_ParamConfigType sSensorConfig = {};
                sSensorConfig.paramId = QCARCAM_PARAM_EXPOSURE;
                sSensorConfig.srcId = srcId;
                sSensorConfig.param.pExposureConfig = (qcarcam_exposure_config_t*)param;

                result = CameraDeviceControl(m_InputDevices[devId].hDevice,
                    Camera_Sensor_AEEUID_CTL_GET_SENSOR_PARAM,
                    NULL, 0,
                    &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != result),
                        "Error getting  AIS_INPUT_EXPOSURE_CONFIG for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, result);
            }
            break;
        }
        case AIS_INPUT_CTRL_HDR_EXPOSURE_CONFIG:
        {
            for (uint32 streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == result; streamIdx++)
            {
                AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];
                uint32 devId = pStream->inputCfg.devId;
                uint32 srcId = pStream->inputCfg.srcId;

                Camera_Sensor_ParamConfigType sSensorConfig = {};
                sSensorConfig.paramId = QCARCAM_PARAM_HDR_EXPOSURE;
                sSensorConfig.srcId = srcId;
                sSensorConfig.param.pHdrExposureConfig = (qcarcam_hdr_exposure_config_t*)param;

                result = CameraDeviceControl(m_InputDevices[devId].hDevice,
                    Camera_Sensor_AEEUID_CTL_GET_SENSOR_PARAM,
                    NULL, 0,
                    &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != result),
                        "Error getting  AIS_INPUT_EXPOSURE_CONFIG for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, result);
            }
            break;
        }
        case AIS_INPUT_CTRL_MODE:
        {
            for (uint32 streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == result; streamIdx++)
            {
                AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];
                uint32 devId = pStream->inputCfg.devId;
                uint32 srcId = pStream->inputCfg.srcId;

                Camera_Sensor_ParamConfigType sSensorConfig;
                sSensorConfig.paramId = QCARCAM_PARAM_INPUT_MODE;
                sSensorConfig.srcId = srcId;

                result = CameraDeviceControl(m_InputDevices[devId].hDevice,
                    Camera_Sensor_AEEUID_CTL_GET_SENSOR_PARAM,
                    NULL, 0,
                    &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != result),
                        "Error getting  AIS_INPUT_CTRL_MODE for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, result);
                if(CAMERA_SUCCESS == result)
                {
                    *((uint32 *)param) = sSensorConfig.param.uVal;
                }
            }
            break;
        }
        case AIS_INPUT_CTRL_COLOR_SPACE:
        {
            for (uint32 streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == result; streamIdx++)
            {
                AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];
                uint32 devId = pStream->inputCfg.devId;
                uint32 srcId = pStream->inputCfg.srcId;

                Camera_Sensor_ParamConfigType sSensorConfig;
                sSensorConfig.paramId = QCARCAM_PARAM_INPUT_COLOR_SPACE;
                sSensorConfig.srcId = srcId;
                result = CameraDeviceControl(m_InputDevices[devId].hDevice,
                    Camera_Sensor_AEEUID_CTL_GET_SENSOR_PARAM,
                    NULL, 0,
                    &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != result),
                        "Error getting  AIS_INPUT_CTRL_COLOR_SPACE for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, result);
                if(CAMERA_SUCCESS == result)
                {
                    *((uint32 *)param) = sSensorConfig.param.uVal;
                }
            }
            break;
        }
        case AIS_INPUT_CTRL_SATURATION_CONFIG:
        {
            for (uint32 streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == result; streamIdx++)
            {
                AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];
                uint32 devId = pStream->inputCfg.devId;
                uint32 srcId = pStream->inputCfg.srcId;

                Camera_Sensor_ParamConfigType sSensorConfig = {};
                sSensorConfig.paramId = QCARCAM_PARAM_SATURATION;
                sSensorConfig.srcId = srcId;

                result = CameraDeviceControl(m_InputDevices[devId].hDevice,
                    Camera_Sensor_AEEUID_CTL_GET_SENSOR_PARAM,
                    NULL, 0,
                    &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != result),
                        "Error getting  AIS_INPUT_CTRL_SATURATION_CONFIG for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, result);

                if (CAMERA_SUCCESS == result)
                {
                    *((float *)param) = sSensorConfig.param.fVal;
                }
            }
            break;
        }
        case AIS_INPUT_CTRL_HUE_CONFIG:
        {
            for (uint32 streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == result; streamIdx++)
            {
                AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];
                uint32 devId = pStream->inputCfg.devId;
                uint32 srcId = pStream->inputCfg.srcId;

                Camera_Sensor_ParamConfigType sSensorConfig = {};
                sSensorConfig.paramId = QCARCAM_PARAM_HUE;
                sSensorConfig.srcId = srcId;

                result = CameraDeviceControl(m_InputDevices[devId].hDevice,
                    Camera_Sensor_AEEUID_CTL_GET_SENSOR_PARAM,
                    NULL, 0,
                    &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != result),
                        "Error getting  AIS_INPUT_CTRL_HUE_CONFIG for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, result);
                if (CAMERA_SUCCESS == result)
                {
                    *((float *)param) = sSensorConfig.param.fVal;
                }
            }
            break;
        }
        case AIS_INPUT_CTRL_GAMMA_CONFIG:
        {
            for (uint32 streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == result; streamIdx++)
            {
                AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];
                uint32 devId = pStream->inputCfg.devId;
                uint32 srcId = pStream->inputCfg.srcId;

                Camera_Sensor_ParamConfigType sSensorConfig = {};

                sSensorConfig.paramId = QCARCAM_PARAM_GAMMA;
                sSensorConfig.srcId = srcId;
                sSensorConfig.param.pGammaConfig = (qcarcam_gamma_config_t*)param;

                result = CameraDeviceControl(m_InputDevices[devId].hDevice,
                    Camera_Sensor_AEEUID_CTL_GET_SENSOR_PARAM,
                    NULL, 0,
                    &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != result),
                        "Error getting  AIS_INPUT_CTL_GAMMA_CONFIG for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, result);
            }
            break;
        }
        case AIS_INPUT_CTRL_BRIGHTNESS:
        {
            for (uint32 streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == result; streamIdx++)
            {
                AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];
                uint32 devId = pStream->inputCfg.devId;
                uint32 srcId = pStream->inputCfg.srcId;

                Camera_Sensor_ParamConfigType sSensorConfig = {};
                sSensorConfig.paramId = QCARCAM_PARAM_BRIGHTNESS;
                sSensorConfig.srcId = srcId;

                result = CameraDeviceControl(m_InputDevices[devId].hDevice,
                    Camera_Sensor_AEEUID_CTL_GET_SENSOR_PARAM,
                    NULL, 0,
                    &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != result),
                        "Error getting  AIS_INPUT_CTRL_BRIGHTNESS for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, result);

                if (CAMERA_SUCCESS == result)
                {
                    *((float *)param) = sSensorConfig.param.fVal;
                }
            }
            break;
        }
        case AIS_INPUT_CTRL_CONTRAST:
        {
            for (uint32 streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == result; streamIdx++)
            {
                AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];
                uint32 devId = pStream->inputCfg.devId;
                uint32 srcId = pStream->inputCfg.srcId;

                Camera_Sensor_ParamConfigType sSensorConfig = {};
                sSensorConfig.paramId = QCARCAM_PARAM_CONTRAST;
                sSensorConfig.srcId = srcId;

                result = CameraDeviceControl(m_InputDevices[devId].hDevice,
                    Camera_Sensor_AEEUID_CTL_GET_SENSOR_PARAM,
                    NULL, 0,
                    &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != result),
                        "Error getting  AIS_INPUT_CTRL_CONTRAST for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, result);

                if (CAMERA_SUCCESS == result)
                {
                    *((float *)param) = sSensorConfig.param.fVal;
                }
            }
            break;
        }
        case AIS_INPUT_CTRL_MIRROR_H:
        {
            for (uint32 streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == result; streamIdx++)
            {
                AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];
                uint32 devId = pStream->inputCfg.devId;
                uint32 srcId = pStream->inputCfg.srcId;

                Camera_Sensor_ParamConfigType sSensorConfig = {};
                sSensorConfig.paramId = QCARCAM_PARAM_MIRROR_H;
                sSensorConfig.srcId = srcId;

                result = CameraDeviceControl(m_InputDevices[devId].hDevice,
                    Camera_Sensor_AEEUID_CTL_GET_SENSOR_PARAM,
                    NULL, 0,
                    &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != result),
                        "Error getting  AIS_INPUT_CTRL_MIRROR_H for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, result);

                if (CAMERA_SUCCESS == result)
                {
                    *((unsigned int *)param) = sSensorConfig.param.uVal;
                }
            }
            break;
        }
        case AIS_INPUT_CTRL_MIRROR_V:
        {
            for (uint32 streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == result; streamIdx++)
            {
                AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];
                uint32 devId = pStream->inputCfg.devId;
                uint32 srcId = pStream->inputCfg.srcId;

                Camera_Sensor_ParamConfigType sSensorConfig = {};
                sSensorConfig.paramId = QCARCAM_PARAM_MIRROR_V;
                sSensorConfig.srcId = srcId;

                result = CameraDeviceControl(m_InputDevices[devId].hDevice,
                    Camera_Sensor_AEEUID_CTL_GET_SENSOR_PARAM,
                    NULL, 0,
                    &sSensorConfig, sizeof(Camera_Sensor_ParamConfigType), NULL);
                AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != result),
                        "Error getting AIS_INPUT_CTRL_MIRROR_V for devID =%d, inputid=%d, result=%d",
                        devId, pUsrCtxt->m_inputId, result);

                if (CAMERA_SUCCESS == result)
                {
                    *((unsigned int *)param) = sSensorConfig.param.uVal;
                }
            }
            break;
        }
        default:
            AIS_LOG(ENGINE, ERROR, "Unsupported CTRL_ID %d", nCtrl);
            result = CAMERA_EBADPARM;
            break;
    }


    return result;
}


//////////////////////////////////////////////////////////////////////////////////
/// FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////
CameraResult AisInputConfigurer::GetModeInfo(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream,
                                             uint32 mode, AisInputModeInfoType* pModeInfo)
{
    CameraResult rc = CAMERA_ERESOURCENOTFOUND;
    uint32 i;
    uint32 devId = pStream->inputCfg.devId;
    uint32 srcId = pStream->inputCfg.srcId;

    if (devId >= MAX_CAMERA_INPUT_DEVICES)
    {
        AIS_LOG(ENGINE, ERROR, "Invalid input devId %d srcId %d");
        return CAMERA_EBADPARM;
    }

    for (i = 0; i < m_InputDevices[devId].subchannelsInfo.num_subchannels; i++)
    {
        if (m_InputDevices[devId].subchannelsInfo.subchannels[i].src_id == srcId)
        {
            pModeInfo->width = m_InputDevices[devId].subchannelsInfo.subchannels[i].modes[mode].res.width;
            pModeInfo->height = m_InputDevices[devId].subchannelsInfo.subchannels[i].modes[mode].res.height;
            pModeInfo->fps = m_InputDevices[devId].subchannelsInfo.subchannels[i].modes[mode].res.fps;
            pModeInfo->vc = m_InputDevices[devId].subchannelsInfo.subchannels[i].modes[mode].channel_info.vc;
            pModeInfo->dt = m_InputDevices[devId].subchannelsInfo.subchannels[i].modes[mode].channel_info.dt;
            pModeInfo->fmt = m_InputDevices[devId].subchannelsInfo.subchannels[i].modes[mode].fmt;
            pModeInfo->interlaced = m_InputDevices[devId].subchannelsInfo.subchannels[i].modes[mode].interlaced;
            pModeInfo->crop_info.left = m_InputDevices[devId].subchannelsInfo.subchannels[i].modes[mode].crop_info.x_offset;
            pModeInfo->crop_info.top = m_InputDevices[devId].subchannelsInfo.subchannels[i].modes[mode].crop_info.y_offset;
            pModeInfo->crop_info.right = m_InputDevices[devId].subchannelsInfo.subchannels[i].modes[mode].crop_info.width + pModeInfo->crop_info.left - 1;
            pModeInfo->crop_info.bottom = m_InputDevices[devId].subchannelsInfo.subchannels[i].modes[mode].crop_info.height + pModeInfo->crop_info.top - 1;
            rc = CAMERA_SUCCESS;
            break;
        }
    }

    return rc;
}



uint32 AisInputConfigurer::GetDeviceId(uint32 device_id)
{
    uint32 devId;

    for (devId = 0; devId < m_nInputDevices; ++devId)
    {
        if (m_InputDevices[devId].devId == device_id)
        {
            return devId;
        }
    }

    return MAX_CAMERA_INPUT_DEVICES;
}

/**
 * InputDeviceCallback
 *
 * @brief Input Device Callback function
 *
 * @param pClientData
 * @param uidEvent
 * @param nEventDataLen
 * @param pEventData
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::InputDeviceCallback(void* pClientData,
        uint32 uidEvent, int nEventDataLen, void* pEventData)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisInputConfigurer* pCtxt = (AisInputConfigurer*)pClientData;
    Camera_Sensor_EventType * pEvent = (Camera_Sensor_EventType *)pEventData;

    if (nEventDataLen != sizeof(Camera_Sensor_EventType))
    {
        AIS_LOG(ENGINE, ERROR, "Received input callback with incorrect size=%d", nEventDataLen);
        return CAMERA_EBADPARM;
    }
    else if (!pEvent)
    {
        AIS_LOG(ENGINE, ERROR, "Received input callback with null event");
        return CAMERA_EMEMPTR;
    }
    else if (!pCtxt)
    {
        AIS_LOG(ENGINE, ERROR, "Received input callback with null data");
        return CAMERA_EMEMPTR;
    }

    AIS_LOG(ENGINE, LOW, "Received %d", uidEvent);

    switch (uidEvent)
    {
    case INPUT_EVENT_LOCK_STATUS:
    {
        if (pEvent->pPayload != NULL)
        {
            CameraInputEventPayloadType* pPayload = pEvent->pPayload;
            AisEventMsgType msg = {};

            AIS_LOG(ENGINE, HIGH, "LOCK_STATUS for 0x%x:%u is %s",
                    pEvent->device_id, pPayload->src_id,
                    (pPayload->lock_status == QCARCAM_INPUT_SIGNAL_VALID) ? "Valid" : "LOST");

            msg.eventId = AIS_EVENT_INPUT_STATUS;
            msg.payload.inputStatus.devId = pCtxt->GetDeviceId(pEvent->device_id);
            msg.payload.inputStatus.status = pPayload->lock_status;
            msg.payload.inputStatus.srcId = pPayload->src_id;
            rc = AisEngine::GetInstance()->QueueEvent(&msg);
        }
        else
        {
            AIS_LOG(ENGINE, ERROR, "NULL payload for LOCK_STATUS");
        }

        break;
    }
    case INPUT_EVENT_MODE_CHANGE:
    {
        if (pEvent->pPayload != NULL)
        {
            CameraInputEventPayloadType* pPayload = pEvent->pPayload;
            AisEventMsgType msg = {};

            AIS_LOG(ENGINE, HIGH, "MODE_CHANGE for 0x%x:%u",
                    pEvent->device_id, pPayload->src_id);
        }
        else
        {
            AIS_LOG(ENGINE, ERROR, "NULL payload for MODE_CHANGE");
        }
        break;
    }
    case INPUT_EVENT_FATAL_ERROR:
    {
        if (pEvent != NULL && pEvent->pPayload != NULL)
        {
            CameraInputEventPayloadType* pPayload = pEvent->pPayload;
            AisEventMsgType msg = {};

            AIS_LOG(ENGINE, WARN, "FATAL ERROR for 0x%x:%u",
                    pEvent->device_id, pPayload->src_id);

            msg.eventId = AIS_EVENT_INPUT_FATAL_ERROR;
            msg.payload.inputStatus.devId = pCtxt->GetDeviceId(pEvent->device_id);
            msg.payload.inputStatus.srcId = pPayload->src_id;
            rc = AisEngine::GetInstance()->QueueEvent(&msg);
        }
        else
        {
            AIS_LOG(ENGINE, ERROR, "NULL payload for MODE_CHANGE");
        }
        break;
    }
    case INPUT_EVENT_FRAME_FREEZE:
    {
        if (pEvent != NULL && pEvent->pPayload != NULL)
        {
            CameraInputEventPayloadType* pPayload = pEvent->pPayload;
            AisEventMsgType msg = {};

            AIS_LOG(ENGINE, HIGH, "FRAME_FREEZE for 0x%x:%u",
                    pEvent->device_id, pPayload->src_id);

            msg.eventId = AIS_EVENT_INPUT_FRAME_FREEZE;
            msg.payload.inputFrameFreeze.devId = pCtxt->GetDeviceId(pEvent->device_id);
            msg.payload.inputFrameFreeze.srcId = pPayload->src_id;
            msg.payload.inputFrameFreeze.hStream = pPayload->stream_hndl;
            msg.payload.inputFrameFreeze.status = pPayload->frame_freeze;
            rc = AisEngine::GetInstance()->QueueEvent(&msg);

            AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != rc),
                    "fail to enqueue FRAME_FREEZE event rc=%d", rc);
        }
        else
        {
            AIS_LOG(ENGINE, ERROR, "NULL payload for FRAME_FREEZE");
        }
        break;
    }
    case INPUT_EVENT_VENDOR:
    {
        if (pEvent != NULL && pEvent->pPayload != NULL)
        {
            CameraInputEventPayloadType* pPayload = pEvent->pPayload;
            AisEventMsgType msg = {};

            AIS_LOG(ENGINE, HIGH, "VENDOR event for 0x%x:%u",
                    pEvent->device_id, pPayload->src_id);

            msg.eventId = AIS_EVENT_VENDOR;
            msg.payload.inputStatus.devId = pCtxt->GetDeviceId(pEvent->device_id);
            msg.payload.inputStatus.srcId = pPayload->src_id;
            msg.payload.vendorEvent.vendor_param = pPayload->vendor_data;
            rc = AisEngine::GetInstance()->QueueEvent(&msg);

            AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != rc),
                    "fail to enqueue VENDOR event rc=%d", rc);
        }
        else
        {
            AIS_LOG(ENGINE, ERROR, "NULL payload for VENDOR");
        }
        break;
    }
    default:
        AIS_LOG(ENGINE, ERROR, "Received unknown input msg id %u", uidEvent);
        rc = CAMERA_EUNSUPPORTED;
        break;
    }

    return rc;
}

/**
 * DetectInput
 *
 * @brief Detects input with devIdx
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::DetectInput(uint32 devIdx)
{
    CameraResult result = CAMERA_SUCCESS;

    InputDeviceType* pInputDev = &m_InputDevices[devIdx];

    result = CameraDeviceControl(pInputDev->hDevice,
            Camera_Sensor_AEEUID_CTL_DETECT_DEVICE,
            NULL, 0, NULL, 0, NULL);
    if (CAMERA_SUCCESS != result)
    {
        AIS_LOG(ENGINE, ERROR, "Could not detect sensor with ID: 0x%x",
                pInputDev->sDeviceInfo.deviceID);
        goto error;
    }

    pInputDev->state = AIS_INPUT_STATE_DETECTED;

    result = CameraDeviceControl(pInputDev->hDevice,
            Camera_Sensor_AEEUID_CTL_DETECT_DEVICE_CHANNELS,
            NULL, 0, &pInputDev->src_id_enable_mask,
            sizeof(pInputDev->src_id_enable_mask), NULL);
    if (CAMERA_SUCCESS != result)
    {
        AIS_LOG(ENGINE, HIGH, "Could not detect channels for sensor with ID: 0x%x",
                pInputDev->sDeviceInfo.deviceID);
        goto error;
    }
    AIS_LOG(ENGINE, MED, "src id enable mask = %x", pInputDev->src_id_enable_mask);

    result = CameraDeviceControl(pInputDev->hDevice,
            Camera_Sensor_AEEUID_CTL_INFO_CHANNELS,
            NULL,
            0,
            &pInputDev->channelsInfo,
            sizeof(pInputDev->channelsInfo),
            NULL);
    if (CAMERA_SUCCESS == result)
    {
        result = CameraDeviceControl(pInputDev->hDevice,
                Camera_Sensor_AEEUID_CTL_INFO_SUBCHANNELS,
                NULL,
                0,
                &pInputDev->subchannelsInfo,
                sizeof(pInputDev->subchannelsInfo),
                NULL);
    }

    if (CAMERA_SUCCESS == result)
    {
        result = CameraDeviceControl(pInputDev->hDevice,
                Camera_Sensor_AEEUID_CTL_STATE_POWER_ON,
                NULL, 0, NULL, 0, NULL);
        AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != result), "Failed to power on input device %d rc=%d", devIdx, result);
        if (CAMERA_SUCCESS == result)
        {
            pInputDev->state = AIS_INPUT_STATE_ON;
        }
    }

    if (CAMERA_SUCCESS == result)
    {
        uint32 i, subchannel;
        Camera_Sensor_SubchannelsInfoType* pSubchannelsInfo = &pInputDev->subchannelsInfo;

        pInputDev->isAvailable = TRUE;

        for (i = 0; i < m_nInputMapping; i++)
        {
            InputMappingType* pInputMap = &m_InputMappingTable[i];

            uint32 srcIdx = 0;

            //Check and mark stream as detected
            for (srcIdx = 0; srcIdx < pInputMap->numStreams; srcIdx++)
            {
                if (pInputDev->devId != pInputMap->sInfo.inputSrc[srcIdx].devId)
                {
                    continue;
                }

                for (subchannel = 0; subchannel < pSubchannelsInfo->num_subchannels; subchannel++)
                {
                    if (pSubchannelsInfo->subchannels[subchannel].src_id ==
                            pInputMap->sInfo.inputSrc[srcIdx].srcId)
                    {
                        pInputMap->streamsAvailable++;
                        AIS_LOG(ENGINE, HIGH, "match dev 0x%x src %d  (%d out of %d)",
                                pInputMap->sInfo.inputSrc[srcIdx].devId, pInputMap->sInfo.inputSrc[srcIdx].srcId,
                                pInputMap->streamsAvailable, pInputMap->numStreams);
                        break;
                    }
                }

                if (subchannel == pSubchannelsInfo->num_subchannels)
                {
                    AIS_LOG(ENGINE, WARN, "Cannot match dev 0x%x src %d to sensor subchannels",
                            pInputMap->sInfo.inputSrc[srcIdx].devId, pInputMap->sInfo.inputSrc[srcIdx].srcId);
                }
                //If all streams detected, mark input as avaialble
                if (pInputMap->streamsAvailable == pInputMap->numStreams)
                {
                    AIS_LOG(ENGINE, HIGH, "Input %d available", pInputMap->sInfo.aisInputId);
                    pInputMap->isAvailable = TRUE;
                    break;
                }
            }
        }
    }

    CameraLockMutex(m_detect_mutex);
    m_detectInProgress--;
    if (m_detectInProgress == 0)
    {
        CameraSetSignal(m_detectDone);
    }
    CameraUnlockMutex(m_detect_mutex);

    return CAMERA_SUCCESS;

error:
    CameraLockMutex(m_detect_mutex);
    m_detectInProgress--;
    if (m_detectInProgress == 0)
    {
        CameraSetSignal(m_detectDone);
    }
    CameraUnlockMutex(m_detect_mutex);

    return result;
}

/**
 * DetectAll
 *
 * @brief Detects all available devices
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::DetectAll()
{
    CameraResult result = CAMERA_SUCCESS;
    uint32 devIdx;
    InputEventMsgType sDetect = {};

    sDetect.eventId = INPUT_EVENT_DEFER_DETECT;

    for (devIdx = 0; devIdx < m_nInputDevices; devIdx++)
    {
        if (m_InputDevices[devIdx].devId != 0 && m_InputDevices[devIdx].hDevice)
        {
            CameraResult tmpResult;
            uint32 detectThrdIdx = 0;
            const CameraSensorBoardType* pCameraBoard = CameraPlatformGetDeviceSensorBoardType(m_InputDevices[devIdx].devId);

            if (pCameraBoard)
            {
                detectThrdIdx = pCameraBoard->detectThrdId % MAX_DETECT_THREAD_IDX;
            }

            AIS_LOG(ENGINE, HIGH, "device_id 0x%x detectThrd %d", m_InputDevices[devIdx].devId, detectThrdIdx);

            sDetect.devIdx = devIdx;

            CameraLockMutex(m_detect_mutex);
            m_detectInProgress++;
            CameraUnlockMutex(m_detect_mutex);

            CAMERA_LOG_FIRST_ERROR(QueueDetectEvent(&m_detectHandler[detectThrdIdx], &sDetect), result, tmpResult);
        }
    }

    while (m_detectInProgress > 0)
    {
        AIS_LOG(ENGINE, LOW, "Wait for detect done %d", m_detectInProgress);
        CameraWaitOnSignal(m_detectDone, WAIT_DETECT_DONE_TIMEOUT);
    }

    return result;
}

/**
 * ProcessFrameData
 *
 * @brief Sends raw frame to sensor driver for any required parsing or processing
 *
 * @param pUsrCtxt
 * @param pBuffer
 * @param pFrameInfo
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::ProcessFrameData(AisUsrCtxt* pUsrCtxt,
        const AisBuffer* pBuffer, qcarcam_frame_info_v2_t* pFrameInfo)
{
    CameraResult rc = CAMERA_SUCCESS;
    CameraInputProcessFrameDataType frameData = {};
    uint32 streamIdx;

    frameData.pBuffer = pBuffer;
    frameData.pFrameInfo = pFrameInfo;
    frameData.hStream = (uint64)pUsrCtxt->m_qcarcamHndl;

    for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == rc; streamIdx++)
    {
        AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];

        frameData.srcId = pStream->inputCfg.srcId;

        rc = CameraDeviceControl(m_InputDevices[pStream->inputCfg.devId].hDevice,
                Camera_Sensor_AEEUID_CTL_PROCESS_FRAME_DATA,
                &frameData, sizeof(frameData), NULL, 0, NULL);
        if (CAMERA_EUNSUPPORTED == rc)
        {
            rc = CAMERA_SUCCESS;
        }

        if (CAMERA_SUCCESS != rc)
        {
            AIS_LOG(ENGINE, ERROR, "Failed ProcessFrameData inputId=%d (%d:%d), result=%d",
                pUsrCtxt->m_inputId, pStream->inputCfg.devId, pStream->inputCfg.srcId, rc);
        }
    }

    return rc;
}

/**
 * IsInputAvailable
 *
 * @brief Check if qcarcam input id is available to use
 *
 * @param input_id
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::IsInputAvailable(uint32 input_id)
{
    CameraResult rc = CAMERA_ERESOURCENOTFOUND;
    uint32 idx;

    for (idx = 0; idx < m_nInputMapping; idx++)
    {
        if (input_id == m_InputMappingTable[idx].sInfo.aisInputId)
        {
            rc = (m_InputMappingTable[idx].isAvailable) ? CAMERA_SUCCESS : CAMERA_EFAILED;
            break;
        }
    }

    return rc;
}

/**
 * GetInterface
 *
 * @brief Returns input interface parameters of particular qcarcam input id and stream index
 *
 * @param interf - interf->inputId and interf->streamIdx specifies which input id to look up
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::GetInterface(AisInputInterfaceType* interf)
{
    CameraResult rc = CAMERA_ERESOURCENOTFOUND;
    uint32 idx, devId, subchannel;

    uint32 streamIdx = interf->streamIdx;
    uint32 inputId = interf->inputId;

    for (idx = 0; idx < m_nInputMapping; idx++)
    {
        if (inputId == m_InputMappingTable[idx].sInfo.aisInputId)
        {
            devId = m_InputMappingTable[idx].devIdx[streamIdx];

            /*get cid*/
            for (subchannel = 0; subchannel < m_InputDevices[devId].subchannelsInfo.num_subchannels; subchannel++)
            {
                if (m_InputDevices[devId].subchannelsInfo.subchannels[subchannel].src_id ==
                        m_InputMappingTable[idx].sInfo.inputSrc[streamIdx].srcId)
                {
                    interf->stream.devId = devId;
                    interf->stream.srcId = m_InputMappingTable[idx].sInfo.inputSrc[streamIdx].srcId;
                    interf->stream.csiphy = m_InputDevices[devId].csiInfo[m_InputDevices[devId].subchannelsInfo.subchannels[subchannel].csi_idx].csiId;
                    interf->stream.cid = m_InputDevices[devId].subchannelsInfo.subchannels[subchannel].modes[0].channel_info.cid;
                    interf->stream.interlaced = m_InputDevices[devId].subchannelsInfo.subchannels[subchannel].modes[0].interlaced;
                    rc = CAMERA_SUCCESS;
                    break;
                }
            }

            break;
        }
    }

    return rc;
}

/**
 * QueryInputs
 *
 * @brief Query available inputs
 *
 * @param p_inputs
 * @param size - size of p_inputs array
 * @param filled_size - If p_inputs is set, number of elements in array that were filled
 *                      If p_inputs is NULL, number of available inputs to query
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::QueryInputs(qcarcam_input_t* pQcarcamInputs, unsigned int nSize,
        unsigned int* pRetSize)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 idx;
    uint32 fillIdx;
    /*cache locally as we may be in deferred detect*/
    uint32 nInputMapping = m_nInputMapping;

    if (!pQcarcamInputs)
    {
        fillIdx = 0;
        for (idx = 0; idx < nInputMapping; idx++)
        {
            if (m_InputMappingTable[idx].isAvailable)
            {
                fillIdx++;
            }
        }
        AIS_LOG(ENGINE, MED, "counted_size=%d", fillIdx);
        *pRetSize = fillIdx;
        return CAMERA_SUCCESS;
    }

    for (idx = 0, fillIdx = 0; idx < nInputMapping && fillIdx < nSize; idx++)
    {
        qcarcam_input_t* pQcarcamInput = &pQcarcamInputs[fillIdx];
        InputMappingType* pInputMap = &m_InputMappingTable[idx];

        /*do not advertise if it is not available*/
        if (!pInputMap->isAvailable)
            continue;

        memset(pQcarcamInput, 0x0, sizeof(*pQcarcamInput));

        pQcarcamInput->desc = (qcarcam_input_desc_t)pInputMap->sInfo.aisInputId;

        if (pInputMap->sInfo.opMode == QCARCAM_OPMODE_PAIRED_INPUT)
        {
            pQcarcamInput->flags |= QCARCAM_INPUT_FLAG_PAIRED;
        }

        FillQCarCamInputResInfo(pQcarcamInput, pInputMap);

        fillIdx++;
    }

    DumpQCarCamQueryInputs(pQcarcamInputs, fillIdx);

    AIS_LOG(ENGINE, MED, "filled_size=%d", fillIdx);
    *pRetSize = fillIdx;

    return rc;
}

/**
 * QueryInputsV2
 *
 * @brief Query available inputs
 *
 * @param p_inputs
 * @param size - size of p_inputs array
 * @param filled_size - If p_inputs is set, number of elements in array that were filled
 *                      If p_inputs is NULL, number of available inputs to query
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::QueryInputsV2(qcarcam_input_v2_t* pQcarcamInputs, unsigned int nSize,
        unsigned int* pRetSize)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 idx;
    uint32 fillIdx;
    /*cache locally as we may be in deferred detect*/
    uint32 nInputMapping = m_nInputMapping;

    if (!pQcarcamInputs)
    {
        fillIdx = 0;
        for (idx = 0; idx < nInputMapping; idx++)
        {
            if (m_InputMappingTable[idx].isAvailable)
            {
                fillIdx++;
            }
        }
        AIS_LOG(ENGINE, MED, "counted_size=%d", fillIdx);
        *pRetSize = fillIdx;
        return CAMERA_SUCCESS;
    }

    for (idx = 0, fillIdx = 0; idx < nInputMapping && fillIdx < nSize; idx++)
    {
        qcarcam_input_v2_t* pQcarcamInput = &pQcarcamInputs[fillIdx];
        InputMappingType* pInputMap = &m_InputMappingTable[idx];

        /*do not advertise if it is not available*/
        if (!pInputMap->isAvailable)
            continue;

        memset(pQcarcamInput, 0x0, sizeof(*pQcarcamInput));

        pQcarcamInput->desc = (qcarcam_input_desc_t)pInputMap->sInfo.aisInputId;

        if (pInputMap->sInfo.opMode == QCARCAM_OPMODE_PAIRED_INPUT)
        {
            pQcarcamInput->flags |= QCARCAM_INPUT_FLAG_PAIRED;
        }

        FillQCarCamInputResInfoV2(pQcarcamInput, pInputMap);

        fillIdx++;
    }

    DumpQCarCamQueryInputsV2(pQcarcamInputs, fillIdx);

    AIS_LOG(ENGINE, MED, "filled_size=%d", fillIdx);
    *pRetSize = fillIdx;

    return rc;
}

/**
 * FillQCarCamInputInfo
 *
 * @brief Translates input stream info to QcarCam input
 *
 * @param pQcarcamInput   QCarCam input structure to be filled
 * @param pInputMap       Input mapping definiiton
 *
 * @return CameraResult
 */
void AisInputConfigurer::FillQCarCamInputResInfo(qcarcam_input_t* pQcarcamInput, InputMappingType* pInputMap)
{
    for (uint32 streamId = 0; streamId < pInputMap->numStreams; streamId++)
    {
        uint32 devIdx = pInputMap->devIdx[streamId];

        InputDeviceType* pInputDev = &m_InputDevices[devIdx];

        /*get cid*/
        for (uint32 subchanIdx = 0; subchanIdx < pInputDev->subchannelsInfo.num_subchannels; subchanIdx++)
        {
            Camera_Sensor_SubchannelType* pSubchannel = &pInputDev->subchannelsInfo.subchannels[subchanIdx];

            if (pSubchannel->src_id ==
                    pInputMap->sInfo.inputSrc[streamId].srcId)
            {
                uint32 modeIdx;

                if (streamId == 0)
                {
                    for (modeIdx = 0; modeIdx < pSubchannel->num_modes; modeIdx++)
                    {
                        pQcarcamInput->res[modeIdx].width = pSubchannel->modes[modeIdx].res.width;
                        pQcarcamInput->res[modeIdx].height = pSubchannel->modes[modeIdx].res.height;
                        pQcarcamInput->res[modeIdx].fps = pSubchannel->modes[modeIdx].res.fps;

                        pQcarcamInput->color_fmt[modeIdx] = pSubchannel->modes[modeIdx].fmt;
                    }

                    pQcarcamInput->num_color_fmt = modeIdx;
                    pQcarcamInput->num_res = modeIdx;

                   if (pInputDev->csiInfo[pSubchannel->csi_idx].isSecure)
                   {
                       pQcarcamInput->flags |= QCARCAM_INPUT_FLAG_CONTENT_PROTECTED;
                   }
                }
                else if ((streamId == 1) && (pQcarcamInput->flags & QCARCAM_INPUT_FLAG_PAIRED))
                {
                    for (modeIdx = 0; modeIdx < pSubchannel->num_modes; modeIdx++)
                    {
                        pQcarcamInput->res[modeIdx].width += pSubchannel->modes[modeIdx].res.width;
                    }
                }
                break;
            }
        }
    }
}

void AisInputConfigurer::DumpQCarCamQueryInputs(qcarcam_input_t* pQcarcamInputs, unsigned int nSize)
{
    for (uint32 idx = 0; idx < nSize; idx++)
    {
        qcarcam_input_t* pQcarcamInput = &pQcarcamInputs[idx];

        AIS_LOG(ENGINE, MED, "%d - input_desc=%d : flags=0x%08x, num_res=%d", idx,
                pQcarcamInput->desc, pQcarcamInput->flags, pQcarcamInput->num_res);

        for (uint32 modeIdx = 0; modeIdx < pQcarcamInput->num_res; modeIdx++)
        {
            AIS_LOG(ENGINE, MED, "\t mode[%d]: %dx%d fmt:0x%08x fps:%.2f", modeIdx,
                    pQcarcamInput->res[modeIdx].width, pQcarcamInput->res[modeIdx].height,
                    pQcarcamInput->color_fmt[modeIdx], pQcarcamInput->res[modeIdx].fps);
        }
    }
}

/**
 * FillQCarCamInputInfo
 *
 * @brief Translates input stream info to QcarCam input
 *
 * @param pQcarcamInput   QCarCam input structure to be filled
 * @param pInputMap       Input mapping definiiton
 *
 * @return CameraResult
 */
void AisInputConfigurer::FillQCarCamInputResInfoV2(qcarcam_input_v2_t* pQcarcamInput, InputMappingType* pInputMap)
{
    for (uint32 streamId = 0; streamId < pInputMap->numStreams; streamId++)
    {
        uint32 devIdx = pInputMap->devIdx[streamId];

        InputDeviceType* pInputDev = &m_InputDevices[devIdx];

        /*get cid*/
        for (uint32 subchanIdx = 0; subchanIdx < pInputDev->subchannelsInfo.num_subchannels; subchanIdx++)
        {
            Camera_Sensor_SubchannelType* pSubchannel = &pInputDev->subchannelsInfo.subchannels[subchanIdx];

            if (pSubchannel->src_id ==
                    pInputMap->sInfo.inputSrc[streamId].srcId)
            {
                uint32 modeIdx;

                if (streamId == 0)
                {
                    for (modeIdx = 0; modeIdx < pSubchannel->num_modes; modeIdx++)
                    {
                        pQcarcamInput->modes[modeIdx].res = pSubchannel->modes[modeIdx].res;
                        pQcarcamInput->modes[modeIdx].fmt = pSubchannel->modes[modeIdx].fmt;
                    }

                    pQcarcamInput->num_modes = modeIdx;

                   if (pInputDev->csiInfo[pSubchannel->csi_idx].isSecure)
                   {
                       pQcarcamInput->flags |= QCARCAM_INPUT_FLAG_CONTENT_PROTECTED;
                   }
                }
                else if ((streamId == 1) && (pQcarcamInput->flags & QCARCAM_INPUT_FLAG_PAIRED))
                {
                    for (modeIdx = 0; modeIdx < pSubchannel->num_modes; modeIdx++)
                    {
                        pQcarcamInput->modes[modeIdx].res.width += pSubchannel->modes[modeIdx].res.width;
                    }
                }
                break;
            }
        }
    }
}

void AisInputConfigurer::DumpQCarCamQueryInputsV2(qcarcam_input_v2_t* pQcarcamInputs, unsigned int nSize)
{
    for (uint32 idx = 0; idx < nSize; idx++)
    {
        qcarcam_input_v2_t* pQcarcamInput = &pQcarcamInputs[idx];

        AIS_LOG(ENGINE, MED, "%d - input_desc=%d : flags=0x%08x, num_modes=%d", idx,
                pQcarcamInput->desc, pQcarcamInput->flags, pQcarcamInput->num_modes);

        for (uint32 modeIdx = 0; modeIdx < pQcarcamInput->num_modes; modeIdx++)
        {
            AIS_LOG(ENGINE, MED, "\t mode[%d]: %dx%d fmt:0x%08x fps:%.2f", modeIdx,
                    pQcarcamInput->modes[modeIdx].res.width, pQcarcamInput->modes[modeIdx].res.height,
                    pQcarcamInput->modes[modeIdx].fmt, pQcarcamInput->modes[modeIdx].res.fps);
        }
    }
}

/**
 * ValidateInputId
 *
 * @brief Validate input id based on connected sensors
 *
 * @param pUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisInputConfigurer::ValidateInputId(AisUsrCtxt* pUsrCtxt)
{
    CameraResult result = CAMERA_EBADPARM;
    uint32 streamIdx;
    uint32 numFound = 0;

    /* Check all streams are available and keep count
     * At end, if number of streams available matches number of streams, return success
     */
    for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams; streamIdx++)
    {
        AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];

        if (m_InputDevices[pStream->inputCfg.devId].src_id_enable_mask & (1 << pStream->inputCfg.srcId))
        {
            numFound++;
        }
    }

    if (numFound == pUsrCtxt->m_numStreams)
    {
        result = CAMERA_SUCCESS;
    }

    return result;
}

/**
 * FillInputDeviceDiagInfo
 *
 * @brief update the input device initial statistics in input diagnostic info
 *
 * @param pInputDeviceInfo input device statistics info
 *
 * @return None
 */
void AisInputConfigurer::FillInputDeviceDiagInfo(QCarCamDiagInputDevInfo* pInputDeviceInfo)
{
    for (uint32 devIdx = 0; devIdx < m_nInputDevices;++devIdx)
    {
        QCarCamDiagInputDevInfo* pInputDeviceDiagInfo = &pInputDeviceInfo[devIdx];
        pInputDeviceDiagInfo->state = (uint32)m_InputDevices[devIdx].state;
        pInputDeviceDiagInfo->srcIdEnableMask = m_InputDevices[devIdx].src_id_enable_mask;

        for (uint32 i = 0; i < m_InputDevices[devIdx].subchannelsInfo.num_subchannels; i++)
        {
            pInputDeviceDiagInfo->inputSourceInfo[i].inputSrcId =
                m_InputDevices[devIdx].subchannelsInfo.subchannels[i].src_id;
            pInputDeviceDiagInfo->inputSourceInfo[i].status =
                pInputDeviceDiagInfo->srcIdEnableMask & (1 << pInputDeviceDiagInfo->inputSourceInfo[i].inputSrcId);
            pInputDeviceDiagInfo->inputSourceInfo[i].sensorMode.fmt =
                m_InputDevices[devIdx].subchannelsInfo.subchannels[i].modes[0].fmt;
            pInputDeviceDiagInfo->inputSourceInfo[i].sensorMode.res.fps =
                m_InputDevices[devIdx].subchannelsInfo.subchannels[i].modes[0].res.fps;
            pInputDeviceDiagInfo->inputSourceInfo[i].sensorMode.res.width =
                m_InputDevices[devIdx].subchannelsInfo.subchannels[i].modes[0].res.width;
            pInputDeviceDiagInfo->inputSourceInfo[i].sensorMode.res.height =
                m_InputDevices[devIdx].subchannelsInfo.subchannels[i].modes[0].res.height;
        }
    }
}

/**
 * SensorRecovery
 *
 * @brief Execute sensor specific recovery sequence.
 *
 * @param deviceId ID of device.
 * @param severity Error severity.
 *
 * @return None
 */
CameraResult AisInputConfigurer::SensorRecovery(uint32 devId, uint32 severity)
{
    CameraResult rc = CAMERA_EFAILED;

    if (devId < MAX_CAMERA_INPUT_DEVICES && m_InputDevices[devId].hDevice)
    {
        rc = CameraDeviceControl(m_InputDevices[devId].hDevice,
                            Camera_Sensor_AEEUID_CTL_STATE_RECOVERY,
                            &severity, sizeof(severity), NULL, 0, NULL);
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Invalid deviceId %d", devId);
    }

    return rc;
}

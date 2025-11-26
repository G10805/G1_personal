/*!
 * Copyright (c) 2016-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "ais_engine.h"
#include "ais_res_mgr.h"
#include "ais_buffer_manager.h"
#include "ais_ife_configurer.h"
#include "ais_input_configurer.h"
#include "ais_csi_configurer.h"
#include "ais_proc_chain.h"
#include "CameraDeviceManager.h"
#include "CameraEventLog.h"

#include "CameraPlatform.h"


//////////////////////////////////////////////////////////////////////////////////
/// MACRO DEFINES
//////////////////////////////////////////////////////////////////////////////////
#define AIS_API CAM_API

#define AIS_API_ENTER() AIS_LOG_ENTER(ENGINE, MED, "");
#define AIS_API_ENTER_HNDL(_ctxt_) AIS_LOG_ENTER(ENGINE, MED, "%p", _ctxt_);


#define CHECK_AIS_CTXT_MAGIC(_hndl_) (AIS_CTXT_MAGIC == _hndl_->magic)

#define USR_CTXT_MAGIC 0xA1500000
#define USR_CTXT_MAGIC_MASK 0xFFF00000
#define CHECK_USR_CTXT_MAGIC(_hndl_) (USR_CTXT_MAGIC == (_hndl_ & USR_CTXT_MAGIC_MASK))

#define USR_CTXT_IDX_MASK  0x0000FE00
#define USR_CTXT_IDX_SHIFT 9

#define USR_CTXT_RAND_MASK 0x000F01FF

#define USR_IDX_FROM_HNDL(_hndl_) (((_hndl_ & USR_CTXT_IDX_MASK) >> USR_CTXT_IDX_SHIFT) - 1)

#define USR_NEW_HNDL(_idx_) \
    (USR_CTXT_MAGIC | \
    (rand() & USR_CTXT_RAND_MASK) | \
    (USR_CTXT_IDX_MASK & ((_idx_ + 1) << USR_CTXT_IDX_SHIFT)))

#define NANOSEC_TO_SEC 1000000000L

/**
 * Threshold in ns for read of field info to be valid.
 * Beyond this threshold, we would not be confident in the reliability of the information read back
 */
#define AIS_FIELD_READ_TIME_THRESHOLD 16000000L


#define USR_CTXT_SET_MASK(_bitmask_, _param_) ((_bitmask_) |= 1 << (_param_))
#define USR_CTXT_CHECK_MASK(_bitmask_, _param_) ((_bitmask_) & (1 << (_param_)))


#define CAMERA_POWER_SUSPEND_DEFAULT CAMERA_POWER_DOWN
#define CAMERA_POWER_RESUME_DEFAULT CAMERA_POWER_UP

//////////////////////////////////////////////////////////////////////////////////
/// FORWARD DECLARE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////
static void UsrCtxtRetryRecoveryTimerHandler(void *pData);
static void UsrCtxtRecoveryTimeoutTimerHandler(void *data);
static CameraResult HandleAISErrorEvent(AisUsrCtxt* pUsrCtxt, void* pData);
static CameraResult AisUsrCtxtDecRefCnt(AisUsrCtxt* pUsrCtxt, void* pUsrData);
static CameraResult CompleteRecovery(AisUsrCtxt* pUsrCtxt, void* pData);
static CameraResult AisUsrCtxtSendInputStatus(AisUsrCtxt* pUsrCtxt, void* pData);
static CameraResult AisUsrCtxtSendVendorEvent(AisUsrCtxt* pUsrCtxt, void* pData);
static CameraResult AisUsrCtxtSendNotificationCb(AisUsrCtxt* pUsrCtxt, void* pData);

//////////////////////////////////////////////////////////////////////////////////
/// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
/// FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////
/**
 * AisCreateJobId
 *
 * @brief Creates unique job Id based on user context ID, stream index and frame input
 *
 * @param pUsrCtxt        User context
 * @param streamIdx       stread index
 * @param frameId         frame id
 * @param ifeCore         IFE core
 * @param ifeOutput       IFE output
 *
 * @return jobId
 */
static inline uint64 AisCreateJobId(AisUsrCtxt* pUsrCtxt, uint32 streamIdx, uint32 frameId, uint8 ifeCore, uint8 ifeOutput)
{
    uint64 jobId = ((uint64)(pUsrCtxt->m_mapIdx & 0xFF) << 56) | ((uint64)(pUsrCtxt->m_inputId & 0xFF) << 48) |
            ((uint64)(streamIdx & 0xF) << 40) |
            ((uint64)(ifeCore & 0xF) << 36) |
            ((uint64)(ifeOutput & 0xF) << 32) |
            frameId;

    return jobId;
}


/**
 * Deinitialize
 *
 * @brief Deinitialize AIS Engine & frees resources
 *
 * @return n/a
 */
void AisEngine::Deinitialize(void)
{
    int i;
    CameraResult rc = CAMERA_SUCCESS;

    m_eventHandlerIsExit = TRUE;

    CameraSetSignal(m_delaySuspend);

    for (i = 0; i < NUM_EVENT_HNDLR_POOL; i++)
    {
        CameraSetSignal(m_eventHandlerSignal);
    }

    for (i = 0; i < NUM_EVENT_HNDLR_POOL; i++)
    {
        if (m_eventHandlerThreadId[i])
        {
            CameraJoinThread(m_eventHandlerThreadId[i], NULL);
            CameraReleaseThread(m_eventHandlerThreadId[i]);
            m_eventHandlerThreadId[i] = NULL;
        }
    }

    CameraDestroySignal(m_eventHandlerSignal);
    m_eventHandlerSignal = NULL;

    CameraDestroySignal(m_delaySuspend);
    m_delaySuspend = NULL;

    for (i = 0; i < AIS_ENGINE_QUEUE_MAX; i++)
    {
        if (m_eventQ[i])
        {
            CameraQueueDestroy(m_eventQ[i]);
        }
    }

    //Stop Diag
    if (m_DiagManager)
    {
        m_DiagManager->DestroyInstance();
        m_DiagManager = NULL;
    }

    rc = CameraPlatformClockEnable(CAM_CLOCK_OVERALL);

    if (CAMERA_SUCCESS == rc)
    {
        // Stopping CSIphy and sensors for multiSoC
        if (m_Configurers[AIS_CONFIGURER_INPUT] && m_Configurers[AIS_CONFIGURER_CSI])
        {
            ((AisInputConfigurer*)m_Configurers[AIS_CONFIGURER_INPUT])->GlobalStop();
            ((AisCSIConfigurer*)m_Configurers[AIS_CONFIGURER_CSI])->GlobalStop();
        }

        ((AisInputConfigurer*)m_Configurers[AIS_CONFIGURER_INPUT])->DestroyInstance();
        ((AisCSIConfigurer*)m_Configurers[AIS_CONFIGURER_CSI])->DestroyInstance();
        ((AisIFEConfigurer*)m_Configurers[AIS_CONFIGURER_IFE])->DestroyInstance();

        CameraPlatformClockDisable(CAM_CLOCK_OVERALL);
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "CameraPlatformClockEnable failed before deinit, skip deinit");
    }

    if (m_ResourceManager)
    {
        m_ResourceManager->DestroyInstance();
        m_ResourceManager = NULL;
    }

    if (m_ProcChainManager)
    {
        m_ProcChainManager->DestroyInstance();
        m_ProcChainManager = NULL;
    }

    if (m_DeviceManager)
    {
        m_DeviceManager->DestroyInstance();
        m_DeviceManager = NULL;
    }

    if (m_LatencyMeasurementMode > CAMERA_LM_MODE_DISABLE)
    {
        CameraDisableMPMTimer();
    }

    CameraDestroyMutex(m_mutex);
    CameraDestroyMutex(m_usrCtxtMapMutex);
    CameraDestroyMutex(m_evtListnrMutex);

    CameraPlatformDeinit();

    m_state = AIS_ENGINE_STATE_UNINITIALIZED;
}

AisEngine::AisEngine()
{
    time_t t;
    srand((unsigned) time(&t));

    m_magic = AIS_CTXT_MAGIC;
    m_state = AIS_ENGINE_STATE_UNINITIALIZED;
    m_clientCount = 0;

    m_mutex = NULL;
    m_delaySuspend = NULL;

    m_isPowerGranular = FALSE;
    m_isMultiSoc = FALSE;
    m_PowerManagerPolicyType = CAMERA_PM_POLICY_LPM_EVENT_ALL;
    m_LatencyMeasurementMode = CAMERA_LM_MODE_DISABLE;

    m_eventHandlerSignal = NULL;
    std_memset(m_eventHandlerThreadId, 0x0, sizeof(m_eventHandlerThreadId));
    m_eventHandlerIsExit = FALSE;
    std_memset(m_eventQ, 0x0, sizeof(m_eventQ));
    m_usrCtxtMapMutex = NULL;
    m_evtListnrMutex = NULL;

    std_memset(m_usrCtxtMap, 0x0, sizeof(m_usrCtxtMap));
    m_DeviceManager = NULL;

    m_ResourceManager = NULL;
    m_DiagManager = NULL;
    m_ProcChainManager = NULL;
    m_engineSettings = NULL;
    std_memset(m_Configurers, 0x0, sizeof(m_Configurers));
}

AisEngine::~AisEngine()
{

}

/**
 * CameraPowerEventCallback
 *
 * @brief Camera Power event callback function.
 *
 * @return CameraResult
 */
CameraResult AisEngine::CameraPowerEventCallback(CameraPowerEventType powerEventId, void* pUsrData)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisEngine* pEngine = (AisEngine*)pUsrData;

    if (!pEngine)
    {
        AIS_LOG(ENGINE, ERROR, "AisEngine not present");
        return CAMERA_EBADSTATE;
    }

    rc = pEngine->ProcessPowerEvent(powerEventId);

    return rc;
}

/**
 * Initialize
 *
 * @brief Initialize AIS Engine modules & resources
 *
 * @return CameraResult
 */
CameraResult AisEngine::Initialize(qcarcam_init_t* pInitParams)
{
    CameraResult rc = CAMERA_SUCCESS;
    int i;

    if (TRUE != CameraPlatformInit())
    {
        return CAMERA_EFAILED;
    }

    rc = CameraCreateMutex(&m_mutex);

    if (CAMERA_SUCCESS == rc)
    {
        rc = CameraCreateMutex(&m_usrCtxtMapMutex);
    }

    if (CAMERA_SUCCESS == rc)
    {
        rc = CameraCreateMutex(&m_evtListnrMutex);
    }

    if (CAMERA_SUCCESS == rc)
    {
        rc = CameraCreateSignal(&m_delaySuspend);
    }

    if (CAMERA_SUCCESS == rc)
    {
        rc = CameraPlatformRegisterPowerCallback(AisEngine::CameraPowerEventCallback, this);
    }

    if (CAMERA_SUCCESS == rc)
    {
        CameraQueueCreateParamType sCreateParams = {};

        /*create buffer done Q*/
        sCreateParams.nCapacity = EVENT_QUEUE_MAX_SIZE;
        sCreateParams.nDataSizeInBytes = sizeof(AisEventMsgType);
        sCreateParams.eLockType = CAMERAQUEUE_LOCK_THREAD;
        for (i = 0; i < AIS_ENGINE_QUEUE_MAX; i++)
        {
            rc = CameraQueueCreate(&m_eventQ[i], &sCreateParams);
            AIS_LOG_ON_ERR(ENGINE, rc, "Failed to create event queue: %d", rc);
        }
    }

    if (CAMERA_SUCCESS == rc)
    {
        rc = CameraCreateSignal(&m_eventHandlerSignal);
        AIS_LOG_ON_ERR(ENGINE, rc, "Failed to create signal: %d", rc);
    }

    if (CAMERA_SUCCESS == rc)
    {
        m_eventHandlerIsExit = FALSE;

        for (i = 0; i < NUM_EVENT_HNDLR_POOL; i++)
        {
            char name[64];
            snprintf(name, sizeof(name), "engine_evnt_hndlr_%d", i);

            if (0 !=  CameraCreateThread(CAMERA_THREAD_PRIO_HIGH_REALTIME,
                    0,
                    AisEngine::EventHandler,
                    this,
                    0x8000,
                    name,
                    &m_eventHandlerThreadId[i]))
            {
                AIS_LOG(ENGINE, ERROR, "CameraCreateThread failed");
                rc = CAMERA_EFAILED;
                break;
            }
        }
    }

    if (CAMERA_SUCCESS == rc)
    {
        m_DeviceManager = CameraDeviceManager::CreateInstance();
        if (!m_DeviceManager)
        {
            rc = CAMERA_EFAILED;
        }
    }

    //Multi-SOC Initialization
    if (CAMERA_SUCCESS == rc)
    {
        // Get Multi SOC Environment
        m_isMultiSoc = CameraPlatform_GetMultiSocEnv();

        // Initialize global ctxt
        for (i = 0; i < MAX_CAMERA_INPUT_CHANNELS; i++)
        {
            m_GlobalCtxt.m_inputId[i] = QCARCAM_INPUT_MAX;
            m_GlobalCtxt.m_numStreams[i] = 0;
            memset(&m_GlobalCtxt.m_streams[i], 0x0, sizeof(m_GlobalCtxt.m_streams[i]));
        }
    }

    if (CAMERA_SUCCESS == rc)
    {
        CameraLogEvent(CAMERA_ENGINE_EVENT_CONFIGURERS_INIT_START, AIS_CONFIGURER_INPUT, 0);
        m_Configurers[AIS_CONFIGURER_INPUT] = AisInputConfigurer::CreateInstance();
        if (!m_Configurers[AIS_CONFIGURER_INPUT])
        {
            rc = CAMERA_EFAILED;
        }
        CameraLogEvent(CAMERA_ENGINE_EVENT_CONFIGURERS_INIT_DONE, AIS_CONFIGURER_INPUT, 0);
    }

    if (CAMERA_SUCCESS == rc)
    {
        CameraLogEvent(CAMERA_ENGINE_EVENT_CONFIGURERS_INIT_START, AIS_CONFIGURER_IFE, 0);
        m_Configurers[AIS_CONFIGURER_IFE] = AisIFEConfigurer::CreateInstance();
        if (!m_Configurers[AIS_CONFIGURER_IFE])
        {
            rc = CAMERA_EFAILED;
        }
        CameraLogEvent(CAMERA_ENGINE_EVENT_CONFIGURERS_INIT_DONE, AIS_CONFIGURER_IFE, 0);
    }

    if (CAMERA_SUCCESS == rc)
    {
        CameraLogEvent(CAMERA_ENGINE_EVENT_CONFIGURERS_INIT_START, AIS_CONFIGURER_CSI, 0);
        m_Configurers[AIS_CONFIGURER_CSI] = AisCSIConfigurer::CreateInstance();
        if (!m_Configurers[AIS_CONFIGURER_CSI])
        {
            rc = CAMERA_EFAILED;
        }
        CameraLogEvent(CAMERA_ENGINE_EVENT_CONFIGURERS_INIT_DONE, AIS_CONFIGURER_CSI, 0);
    }

    // Configure everything and start csiphy for multi SOC
    if (CAMERA_SUCCESS == rc)
    {
        for (i = 0; i < AIS_CONFIGURER_MAX && CAMERA_SUCCESS == rc; i++)
        {
             AisConfigurerType global_config_seq[AIS_CONFIGURER_MAX] =
                 {AIS_CONFIGURER_INPUT, AIS_CONFIGURER_CSI, AIS_CONFIGURER_IFE};
             rc = m_Configurers[global_config_seq[i]]->GlobalConfig(&m_GlobalCtxt);
        }

        if (CAMERA_SUCCESS == rc)
        {
            rc = AisCSIConfigurer::GetInstance()->GlobalStart();
        }
    }

    // Initialize resource manager
    if (CAMERA_SUCCESS == rc)
    {
        m_ResourceManager = AisResourceManager::CreateInstance();
        if (!m_ResourceManager)
        {
            rc = CAMERA_EFAILED;
        }
    }

    // Initialize Diagnostic Manager
    if (CAMERA_SUCCESS == rc)
    {
        m_DiagManager = AisDiagManager::CreateInstance();
        if (!m_DiagManager)
        {
            rc = CAMERA_EFAILED;
        }
    }

    if (CAMERA_SUCCESS == rc)
    {
        m_ProcChainManager = AisProcChainManager::CreateInstance();
        if (!m_ProcChainManager)
        {
            rc = CAMERA_EFAILED;
        }
    }

    if (CAMERA_SUCCESS == rc)
    {
        m_engineSettings = CameraPlatformGetEngineSettings();
        if (m_engineSettings == NULL)
        {
            rc = CAMERA_EBADCLASS;
        }
    }

    if (CAMERA_SUCCESS == rc)
    {
        CameraLatencyMeasurementModeType latencyMeasurementMode = m_engineSettings->latencyMeasurementMode;

        if (latencyMeasurementMode >= CAMERA_LM_MODE_TYPE_MAX)
        {
            m_LatencyMeasurementMode = CAMERA_LM_MODE_ALL_STEPS;
        }
        else
        {
            m_LatencyMeasurementMode = latencyMeasurementMode;
        }

        if (m_LatencyMeasurementMode > CAMERA_LM_MODE_DISABLE)
        {
            rc = CameraEnableMPMTimer();
        }

        AIS_LOG(ENGINE, HIGH, "m_LatencyMeasurementMode %d", m_LatencyMeasurementMode);
    }

    if (CAMERA_SUCCESS == rc)
    {
        CameraPowerManagerPolicyType powerManagementPolicy = m_engineSettings->powerManagementPolicy;

        if (powerManagementPolicy >= CAMERA_PM_POLICY_TYPE_MAX)
        {
            m_PowerManagerPolicyType = CAMERA_PM_POLICY_LPM_EVENT_ALL;
        }
        else
        {
            m_PowerManagerPolicyType = powerManagementPolicy;
        }

        if (pInitParams && (pInitParams->flags & AIS_INITIALIZE_POWER_INPUT_GRANULAR))
        {
            m_isPowerGranular = TRUE;
        }

        AIS_LOG(ENGINE, HIGH, "m_PowerManagerPolicyType %d m_isPowerGranular %d", m_PowerManagerPolicyType, m_isPowerGranular);

        if (pInitParams && (pInitParams->flags & AIS_INITIALIZE_DEFER_INPUT_DETECTION))
        {
            m_state = AIS_ENGINE_STATE_DETECTION_PENDING;
        }
        else
        {
            CameraLogEvent(CAMERA_ENGINE_EVENT_INPUT_PROBE, 0, 0);
            rc = AisInputConfigurer::GetInstance()->DetectAll();
            CameraLogEvent(CAMERA_ENGINE_EVENT_INPUT_PROBE_DONE, 0, rc);

            m_state = AIS_ENGINE_STATE_SUSPEND_PENDING;
        }
    }

    if (CAMERA_SUCCESS == rc)
    {
        CameraResetSignal(m_delaySuspend);

        AisEventMsgType sEvent;
        sEvent.eventId = (m_state == AIS_ENGINE_STATE_SUSPEND_PENDING) ?
                AIS_EVENT_DELAYED_SUSPEND : AIS_EVENT_DEFER_INPUT_DETECT;
        if (sEvent.eventId == AIS_EVENT_DELAYED_SUSPEND && m_PowerManagerPolicyType == CAMERA_PM_POLICY_LPM_EVENT_ALL)
        {
            AIS_LOG(ENGINE, HIGH, "no need suspend in PM event mode");
        }
        else
        {
            rc = QueueEvent(&sEvent);
            if (CAMERA_SUCCESS != rc)
            {
                //If fail to enqueue event, then suspend right away
                AIS_LOG(ENGINE, ERROR, "Failed (%d) to enqueue delayed suspend. Will suspend right away", rc);
                if (m_PowerManagerPolicyType == CAMERA_PM_POLICY_LPM_EVENT_FOR_INPUTS)
                {
                    PowerSuspend(CAMERA_POWER_SOC_CLK_OFF, FALSE);
                }
                else
                {
                    PowerSuspend(CAMERA_POWER_SUSPEND_DEFAULT, FALSE);
                }
            }
        }
    }

    if (CAMERA_SUCCESS == rc)
    {
        CameraLogEvent(CAMERA_ENGINE_EVENT_INITIALIZE_DONE, 0, 0);
    }
    else
    {
        /*clean up on error*/
        Deinitialize();
    }

    return rc;
}

/**
 * ais_initialize
 *
 * @brief Initialize AIS context, modules & resources
 *
 * @return CameraResult
 */
CameraResult AisEngine::ais_initialize(qcarcam_init_t* p_init_params)
{
    return Initialize(p_init_params);
}

/**
 * ais_uninitialize
 *
 * @brief Uninitialize AIS context, modules & resources
 *
 * @return CameraResult
 */
CameraResult AisEngine::ais_uninitialize(void)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (AIS_ENGINE_STATE_UNINITIALIZED != m_state)
    {
        int i;
        for (i = 0; i < AIS_MAX_USR_CONTEXTS; i++)
        {
            if (m_usrCtxtMap[i].in_use)
            {
                (void)ais_close((qcarcam_hndl_t)(m_usrCtxtMap[i].phndl));
            }
        }

        Deinitialize();

        //Clean up any user contexts in bad state
        for (i = 0; i < AIS_MAX_USR_CONTEXTS; i++)
        {
            if (m_usrCtxtMap[i].in_use)
            {
                AIS_LOG(ENGINE, ERROR, "Non destroyed client %d (%p %d) with refcount %d",
                        i, m_usrCtxtMap[i].phndl, m_usrCtxtMap[i].in_use, m_usrCtxtMap[i].refcount);

                AisUsrCtxt::DestroyUsrCtxt((AisUsrCtxt*)m_usrCtxtMap[i].usr_ctxt);
            }
        }
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Engine already uninitialized, should not happen");
        rc = CAMERA_EBADSTATE;
    }

    DestroyInstance();

    return rc;
}

/**
 * ProcessPowerEvent
 *
 * @brief Processes Power events depending on power policy of the engine
 *
 * @return CameraResult
 */
CameraResult AisEngine::ProcessPowerEvent(CameraPowerEventType powerEventId)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (powerEventId == CAMERA_POWER_SUSPEND || powerEventId == CAMERA_POWER_DOWN)
    {
        Lock();
        if (m_clientCount)
        {
            Unlock();
            AIS_LOG(ENGINE, ERROR, "Active clients present, m_clientCount: %d", m_clientCount);
            return CAMERA_EBADSTATE;
        }
        Unlock();

        if (m_PowerManagerPolicyType == CAMERA_PM_POLICY_LPM_EVENT_ALL)
        {
            rc = PowerSuspend(CAMERA_POWER_DOWN, FALSE);
        }
        else if (m_PowerManagerPolicyType == CAMERA_PM_POLICY_LPM_EVENT_FOR_INPUTS)
        {
            rc = PowerSuspend(CAMERA_POWER_SUSPEND, FALSE);
        }
        else
        {
            AIS_LOG(ENGINE, HIGH, "Not support handle LPM event");
        }
        AIS_LOG(ENGINE, HIGH, "PowerSuspend - S2R or Hibernation suspend with rc = %d", rc);
    }
    else
    {
        Lock();
        if (m_clientCount)
        {
            Unlock();
            AIS_LOG(ENGINE, ERROR, "Active clients already present, m_clientCount: %d", m_clientCount);
            return CAMERA_EBADSTATE;
        }
        Unlock();

        if (m_PowerManagerPolicyType != CAMERA_PM_POLICY_NO_LPM)
        {
            rc = PowerResume(powerEventId, FALSE);
        }
        else
        {
            AIS_LOG(ENGINE, HIGH, "Not support handle LPM event");
        }
        AIS_LOG(ENGINE, HIGH, "PowerResume - S2R or Hibernation restore with rc = %d", rc);
    }

    return rc;
}

/**
 * PowerSuspend
 *
 * @brief Goes into low power mode
 *
 * @return CameraResult
 */
CameraResult AisEngine::PowerSuspend(CameraPowerEventType powerEventId, boolean isPowerGranular)
{
    CameraResult rc = CAMERA_SUCCESS;
    int i = 0;

    if (TRUE == m_isMultiSoc)
    {
        AIS_LOG(ENGINE, HIGH, "Skip for multiSOC");
        return rc;
    }

    CameraLogEvent(CAMERA_ENGINE_EVENT_POWER_SUSPEND_START, 0, 0);

    switch (powerEventId)
    {
    case CAMERA_POWER_DOWN:
        for (i = 0; i < AIS_CONFIGURER_MAX && CAMERA_SUCCESS == rc; i++)
        {
            rc = m_Configurers[i]->PowerSuspend(NULL, FALSE, powerEventId);
        }

        rc = CameraSensorI2C_PowerDown();

        rc = CameraPlatformClockDisable(CAM_CLOCK_OVERALL);
        break;
    case CAMERA_POWER_SUSPEND:
        if (!isPowerGranular)
        {
            rc = m_Configurers[AIS_CONFIGURER_INPUT]->PowerSuspend(NULL, FALSE, powerEventId);
        }
        rc = CameraSensorI2C_PowerDown();
        break;
    case CAMERA_POWER_SOC_CLK_OFF:
        if (!isPowerGranular)
        {
            rc = m_Configurers[AIS_CONFIGURER_IFE]->PowerSuspend(NULL, FALSE, powerEventId);
            rc = m_Configurers[AIS_CONFIGURER_CSI]->PowerSuspend(NULL, FALSE, powerEventId);
        }
        CameraPlatformClockDisable(CAM_CLOCK_OVERALL);
        break;
    default:
        AIS_LOG(ENGINE, ERROR, "Unsupported power event ID %d", powerEventId);
        break;
    }

    rc = CAMERA_SUCCESS;

    CameraLogEvent(CAMERA_ENGINE_EVENT_POWER_SUSPEND_DONE, 0, rc);

    m_state = AIS_ENGINE_STATE_SUSPEND;

    return rc;
}

/**
 * PowerResume
 *
 * @brief Power back up
 *
 * @return CameraResult
 */
CameraResult AisEngine::PowerResume(CameraPowerEventType powerEventId, boolean isPowerGranular)
{
    CameraResult rc = CAMERA_SUCCESS;
    int i = 0;

    if (TRUE == m_isMultiSoc)
    {
        AIS_LOG(ENGINE, HIGH, "Skip for multiSOC");
        return rc;
    }
    CameraLogEvent(CAMERA_ENGINE_EVENT_POWER_RESUME_START, 0, 0);

    switch (powerEventId)
    {
    case CAMERA_POWER_UP:
    case CAMERA_POWER_POST_LPM:
    case CAMERA_POWER_POST_HIBERNATION:
        rc = CameraPlatformClockEnable(CAM_CLOCK_OVERALL);

        (void)CameraSensorI2C_PowerUp();

        if (!isPowerGranular)
        {
            for (i = 0; i < AIS_CONFIGURER_MAX && CAMERA_SUCCESS == rc; i++)
            {
                rc = m_Configurers[i]->PowerResume(NULL, FALSE, powerEventId);
            }
        }
        break;
    case CAMERA_POWER_RESUME:
        (void)CameraSensorI2C_PowerUp();
        if (!isPowerGranular)
        {
            rc = m_Configurers[AIS_CONFIGURER_INPUT]->PowerResume(NULL, FALSE, powerEventId);
        }
        break;
    case CAMERA_POWER_SOC_CLK_ON:
        rc = CameraPlatformClockEnable(CAM_CLOCK_OVERALL);
        if (!isPowerGranular)
        {
            rc |= m_Configurers[AIS_CONFIGURER_IFE]->PowerResume(NULL, FALSE, powerEventId);
            rc |= m_Configurers[AIS_CONFIGURER_CSI]->PowerResume(NULL, FALSE, powerEventId);
        }
        break;
    default:
        AIS_LOG(ENGINE, ERROR, "Unsupported power event ID %d", powerEventId);
        break;
    }

    CameraLogEvent(CAMERA_ENGINE_EVENT_POWER_RESUME_DONE, 0, rc);

    m_state = AIS_ENGINE_STATE_READY;

    return rc;
}

/**
 * AssignNewHndl
 *
 * @brief Assigns handle to new user context ptr
 *        Finds free slot in handle to usr_ctxt mapping table and
 *        creates new handle for user context using available index
 *
 * @param AisUsrCtxt
 *
 * @return uintptr_t - new handle
 */
uintptr_t AisEngine::AssignNewHndl(AisUsrCtxt* pUsrCtxt)
{
    uintptr_t hndl = 0;
    uint32 idx;

    CameraLockMutex(m_usrCtxtMapMutex);

    for (idx = 0; idx < AIS_MAX_USR_CONTEXTS; idx++)
    {
        if (!m_usrCtxtMap[idx].in_use)
        {
            m_usrCtxtMap[idx].in_use = AIS_USR_CTXT_MAP_USED;
            m_usrCtxtMap[idx].usr_ctxt = pUsrCtxt;
            m_usrCtxtMap[idx].phndl = USR_NEW_HNDL(idx);
            pUsrCtxt->m_mapIdx = idx;
            hndl = m_usrCtxtMap[idx].phndl;
            break;
        }
    }

    CameraUnlockMutex(m_usrCtxtMapMutex);

    if (idx == AIS_MAX_USR_CONTEXTS)
    {
        AIS_LOG(ENGINE, ERROR, "Exceed Max number of contexts (%d)", AIS_MAX_USR_CONTEXTS);
    }

    return hndl;
}

/**
 * ReleaseHndl
 *
 * @brief Frees index in mapping table used by user_hndl
 *
 * @note: user_hndl is assumed to be validated through GetUsrCtxt
 *
 * @param user_hndl
 *
 * @return n/a
 */
void AisEngine::ReleaseHndl(void* user_hndl)
{
    uint32 idx;
    uintptr_t hndl = (uintptr_t)user_hndl;

    idx = USR_IDX_FROM_HNDL(hndl);

    if (idx < AIS_MAX_USR_CONTEXTS)
    {
        AisUsrCtxt* usr_ctxt = NULL;

        CameraLockMutex(m_usrCtxtMapMutex);

        m_usrCtxtMap[idx].refcount--;

        if (0 == m_usrCtxtMap[idx].refcount)
        {
            usr_ctxt = (AisUsrCtxt*)m_usrCtxtMap[idx].usr_ctxt;
            memset(&m_usrCtxtMap[idx], 0x0, sizeof(m_usrCtxtMap[idx]));
        }
        else
        {
            m_usrCtxtMap[idx].in_use = AIS_USR_CTXT_MAP_DESTROY;
        }

        CameraUnlockMutex(m_usrCtxtMapMutex);

        if (usr_ctxt)
        {
            AisUsrCtxt::DestroyUsrCtxt(usr_ctxt);
        }
    }
}

/**
 * AisUsrctxtNotifyEventListener
 *
 * @brief Send cb events to subscribed streams
 *
 * @param input_id
 * @param evt_param
 * @return CameraResult SUCCESS if
 */
CameraResult AisEngine::AisUsrctxtNotifyEventListener(qcarcam_input_desc_t inputId, qcarcam_param_t evtParam)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisUsrCtxtList::iterator it;

    CameraLockMutex(m_evtListnrMutex);

    if (m_eventListener.find(inputId) == m_eventListener.end() ||
        (*m_eventListener[inputId]).empty())
    {
        AIS_LOG(ENGINE, DBG, "No other UsrCtxts for inputId %d", inputId);
    }
    else
    {
        for(it = (*m_eventListener[inputId]).begin();
            it != (*m_eventListener[inputId]).end() && CAMERA_SUCCESS == rc; it++)
        {
            AisUsrCtxt* pUsrCtxt = *it;

            rc = pUsrCtxt->IncRefCnt();
            if (CAMERA_SUCCESS == rc)
            {
                if (pUsrCtxt->m_notificationEvtMask & (1 << evtParam))
                {
                    AisEventMsgType pMsg;
                    pMsg.eventId = AIS_EVENT_USER_NOTIFICATION;
                    pMsg.payload.notifyParam.paramVal = evtParam;
                    pMsg.payload.notifyParam.pUsrCtxt = pUsrCtxt;
                    rc = QueueEvent(&pMsg);
                    if (CAMERA_SUCCESS != rc)
                    {
                        AIS_LOG(ENGINE, ERROR, "Failed (%d) to enqueue Notification event.", rc);
                    }
                }
                else
                {
                    pUsrCtxt->DecRefCnt();
                }
            }
        }
    }

    CameraUnlockMutex(m_evtListnrMutex);

    return rc;
}

/**
 * AisEvtListnerAddUsrhdl
 *
 * @brief Add usr hdl to Notifications listners list
 * @pUsrCtxt             Userctxt
 * @return n/a
 */

void AisEngine::AisEvtListnerAddUsrhdl(AisUsrCtxt* pUsrCtxt)
{
    CameraLockMutex(m_evtListnrMutex);
    if (m_eventListener.find(pUsrCtxt->m_inputId) == m_eventListener.end())
    {
        AisUsrCtxtList *pUsrList = new AisUsrCtxtList();
        (*pUsrList).push_back(pUsrCtxt);
        m_eventListener[pUsrCtxt->m_inputId] = pUsrList;
    }
    else
    {
        (*m_eventListener[pUsrCtxt->m_inputId]).push_back(pUsrCtxt);
    }
    CameraUnlockMutex(m_evtListnrMutex);
}

/**
 * AisEvtListnerRemoveUsrhdl
 *
 * @brief Add usr hdl to Notifications listners list
 * @pUsrCtxt             Userctxt
 * @return n/a
 */

void AisEngine::AisEvtListnerRemoveUsrhdl(AisUsrCtxt* pUsrCtxt)
{
    CameraLockMutex(m_evtListnrMutex);
    if (m_eventListener.find(pUsrCtxt->m_inputId) == m_eventListener.end() ||
        (*m_eventListener[pUsrCtxt->m_inputId]).empty())
    {
        AIS_LOG(ENGINE, ERROR, "Client is not present");
    }
    else
    {
        AisUsrCtxtList::iterator it;
        for(it = (*m_eventListener[pUsrCtxt->m_inputId]).begin(); it != (*m_eventListener[pUsrCtxt->m_inputId]).end(); it++)
        {
            if(*it == pUsrCtxt)
            {
                (*m_eventListener[pUsrCtxt->m_inputId]).erase(it);
                if ((*m_eventListener[pUsrCtxt->m_inputId]).empty())
                {
                    delete m_eventListener[pUsrCtxt->m_inputId];
                    m_eventListener.erase(pUsrCtxt->m_inputId);
                }
                break;
            }
        }
    }
    CameraUnlockMutex(m_evtListnrMutex);
}

/**
 * AisEvtSetMaster
 *
 * @brief Add usr hdl as master
 * @pUsrCtxt             Userctxt
 * @return CameraResult
 */

CameraResult AisEngine::AisEvtSetMaster(AisUsrCtxt* pUsrCtxt, boolean isUsrSet)
{
    CameraResult rc = CAMERA_SUCCESS;

    CameraLockMutex(m_evtListnrMutex);
    if(m_masterHdl.find(pUsrCtxt->m_inputId) == m_masterHdl.end())
    {
        m_masterHdl[pUsrCtxt->m_inputId] = pUsrCtxt;
        pUsrCtxt->m_isMaster = TRUE;
        pUsrCtxt->m_isUsrSetMaster = isUsrSet;
        rc = CAMERA_SUCCESS;
    }
    else if(m_masterHdl[pUsrCtxt->m_inputId] == pUsrCtxt)
    {
        pUsrCtxt->m_isUsrSetMaster = isUsrSet;
        AIS_LOG(ENGINE, ERROR, "This client is already master");
        rc = CAMERA_SUCCESS;
    }
    else
    {
        if(!(m_masterHdl[pUsrCtxt->m_inputId]->m_isUsrSetMaster) && isUsrSet)
        {
            m_masterHdl[pUsrCtxt->m_inputId]->m_isMaster = FALSE;
            m_masterHdl[pUsrCtxt->m_inputId]->m_isUsrSetMaster = FALSE;

            m_masterHdl[pUsrCtxt->m_inputId] = pUsrCtxt;
            pUsrCtxt->m_isMaster = TRUE;
            pUsrCtxt->m_isUsrSetMaster = isUsrSet;
            rc = CAMERA_SUCCESS;
        }
        else
        {
            AIS_LOG(ENGINE, ERROR, "Master is already set");
            rc = CAMERA_EALREADY;
        }
    }
    CameraUnlockMutex(m_evtListnrMutex);

    return rc;
}

/**
 * AisEvtReleaseMaster
 *
 * @brief Release usr hdl as master
 * @pUsrCtxt             Userctxt
 * @return Cameraresult
 */

CameraResult AisEngine::AisEvtReleaseMaster(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;

    if(pUsrCtxt->m_isMaster)
    {
        CameraLockMutex(m_evtListnrMutex);
        m_masterHdl.erase(pUsrCtxt->m_inputId);
        pUsrCtxt->m_isMaster = FALSE;
        pUsrCtxt->m_isUsrSetMaster = FALSE;
        CameraUnlockMutex(m_evtListnrMutex);
        rc = AisUsrctxtNotifyEventListener(pUsrCtxt->m_inputId, QCARCAM_PARAM_MASTER);
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "This client is not master");
        rc = CAMERA_EBADSTATE;
    }

    return rc;
}


/**
 * GetUsrCtxt
 *
 * @brief Get user context pointer from handle
 *
 * @param user_hndl
 *
 * @return AisUsrCtxt*  - NULL if fail
 */
AisUsrCtxt* AisEngine::GetUsrCtxt(void* user_hndl)
{
    uint32 idx;
    uintptr_t hndl = (uintptr_t)user_hndl;

    if (!CHECK_USR_CTXT_MAGIC(hndl))
    {
        AIS_LOG(ENGINE, ERROR, "invalid hndl ptr %p", hndl);
        return NULL;
    }

    idx = USR_IDX_FROM_HNDL(hndl);

    CameraLockMutex(m_usrCtxtMapMutex);
    if(idx < AIS_MAX_USR_CONTEXTS &&
       hndl == m_usrCtxtMap[idx].phndl &&
       m_usrCtxtMap[idx].in_use)
    {
        m_usrCtxtMap[idx].refcount++;
        CameraUnlockMutex(m_usrCtxtMapMutex);
        return (AisUsrCtxt*)m_usrCtxtMap[idx].usr_ctxt;
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "invalid hndl ptr %p", hndl);
    }

    CameraUnlockMutex(m_usrCtxtMapMutex);

    return NULL;
}

/**
 * PutUsrCtxt
 *
 * @brief relinquishes user ctxt
 *
 * @param usr_ctxt
 *
 * @return none
 */
void AisEngine::PutUsrCtxt(AisUsrCtxt* pUsrCtxt)
{
    RelinquishUsrCtxt(pUsrCtxt->m_mapIdx);
}

/**
 * AcquireUsrCtxt
 *
 * @brief Increments refcount of usr_ctxt handle if handle is in use
 *
 * @param idx
 *
 * @return AisUsrCtxt*  - NULL if fail
 */
AisUsrCtxt* AisEngine::AcquireUsrCtxt(unsigned int idx)
{
    if (idx < AIS_MAX_USR_CONTEXTS)
    {
        CameraLockMutex(m_usrCtxtMapMutex);
        if(AIS_USR_CTXT_MAP_USED == m_usrCtxtMap[idx].in_use)
        {
            m_usrCtxtMap[idx].refcount++;
            CameraUnlockMutex(m_usrCtxtMapMutex);

            return (AisUsrCtxt*)m_usrCtxtMap[idx].usr_ctxt;
        }
        CameraUnlockMutex(m_usrCtxtMapMutex);
    }

    return NULL;
}

/**
 * RelinquishUsrCtxt
 *
 * @brief Decrement refcount and cleanup if handle is being closed and last user
 *
 * @param idx
 *
 * @return none
 */
void AisEngine::RelinquishUsrCtxt(unsigned int idx)
{
    if (idx < AIS_MAX_USR_CONTEXTS)
    {
        AisUsrCtxt* usr_ctxt = NULL;

        CameraLockMutex(m_usrCtxtMapMutex);

        if (AIS_USR_CTXT_MAP_UNUSED != m_usrCtxtMap[idx].in_use)
        {
            m_usrCtxtMap[idx].refcount--;

            if (AIS_USR_CTXT_MAP_DESTROY == m_usrCtxtMap[idx].in_use &&
                !m_usrCtxtMap[idx].refcount)
            {
                m_usrCtxtMap[idx].in_use = AIS_USR_CTXT_MAP_UNUSED;
                usr_ctxt = (AisUsrCtxt*)m_usrCtxtMap[idx].usr_ctxt;
                m_usrCtxtMap[idx].usr_ctxt = NULL;
            }
        }

        CameraUnlockMutex(m_usrCtxtMapMutex);

        if (usr_ctxt)
        {
            AisUsrCtxt::DestroyUsrCtxt(usr_ctxt);
        }
    }
}

/**
 * IsInputAvailable
 *
 * @brief Checks the qcarcam input id is valid & available
 *
 * @param desc
 *
 * @return boolean - TRUE if valid
 */
boolean AisEngine::IsInputAvailable(qcarcam_input_desc_t desc)
{
    return (CAMERA_SUCCESS == AisInputConfigurer::GetInstance()->IsInputAvailable(desc));
}

/**
 * GetLatencyMeasurementMode
 *
 * @brief Get camera latency measurement mode type
 *
 * @return CameraLatencyMeasurementModeType
 */
CameraLatencyMeasurementModeType AisEngine::GetLatencyMeasurementMode()
{
    return m_LatencyMeasurementMode;
}

/**
 * InitMutex
 *
 * @brief Initialize user context mutexes
 *
 * @param usr_ctxt
 *
 * @return CameraResult
 */
CameraResult AisUsrCtxt::InitMutex(void)
{
    CameraResult rc;

    rc = CameraCreateMutex(&m_mutex);

    return rc;
}

/**
 * DeinitMutex
 *
 * @brief Deinitialize user context mutexes
 *
 * @param usr_ctxt
 *
 * @return CameraResult
 */
CameraResult AisUsrCtxt::DeinitMutex(void)
{
    CameraDestroyMutex(m_mutex);

    return CAMERA_SUCCESS;
}

/**
 * InitUserBufferLists
 *
 * @brief Initializes user buffer lists (i.e. INPUT and OUTPUT list)
 *
 * @return CameraResult
 */
CameraResult AisUsrCtxt::InitUserBufferLists(void)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 idx;

    for (idx = AIS_BUFLIST_USR_FIRST; idx <= AIS_BUFLIST_USR_LAST; idx++)
    {
        m_bufferList[idx] = AisBufferList::Create((AisBuflistIdType)idx, m_numBufMax);
        if (!m_bufferList[idx])
        {
            rc = CAMERA_ENOMEMORY;
            break;
        }
    }

    return rc;
}

/**
 * InitBufferLists
 *
 * @brief Initializes buffer lists from buffer list definition
 *
 * @param pBuflistDef
 *
 * @return CameraResult
 */
CameraResult AisUsrCtxt::InitBufferLists(const AisBuflistDefType* pBuflistDef)
{
    CameraResult rc = CAMERA_EFAILED;

    if (m_bufferList[pBuflistDef->id] && pBuflistDef->id > AIS_BUFLIST_USR_LAST)
    {
        AIS_LOG(ENGINE, ERROR, "internal buffer list %d already allocated!", pBuflistDef->id);
    }
    else
    {
        if (pBuflistDef->id <= AIS_BUFLIST_USR_LAST)
        {
            if (!m_bufferList[pBuflistDef->id])
            {
                AIS_LOG(ENGINE, ERROR, "user buffer list %d not allocated!", pBuflistDef->id);
                return rc;
            }

            AisBufferList* pBufferList = m_bufferList[pBuflistDef->id];
            pBufferList->SetMaxBuffers(STD_MIN(pBuflistDef->allocParams.maxBuffers, m_numBufMax));
            pBufferList->Init(pBuflistDef->GetFreeBuf, pBuflistDef->ReturnBuf, pBuflistDef->AllocBuf);

            rc = CAMERA_SUCCESS;
        }
        else
        {
            m_bufferList[pBuflistDef->id] = AisBufferList::Create(pBuflistDef->id, STD_MIN(pBuflistDef->allocParams.maxBuffers, m_numBufMax));
            if (m_bufferList[pBuflistDef->id])
            {
                AisBufferList* pBufferList = m_bufferList[pBuflistDef->id];
                pBufferList->Init(pBuflistDef->GetFreeBuf, pBuflistDef->ReturnBuf, pBuflistDef->AllocBuf);

                rc = CAMERA_SUCCESS;
            }
        }
    }

    return rc;
}

/**
 * FreeUserBufferLists
 *
 * @brief Free user buffer lists. Unmaps buffers that are mapped
 *
 * @return CameraResult
 */
CameraResult AisUsrCtxt::FreeUserBufferLists(void)
{
    uint32 i;

    for (i = AIS_BUFLIST_USR_FIRST; i <= AIS_BUFLIST_USR_LAST; i++)
    {
        if (m_bufferList[i])
        {
            (void)AisBufferManager::UnmapBuffers(m_bufferList[i]);

            m_bufferList[i]->Destroy();
            m_bufferList[i] = NULL;
        }
    }

    return CAMERA_SUCCESS;
}

/**
 * FreeBufferLists
 *
 * @brief Free internal buffer lists. Frees and Unmaps buffers.
 *
 * @return CameraResult
 */
CameraResult AisUsrCtxt::FreeInternalBufferLists(void)
{
    uint32 i;

    for (i = (AIS_BUFLIST_USR_LAST+1); i < AIS_BUFLIST_MAX; i++)
    {
        if (m_bufferList[i])
        {
            uint32 j;

            for (j = 0; j < m_bufferList[i]->m_nBuffers; j++)
            {
                CameraBufferFree(&m_bufferList[i]->m_pBuffers[j]);
            }

            m_bufferList[i]->Destroy();
            m_bufferList[i] = NULL;
        }
    }

    return CAMERA_SUCCESS;
}

CameraResult AisUsrCtxt::InitOperationMode(void)
{
    CameraResult rc;

    //Fill initial opmode and streams
    rc = AisInputConfigurer::GetInstance()->GetParam(this, AIS_INPUT_CTRL_INPUT_INTERF, NULL);
    if (CAMERA_SUCCESS == rc)
    {
        //Get initial opmode proc chain
        rc = AisProcChainManager::GetInstance()->GetProcChain(this);
        if (CAMERA_SUCCESS == rc)
        {
            //allocate user bufferlists
            rc = InitUserBufferLists();
        }
    }

    return rc;
}

CameraResult AisUsrCtxt::StopRecoveryTimeoutTimer(void)
{
    CameraResult rc;

    if (m_RecoveryTimeoutTimer)
    {
        CameraStopTimer(m_RecoveryTimeoutTimer);
        rc = CAMERA_SUCCESS;
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Recovery timeout timer not created");
        rc = CAMERA_EBADHANDLE;
    }

    return rc;
}

/**
 * FillUsrCtxtDiagInfo
 *
 * @brief Updates the usrctxt info in diagnostic structures
 * @param hndl userctxt hndl
 * @pDiagClientInfo DiagnosticInfo handle
 *
 * @return CAMERA_SUCCESS if success
 */

CameraResult AisEngine::FillUsrCtxtDiagInfo(QCarCamDiagClientInfo* pDiagClientInfo)
{
    CameraResult rc = CAMERA_SUCCESS;

    uint32 diagIdx = 0;

    for (uint32 idx = 0; idx < AIS_MAX_USR_CONTEXTS; idx++)
    {
        if (m_usrCtxtMap[idx].in_use)
        {
            AisUsrCtxt* pUsrCtxt = (AisUsrCtxt*)m_usrCtxtMap[idx].usr_ctxt;
            if (pUsrCtxt)
            {
                pDiagClientInfo[diagIdx].usrHdl = pUsrCtxt->m_qcarcamHndl;
                pDiagClientInfo[diagIdx].opMode = pUsrCtxt->m_opMode;
                pDiagClientInfo[diagIdx].inputId = pUsrCtxt->m_inputId;
                pDiagClientInfo[diagIdx].state = pUsrCtxt->m_state;
                pDiagClientInfo[diagIdx].frameRate = pUsrCtxt->m_frameRate;
                pDiagClientInfo[diagIdx].timeStampStart = pUsrCtxt->m_startTime;
                pDiagClientInfo[diagIdx].sofCounter = pUsrCtxt->m_numSof;
                pDiagClientInfo[diagIdx].frameCounter = pUsrCtxt->m_numFrameDone;
                pDiagClientInfo[diagIdx].inputDevId = pUsrCtxt->m_streams[0].inputCfg.devId;
                pDiagClientInfo[diagIdx].csiphyDevId = pUsrCtxt->m_streams[0].resources.csiphy;
                pDiagClientInfo[diagIdx].ifeDevId = pUsrCtxt->m_streams[0].resources.csid;
                pDiagClientInfo[diagIdx].rdiId = (uint32)pUsrCtxt->m_streams[0].resources.ifeStream.ifeOutput;

                /* If there are multiple streams, use a bitmask for IFE id and RDI id instead */
                if (QCARCAM_OPMODE_RDI_CONVERSION == pUsrCtxt->m_opMode)
                {
                    pDiagClientInfo[diagIdx].ifeDevId = 1 << pUsrCtxt->m_streams[0].resources.csid | 1 << pUsrCtxt->m_streams[1].resources.csid;
                    pDiagClientInfo[diagIdx].rdiId = 1 << pUsrCtxt->m_streams[0].resources.ifeStream.ifeOutput | 1 << pUsrCtxt->m_streams[1].resources.ifeStream.ifeOutput;
                }

                AisBufferList* pBufferList;
                pBufferList = pUsrCtxt->m_bufferList[AIS_BUFLIST_USR_FIRST];
                if (pBufferList != NULL)
                {
                    for (uint32 i = 0; i < pBufferList->m_nBuffers; i++)
                    {
                        pDiagClientInfo[diagIdx].bufInfo[i].bufId = pBufferList->m_pBuffers[i].idx;
                        pDiagClientInfo[diagIdx].bufInfo[i].bufStatus = (uint32)pBufferList->m_pBuffers[i].state;
                    }
                }
                else
                {
                    AIS_LOG(ENGINE, ERROR, "pBufferList is empty");
                    rc = CAMERA_EFAILED;
                }
                diagIdx++;
            }
            else
            {
                AIS_LOG(ENGINE, ERROR, "Invalid usr hdl for index %d", idx);
                rc = CAMERA_EFAILED;
            }
        }
    }
    return rc;
}

/**
 * Initialize
 *
 * @brief Initialize new user context
 *
 * @return CameraResult
 */
CameraResult AisUsrCtxt::Initialize(void)
{
    CameraResult result;
    AisEngine* pEngine = AisEngine::GetInstance();

    m_numBufMax = STD_MIN(pEngine->m_engineSettings->numBufMax, QCARCAM_MAX_NUM_BUFFERS);

    result = InitMutex();
    if (result == CAMERA_SUCCESS)
    {
        result = InitOperationMode();
        if (result != CAMERA_SUCCESS)
        {
            DeinitMutex();
        }
    }

    return result;
}

/**
 * CreateUsrCtxt
 *
 * @brief Create new user context
 *
 * @param input_id - qcarcam input id
 *
 * @return AisUsrCtxt* - NULL if fail
 */
AisUsrCtxt* AisUsrCtxt::CreateUsrCtxt(qcarcam_input_desc_t input_id)
{
    AisUsrCtxt* pUsrCtxt = new AisUsrCtxt(input_id);

    if (pUsrCtxt)
    {
        CameraResult result = CAMERA_SUCCESS;

        result = pUsrCtxt->Initialize();
        if (result != CAMERA_SUCCESS)
        {
            AIS_LOG(ENGINE, ERROR, "Failed to create user context %d", result);
            delete pUsrCtxt;
            pUsrCtxt = NULL;
        }
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Failed to allocate handle");
    }

    return pUsrCtxt;
}

/**
 * DestroyUsrCtxt
 *
 * @brief Destroy user context
 *
 * @param usr_ctxt
 *
 * @return n/a
 */
void AisUsrCtxt::DestroyUsrCtxt(AisUsrCtxt* pUsrCtxt)
{
    AIS_LOG(ENGINE, HIGH, "%p", pUsrCtxt);

    if (pUsrCtxt)
    {
        if (pUsrCtxt->m_RecoveryTimeoutTimer)
        {
            CameraReleaseTimer(pUsrCtxt->m_RecoveryTimeoutTimer);
        }

        if (pUsrCtxt->m_RetryRecoveryTimer)
        {
            CameraReleaseTimer(pUsrCtxt->m_RetryRecoveryTimer);
        }

        pUsrCtxt->FreeInternalBufferLists();

        pUsrCtxt->FreeUserBufferLists();

        pUsrCtxt->DeinitMutex();

        delete pUsrCtxt;
    }
}

/**
 * Lock
 *
 * @brief Locks user context mutex
 */
void AisUsrCtxt::Lock(void)
{
    CameraLockMutex(m_mutex);
};

/**
 * Unlock
 *
 * @brief Unlocks user context mutex
 */
void AisUsrCtxt::Unlock(void)
{
    CameraUnlockMutex(m_mutex);
};

/**
 * IncRefCnt
 *
 * @brief Increments ref count of the user context
 *
 * @return CameraResult
 */
CameraResult AisUsrCtxt::IncRefCnt(void)
{
    return AisEngine::GetInstance()->AcquireUsrCtxt(m_mapIdx) ? CAMERA_SUCCESS : CAMERA_EFAILED;
}

/**
 * IncRefCnt
 *
 * @brief Decrements ref count of the user context
 *
 * @return CameraResult
 */
void AisUsrCtxt::DecRefCnt(void)
{
    AisEngine::GetInstance()->RelinquishUsrCtxt(m_mapIdx);
}

void AisUsrCtxt::RegisterEvents(uint64 nEvtMask)
{
    m_notificationEvtMask |= nEvtMask;
}

void AisUsrCtxt::UnRegisterEvents(uint64 nEvtMask)
{
    m_notificationEvtMask &= ~nEvtMask;
}

/**
 * UpdateFrameRate
 *
 * @brief Updates the usrctxt framerate
 *
 * @return CameraResult
 */
void AisUsrCtxt::UpdateFrameRate()
{
    if (m_state == AIS_USR_STATE_STREAMING)
    {
        uint64 cur_time = 0;
        CameraGetTime(&cur_time);

        m_frameRate = m_numFrameDone/((cur_time-m_prevTime)/1000);

        m_numFrameDone = 0;
        m_prevTime = cur_time;
    }
}

/**
 * InjectionStart
 *
 * @brief Submits new postprocessing job for input buffer
 *
 * @param pUsrCtxt
 * @param idx  -  input buffer index ready to inject
 *
 * @return CameraResult
 */
CameraResult AisEngine::InjectionStart(AisUsrCtxt* pUsrCtxt, uint32 idx)
{
    CameraResult rc;

    AIS_LOG(ENGINE, LOW, "InjectionStart %d", idx);

    if (AIS_USR_STATE_STREAMING != pUsrCtxt->m_state ||
            QCARCAM_OPMODE_INJECT != pUsrCtxt->m_opMode)
    {
        AIS_LOG(ENGINE, ERROR, "%p Injection start in wrong state %d or opmode %d",
                pUsrCtxt->m_qcarcamHndl, pUsrCtxt->m_state, pUsrCtxt->m_opMode);
        return CAMERA_EBADSTATE;
    }

    AisBufferList* pBufferList = pUsrCtxt->m_bufferList[AIS_BUFLIST_INPUT_USR];
    AisBuffer* pBuffer = pBufferList->GetBuffer(idx);
    if (NULL == pBuffer)
    {
        AIS_LOG(ENGINE, ERROR, "Invalid buffer index %d for injection", idx);
        return CAMERA_EBADPARM;
    }

    rc = pBufferList->ReturnBuffer(pUsrCtxt, idx);
    if (CAMERA_SUCCESS == rc)
    {
        //set buffer as ready and to consume and queue pproc event
        AisEventMsgType sEvent = {};
        uint64 jobId = AisCreateJobId(pUsrCtxt, 0, idx, 0xFF, 0xFF);
        pBufferList->QueueReadyBuffer(jobId, pBuffer);

        sEvent.eventId = AIS_EVENT_PPROC_JOB;
        sEvent.payload.pprocJob.pUsrCtxt = pUsrCtxt;
        sEvent.payload.pprocJob.jobId = jobId;
        sEvent.payload.pprocJob.currentHop = 0;
        sEvent.payload.pprocJob.frameInfo.seq_no[0] = idx;
        CameraGetTime((uint64*)&sEvent.payload.pprocJob.frameInfo.timestamp);

        //acquire user context for pprocJob. Completion of pprocJob will release context
        pUsrCtxt->IncRefCnt();
        QueueEvent(&sEvent);
    }

    return rc;
}

/**
 * AisUsrCtxt::Reserve
 *
 * @brief Reserve HW pipeline resources for user context
 *
 * @param usr_ctxt
 *
 * @return CameraResult
 */
CameraResult AisUsrCtxt::Reserve(void)
{
    CameraResult rc = CAMERA_SUCCESS;

    AIS_LOG(ENGINE, HIGH, "m_state %d", m_state);

    if (AIS_USR_STATE_OPENED == m_state)
    {
        //Get proc chain and initialize buffer lists
        rc = AisProcChainManager::GetInstance()->GetProcChain(this);
        if (CAMERA_SUCCESS == rc)
        {
            uint32 i;
            for (i = 0; i < m_pProcChainDef->nBuflist; i++)
            {
                rc = InitBufferLists(&m_pProcChainDef->pBufferlistDef[i]);
            }
        }

        if (CAMERA_SUCCESS == rc)
        {
            rc = AisResourceManager::GetInstance()->Reserve(this);
        }

        if (CAMERA_SUCCESS == rc)
        {
            m_state = AIS_USR_STATE_RESERVED;
        }
        else
        {
            FreeInternalBufferLists();
            AisProcChainManager::GetInstance()->ReleaseProcChain(this);
        }
    }
    return rc;
}

/**
 * AisUsrCtxt::Release
 *
 * @brief Release HW pipeline resources for user context
 *
 * @param usr_ctxt
 *
 * @return CameraResult
 */
CameraResult AisUsrCtxt::Release(void)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (AIS_USR_STATE_RESERVED == m_state)
    {
        m_state = AIS_USR_STATE_OPENED;

        //release then free buffer lists
        rc = AisResourceManager::GetInstance()->Release(this);

        FreeInternalBufferLists();

        AisProcChainManager::GetInstance()->ReleaseProcChain(this);
    }

    return rc;
}

/**
 * PowerResumeForStart
 *
 *  @brief PowerResume in ais_start
 *
 *  @param pUsrCtxt       Handle of input
 *
 *  @return CameraResult
 */
CameraResult AisEngine:: PowerResumeForStart(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (FALSE == m_isMultiSoc)
    {
        if (m_isPowerGranular && m_PowerManagerPolicyType == CAMERA_PM_POLICY_NO_LPM)
        {
            for (int i = 0; i < AIS_CONFIGURER_MAX; i++)
            {
                CameraLogEvent(CAMERA_ENGINE_EVENT_POWER_RESUME_START, i, 0);
                rc = m_Configurers[i]->PowerResume(pUsrCtxt, m_isPowerGranular, CAMERA_POWER_RESUME_DEFAULT);
                CameraLogEvent(CAMERA_ENGINE_EVENT_POWER_RESUME_DONE, i, 0);
                if (CAMERA_SUCCESS != rc)
                {
                    break;
                }
            }
        }
    }

    return rc;
}

/**
 *  PowerSuspendForStop
 *
 *  @brief PowerSuspend in ais_stop
 *
 *  @param pUsrCtxt
 *
 *  @return CameraResult
 */
CameraResult AisEngine::PowerSuspendForStop(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (FALSE == m_isMultiSoc)
    {
        if (m_isPowerGranular && m_PowerManagerPolicyType == CAMERA_PM_POLICY_NO_LPM)
        {
            for (int i = 0; i < AIS_CONFIGURER_MAX; i++)
            {
                CameraLogEvent(CAMERA_ENGINE_EVENT_POWER_SUSPEND_START, i, 0);
                rc = m_Configurers[i]->PowerSuspend(pUsrCtxt, m_isPowerGranular, CAMERA_POWER_SUSPEND_DEFAULT);
                CameraLogEvent(CAMERA_ENGINE_EVENT_POWER_SUSPEND_DONE, i, 0);
                if (rc != CAMERA_SUCCESS)
                {
                    break;
                }
            }
        }
    }

    return rc;
}

/**
 * AisUsrCtxt::Start
 *
 * @brief Start HW pipeline resources for user context
 *
 * @param usr_ctxt
 *
 * @return CameraResult
 */
CameraResult AisUsrCtxt::Start(void)
{
    CameraResult rc = CAMERA_SUCCESS;

    AIS_LOG(ENGINE, HIGH, "m_state=%d", m_state);

    if (AIS_USR_STATE_RESERVED == m_state || (AIS_USR_STATE_RECOVERY_START == m_state && m_usrSettings.recovery))
    {
        AisConfigurerType configSeq[AIS_CONFIGURER_MAX] =
                        {AIS_CONFIGURER_INPUT, AIS_CONFIGURER_CSI, AIS_CONFIGURER_IFE};

        AisEngine* pEngine = AisEngine::GetInstance();

        // workround for map to multi-ife, skip 1 frame when start new IFE
        if (TRUE == AisCSIConfigurer::GetInstance()->IsCSIStarted(this)
            && FALSE == AisIFEConfigurer::GetInstance()->IsIFEStarted(this))
        {
            m_usrSettings.init_frame_drop = pEngine->m_engineSettings->multiIfeInitFrameDrop;
            AIS_LOG(ENGINE, HIGH, "multi ife init frame drop %d",m_usrSettings.init_frame_drop);
        }

        for (int i = 0; i < AIS_CONFIGURER_MAX; i++)
        {
            CameraLogEvent(CAMERA_ENGINE_EVENT_CONFIGURERS_CONFIG, configSeq[i], 0);
            rc = pEngine->m_Configurers[configSeq[i]]->Config(this);
            CameraLogEvent(CAMERA_ENGINE_EVENT_CONFIGURERS_CONFIG_DONE, configSeq[i], rc);
            if (CAMERA_SUCCESS != rc)
            {
                break;
            }

            /*Allocate internal buffers if needed*/
            if (AIS_CONFIGURER_INPUT == i && AIS_USR_STATE_RECOVERY_START != m_state)
            {
                for (int j = 0; j < (int)m_pProcChainDef->nBuflist; j++)
                {
                    if (m_pProcChainDef->pBufferlistDef[j].AllocBuf)
                    {
                        rc = m_pProcChainDef->pBufferlistDef[j].AllocBuf(this,
                            &m_pProcChainDef->pBufferlistDef[j]);
                    }

                    if (CAMERA_SUCCESS != rc)
                    {
                        AIS_LOG(ENGINE, ERROR, "Failed to allocate bufferlist %d", m_pProcChainDef->pBufferlistDef[j].id);
                        break;
                    }
                }
            }

            if (CAMERA_SUCCESS != rc)
            {
                break;
            }
        }

        /*Create PProc Sessions*/
        if (CAMERA_SUCCESS == rc && m_pProcChainDef)
        {
            for (int i = 0; i < (int)m_pProcChainDef->nProc; i++)
            {
                rc = m_pPProc[i]->CreateSession(this, &m_pProcChainDef->pProcChain[i]);
                if (CAMERA_SUCCESS != rc)
                {
                    AIS_LOG(ENGINE, ERROR, "Failed to create session for Node %d", m_pProcChainDef->pProcChain[i].id);
                    break;
                }

                rc = m_pPProc[i]->SetParams(this);
                if (rc != CAMERA_SUCCESS)
                {
                    AIS_LOG(ENGINE, ERROR, "Failed SetParam on node %d", i);
                }
            }
        }

        if (CAMERA_SUCCESS == rc)
        {
            CameraLogEvent(CAMERA_ENGINE_EVENT_QUEUE_BUFFERS, 0, 0);
            rc = ((AisIFEConfigurer*)pEngine->m_Configurers[AIS_CONFIGURER_IFE])->QueueBuffers(this);

            /*start all resources*/
            if (CAMERA_SUCCESS == rc)
            {
                AisConfigurerType startSeq[AIS_CONFIGURER_MAX] =
                    {AIS_CONFIGURER_CSI, AIS_CONFIGURER_IFE, AIS_CONFIGURER_INPUT};

                m_isPendingStart = TRUE;

                for (int i = 0; i < AIS_CONFIGURER_MAX; i++)
                {
                    /* start time is right before we start the IFE as there are cases when CSI TX is already
                       started and SOF/FD may be received right away */
                    if (AIS_CONFIGURER_IFE == i)
                    {
                        CameraGetTime(&m_startTime);
                    }

                    CameraLogEvent(CAMERA_ENGINE_EVENT_CONFIGURERS_START, startSeq[i], 0);
                    rc = pEngine->m_Configurers[startSeq[i]]->Start(this);
                    CameraLogEvent(CAMERA_ENGINE_EVENT_CONFIGURERS_START_DONE, startSeq[i], rc);
                    if (CAMERA_SUCCESS != rc)
                    {
                        //undo previous steps configurer starts
                        int cleanupIdx = i;

                        while (cleanupIdx)
                        {
                            cleanupIdx--;
                            (void)pEngine->m_Configurers[startSeq[cleanupIdx]]->Stop(this);
                        }
                        m_isPendingStart = FALSE;

                        //start input failed, then destroy session
                        if (startSeq[i] == AIS_CONFIGURER_INPUT)
                        {
                            AIS_LOG(ENGINE, ERROR, "start input fail, destroy session");
                            //Destroy PProc sessions
                            for (uint32 i = 0; i < m_pProcChainDef->nProc; i++)
                            {
                                m_pPProc[i]->DestroySession(this, &m_pProcChainDef->pProcChain[i]);
                            }
                        }

                        break;
                    }
                }

                CameraGetTime(&m_prevTime);
            }
        }

        CameraLogEvent(CAMERA_ENGINE_EVENT_USRCTXT_START, 0, rc);

        /* move to streaming state if not in recovery */
        if (CAMERA_SUCCESS == rc && AIS_USR_STATE_RECOVERY_START != m_state)
        {
            m_state = AIS_USR_STATE_STREAMING;
            m_numRecoveryAttempts = 0;
        }
    }

    return rc;
}

/**
 * AisUsrCtxt::Stop
 *
 * @brief Stop HW pipeline resources for user context
 *
 * @return CameraResult
 */
CameraResult AisUsrCtxt::Stop(void)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (AIS_USR_STATE_STOP_PENDING == m_state ||
        (m_state == AIS_USR_STATE_RECOVERY_START && m_usrSettings.recovery))
    {
        CameraResult tmpRet;
        AisEngine* pEngine = AisEngine::GetInstance();
        AisConfigurerType stopSeq[AIS_CONFIGURER_MAX] =
                {AIS_CONFIGURER_IFE, AIS_CONFIGURER_CSI, AIS_CONFIGURER_INPUT};

        for (uint32 i = 0; i < AIS_CONFIGURER_MAX; i++)
        {
            tmpRet = pEngine->m_Configurers[stopSeq[i]]->Stop(this);
            if (rc == CAMERA_SUCCESS) { rc = tmpRet;}
        }

        /* Reset all the buffer states */
        for (uint32 buflistIdx = AIS_BUFLIST_USR_FIRST; buflistIdx < AIS_BUFLIST_MAX; buflistIdx++)
        {
            AisBufferList* pBufferList = m_bufferList[buflistIdx];
            if (NULL == pBufferList)
            {
                continue;
            }

            pBufferList->Reset();
        }

        //Destroy PProc sessions
        for (uint32 i = 0; i < m_pProcChainDef->nProc; i++)
        {
            m_pPProc[i]->DestroySession(this, &m_pProcChainDef->pProcChain[i]);
        }

        if (AIS_USR_STATE_STOP_PENDING == m_state)
        {
            m_state = AIS_USR_STATE_RESERVED;
        }
    }

    return rc;
}

/**
 * AisUsrCtxt::Pause
 *
 * @brief Pause HW pipeline resources for user context.
 *        For now we only stop IFE.
 *
 * @return CameraResult
 */
CameraResult AisUsrCtxt::Pause(void)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (AIS_USR_STATE_STREAMING == m_state ||
        AIS_USR_STATE_PAUSE_PENDING == m_state)
    {
        AisEngine* pEngine = AisEngine::GetInstance();

        rc = pEngine->m_Configurers[AIS_CONFIGURER_IFE]->Pause(this);
        if (CAMERA_SUCCESS == rc)
        {
            CameraResult tmpRet;
            uint32 i;

            m_state = AIS_USR_STATE_PAUSED;


            /* Reset all the buffer states */
            for (uint32 buflistIdx = AIS_BUFLIST_USR_FIRST; buflistIdx < AIS_BUFLIST_MAX; buflistIdx++)
            {
                AisBufferList* pBufferList = m_bufferList[buflistIdx];
                if (NULL == pBufferList)
                {
                    continue;
                }

                pBufferList->Reset();
            }
        }
        else
        {
            AIS_LOG(ENGINE, ERROR, "Pause failed for %p (%d) and transition to ERROR state", m_qcarcamHndl, rc);
            m_state = AIS_USR_STATE_ERROR;
        }
    }

    return rc;
}

/**
 * AisUsrCtxt::Resume
 *
 * @brief Resume HW pipeline resources for user context.
 *        For now we only restart IFE.
 *
 * @return CameraResult
 */
CameraResult AisUsrCtxt::Resume(void)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (AIS_USR_STATE_PAUSED == m_state)
    {
        AisEngine* pEngine = AisEngine::GetInstance();

        rc = ((AisIFEConfigurer*)pEngine->m_Configurers[AIS_CONFIGURER_IFE])->QueueBuffers(this);
        if (CAMERA_SUCCESS == rc)
        {
            rc = pEngine->m_Configurers[AIS_CONFIGURER_IFE]->Resume(this);
        }

        if (CAMERA_SUCCESS == rc)
        {
            m_state = AIS_USR_STATE_STREAMING;
            m_numRecoveryAttempts = 0;
        }
    }

    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_query_inputs
///
/// @brief Queries available inputs. To get the number of available inputs to query, call with p_inputs set to NULL.
///
/// @param p_inputs   Pointer to array inputs. If NULL, then ret_size returns number of available inputs to query
/// @param size       Number of elements in array
/// @param ret_size   If p_inputs is set, number of elements in array that were filled
///                   If p_inputs is NULL, number of available inputs to query
///
/// @return CAMERA_SUCCESS if successful
///         CAMERA_EITEMBUSY if engine is in detection.
///////////////////////////////////////////////////////////////////////////////
CameraResult AisEngine::ais_query_inputs(qcarcam_input_t* p_inputs, unsigned int size, unsigned int* ret_size)
{
    CameraResult rc = CAMERA_SUCCESS;

    AIS_API_ENTER();

    if (!ret_size)
    {
        AIS_LOG(ENGINE, ERROR, "Bad ret_size ptr");
        rc = CAMERA_EMEMPTR;
    }
    else
    {
        rc = AisInputConfigurer::GetInstance()->QueryInputs(p_inputs, size, ret_size);

        if (m_state == AIS_ENGINE_STATE_DETECTION_PENDING &&
            rc == CAMERA_SUCCESS)
        {
            rc = CAMERA_EITEMBUSY;
        }
    }

    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_query_inputs_v2
///
/// @brief Queries available inputs. To get the number of available inputs to query, call with p_inputs set to NULL.
///
/// @param p_inputs   Pointer to array inputs. If NULL, then ret_size returns number of available inputs to query
/// @param size       Number of elements in array
/// @param ret_size   If p_inputs is set, number of elements in array that were filled
///                   If p_inputs is NULL, number of available inputs to query
///
/// @return CAMERA_SUCCESS if successful
///         CAMERA_EITEMBUSY if engine is in detection.
///////////////////////////////////////////////////////////////////////////////
CameraResult AisEngine::ais_query_inputs_v2(qcarcam_input_v2_t* p_inputs, unsigned int size, unsigned int* ret_size)
{
    CameraResult rc = CAMERA_SUCCESS;

    AIS_API_ENTER();

    if (!ret_size)
    {
        AIS_LOG(ENGINE, ERROR, "Bad ret_size ptr");
        rc = CAMERA_EMEMPTR;
    }
    else
    {
        rc = AisInputConfigurer::GetInstance()->QueryInputsV2(p_inputs, size, ret_size);

        if (m_state == AIS_ENGINE_STATE_DETECTION_PENDING &&
            rc == CAMERA_SUCCESS)
        {
            rc = CAMERA_EITEMBUSY;
        }
    }

    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_query_diagnsotics
///
/// @brief Queries the system diagnostic Info.
///
/// @param p_diag_info   Pointer to diagnostic info.
/// @param diag_size     Size of user allocated memory
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult AisEngine::ais_query_diagnostics(void *p_ais_diag_info, unsigned int diag_size)
{
    CameraResult rc = CAMERA_SUCCESS;
    AIS_API_ENTER();
    rc = m_DiagManager->QueryDiagnostics(p_ais_diag_info, diag_size);
    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_open
///
/// @brief Opens handle to input
///
/// @param desc   Unique identifier of input to be opened
///
/// @return NOT NULL if successful; NULL on failure
///////////////////////////////////////////////////////////////////////////////
/* Check input is available (physically connected as well as SW resource)
 * Create SW context */
qcarcam_hndl_t AisEngine::ais_open(qcarcam_input_desc_t desc)
{
    qcarcam_hndl_t qcarcam_hndl = NULL;
    CameraResult rc = CAMERA_SUCCESS;
    uint32 mode = 0;

    AIS_API_ENTER();

    if (IsInputAvailable(desc))
    {
        AIS_LOG(ENGINE, LOW, "input id %d", desc);
        AisUsrCtxt* pUsrCtxt = AisUsrCtxt::CreateUsrCtxt(desc);
        if (pUsrCtxt)
        {
            qcarcam_hndl = (qcarcam_hndl_t)AssignNewHndl(pUsrCtxt);
            if (qcarcam_hndl)
            {
                AIS_LOG(ENGINE, HIGH, "%p [%p, %d]", pUsrCtxt, qcarcam_hndl, desc);
                pUsrCtxt->m_qcarcamHndl = qcarcam_hndl;
                pUsrCtxt->m_state = AIS_USR_STATE_OPENED;

                rc = AisInputConfigurer::GetInstance()->GetParam(pUsrCtxt,
                            AIS_INPUT_CTRL_MODE, &mode);
                if (rc)
                {
                    AIS_LOG(ENGINE, WARN, "Could not read sensormdoe with index %d from input device %d (rc = %d).", mode, pUsrCtxt->m_inputId, rc);
                }
                else
                {
                    pUsrCtxt->m_usrSettings.sensormode = mode;
                }

                AisEvtSetMaster(pUsrCtxt,FALSE);
                AisEvtListnerAddUsrhdl(pUsrCtxt);
            }
            else
            {
                AIS_LOG(ENGINE, ERROR, "Failed to get new hndl");
                AisUsrCtxt::DestroyUsrCtxt(pUsrCtxt);
            }
        }
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Invalid input id %d", desc);
    }


    return qcarcam_hndl;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_close
///
/// @brief Closes handle to input
///
/// @param hndl   Handle of input that was opened
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult AisEngine::ais_close(qcarcam_hndl_t hndl)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisUsrCtxt* pUsrCtxt;
    AisUsrCtxtList::iterator it;

    AIS_API_ENTER_HNDL(hndl);

    pUsrCtxt = GetUsrCtxt(hndl);
    if (pUsrCtxt)
    {
        pUsrCtxt->Lock();

        if(pUsrCtxt->m_state == AIS_USR_STATE_STREAMING ||
           pUsrCtxt->m_state == AIS_USR_STATE_PAUSED ||
           pUsrCtxt->m_state == AIS_USR_STATE_ERROR)
        {
            AIS_LOG(ENGINE, HIGH, "%p in state %d.  Need to stop first.",
                hndl, pUsrCtxt->m_state);

            pUsrCtxt->Unlock();
            /*stop the context to release resources*/
            rc = ais_stop(hndl);
            AIS_LOG_ON_ERR(ENGINE, rc, "Failed to stop handle %p rc %d", hndl, rc);
            pUsrCtxt->Lock();
        }

        pUsrCtxt->m_state = AIS_USR_STATE_UNINITIALIZED;

        pUsrCtxt->Unlock();

        AisEvtListnerRemoveUsrhdl(pUsrCtxt);

        if(pUsrCtxt->m_isMaster)
        {
            AisEvtReleaseMaster(pUsrCtxt);

            if (m_eventListener.find(pUsrCtxt->m_inputId) == m_eventListener.end() ||
                (*m_eventListener[pUsrCtxt->m_inputId]).empty())
            {
                AIS_LOG(ENGINE, DBG, "No other UsrCtxts for inputId %d", pUsrCtxt->m_inputId);
            }
            else
            {
                it = (*m_eventListener[pUsrCtxt->m_inputId]).begin();
                AisEvtSetMaster(*it,FALSE);
            }
        }

        /*release handle*/
        ReleaseHndl(hndl);

    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Invalid hndl %p", hndl);
        rc = CAMERA_EBADHANDLE;
    }

    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_g_param
///
/// @brief Get parameter value
///
/// @param hndl     Handle of input
/// @param param    Parameter to get
/// @param p_value  Pointer to structure of value that will be retrieved
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult AisEngine::ais_g_param(qcarcam_hndl_t hndl, qcarcam_param_t param, qcarcam_param_value_t* p_value)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisUsrCtxt* pUsrCtxt;

    AIS_API_ENTER_HNDL(hndl);

    pUsrCtxt = GetUsrCtxt(hndl);
    if (pUsrCtxt)
    {
        if (!p_value)
        {
            AIS_LOG(ENGINE, ERROR, "Null pointer for param %d", param);
            rc = CAMERA_EMEMPTR;
        }
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Invalid hndl %p", hndl);
        rc = CAMERA_EBADHANDLE;
    }

    if (CAMERA_SUCCESS == rc)
    {
        pUsrCtxt->Lock();

        if (AIS_USR_STATE_UNINITIALIZED == pUsrCtxt->m_state)
        {
            AIS_LOG(ENGINE, ERROR, "usrctxt %p in incorrect state %d",
                    pUsrCtxt, pUsrCtxt->m_state);
            rc = CAMERA_EBADSTATE;
        }
        else
        {
            switch (param)
            {
            case QCARCAM_PARAM_EVENT_CB:
                p_value->ptr_value = (void*)pUsrCtxt->m_eventCbFcn;
                break;
            case QCARCAM_PARAM_EVENT_MASK:
                p_value->uint_value = pUsrCtxt->m_eventMask;
                break;
            case QCARCAM_PARAM_EXPOSURE:
                rc = AisInputConfigurer::GetInstance()->GetParam(pUsrCtxt,
                            AIS_INPUT_CTRL_EXPOSURE_CONFIG, &p_value->exposure_config);
                if (CAMERA_EUNSUPPORTED == rc)
                {
                    AIS_LOG(ENGINE, WARN, "Could not read Exposure param from input device (rc = %d) - checking user context.", rc);
                    if (USR_CTXT_CHECK_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_EXPOSURE))
                    {
                        p_value->exposure_config = pUsrCtxt->m_usrSettings.exposure_params;
                        rc = CAMERA_SUCCESS;
                    }
                    else
                    {
                        AIS_LOG(ENGINE, ERROR, "Exposure param not set");
                        rc = CAMERA_EBADSTATE;
                    }
                }
                break;
            case QCARCAM_PARAM_HDR_EXPOSURE:
                rc = AisInputConfigurer::GetInstance()->GetParam(pUsrCtxt,
                            AIS_INPUT_CTRL_HDR_EXPOSURE_CONFIG, &p_value->hdr_exposure_config);
                if (CAMERA_EUNSUPPORTED == rc)
                {
                    AIS_LOG(ENGINE, WARN, "Could not read HDR Exposure param from input device (rc = %d) - checking user context.", rc);
                    if (USR_CTXT_CHECK_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_HDR_EXPOSURE))
                    {
                        p_value->hdr_exposure_config = pUsrCtxt->m_usrSettings.hdr_exposure;
                        rc = CAMERA_SUCCESS;
                    }
                    else
                    {
                        AIS_LOG(ENGINE, ERROR, "HDR Exposure param not set");
                        rc = CAMERA_EBADSTATE;
                    }
                }
                break;
            case QCARCAM_PARAM_INPUT_MODE:
                rc = AisInputConfigurer::GetInstance()->GetParam(pUsrCtxt,
                            AIS_INPUT_CTRL_MODE, &p_value->uint_value);
                if (rc)
                {
                    AIS_LOG(ENGINE, WARN, "Could not read sensormdoe with index %d from input device %d (rc = %d).", p_value->uint_value, pUsrCtxt->m_inputId, rc);
                }
                break;
            case QCARCAM_PARAM_INPUT_COLOR_SPACE:
                rc = AisInputConfigurer::GetInstance()->GetParam(pUsrCtxt,
                            AIS_INPUT_CTRL_COLOR_SPACE, &p_value->color_space);
                if (rc)
                {
                    AIS_LOG(ENGINE, WARN, "Could not read color_space from input device %d (rc = %d).", pUsrCtxt->m_inputId, rc);
                }
                break;
            case QCARCAM_PARAM_FRAME_RATE:
                if (USR_CTXT_CHECK_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_FRAME_RATE))
                {
                    /* Get this parameter from the user context */
                    p_value->frame_rate_config.frame_drop_mode =
                        pUsrCtxt->m_usrSettings.frame_rate_config.frame_drop_mode;
                    p_value->frame_rate_config.frame_drop_period =
                        pUsrCtxt->m_usrSettings.frame_rate_config.frame_drop_period;
                    p_value->frame_rate_config.frame_drop_pattern =
                        pUsrCtxt->m_usrSettings.frame_rate_config.frame_drop_pattern;
                }
                else
                {
                    p_value->frame_rate_config.frame_drop_mode = QCARCAM_KEEP_ALL_FRAMES;
                }
                break;
            case QCARCAM_PARAM_PRIVATE_DATA:
                if (USR_CTXT_CHECK_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_PRIVATE_DATA))
                {
                    p_value->ptr_value = pUsrCtxt->m_usrSettings.pPrivData;
                }
                else
                {
                    AIS_LOG(ENGINE, ERROR, "PRIVATE_DATA param not set");
                    rc = CAMERA_EBADSTATE;
                }
                break;
            case QCARCAM_PARAM_HUE:
                rc = AisInputConfigurer::GetInstance()->GetParam(pUsrCtxt,
                            AIS_INPUT_CTRL_HUE_CONFIG, &p_value->float_value);
                if (CAMERA_EUNSUPPORTED == rc)
                {
                    AIS_LOG(ENGINE, WARN, "Could not read Hue param from input device (rc = %d) - checking user context.", rc);
                    if (USR_CTXT_CHECK_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_HUE))
                    {
                        /* Get this parameter from the user context */
                        p_value->float_value =
                            pUsrCtxt->m_usrSettings.hue_param;
                        rc = CAMERA_SUCCESS;
                    }
                    else
                    {
                        AIS_LOG(ENGINE, ERROR, "Hue param not set");
                        rc = CAMERA_EBADSTATE;
                    }
                }
                break;
            case QCARCAM_PARAM_SATURATION:
                rc = AisInputConfigurer::GetInstance()->GetParam(pUsrCtxt,
                            AIS_INPUT_CTRL_SATURATION_CONFIG, &p_value->float_value);
                if (CAMERA_EUNSUPPORTED == rc)
                {
                    AIS_LOG(ENGINE, WARN, "Could not read Saturation param from input device (rc = %d) - checking user context." , rc);
                    if (USR_CTXT_CHECK_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_SATURATION))
                    {
                        /* Get this parameter from the user context */
                        p_value->float_value =
                            pUsrCtxt->m_usrSettings.saturation_param;
                        rc = CAMERA_SUCCESS;
                    }
                    else
                    {
                        AIS_LOG(ENGINE, ERROR, "Saturation param not set");
                        rc = CAMERA_EBADSTATE;
                    }
                }
                break;
            case QCARCAM_PARAM_OPMODE:
                p_value->uint_value = (uint32)pUsrCtxt->m_opMode;
                break;
            case QCARCAM_PARAM_GAMMA:
                rc = AisInputConfigurer::GetInstance()->GetParam(pUsrCtxt,
                            AIS_INPUT_CTRL_GAMMA_CONFIG, &p_value->gamma_config);
                if (CAMERA_EUNSUPPORTED == rc)
                {
                    AIS_LOG(ENGINE, WARN, "Could not read Gamma param from input device (rc = %d) - checking user context.", rc);
                    if (USR_CTXT_CHECK_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_GAMMA))
                    {
                        rc = CAMERA_SUCCESS;
                        /* Get this parameter from the user context */
                        if (QCARCAM_GAMMA_EXPONENT == pUsrCtxt->m_usrSettings.gamma_params.config_type &&
                            QCARCAM_GAMMA_EXPONENT == p_value->gamma_config.config_type)
                        {
                            p_value->gamma_config.gamma.f_value =
                                pUsrCtxt->m_usrSettings.gamma_params.f_value;
                        }
                        else if (QCARCAM_GAMMA_KNEEPOINTS == pUsrCtxt->m_usrSettings.gamma_params.config_type &&
                                QCARCAM_GAMMA_KNEEPOINTS == p_value->gamma_config.config_type)
                        {
                            if (p_value->gamma_config.gamma.table.length !=
                                pUsrCtxt->m_usrSettings.gamma_params.length)
                            {
                                AIS_LOG(ENGINE, ERROR, "Gamma table length does not match");
                                rc = CAMERA_EBADPARM;
                            }
                            else if (!p_value->gamma_config.gamma.table.p_value)
                            {
                                AIS_LOG(ENGINE, ERROR, "Invalid ptr");
                                rc = CAMERA_EBADPARM;
                            }
                            else
                            {
                                memcpy(p_value->gamma_config.gamma.table.p_value,
                                        pUsrCtxt->m_usrSettings.gamma_params.table,
                                        pUsrCtxt->m_usrSettings.gamma_params.length*sizeof(unsigned int));
                            }
                        }
                        else
                        {
                            AIS_LOG(ENGINE, ERROR, "Unsupported Gamma config type (%d)", p_value->gamma_config.config_type);
                            rc = CAMERA_EBADPARM;
                        }
                    }
                    else
                    {
                        AIS_LOG(ENGINE, ERROR, "Gamma param not set");
                        rc = CAMERA_EBADSTATE;
                    }
                }
                break;
            case QCARCAM_PARAM_ISP_CTRLS:
            {
                if (AIS_USR_STATE_STREAMING != pUsrCtxt->m_state)
                {
                    if (USR_CTXT_CHECK_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_ISP_CTRLS))
                    {
                        p_value->isp_ctrls = pUsrCtxt->m_usrSettings.isp_ctrls;
                    }
                    else
                    {
                        AIS_LOG(ENGINE, ERROR, "ISP ctrls not set");
                        rc = CAMERA_EBADSTATE;
                    }
                    break;
                }

                //TODO:: Call GetPram only for ISP node
                if (pUsrCtxt->m_pProcChainDef)
                {
                    for (int i = 0; i < (int)pUsrCtxt->m_pProcChainDef->nProc; i++)
                    {
                        if (pUsrCtxt->m_pPProc[i])
                        {
                            rc = pUsrCtxt->m_pPProc[i]->GetParams(pUsrCtxt, &p_value->isp_ctrls);
                            if (rc != CAMERA_SUCCESS)
                            {
                                AIS_LOG(ENGINE, ERROR, "Failed to get ISP param from node %d", i);
                            }
                        }
                    }
                }
                break;
            }
            case QCARCAM_PARAM_VENDOR:
            {
                rc = AisInputConfigurer::GetInstance()->GetParam(pUsrCtxt,
                            AIS_INPUT_CTRL_VENDOR_PARAM, &p_value->vendor_param);
                if (rc != CAMERA_SUCCESS)
                {
                    AIS_LOG(ENGINE, ERROR, "Failed to get vendor config");
                }
                break;
            }
            case QCARCAM_PARAM_BRIGHTNESS:
            {
                rc = AisInputConfigurer::GetInstance()->GetParam(pUsrCtxt,
                            AIS_INPUT_CTRL_BRIGHTNESS, &p_value->float_value);
                if (CAMERA_EUNSUPPORTED == rc)
                {
                    AIS_LOG(ENGINE, WARN, "Could not read Brightness param from input device (rc = %d) - checking user context.", rc);
                    if (USR_CTXT_CHECK_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_BRIGHTNESS))
                    {
                        /* Get this parameter from the user context */
                        p_value->float_value =
                            pUsrCtxt->m_usrSettings.brightness_param;
                        rc = CAMERA_SUCCESS;
                    }
                    else
                    {
                        AIS_LOG(ENGINE, ERROR, "Brightness param not set");
                        rc = CAMERA_EBADSTATE;
                    }
                }
                break;
            }
            case QCARCAM_PARAM_CONTRAST:
            {
                rc = AisInputConfigurer::GetInstance()->GetParam(pUsrCtxt,
                            AIS_INPUT_CTRL_CONTRAST, &p_value->float_value);
                if (CAMERA_EUNSUPPORTED == rc)
                {
                    AIS_LOG(ENGINE, WARN, "Could not read Contrast param from input device (rc = %d) - checking user context.", rc);
                    if (USR_CTXT_CHECK_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_CONTRAST))
                    {
                        /* Get this parameter from the user context */
                        p_value->float_value =
                            pUsrCtxt->m_usrSettings.contrast_param;
                        rc = CAMERA_SUCCESS;
                    }
                    else
                    {
                        AIS_LOG(ENGINE, ERROR, "Contrast param not set");
                        rc = CAMERA_EBADSTATE;
                    }
                }
                break;
            }
            case QCARCAM_PARAM_MIRROR_H:
            {
                rc = AisInputConfigurer::GetInstance()->GetParam(pUsrCtxt,
                            AIS_INPUT_CTRL_MIRROR_H, &p_value->uint_value);
                if (CAMERA_EUNSUPPORTED == rc)
                {
                    AIS_LOG(ENGINE, WARN, "Could not read Mirror (h) param from input device (rc = %d) - checking user context.", rc);
                    if (USR_CTXT_CHECK_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_MIRROR_H))
                    {
                        /* Get this parameter from the user context */
                        p_value->uint_value =
                            pUsrCtxt->m_usrSettings.mirror_h_param;
                        rc = CAMERA_SUCCESS;
                    }
                    else
                    {
                        AIS_LOG(ENGINE, ERROR, "Mirror (H) param not set");
                        rc = CAMERA_EBADSTATE;
                    }
                }
                break;
            }
            case QCARCAM_PARAM_MIRROR_V:
            {
                rc = AisInputConfigurer::GetInstance()->GetParam(pUsrCtxt,
                            AIS_INPUT_CTRL_MIRROR_V, &p_value->uint_value);
                if (CAMERA_EUNSUPPORTED == rc)
                {
                    AIS_LOG(ENGINE, WARN, "Could not read Mirror (v) param from input device (rc = %d) - checking user context." , rc);
                    if (USR_CTXT_CHECK_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_MIRROR_V))
                    {
                        /* Get this parameter from the user context */
                        p_value->uint_value =
                            pUsrCtxt->m_usrSettings.mirror_v_param;
                        rc = CAMERA_SUCCESS;
                    }
                    else
                    {
                        AIS_LOG(ENGINE, ERROR, "Mirror (V) param not set");
                        rc = CAMERA_EBADSTATE;
                    }
                }
                break;
            }
            case QCARCAM_PARAM_RECOVERY:
                p_value->uint_value = (uint32)pUsrCtxt->m_usrSettings.recovery;
                break;
            case QCARCAM_PARAM_LATENCY_MAX:
                p_value->uint_value = pUsrCtxt->m_usrSettings.n_latency_max;
                break;
            case QCARCAM_PARAM_LATENCY_REDUCE_RATE:
                p_value->uint_value = pUsrCtxt->m_usrSettings.n_latency_reduce_rate;
                break;
            case QCARCAM_PARAM_BATCH_MODE:
                p_value->batch_config = pUsrCtxt->m_usrSettings.batch_config;
                break;
            case QCARCAM_PARAM_ISP_USECASE:
                if (p_value->isp_config.id >= AIS_USER_CTXT_MAX_ISP_INSTANCES)
                {
                    AIS_LOG(ENGINE, ERROR, "invalid isp instance id %u", p_value->isp_config.id);
                    rc = CAMERA_EBADPARM;
                }
                else
                {
                    p_value->isp_config.camera_id = pUsrCtxt->m_ispInstance[p_value->isp_config.id].cameraId;
                    p_value->isp_config.use_case = pUsrCtxt->m_ispInstance[p_value->isp_config.id].useCase;
                    rc = CAMERA_SUCCESS;
                }
                break;
            default:
                AIS_LOG(ENGINE, ERROR, "Unsupported param %d", param);
                rc = CAMERA_EUNSUPPORTED;
                break;
            }
        }

        pUsrCtxt->Unlock();

        PutUsrCtxt(pUsrCtxt);
    }

    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_s_param
///
/// @brief Set parameter
///
/// @param hndl     Handle of input
/// @param param    Parameter to set
/// @param p_value  Pointer to structure of value that will be set
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult AisEngine::ais_s_param(qcarcam_hndl_t hndl, qcarcam_param_t param, const qcarcam_param_value_t* p_value)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisUsrCtxt* pUsrCtxt;

    AIS_API_ENTER_HNDL(hndl);

    pUsrCtxt = GetUsrCtxt(hndl);
    if (pUsrCtxt)
    {
        if (!p_value)
        {
            AIS_LOG(ENGINE, ERROR, "Null pointer for param %d", param);
            rc = CAMERA_EMEMPTR;
        }
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Invalid hndl %p", hndl);
        rc = CAMERA_EBADHANDLE;
    }

    if (CAMERA_SUCCESS == rc)
    {
        pUsrCtxt->Lock();

        if (AIS_USR_STATE_UNINITIALIZED == pUsrCtxt->m_state)
        {
            AIS_LOG(ENGINE, ERROR, "Usrctxt %p in incorrect state %d",
                    pUsrCtxt, pUsrCtxt->m_state);
            rc = CAMERA_EBADSTATE;
        }
        else
        {
            switch (param)
            {
            case QCARCAM_PARAM_EVENT_CB:
                pUsrCtxt->m_eventCbFcn = (qcarcam_event_cb_t)p_value->ptr_value;
                break;
            case QCARCAM_PARAM_EVENT_MASK:
                pUsrCtxt->m_eventMask = p_value->uint_value;
                break;
            case QCARCAM_PARAM_EXPOSURE:
                pUsrCtxt->m_usrSettings.exposure_params = p_value->exposure_config;

                /* Send this parameter to sensor */
                rc = AisInputConfigurer::GetInstance()->SetParam(pUsrCtxt,
                    AIS_INPUT_CTRL_EXPOSURE_CONFIG, NULL);
                if (CAMERA_SUCCESS == rc)
                {
                    USR_CTXT_SET_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_EXPOSURE);
                    AisUsrctxtNotifyEventListener(pUsrCtxt->m_inputId, QCARCAM_PARAM_EXPOSURE);
                }
                else
                {
                    AIS_LOG(ENGINE, ERROR, "ais_input_configurer set param (QCARCAM_PARAM_EXPOSURE) failed, rc = %d", rc);
                }
                break;
            case QCARCAM_PARAM_HDR_EXPOSURE:
                pUsrCtxt->m_usrSettings.hdr_exposure = p_value->hdr_exposure_config;

                /* Send this parameter to sensor */
                rc = AisInputConfigurer::GetInstance()->SetParam(pUsrCtxt,
                    AIS_INPUT_CTRL_HDR_EXPOSURE_CONFIG, NULL);
                if (CAMERA_SUCCESS == rc)
                {
                    USR_CTXT_SET_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_HDR_EXPOSURE);
                    AisUsrctxtNotifyEventListener(pUsrCtxt->m_inputId, QCARCAM_PARAM_HDR_EXPOSURE);
                }
                else
                {
                    AIS_LOG(ENGINE, ERROR, "ais_input_configurer set param (QCARCAM_PARAM_HDR_EXPOSURE) failed, rc = %d", rc);
                }
                break;
            case QCARCAM_PARAM_INPUT_MODE:
                if(pUsrCtxt->m_isMaster)
                {
                    pUsrCtxt->m_usrSettings.sensormode = p_value->uint_value;
                    /* Send this parameter to sensor */
                    rc = AisInputConfigurer::GetInstance()->SetParam(pUsrCtxt,
                        AIS_INPUT_CTRL_MODE, NULL);
                    if (CAMERA_SUCCESS == rc)
                    {
                        USR_CTXT_SET_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_INPUT_MODE);
                        AisUsrctxtNotifyEventListener(pUsrCtxt->m_inputId, QCARCAM_PARAM_INPUT_MODE);
                    }
                    else
                    {
                        AIS_LOG(ENGINE, ERROR, "ais_input_configurer set param (QCARCAM_PARAM_INPUT_MODE) failed, index = %d, device = %d, rc = %d", p_value->uint_value, pUsrCtxt->m_inputId, rc);
                    }
                }
                else
                {
                    AIS_LOG(ENGINE, ERROR, "This client is not the master");
                }
                break;
            case QCARCAM_PARAM_SATURATION:
                pUsrCtxt->m_usrSettings.saturation_param = p_value->float_value;

                /* Send this parameter to sensor */
                rc = AisInputConfigurer::GetInstance()->SetParam(pUsrCtxt,
                    AIS_INPUT_CTRL_SATURATION_CONFIG, NULL);
                if (CAMERA_SUCCESS == rc)
                {
                    USR_CTXT_SET_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_SATURATION);
                    AisUsrctxtNotifyEventListener(pUsrCtxt->m_inputId, QCARCAM_PARAM_SATURATION);
                }
                else
                {
                    AIS_LOG(ENGINE, ERROR, "ais_input_configurer set param (QCARCAM_PARAM_SATURATION) failed, rc = %d", rc);
                }
                break;
            case QCARCAM_PARAM_HUE:
                pUsrCtxt->m_usrSettings.hue_param = p_value->float_value;

                /* Send this parameter to sensor */
                rc = AisInputConfigurer::GetInstance()->SetParam(pUsrCtxt,
                    AIS_INPUT_CTRL_HUE_CONFIG, NULL);
                if (CAMERA_SUCCESS == rc)
                {
                    USR_CTXT_SET_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_HUE);
                    AisUsrctxtNotifyEventListener(pUsrCtxt->m_inputId, QCARCAM_PARAM_HUE);
                }
                else
                {
                    AIS_LOG(ENGINE, ERROR, "ais_input_configurer set param (QCARCAM_PARAM_HUE) failed, rc = %d", rc);
                }
                break;
            case QCARCAM_PARAM_RESOLUTION:
                if (AIS_USR_STATE_OPENED != pUsrCtxt->m_state &&
                    AIS_USR_STATE_RESERVED != pUsrCtxt->m_state)
                {
                    AIS_LOG(ENGINE, ERROR, "Usrctxt %p in incorrect state %d",
                            pUsrCtxt, pUsrCtxt->m_state);
                    rc = CAMERA_EBADSTATE;
                }
                else
                {
                    USR_CTXT_SET_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_RESOLUTION);

                    pUsrCtxt->m_usrSettings.width = p_value->res_value.width;
                    pUsrCtxt->m_usrSettings.height = p_value->res_value.height;
                }
                break;
            case QCARCAM_PARAM_FRAME_RATE:
                pUsrCtxt->m_usrSettings.frame_rate_config.frame_drop_mode =
                    p_value->frame_rate_config.frame_drop_mode;
                pUsrCtxt->m_usrSettings.frame_rate_config.frame_drop_period =
                    p_value->frame_rate_config.frame_drop_period;
                pUsrCtxt->m_usrSettings.frame_rate_config.frame_drop_pattern =
                    p_value->frame_rate_config.frame_drop_pattern;
//@todo: update framedrop rate if streaming
#if 0
                if (AIS_USR_STATE_STREAMING == pUsrCtxt->m_state)
                {
                    rc = ais_config_ife_framedrop_update(g_ais_ctxt.configurers[AIS_CONFIGURER_IFE].hndl, pUsrCtxt);
                }
#endif
                if (CAMERA_SUCCESS == rc)
                {
                    USR_CTXT_SET_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_FRAME_RATE);
                    AisUsrctxtNotifyEventListener(pUsrCtxt->m_inputId, QCARCAM_PARAM_FRAME_RATE);
                }
                break;
            case QCARCAM_PARAM_INJECTION_START:
                rc = InjectionStart(pUsrCtxt, p_value->uint_value);
                break;
            case QCARCAM_PARAM_PRIVATE_DATA:
                pUsrCtxt->m_usrSettings.pPrivData = p_value->ptr_value;
                USR_CTXT_SET_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_PRIVATE_DATA);
                break;
            case QCARCAM_PARAM_OPMODE:
                if (p_value->uint_value >= QCARCAM_OPMODE_MAX)
                {
                    AIS_LOG(ENGINE, ERROR, "Invalid operation mode %d", p_value->uint_value);
                    rc = CAMERA_EBADPARM;
                }
                else
                {
                    pUsrCtxt->m_opMode = (qcarcam_opmode_type)p_value->uint_value;
                }
                break;
            case QCARCAM_PARAM_GAMMA:
            {
                if (p_value->gamma_config.config_type ==
                    QCARCAM_GAMMA_EXPONENT)
                {
                    pUsrCtxt->m_usrSettings.gamma_params.f_value
                        = p_value->gamma_config.gamma.f_value;
                }
                else if (p_value->gamma_config.config_type ==
                    QCARCAM_GAMMA_KNEEPOINTS)
                {
                    unsigned int len = p_value->gamma_config.gamma.table.length;
                    unsigned int* pTable = p_value->gamma_config.gamma.table.p_value;
                    if (!len || !pTable || len > QCARCAM_MAX_GAMMA_TABLE)
                    {
                        AIS_LOG(ENGINE, ERROR, "Invalid gamma table size (%d) or ptr(%p)", len, pTable);
                        rc = CAMERA_EBADPARM;
                    }
                    else
                    {
                        memcpy(pUsrCtxt->m_usrSettings.gamma_params.table, pTable, len * sizeof(*pTable));
                        pUsrCtxt->m_usrSettings.gamma_params.length = len;
                    }
                }
                else
                {
                    AIS_LOG(ENGINE, ERROR, "Unsupported Gamma config type (%d)", p_value->gamma_config.config_type);
                    rc = CAMERA_EBADPARM;
                }

                if (CAMERA_SUCCESS == rc)
                {
                    pUsrCtxt->m_usrSettings.gamma_params.config_type =
                        p_value->gamma_config.config_type;

                    /* Send this parameter to sensor */
                    rc = AisInputConfigurer::GetInstance()->SetParam(pUsrCtxt,
                            AIS_INPUT_CTRL_GAMMA_CONFIG, NULL);
                    if (CAMERA_SUCCESS == rc)
                    {
                        USR_CTXT_SET_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_GAMMA);
                        AisUsrctxtNotifyEventListener(pUsrCtxt->m_inputId, QCARCAM_PARAM_GAMMA);
                    }
                }
                break;
            }
            case QCARCAM_PARAM_ISP_CTRLS:
            {
                pUsrCtxt->m_usrSettings.isp_ctrls = p_value->isp_ctrls;

                if (AIS_USR_STATE_STREAMING != pUsrCtxt->m_state)
                {
                    USR_CTXT_SET_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_ISP_CTRLS);
                    break;
                }

                //TODO:: Call SetParam only on ISP node
                if (pUsrCtxt->m_pProcChainDef)
                {
                    for (int i = 0; i < (int)pUsrCtxt->m_pProcChainDef->nProc; i++)
                    {
                        if (pUsrCtxt->m_pPProc[i])
                        {
                            rc = pUsrCtxt->m_pPProc[i]->SetParams(pUsrCtxt);
                            if (CAMERA_SUCCESS != rc)
                            {
                                AIS_LOG(ENGINE, ERROR, "Failed Set ISP Param on node %d", i);
                                break;
                            }
                        }
                    }
                }

                if (CAMERA_SUCCESS == rc)
                {
                    USR_CTXT_SET_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_ISP_CTRLS);
                }
                break;
            }
            case QCARCAM_PARAM_VENDOR:
            {
                rc = AisInputConfigurer::GetInstance()->SetParam(pUsrCtxt,
                        AIS_INPUT_CTRL_VENDOR_PARAM, (void*)&p_value->vendor_param);
                if (CAMERA_SUCCESS != rc)
                {
                    AIS_LOG(ENGINE, ERROR, "Failed Set vendor param");
                }
                break;
            }
            case QCARCAM_PARAM_BRIGHTNESS:
            {
                pUsrCtxt->m_usrSettings.brightness_param = p_value->float_value;

                /* Send this parameter to sensor */
                rc = AisInputConfigurer::GetInstance()->SetParam(pUsrCtxt,
                    AIS_INPUT_CTRL_BRIGHTNESS, NULL);
                if (CAMERA_SUCCESS == rc)
                {
                    USR_CTXT_SET_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_BRIGHTNESS);
                }
                else
                {
                    AIS_LOG(ENGINE, ERROR, "ais_input_configurer set param (QCARCAM_PARAM_BRIGHTNESS) failed, rc = %d");
                }
                break;
            }
            case QCARCAM_PARAM_CONTRAST:
            {
                pUsrCtxt->m_usrSettings.contrast_param = p_value->float_value;

                /* Send this parameter to sensor */
                rc = AisInputConfigurer::GetInstance()->SetParam(pUsrCtxt,
                    AIS_INPUT_CTRL_CONTRAST, NULL);
                if (CAMERA_SUCCESS == rc)
                {
                    USR_CTXT_SET_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_CONTRAST);
                }
                else
                {
                    AIS_LOG(ENGINE, ERROR, "ais_input_configurer set param (QCARCAM_PARAM_CONTRAST) failed, rc = %d");
                }
                break;
            }
            case QCARCAM_PARAM_MIRROR_H:
            {
                pUsrCtxt->m_usrSettings.mirror_h_param = p_value->uint_value;

                /* Send this parameter to sensor */
                rc = AisInputConfigurer::GetInstance()->SetParam(pUsrCtxt,
                    AIS_INPUT_CTRL_MIRROR_H, NULL);
                if (CAMERA_SUCCESS == rc)
                {
                    USR_CTXT_SET_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_MIRROR_H);
                }
                else
                {
                    AIS_LOG(ENGINE, ERROR, "ais_input_configurer set param (QCARCAM_PARAM_MIRROR_H) failed, rc = %d");
                }
                break;
            }
            case QCARCAM_PARAM_MIRROR_V:
            {
                pUsrCtxt->m_usrSettings.mirror_v_param = p_value->uint_value;

                /* Send this parameter to sensor */
                rc = AisInputConfigurer::GetInstance()->SetParam(pUsrCtxt,
                    AIS_INPUT_CTRL_MIRROR_V, NULL);
                if (CAMERA_SUCCESS == rc)
                {
                    USR_CTXT_SET_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_MIRROR_V);
                }
                else
                {
                    AIS_LOG(ENGINE, ERROR, "ais_input_configurer set param (QCARCAM_PARAM_MIRROR_V) failed, rc = %d");
                }
                break;
            }
            case QCARCAM_PARAM_MASTER:
            {
                if (p_value->uint_value)
                {
                    rc = AisEvtSetMaster(pUsrCtxt,TRUE);
                }
                else
                {
                    rc = AisEvtReleaseMaster(pUsrCtxt);
                }
                break;
            }
            case QCARCAM_PARAM_EVENT_CHANGE_SUBSCRIBE:
            {
                pUsrCtxt->RegisterEvents(p_value->uint64_value);
                break;
            }
            case QCARCAM_PARAM_EVENT_CHANGE_UNSUBSCRIBE:
            {
                pUsrCtxt->UnRegisterEvents(p_value->uint64_value);
                break;
            }
            case QCARCAM_PARAM_RECOVERY:
            {
                pUsrCtxt->m_usrSettings.recovery = (boolean)p_value->uint_value;
                break;
            }
            case QCARCAM_PARAM_LATENCY_MAX:
            {
                pUsrCtxt->m_usrSettings.n_latency_max = p_value->uint_value;
                AIS_LOG(ENGINE, LOW, "set latency max %u",
                    pUsrCtxt->m_usrSettings.n_latency_max);
                break;
            }
            case QCARCAM_PARAM_LATENCY_REDUCE_RATE:
            {
                pUsrCtxt->m_usrSettings.n_latency_reduce_rate = p_value->uint_value;
                AIS_LOG(ENGINE, LOW, "set latency reduce rate %u",
                    pUsrCtxt->m_usrSettings.n_latency_reduce_rate);
                break;
            }
            case QCARCAM_PARAM_BATCH_MODE:
            {
                pUsrCtxt->m_usrSettings.batch_config = p_value->batch_config;
                AIS_LOG(ENGINE, LOW, "set num_batch_frames %d sof time %d",pUsrCtxt->m_usrSettings.batch_config.num_batch_frames,
                         pUsrCtxt->m_usrSettings.batch_config.detect_first_phase_timer);
                break;
            }
            case QCARCAM_PARAM_ISP_USECASE:
            {
                if (p_value->isp_config.id >= AIS_USER_CTXT_MAX_ISP_INSTANCES)
                {
                    AIS_LOG(ENGINE, ERROR, "invalid isp instance id %u", p_value->isp_config.id);
                    rc = CAMERA_EBADPARM;
                }
                else if (p_value->isp_config.use_case >= QCARCAM_ISP_USECASE_MAX)
                {
                    AIS_LOG(ENGINE, ERROR, "invalid isp use_case %d", p_value->isp_config.use_case);
                    rc = CAMERA_EBADPARM;
                }
                else
                {
                    pUsrCtxt->m_ispInstance[p_value->isp_config.id].cameraId = p_value->isp_config.camera_id;
                    pUsrCtxt->m_ispInstance[p_value->isp_config.id].useCase = p_value->isp_config.use_case;
                    AIS_LOG(ENGINE, LOW, "set isp instance %u config cameraId %u, useCase %d",
                        p_value->isp_config.id, p_value->isp_config.camera_id, p_value->isp_config.use_case);
                    rc = CAMERA_SUCCESS;
                }
                break;
            }
            default:
                AIS_LOG(ENGINE, ERROR, "Unsupported param %d", param);
                rc = CAMERA_EUNSUPPORTED;
                break;
            }
        }

        pUsrCtxt->Unlock();

        PutUsrCtxt(pUsrCtxt);
    }

    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_s_buffers
///
/// @brief Set buffers
///
/// @param hndl       Handle of input
/// @param p_buffers  Pointer to set buffers structure
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult AisEngine::ais_s_buffers(qcarcam_hndl_t hndl, qcarcam_buffers_t* p_buffers)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisUsrCtxt* pUsrCtxt;

    AIS_API_ENTER_HNDL(hndl);

    pUsrCtxt = GetUsrCtxt(hndl);
    if (pUsrCtxt)
    {
        pUsrCtxt->Lock();

        /*check we are in correct state*/
        if (!p_buffers)
        {
            AIS_LOG(ENGINE, ERROR, "p_buffers is NULL");
            rc = CAMERA_EMEMPTR;
        }
        else if (AIS_USR_STATE_OPENED != pUsrCtxt->m_state &&
            AIS_USR_STATE_RESERVED != pUsrCtxt->m_state)
        {
            AIS_LOG(ENGINE, ERROR, "Usrctxt %p in incorrect state %d",
                    pUsrCtxt, pUsrCtxt->m_state);
            rc = CAMERA_EBADSTATE;
        }
        else if (p_buffers->n_buffers < MIN_USR_NUMBER_OF_BUFFERS || p_buffers->n_buffers > pUsrCtxt->m_numBufMax)
        {
            AIS_LOG(ENGINE, ERROR, "Invalid number of buffers set [%d] for user %p. Need in range [%d->%d]",
                    p_buffers->n_buffers, pUsrCtxt, MIN_USR_NUMBER_OF_BUFFERS, pUsrCtxt->m_numBufMax);
            rc = CAMERA_EBADPARM;
        }
        else if ((pUsrCtxt->m_secureMode && !(p_buffers->flags & QCARCAM_BUFFER_FLAG_SECURE)) ||
                 (!pUsrCtxt->m_secureMode && (p_buffers->flags & QCARCAM_BUFFER_FLAG_SECURE)))
        {
            AIS_LOG(ENGINE, ERROR, "Invalid secure flag 0x%x set for usrCtxt secure mode %d",
                    p_buffers->flags, pUsrCtxt->m_secureMode);
            rc = CAMERA_EBADPARM;
        }
        else
        {
            AisBufferList* pBufferList;

            pBufferList = pUsrCtxt->m_bufferList[AIS_BUFLIST_OUTPUT_USR];
            if (pBufferList->m_nBuffers)
            {
                AIS_LOG(ENGINE, HIGH, "ais_s_buffers usrctxt %p unmap buffers", pUsrCtxt);
                rc = AisBufferManager::UnmapBuffers(pBufferList);
            }

            rc = AisBufferManager::MapBuffers(pBufferList, p_buffers);

        }

        pUsrCtxt->Unlock();

        PutUsrCtxt(pUsrCtxt);
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Invalid hndl %p", hndl);
        rc = CAMERA_EBADHANDLE;
    }
    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_s_buffers_v2
///
/// @brief Set buffers for particular buffer list
///
/// @param hndl       Handle of input
/// @param p_bufferlist  Pointer to bufferlist
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult AisEngine::ais_s_buffers_v2(qcarcam_hndl_t hndl, const qcarcam_bufferlist_t* p_bufferlist)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisUsrCtxt* pUsrCtxt;

    AIS_API_ENTER_HNDL(hndl);

    pUsrCtxt = GetUsrCtxt(hndl);
    if (pUsrCtxt)
    {
        pUsrCtxt->Lock();

        /*check we are in correct state*/
        if (!p_bufferlist)
        {
            AIS_LOG(ENGINE, ERROR, "p_buffers is NULL");
            rc = CAMERA_EMEMPTR;
        }
        else if (AIS_USR_STATE_OPENED != pUsrCtxt->m_state &&
            AIS_USR_STATE_RESERVED != pUsrCtxt->m_state)
        {
            AIS_LOG(ENGINE, ERROR, "Usrctxt %p in incorrect state %d",
                    pUsrCtxt, pUsrCtxt->m_state);
            rc = CAMERA_EBADSTATE;
        }
        else if (p_bufferlist->n_buffers < MIN_USR_NUMBER_OF_INPUT_BUFFERS || p_bufferlist->n_buffers > pUsrCtxt->m_numBufMax)
        {
            AIS_LOG(ENGINE, ERROR, "Invalid number of buffers set [%d] for user %p. Need in range [%d->%d]",
                    p_bufferlist->n_buffers, pUsrCtxt, MIN_USR_NUMBER_OF_BUFFERS, pUsrCtxt->m_numBufMax);
            rc = CAMERA_EBADPARM;
        }
        else if (p_bufferlist->id > AIS_BUFLIST_USR_LAST)
        {
            AIS_LOG(ENGINE, ERROR, "Invalid bufferlist id %d", p_bufferlist->id);
            rc = CAMERA_EBADPARM;
        }
        else if (!pUsrCtxt->m_bufferList[p_bufferlist->id])
        {
            AIS_LOG(ENGINE, ERROR, "bufferlist id %d not available for opmode %d",
                    p_bufferlist->id, pUsrCtxt->m_opMode);
            rc = CAMERA_EUNSUPPORTED;
        }
        else if ((pUsrCtxt->m_secureMode && !(p_bufferlist->flags & QCARCAM_BUFFER_FLAG_SECURE)) ||
                 (!pUsrCtxt->m_secureMode && (p_bufferlist->flags & QCARCAM_BUFFER_FLAG_SECURE)))
        {
            AIS_LOG(ENGINE, ERROR, "Invalid secure flag 0x%x set for usrCtxt secure mode %d",
                    p_bufferlist->flags, pUsrCtxt->m_secureMode);
            rc = CAMERA_EBADPARM;
        }
        else
        {
            AisBufferList* pBufferList = pUsrCtxt->m_bufferList[p_bufferlist->id];
            if (pBufferList->m_nBuffers)
            {
                AIS_LOG(ENGINE, HIGH, "usrctxt %p unmap buffers", pUsrCtxt);
                rc = AisBufferManager::UnmapBuffers(pBufferList);
            }

            rc = AisBufferManager::MapBuffersV2(pBufferList, p_bufferlist);
        }

        pUsrCtxt->Unlock();

        PutUsrCtxt(pUsrCtxt);
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Invalid hndl %p", hndl);
        rc = CAMERA_EBADHANDLE;
    }
    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_start
///
/// @brief Start input
///
/// @param hndl       Handle of input
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult AisEngine::ais_start(qcarcam_hndl_t hndl)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisUsrCtxt* pUsrCtxt;

    AIS_API_ENTER_HNDL(hndl);

    pUsrCtxt = GetUsrCtxt(hndl);
    if (pUsrCtxt)
    {

        pUsrCtxt->Lock();

        AisBufferList* pBufferList;
        pBufferList = pUsrCtxt->m_bufferList[AIS_BUFLIST_OUTPUT_USR];

        if (AIS_USR_STATE_OPENED != pUsrCtxt->m_state &&
            AIS_USR_STATE_RESERVED != pUsrCtxt->m_state)
        {
            AIS_LOG(ENGINE, ERROR, "Usrctxt %p in incorrect state %d",
                    pUsrCtxt, pUsrCtxt->m_state);
            pUsrCtxt->Unlock();
            rc = CAMERA_EBADSTATE;
        }
        else if (!pBufferList->m_nBuffers)
        {
            AIS_LOG(ENGINE, ERROR, "Usrctxt %p no buffers set",
                                pUsrCtxt);
            pUsrCtxt->Unlock();
            rc = CAMERA_EBADSTATE;
        }
        else
        {
            /*reserve first*/
            rc = pUsrCtxt->Reserve();
            pUsrCtxt->Unlock();

            Lock();
            if (0 == m_clientCount)
            {
                AIS_LOG(ENGINE, HIGH, "Usrctxt %p, first client", pUsrCtxt);

                /*If detection of pending suspend, cancel delayed suspend*/
                if (AIS_ENGINE_STATE_SUSPEND_PENDING == m_state)
                {
                    m_state = AIS_ENGINE_STATE_READY;
                    CameraSetSignal(m_delaySuspend);
                }
                else if (AIS_ENGINE_STATE_DETECTION_PENDING == m_state)
                {
                     CameraSetSignal(m_delaySuspend);
                }
                else
                {
                    if (m_PowerManagerPolicyType == CAMERA_PM_POLICY_NO_LPM)
                    {
                        PowerResume(CAMERA_POWER_UP, m_isPowerGranular);
                    } else if (m_PowerManagerPolicyType == CAMERA_PM_POLICY_LPM_EVENT_FOR_INPUTS)
                    {
                        PowerResume(CAMERA_POWER_SOC_CLK_ON, FALSE);
                    }
                }
            }

            /*power resume then config and start*/
            if (CAMERA_SUCCESS == rc)
            {
                rc = PowerResumeForStart(pUsrCtxt);
            }

            if (CAMERA_SUCCESS == rc)
            {
                pUsrCtxt->Lock();
                rc = pUsrCtxt->Start();

                if (CAMERA_SUCCESS != rc)
                {
                    (void)pUsrCtxt->Release();
                }

                pUsrCtxt->Unlock();
            }

            if (CAMERA_SUCCESS == rc)
            {
                m_clientCount++;
            }
            Unlock();
        }

        PutUsrCtxt(pUsrCtxt);

        AIS_LOG_ON_ERR(ENGINE, (CAMERA_SUCCESS != rc), "Failed to start usrctx %p rc=%d", pUsrCtxt, rc);
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Invalid hndl %p", hndl);
        rc = CAMERA_EBADHANDLE;
    }

    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_stop
///
/// @brief Stop input that was started
///
/// @param hndl       Handle of input
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult AisEngine::ais_stop(qcarcam_hndl_t hndl)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisUsrCtxt* pUsrCtxt;

    AIS_API_ENTER_HNDL(hndl);

    pUsrCtxt = GetUsrCtxt(hndl);
    if (pUsrCtxt)
    {
        CameraResult tmpRet = CAMERA_SUCCESS;

        pUsrCtxt->Lock();

        if (pUsrCtxt->m_state != AIS_USR_STATE_STREAMING &&
            pUsrCtxt->m_state != AIS_USR_STATE_PAUSED &&
            pUsrCtxt->m_state != AIS_USR_STATE_ERROR)
        {
            AIS_LOG(ENGINE, ERROR, "Usrctxt %p in incorrect state %d",
                                pUsrCtxt, pUsrCtxt->m_state);
            pUsrCtxt->Unlock();
            rc = CAMERA_EBADSTATE;
        }
        else
        {

            //wait for any ppjob in progress to finish
            pUsrCtxt->m_state = AIS_USR_STATE_STOP_PENDING;

            for (int i = 0; i < (int)pUsrCtxt->m_pProcChainDef->nProc; i++)
            {
                if (pUsrCtxt->m_pPProc[i])
                {
                    pUsrCtxt->m_pPProc[i]->Flush(pUsrCtxt,&pUsrCtxt->m_pProcChainDef->pProcChain[i]);
                }
            }

            while (pUsrCtxt->m_ppjobInProgress > 0)
            {
                AIS_LOG(ENGINE, ERROR, "Wait for pproc jobinprogress %d", pUsrCtxt->m_ppjobInProgress);
                pUsrCtxt->Unlock();
                CameraSleep(1);
                pUsrCtxt->Lock();
            }

            pUsrCtxt->Unlock();

            Lock();

            pUsrCtxt->Lock();
            tmpRet = pUsrCtxt->Stop();
            if (rc == CAMERA_SUCCESS) { rc = tmpRet;}
            pUsrCtxt->Unlock();

            //power suspend before releasing resources
            tmpRet = PowerSuspendForStop(pUsrCtxt);
            if (rc == CAMERA_SUCCESS) { rc = tmpRet;}

            pUsrCtxt->Lock();
            tmpRet = pUsrCtxt->Release();
            if (rc == CAMERA_SUCCESS) { rc = tmpRet;}
            pUsrCtxt->Unlock();

            m_clientCount--;
            if (0 == m_clientCount  && m_state != AIS_ENGINE_STATE_DETECTION_PENDING)
            {
                AIS_LOG(ENGINE, HIGH, "Usrctxt %p, last client", pUsrCtxt);
                if (m_PowerManagerPolicyType == CAMERA_PM_POLICY_NO_LPM)
                {
                    PowerSuspend(CAMERA_POWER_DOWN, m_isPowerGranular);
                }
                else if (m_PowerManagerPolicyType == CAMERA_PM_POLICY_LPM_EVENT_FOR_INPUTS)
                {
                    PowerSuspend(CAMERA_POWER_SOC_CLK_OFF, FALSE);
                }
            }

            Unlock();
        }

        PutUsrCtxt(pUsrCtxt);
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Invalid hndl %p", hndl);
        rc = CAMERA_EBADHANDLE;
    }
    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_pause
///
/// @brief Pause input that was started. Does not relinquish resource
///
/// @param hndl       Handle of input
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult AisEngine::ais_pause(qcarcam_hndl_t hndl)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisUsrCtxt* pUsrCtxt;

    AIS_API_ENTER_HNDL(hndl);

    pUsrCtxt = GetUsrCtxt(hndl);
    if (pUsrCtxt)
    {
        pUsrCtxt->Lock();

        /*must be streaming to pause*/
        if (pUsrCtxt->m_state != AIS_USR_STATE_STREAMING)
        {
            AIS_LOG(ENGINE, ERROR, "Usrctxt %p in incorrect state %d",
                                pUsrCtxt, pUsrCtxt->m_state);
            rc = CAMERA_EBADSTATE;
            pUsrCtxt->Unlock();
        }
        else
        {
            //wait for any ppjob in progress to finish
            pUsrCtxt->m_state = AIS_USR_STATE_PAUSE_PENDING;

            while (pUsrCtxt->m_ppjobInProgress > 0)
            {
                AIS_LOG(ENGINE, ERROR, "Wait for pproc jobinprogress %d", pUsrCtxt->m_ppjobInProgress);
                pUsrCtxt->Unlock();
                CameraSleep(1);
                pUsrCtxt->Lock();
            }

            pUsrCtxt->Unlock();

            Lock();

            pUsrCtxt->Lock();
            rc = pUsrCtxt->Pause();
            pUsrCtxt->Unlock();

            Unlock();
        }

        PutUsrCtxt(pUsrCtxt);
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Invalid hndl %p", hndl);
        rc = CAMERA_EBADHANDLE;
    }

    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_resume
///
/// @brief Resumes input that was paused
///
/// @param hndl       Handle of input
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult AisEngine::ais_resume(qcarcam_hndl_t hndl)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisUsrCtxt* pUsrCtxt;

    AIS_API_ENTER_HNDL(hndl);

    pUsrCtxt = GetUsrCtxt(hndl);
    if (pUsrCtxt)
    {
        pUsrCtxt->Lock();

        /*must be in paused state to resume*/
        if (pUsrCtxt->m_state != AIS_USR_STATE_PAUSED)
        {
            rc = CAMERA_EBADSTATE;
            pUsrCtxt->Unlock();
        }
        else
        {
            pUsrCtxt->Unlock();

            Lock();
            pUsrCtxt->Lock();
            rc = pUsrCtxt->Resume();
            pUsrCtxt->Unlock();
            Unlock();
        }

        PutUsrCtxt(pUsrCtxt);
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Invalid hndl %p", hndl);
        rc = CAMERA_EBADHANDLE;
    }
    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_get_frame
///
/// @brief Get available frame
///
/// @param hndl          Handle of input
/// @param p_frame_info  Pointer to frame information that will be filled
/// @param timeout       Max wait time in ms for frame to be available before timeout
/// @param flags         Flags
///
/// @return CAMERA_SUCCESS if successful; CAMERA_EEXPIRED if timeout
///////////////////////////////////////////////////////////////////////////////
CameraResult AisEngine::ais_get_frame(qcarcam_hndl_t hndl,
        qcarcam_frame_info_t* p_frame_info,
        unsigned long long int timeout,
        unsigned int flags)
{
    CameraResult result;
    qcarcam_frame_info_v2_t frame_info_V2 = {};

    result = ais_get_frame_v2(hndl, &frame_info_V2, timeout, flags);
    if (CAMERA_SUCCESS == result)
    {
        p_frame_info->idx = frame_info_V2.idx;
        p_frame_info->flags = frame_info_V2.flags;
        p_frame_info->seq_no = frame_info_V2.seq_no[0];
        p_frame_info->timestamp = frame_info_V2.timestamp;
        p_frame_info->timestamp_system = frame_info_V2.timestamp_system;
        p_frame_info->sof_qtimestamp = frame_info_V2.sof_qtimestamp[0];
        p_frame_info->field_type = frame_info_V2.field_type;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_get_frame_v2
///
/// @brief Get available frame
///
/// @param hndl          Handle of input
/// @param p_frame_info  Pointer to frame information that will be filled
/// @param timeout       Max wait time in ms for frame to be available before timeout
/// @param flags         Flags
///
/// @return CAMERA_SUCCESS if successful; CAMERA_EEXPIRED if timeout
///////////////////////////////////////////////////////////////////////////////
CameraResult AisEngine::ais_get_frame_v2(qcarcam_hndl_t hndl,
        qcarcam_frame_info_v2_t* p_frame_info,
        unsigned long long int timeout,
        unsigned int flags)
{
    CameraResult rc = CAMERA_SUCCESS;
    struct timespec t;
    struct timespec to;
    AisUsrCtxt* pUsrCtxt;

    AIS_API_ENTER_HNDL(hndl);

    pUsrCtxt = GetUsrCtxt(hndl);
    if (pUsrCtxt)
    {
        AisBufferList* pBufferList = NULL;
        uint32 bufferlistId = flags;

        if (!p_frame_info)
        {
            AIS_LOG(ENGINE, ERROR, "p_frame_info is null");
            PutUsrCtxt(pUsrCtxt);
            return CAMERA_EMEMPTR;
        }

        if (bufferlistId > AIS_BUFLIST_USR_LAST)
        {
            AIS_LOG(ENGINE, ERROR, "invalid buffer list id %d", bufferlistId);
            PutUsrCtxt(pUsrCtxt);
            return CAMERA_EMEMPTR;
        }

        pBufferList = pUsrCtxt->m_bufferList[bufferlistId];

        pUsrCtxt->Lock();

        /*must be started to be able to get frame*/
        if (pUsrCtxt->m_state != AIS_USR_STATE_STREAMING &&
            pUsrCtxt->m_state != AIS_USR_STATE_RECOVERY_START &&
            pUsrCtxt->m_state != AIS_USR_STATE_RECOVERY)
        {
            AIS_LOG(ENGINE, ERROR, "Usrctxt %p in incorrect state %d",
                                pUsrCtxt, pUsrCtxt->m_state);
            pUsrCtxt->Unlock();
            PutUsrCtxt(pUsrCtxt);
            return CAMERA_EBADSTATE;
        }

        pUsrCtxt->Unlock();

        if (timeout != QCARCAM_TIMEOUT_NO_WAIT && timeout != QCARCAM_TIMEOUT_INIFINITE)
        {
#ifndef __INTEGRITY
            if (-1 == clock_gettime(CLOCK_MONOTONIC, &to))
#else
            if (-1 == clock_gettime(CLOCK_REALTIME, &to))
#endif
            {
                AIS_LOG(ENGINE, ERROR, "clock_gettime failed: %s", strerror(errno));
                rc = ERRNO_TO_RESULT(errno);
            }
            else
            {
#if defined (__QNXNTO__)
                nsec2timespec(&t, timeout);
#else
                t.tv_sec = timeout / NANOSEC_TO_SEC;
                t.tv_nsec = timeout % NANOSEC_TO_SEC;
#endif

                AIS_LOG(ENGINE, LOW, "t.tv_sec %d t.tv_nsec %d: timeout %d ns",
                    t.tv_sec,  t.tv_nsec, timeout);

                to.tv_sec  += t.tv_sec;
                to.tv_nsec += t.tv_nsec;
                if (to.tv_nsec >= NANOSEC_TO_SEC)
                {
                    to.tv_sec  += 1;
                    to.tv_nsec -= NANOSEC_TO_SEC;
                }
            }
        }

        if (CAMERA_SUCCESS == rc)
        {
            int result = EOK;
            boolean isEmpty = TRUE;

            pthread_mutex_lock(&pBufferList->m_bufferDoneQMutex);

            do {
                /*in case of abort*/
                if (pUsrCtxt->m_state != AIS_USR_STATE_STREAMING &&
                    pUsrCtxt->m_state != AIS_USR_STATE_RECOVERY_START &&
                    pUsrCtxt->m_state != AIS_USR_STATE_RECOVERY)
                {
                    AIS_LOG(ENGINE, ERROR, "abort called %d", pUsrCtxt->m_state);
                    rc = CAMERA_ENOMORE;
                    isEmpty = TRUE;
                    break;
                }

                rc = CameraQueueIsEmpty(pBufferList->m_bufferDoneQ, &isEmpty);

                if (isEmpty)
                {
                    if (QCARCAM_TIMEOUT_NO_WAIT == timeout)
                    {
                        rc = CAMERA_EEXPIRED;
                    }
                    else if (QCARCAM_TIMEOUT_INIFINITE == timeout)
                    {
                        result = pthread_cond_wait(&pBufferList->m_bufferDoneQCond,
                                &pBufferList->m_bufferDoneQMutex);
                    }
                    else
                    {
                        result = pthread_cond_timedwait(&pBufferList->m_bufferDoneQCond,
                                &pBufferList->m_bufferDoneQMutex, &to);
                    }
                }
                else
                {
                    //Something in Q, break out of loop
                    result = EOK;
                    break;
                }

                AIS_LOG(ENGINE, LOW, "ais_get_frame usrctxt %p(%d) isEmpty %d result %d rc %d",
                        pUsrCtxt, bufferlistId, isEmpty, result, rc);

            } while (isEmpty && (EOK == result) && rc == CAMERA_SUCCESS);

            if (EOK != result)
            {
                AIS_LOG(ENGINE, ERROR, "pthread_cond_wait failed: %s", strerror(result));
                rc = ERRNO_TO_RESULT(result);
            }
            else if(CAMERA_SUCCESS != rc)
            {
                AIS_LOG(ENGINE, ERROR, "CameraQueueIsEmpty call failed: %d", rc);
            }
            else if (!isEmpty)
            {
                rc = CameraQueueDequeue(pBufferList->m_bufferDoneQ, p_frame_info);

                if (rc == CAMERA_SUCCESS)
                {
                    rc = pBufferList->SetBufferState(p_frame_info->idx, AIS_USER_ACQUIRED);

                    AIS_LOG(ENGINE, LOW, "usrctxt %p(%d) idx=%d", pUsrCtxt, bufferlistId, p_frame_info->idx);
                }
            }

            pthread_mutex_unlock(&pBufferList->m_bufferDoneQMutex);

        }
        PutUsrCtxt(pUsrCtxt);
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Invalid hndl %p", hndl);
        rc = CAMERA_EBADHANDLE;
    }
    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_release_frame
///
/// @brief Re-enqueue frame buffers
///
/// @param hndl       Handle of input
/// @param idx        Index into the qcarcam_buffers_t buffers table to reenqueue
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult AisEngine::ais_release_frame(qcarcam_hndl_t hndl, unsigned int idx)
{
    return ais_release_frame_v2(hndl, 0, idx);
}

///////////////////////////////////////////////////////////////////////////////
/// ais_release_frame_v2
///
/// @brief Re-enqueue frame buffers
///
/// @param hndl       Handle of input
/// @param id         bufferlist id
/// @param idx        Index into the qcarcam_buffers_t buffers table to reenqueue
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult AisEngine::ais_release_frame_v2(qcarcam_hndl_t hndl, unsigned int id, unsigned int idx)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisUsrCtxt* pUsrCtxt;

    AIS_API_ENTER_HNDL(hndl);

    pUsrCtxt = GetUsrCtxt(hndl);
    if (pUsrCtxt)
    {
        pUsrCtxt->Lock();

        /*must be streaming to be able to release frame*/
        if (pUsrCtxt->m_state != AIS_USR_STATE_STREAMING)
        {
            AIS_LOG(ENGINE, ERROR, "usrctxt %p in incorrect state %d",
                                pUsrCtxt, pUsrCtxt->m_state);
            rc = CAMERA_EBADSTATE;
        }
        else if (id > AIS_BUFLIST_USR_LAST)
        {
            AIS_LOG(ENGINE, ERROR, "invalid buffer list id %d", id);
            rc = CAMERA_EBADPARM;
        }
        else
        {
            AisBufferList* pBufferList = pUsrCtxt->m_bufferList[id];
            if (pBufferList->GetBufferState(idx) == AIS_USER_ACQUIRED)
            {
                rc = pBufferList->ReturnBuffer(pUsrCtxt, idx);
                if( rc != CAMERA_SUCCESS)
                {
                    AIS_LOG(ENGINE, ERROR, "PutBuf %u failed %d", idx, rc);
                }
            }
            else
            {
                AIS_LOG(ENGINE, ERROR, "buffer %u has already been released", idx);
                rc = CAMERA_EBADSTATE;
            }
        }

        pUsrCtxt->Unlock();

        PutUsrCtxt(pUsrCtxt);
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Invalid hndl %p", hndl);
        rc = CAMERA_EBADHANDLE;
    }
    return rc;
}


//////////////////////////////////////////////////////////////////////////////////
/// INTERNAL ENGINE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////

/**
 * TraverseUsrCtxt
 *
 * @brief Executes a function on all contexts in a AisUsrCtxtList
 *
 * @param pList  List of user contexts to traverse
 * @param opFunc AisUsrCtxtOperateFunc to call on pList user contexts
 * @param pData  Parameter to opFunc
 *
 * @return None
 */
void AisEngine::TraverseUsrCtxt(AisUsrCtxtList* pList,
        AisUsrCtxtOperateFunc opFunc, void* pData)
{
    AisUsrCtxtList::iterator it;

    for (it = pList->begin(); it != pList->end(); ++it)
    {
        opFunc(*it, pData);
    }
}

/**
 * SendErrorCb
 *
 * @brief handles kinds of errors. Notifies users they have been aborted and enters user
 * context state machine to error state. User will have to take appropriate measures.
 * Usually it is stop/start use case or close the context altogether.
 *
 * @note The caller must lock the UsrCtxt mutex before calling the function, and unlock it afterwards
 *
 * @param pPayload  qcarcam event payload
 *
 * @return None
 */
void AisUsrCtxt::SendErrorCb(qcarcam_event_payload_t* pPayload)
{
    AIS_LOG(ENGINE, WARN, "Fatal Error for context %p", this);

    if (AIS_USR_STATE_STREAMING == m_state)
    {
        m_state = AIS_USR_STATE_ERROR;
        if (m_eventCbFcn && (m_eventMask & QCARCAM_EVENT_ERROR))
        {
            AIS_LOG(ENGINE, HIGH, "Send error for context %p", this);
            m_eventCbFcn(m_qcarcamHndl, QCARCAM_EVENT_ERROR, pPayload);
        }
    }
}

//===============================
/**
 * HandleAISErrorEvent
 *
 * @brief either initiates recovery, or calls SendErrorCb,
 * depending on if error recovery is enabled or not
 * @param pUsrCtxt
 * @param user_data
 * @return boolean TRUE if success
 */
static CameraResult HandleAISErrorEvent(AisUsrCtxt* pUsrCtxt, void* pData)
{
    AIS_LOG(ENGINE, HIGH, "Error event for context %p", pUsrCtxt);

    CameraResult rc = CAMERA_SUCCESS;
    AisEventErrorHandlerType* pErrorEvent = (AisEventErrorHandlerType*)pData;

    pUsrCtxt->Lock();
    if (pUsrCtxt->m_usrSettings.recovery)
    {
        switch(pUsrCtxt->m_state)
        {
            case AIS_USR_STATE_ERROR:
                //@TODO: add necessary logic to determine if we are coming in from a recovery retry attempt
                // For now do nothing and ignore errors if we are in error state
                break;
            case AIS_USR_STATE_STREAMING:
                pUsrCtxt->m_state = AIS_USR_STATE_RECOVERY_START;
                pUsrCtxt->m_numRecoveryAttempts++;
                pUsrCtxt->m_totalRecoveryAttempts++;

                AIS_LOG(ENGINE, WARN, "Initiate recovery for context %p (%d,%d)", pUsrCtxt,
                        pUsrCtxt->m_numRecoveryAttempts, pUsrCtxt->m_totalRecoveryAttempts);

                while (pUsrCtxt->m_ppjobInProgress > 0)
                {
                    AIS_LOG(ENGINE, ERROR, "Wait for pproc jobinprogress %d", pUsrCtxt->m_ppjobInProgress);
                    pUsrCtxt->Unlock();
                    CameraSleep(1);
                    pUsrCtxt->Lock();
                }

                rc = pUsrCtxt->Stop();

                if (rc == CAMERA_SUCCESS)
                {
                    pErrorEvent->bRecoveryEnabled = TRUE;

                    if (pUsrCtxt->m_eventCbFcn && (pUsrCtxt->m_eventMask & QCARCAM_EVENT_RECOVERY))
                    {
                        AIS_LOG(ENGINE, HIGH, "Send recovery start for context %p", pUsrCtxt);
                        pUsrCtxt->m_eventCbFcn(pUsrCtxt->m_qcarcamHndl,
                                               QCARCAM_EVENT_RECOVERY,
                                               &pErrorEvent->eventPayload);
                    }
                }
                else
                {
                    pUsrCtxt->ProcessRecoveryFailEvent();
                }
                break;
            case AIS_USR_STATE_RECOVERY_START:
                //ignore errors during recovery start
                break;
            case AIS_USR_STATE_RECOVERY:
                //errors during recovery is fatal
                rc = pUsrCtxt->ProcessRecoveryFailEvent();
                break;
            default:
                //some handling of the other states is necessary
                rc = pUsrCtxt->ProcessRecoveryFailEvent();
                break;
        }
    }
    else
    {
        pUsrCtxt->SendErrorCb(&pErrorEvent->eventPayload);
    }

    pUsrCtxt->Unlock();

    return rc;
}

/**
 * UsrCtxtRetryRecoveryTimerHandler
 *
 * @brief Attempts to initiate another recovery after a set amount of time
 * if the current one has failed (timed out due to a frame done event
 * not being received after a set time after successfully calling AisUsrCtxt::Start)
 *
 * @param pData - the user context, which is in recovery
 *
 * @return None
 */
static void UsrCtxtRetryRecoveryTimerHandler(void *pData)
{
    if (pData != NULL)
    {
        CameraResult rc;
        AisUsrCtxt *pUsrCtxt = (AisUsrCtxt *)pData;
        AisEventErrorHandlerType errorEvent;
        AisEngine* pEngine = AisEngine::GetInstance();

        errorEvent.eventPayload.uint_payload = QCARCAM_FATAL_ERROR;
        rc = HandleAISErrorEvent(pUsrCtxt, &errorEvent);
        if (CAMERA_SUCCESS == rc)
        {
            CameraSleep(pEngine->m_engineSettings->recoveryRestartDelay);
            rc = CompleteRecovery(pUsrCtxt, NULL);
        }
        if (CAMERA_SUCCESS != rc) //fatal error, recovery retry attempt failed
        {
            pUsrCtxt->Lock();
            pUsrCtxt->m_state = AIS_USR_STATE_ERROR;
            pUsrCtxt->Unlock();
            AIS_LOG(ENGINE, ERROR, "Recovery retry attempt failed");
        }
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Null data pased to retry recovery timer handler");
    }
}

/**
 * UsrCtxtRecoveryTimeoutTimerHandler
 *
 * @brief When a user context is undergoing recovery, checks if a frame done
 * event is received after a set time after successfully calling AisUsrCtxt::Start
 * For a successful recovery, the event should be received in the specified time frame.
 * If it isn't, the recovery attempt has failed and this handler is triggered.
 * @param data - the user context, which is in recovery
 * @return nothing
 */
static void UsrCtxtRecoveryTimeoutTimerHandler(void *data)
{
    if (data != NULL)
    {
        AisUsrCtxt *pUsrCtxt = (AisUsrCtxt *)data;
        AisEngine* pEngine = AisEngine::GetInstance();

        AIS_LOG(ENGINE, ERROR, "Recovery failed for context %p", pUsrCtxt);

        pUsrCtxt->Lock();

        pUsrCtxt->m_state = AIS_USR_STATE_ERROR;

        if (pUsrCtxt->m_numRecoveryAttempts < pEngine->m_engineSettings->recoveryMaxNumAttempts)
        {
            if (!pUsrCtxt->m_RetryRecoveryTimer)
            {
                if (CameraCreateTimer(pEngine->m_engineSettings->recoveryRetryDelay,
                                      0,
                                      UsrCtxtRetryRecoveryTimerHandler,
                                      (void *)pUsrCtxt,
                                      &pUsrCtxt->m_RetryRecoveryTimer))
                {
                    AIS_LOG(ENGINE, ERROR, "CameraCreateTimer failed for retry recovery attempt");
                    pUsrCtxt->m_state = AIS_USR_STATE_ERROR;
                }
            }
            else
            {
                if (CameraUpdateTimer(pUsrCtxt->m_RetryRecoveryTimer, pEngine->m_engineSettings->recoveryRetryDelay))
                {
                    AIS_LOG(ENGINE, ERROR, "CameraUpdateTimer failed for retry recovery attempt");
                }
            }
        }

        pUsrCtxt->ProcessRecoveryFailEvent();
        pUsrCtxt->Unlock();
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Null data pased to recovery timer handler");
    }
}

/**
 * AisUsrCtxtDecRefCnt
 *
 * @brief decrements ref count for user context
 *
 * @param pUsrCtxt
 * @param pUsrData
 *
 * @return boolean TRUE if success
 */
static CameraResult AisUsrCtxtDecRefCnt(AisUsrCtxt* pUsrCtxt, void* pUsrData)
{
    pUsrCtxt->DecRefCnt();
    return CAMERA_SUCCESS;
}

/**
 * CompleteRecovery
 *
 * @brief calls start after all affected user contexts have been stopped
 * @param pUsrCtxt
 * @param pUsrData
 *
 * @return CameraResult
 */
static CameraResult CompleteRecovery(AisUsrCtxt* pUsrCtxt, void* pData)
{
    AIS_LOG(ENGINE, HIGH, "Complete Recovery for context %p", pUsrCtxt);

    CameraResult rc = CAMERA_SUCCESS;

    pUsrCtxt->Lock();
    if (AIS_USR_STATE_RECOVERY_START == pUsrCtxt->m_state && pUsrCtxt->m_usrSettings.recovery)
    {
        AIS_LOG(ENGINE, WARN, "Recovery start context %p", pUsrCtxt);
        rc = pUsrCtxt->Start();
        if (rc == CAMERA_SUCCESS)
        {
            AisEngine* pEngine = AisEngine::GetInstance();
            pUsrCtxt->m_state = AIS_USR_STATE_RECOVERY;

            if (!pUsrCtxt->m_RecoveryTimeoutTimer)
            {
                if (CameraCreateTimer(pEngine->m_engineSettings->recoveryTimeoutAfterUsrCtxtRestart,
                                      0,
                                      UsrCtxtRecoveryTimeoutTimerHandler,
                                      (void *)pUsrCtxt,
                                      &pUsrCtxt->m_RecoveryTimeoutTimer))
                {
                    AIS_LOG(ENGINE, ERROR, "CameraCreateTimer failed");
                    rc = CAMERA_EFAILED;
                }
            }
            else
            {
                if (CameraUpdateTimer(pUsrCtxt->m_RecoveryTimeoutTimer, pEngine->m_engineSettings->recoveryTimeoutAfterUsrCtxtRestart))
                {
                    AIS_LOG(ENGINE, ERROR, "CameraUpdateTimer failed");
                    rc = CAMERA_EFAILED;
                }
            }
        }
    }

    if (rc != CAMERA_SUCCESS)
    {
        rc = pUsrCtxt->ProcessRecoveryFailEvent();
    }

    pUsrCtxt->Unlock();

    return rc;
}

/**
 * FailedRecovery
 *
 * @brief Recovery Failure handling. Send recovery failed event for the user.
 * @param pUsrCtxt
 * @param pUsrData
 *
 * @return CameraResult
 */
static CameraResult FailedRecovery(AisUsrCtxt* pUsrCtxt, void* pData)
{
    CameraResult rc = CAMERA_SUCCESS;

    AIS_LOG(ENGINE, HIGH, "Failed Recovery for context %p", pUsrCtxt);

    pUsrCtxt->Lock();

    rc = pUsrCtxt->ProcessRecoveryFailEvent();

    pUsrCtxt->Unlock();

    return rc;
}

/**
 * AisUsrCtxtInputStatus
 *
 * @brief handles input signal changes. Notifies users of input signal event
 *
 * @param pUsrCtxt
 * @param pData
 * @return CameraResult
 */
static CameraResult AisUsrCtxtSendInputStatus(AisUsrCtxt* pUsrCtxt, void* pData)
{
    if (pUsrCtxt->m_eventCbFcn && (pUsrCtxt->m_eventMask & QCARCAM_EVENT_INPUT_SIGNAL))
    {
        AIS_LOG(ENGINE, HIGH, "Send input Status for context %p", pUsrCtxt);
        pUsrCtxt->m_eventCbFcn(pUsrCtxt->m_qcarcamHndl, QCARCAM_EVENT_INPUT_SIGNAL, (qcarcam_event_payload_t*)pData);
    }

    return CAMERA_SUCCESS;
}

/**
 * AisUsrCtxtSendFrameFreeze
 *
 * @brief Notifies users of frame freeze event
 *
 * @param pUsrCtxt
 * @param pData
 * @return CameraResult
 */
static CameraResult AisUsrCtxtSendFrameFreeze(AisUsrCtxt* pUsrCtxt, void* pData)
{
    if (pUsrCtxt->m_eventCbFcn && (pUsrCtxt->m_eventMask & QCARCAM_EVENT_FRAME_FREEZE))
    {
        AIS_LOG(ENGINE, HIGH, "Send frame freeze for context %p", pUsrCtxt);
        pUsrCtxt->m_eventCbFcn(pUsrCtxt->m_qcarcamHndl, QCARCAM_EVENT_FRAME_FREEZE, (qcarcam_event_payload_t*)pData);
    }

    return CAMERA_SUCCESS;
}

/**
*
 * AisUsrCtxtVendorEvent
 *
 * @brief handles vendor event.
 *
 * @param pUsrCtxt
 * @param pData
 * @return CameraResult
 */
static CameraResult AisUsrCtxtSendVendorEvent(AisUsrCtxt* pUsrCtxt, void* pData)
{
    if (pUsrCtxt->m_eventCbFcn && (pUsrCtxt->m_eventMask & QCARCAM_EVENT_VENDOR))
    {
        AIS_LOG(ENGINE, HIGH, "Send vendor event for context %p", pUsrCtxt);
        pUsrCtxt->m_eventCbFcn(pUsrCtxt->m_qcarcamHndl, QCARCAM_EVENT_VENDOR, (qcarcam_event_payload_t*)pData);
    }

    return CAMERA_SUCCESS;
}

/**
 * UpdateDiagErrInfo
 *
 * @brief Updates the error info to diagnostic structures
 *
 * @param pUsrCtxt
 * @param msg - error msg
 * @return errType - type of error
 */
void AisEngine::UpdateDiagErrInfo(void* pErr, AisEventId errType, AisUsrCtxt* pUsrCtxt)
{
    if (m_DiagManager == NULL)
    {
        AIS_LOG(ENGINE, ERROR, "DiagManager instance not available");
        return;
    }

    QCarCamDiagErrorInfo* pErrInfo = m_DiagManager->GetErrorQueueTop();

    if (pErrInfo == NULL)
    {
        AIS_LOG(ENGINE, ERROR, "no memory for diagnostic error info ");
        return;
    }

    pErrInfo->errorType = errType;

    switch(errType)
    {
        case AIS_EVENT_CSID_FATAL_ERROR:
        case AIS_EVENT_CSID_WARNING:
        {
            AisEventIfeErrorType* pMsg = (AisEventIfeErrorType*)pErr;
            pErrInfo->ifeDevId = pMsg->ifeCore;
            pErrInfo->errorTimeStamp = pMsg->timestamp;
            memcpy(pErrInfo->payload, &pMsg->errorStatus, sizeof(pMsg->errorStatus));
            break;
        }
        case AIS_EVENT_IFE_OUTPUT_ERROR:
        case AIS_EVENT_IFE_OUTPUT_WARNING:
        {
            AisEventIfeErrorType* pMsg = (AisEventIfeErrorType*)pErr;
            pErrInfo->ifeDevId = pMsg->ifeCore;
            pErrInfo->errorTimeStamp = pMsg->timestamp;
            pErrInfo->rdiId = pMsg->ifeOutput;
            memcpy(pErrInfo->payload, &pMsg->errorStatus, sizeof(pMsg->errorStatus));
            break;
        }
        case AIS_EVENT_INPUT_STATUS:
        {
            AisEventInputSignalType* pMsg = (AisEventInputSignalType*)pErr;
            pErrInfo->inputDevId = pMsg->devId;
            pErrInfo->inputSrcId = pMsg->srcId;
            memcpy(pErrInfo->payload, &pMsg->status, sizeof(pMsg->status));
            break;
        }
        case AIS_EVENT_INPUT_FRAME_FREEZE:
        {
            AisEventInputFrameFreezeType* pMsg = (AisEventInputFrameFreezeType*)pErr;
            pErrInfo->inputDevId = pMsg->devId;
            pErrInfo->inputSrcId = pMsg->srcId;
            memcpy(pErrInfo->payload, &pMsg->status, STD_MIN(sizeof(pErrInfo->payload), sizeof(pMsg->status)));
            break;
        }
        case AIS_EVENT_CSIPHY_WARNING:
        {
            AisEventCsiStatusType* pMsg = (AisEventCsiStatusType*)pErr;
            pErrInfo->csiphyDevId = pMsg->csiphyId;
            memcpy(pErrInfo->payload, pMsg->status, pMsg->statusMsgSize);
            break;
        }
        default:
            break;
    }

    if (pUsrCtxt)
    {
        pErrInfo->usrHdl      = pUsrCtxt->m_qcarcamHndl;
        pErrInfo->inputSrcId  = pUsrCtxt->m_inputId;
        pErrInfo->inputDevId  = pUsrCtxt->m_streams[0].inputCfg.devId;
        pErrInfo->csiphyDevId = pUsrCtxt->m_streams[0].resources.csiphy;
        pErrInfo->ifeDevId    = pUsrCtxt->m_streams[0].resources.csid;
        pErrInfo->rdiId       = (uint32)pUsrCtxt->m_streams[0].resources.ifeStream.ifeOutput;
    }
}

/**
 * AisUsrCtxt::AddSofFieldInfo
 *
 * @brief  Add SOF field info for user context
 *
 * @param AisFieldInfoType*
 *
 * @return CameraResult  CAMERA_SUCCESS on success
 */
CameraResult AisUsrCtxt::AddSofFieldInfo(AisFieldInfoType* pFieldInfo)
{
    CameraResult result = CAMERA_SUCCESS;
    int i;

    //context must be streaming
    if (m_state != AIS_USR_STATE_STREAMING &&
        m_state != AIS_USR_STATE_RECOVERY)
    {
        AIS_LOG(ENGINE, HIGH, "Don't insert field info into context %p in wrong state %d",
                        this, m_state);
        return CAMERA_EBADSTATE;
    }

    for (i = 0; i < AIS_FIELD_BUFFER_SIZE; i++)
    {
        if (m_fieldInfo[i].valid != TRUE)
        {
            m_fieldInfo[i] = *pFieldInfo;
            m_fieldInfo[i].valid = TRUE;
            AIS_LOG(ENGINE, MED, "Insert [%d]: %llu, %llu, %d %d",
                i, m_fieldInfo[i].sofTimestamp/1000,
                m_fieldInfo[i].timestamp/1000,
                m_fieldInfo[i].frameId,
                m_fieldInfo[i].fieldType);
            break;
        }
    }

    if (i == AIS_FIELD_BUFFER_SIZE)
    {
        int min_idx = 0;
        uint64_t min_sof_ts = m_fieldInfo[0].sofTimestamp;
        for (i = 1; i < AIS_FIELD_BUFFER_SIZE; i++)
        {
            if (m_fieldInfo[i].sofTimestamp < min_sof_ts)
            {
                min_sof_ts = m_fieldInfo[i].sofTimestamp;
                min_idx = i;
            }
        }
        m_fieldInfo[min_idx] = *pFieldInfo;
        m_fieldInfo[min_idx].valid = TRUE;
        AIS_LOG(ENGINE, MED, "Overwrite [%d]: %llu, %llu, %d %d",
            min_idx, m_fieldInfo[min_idx].sofTimestamp/1000,
            m_fieldInfo[min_idx].timestamp/1000,
            m_fieldInfo[min_idx].frameId,
            m_fieldInfo[min_idx].fieldType);
    }

    return result;
}

/**
 * AisUsrCtxt::GetFrameFieldInfo
 *
 * @brief figure out field type in the current frame
 *
 * @param AisEventFrameDoneType*
 *
 * @return CameraResult  CAMERA_SUCCESS on success
 */
CameraResult AisUsrCtxt::GetFrameFieldInfo(AisEventFrameDoneType* pFrameDone)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32_t frameId = pFrameDone->frameInfo.seq_no[0];
    uint64_t frameTs = pFrameDone->frameInfo.timestamp;
    AisFieldInfoType fieldInfo = {};

    for (int i = 0; i < AIS_FIELD_BUFFER_SIZE; i++)
    {
        if (m_fieldInfo[i].valid == TRUE &&
            m_fieldInfo[i].frameId <= frameId)
        {
            //match if same frameId and read timestamp is newer than sof (takes care of start/stop scenarios)
            if (m_fieldInfo[i].frameId == frameId &&
                m_fieldInfo[i].timestamp > fieldInfo.sofTimestamp)
            {
                fieldInfo = m_fieldInfo[i];
            }

            m_fieldInfo[i].valid = FALSE;
        }
    }

    if (fieldInfo.valid)
    {
        //ensure the timestamp of field read is within threshold
        if ((fieldInfo.timestamp > fieldInfo.sofTimestamp) &&
            ((fieldInfo.timestamp - fieldInfo.sofTimestamp) < AIS_FIELD_READ_TIME_THRESHOLD))
        {
            pFrameDone->frameInfo.field_type = fieldInfo.fieldType;
        }
        else
        {
            pFrameDone->frameInfo.field_type = QCARCAM_FIELD_UNKNOWN;
            AIS_LOG(ENGINE, ERROR, "Trustless field info %llu-%llu  %d %llu %d vs %d",
                    fieldInfo.sofTimestamp, fieldInfo.timestamp, fieldInfo.fieldType, frameTs,
                    fieldInfo.frameId, frameId);
        }
    }
    else
    {
        pFrameDone->frameInfo.field_type = QCARCAM_FIELD_UNKNOWN;
        AIS_LOG(ENGINE, ERROR, "Not enough field info for current frame %d %llu", frameId, frameTs);
        for (int i = 0; i < AIS_FIELD_BUFFER_SIZE; i++)
        {
            AIS_LOG(ENGINE, ERROR, "[%d]: %d %llu-%llu (diff %llu us) %d %d", i, m_fieldInfo[i].valid,
                           m_fieldInfo[i].sofTimestamp, m_fieldInfo[i].timestamp,
                           (m_fieldInfo[i].timestamp - m_fieldInfo[i].sofTimestamp)/1000,
                           m_fieldInfo[i].frameId, m_fieldInfo[i].fieldType);
        }
    }

    return rc;
}

/**
 * AisUsrCtxtSendNotificationCb
 *
 * @brief Notifies User if there are any changes on property settings.
 *
 * @param pUsrCtxt
 * @param pData
 * @return CameraResult
 */

static CameraResult AisUsrCtxtSendNotificationCb(AisUsrCtxt* pUsrCtxt, void* pData)
{
    if (NULL != pUsrCtxt &&  NULL != pUsrCtxt->m_eventCbFcn)
    {
        pUsrCtxt->m_eventCbFcn(pUsrCtxt->m_qcarcamHndl, QCARCAM_EVENT_PROPERTY_NOTIFY, (qcarcam_event_payload_t*)pData);
    }

    return CAMERA_SUCCESS;
}

/**
 * QueueEvent
 *
 * @brief Queues events for engine
 *
 * @param pMsg
 *
 * @return CameraResult
 */
CameraResult AisEngine::QueueEvent(AisEventMsgType* pMsg)
{
    CameraResult result;

    AIS_LOG(ENGINE, LOW, "q_event %d", pMsg->eventId);

    result = CameraQueueEnqueue(m_eventQ[AIS_ENGINE_QUEUE_EVENTS], pMsg);
    if (result == CAMERA_SUCCESS)
    {
        result = CameraSetSignal(m_eventHandlerSignal);
    }
    else
    {
        AIS_LOG(ENGINE, ERROR,"Failed to enqueue event %d (%d)", pMsg->eventId, result);
    }
    return result;
}

/**
 * UpdateUsrCtxtInfo
 *
 * @brief Updates the frame rate
 *
 * @return CameraResult
 */
void AisEngine::UpdateUsrCtxtInfo()
{
    for (uint32 idx = 0; idx < AIS_MAX_USR_CONTEXTS; idx++)
    {
        AisUsrCtxt* pUsrCtxt = AcquireUsrCtxt(idx);
        if (pUsrCtxt)
        {
            pUsrCtxt->UpdateFrameRate();

            RelinquishUsrCtxt(idx);
        }
    }
}

/**
 * ProcessRawFrameDone
 *
 * @brief Process RAW frame done event. Queues new PPROC job.
 *
 * @param pMsg
 */
void AisEngine::ProcessRawFrameDone(AisEventMsgType* pMsg)
{
    CameraResult rc;
    AisUsrCtxt* pUsrCtxt = NULL;
    AisEventFrameDoneType* pFrameDone = &pMsg->payload.ifeFrameDone;

    AisResMgrMatch sMatch = {};
    AisUsrCtxtList matchList;
    sMatch.matchType = AIS_RESMGR_MATCH_IFE_PATH;
    sMatch.dataType = AIS_RESMGR_MATCH_DATA_IFE;
    sMatch.device = pFrameDone->ifeCore;
    sMatch.path = pFrameDone->ifeOutput;

    rc = m_ResourceManager->MatchUserList(&sMatch, &matchList);
    if (CAMERA_SUCCESS == rc && !matchList.empty())
    {
        pUsrCtxt = matchList.front();
        matchList.pop_front();
    }

    if (pUsrCtxt)
    {
        AisEventMsgType sNewJob = {};
        uint32 streamIdx;

        pUsrCtxt->Lock();

        // move to streaming state on receiving first frame during recovery
        if (AIS_USR_STATE_RECOVERY == pUsrCtxt->m_state)
        {
            pUsrCtxt->ProcessRecoverySuccessEvent();
        }

        if (pUsrCtxt->m_state != AIS_USR_STATE_STREAMING)
        {
            AIS_LOG(ENGINE, HIGH, "ctxt %p aborted!", pUsrCtxt);
            pUsrCtxt->Unlock();
            pUsrCtxt->DecRefCnt();
            return;
        }

        pUsrCtxt->m_numFrameDone++;

        if (pUsrCtxt->m_isPendingStart)
        {
            AIS_LOG(ENGINE, HIGH, "IFE %d Output %d First Frame buffer %d",
                    pFrameDone->ifeCore, pFrameDone->ifeOutput, pFrameDone->frameInfo.idx);
            pUsrCtxt->m_isPendingStart = FALSE;
        }

        AisIfeStreamType* pIfeStream = NULL;
        for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams; streamIdx++)
        {
            pIfeStream = &pUsrCtxt->m_streams[streamIdx].resources.ifeStream;
            if (pIfeStream->ifeCore == pFrameDone->ifeCore &&
                pIfeStream->ifeOutput == pFrameDone->ifeOutput)
            {
                break;
            }
        }

        if (streamIdx == pUsrCtxt->m_numStreams)
        {
            AIS_LOG(ENGINE, ERROR, "Could not find matching stream for FrameDone %d %d",
                    pFrameDone->ifeCore, pFrameDone->ifeOutput);
            streamIdx = 0;
            pIfeStream = &pUsrCtxt->m_streams[streamIdx].resources.ifeStream;
        }

        if (pUsrCtxt->m_streams[streamIdx].inputCfg.inputModeInfo.interlaced == INTERLACED_QUERY_FIELD_TYPE)
        {
            pUsrCtxt->GetFrameFieldInfo(pFrameDone);
        }

        AisBufferList* pBufferList = NULL;
        AisBuffer* pBuffer = NULL;
        if (NULL != pIfeStream &&
            pIfeStream->bufferListIdx >= 0 &&
            pIfeStream->bufferListIdx < AIS_BUFLIST_MAX)
        {
            pBufferList = pUsrCtxt->m_bufferList[pIfeStream->bufferListIdx];
            if (NULL != pBufferList)
            {
                pBuffer = pBufferList->GetBuffer(pFrameDone->frameInfo.idx);
            }
        }

        if (NULL == pBuffer || NULL == pBufferList)
        {
            if (NULL != pBufferList)
            {
                AIS_LOG(ENGINE, HIGH, "ctxt %p invalid buffer index %d (%d, %d)",
                    pUsrCtxt, pFrameDone->frameInfo.idx, pBufferList->GetId(), pBufferList->m_nBuffers);
            }
            pUsrCtxt->Unlock();
            pUsrCtxt->DecRefCnt();
            return;
        }

        pUsrCtxt->Unlock();


        //post process frame data if not secure
        if (!pUsrCtxt->m_secureMode)
        {
            rc = ((AisInputConfigurer*)m_Configurers[AIS_CONFIGURER_INPUT])->ProcessFrameData(pUsrCtxt, pBuffer, &pFrameDone->frameInfo);
        }

        uint64 jobId = AisCreateJobId(pUsrCtxt,
                streamIdx,
                pFrameDone->frameInfo.seq_no[0],
                pFrameDone->ifeCore, pFrameDone->ifeOutput);

        //push buffer as ready to consume and queue PPROC event
        pBufferList->QueueReadyBuffer(jobId, pBuffer);

        sNewJob.eventId = AIS_EVENT_PPROC_JOB;
        sNewJob.payload.pprocJob.pUsrCtxt = pUsrCtxt;
        sNewJob.payload.pprocJob.jobId = jobId;
        sNewJob.payload.pprocJob.currentHop = 0;
        sNewJob.payload.pprocJob.streamIdx = streamIdx;
        sNewJob.payload.pprocJob.frameInfo = pFrameDone->frameInfo;

        if (pUsrCtxt->m_usrSettings.isp_ctrls.param_mask & (1 << QCARCAM_CONTROL_DUMP_FRAME))
        {
            sNewJob.payload.pprocJob.bDumpBuffers = TRUE;
            pUsrCtxt->m_usrSettings.isp_ctrls.param_mask &= ~(1 << QCARCAM_CONTROL_DUMP_FRAME);
        }

        QueueEvent(&sNewJob);

        AIS_TRACE_MESSAGE_F(AISTraceGroupEngine, "IFE %d:%d idx %d_%d ProcessRawFrameDone",
                    pUsrCtxt->m_streams[streamIdx].resources.ifeStream.ifeCore,
                    pUsrCtxt->m_streams[streamIdx].resources.ifeStream.ifeOutput,
                    pFrameDone->frameInfo.idx,
                    pFrameDone->frameInfo.seq_no[0]);

        //pUsrCtxt->DecRefCnt(); <== do not decrement as we have it Queued for PPROC!
    }
}

/**
 * ProcessPProcJob
 *
 * @brief Process PPROC Job
 *
 * @param pMsg
 */
void AisEngine::ProcessPProcJob(AisEventMsgType* pMsg)
{
    CameraResult result;
    AisEventPProcJobType* pPPJob = &pMsg->payload.pprocJob;
    AisUsrCtxt* pUsrCtxt = pPPJob->pUsrCtxt;

    //pUsrCtxt->IncRefCnt() <== pUsrCtxt incremented by caller on queueing so no need to increment again

    pUsrCtxt->Lock();
    if (pUsrCtxt->m_state != AIS_USR_STATE_STREAMING)
    {
        AIS_LOG(ENGINE, HIGH, "job %p aborted!", pUsrCtxt);
        pUsrCtxt->Unlock();
        pUsrCtxt->DecRefCnt();
        return;
    }
    pUsrCtxt->m_ppjobInProgress++;
    pUsrCtxt->Unlock();


    pPPJob->pProcChain = &pUsrCtxt->m_pProcChainDef->pProcChain[pPPJob->currentHop];
    result = pUsrCtxt->m_pPProc[pPPJob->currentHop]->ProcessEvent(pUsrCtxt, pMsg);

    if (CAMERA_ENOREADYBUFFER == result &&
        AIS_PPROC_STEP_OPTIONAL_INPUT_BUFFERS == pPPJob->pProcChain->stepType)
    {
        AIS_LOG(ENGINE, HIGH, "job 0x%llx skip optional hop %d", pPPJob->currentHop);
        pPPJob->status = CAMERA_SUCCESS;

        ProcessPProcJobDone(pMsg);
    }
    else if (CAMERA_SUCCESS != result)
    {
        //bail out on failure in one of the stages
        //ENEEDMORE is abort on purpose and need not print as error
        AIS_LOG_ON_ERR(ENGINE, (CAMERA_ENEEDMORE != result), "job 0x%llx failed on hop %d (%d)",
                pPPJob->jobId, pPPJob->currentHop, result);

        pUsrCtxt->Lock();
        pUsrCtxt->m_ppjobInProgress--;
        pUsrCtxt->Unlock();

        pUsrCtxt->DecRefCnt();
    }

}

/**
 * ProcessPProcJobDone
 *
 * @brief Process PPROC Job done and moves it to next hop
 *
 * @param pMsg
 */
void AisEngine::ProcessPProcJobDone(AisEventMsgType* pMsg)
{
    AisEventPProcJobType* pPPJob = &pMsg->payload.pprocJob;
    AisUsrCtxt* pUsrCtxt = pPPJob->pUsrCtxt;
    boolean isFinished = FALSE;

    if (CAMERA_SUCCESS != pPPJob->status)
    {
        //bail out on failure in one of the stages
        AIS_LOG_ON_ERR(ENGINE, (CAMERA_ENEEDMORE != pPPJob->status), "job %p failed on hop %d (%d)",
                pUsrCtxt, pPPJob->currentHop, pPPJob->status);
        isFinished = TRUE;
    }
    else
    {
        pPPJob->currentHop++;

        if (pPPJob->currentHop == pUsrCtxt->m_pProcChainDef->nProc)
        {
            AIS_LOG(ENGINE, MED, "job %p finished hop %d", pUsrCtxt, pPPJob->currentHop);
            isFinished = TRUE;
        }
        else
        {
            AIS_LOG(ENGINE, MED, "job %p next hop %d", pUsrCtxt, pPPJob->currentHop);
            pMsg->eventId = AIS_EVENT_PPROC_JOB;
            QueueEvent(pMsg);
        }
    }

    AIS_TRACE_MESSAGE_F(AISTraceGroupEngine, "IFE %d:%d idx %d_%u ProcessPProcJobDone",
                        pUsrCtxt->m_streams[pPPJob->streamIdx].resources.ifeStream.ifeCore,
                        pUsrCtxt->m_streams[pPPJob->streamIdx].resources.ifeStream.ifeOutput,
                        pPPJob->frameInfo.idx,
                        pPPJob->frameInfo.seq_no[0]);

    pUsrCtxt->Lock();
    pUsrCtxt->m_ppjobInProgress--;
    pUsrCtxt->Unlock();


    if(isFinished)
    {
        pUsrCtxt->DecRefCnt();
    }
}

/**
 * ProcessPProcJobFail
 *
 * @brief Process PPROC Job failure
 *
 * @param pMsg
 */
void AisEngine::ProcessPProcJobFail(AisEventMsgType* pMsg)
{
    AisUsrCtxt* pUsrCtxt = pMsg->payload.pprocJob.pUsrCtxt;
    uint32_t deviceId = pUsrCtxt->m_streams[pMsg->payload.pprocJob.streamIdx].inputCfg.devId;
    AisEventErrorHandlerType errorEvent;
    CameraResult result;

    errorEvent.eventPayload.uint_payload = pMsg->payload.pprocJob.error;
    errorEvent.bRecoveryEnabled = FALSE;
    result = HandleAISErrorEvent(pUsrCtxt, &errorEvent);

    UpdateDiagErrInfo(NULL, AIS_EVENT_PPROC_JOB_FAIL, pUsrCtxt);

    if (errorEvent.bRecoveryEnabled && result == CAMERA_SUCCESS)
    {
            CameraSleep(m_engineSettings->recoveryRestartDelay);
            CompleteRecovery(pUsrCtxt, NULL);
    }

    //@todo: find out why not needed... pUsrCtxt->DecRefCnt();
}

/**
 * ProcessCsidFatalError
 *
 * @brief Process CSID fatal error event.
 *
 * @param pMsg
 */
void AisEngine::ProcessCsidFatalError(AisEventMsgType* pMsg)
{
    CameraResult rc;
    AisResMgrMatch sMatch = {};
    AisUsrCtxtList matchList;

    sMatch.matchType = (AisResMgrMatchType) m_engineSettings->customMatchFunctions[CAMERA_CONFIG_EVENT_CSID_FATAL_ERROR].type;
    sMatch.dataType = AIS_RESMGR_MATCH_DATA_IFE;
    sMatch.device = pMsg->payload.ifeError.ifeCore;

    rc = m_ResourceManager->MatchUserList(&sMatch, &matchList);
    if (CAMERA_SUCCESS == rc && !matchList.empty())
    {
        AisEventErrorHandlerType errorEvent;
        errorEvent.eventPayload.uint_payload = QCARCAM_FATAL_ERROR;
        errorEvent.bRecoveryEnabled = FALSE;
        TraverseUsrCtxt(&matchList, HandleAISErrorEvent, &errorEvent);

        UpdateDiagErrInfo((void*)&pMsg->payload.ifeError, AIS_EVENT_CSID_FATAL_ERROR, NULL);

        if (errorEvent.bRecoveryEnabled)
        {
            uint32_t severity = m_engineSettings->customMatchFunctions[CAMERA_CONFIG_EVENT_CSID_FATAL_ERROR].severity;
            AisUsrCtxt* pUsrCtxt = matchList.front();
            rc = AisInputConfigurer::GetInstance()->SensorRecovery(pUsrCtxt->m_streams[0].inputCfg.devId, severity);
            if (CAMERA_SUCCESS == rc)
            {
                CameraSleep(m_engineSettings->recoveryRestartDelay);
                TraverseUsrCtxt(&matchList, CompleteRecovery, NULL);
            }
            else
            {
                TraverseUsrCtxt(&matchList, FailedRecovery, NULL);
            }
        }

        //decrement ref count
        TraverseUsrCtxt(&matchList, AisUsrCtxtDecRefCnt, NULL);
    }
}

/**
 * ProcessInputStatusEvent
 *
 * @brief Process Input status event
 *
 * @param pMsg
 */
void AisEngine::ProcessInputStatusEvent(AisEventMsgType* pMsg)
{
    CameraResult rc;
    AisUsrCtxtList matchList;

    uint32 device = pMsg->payload.inputStatus.devId;
    uint32 path = pMsg->payload.inputStatus.srcId;

    rc = MatchUserList(device, path, &matchList);
    if (CAMERA_SUCCESS == rc && !matchList.empty())
    {
        qcarcam_event_payload_t input_event;
        input_event.uint_payload = pMsg->payload.inputStatus.status;

        TraverseUsrCtxt(&matchList, AisUsrCtxtSendInputStatus, &input_event);
        UpdateDiagErrInfo((void*)&pMsg->payload.inputStatus, AIS_EVENT_INPUT_STATUS, NULL);

        //decrement ref count
        TraverseUsrCtxt(&matchList, AisUsrCtxtDecRefCnt, NULL);
    }
}

/**
 * ProcessInputFatalError
 *
 * @brief Process Input fatal error
 *
 * @param pMsg
 */
void AisEngine::ProcessInputFatalError(AisEventMsgType* pMsg)
{
    CameraResult rc;
    AisEventErrorHandlerType errorEvent;
    AisResMgrMatch sMatch = {};
    AisUsrCtxtList matchList;

    sMatch.matchType = (AisResMgrMatchType) m_engineSettings->customMatchFunctions[CAMERA_CONFIG_EVENT_INPUT_FATAL_ERROR].type;
    sMatch.dataType = AIS_RESMGR_MATCH_DATA_INPUT;
    sMatch.device = pMsg->payload.inputStatus.devId;

    rc = m_ResourceManager->MatchUserList(&sMatch, &matchList);
    if (CAMERA_SUCCESS == rc && !matchList.empty())
    {
        errorEvent.eventPayload.uint_payload = QCARCAM_FATAL_ERROR;
        errorEvent.bRecoveryEnabled = FALSE;
        TraverseUsrCtxt(&matchList, HandleAISErrorEvent, &errorEvent);
        if (errorEvent.bRecoveryEnabled)
        {
            uint32_t severity = m_engineSettings->customMatchFunctions[CAMERA_CONFIG_EVENT_INPUT_FATAL_ERROR].severity;
            rc = AisInputConfigurer::GetInstance()->SensorRecovery(pMsg->payload.inputStatus.devId, severity);
            CameraSleep(m_engineSettings->recoveryRestartDelay);
            if (CAMERA_SUCCESS == rc)
            {
                TraverseUsrCtxt(&matchList, CompleteRecovery, NULL);
            }
            else
            {
                TraverseUsrCtxt(&matchList, FailedRecovery, NULL);
            }
        }

        //decrement ref count
        TraverseUsrCtxt(&matchList, AisUsrCtxtDecRefCnt, NULL);
    }
}

/**
 * ProcessInputFrameFreeze
 *
 * @brief Process input frame freeze
 *
 * @param pMsg
 */
void AisEngine::ProcessInputFrameFreeze(AisEventMsgType* pMsg)
{
    CameraResult rc;
    AisResMgrMatch sMatch = {};
    AisUsrCtxtList matchList;
    AisEventInputFrameFreezeType* pFrameFreeze = &pMsg->payload.inputFrameFreeze;

    //If stream handle provided, use that or else match clients based on input
    if (pFrameFreeze->hStream)
    {
        AisUsrCtxt* pUsrCtxt = GetUsrCtxt((void*)pFrameFreeze->hStream);
        if (!pUsrCtxt)
        {
            AIS_LOG(ENGINE, ERROR, "Invalid user context %p passed for FRAME FREEZE event", pUsrCtxt);
            rc = CAMERA_EFAILED;
        }
        else
        {
            rc = CAMERA_SUCCESS;
            matchList.push_back(pUsrCtxt);
        }
    }
    else
    {
        sMatch.matchType = AIS_RESMGR_MATCH_INPUT_SRC;
        sMatch.dataType = AIS_RESMGR_MATCH_DATA_INPUT;
        sMatch.device = pFrameFreeze->devId;
        sMatch.path = pFrameFreeze->srcId;

        rc = m_ResourceManager->MatchUserList(&sMatch, &matchList);
    }

    if (CAMERA_SUCCESS == rc && !matchList.empty())
    {
        qcarcam_event_payload_t qcarcamEvent;
        qcarcamEvent.frame_freeze = pFrameFreeze->status;

        TraverseUsrCtxt(&matchList, AisUsrCtxtSendFrameFreeze, &qcarcamEvent);
        UpdateDiagErrInfo((void*)&pMsg->payload.inputFrameFreeze, AIS_EVENT_INPUT_FRAME_FREEZE, NULL);

        //decrement ref count
        TraverseUsrCtxt(&matchList, AisUsrCtxtDecRefCnt, NULL);
    }
}

/**
 * ProcessSOF
 *
 * @brief Process SOF (start of frame) event
 *
 * @param pMsg
 */
void AisEngine::ProcessSOF(AisEventMsgType* pMsg)
{
    CameraResult rc;
    AisEventSofType* pSofInfo = &pMsg->payload.sofInfo;
    AisResMgrMatch sMatch = {};
    AisUsrCtxtList matchList;
    AisUsrCtxt* pUsrCtxt = NULL;

    sMatch.matchType = AIS_RESMGR_MATCH_IFE_PATH;
    sMatch.dataType = AIS_RESMGR_MATCH_DATA_IFE;
    sMatch.device = pSofInfo->ifeCore;
    sMatch.path = pSofInfo->ifeInput;

    rc = m_ResourceManager->MatchUserList(&sMatch, &matchList);
    if (CAMERA_SUCCESS == rc && !matchList.empty())
    {
        pUsrCtxt = matchList.front();

        pUsrCtxt->Lock();
        if (AIS_USR_STATE_STREAMING == pUsrCtxt->m_state ||
            AIS_USR_STATE_RECOVERY == pUsrCtxt->m_state)
        {
            pUsrCtxt->m_numSof++;

            //send sof event if needed
            if (pUsrCtxt->m_eventCbFcn && (pUsrCtxt->m_eventMask & QCARCAM_EVENT_FRAME_SOF))
            {
                qcarcam_event_payload_t sofEvent = {};
                sofEvent.sof_timestamp.qtimestamp = pSofInfo->hwTimestamp;
                sofEvent.sof_timestamp.timestamp_system = pSofInfo->timestamp;
                AIS_LOG(ENGINE, HIGH, "Send SOF event for context %p", pUsrCtxt);
                pUsrCtxt->m_eventCbFcn(pUsrCtxt->m_qcarcamHndl, QCARCAM_EVENT_FRAME_SOF, (qcarcam_event_payload_t*)&sofEvent);
            }

            //query field information for interlaced case
            if (pUsrCtxt->m_streams[0].inputCfg.inputModeInfo.interlaced == INTERLACED_QUERY_FIELD_TYPE)
            {
                AisFieldInfoType fieldInfo = {};

                rc = m_Configurers[AIS_CONFIGURER_INPUT]->GetParam(pUsrCtxt,
                                      AIS_INPUT_CTRL_FIELD_TYPE, &fieldInfo);
                if (rc == CAMERA_SUCCESS)
                {
                    fieldInfo.sofTimestamp = pSofInfo->timestamp;
                    fieldInfo.frameId = pSofInfo->frameId;
                    pUsrCtxt->AddSofFieldInfo(&fieldInfo);
                }
                else
                {
                    AIS_LOG(ENGINE, ERROR, "InputConfigurer GetParam failed: %d", rc);
                }
            }
        }
        pUsrCtxt->Unlock();

        //decrement ref count
        TraverseUsrCtxt(&matchList, AisUsrCtxtDecRefCnt, NULL);
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Can't find user context for SOF ife %d input %d",
                pSofInfo->ifeCore, pSofInfo->ifeInput);
    }
}

/**
 * ProcessIfeOutputError
 *
 * @brief Process IFE Output error event
 *
 * @param pMsg
 */
void AisEngine::ProcessIfeOutputError(AisEventMsgType* pMsg)
{
    CameraResult rc;
    AisResMgrMatch sMatch = {};
    AisUsrCtxtList matchList;

    sMatch.matchType = (AisResMgrMatchType)m_engineSettings->customMatchFunctions[CAMERA_CONFIG_EVENT_IFE_OUTPUT_ERROR].type;
    sMatch.dataType = AIS_RESMGR_MATCH_DATA_IFE;
    sMatch.device = pMsg->payload.ifeError.ifeCore;

    rc = m_ResourceManager->MatchUserList(&sMatch, &matchList);
    if (CAMERA_SUCCESS == rc && !matchList.empty())
    {
        AisEventErrorHandlerType errorEvent;
        errorEvent.eventPayload.uint_payload = QCARCAM_IFE_OVERFLOW_ERROR;
        errorEvent.bRecoveryEnabled = FALSE;
        TraverseUsrCtxt(&matchList, HandleAISErrorEvent, &errorEvent);
        UpdateDiagErrInfo((void*)&pMsg->payload.ifeError, AIS_EVENT_IFE_OUTPUT_ERROR, matchList.front());

        if (errorEvent.bRecoveryEnabled)
        {
            uint32_t severity = m_engineSettings->customMatchFunctions[CAMERA_CONFIG_EVENT_IFE_OUTPUT_ERROR].severity;
            AisUsrCtxt* pUsrCtxt = matchList.front();
            rc = AisInputConfigurer::GetInstance()->SensorRecovery(pUsrCtxt->m_streams[0].inputCfg.devId, severity);
            if (CAMERA_SUCCESS == rc)
            {
                CameraSleep(m_engineSettings->recoveryRestartDelay);
                TraverseUsrCtxt(&matchList, CompleteRecovery, NULL);
            }
            else
            {
                TraverseUsrCtxt(&matchList, FailedRecovery, NULL);
            }
        }

        //decrement ref count
        TraverseUsrCtxt(&matchList, AisUsrCtxtDecRefCnt, NULL);
    }
}

/**
 * ProcessApplyParam
 *
 * @brief Process apply parameter event
 *
 * @param pMsg
 */
void AisEngine::ProcessApplyParam(AisEventMsgType* pMsg)
{
    AIS_LOG(ENGINE, DBG, "Queue apply param %d", pMsg->payload.applyParam.param);
            AisUsrCtxt* pUsrCtxt = pMsg->payload.applyParam.pUsrCtxt;

            if (pUsrCtxt->m_state != AIS_USR_STATE_STREAMING)
            {
                AIS_LOG(ENGINE, DBG, "%p apply Exp aborted!", pUsrCtxt);
            }
            else if (USR_CTXT_CHECK_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_HDR_EXPOSURE)
                && pUsrCtxt->m_usrSettings.hdr_exposure.exposure_mode_type == QCARCAM_EXPOSURE_MANUAL)
            {
                AIS_LOG(ENGINE, DBG, "%p maunal expo set from API, not apply HDR Exp from ISP!", pUsrCtxt);
            }
            else if (USR_CTXT_CHECK_MASK(pUsrCtxt->m_usrSettings.bitmask, QCARCAM_PARAM_EXPOSURE)
                && pUsrCtxt->m_usrSettings.exposure_params.exposure_mode_type == QCARCAM_EXPOSURE_MANUAL)
            {
                AIS_LOG(ENGINE, DBG, "%p maunal expo set from API, not apply Exp from ISP!", pUsrCtxt);
            }
            else if (QCARCAM_PARAM_HDR_EXPOSURE == pMsg->payload.applyParam.param)
            {
                pUsrCtxt->m_usrSettings.hdr_exposure = pMsg->payload.applyParam.val.hdr_exposure_config;
                AisInputConfigurer::GetInstance()->SetParam(pUsrCtxt, AIS_INPUT_CTRL_HDR_EXPOSURE_CONFIG, NULL);
                AisUsrctxtNotifyEventListener(pUsrCtxt->m_inputId, QCARCAM_PARAM_HDR_EXPOSURE);
                AIS_LOG(ENGINE, DBG, "Applied HDR Exp");
            }
            else if (QCARCAM_PARAM_EXPOSURE == pMsg->payload.applyParam.param)
            {
                pUsrCtxt->m_usrSettings.exposure_params = pMsg->payload.applyParam.val.exposure_config;
                AisInputConfigurer::GetInstance()->SetParam(pUsrCtxt, AIS_INPUT_CTRL_EXPOSURE_CONFIG, NULL);
                AisUsrctxtNotifyEventListener(pUsrCtxt->m_inputId, QCARCAM_PARAM_EXPOSURE);
                AIS_LOG(ENGINE, DBG, "Applied Exp");
            }

            pUsrCtxt->DecRefCnt();
}

/**
 * ProcessRecoverySuccessEvent
 *
 * @brief Process Recovery Success Event
 *
 * @note The caller must lock the UsrCtxt mutex before calling the function, and unlock it afterwards
 */
void AisUsrCtxt::ProcessRecoverySuccessEvent(void)
{
    m_state = AIS_USR_STATE_STREAMING;
    m_numRecoveryAttempts = 0;
    m_numSuccessfulRecoveries++;

    AIS_LOG(ENGINE, WARN, "Recovery success for context %p (%d %d)",
            this, m_numSuccessfulRecoveries, m_totalRecoveryAttempts);

    StopRecoveryTimeoutTimer();
    if (m_eventCbFcn && (m_eventMask & QCARCAM_EVENT_RECOVERY_SUCCESS))
    {
        AIS_LOG(ENGINE, HIGH, "Send recovery success event for context %p", this);
        m_eventCbFcn(m_qcarcamHndl, QCARCAM_EVENT_RECOVERY_SUCCESS, NULL);
    }

    return;
}

/**
 * ProcessRecoveryFailEvent
 *
 * @brief Process Recovery Fail Event
 *
 * @note The caller must lock the UsrCtxt mutex before calling the function, and unlock it afterwards
 *
 * @return CameraResult If the recovery timeout timer has been stopped successfully
 */
CameraResult AisUsrCtxt::ProcessRecoveryFailEvent(void)
{
    CameraResult rc;

    AIS_LOG(ENGINE, WARN, "Recovery Failed for context %p", this);

    m_state = AIS_USR_STATE_ERROR;
    rc = StopRecoveryTimeoutTimer();

    if (m_usrSettings.recovery && m_eventCbFcn && (m_eventMask & QCARCAM_EVENT_ERROR_ABORTED))
    {
        AIS_LOG(ENGINE, HIGH, "Sent recovery failed event for context %p", this);
        m_eventCbFcn(m_qcarcamHndl, QCARCAM_EVENT_ERROR_ABORTED, NULL);
    }

    return rc;
}

/**
 * ProcessDeferInputDetect
 *
 * @brief Process deferred input detection. Detects available inputs and publishes them.
 *
 * @param pMsg
 */
void AisEngine::ProcessDeferInputDetect(AisEventMsgType* pMsg)
{
    CameraResult rc;

    AIS_LOG(ENGINE, HIGH, "Process defer detect");

    CameraLogEvent(CAMERA_ENGINE_EVENT_INPUT_PROBE, 0, 0);
    (void)AisInputConfigurer::GetInstance()->DetectAll();
    CameraLogEvent(CAMERA_ENGINE_EVENT_INPUT_PROBE_DONE, 0, 0);

    Lock();

    m_state = AIS_ENGINE_STATE_SUSPEND_PENDING;

    if (m_clientCount != 0)
    {
        m_state = AIS_ENGINE_STATE_READY;
        CameraSetSignal(m_delaySuspend);
    }

    Unlock();

    ProcessDelayedSuspend(pMsg);
}

/**
 * ProcessDelayedSuspend
 *
 * @brief Process delayed suspend event. Waits for timeout before going into suspend.
 *        If it is signaled in that time, it aborts going into suspend.
 *
 * @param pMsg
 */
void AisEngine::ProcessDelayedSuspend(AisEventMsgType* pMsg)
{
    CameraResult rc;

    AIS_LOG(ENGINE, HIGH, "Process delayed suspend");

    rc = CameraWaitOnSignal(m_delaySuspend, AIS_ENGINE_DELAYED_SUSPEND_TIMEOUT);

    Lock();

    AIS_LOG(ENGINE, HIGH, "rc %d, m_state %d", rc, m_state);

    /*Power suspend if timeout and still in suspend pending state*/
    if (rc == CAMERA_EEXPIRED && AIS_ENGINE_STATE_SUSPEND_PENDING == m_state)
    {
        if (m_PowerManagerPolicyType == CAMERA_PM_POLICY_LPM_EVENT_FOR_INPUTS)
        {
            PowerSuspend(CAMERA_POWER_SOC_CLK_OFF, FALSE);
        }
        else if (m_PowerManagerPolicyType == CAMERA_PM_POLICY_NO_LPM)
        {
            PowerSuspend(CAMERA_POWER_DOWN, m_isPowerGranular);
        }
        else
        {
            AIS_LOG(ENGINE, HIGH, "No need to suspend m_PowerManagerPolicyType = %d", m_PowerManagerPolicyType);
        }
    }

    Unlock();
}

/**
 * ProcessVendorEvent
 *
 * @brief Process vendor event.
 *        Finds all users matching same input device and source and broadcasts the event.
 *
 * @param pMsg
 */
void AisEngine::ProcessVendorEvent(AisEventMsgType* pMsg)
{
    CameraResult rc;
    AisUsrCtxtList matchList;

    uint32 device = pMsg->payload.inputStatus.devId;
    uint32 path = pMsg->payload.inputStatus.srcId;

    rc = MatchUserList(device, path, &matchList);
    if (CAMERA_SUCCESS == rc && !matchList.empty())
    {
        qcarcam_event_payload_t input_event;
        input_event.vendor_data = pMsg->payload.vendorEvent.vendor_param;

        TraverseUsrCtxt(&matchList, AisUsrCtxtSendVendorEvent, &input_event);

        //decrement ref count
        TraverseUsrCtxt(&matchList, AisUsrCtxtDecRefCnt, NULL);
    }
}

/**
 * ProcessEventUserNotification
 *
 * @brief Process event user notification
 *        Sends notification of event param change to user
 *
 * @param pMsg
 */
void AisEngine::ProcessEventUserNotification(AisEventMsgType* pMsg)
{
    AisUsrCtxt* pUsrCtxt = pMsg->payload.notifyParam.pUsrCtxt;
    qcarcam_event_payload_t qcarcamEvent;

    qcarcamEvent.uint_payload = pMsg->payload.notifyParam.paramVal;

    AisUsrCtxtSendNotificationCb(pUsrCtxt, &qcarcamEvent);

    //decrement ref count of client
    pUsrCtxt->DecRefCnt();
}

/**
* MatchUserList
* @brief MatchUserList special for input event
* @param [in]pMatch    Matching criteria
* @param [in/out]pList    List to be filled with matching user contexts based on criteria
* return CameraResult
*/
CameraResult AisEngine::MatchUserList(uint32 device, uint32 path, AisUsrCtxtList* pList)
{
    uint32 idx;
    AisUsrCtxt* pUsrCtxt;
    uint32 streamIdx;

    for (idx = 0; idx < AIS_MAX_USR_CONTEXTS; idx++)
    {
        pUsrCtxt = AcquireUsrCtxt(idx);
        if (pUsrCtxt)
        {
            CameraResult rc = CAMERA_EFAILED;

            for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams; streamIdx++)
            {
                if ((pUsrCtxt->m_streams[streamIdx].inputCfg.devId == device) &&
                    (pUsrCtxt->m_streams[streamIdx].inputCfg.srcId == path))
                {
                    pList->push_back(pUsrCtxt);
                    rc = CAMERA_SUCCESS;
                    break;
                }
            }

            if (rc == CAMERA_EFAILED)
            {
                RelinquishUsrCtxt(idx);
            }
        }
    }

    return CAMERA_SUCCESS;
}

/**
 * ProcessEvent
 *
 * @brief Dequeues event from event Q and processes it
 *
 * @param pMsg
 *
 * @return int
 */
int AisEngine::ProcessEvent(AisEventMsgType* pMsg)
{
    CameraResult rc;

    rc = CameraQueueDequeue(m_eventQ[AIS_ENGINE_QUEUE_EVENTS], pMsg);
    if (CAMERA_SUCCESS != rc)
    {
        if (CAMERA_ENOMORE != rc)
        {
            AIS_LOG(ENGINE, ERROR, "Failed to dequeue event (%d)", rc);
        }
        return 0;
    }

    switch (pMsg->eventId)
    {
    case AIS_EVENT_RAW_FRAME_DONE:
    {
        ProcessRawFrameDone(pMsg);
        break;
    }
    case AIS_EVENT_PPROC_JOB:
    {
        ProcessPProcJob(pMsg);
        break;
    }
    case AIS_EVENT_PPROC_JOB_DONE:
    {
        ProcessPProcJobDone(pMsg);
        break;
    }
    case AIS_EVENT_PPROC_JOB_FAIL:
    {
        ProcessPProcJobFail(pMsg);
        break;
    }
    case AIS_EVENT_CSID_FATAL_ERROR:
    {
        ProcessCsidFatalError(pMsg);
        break;
    }
    case AIS_EVENT_INPUT_STATUS:
    {
        ProcessInputStatusEvent(pMsg);
        break;
    }
    case AIS_EVENT_INPUT_FATAL_ERROR:
    {
        ProcessInputFatalError(pMsg);
        break;
    }
    case AIS_EVENT_INPUT_FRAME_FREEZE:
    {
        ProcessInputFrameFreeze(pMsg);
        break;
    }
    case AIS_EVENT_SOF:
    {
        ProcessSOF(pMsg);
        break;
    }
    case AIS_EVENT_IFE_OUTPUT_ERROR:
    {
        ProcessIfeOutputError(pMsg);
        break;
    }
    case AIS_EVENT_APPLY_PARAM:
    {
        ProcessApplyParam(pMsg);
        break;
    }
    case AIS_EVENT_DEFER_INPUT_DETECT:
    {
        ProcessDeferInputDetect(pMsg);

        /* This means all the Initializations of ais_server done, can initialize device Diagnostic stats */
        m_DiagManager->InitializeDiagstats();
        break;
    }
    case AIS_EVENT_DELAYED_SUSPEND:
    {
        ProcessDelayedSuspend(pMsg);
        break;
    }
    case AIS_EVENT_VENDOR:
    {
        ProcessVendorEvent(pMsg);
        break;
    }
    case AIS_EVENT_USER_NOTIFICATION:
    {
        ProcessEventUserNotification(pMsg);
        break;
    }
    default:
        break;
    }

    return 1;
}

/**
 * EventHandler
 *
 * @brief Engine event handler thread to process engine events
 *
 * @param arg: AisEngine
 *
 * @return int
 */
int AisEngine::EventHandler(void *arg)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisEngine* pEngine = (AisEngine*)arg;

    if (pEngine)
    {
        AisEventMsgType event = {};

        while (!pEngine->m_eventHandlerIsExit)
        {
            AIS_LOG(ENGINE, LOW, "Awake; ready to work");

            (void)pEngine->ProcessEvent(&event);

            AIS_LOG(ENGINE, LOW, "Going to wait");
            CameraWaitOnSignal(pEngine->m_eventHandlerSignal, CAM_SIGNAL_WAIT_NO_TIMEOUT);
        }

        AIS_LOG(ENGINE, HIGH, "Terminating (%d) ...", rc);
    }

    // Don't use MM_Thread_Exit.
    // - Depending on scheduler's thread exection,
    //   MM_Thread_Exit may just do "return 1",
    //   which is not what we want.

    return 0;
}

///@brief AisEngine singleton
AisEngine* AisEngine::m_pEngineInstance = nullptr;

/**
* AisEngine::CreateInstance
*
* @brief
*     Create singleton instance for AisEngine
*/
AisEngine* AisEngine::CreateInstance()
{
    if(m_pEngineInstance == nullptr)
    {
        m_pEngineInstance = new AisEngine();
    }

    return m_pEngineInstance;
}

/**
* AisEngine::GetInstance
*
* @brief
*     Gets the singleton instance for AisEngine
*/
AisEngine* AisEngine::GetInstance()
{
    return m_pEngineInstance;
}

/**
* AisEngine::DestroyInstance
*
* @brief
*     Destroy the singleton instance of the AisEngine class
*
* @return
*     void
*/
void AisEngine::DestroyInstance()
{
    if(m_pEngineInstance != nullptr)
    {
        delete m_pEngineInstance;
        m_pEngineInstance = nullptr;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// ais_initialize
///
/// @brief Initialize AIS engine
///
/// @param initialization parameters
///
/// @return CAMERA_SUCCESS if successful;
///////////////////////////////////////////////////////////////////////////////
AIS_API CameraResult ais_initialize(qcarcam_init_t* p_init_params)
{
    CameraResult result;
    AisEngine* pEngine = AisEngine::GetInstance();

    if (pEngine)
    {
        AIS_LOG(ENGINE, ERROR, "AisEngine already initialized");
        return CAMERA_EBADSTATE;
    }

    pEngine = AisEngine::CreateInstance();
    if (!pEngine)
    {
        return CAMERA_ENOMEMORY;
    }

    result = pEngine->ais_initialize(p_init_params);

    if (CAMERA_SUCCESS != result)
    {
        AisEngine::DestroyInstance();
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_uninitialize
///
/// @brief Uninitialize AIS engine
///
/// @return CAMERA_SUCCESS if successful;
///////////////////////////////////////////////////////////////////////////////
AIS_API CameraResult ais_uninitialize(void)
{
    CameraResult result = CAMERA_EBADSTATE;
    AisEngine* pEngine = AisEngine::GetInstance();

    if (pEngine)
    {
        result = pEngine->ais_uninitialize();
    }

    return result;
}


///////////////////////////////////////////////////////////////////////////////
/// ais_query_inputs
///
/// @brief Queries available inputs. To get the number of available inputs to query, call with p_inputs set to NULL.
///
/// @param p_inputs   Pointer to array inputs. If NULL, then ret_size returns number of available inputs to query
/// @param size       Number of elements in array
/// @param ret_size   If p_inputs is set, number of elements in array that were filled
///                   If p_inputs is NULL, number of available inputs to query
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
AIS_API CameraResult ais_query_inputs(qcarcam_input_t* p_inputs, unsigned int size, unsigned int* ret_size)
{
    CameraResult result = CAMERA_EBADSTATE;
    AisEngine* pEngine = AisEngine::GetInstance();

    if (pEngine)
    {
        result = pEngine->ais_query_inputs(p_inputs, size, ret_size);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_query_inputs_v2
///
/// @brief Queries available inputs. To get the number of available inputs to query, call with p_inputs set to NULL.
///
/// @param p_inputs   Pointer to array inputs. If NULL, then ret_size returns number of available inputs to query
/// @param size       Number of elements in array
/// @param ret_size   If p_inputs is set, number of elements in array that were filled
///                   If p_inputs is NULL, number of available inputs to query
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
AIS_API CameraResult ais_query_inputs_v2(qcarcam_input_v2_t* p_inputs, unsigned int size, unsigned int* ret_size)
{
    CameraResult result = CAMERA_EBADSTATE;
    AisEngine* pEngine = AisEngine::GetInstance();

    if (pEngine)
    {
        result = pEngine->ais_query_inputs_v2(p_inputs, size, ret_size);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_query_diagnsotics
///
/// @brief Queries the system diagnostic Info.
///
/// @param p_diag_info   Pointer to diagnostic info.
/// @param diag_size     Size of user allocated memory
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
AIS_API CameraResult ais_query_diagnostics(void *p_ais_diag_info, unsigned int diag_size)
{
    CameraResult result = CAMERA_EBADSTATE;
    AisEngine* pEngine = AisEngine::GetInstance();
    if (pEngine)
    {
        result = pEngine->ais_query_diagnostics(p_ais_diag_info, diag_size);
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_open
///
/// @brief Opens handle to input
///
/// @param desc   Unique identifier of input to be opened
///
/// @return NOT NULL if successful; NULL on failure
///////////////////////////////////////////////////////////////////////////////
AIS_API qcarcam_hndl_t ais_open(qcarcam_input_desc_t desc)
{
    AisEngine* pEngine = AisEngine::GetInstance();

    if (pEngine)
    {
        return pEngine->ais_open(desc);
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_close
///
/// @brief Closes handle to input
///
/// @param hndl   Handle of input that was opened
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
AIS_API CameraResult ais_close(qcarcam_hndl_t hndl)
{
    CameraResult result = CAMERA_EBADSTATE;
    AisEngine* pEngine = AisEngine::GetInstance();

    if (pEngine)
    {
        result = pEngine->ais_close(hndl);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_g_param
///
/// @brief Get parameter value
///
/// @param hndl     Handle of input
/// @param param    Parameter to get
/// @param p_value  Pointer to structure of value that will be retrieved
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
AIS_API CameraResult ais_g_param(qcarcam_hndl_t hndl, qcarcam_param_t param, qcarcam_param_value_t* p_value)
{
    CameraResult result = CAMERA_EBADSTATE;
    AisEngine* pEngine = AisEngine::GetInstance();

    if (pEngine)
    {
        result = pEngine->ais_g_param(hndl, param, p_value);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_s_param
///
/// @brief Set parameter
///
/// @param hndl     Handle of input
/// @param param    Parameter to set
/// @param p_value  Pointer to structure of value that will be set
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
AIS_API CameraResult ais_s_param(qcarcam_hndl_t hndl, qcarcam_param_t param, const qcarcam_param_value_t* p_value)
{
    CameraResult result = CAMERA_EBADSTATE;
    AisEngine* pEngine = AisEngine::GetInstance();

    if (pEngine)
    {
        result = pEngine->ais_s_param(hndl, param, p_value);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_s_buffers
///
/// @brief Set buffers
///
/// @param hndl       Handle of input
/// @param p_buffers  Pointer to set buffers structure
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
AIS_API CameraResult ais_s_buffers(qcarcam_hndl_t hndl, qcarcam_buffers_t* p_buffers)
{
    CameraResult result = CAMERA_EBADSTATE;
    AisEngine* pEngine = AisEngine::GetInstance();

    if (pEngine)
    {
        result = pEngine->ais_s_buffers(hndl, p_buffers);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_s_buffers_v2
///
/// @brief Set buffers for specifc buffer list
///
/// @param hndl       Handle of input
/// @param p_bufferlist  Pointer to bufferlist
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
AIS_API CameraResult ais_s_buffers_v2(qcarcam_hndl_t hndl, const qcarcam_bufferlist_t* p_bufferlist)
{
    CameraResult result = CAMERA_EBADSTATE;
    AisEngine* pEngine = AisEngine::GetInstance();

    if (pEngine)
    {
        result = pEngine->ais_s_buffers_v2(hndl, p_bufferlist);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_start
///
/// @brief Start input
///
/// @param hndl       Handle of input
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
AIS_API CameraResult ais_start(qcarcam_hndl_t hndl)
{
    CameraResult result = CAMERA_EBADSTATE;
    AisEngine* pEngine = AisEngine::GetInstance();

    if (pEngine)
    {
        result = pEngine->ais_start(hndl);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_stop
///
/// @brief Stop input that was started
///
/// @param hndl       Handle of input
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
AIS_API CameraResult ais_stop(qcarcam_hndl_t hndl)
{
    CameraResult result = CAMERA_EBADSTATE;
    AisEngine* pEngine = AisEngine::GetInstance();

    if (pEngine)
    {
        result = pEngine->ais_stop(hndl);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_pause
///
/// @brief Pause input that was started. Does not relinquish resource
///
/// @param hndl       Handle of input
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
AIS_API CameraResult ais_pause(qcarcam_hndl_t hndl)
{
    CameraResult result = CAMERA_EBADSTATE;
    AisEngine* pEngine = AisEngine::GetInstance();

    if (pEngine)
    {
        result = pEngine->ais_pause(hndl);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_resume
///
/// @brief Resumes input that was paused
///
/// @param hndl       Handle of input
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
AIS_API CameraResult ais_resume(qcarcam_hndl_t hndl)
{
    CameraResult result = CAMERA_EBADSTATE;
    AisEngine* pEngine = AisEngine::GetInstance();

    if (pEngine)
    {
        result = pEngine->ais_resume(hndl);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_get_frame
///
/// @brief Get available frame
///
/// @param hndl          Handle of input
/// @param p_frame_info  Pointer to frame information that will be filled
/// @param timeout       Max wait time in ns for frame to be available before timeout
/// @param flags         Flags
///
/// @return CAMERA_SUCCESS if successful; CAMERA_EEXPIRED if timeout
///////////////////////////////////////////////////////////////////////////////
AIS_API CameraResult ais_get_frame(qcarcam_hndl_t hndl, qcarcam_frame_info_t* p_frame_info,
        unsigned long long int timeout, unsigned int flags)
{
    CameraResult result = CAMERA_EBADSTATE;
    AisEngine* pEngine = AisEngine::GetInstance();
    if (pEngine)
    {
        result = pEngine->ais_get_frame(hndl, p_frame_info, timeout, flags);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_get_frame_v2
///
/// @brief Get available frame
///
/// @param hndl          Handle of input
/// @param p_frame_info  Pointer to frame information that will be filled
/// @param timeout       Max wait time in ns for frame to be available before timeout
/// @param flags         Flags
///
/// @return CAMERA_SUCCESS if successful; CAMERA_EEXPIRED if timeout
///////////////////////////////////////////////////////////////////////////////
AIS_API CameraResult ais_get_frame_v2(qcarcam_hndl_t hndl, qcarcam_frame_info_v2_t* p_frame_info,
        unsigned long long int timeout, unsigned int flags)
{
    CameraResult result = CAMERA_EBADSTATE;
    AisEngine* pEngine = AisEngine::GetInstance();
    if (pEngine)
    {
        result = pEngine->ais_get_frame_v2(hndl, p_frame_info, timeout, flags);
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_release_frame
///
/// @brief Re-enqueue frame buffers
///
/// @param hndl       Handle of input
/// @param idx        Index into the qcarcam_buffers_t buffers table to reenqueue
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
AIS_API CameraResult ais_release_frame(qcarcam_hndl_t hndl, unsigned int idx)
{
    CameraResult result = CAMERA_EBADSTATE;
    AisEngine* pEngine = AisEngine::GetInstance();

    if (pEngine)
    {
        result = pEngine->ais_release_frame(hndl, idx);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// ais_release_frame_v2
///
/// @brief Re-enqueue frame buffers
///
/// @param hndl       Handle of input
/// @param id         bufferlist id
/// @param idx        Index into the qcarcam_buffers_t buffers table to reenqueue
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
AIS_API CameraResult ais_release_frame_v2(qcarcam_hndl_t hndl, unsigned int id, unsigned int idx)
{
    CameraResult result = CAMERA_EBADSTATE;
    AisEngine* pEngine = AisEngine::GetInstance();

    if (pEngine)
    {
        result = pEngine->ais_release_frame_v2(hndl, id, idx);
    }

    return result;
}

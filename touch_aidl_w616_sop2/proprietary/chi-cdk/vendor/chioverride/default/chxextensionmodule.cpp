////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxextensionmodule.cpp
/// @brief Extension Module implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#if defined (_WIN32)    // This file for Win32 build only
#include <malloc.h>
#else
#include <stdlib.h>
#endif // WIN32

#include <algorithm>
#include <vector>
#include <string>

#if !defined(__QNXNTO__) && !defined(CAMERA_UNITTEST)
#include <utils/Errors.h>
#endif

#include "chi.h"
#include "chioverride.h"
#include "camxcdktypes.h"
#include "chxusecaseutils.h"

#include "chxextensionmodule.h"
#include "chxsensorselectmode.h"
#include "chxsession.h"
#include "chxusecase.h"
#include "chxmulticamcontroller.h"

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

extern CHICONTEXTOPS       g_chiContextOps;
extern CHIBUFFERMANAGEROPS g_chiBufferManagerOps;

/// @brief Primarily an example for how to use the VOID* in ExtendOpen and ModifySettings.  Must match data structure
///        being passed into the function
struct ChiOverrideToken
{
    UINT32    id;
    UINT32    size;
};

/// @brief Primarily an example for how to use the VOID* in ExtendOpen and ModifySettings  Must match data structure
///        being passed into the function
struct ChiOverrideExtendOpen
{
    UINT32              numTokens;
    ChiOverrideToken*   pTokens;
};

/// @brief Primarily an example for how to use the VOID* in ExtendClose and ModifySettings  Must match data structure
///        being passed into the function
struct ChiOverrideExtendClose
{
    UINT32              numTokens;
    ChiOverrideToken*   pTokens;
};

/// @brief Primarily an example for how to use the VOID* in ExtendOpen and ModifySettings  Must match data structure
///        being passed into the function
struct ChiOverrideModifySetting
{
    ChiOverrideToken    token;
    VOID*               pData;
};

/// @brief Logical camera configuration which will be configured by customer themselves according to their requirements.
static LogicalCameraConfiguration logicalCameraConfiguration[] =
{
    /*cameraId cameraType              exposeFlag phyDevCnt  sensorId, role, transition low, high, smoothZoom, alwaysOn  primarySensorID*/
    {0,        LogicalCameraType_Default, TRUE,      1,    {{0, CameraRoleTypeDefault,   0.0, 0.0,   FALSE,    TRUE}},     0   }, ///< Wide camera
    {1,        LogicalCameraType_Default, TRUE,      1,    {{2, CameraRoleTypeDefault,   0.0, 0.0,   FALSE,    TRUE}},     2   }, ///< Front camera
    {2,        LogicalCameraType_Default, TRUE,      1,    {{1, CameraRoleTypeDefault,   0.0, 0.0,   FALSE,    TRUE}},     1   }, ///< Tele camera
    {3,        LogicalCameraType_SAT,     TRUE,      2,    {{0, CameraRoleTypeWide,      1.0, 2.0,   TRUE,     FALSE},
                                                            {1, CameraRoleTypeTele,      2.0, 8.0,   TRUE,     FALSE}},    0   }, ///< SAT
    {4,        LogicalCameraType_RTB,     TRUE,      2,    {{0, CameraRoleTypeWide,      1.0, 2.0,   FALSE,    TRUE},
                                                            {1, CameraRoleTypeTele,      2.0, 8.0,   FALSE,    TRUE}},     1   }, ///< RTB
    {5,        LogicalCameraType_Default, TRUE,      1,    {{3, CameraRoleTypeDefault,   0.0, 0.0,   FALSE,    TRUE}},     3   }, ///< Extra Camera

    {6,        LogicalCameraType_SAT,     TRUE,      3,    {{2, CameraRoleTypeUltraWide, 1.0, 1.5,   FALSE,    FALSE},
                                                            {0, CameraRoleTypeWide,      1.5, 2.0,   TRUE,     FALSE },
                                                            {1, CameraRoleTypeTele,      2.0, 8.0,   TRUE,     FALSE}},    2   },   ///< U+W+T FOV transition
    {7,        LogicalCameraType_DualApp, FALSE,     2,    {{0, CameraRoleTypeWide,      1.0, 2.0,   TRUE,     FALSE},
                                                            {1, CameraRoleTypeTele,      2.0, 8.0,   TRUE,     FALSE}},    0   }, ///< Dual application
    {8,        LogicalCameraType_Default, TRUE,      1,    {{4, CameraRoleTypeDefault,   0.0, 0.0,   FALSE,    TRUE}},     4   }, /// Extra camera2 / Wide camera
    {99,       LogicalCameraType_Default, FALSE,     1,    {{3, CameraRoleTypeDefault,   0.0, 0.0,   FALSE,    TRUE}},     3   }, ///< Secure camera

    /*-------------------------------------------------------------------------------------------------*/
    /*                                   Add cameras for triple camera module                          */
    /*-------------------------------------------------------------------------------------------------*/
    {9,        LogicalCameraType_Default, TRUE,      1,    {{5, CameraRoleTypeDefault,   0.0, 0.0,   FALSE,    TRUE}},     5   }, ///< Tele camera
    {10,       LogicalCameraType_Default, TRUE,      1,    {{6, CameraRoleTypeDefault,   0.0, 0.0,   FALSE,    TRUE}},     6   }, ///< Ultra wide  camera
    {11,       LogicalCameraType_SAT,     TRUE,      3,    {{6, CameraRoleTypeUltraWide, 1.0, 1.5,   FALSE,    FALSE},
                                                            {4, CameraRoleTypeWide,      1.5, 2.0,   TRUE,     TRUE},
                                                            {5, CameraRoleTypeTele,      2.0, 8.0,   TRUE,     FALSE}},    6   },   ///< U+W+T FOV transition
    {12,       LogicalCameraType_RTB,     TRUE,      2,    {{4, CameraRoleTypeWide,      1.0, 2.0,   FALSE,    TRUE},
                                                            {5, CameraRoleTypeTele,      2.0, 8.0,   FALSE,    TRUE}},     5   },   ///< W+T RTB
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::GetInstance
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ExtensionModule* ExtensionModule::GetInstance()
{
    static ExtensionModule s_extensionModuleSingleton;

    return &s_extensionModuleSingleton;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::ExtensionModule
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ExtensionModule::ExtensionModule()
    : m_hCHIContext(NULL)
    , m_numPhysicalCameras(0)
    , m_numLogicalCameras(0)
    , m_previousPowerHint(PERF_LOCK_COUNT)
    , m_numMetadataResults(0)
{
    memset(&m_chiFenceOps, 0, sizeof(m_chiFenceOps));
    memset(m_logicalCameraInfo, 0, sizeof(m_logicalCameraInfo));
    memset(m_IFEResourceCost, 0, sizeof(m_IFEResourceCost));
    m_pUsecaseSelector = UsecaseSelector::Create(this);
    m_pUsecaseFactory  = UsecaseFactory::Create(this);

    // To make vendor tags avaliable in ConsolidateCameraInfo
    MultiCamControllerManager::GetInstance();

    m_longExposureFrame      = static_cast<UINT32>(InvalidFrameNumber);
    m_isFlushLocked          = FALSE;
    m_singleISPResourceCost  = 50;  // Single ISP resource cost
    m_resourcesUsed          = 0;   // initialize resource cost

#if defined(__AGL__)
#if defined(USELIB64)
    const CHAR*     pChiDriver = "/usr/lib64/hw/camera.qcom.so";
#else // USELIB64
    const CHAR*     pChiDriver = "/usr/lib/hw/camera.qcom.so";
#endif
#elif defined(__QNXNTO__)
    const CHAR*     pChiDriver = "/lib/camera/camera.qcom.so";
#elif defined(ANDROID)
#if defined(_LP64)
    const CHAR*     pChiDriver = "/vendor/lib64/hw/camera.qcom.so";
#else // _LP64
    const CHAR*     pChiDriver = "/vendor/lib/hw/camera.qcom.so";
#endif // _LP64
#else
#error Unsupported target
#endif // __AGL__
    const CHAR* pPerfModule = "libqti-perfd-client.so";

    m_perfLibHandle = ChxUtils::LibMap(pPerfModule);

    if (NULL == m_perfLibHandle)
    {
        CHX_LOG_ERROR("Failed to load perf lib");
    }

    ChxUtils::Memset(&m_pPerfLockManager[0], 0, sizeof(m_pPerfLockManager));

    m_pPCRLock                  = Mutex::Create();
    m_pDestroyLock              = Mutex::Create();
    m_pRecoveryLock             = Mutex::Create();
    m_pTriggerRecoveryLock      = Mutex::Create();
    m_pResourcesUsedLock        = Mutex::Create();
    m_pTriggerRecoveryCondition = Condition::Create();
    m_pRecoveryCondition        = Condition::Create();
    m_TeardownInProgress        = FALSE;
    m_RecoveryInProgress        = FALSE;
    m_terminateRecoveryThread   = FALSE;
    m_pConfigSettings           = static_cast<UINT32*>(CHX_CALLOC(sizeof(UINT32) * MaxConfigSettings));

    // Create recovery thread and wait on being signaled
    m_pRecoveryThread.pPrivateData = this;

    ChxUtils::ThreadCreate(ExtensionModule::RecoveryThread,
                            &m_pRecoveryThread,
                            &m_pRecoveryThread.hThreadHandle);

    m_numOfLogicalCameraConfiguration = CHX_ARRAY_SIZE(logicalCameraConfiguration);

    if (0 != m_numOfLogicalCameraConfiguration)
    {
        m_pLogicalCameraConfigurationInfo = static_cast<LogicalCameraConfiguration*>
            (CHX_CALLOC(sizeof(LogicalCameraConfiguration)* m_numOfLogicalCameraConfiguration));
        if (NULL != m_pLogicalCameraConfigurationInfo)
        {
            ChxUtils::Memcpy(m_pLogicalCameraConfigurationInfo, logicalCameraConfiguration,
                sizeof(LogicalCameraConfiguration)*m_numOfLogicalCameraConfiguration);
        }
        else
        {
            CHX_LOG_ERROR("Allocation failed for m_pLogicalCameraConfigurationInfo: Fatal");
        }
    }

    OSLIBRARYHANDLE handle  = ChxUtils::LibMap(pChiDriver);

    if (NULL != handle)
    {
        CHX_LOG("CHI Opened driver library");

        PCHIENTRY funcPChiEntry = reinterpret_cast<PCHIENTRY>(ChxUtils::LibGetAddr(handle, "ChiEntry"));

        if (NULL != funcPChiEntry)
        {
            CHX_LOG("CHI obtained ChiEntry point function");

            funcPChiEntry(&g_chiContextOps);

            m_hCHIContext = g_chiContextOps.pOpenContext();

            CHX_LOG("CHI context functions - CreateSession  %p", g_chiContextOps.pCreateSession);

            if (NULL != m_hCHIContext)
            {
                ChiVendorTagsOps vendorTagOps = { 0 };

                g_chiContextOps.pTagOps(&vendorTagOps);
                g_chiContextOps.pGetFenceOps(&m_chiFenceOps);

                // Since this is a one time operation do it at boot up time
                if (NULL != vendorTagOps.pQueryVendorTagLocation)
                {
                    m_pvtVendorTags[SensorBpsModeIndex].pSectionName    = "com.qti.sensorbps";
                    m_pvtVendorTags[SensorBpsModeIndex].pTagName        = "mode_index";
                    m_pvtVendorTags[SensorBpsGain].pSectionName         = "com.qti.sensorbps";
                    m_pvtVendorTags[SensorBpsGain].pTagName             = "gain";
                    m_pvtVendorTags[DebugDataTag].pSectionName          = "org.quic.camera.debugdata";
                    m_pvtVendorTags[DebugDataTag].pTagName              = "DebugDataAll";
                    m_pvtVendorTags[SensorModeIndex].pSectionName       = "org.codeaurora.qcamera3.sensor_meta_data";
                    m_pvtVendorTags[SensorModeIndex].pTagName           = "sensor_mode_index";
                    m_pvtVendorTags[CropRegions].pSectionName           = "com.qti.cropregions";
                    m_pvtVendorTags[CropRegions].pTagName               = "crop_regions";
                    m_pvtVendorTags[TuningMode].pSectionName            = "org.quic.camera2.tuning.mode";
                    m_pvtVendorTags[TuningMode].pTagName                = "TuningMode";
                    m_pvtVendorTags[RefCropSize].pSectionName           = "org.quic.camera2.ref.cropsize";
                    m_pvtVendorTags[RefCropSize].pTagName               = "RefCropSize";
                    m_pvtVendorTags[MultiCamera].pSectionName           = "com.qti.chi.multicamerainfo";
                    m_pvtVendorTags[MultiCamera].pTagName               = "MultiCameraIdRole";
                    m_pvtVendorTags[IsFlashRequiredTag].pSectionName    = "com.qti.stats_control";
                    m_pvtVendorTags[IsFlashRequiredTag].pTagName        = "is_flash_snapshot";
                    m_pvtVendorTags[Feature1Mode].pSectionName          = "org.quic.camera2.tuning.feature";
                    m_pvtVendorTags[Feature1Mode].pTagName              = "Feature1Mode";
                    m_pvtVendorTags[Feature2Mode].pSectionName          = "org.quic.camera2.tuning.feature";
                    m_pvtVendorTags[Feature2Mode].pTagName              = "Feature2Mode";
                    m_pvtVendorTags[VideoHDR10Mode].pSectionName        = "org.quic.camera2.streamconfigs";
                    m_pvtVendorTags[VideoHDR10Mode].pTagName            = "HDRVideoMode";
                    m_pvtVendorTags[StatsSkipMode].pSectionName         = "com.qti.chi.statsSkip";
                    m_pvtVendorTags[StatsSkipMode].pTagName             = "skipFrame";
                    m_pvtVendorTags[BurstFps].pSectionName              = "org.quic.camera.BurstFPS";
                    m_pvtVendorTags[BurstFps].pTagName                  = "burstfps";
                    m_pvtVendorTags[CustomNoiseReduction].pSectionName  = "org.quic.camera.CustomNoiseReduction";
                    m_pvtVendorTags[CustomNoiseReduction].pTagName      = "CustomNoiseReduction";
                    m_pvtVendorTags[FastShutterMode].pSectionName       = "org.quic.camera.SensorModeFS";
                    m_pvtVendorTags[FastShutterMode].pTagName           = "SensorModeFS";
                    m_pvtVendorTags[IFEMaxWidth].pSectionName           = "org.quic.camera.MaxSingleISPCapabilities";
                    m_pvtVendorTags[IFEMaxWidth].pTagName               = "MaxSingleISPCapabilities";
                    m_pvtVendorTags[IHDRControl].pSectionName           = "com.qti.ihdr_control";
                    m_pvtVendorTags[IHDRControl].pTagName               = "state_ihdr_snapshot";
                    m_pvtVendorTags[HistNodeLTCRatioIndex].pSectionName = "org.quic.camera2.statsconfigs";
                    m_pvtVendorTags[HistNodeLTCRatioIndex].pTagName     = "HistNodeLTCRatioIndex";

                    for (UINT32 id = 0; id < MaxNumImageSensors; id++)
                    {
                        m_pStreamConfig[id] = NULL;
                    }
                    // Go thru each tag and get the tag id from the driver
                    for (UINT index = 0; index < NumVendorTags; index++)
                    {
                        CDKResult result = vendorTagOps.pQueryVendorTagLocation(m_pvtVendorTags[index].pSectionName,
                                                                                m_pvtVendorTags[index].pTagName,
                                                                                &m_pvtVendorTags[index].tagId);
                        if (CDKResultSuccess == result)
                        {
                            CHX_LOG("Vendor Tag %d: Section %s, TagName %s, TagId: 0x%x",
                                index,
                                m_pvtVendorTags[index].pSectionName,
                                m_pvtVendorTags[index].pTagName,
                                m_pvtVendorTags[index].tagId);
                        }
                        else
                        {
                            CHX_LOG("Failed to find TagId: %d, Section %s, TagName %s",
                                index,
                                m_pvtVendorTags[index].pSectionName,
                                m_pvtVendorTags[index].pTagName);
                        }
                    }
                }

                g_chiContextOps.pGetBufferManagerOps(&g_chiBufferManagerOps);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::~ExtensionModule
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ExtensionModule::~ExtensionModule()
{
    m_pUsecaseSelector->Destroy();
    m_pUsecaseSelector = NULL;
    if (NULL != m_pTriggerRecoveryLock)
    {
       m_pTriggerRecoveryLock->Lock();
       m_terminateRecoveryThread = TRUE;
       m_pTriggerRecoveryCondition->Signal();
       m_pTriggerRecoveryLock->Unlock();
    }

    for (UINT i = 0; i < MaxNumImageSensors; i++)
    {
        if (NULL != m_pPerfLockManager[i])
        {
            m_pPerfLockManager[i]->Destroy();
            m_pPerfLockManager[i] = NULL;
        }
    }

    m_chiFenceOps = {NULL};

    g_chiContextOps.pCloseContext(m_hCHIContext);
    m_hCHIContext = NULL;

    for (UINT i = 0; i < m_numLogicalCameras; i++)
    {
        // if physical camera numbers is bigger than 1, it should be multi camera.
        // and the static meta is allocated in chiextension module. Free it when
        // exit
        if (1 < m_logicalCameraInfo[i].numPhysicalCameras)
        {
            if (NULL != m_logicalCameraInfo[i].m_cameraInfo.static_camera_characteristics)
            {
                free_camera_metadata(
                    const_cast<camera_metadata_t*>(m_logicalCameraInfo[i].m_cameraInfo.static_camera_characteristics));
                m_logicalCameraInfo[i].m_cameraInfo.static_camera_characteristics = NULL;
            }
        }

        // if it is not physical camera, the sensor mode is from physical camera which is not allocated.
        if (1 == m_logicalCameraInfo[i].numPhysicalCameras)
        {
            if (NULL != m_logicalCameraInfo[i].pSensorModeInfo)
            {
                CHX_FREE(m_logicalCameraInfo[i].pSensorModeInfo);
                m_logicalCameraInfo[i].pSensorModeInfo = NULL;
            }
        }

        if (NULL != m_logicalCameraInfo[i].ppDeviceInfo)
        {
            // if logical camera is invalid, the resource for logical camera is not allocated.
            for (UINT j = 0; j < m_logicalCameraInfo[i].numPhysicalCameras; j++)
            {
                if (NULL != m_logicalCameraInfo[i].ppDeviceInfo[j])
                {
                    CHX_FREE(m_logicalCameraInfo[i].ppDeviceInfo[j]);
                    m_logicalCameraInfo[i].ppDeviceInfo[j] = NULL;
                }
            }

            CHX_FREE(m_logicalCameraInfo[i].ppDeviceInfo);
            m_logicalCameraInfo[i].ppDeviceInfo = NULL;
        }
    }

    if (NULL != m_pConfigSettings)
    {
        CHX_FREE(m_pConfigSettings);
        m_pConfigSettings = NULL;
    }

    for (UINT32 id = 0; id < MaxNumImageSensors; id++)
    {
        if (NULL != m_pStreamConfig[id])
        {
            if (NULL != m_pStreamConfig[id]->streams)
            {
                CHX_FREE(m_pStreamConfig[id]->streams);
                m_pStreamConfig[id]->streams = NULL;
            }
            CHX_FREE(m_pStreamConfig[id]);
            m_pStreamConfig[id] = NULL;
        }
    }

    if (NULL != m_pPCRLock)
    {

        m_pPCRLock->Destroy();
        m_pPCRLock = NULL;
    }

    if (NULL != m_pRecoveryLock)
    {
        m_pRecoveryLock->Destroy();
        m_pRecoveryLock = NULL;
    }

    if (NULL != m_pResourcesUsedLock)
    {
        m_pResourcesUsedLock->Destroy();
        m_pResourcesUsedLock = NULL;
    }

    if (NULL != m_pRecoveryCondition)
    {
        m_pRecoveryCondition->Destroy();
        m_pRecoveryCondition = NULL;
    }

    if (NULL != m_pTriggerRecoveryLock)
    {
        m_pTriggerRecoveryLock->Destroy();
        m_pTriggerRecoveryLock = NULL;
    }

    if (NULL != m_pTriggerRecoveryCondition)
    {
        m_pTriggerRecoveryCondition->Destroy();
        m_pTriggerRecoveryCondition = NULL;
    }

    if (NULL != m_pDestroyLock)
    {
        m_pDestroyLock->Destroy();
        m_pDestroyLock = NULL;
    }

    if (m_pDefaultSettings != NULL)
    {
#if defined (_LINUX)
        free_camera_metadata(m_pDefaultSettings);
#endif
    }

    if (NULL != m_pLogicalCameraConfigurationInfo)
    {
        CHX_FREE(m_pLogicalCameraConfigurationInfo);
        m_pLogicalCameraConfigurationInfo = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::SetHALOps
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::SetHALOps(
    const chi_hal_ops_t* pHalOps, uint32_t logicalCameraId)
{
    m_pHALOps[logicalCameraId] = pHalOps;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::SetCHIContextOps
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::SetCHIContextOps()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::RemapCameraId
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 ExtensionModule::RemapCameraId(
    UINT32              frameworkCameraId,
    CameraIdRemapMode   mode)
{
    (VOID)mode;

    UINT32 cameraId = GetCameraIdIndex(frameworkCameraId);

    if (INVALID_INDEX != cameraId)
    {
        switch (mode)
        {
            case IdRemapCamera:
                frameworkCameraId = cameraId;
                break;

            case IdReverseMapCamera:
                frameworkCameraId = m_cameraReverseMap[cameraId];
                break;

            case IdRemapTorch:
                UINT32 pPrimaryCameraIndex = GetPrimaryCameraIndex(&m_logicalCameraInfo[cameraId]);
                frameworkCameraId          = m_logicalCameraInfo[cameraId].ppDeviceInfo[pPrimaryCameraIndex]->cameraId;
                break;
        }
    }

    return frameworkCameraId;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::ExtendOpen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::ExtendOpen(
    uint32_t  logicalCameraId,
    VOID*     pPriv)
{
    // The ExtensionModule has been initialized, and if there is any work needed to be done during HAL open, this is where
    // to add that code

    // sample code...this uses the input to create a data structure used to hold configuration settings
    ChiOverrideExtendOpen* pExtend = static_cast<ChiOverrideExtendOpen*>(pPriv);
    UINT32     openCameraCost      = m_singleISPResourceCost;
    CDKResult  result              = CDKResultSuccess;

    if ((NULL == m_pConfigSettings) || (pExtend->numTokens > MaxConfigSettings))
    {
        CHX_LOG_ERROR("ExtendOpen failed! m_pConfigSettings=%p, numTokens=%d MaxConfigSettings=%d",
                      m_pConfigSettings, pExtend->numTokens, MaxConfigSettings);
        result = CDKResultEInvalidArg;
    }

    if (m_logicalCameraInfo[logicalCameraId].numPhysicalCameras > 1)
    {
        openCameraCost = m_singleISPResourceCost * 2; // dual/multi
    }

    if ((CDKResultSuccess == result) && (openCameraCost + CostOfAnyCurrentlyOpenLogicalCameras()) > m_totalResourceBudget)
    {
        CHX_LOG_ERROR("ExtendOpen failed! HW resource insufficient! openCameraCost=%d"
                    "CostOfAnyCurrentlyOpenLogicalCameras =%d, m_totalResourceBudget = %d",
                    openCameraCost, CostOfAnyCurrentlyOpenLogicalCameras(), m_totalResourceBudget);
        result = CamxResultETooManyUsers; // over capacity
    }
    if (CDKResultSuccess == result)
    {
        if (NULL == m_pPerfLockManager[logicalCameraId])
        {
            m_pPerfLockManager[logicalCameraId] = PerfLockManager::Create();
            if (NULL == m_pPerfLockManager[logicalCameraId])
            {
                CHX_LOG_ERROR("Failed to create perflock manager %d", logicalCameraId);
            }
        }

        if (NULL != m_pPerfLockManager[logicalCameraId])
        {
            m_pPerfLockManager[logicalCameraId]->AcquirePerfLock(PERF_LOCK_OPEN_CAMERA, 1000);
        }

        MappingConfigSettings(pExtend->numTokens, static_cast<VOID*>(pExtend->pTokens));

        // Update camera open and close status in static settings
        for (UINT8 index = 0; index < m_logicalCameraInfo[logicalCameraId].numPhysicalCameras; index++)
        {
            UINT32 cameraId = m_logicalCameraInfo[logicalCameraId].ppDeviceInfo[index]->cameraId;
            *m_pOverrideCameraOpen     |=  (1 << cameraId);
            *m_pOverrideCameraClose    &= ~(1 << cameraId);
        }
        CHX_LOG_INFO("Open Logical cameraId: %d  numPhysicalCameras: %d"
                    "CameraOpen Mask = 0x%x CameraClose Mask 0x%x",
                    logicalCameraId, m_logicalCameraInfo[logicalCameraId].numPhysicalCameras,
                    *m_pOverrideCameraOpen, *m_pOverrideCameraClose);

        m_TeardownInProgress = FALSE;
        m_RecoveryInProgress = FALSE;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::ExtendClose
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::ExtendClose(
    uint32_t  logicalCameraId,
    VOID*     pPriv)
{
    // The ExtensionModule has been initialized, and if there is any work needed to be done during HAL close, this is where
    // to add that code

    m_pRecoveryLock->Lock();
    if (TRUE == m_RecoveryInProgress)
    {
        CHX_LOG_INFO("Wait for recovery to finish, before starting teardown");
        m_pRecoveryCondition->Wait(m_pRecoveryLock->GetNativeHandle());
    }
    m_TeardownInProgress = TRUE;
    m_pRecoveryLock->Unlock();

    if (NULL != m_pPerfLockManager[logicalCameraId])
    {
        m_pPerfLockManager[logicalCameraId]->AcquirePerfLock(PERF_LOCK_CLOSE_CAMERA, 1000);
    }


    if (NULL != m_pStreamConfig[logicalCameraId])
    {
        if (NULL != m_pStreamConfig[logicalCameraId]->streams)
        {
            CHX_FREE(m_pStreamConfig[logicalCameraId]->streams);
            m_pStreamConfig[logicalCameraId]->streams = NULL;
        }
        CHX_FREE(m_pStreamConfig[logicalCameraId]);
        m_pStreamConfig[logicalCameraId] = NULL;
    }

    // sample code...this uses the input to create a data structure used to hold configuration settings
    ChiOverrideExtendClose* pExtend = static_cast<ChiOverrideExtendClose*>(pPriv);

    if ((NULL == m_pConfigSettings) || (pExtend->numTokens > MaxConfigSettings))
    {
        CHX_LOG_ERROR("ExtendClose failed! m_pConfigSettings=%p, numTokens=%d MaxConfigSettings=%d",
                      m_pConfigSettings, pExtend->numTokens, MaxConfigSettings);
        return;
    }

    MappingConfigSettings(pExtend->numTokens, static_cast<VOID*>(pExtend->pTokens));

    for (UINT8 index = 0; index < m_logicalCameraInfo[logicalCameraId].numPhysicalCameras; index++)
    {
        UINT32 cameraId = m_logicalCameraInfo[logicalCameraId].ppDeviceInfo[index]->cameraId;
        *m_pOverrideCameraOpen    &= ~(1 << cameraId);
        *m_pOverrideCameraClose   |=  (1 << cameraId);
    }
    CHX_LOG_INFO("Close Logical cameraId: %d  numPhysicalCameras: %d CameraOpen Mask = 0x%x  CameraClose Mask 0x%x ",
        logicalCameraId, m_logicalCameraInfo[logicalCameraId].numPhysicalCameras,
        *m_pOverrideCameraOpen, *m_pOverrideCameraClose);

    FreeLastKnownRequestSetting(logicalCameraId);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::SearchNumBatchedFrames
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::SearchNumBatchedFrames(
    uint32_t                        cameraId,
    camera3_stream_configuration_t* pStreamConfigs,
    UINT*                           pBatchSize,
    UINT*                           pFPSValue,
    UINT                            maxSessionFps)
{
    INT32 width  = 0;
    INT32 height = 0;

    // We will take the following steps -
    //  1) We will search SupportedHFRVideoSizes, for the matching Video/Preview stream.
    //     Note: For the use case of multiple output streams, application must select one unique size from this metadata
    //           to use (e.g., preview and recording streams must have the same size). Otherwise, the high speed capture
    //           session creation will fail
    //  2) If a single entry is found in SupportedHFRVideoSizes, we choose the batchsize from that entry

    for (UINT streamIndex = 0; streamIndex < pStreamConfigs->num_streams; streamIndex++)
    {
        if (CAMERA3_STREAM_OUTPUT == pStreamConfigs->streams[streamIndex]->stream_type)
        {
            width  = pStreamConfigs->streams[streamIndex]->width;
            height = pStreamConfigs->streams[streamIndex]->height;
            break;
        }
    }

    SIZE_T numDefaultHFRVideoSizes    = 0;
    /// @todo Implement metadata merging
    camera_metadata_entry_t entry   = { 0 };

    entry.tag = ANDROID_CONTROL_AVAILABLE_HIGH_SPEED_VIDEO_CONFIGURATIONS;

#if defined (_LINUX)
    find_camera_metadata_entry((camera_metadata_t*)(
        m_logicalCameraInfo[cameraId].m_cameraInfo.static_camera_characteristics), entry.tag, &entry);
#endif // _LINUX

    numDefaultHFRVideoSizes = entry.count;

    const HFRConfigurationParams* pHFRParams[MaxHFRConfigs] = { NULL };
    HFRConfigurationParams*       pDefaultHFRVideoSizes     = reinterpret_cast<HFRConfigurationParams*>(entry.data.i32);
    UINT                          numHFREntries             = 0;

    if ((0 != width) && (0 != height))
    {
        for (UINT i = 0; i < numDefaultHFRVideoSizes; i++)
        {
            if ((pDefaultHFRVideoSizes[i].width == width) && (pDefaultHFRVideoSizes[i].height == height))
            {
                // Check if maxSessionFps is non-zero, otherwise default it to HFR size max fps
                if ((maxSessionFps != 0) && (maxSessionFps != (UINT)pDefaultHFRVideoSizes[i].maxFPS))
                {
                    // Go to next entry
                    continue;
                }
                // Out of the pair of entries in the table, we would like to store the second entry
                pHFRParams[numHFREntries++] = &pDefaultHFRVideoSizes[i + 1];
                // Make sure that we don't hit the other entry in the pair, again
                i++;
            }
        }

        if (numHFREntries > 0)
        {
            *pBatchSize = pHFRParams[0]->batchSizeMax;
            *pFPSValue  = pHFRParams[0]->maxFPS;
            CHX_LOG("HFR entry batch size fps min max (%d %d %d)",
                    pHFRParams[0]->batchSizeMax,
                    pHFRParams[0]->minFPS,
                    pHFRParams[0]->maxFPS);
        }
        else
        {
            CHX_LOG_ERROR("Failed to find supported HFR entry");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::GetInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::GetInfo(
   CDKGetInfoCmd       infoCmd,
   void*               pInputParams,
   void*               pOutputParams)
{
    CDKResult result = CDKResultEInvalidArg;

    if ((CDKGetInfoMax > infoCmd) &&
        (NULL != pInputParams)     &&
        (NULL != pOutputParams))
    {
        switch (infoCmd)
        {
            case CDKGetInfoNumPhysicalCameras:
            {
                UINT32 cameraId    = (static_cast<CDKInfoCameraId*>(pInputParams))->cameraId;
                UINT32 cameraIndex = GetCameraIdIndex(cameraId);

                if (INVALID_INDEX != cameraIndex)
                {
                    (static_cast<CDKInfoNumCameras*>(pOutputParams))->numCameras =
                        m_logicalCameraInfo[cameraId].numPhysicalCameras;
                    result = CDKResultSuccess;
                }
                else
                {
                    CHX_LOG_ERROR("Invalid logical cameraId=%u", cameraId);
                }
            }
            break;

            case CDKGetInfoPhysicalCameraIds:
            {
                CDKInfoPhysicalCameraIds* pPhysIds = static_cast<CDKInfoPhysicalCameraIds*>(pOutputParams);
                if (NULL != pPhysIds->physicalCameraIds)
                {
                    UINT32 cameraId    = (static_cast<CDKInfoCameraId*>(pInputParams))->cameraId;
                    UINT32 cameraIndex = GetCameraIdIndex(cameraId);

                    if (INVALID_INDEX != cameraIndex)
                    {

                        for (UINT32 camIdx = 0; camIdx < m_logicalCameraInfo[cameraId].numPhysicalCameras; camIdx++)
                        {
                            pPhysIds->physicalCameraIds[camIdx] = m_logicalCameraInfo[cameraId].ppDeviceInfo[camIdx]->cameraId;
                        }
                        pPhysIds->numCameras = m_logicalCameraInfo[cameraId].numPhysicalCameras;
                        result               = CDKResultSuccess;
                    }
                    else
                    {
                        CHX_LOG_ERROR("Invalid logical cameraId=%u", cameraId);
                    }
                }
                else
                {
                    CHX_LOG_ERROR("pPhysIds->physicalCameraIds is NULL");
                }
            }
            break;

            default:
                break;
        }
    }
    else
    {
        CHX_LOG_ERROR("Invalid Params: infoCmd=%u inputParams=%p outputParams=%p", infoCmd, pInputParams, pOutputParams);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::GetCameraIdIndex
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 ExtensionModule::GetCameraIdIndex(
    UINT32 logicalCameraId) const
{
    UINT32 index = INVALID_INDEX;

    for (UINT32 i = 0; i < m_numLogicalCameras; i++)
    {
        if (m_cameraMap[i] == logicalCameraId)
        {
            index = i;
            break;
        }
    }

    CHX_LOG_INFO("AppId => LogicalId:%d  =>  %d",logicalCameraId, index);

    return index;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::GetPrimaryCameraIndex
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 ExtensionModule::GetPrimaryCameraIndex(const LogicalCameraInfo * pLogicalCameraInfo)
{
    UINT32 primaryCameraId     = pLogicalCameraInfo->primaryCameraId;
    UINT32 primaryCameraIndex  = INVALID_INDEX;

    for (UINT32 i = 0; i < pLogicalCameraInfo->numPhysicalCameras; i++)
    {
        if (primaryCameraId == pLogicalCameraInfo->ppDeviceInfo[i]->cameraId)
        {
            primaryCameraIndex = i;
            break;
        }
    }

    return primaryCameraIndex;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::GetNumCameras
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::GetNumCameras(
    UINT32* pNumFwCameras,
    UINT32* pNumLogicalCameras)
{

    if (0 == m_numLogicalCameras)
    {
        SortCameras();
        EnumerateCameras();
    }

#if defined(CAMX_ANDROID_API) && (CAMX_ANDROID_API >= 28) //Android-P or better
    /// Update conflicting devices and physical camera ids
    updateCameraIdsInCaps();
#endif

    *pNumLogicalCameras = m_numExposedLogicalCameras;
    *pNumFwCameras      = m_numExposedLogicalCameras;
    CHX_LOG("exposedNumOfCamera:%d, all:%d", m_numExposedLogicalCameras, m_numLogicalCameras);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::GetPhysicalCameraInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const LogicalCameraInfo* ExtensionModule::GetPhysicalCameraInfo(
    UINT32 physicalCameraId
    )const
{
    const LogicalCameraInfo *pLogicalCameraInfo = NULL;

    for (UINT32 i = 0 ; i < m_numOfLogicalCameraConfiguration; i++)
    {
        if ((LogicalCameraType_Default != m_logicalCameraInfo[i].logicalCameraType))
        {
            continue;
        }

        if ((NULL != m_logicalCameraInfo[i].ppDeviceInfo) &&
            (m_logicalCameraInfo[i].ppDeviceInfo[0]->cameraId == physicalCameraId))
        {
            pLogicalCameraInfo = &m_logicalCameraInfo[i];
            break;
        }
    }

    return pLogicalCameraInfo;
}

#if defined(CAMX_ANDROID_API) && (CAMX_ANDROID_API >= 28) //Android-P or better
VOID ExtensionModule::updateCameraIdsInCaps() {
    for (UINT i = 0; i < m_numLogicalCameras; i++)
    {
        LogicalCameraInfo *logicalcam       = &(m_logicalCameraInfo[i]);
        LogicalCameraType logicalCameraType = GetCameraType(m_logicalCameraInfo[i].cameraId);
        // Update conflicting devices and physical camera ids with remapped
        // camera id. Assuming that each physical camera only belongs to one
        // logical camera.

        if ((logicalcam->numPhysicalCameras > 1) && (LogicalCameraType_DualApp == logicalCameraType))
        {
            CHX_ASSERT(2 == logicalcam->numPhysicalCameras);
            CHX_ASSERT(NULL != logicalcam->ppDeviceInfo);
            CHX_ASSERT(NULL != logicalcam->ppDeviceInfo[0]);
            CHX_ASSERT(NULL != logicalcam->ppDeviceInfo[1]);

            UINT remappedLogicalId = m_cameraMap[logicalcam->cameraId];
            UINT physicalIds[2];
            UINT remappedPhysicalIds[2];
            physicalIds[0] = logicalcam->ppDeviceInfo[0]->cameraId;
            remappedPhysicalIds[0] = m_cameraReverseMap[physicalIds[0]];
            physicalIds[1] = logicalcam->ppDeviceInfo[1]->cameraId;
            remappedPhysicalIds[1] = m_cameraReverseMap[physicalIds[1]];
            std::string logicalId = std::to_string(remappedLogicalId);
            std::string physical1Id = std::to_string(remappedPhysicalIds[0]);
            std::string physical2Id = std::to_string(remappedPhysicalIds[1]);
            CHX_LOG_VERBOSE("numPhysicalCameras %d, physicalIds[0] %d physicalIds[1] %d",
                logicalcam->numPhysicalCameras, physicalIds[0], physicalIds[1]);

            camera_info_t& logicalInfo = logicalcam->m_cameraInfo;
            camera_info_t& physical1Info = m_logicalCameraInfo[physicalIds[0]].m_cameraInfo;
            camera_info_t& physical2Info = m_logicalCameraInfo[physicalIds[1]].m_cameraInfo;

            // Resource cost and conflicting devices for logical device
            logicalInfo.resource_cost = 100;
            logicalInfo.conflicting_devices = static_cast<char**>(CHX_CALLOC(2*sizeof(char*)));
            if (NULL != logicalInfo.conflicting_devices)
            {
                logicalInfo.conflicting_devices[0] = static_cast<char*>(CHX_CALLOC(physical1Id.size() + 1));
                physical1Id.copy(logicalInfo.conflicting_devices[0], physical1Id.size());
                logicalInfo.conflicting_devices[1] = static_cast<char*>(CHX_CALLOC(physical2Id.size() + 1));
                physical2Id.copy(logicalInfo.conflicting_devices[1], physical2Id.size());
                logicalInfo.conflicting_devices_length = 2;
            }

            // Conflicting device for first physical device
            physical1Info.conflicting_devices = static_cast<char**>(CHX_CALLOC(sizeof(char*)));
            if (NULL != physical1Info.conflicting_devices)
            {
                physical1Info.conflicting_devices[0] = static_cast<char*>(CHX_CALLOC(logicalId.size() + 1));
                logicalId.copy(physical1Info.conflicting_devices[0], logicalId.size());
                physical1Info.conflicting_devices_length = 1;
            }

            // Conflicting device for second physical device
            physical2Info.conflicting_devices = static_cast<char**>(CHX_CALLOC(sizeof(char*)));
            if (NULL != physical2Info.conflicting_devices)
            {
                physical2Info.conflicting_devices[0] = static_cast<char*>(CHX_CALLOC(logicalId.size() + 1));
                logicalId.copy(physical2Info.conflicting_devices[0], logicalId.size());
                physical2Info.conflicting_devices_length = 1;
            }

            CHIHANDLE    staticMetaDataHandle = const_cast<camera_metadata_t*>(
                m_logicalCameraInfo[i].m_cameraInfo.static_camera_characteristics);
            CHITAGSOPS   tagOps = { 0 };
            g_chiContextOps.pTagOps(&tagOps);

            // Update ANDROID_LOGICAL_MULTI_CAMERA_PHYSICAL_IDS
            char* physicalCamIds = static_cast<char*>(CHX_CALLOC(physical1Id.size()+1+physical2Id.size()+1));
            if (NULL != physicalCamIds)
            {
                physical1Id.copy(physicalCamIds, physical1Id.size());
                physical2Id.copy(physicalCamIds+physical1Id.size()+1, physical2Id.size());

                tagOps.pSetMetaData(staticMetaDataHandle, ANDROID_LOGICAL_MULTI_CAMERA_PHYSICAL_IDS, physicalCamIds, physical1Id.size() + 1 + physical2Id.size() + 1);
            }

            BYTE sensorSyncType = 0;
            tagOps.pSetMetaData(staticMetaDataHandle, ANDROID_LOGICAL_MULTI_CAMERA_SENSOR_SYNC_TYPE, &sensorSyncType, sizeof(BYTE));

            camera_metadata_t *static_metadata =
                    const_cast<camera_metadata_t*>(logicalInfo.static_camera_characteristics);

            // Update ANDROID_REQUEST_AVAILABLE_CAPABILITIES
            camera_metadata_entry_t entry = {0};
            if (android::OK != find_camera_metadata_entry(static_metadata,
                    ANDROID_REQUEST_AVAILABLE_CAPABILITIES, &entry)) {
                CHX_LOG_ERROR("Failed to find ANDROID_REQUEST_AVAILABLE_CAPABILITIES entry");
                return;
            }

            uint8_t* capKeys = entry.data.u8;
            std::vector<uint8_t> newCapKeys(capKeys, capKeys+entry.count);
            if(newCapKeys.empty())
            {
                CHX_LOG_ERROR("Failed to find ANDROID_REQUEST_AVAILABLE_CAPABILITIES entry");
                break;
            }
            else
            {
                newCapKeys.push_back(ANDROID_REQUEST_AVAILABLE_CAPABILITIES_LOGICAL_MULTI_CAMERA);
                android::status_t res = find_camera_metadata_entry(static_metadata, ANDROID_REQUEST_AVAILABLE_CAPABILITIES, &entry);
                if (res == android::NAME_NOT_FOUND)
                {
                    res = add_camera_metadata_entry(static_metadata, ANDROID_REQUEST_AVAILABLE_CAPABILITIES, (const void *)newCapKeys.data(), newCapKeys.size());
                }
                else if (res == android::OK)
                {
                    res = update_camera_metadata_entry(static_metadata, entry.index, (const void *)newCapKeys.data(), newCapKeys.size(), NULL);
                }
            }
        }
    }
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::BuildLogicalCameraInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::BuildLogicalCameraInfo(
        UINT32* sensorIdMap,
        BOOL    isMultiCameraEnabled,
        BOOL    isForExposed)
{
    CDKResult          result              = CDKResultSuccess;
    CHITAGSOPS         tagOps              = { 0 };
    UINT32             tag;
    LogicalCameraType  logicalCameraType   = LogicalCameraType_Default;
    UINT32             numOfPhysicalCamera = g_chiContextOps.pGetNumCameras(m_hCHIContext);

    LogicalCameraConfiguration *pLogicalCameraConf = NULL;
    LogicalCameraInfo          *pLogicalCameraInfo = NULL;
    UINT32                      sensorId           = INVALID_INDEX;
    UINT32                      physicalCameraId   = INVALID_INDEX;
    BOOL                        isValid            = TRUE;
    g_chiContextOps.pTagOps(&tagOps);
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// 1) Loop physical camera in configuration table
    ///       Get sensor mode and fill device information for every physical camera.
    /// 2) Loop non-physical camera in configuration table
    ///       Loop physical cameras for this logical camera
    ///           Fill device info for non-physical camera
    ///       Composite sensor mode for non-physical camera
    ///       Consolidate static metadata for non-physical camera
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Go through logical camera configuration to create logic camera for physical camera. And get sensor mode
    // information and camera capability
    for (UINT32 i = 0 ; i < m_numOfLogicalCameraConfiguration; i++)
    {
        pLogicalCameraConf = &m_pLogicalCameraConfigurationInfo[i];

        if (isForExposed != pLogicalCameraConf->publicVisibility)
        {
            continue;
        }

        if (LogicalCameraType_Default != pLogicalCameraConf->logicalCameraType)
        {
            continue;
        }

        pLogicalCameraInfo                     = &m_logicalCameraInfo[m_numLogicalCameras];
        // use logical camera create sequence id as camera id
        pLogicalCameraInfo->cameraId           = m_numLogicalCameras;

        pLogicalCameraInfo->logicalCameraType  = static_cast<LogicalCameraType>(
            pLogicalCameraConf->logicalCameraType);
        pLogicalCameraInfo->numPhysicalCameras = pLogicalCameraConf->numOfDevices;
        CHICAMERAINFO* pChiCameraInfo          = &(pLogicalCameraInfo->m_cameraCaps);
        pChiCameraInfo->pLegacy                = &(pLogicalCameraInfo->m_cameraInfo);

        sensorId                               = pLogicalCameraConf->deviceInfo[0].sensorId;
        isValid                                = TRUE;

        CHX_LOG("index:%d,logical cameraid:%d,physicalCameraId:%d,type:%d,publicvisiblity:%d",
            i, pLogicalCameraConf->logicalCameraId, pLogicalCameraConf->deviceInfo[0].sensorId,
            pLogicalCameraConf->logicalCameraType, pLogicalCameraConf->publicVisibility);

        if (MaxNumImageSensors <= sensorId)
        {
            CHX_LOG_ERROR("Invalid sensor id! please check if m_pLogicalCameraConfigurationInfo[%d] is valid!", i);
            isValid = FALSE;
            return CDKResultEOutOfBounds;
        }

        physicalCameraId = sensorIdMap[sensorId];
        if (physicalCameraId == INVALID_INDEX)
        {
            CHX_LOG_WARN("Please check if sensor id:%d is probed successfully!", sensorId);
            continue;
        }

        result = g_chiContextOps.pGetCameraInfo(m_hCHIContext, physicalCameraId,
            pChiCameraInfo);

        if (CDKResultSuccess == result)
        {
            camera_metadata_ro_entry_t entry   = { 0 };
            const camera_metadata_t* pMetadata =
                (static_cast<camera_info_t*>(pChiCameraInfo->pLegacy))->static_camera_characteristics;

            if (0 == find_camera_metadata_ro_entry(pMetadata, ANDROID_REQUEST_PARTIAL_RESULT_COUNT, &entry))
            {
                UINT32 value = static_cast<UINT32>(*(entry.data.i32));
                m_numMetadataResults = (m_numMetadataResults > value) ? m_numMetadataResults : value;
            }

            pLogicalCameraInfo->primaryCameraId = sensorIdMap[pLogicalCameraConf->primarySensorID];
            if (pLogicalCameraInfo->primaryCameraId == INVALID_INDEX)
            {
                CHX_LOG_WARN("Invalid Primary Cam Id. Check if sensor id:%d is configured right!", sensorId);
                isValid = FALSE;
                continue;
            }

            pLogicalCameraInfo->pSensorModeInfo =
                static_cast<CHISENSORMODEINFO*>(CHX_CALLOC(sizeof(CHISENSORMODEINFO) * pChiCameraInfo->numSensorModes));

            if (NULL != pLogicalCameraInfo->pSensorModeInfo)
            {
                result = EnumerateSensorModes(physicalCameraId,
                                                pChiCameraInfo->numSensorModes,
                                                pLogicalCameraInfo->pSensorModeInfo);

                if (CDKResultSuccess == result)
                {
                    pLogicalCameraInfo->ppDeviceInfo =
                        static_cast<DeviceInfo**>(CHX_CALLOC(sizeof(DeviceInfo *) * pLogicalCameraConf->numOfDevices));

                    if (NULL != pLogicalCameraInfo->ppDeviceInfo)
                    {
                        pLogicalCameraInfo->ppDeviceInfo[0] =
                                        static_cast<DeviceInfo*>(CHX_CALLOC(sizeof(DeviceInfo)));

                        if (NULL != pLogicalCameraInfo->ppDeviceInfo[0])
                        {
                            pLogicalCameraInfo->ppDeviceInfo[0]->cameraId        = physicalCameraId;
                            pLogicalCameraInfo->ppDeviceInfo[0]->pSensorModeInfo = pLogicalCameraInfo->pSensorModeInfo;
                            pLogicalCameraInfo->ppDeviceInfo[0]->m_pDeviceCaps   = &(pLogicalCameraInfo->m_cameraCaps);
                            pLogicalCameraInfo->ppDeviceInfo[0]->pDeviceConfig   = NULL;
                        }
                        else
                        {
                            CHX_LOG_ERROR("Out of memory:pLogicalCameraInfo->ppDeviceInfo[0] allocate failed!");
                            isValid = FALSE;
                            result = CDKResultENoMemory;
                        }
                    }
                    else
                    {
                        CHX_LOG_ERROR("Out of memory:pLogicalCameraInfo->ppDeviceInfo allocate failed!");
                        if (NULL != pLogicalCameraInfo->pSensorModeInfo)
                        {
                            CHX_FREE(pLogicalCameraInfo->pSensorModeInfo);
                            pLogicalCameraInfo->pSensorModeInfo = NULL;
                        }
                        isValid = FALSE;
                        result = CDKResultENoMemory;
                    }
                }
                else
                {
                    CHX_LOG_ERROR("EnumerateSensorModes failed:physical cameraID:%d,numofModes:%d",
                        physicalCameraId,
                        pChiCameraInfo->numSensorModes);
                    if (NULL != pLogicalCameraInfo->pSensorModeInfo)
                    {
                        CHX_FREE(pLogicalCameraInfo->pSensorModeInfo);
                        pLogicalCameraInfo->pSensorModeInfo = NULL;
                    }
                    isValid = FALSE;
                }
            }
            else
            {
                isValid = FALSE;
                result  = CDKResultENoMemory;
                CHX_LOG_ERROR("Out of memory:pLogicalCameraInfo->pSensorModeInfo allocate failed!");
            }
        }
        else
        {
            isValid = FALSE;
            CHX_LOG_ERROR("GetCameraInfo failed! Please check if the sensor(slot id:%d) probe successfully!",
                physicalCameraId);
        }

        if (FALSE == isValid)
        {
            // go on going through other physical device.
            result = CDKResultSuccess;
        }
        else
        {
            if (CDKResultSuccess == tagOps.pQueryVendorTagLocation("org.codeaurora.qcamera3.logicalCameraType",
                "logical_camera_type", &tag))
            {
                CHIHANDLE  staticMetaDataHandle = const_cast<camera_metadata_t*>(
                    pLogicalCameraInfo->m_cameraInfo.static_camera_characteristics);
                tagOps.pSetMetaData(staticMetaDataHandle, tag, &pLogicalCameraInfo->logicalCameraType, sizeof(SBYTE));
            }

            if (TRUE == pLogicalCameraConf->publicVisibility)
            {
                m_cameraMap[m_numLogicalCameras]     = m_numLogicalCameras;
                m_numExposedLogicalCameras ++;
            }
            else
            {
                m_cameraMap[m_numLogicalCameras]     = pLogicalCameraConf->logicalCameraId;
            }
            m_cameraReverseMap[physicalCameraId] = pLogicalCameraInfo->cameraId;
            m_numLogicalCameras++ ;
            m_numPhysicalCameras++;
            pLogicalCameraInfo->publicVisiblity = pLogicalCameraConf->publicVisibility;
        }
    }

    // Go through logical camera configuration to create non-physical camera logical camera.
    for(UINT32 i = 0 ; i < m_numOfLogicalCameraConfiguration; i++)
    {
        if (FALSE == isMultiCameraEnabled)
        {
            CHX_LOG_INFO("Multi Camera is disabled!");
            break;
        }

        pLogicalCameraConf = &m_pLogicalCameraConfigurationInfo[i];

        if (isForExposed != pLogicalCameraConf->publicVisibility)
        {
            continue;
        }

        if (LogicalCameraType_Default == pLogicalCameraConf->logicalCameraType)
        {
            continue;
        }

#if ((CAMX_ANDROID_API) && (CAMX_ANDROID_API < 28)) // Android-P or better
        if (LogicalCameraType_DualApp == pLogicalCameraConf->logicalCameraType)
        {
            continue;
        }
#endif // ((CAMX_ANDROID_API) && (CAMX_ANDROID_API >= 28))

        CHX_LOG("index:%d,logical cameraid:%d,sensorId:%d,type:%d, publicVisibility:%d",
            i, pLogicalCameraConf->logicalCameraId, pLogicalCameraConf->deviceInfo[0].sensorId,
            pLogicalCameraConf->logicalCameraType, pLogicalCameraConf->publicVisibility);

        pLogicalCameraInfo                     = &m_logicalCameraInfo[m_numLogicalCameras];
        pLogicalCameraInfo->cameraId           = m_numLogicalCameras;
        pLogicalCameraInfo->logicalCameraType  = LogicalCameraType(pLogicalCameraConf->logicalCameraType);
        pLogicalCameraInfo->numPhysicalCameras = pLogicalCameraConf->numOfDevices;

        pLogicalCameraInfo->primaryCameraId    = sensorIdMap[pLogicalCameraConf->primarySensorID];
        if (pLogicalCameraInfo->primaryCameraId == INVALID_INDEX)
        {
            CHX_LOG_ERROR("Invalid Primary Cam Id. Check if logical id:%d is configured right!",
                          pLogicalCameraConf->logicalCameraId);
            isValid = FALSE;
            continue;
        }

        isValid                                = TRUE;

        // number of phsyical device in this logical camera should be less than number of physical device on this device
        if (pLogicalCameraConf->numOfDevices > numOfPhysicalCamera)
        {
            CHX_LOG_ERROR("Please check m_pLogicalCameraConfigurationInfo[%d] definition, nonOfPhysicalCamera:%d!", i,
                numOfPhysicalCamera);
            isValid = FALSE;
            continue;
        }

        pLogicalCameraInfo->ppDeviceInfo       =
            static_cast<DeviceInfo**>(CHX_CALLOC(sizeof(DeviceInfo *) * pLogicalCameraConf->numOfDevices));

        // fill device information for logical camera from physical camera
        if (NULL != pLogicalCameraInfo->ppDeviceInfo)
        {
            // Fill device information of logical camera
            for(UINT32 deviceIndex = 0 ; deviceIndex < pLogicalCameraConf->numOfDevices; deviceIndex ++)
            {
                sensorId = pLogicalCameraConf->deviceInfo[deviceIndex].sensorId;
                if (MaxNumImageSensors <= sensorId)
                {
                    CHX_LOG_ERROR("Invalid sensor id! please check if m_pLogicalCameraConfigurationInfo[%d] is valid!", i);
                    isValid = FALSE;
                    break;
                }

                physicalCameraId = sensorIdMap[sensorId];

                if (physicalCameraId == INVALID_INDEX)
                {
                    CHX_LOG_ERROR("Please check if sensor id:%d is probed successfully!", sensorId);
                    isValid = FALSE;
                    break;
                }

                const LogicalCameraInfo* pPhysicalCameraInfo = GetPhysicalCameraInfo(physicalCameraId);
                if (NULL != pPhysicalCameraInfo)
                {
                    pLogicalCameraInfo->ppDeviceInfo[deviceIndex] =
                                        static_cast<DeviceInfo*>(CHX_CALLOC(sizeof(DeviceInfo)));

                    if (NULL != pLogicalCameraInfo->ppDeviceInfo[deviceIndex])
                    {
                        *pLogicalCameraInfo->ppDeviceInfo[deviceIndex] = *pPhysicalCameraInfo->ppDeviceInfo[0];
                    }
                    else
                    {
                        CHX_LOG_ERROR("Out of memory:pLogicalCameraInfo->ppDeviceInfo[0] allocate failed!");
                        isValid = FALSE;
                        result = CDKResultENoMemory;
                    }

                    pLogicalCameraInfo->ppDeviceInfo[deviceIndex]->pDeviceConfig = &(pLogicalCameraConf->deviceInfo[deviceIndex]);
                }
                else
                {
                    CHX_LOG_ERROR("Please check m_pLogicalCameraConfigurationInfo[%d] definition, camera Id:%d is not exited",
                        i, physicalCameraId);
                    isValid = FALSE;
                    break;
                }
            }

            // Fill Logical camera camera information and logical camera capability.
            if (TRUE == isValid)
            {
                // if it is not default logical camera, it needs to call multi camera controller to create
                // consolidate camera info and consolidate capability for logical camera
                if (LogicalCameraType_Default != pLogicalCameraInfo->logicalCameraType)
                {
                    // Find primary camera index by matching logical camera configuration table to physical device id
                    UINT32 primaryCameraIndex = GetPrimaryCameraIndex(pLogicalCameraInfo);

                    camera_info_t* pCameraInfo = static_cast<camera_info_t*>
                        (pLogicalCameraInfo->ppDeviceInfo[primaryCameraIndex]->m_pDeviceCaps->pLegacy);

                    ChxUtils::Memcpy(&(pLogicalCameraInfo->m_cameraInfo), pCameraInfo, sizeof(camera_info_t));

                    pLogicalCameraInfo->m_cameraInfo.static_camera_characteristics =
                        reinterpret_cast<const camera_metadata_t*>(ChxUtils::AndroidMetadata::AllocateCopyMetaData(
                           pCameraInfo->static_camera_characteristics));

                    pLogicalCameraInfo->m_cameraCaps.pLegacy = &(pLogicalCameraInfo->m_cameraInfo);

                    MultiCamController::ConsolidateCameraInfo(pLogicalCameraInfo);

                    if (CDKResultSuccess == tagOps.pQueryVendorTagLocation("org.codeaurora.qcamera3.logicalCameraType",
                        "logical_camera_type", &tag))
                    {
                        CHIHANDLE  staticMetaDataHandle = const_cast<camera_metadata_t*>(
                            pLogicalCameraInfo->m_cameraInfo.static_camera_characteristics);

                        tagOps.pSetMetaData(staticMetaDataHandle, tag, &pLogicalCameraInfo->logicalCameraType, sizeof(SBYTE));
                    }

                    // here use deviceinfo[primaryCameraIndex] sensor mode info to fill logical camera sensor mode info.
                    // todo: to check if it is necessary to merge sensor mode information from multi sensor.
                    pLogicalCameraInfo->pSensorModeInfo =
                        const_cast<CHISENSORMODEINFO*>(pLogicalCameraInfo->ppDeviceInfo[primaryCameraIndex]->pSensorModeInfo);
                }
            }
        }
        else
        {   isValid            = FALSE;
            CHX_LOG_ERROR("Allocate failed:pLogicalCameraInfo->ppDeviceInfo!");
        }

        if (TRUE == isValid)
        {
            if (TRUE == pLogicalCameraConf->publicVisibility)
            {
                m_cameraMap[m_numLogicalCameras]     = m_numLogicalCameras;
                m_numExposedLogicalCameras ++;
            }
            else
            {
                m_cameraMap[m_numLogicalCameras]     = pLogicalCameraConf->logicalCameraId;
            }
            m_cameraReverseMap[m_numLogicalCameras] = pLogicalCameraInfo->cameraId;
            m_numLogicalCameras++ ;
            pLogicalCameraInfo->publicVisiblity = pLogicalCameraConf->publicVisibility;
        }

    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::EnumerateCameras
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::EnumerateCameras()
{
    CDKResult    result              = CDKResultEFailed;
    CHITAGSOPS   tagOps              = { 0 };
    UINT32       tag;

    UINT32       numOfPhysicalCamera = g_chiContextOps.pGetNumCameras(m_hCHIContext);

    UINT32       sensorIdMap[MaxNumImageSensors];
    UINT32       sensorId           = INVALID_INDEX;

    g_chiContextOps.pTagOps(&tagOps);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// 1) Get map between sensor ID and camera ID
    ///       sensor ID is slot id which is from sensor driver
    ///       camera ID is used in camx which is used to access tuning data/sensor physical device.
    /// 2) Build logical camera information for the camera which will be exposed to application
    /// 3) Build logical camera information for the camera which will be NOT exposed to application
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    for (UINT32 i = 0 ; i < MaxNumImageSensors ; i++)
    {
        sensorIdMap[i] = INVALID_INDEX;
        m_cameraMap[i] = INVALID_INDEX;
    }

    CHICAMERAINFO cameraInfo;
    camera_info_t cam_info;
    cameraInfo.pLegacy = &cam_info;
    BOOL bMultiCameraEnabled = FALSE;
    // Get the sensor id and camera id map information of physical camera
    for (UINT32 i = 0 ; i < numOfPhysicalCamera ; i++)
    {
        result = g_chiContextOps.pGetCameraInfo(m_hCHIContext, i,
            &cameraInfo);

        if (CDKResultSuccess == result)
        {
            UINT32 sensorId = cameraInfo.sensorCaps.sensorId;
            sensorIdMap[sensorId] = i;

            CHX_LOG_INFO("cameraID map information: sensorid:%d,cameraid:%d", sensorId,i);

            if ((REAR_AUX == cameraInfo.sensorCaps.positionType) ||
                (FRONT_AUX == cameraInfo.sensorCaps.positionType))
            {
                bMultiCameraEnabled = TRUE;
                CHX_LOG_INFO("Multi camera is enabled!");
            }
        }
    }

    // build logical camera information for the logical camera which will be exposed to application
    BuildLogicalCameraInfo(sensorIdMap, bMultiCameraEnabled, TRUE);

    // build logical camera information for the logical camera which will be not exposed to application
    BuildLogicalCameraInfo(sensorIdMap, bMultiCameraEnabled, FALSE);

    CHX_LOG_INFO("Camera Count: m_numExposedLogicalCameras = %d, m_numLogicalCameras = %d Physical camera = %d",
        m_numExposedLogicalCameras, m_numLogicalCameras, m_numPhysicalCameras);

    for (UINT i = 0; i < m_numLogicalCameras; i++)
    {
        CHX_LOG_INFO("Internal camera ID = %d Num Devices = %d, logical camera(APP) ID = %d,isExposed:%d",
            i,
            m_logicalCameraInfo[i].numPhysicalCameras,
            m_cameraMap[i],
            m_logicalCameraInfo[i].publicVisiblity);

        for (UINT j = 0; j < m_logicalCameraInfo[i].numPhysicalCameras; j++)
        {
            CHX_LOG_INFO("CameraID Published ID = %d DeviceID = %d", i, m_logicalCameraInfo[i].ppDeviceInfo[j]->cameraId);
        }
    }

    // here is the sensor hardware sync configuration for MTP dual camera solution. customer need to
    // configure sensor hardware sync mode as their hardware design.
    for (UINT i = 0; i < m_numPhysicalCameras; i++)
    {
        CHIHANDLE    staticMetaDataHandle = const_cast<camera_metadata_t*>(
            m_logicalCameraInfo[i].m_cameraInfo.static_camera_characteristics);

        CHITAGSOPS   tagOps        = { 0 };
        UINT32 tag;
        g_chiContextOps.pTagOps(&tagOps);

        CHX_LOG_INFO("position:%d,index:%d,", m_logicalCameraInfo[i].m_cameraCaps.sensorCaps.positionType, i);

        if (REAR == m_logicalCameraInfo[i].m_cameraCaps.sensorCaps.positionType)
        {
            if (CDKResultSuccess == tagOps.pQueryVendorTagLocation("com.qti.chi.multicamerasensorconfig",
                "sensorsyncmodeconfig", &tag))
            {
                SensorSyncModeMetadata syncMode = { 0 };
                syncMode.isValid        = TRUE;
                syncMode.sensorSyncMode = MasterMode;
                tagOps.pSetMetaData(staticMetaDataHandle, tag, &syncMode, sizeof(syncMode));
            }
        }
        else if (REAR_AUX == m_logicalCameraInfo[i].m_cameraCaps.sensorCaps.positionType)
        {
            if (CDKResultSuccess == tagOps.pQueryVendorTagLocation("com.qti.chi.multicamerasensorconfig",
                "sensorsyncmodeconfig", &tag))
            {
                SensorSyncModeMetadata syncMode = { 0 };
                syncMode.isValid        = TRUE;
                syncMode.sensorSyncMode = SlaveMode;
                tagOps.pSetMetaData(staticMetaDataHandle, tag, &syncMode, sizeof(syncMode));
            }
        }
        if (CDKResultSuccess == tagOps.pQueryVendorTagLocation("org.quic.camera.MaxSingleISPCapabilities",
                "numIFEsforGivenTarget", &tag))
        {
            if (i == 0)
            {
                UINT32 numIFEsforGivenTarget = 0;
                tagOps.pGetMetaData(staticMetaDataHandle, tag, &numIFEsforGivenTarget,
                                    sizeof(numIFEsforGivenTarget));
                m_totalResourceBudget = numIFEsforGivenTarget * m_singleISPResourceCost;
                CHX_LOG_INFO("Number of IFE for a given target=%d", numIFEsforGivenTarget);
            }
        }
        else
        {
            m_totalResourceBudget = m_singleISPResourceCost * 2;
        }
    }
    CHX_LOG_INFO(" m_totalResourceBudget=%d,  m_singleISPResourceCost=%d",
            m_totalResourceBudget, m_singleISPResourceCost);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::GetCameraInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::GetCameraInfo(
    uint32_t     logicalCameraId,
    camera_info* cameraInfo)
{
    CDKResult     result = CDKResultEFailed;

    if (logicalCameraId < m_numLogicalCameras)
    {
        // no need for a deep copy...assuming 1 physical camera per logical camera (for now)
        ChxUtils::Memcpy(cameraInfo,
            &m_logicalCameraInfo[logicalCameraId].m_cameraInfo, sizeof(camera_info));

        result = CDKResultSuccess;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::ModifySettings
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::ModifySettings(
    VOID*     pPriv)
{
    // pPriv is an implementation-specific data type.  Implementations can cast this to the format they expect to consume, and
    // store/modify any data which is available in this class

    // sample code...this uses the input to create a data structure used to hold configuration settings
    ChiOverrideModifySetting* pMod = static_cast<ChiOverrideModifySetting*>(pPriv);
    CHX_ASSERT(pMod->token.size <= sizeof(UINT32));

    if (pMod->token.id == static_cast<UINT32>(ChxSettingsToken::OverrideCameraClose))
    {
        // Config settings for camera close indication based on ExtendClose
        *static_cast<UINT32*>(pMod->pData) = ExtensionModule::GetInstance()->IsCameraClose();
        m_pConfigSettings[pMod->token.id] = *static_cast<UINT32*>(pMod->pData);
    }
    else if (pMod->token.id == static_cast<UINT32>(ChxSettingsToken::OverrideCameraOpen))
    {
        // Config settings for camera open indication based on ExtendOpen
        *static_cast<UINT32*>(pMod->pData) = ExtensionModule::GetInstance()->IsCameraOpen();
        m_pConfigSettings[pMod->token.id] = *static_cast<UINT32*>(pMod->pData);
    }
    else if (pMod->token.id == static_cast<UINT32>(ChxSettingsToken::OverrideLogLevels))
    {
        if (NULL != pMod->pData)
        {
            g_enableChxLogs = *static_cast<UINT32*>(pMod->pData);
        }
    }
    else if (pMod->token.id == static_cast<UINT32>(ChxSettingsToken::EnableRequestMapping))
    {
        // This setting's pData is NOT A POINTER. It is a BOOL saved in the pointer
        g_logRequestMapping = (TRUE == reinterpret_cast<UINT64>(pMod->pData));
    }
    else if (pMod->token.id == 13)
    {
        m_pNumPCRsBeforeStreamOn = static_cast<UINT32*>(pMod->pData);
    }
    else
    {
        if (NULL != pMod->pData)
        {
            m_pConfigSettings[pMod->token.id] = *static_cast<UINT32*>(pMod->pData);
        }
        else
        {
            m_pConfigSettings[pMod->token.id] = 0;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::DefaultRequestSettings
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::DefaultRequestSettings(
    uint32_t                  cameraId,
    int                       requestTemplate,
    const camera_metadata_t** settings)
{
    (VOID)cameraId;
    (VOID)requestTemplate;
    (VOID)settings;
#if defined (_LINUX)
    if (NULL == m_pDefaultSettings)
    {
        m_pDefaultSettings = allocate_camera_metadata(DefaultSettingsNumEntries, DefaultSettingsDataSize);
    }
    else
    {
        ChxUtils::Memset(m_pDefaultSettings, 0, DefaultSettingsDataSize);
    }

    if (m_pDefaultSettings != NULL)
    {
        // Fill in any default settings (vendor tags) that needs to be added
        *settings = (const camera_metadata_t*)(m_pDefaultSettings);
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::InitializeOverrideSession
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::InitializeOverrideSession(
    uint32_t                        logicalCameraId,
    const camera3_device_t*         pCamera3Device,
    const chi_hal_ops_t*            chiHalOps,
    camera3_stream_configuration_t* pStreamConfig,
    int*                            pIsOverrideEnabled,
    VOID**                          pPrivate)
{
    CDKResult          result             = CDKResultSuccess;
    UINT32             modeCount          = 0;
    ChiSensorModeInfo* pAllModes          = NULL;
    UINT32             fps                = *m_pDefaultMaxFPS;
    BOOL               isVideoMode        = FALSE;
    uint32_t           operation_mode;
    static BOOL        fovcModeCheck      = EnableFOVCUseCase();
    UsecaseId          selectedUsecaseId  = UsecaseId::NoMatch;
    UINT               minSessionFps      = 0;
    UINT               maxSessionFps      = 0;

    *pPrivate             = NULL;
    *pIsOverrideEnabled   = FALSE;
    m_aFlushInProgress    = FALSE;
    m_firstResult         = FALSE;
    m_hasFlushOccurred    = FALSE;

    if (NULL == m_hCHIContext)
    {
        m_hCHIContext = g_chiContextOps.pOpenContext();
    }

    ChiVendorTagsOps vendorTagOps = { 0 };
    g_chiContextOps.pTagOps(&vendorTagOps);
    operation_mode                = pStreamConfig->operation_mode >> 16;
    operation_mode                = operation_mode & 0x000F;
    pStreamConfig->operation_mode = pStreamConfig->operation_mode & 0xFFFF;

    for (UINT32 stream = 0; stream < pStreamConfig->num_streams; stream++)
    {
        if (0 != (pStreamConfig->streams[stream]->usage & GrallocUsageHwVideoEncoder))
        {
            isVideoMode = TRUE;
            break;
        }
    }

    if ((isVideoMode == TRUE) && (operation_mode != 0))
    {
        UINT32             numSensorModes  = m_logicalCameraInfo[logicalCameraId].m_cameraCaps.numSensorModes;
        CHISENSORMODEINFO* pAllSensorModes = m_logicalCameraInfo[logicalCameraId].pSensorModeInfo;

        if ((operation_mode - 1) >= numSensorModes)
        {
            result = CDKResultEOverflow;
            CHX_LOG_ERROR("operation_mode: %d, numSensorModes: %d", operation_mode, numSensorModes);
        }
        else
        {
            fps = pAllSensorModes[operation_mode - 1].frameRate;
        }
    }
    m_pResourcesUsedLock->Lock();

    if (m_totalResourceBudget > CostOfAnyCurrentlyOpenLogicalCameras())
    {
        UINT32 myLogicalCamCost = CostOfLogicalCamera(logicalCameraId, pStreamConfig);
        if (myLogicalCamCost > (m_totalResourceBudget - CostOfAnyCurrentlyOpenLogicalCameras()))
        {
            CHX_LOG_ERROR("Insufficient HW resources! myLogicalCamCost = %d, remaining cost = %d",
                myLogicalCamCost, (m_totalResourceBudget - CostOfAnyCurrentlyOpenLogicalCameras()));
            result = CamxResultEResource;
        }
        else
        {
            m_IFEResourceCost[logicalCameraId] = myLogicalCamCost;
            m_resourcesUsed                   += myLogicalCamCost;
        }
    }
    else
    {
        CHX_LOG_ERROR("Insufficient HW resources! TotalResourceCost = %d, CostOfAnyCurrentlyOpencamera = %d",
            m_totalResourceBudget, CostOfAnyCurrentlyOpenLogicalCameras());
        result = CamxResultEResource;
    }
    m_pResourcesUsedLock->Unlock();

    if (CDKResultSuccess == result)
    {
#if defined(CAMX_ANDROID_API) && (CAMX_ANDROID_API >= 28) //Android-P or better
        camera_metadata_t *metadata = const_cast<camera_metadata_t*>(pStreamConfig->session_parameters);

        camera_metadata_entry_t entry = { 0 };
        entry.tag = ANDROID_CONTROL_AE_TARGET_FPS_RANGE;

        // The client may choose to send NULL sesssion parameter, which is fine. For example, torch mode
        // will have NULL session param.
        if (metadata != NULL)
        {
            int ret = find_camera_metadata_entry(metadata, entry.tag, &entry);

            if(ret == 0) {
                minSessionFps = entry.data.i32[0];
                maxSessionFps = entry.data.i32[1];
                m_usecaseMaxFPS = maxSessionFps;
            }
        }
#endif

        CHIHANDLE    staticMetaDataHandle = const_cast<camera_metadata_t*>(
                                            m_logicalCameraInfo[logicalCameraId].m_cameraInfo.static_camera_characteristics);
        UINT32       metaTagPreviewFPS    = 0;
        UINT32       metaTagVideoFPS      = 0;
        CHITAGSOPS   vendorTagOps;

        m_previewFPS           = 0;
        m_videoFPS             = 0;
        GetInstance()->GetVendorTagOps(&vendorTagOps);

        result = vendorTagOps.pQueryVendorTagLocation("org.quic.camera2.streamBasedFPS.info", "PreviewFPS",
                                                      &metaTagPreviewFPS);
        if (CDKResultSuccess == result)
        {
            vendorTagOps.pGetMetaData(staticMetaDataHandle, metaTagPreviewFPS, &m_previewFPS,
                                      sizeof(m_previewFPS));
        }

        result = vendorTagOps.pQueryVendorTagLocation("org.quic.camera2.streamBasedFPS.info", "VideoFPS", &metaTagVideoFPS);
        if (CDKResultSuccess == result)
        {
            vendorTagOps.pGetMetaData(staticMetaDataHandle, metaTagVideoFPS, &m_videoFPS,
                                      sizeof(m_videoFPS));
        }

        if ((StreamConfigModeConstrainedHighSpeed == pStreamConfig->operation_mode) ||
            (StreamConfigModeSuperSlowMotionFRC == pStreamConfig->operation_mode))
        {
            SearchNumBatchedFrames(logicalCameraId, pStreamConfig,
                                   &m_usecaseNumBatchedFrames, &m_usecaseMaxFPS, maxSessionFps);
            if (480 > m_usecaseMaxFPS)
            {
                m_CurrentpowerHint = PERF_LOCK_POWER_HINT_VIDEO_ENCODE_HFR;
            }
            else
            {
                // For 480FPS or higher, require more aggresive power hint
                m_CurrentpowerHint = PERF_LOCK_POWER_HINT_VIDEO_ENCODE_HFR_480FPS;
            }
        }
        else
        {
            // Not a HFR usecase, batch frames value need to be set to 1.
            m_usecaseNumBatchedFrames = 1;
            if (maxSessionFps == 0)
            {
                m_usecaseMaxFPS = fps;
            }
            if (TRUE == isVideoMode)
            {
                if (30 >= m_usecaseMaxFPS)
                {
                    m_CurrentpowerHint = PERF_LOCK_POWER_HINT_VIDEO_ENCODE;
                }
                else
                {
                    m_CurrentpowerHint = PERF_LOCK_POWER_HINT_VIDEO_ENCODE_60FPS;
                }
            }
            else
            {
                m_CurrentpowerHint = PERF_LOCK_POWER_HINT_PREVIEW;
            }
        }

        if ((NULL != m_pPerfLockManager[logicalCameraId]) && (m_CurrentpowerHint != m_previousPowerHint))
        {
            m_pPerfLockManager[logicalCameraId]->ReleasePerfLock(m_previousPowerHint);
        }

        // Example [B == batch]: (240 FPS / 4 FPB = 60 BPS) / 30 FPS (Stats frequency goal) = 2 BPF i.e. skip every other stats
        *m_pStatsSkipPattern = m_usecaseMaxFPS / m_usecaseNumBatchedFrames / 30;
        if (*m_pStatsSkipPattern < 1)
        {
            *m_pStatsSkipPattern = 1;
        }

        m_VideoHDRMode = (StreamConfigModeVideoHdr == pStreamConfig->operation_mode);

        m_torchWidgetUsecase = (StreamConfigModeQTITorchWidget == pStreamConfig->operation_mode);

        // this check is introduced to avoid set *m_pEnableFOVC == 1 if fovcEnable is disabled in
        // overridesettings & fovc bit is set in operation mode.
        // as well as to avoid set,when we switch Usecases.
        if (TRUE == fovcModeCheck)
        {
            *m_pEnableFOVC = ((pStreamConfig->operation_mode & StreamConfigModeQTIFOVC) == StreamConfigModeQTIFOVC) ? 1 : 0;
        }

        SetHALOps(chiHalOps, logicalCameraId);

        m_logicalCameraInfo[logicalCameraId].m_pCamera3Device = pCamera3Device;

        selectedUsecaseId = m_pUsecaseSelector->GetMatchingUsecase(&m_logicalCameraInfo[logicalCameraId],
                                                                   pStreamConfig);

        CHX_LOG_CONFIG("Session_parameters FPS range %d:%d, previewFPS %d, videoFPS %d,"
                       "BatchSize: %u FPS: %u SkipPattern: %u,"
                       "cameraId = %d selected use case = %d",
                       minSessionFps,
                       maxSessionFps,
                       m_previewFPS,
                       m_videoFPS,
                       m_usecaseNumBatchedFrames,
                       m_usecaseMaxFPS,
                       *m_pStatsSkipPattern,
                       logicalCameraId,
                       selectedUsecaseId);

        // FastShutter mode supported only in ZSL usecase.
        if ((pStreamConfig->operation_mode == StreamConfigModeFastShutter) &&
            (UsecaseId::PreviewZSL         != selectedUsecaseId))
        {
            pStreamConfig->operation_mode = StreamConfigModeNormal;
        }
        m_operationMode[logicalCameraId] = pStreamConfig->operation_mode;
    }

    if (UsecaseId::NoMatch != selectedUsecaseId)
    {
        m_pSelectedUsecase[logicalCameraId] =
            m_pUsecaseFactory->CreateUsecaseObject(&m_logicalCameraInfo[logicalCameraId],
                                                   selectedUsecaseId, pStreamConfig);

        if (NULL != m_pSelectedUsecase[logicalCameraId])
        {
            m_pStreamConfig[logicalCameraId] = static_cast<camera3_stream_configuration_t*>(
                CHX_CALLOC(sizeof(camera3_stream_configuration_t)));
            m_pStreamConfig[logicalCameraId]->streams = static_cast<camera3_stream_t**>(
                CHX_CALLOC(sizeof(camera3_stream_t*) * pStreamConfig->num_streams));
            m_pStreamConfig[logicalCameraId]->num_streams = pStreamConfig->num_streams;

            for (UINT32 i = 0; i< m_pStreamConfig[logicalCameraId]->num_streams; i++)
            {
                m_pStreamConfig[logicalCameraId]->streams[i] = pStreamConfig->streams[i];
            }

            m_pStreamConfig[logicalCameraId]->operation_mode = pStreamConfig->operation_mode;

            // use camera device / used for recovery only for regular session
            m_SelectedUsecaseId[logicalCameraId] = (UINT32)selectedUsecaseId;
            CHX_LOG_CONFIG("Logical cam Id = %d usecase addr = %p", logicalCameraId, m_pSelectedUsecase[
                logicalCameraId]);

            m_pCameraDeviceInfo[logicalCameraId].m_pCamera3Device = pCamera3Device;

            *pIsOverrideEnabled = TRUE;
        }
        else
        {
            CHX_LOG_ERROR("For cameraId = %d CreateUsecaseObject failed", logicalCameraId);
            m_logicalCameraInfo[logicalCameraId].m_pCamera3Device = NULL;

            m_pResourcesUsedLock->Lock();
            // Reset the m_resourcesUsed & m_IFEResourceCost if usecase creation failed
            if (m_resourcesUsed >= m_IFEResourceCost[logicalCameraId])                   // to avoid underflow
            {
                m_resourcesUsed                   -= m_IFEResourceCost[logicalCameraId]; // reduce the total cost
                m_IFEResourceCost[logicalCameraId] = 0;
            }
            m_pResourcesUsedLock->Unlock();
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::FinalizeOverrideSession
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::FinalizeOverrideSession(
     const camera3_device_t* camera3_device,
     VOID*                   pPriv)
{
    (VOID)camera3_device;
    (VOID)pPriv;

    return CDKResultSuccess;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////
// ExtensionModule::GetSelectedUsecase
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////
Usecase* ExtensionModule::GetSelectedUsecase(
    const camera3_device_t *pCamera3Device)
{
    Usecase* pUsecase = NULL;

    //TODO: enable when FastAec is enabled
    //look for matching entry in the fast aec list first

    //for (UINT32 id = 0; id < MaxNumImageSensors; id++)
    //{
    //    if ((NULL != m_pFastAecSession[id]) &&
    //        (m_pFastAecSession[id]->GetCamera3Device() == pCamera3Device))
    //    {
    //        pUsecase = m_pFastAecSession[id]->GetUsecase();
    //        break;
    //    }
    //}

    if (NULL == pUsecase)
    {
        for (UINT32 id = 0; id < MaxNumImageSensors; id++)
        {
            if (m_pCameraDeviceInfo[id].m_pCamera3Device && m_pCameraDeviceInfo[id].m_pCamera3Device
                == pCamera3Device)
            {
                pUsecase = m_pSelectedUsecase[id];
                break;
            }
        }
    }

    return pUsecase;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::TeardownOverrideSession
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::TeardownOverrideSession(
    const camera3_device_t* camera3_device,
    UINT64                  session,
    VOID*                   pPriv)
{
    (VOID)session;
    (VOID)pPriv;

    UINT32 logicalCameraId = GetCameraIdfromDevice(camera3_device);

    if (logicalCameraId >= MaxNumImageSensors)
    {
        CHX_LOG_ERROR("logicalCameraId: %d excess MaxnumImageSensors", logicalCameraId);
        return;
    }
    m_pRecoveryLock->Lock();
    if (TRUE == m_RecoveryInProgress)
    {
        CHX_LOG_INFO("Wait for recovery to finish, before starting teardown");
        m_pRecoveryCondition->Wait(m_pRecoveryLock->GetNativeHandle());
    }
    m_TeardownInProgress = TRUE;
    m_pRecoveryLock->Unlock();
    TeardownOverrideUsecase(camera3_device, FALSE);
    if (logicalCameraId < MaxNumImageSensors)
    {
        m_logicalCameraInfo[logicalCameraId].m_pCamera3Device = NULL;
    }

    CHX_LOG("Free up stream config memory");
    m_pCameraDeviceInfo[logicalCameraId].m_pCamera3Device = NULL;

    // free up m_pStreamConfig
    if (NULL != m_pStreamConfig[logicalCameraId])
    {
        if (NULL != m_pStreamConfig[logicalCameraId]->streams)
        {
            CHX_FREE(m_pStreamConfig[logicalCameraId]->streams);
            m_pStreamConfig[logicalCameraId]->streams = NULL;
        }
        CHX_FREE(m_pStreamConfig[logicalCameraId]);
        m_pStreamConfig[logicalCameraId] = NULL;
    }

    m_logicalCameraInfo[logicalCameraId].m_pCamera3Device = NULL;
    SetHALOps(NULL, logicalCameraId);

    if (NULL != m_pPerfLockManager[logicalCameraId])
    {
        m_pPerfLockManager[logicalCameraId]->ReleasePerfLock(m_CurrentpowerHint);
    }

    m_pRecoveryLock->Lock();
    m_TeardownInProgress = FALSE;
    m_pRecoveryLock->Unlock();

    m_pResourcesUsedLock->Lock();
    if (m_resourcesUsed >= m_IFEResourceCost[logicalCameraId])                   // to avoid underflow
    {
        m_resourcesUsed                   -= m_IFEResourceCost[logicalCameraId]; // reduce the total cost
        m_IFEResourceCost[logicalCameraId] = 0;
    }
    m_pResourcesUsedLock->Unlock();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::TeardownOverrideUsecase
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::TeardownOverrideUsecase(
    const camera3_device_t* camera3_device,
    BOOL                    isForced)
{
    CHX_LOG_INFO("[E]");

    // Overriding the camera close indication in ExtendClose
    UINT32   logicalCameraId = GetCameraIdfromDevice(camera3_device);
    Usecase* pUsecase        = NULL;

    m_pDestroyLock->Lock();
    if (logicalCameraId < MaxNumImageSensors)
    {
        pUsecase = m_pSelectedUsecase[logicalCameraId];
        m_pSelectedUsecase[logicalCameraId] = NULL;
    }
    m_pDestroyLock->Unlock();

    if (NULL != pUsecase)
    {
        pUsecase->DestroyObject(isForced);
    }

    CHX_LOG_INFO("[X]");

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::OverrideProcessRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::OverrideProcessRequest(
    const camera3_device_t*     camera3_device,
    camera3_capture_request_t*  pCaptureRequest,
    VOID*                       pPriv)
{
    CDKResult result = CDKResultSuccess;

    for (UINT32 i = 0; i < pCaptureRequest->num_output_buffers; i++)
    {
        if (NULL != pCaptureRequest->output_buffers)
        {
            ChxUtils::WaitOnAcquireFence(&pCaptureRequest->output_buffers[i]);
            INT*   pAcquireFence = (INT*)&pCaptureRequest->output_buffers[i].acquire_fence;
            *pAcquireFence = -1;
        }
    }

    UINT32 logicalCameraId = GetCameraIdfromDevice(camera3_device);

    if (logicalCameraId != CDKInvalidId && NULL != pCaptureRequest->settings)
    {
        FreeLastKnownRequestSetting(logicalCameraId);
        m_pLastKnownRequestSettings[logicalCameraId] = allocate_copy_camera_metadata_checked(pCaptureRequest->settings,
            get_camera_metadata_size(pCaptureRequest->settings));
    }

    // Set valid metadata after flush if settings aren't available
    if ((logicalCameraId != CDKInvalidId) &&
        (TRUE == m_hasFlushOccurred) &&
        (NULL == pCaptureRequest->settings))
    {
        CHX_LOG_INFO("Setting Request to m_pLastKnownRequestSettings after flush for frame_number:%d",
                     pCaptureRequest->frame_number);
        pCaptureRequest->settings   = m_pLastKnownRequestSettings[logicalCameraId];
        m_hasFlushOccurred          = FALSE;
    }

    if (TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aFlushInProgress)))
    {
        CHX_LOG_INFO("flush enabled, frame %d", pCaptureRequest->frame_number);
        HandleProcessRequestErrorAllPCRs(pCaptureRequest, logicalCameraId);
        return CDKResultSuccess;
    }

    if (ChxUtils::AndroidMetadata::IsLongExposureCapture(const_cast<camera_metadata_t*>(pCaptureRequest->settings)))
    {
        ChxUtils::AtomicStoreU32(&m_aLongExposureInProgress, TRUE);
        m_longExposureFrame = pCaptureRequest->frame_number;
        CHX_LOG_INFO("Long exposure enabled in frame %d", pCaptureRequest->frame_number);
    }

    if(TRUE  == m_isFlushLocked)
    {
        result = m_pPCRLock->TryLock();
        if (CamxResultSuccess != result)
        {
            UINT32 logicalCameraId = GetCameraIdfromDevice(camera3_device);
            CHX_LOG_INFO(" not able to get the lock flush enabled frame %d", pCaptureRequest->frame_number);
            HandleProcessRequestErrorAllPCRs(pCaptureRequest, logicalCameraId);
            return CDKResultSuccess;
        }
    }
    else    //in case multi camera module call this to process request, should wait
    {
        m_pRecoveryLock->Lock();
        if (TRUE == m_RecoveryInProgress)
        {
            CHX_LOG_INFO("Wait for recovery to finish, before proceeding with new request");
            m_pRecoveryCondition->Wait(m_pRecoveryLock->GetNativeHandle());
        }
        m_pRecoveryLock->Unlock();

        m_pPCRLock->Lock();
    }

    // Save the original metadata
    const camera_metadata_t* pOriginalMetadata = pCaptureRequest->settings;
    (VOID)pPriv;

    if (logicalCameraId != CDKInvalidId && NULL != m_pSelectedUsecase[logicalCameraId])
    {
        m_originalFrameWorkNumber = pCaptureRequest->frame_number;

        //Recovery happened if framework didn't send any metadata, send valid metadata
        if (m_firstFrameAfterRecovery == pCaptureRequest->frame_number &&
            NULL == pCaptureRequest->settings)
        {
            CHX_LOG_INFO("Setting Request for first frame after case =%d", m_firstFrameAfterRecovery);
            pCaptureRequest->settings = m_pLastKnownRequestSettings[logicalCameraId];
            m_firstFrameAfterRecovery = 0;
        }

        if (pCaptureRequest->output_buffers != NULL)
        {
            for (UINT i = 0; i < pCaptureRequest->num_output_buffers; i++)
            {
                if ((NULL != m_pPerfLockManager[logicalCameraId]) &&
                    (pCaptureRequest->output_buffers[i].stream->format == ChiStreamFormatBlob) &&
                    ((pCaptureRequest->output_buffers[i].stream->data_space == static_cast<android_dataspace_t>(DataspaceV0JFIF)) ||
                    (pCaptureRequest->output_buffers[i].stream->data_space == static_cast<android_dataspace_t>(DataspaceJFIF))))
                {
                      m_pPerfLockManager[logicalCameraId]->AcquirePerfLock(PERF_LOCK_SNAPSHOT_CAPTURE, 2000);
                      break;
                }

                if ((NULL != m_pPerfLockManager[logicalCameraId]) &&
                    TRUE == UsecaseSelector::IsHEIFStream(pCaptureRequest->output_buffers[i].stream))
                {
                    m_pPerfLockManager[logicalCameraId]->AcquirePerfLock(PERF_LOCK_SNAPSHOT_CAPTURE, 2000);
                    break;
                }
            }
        }
        result = m_pSelectedUsecase[logicalCameraId]->ProcessCaptureRequest(pCaptureRequest);
    }

    if (pCaptureRequest->settings != NULL)
    {
        // Restore the original metadata pointer that came from the framework
        pCaptureRequest->settings = pOriginalMetadata;
    }

    //Need to return success on PCR to allow FW to continue sending requests
    if (result == CDKResultEBusy)
    {
        result = CDKResultSuccess;
    }

    if ((result == CamxResultECancelledRequest) && (TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aFlushInProgress))))
    {
        // Ignore the Failure if flush comes while long exposure is in progress
        CHX_LOG("Flush while long exposure is in progress %d and so ignore failure", pCaptureRequest->frame_number);
        result = CDKResultSuccess;
    }

    m_pPCRLock->Unlock();
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ExtensionModule::SignalRecoveryCondition
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::SignalRecoveryCondition(
    UINT32 cameraId)
{
    if (TRUE == CheckAndSetRecovery())
    {
        CHX_LOG_CONFIG("Signaling trigger recovery for cameraId=%d", cameraId);

        m_pTriggerRecoveryLock->Lock();
        m_bTeardownRequired[cameraId]   = TRUE;
        m_IsRecoverySignaled            = TRUE;
        m_pTriggerRecoveryCondition->Signal();
        m_pTriggerRecoveryLock->Unlock();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ExtensionModule::RecoveryThread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID* ExtensionModule::RecoveryThread(
    VOID* pThreadData)
{
    PerThreadData*      pPerThreadData      = reinterpret_cast<PerThreadData*>(pThreadData);
    ExtensionModule*    pExtensionModule    = reinterpret_cast<ExtensionModule*>(pPerThreadData->pPrivateData);

    pExtensionModule->RequestThreadProcessing();

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ExtensionModule::RequestThreadProcessing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::RequestThreadProcessing()
{
    BOOL     isTeardownInProgress   = FALSE;
    Usecase* pUsecase               = NULL;

    while (TRUE)
    {
        m_pTriggerRecoveryLock->Lock();
        if (FALSE == m_IsRecoverySignaled)
        {
            m_pTriggerRecoveryCondition->Wait(m_pTriggerRecoveryLock->GetNativeHandle());
        }
        m_pTriggerRecoveryLock->Unlock();

        if (TRUE == m_terminateRecoveryThread)
        {
            CHX_LOG_VERBOSE("Terminating recovery thread");
            break;
        }

        // Find usecase to teardown if it was signaled in signalrecoverycondition
        for (UINT i = 0; i < MaxNumImageSensors; i++)
        {
            if (TRUE == m_bTeardownRequired[i])
            {
                CHX_LOG_INFO("Found usecase to teardown, cameraid: %d", i);
                pUsecase = m_pSelectedUsecase[i];

                if (NULL != pUsecase)
                {
                    CHX_LOG_INFO("Preparing for recovery, cameraId: %d", i);
                    pUsecase->PrepareForRecovery();
                    CHX_LOG_CONFIG("Triggering recovery, cameraId: %d", i);
                    TriggerRecovery(i);
                }
                else
                {
                    CHX_LOG_ERROR("Could not trigger recovery, Null usecase");
                }
                m_IsRecoverySignaled = FALSE;
            }
        }

    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::CheckAndSetRecovery
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL ExtensionModule::CheckAndSetRecovery()
{
    BOOL setRecovery = TRUE;

    // Set Recovery to true ONLY when both either teardown or HAL-flush are not in progress.
    // HAl-flush/teardown are from App/frameworks. Hence given higher priority over internal recovery.
    m_pRecoveryLock->Lock();
    if ((TRUE == m_TeardownInProgress) || (TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aFlushInProgress))))
    {
        CHX_LOG_INFO("Teardown already in progress, no need to recover");
        setRecovery = FALSE;
    }
    else
    {
        CHX_LOG_INFO("Teardown not in progress, start recovery");
        m_RecoveryInProgress = TRUE;
    }
    m_pRecoveryLock->Unlock();

    return setRecovery;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::TriggerRecovery
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::TriggerRecovery(
    UINT32 logicalCameraId)
{
    CHX_LOG_INFO("CHI override is recovering from an error, lets create case again");

    TeardownOverrideUsecase(m_logicalCameraInfo[logicalCameraId].m_pCamera3Device, TRUE);

    m_pSelectedUsecase[logicalCameraId]     = m_pUsecaseFactory->CreateUsecaseObject(&m_logicalCameraInfo[logicalCameraId],
                                                                static_cast<UsecaseId>(m_SelectedUsecaseId[logicalCameraId]),
                                                                m_pStreamConfig[logicalCameraId]);

    m_firstFrameAfterRecovery               = m_originalFrameWorkNumber + 1;
    m_bTeardownRequired[logicalCameraId]    = FALSE;

    CHX_LOG_INFO("CHI override has successfully recovered. use case is created for next request =%d",
                 m_firstFrameAfterRecovery);

    m_pRecoveryLock->Lock();
    m_pRecoveryCondition->Broadcast();
    m_RecoveryInProgress = FALSE;
    m_pRecoveryLock->Unlock();

    m_isUsecaseInBadState = FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::FreeLastKnownRequestSetting
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::FreeLastKnownRequestSetting(UINT32 logicalCameraId)
{
    if (logicalCameraId != CDKInvalidId && NULL != m_pLastKnownRequestSettings[logicalCameraId])
    {
        CHX_LOG_INFO("Freeing last known request setting");
        free_camera_metadata(m_pLastKnownRequestSettings[logicalCameraId]);
        m_pLastKnownRequestSettings[logicalCameraId] = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::OverrideFlush
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::OverrideFlush(
    const camera3_device_t*     camera3_device)
{
    CDKResult   result          = CDKResultSuccess;

    //We do not want to block flush from executing when long exposure capture is in progress
    if (FALSE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aLongExposureInProgress)))
    {
        m_pPCRLock->Lock();
        m_isFlushLocked = TRUE;
    }

    m_pRecoveryLock->Lock();
    if (TRUE == m_RecoveryInProgress)
    {
        CHX_LOG_INFO("Wait for recovery to finish, before starting flush");
        m_pRecoveryCondition->Wait(m_pRecoveryLock->GetNativeHandle());
    }
    m_hasFlushOccurred = TRUE;
    ChxUtils::AtomicStoreU32(&m_aFlushInProgress, TRUE);
    m_pRecoveryLock->Unlock();

    UINT32 logicalCameraId = GetCameraIdfromDevice(camera3_device);

    if (logicalCameraId != CDKInvalidId)
    {
        if (NULL != m_pPerfLockManager[logicalCameraId])
        {
            m_pPerfLockManager[logicalCameraId]->AcquirePerfLock(PERF_LOCK_CLOSE_CAMERA, 1000);
        }
        // If recovery fails, this may goes NULL
        if (NULL != m_pSelectedUsecase[logicalCameraId])
        {
            result = m_pSelectedUsecase[logicalCameraId]->Flush();
        }
    }

    ChxUtils::AtomicStoreU32(&m_aFlushInProgress, FALSE);
    if (m_isFlushLocked == TRUE)
    {
        m_pPCRLock->Unlock();
        m_isFlushLocked = FALSE;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::OverrideDump
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::OverrideDump(
    const camera3_device_t*     camera3_device,
    int                         fd)
{
    CDKResult   result          = CDKResultSuccess;

    UINT32 logicalCameraId = GetCameraIdfromDevice(camera3_device);

    if (logicalCameraId != CDKInvalidId)
    {
        if (NULL != m_pSelectedUsecase[logicalCameraId])
        {
            result = m_pSelectedUsecase[logicalCameraId]->Dump(fd);
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::HandleProcessRequestErrorAllPCRs
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::HandleProcessRequestErrorAllPCRs(
    camera3_capture_request_t* pRequest,
    UINT32 logicalCameraId)
{
    ChiMessageDescriptor messageDescriptor;

    messageDescriptor.messageType                            = ChiMessageTypeError;
    messageDescriptor.message.errorMessage.frameworkFrameNum = pRequest->frame_number;
    messageDescriptor.message.errorMessage.errorMessageCode  = MessageCodeRequest;
    messageDescriptor.message.errorMessage.pErrorStream      = NULL;

    ReturnFrameworkMessage((camera3_notify_msg_t*)&messageDescriptor, logicalCameraId);

    camera3_capture_result_t result = {0};
    result.frame_number             = pRequest->frame_number;
    result.result                   = NULL;
    result.num_output_buffers       = pRequest->num_output_buffers;
    result.output_buffers           = reinterpret_cast<const camera3_stream_buffer_t *>(pRequest->output_buffers);
    result.partial_result           = 0;

    for (UINT i = 0; i < pRequest->num_output_buffers; i++)
    {
        camera3_stream_buffer_t* pStreamBuffer =
            const_cast<camera3_stream_buffer_t*>(&pRequest->output_buffers[i]);
        pStreamBuffer->release_fence = -1;
        pStreamBuffer->status = 1;
    }

    if(NULL != pRequest->input_buffer)
    {
        result.input_buffer = reinterpret_cast<const camera3_stream_buffer_t *>(pRequest->input_buffer);
        camera3_stream_buffer_t* pStreamBuffer =
            const_cast<camera3_stream_buffer_t*>(pRequest->input_buffer);
        pStreamBuffer->release_fence = -1;
        pStreamBuffer->status = 1;
    }
    else
    {
        result.input_buffer = NULL;
    }

    CHX_LOG_ERROR("Sending Request error for frame %d ", pRequest->frame_number);
    ReturnFrameworkResult(reinterpret_cast<const camera3_capture_result_t*>(&result), logicalCameraId);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::CreatePipelineDescriptor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CHIPIPELINEDESCRIPTOR ExtensionModule::CreatePipelineDescriptor(
    PipelineCreateData* pPipelineCreateData) ///< Pipeline create descriptor
{
    return (g_chiContextOps.pCreatePipelineDescriptor(m_hCHIContext,
                                                      pPipelineCreateData->pPipelineName,
                                                      pPipelineCreateData->pPipelineCreateDescriptor,
                                                      pPipelineCreateData->numOutputs,
                                                      pPipelineCreateData->pOutputDescriptors,
                                                      pPipelineCreateData->numInputs,
                                                      pPipelineCreateData->pInputOptions));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::DestroyPipelineDescriptor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::DestroyPipelineDescriptor(
    CHIPIPELINEDESCRIPTOR pipelineHandle)
{
    return g_chiContextOps.pDestroyPipelineDescriptor(m_hCHIContext, pipelineHandle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::CreateSession
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CHIHANDLE ExtensionModule::CreateSession(
    SessionCreateData* pSessionCreateData) ///< Session create descriptor
{
    return g_chiContextOps.pCreateSession(m_hCHIContext,
                                          pSessionCreateData->numPipelines,
                                          pSessionCreateData->pPipelineInfo,
                                          pSessionCreateData->pCallbacks,
                                          pSessionCreateData->pPrivateCallbackData,
                                          pSessionCreateData->flags);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::DestroySession
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::DestroySession(
    CHIHANDLE sessionHandle,
    BOOL isForced)
{
    return g_chiContextOps.pDestroySession(m_hCHIContext, sessionHandle, isForced);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::Flush
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::Flush(CHIHANDLE sessionHandle)
{
    //TODO: Add recovery logic
    CHX_LOG("[E] Flushing Session Handle: %p", sessionHandle);
    return g_chiContextOps.pFlushSession(m_hCHIContext, sessionHandle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::SubmitRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::SubmitRequest(
    CHIPIPELINEREQUEST* pSubmitRequest)
{
    CDKResult result          = CDKResultSuccess;

    if (FALSE == ChxUtils::AtomicLoadU32(&m_aFlushInProgress))
    {
        result = g_chiContextOps.pSubmitPipelineRequest(m_hCHIContext, pSubmitRequest);
        if (result == CDKResultECancelledRequest)
        {
            CHX_LOG_WARN("Session returned that flush was in progress. Rewriting result as success.");
            result = CDKResultSuccess; // Ignore
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::ActivatePipeline
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::ActivatePipeline(
    CHIHANDLE             sessionHandle,
    CHIPIPELINEDESCRIPTOR pipelineHandle)
{
    return g_chiContextOps.pActivatePipeline(m_hCHIContext, sessionHandle, pipelineHandle, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::DeactivatePipeline
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::DeactivatePipeline(
    CHIHANDLE                 sessionHandle,
    CHIPIPELINEDESCRIPTOR     pipelineHandle,
    CHIDEACTIVATEPIPELINEMODE modeBitmask)
{
    return g_chiContextOps.pDeactivatePipeline(m_hCHIContext, sessionHandle, pipelineHandle, modeBitmask);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::CreateChiFence
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::CreateChiFence(
    CHIFENCECREATEPARAMS*  pInfo,
    CHIFENCEHANDLE*        phChiFence)
{
    CDKResult result = CDKResultEFailed;

    if (NULL != m_chiFenceOps.pCreateFence)
    {
        result = m_chiFenceOps.pCreateFence(m_hCHIContext, pInfo, phChiFence);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::ReleaseChiFence
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::ReleaseChiFence(
    CHIFENCEHANDLE  hChiFence)
{
    CDKResult result = CDKResultEFailed;

    if (NULL != m_chiFenceOps.pReleaseFence)
    {
        result = m_chiFenceOps.pReleaseFence(m_hCHIContext, hChiFence);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::WaitChiFenceAsync
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::WaitChiFenceAsync(
    PFNCHIFENCECALLBACK  pCallbackFn,
    CHIFENCEHANDLE       hChiFence,
    VOID*                pData)
{
    CDKResult result = CDKResultEFailed;

    if (NULL != m_chiFenceOps.pWaitFenceAsync)
    {
        result = m_chiFenceOps.pWaitFenceAsync(m_hCHIContext, pCallbackFn, hChiFence, pData);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::SignalChiFence
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::SignalChiFence(
    CHIFENCEHANDLE  hChiFence,
    CDKResult       statusResult)
{
    CDKResult result = CDKResultEFailed;

    if (NULL != m_chiFenceOps.pSignalFence)
    {
        result = m_chiFenceOps.pSignalFence(m_hCHIContext, hChiFence, statusResult);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::GetChiFenceStatus
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::GetChiFenceStatus(
    CHIFENCEHANDLE  hChiFence,
    CDKResult*      pFenceResult)
{
    CDKResult result = CDKResultEFailed;

    if (NULL != m_chiFenceOps.pGetFenceStatus)
    {
        result = m_chiFenceOps.pGetFenceStatus(m_hCHIContext, hChiFence, pFenceResult);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::QueryPipelineMetadataInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::QueryPipelineMetadataInfo(
    CHIHANDLE                   sessionHandle,
    CHIPIPELINEDESCRIPTOR       pipelineHandle,
    CHIPIPELINEMETADATAINFO*    pPipelineMetadataInfo)
{
    return g_chiContextOps.pQueryPipelineMetadataInfo(m_hCHIContext, sessionHandle, pipelineHandle, pPipelineMetadataInfo);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::EnumerateSensorModes
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::EnumerateSensorModes(
    UINT32             physCameraId,
    UINT32             numSensorModes,
    CHISENSORMODEINFO* pSensorModeInfo)
{
    return g_chiContextOps.pEnumerateSensorModes(m_hCHIContext, physCameraId, numSensorModes, pSensorModeInfo);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::GetVendorTagOps
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::GetVendorTagOps(
    CHITAGSOPS* pVendorTagOps)
{
    g_chiContextOps.pTagOps(pVendorTagOps);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::GetMetadataOps
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::GetMetadataOps(
    CHIMETADATAOPS* pMetadataOps)
{
    g_chiContextOps.pMetadataOps(pMetadataOps);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::GetAvailableRequestKeys
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::GetAvailableRequestKeys(
    UINT32   logicalCameraId,
    UINT32*  pTagList,
    UINT32   maxTagListCount,
    UINT32*  pTagCount)
{
    CDKResult result = CDKResultSuccess;

    camera_metadata_entry_t entry = { 0 };

    entry.tag = ANDROID_REQUEST_AVAILABLE_REQUEST_KEYS;

    find_camera_metadata_entry((camera_metadata_t*)(
        m_logicalCameraInfo[logicalCameraId].m_cameraInfo.static_camera_characteristics),
        entry.tag,
        &entry);

    if (maxTagListCount > entry.count)
    {
        ChxUtils::Memcpy(pTagList, entry.data.i32, entry.count * sizeof(UINT32));
        *pTagCount = entry.count;
    }
    else
    {
        result = CDKResultENeedMore;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::GetPhysicalCameraSensorModes
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::GetPhysicalCameraSensorModes(
    UINT32              physicalCameraId,
    UINT32*             pNumSensorModes,
    CHISENSORMODEINFO** ppAllSensorModes)
{
    CDKResult               result              = CDKResultEInvalidArg;
    const LogicalCameraInfo *pLogicalCameraInfo = NULL;

    if (physicalCameraId < m_numPhysicalCameras)
    {
        pLogicalCameraInfo = GetPhysicalCameraInfo(physicalCameraId);

        if ((NULL != pLogicalCameraInfo) && (NULL != pNumSensorModes) && (NULL != ppAllSensorModes))
        {
            *pNumSensorModes  = pLogicalCameraInfo->m_cameraCaps.numSensorModes;
            *ppAllSensorModes = pLogicalCameraInfo->pSensorModeInfo;
            result            = CDKResultSuccess;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::ReturnFrameworkResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::ReturnFrameworkResult(
    const camera3_capture_result_t* pResult,
    UINT32 cameraID)
{
    if ((NULL != m_pPerfLockManager[cameraID]) && (FALSE == m_firstResult))
    {
        m_pPerfLockManager[cameraID]->AcquirePerfLock(m_CurrentpowerHint);
        m_previousPowerHint = m_CurrentpowerHint;
        m_firstResult = TRUE;
    }

    if (pResult->frame_number == m_longExposureFrame)
    {
        if (pResult->num_output_buffers != 0)
        {
            CHX_LOG_INFO("Returning long exposure snapshot");
            ChxUtils::AtomicStoreU32(&m_aLongExposureInProgress, FALSE);
            m_longExposureFrame = static_cast<UINT32>(InvalidFrameNumber);
        }
    }

    m_pHALOps[cameraID]->process_capture_result(m_logicalCameraInfo[cameraID].m_pCamera3Device, pResult);

    if (pResult->output_buffers != NULL)
    {
        for (UINT i = 0; i < pResult->num_output_buffers; i++)
        {
            if ((NULL != m_pPerfLockManager[cameraID]) &&
                (pResult->output_buffers[i].stream->format == ChiStreamFormatBlob) &&
                ((pResult->output_buffers[i].stream->data_space == static_cast<android_dataspace_t>(DataspaceV0JFIF)) ||
                (pResult->output_buffers[i].stream->data_space == static_cast<android_dataspace_t>(DataspaceJFIF))))
            {
                 m_pPerfLockManager[cameraID]->ReleasePerfLock(PERF_LOCK_SNAPSHOT_CAPTURE);
                 break;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::ReturnFrameworkMessage
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::ReturnFrameworkMessage(
    const camera3_notify_msg_t* pMessage,
    UINT32 cameraID)
{
    m_pHALOps[cameraID]->notify_result(m_logicalCameraInfo[cameraID].m_pCamera3Device, pMessage);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::DumpDebugData
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::DumpDebugData(UINT32 cameraID)
{
    CHX_LOG_INFO("m_pchiStreamConfig->num_streams =  %d", m_pStreamConfig[cameraID]->num_streams);
    for (UINT32 stream = 0; stream < m_pStreamConfig[cameraID]->num_streams; stream++)
    {
        if (NULL != m_pStreamConfig[cameraID]->streams[stream])
        {
            CHX_LOG_INFO("  stream[%d] = %p - info:", stream,
                m_pStreamConfig[cameraID]->streams[stream]);
            CHX_LOG_INFO("            format       : %d",
                m_pStreamConfig[cameraID]->streams[stream]->format);
            CHX_LOG_INFO("            width        : %d",
                m_pStreamConfig[cameraID]->streams[stream]->width);
            CHX_LOG_INFO("            height       : %d",
                m_pStreamConfig[cameraID]->streams[stream]->height);
            CHX_LOG_INFO("            stream_type  : %08x",
                m_pStreamConfig[cameraID]->streams[stream]->stream_type);
            CHX_LOG_ERROR("            usage        : %08x",
                m_pStreamConfig[cameraID]->streams[stream]->usage);
            CHX_LOG_INFO("            max_buffers  : %d",
                m_pStreamConfig[cameraID]->streams[stream]->max_buffers);
            CHX_LOG_INFO("            rotation     : %08x",
                m_pStreamConfig[cameraID]->streams[stream]->rotation);
            CHX_LOG_INFO("            data_space   : %08x",
                m_pStreamConfig[cameraID]->streams[stream]->data_space);
        }
        else
        {
            CHX_LOG_ERROR("Invalid streamconfig info");
            break;
        }
    }
    CHX_LOG_INFO("  operation_mode: %d", m_pStreamConfig[cameraID]->operation_mode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::GetSelectedResolutionForActiveSensorMode
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 ExtensionModule::GetSelectedResolutionForActiveSensorMode(
    UINT32                          physCameraId,
    camera3_stream_configuration_t* pStreamConfig)
{
    CHISENSORMODEINFO* sensorMode   = UsecaseSelector::GetSensorModeInfo(physCameraId, pStreamConfig, 1);
    UINT32 selectedSensorResolution = sensorMode->frameDimension.width;

    return selectedSensorResolution;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::GetMyLogicalCameraCost
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 ExtensionModule::GetMyLogicalCameraCost(
    UINT32                          logicalId,
    camera3_stream_configuration_t* pStreamConfig)
{
    UINT32 IFEMaxWidth                     = 0;
    UINT32 numIFEsforGivenTargetTag        = 0;
    UINT32 cost                            = 0;
    CHIHANDLE         staticMetaDataHandle = const_cast<camera_metadata_t*>(
                            m_logicalCameraInfo[logicalId].m_cameraInfo.static_camera_characteristics);
    CHITAGSOPS        tagOps               = { 0 };
    UINT32            tag;

    g_chiContextOps.pTagOps(&tagOps);

    if (NULL != tagOps.pQueryVendorTagLocation)
    {
        if (CDKResultSuccess == tagOps.pQueryVendorTagLocation("org.quic.camera.MaxSingleISPCapabilities",
                "IFEMaxLineWidth", &tag))
        {
            tagOps.pGetMetaData(staticMetaDataHandle, tag, &IFEMaxWidth, sizeof(IFEMaxWidth));
        }

        if (CDKResultSuccess == tagOps.pQueryVendorTagLocation("org.quic.camera.MaxSingleISPCapabilities",
                "numIFEsforGivenTarget", &tag))
        {
            tagOps.pGetMetaData(staticMetaDataHandle, tag, &numIFEsforGivenTargetTag, sizeof(numIFEsforGivenTargetTag));

            for (UINT8 index = 0; index < m_logicalCameraInfo[logicalId].numPhysicalCameras; index++)
            {
                UINT32 cameraId                 = m_logicalCameraInfo[logicalId].ppDeviceInfo[index]->cameraId;
                UINT32 selectedSensorResolution = GetSelectedResolutionForActiveSensorMode(cameraId, pStreamConfig);
                if (selectedSensorResolution > (IFEMaxWidth))
                {
                    cost += m_singleISPResourceCost * 2; // Dual IFE
                }
                else
                {
                    cost += m_singleISPResourceCost; // Each IFE always cost 50%
                }
                if (cost > m_totalResourceBudget)
                {
                   // Limit to max since we can have more physical cameras than the number of IFEs
                   cost = m_totalResourceBudget;
                   break;
                }
            }
        }
        else
        {
            cost = m_totalResourceBudget;
        }
    }
    return cost;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::CostOfLogicalCamera
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 ExtensionModule::CostOfLogicalCamera(
    UINT32                          logicalCameraId,
    camera3_stream_configuration_t* pStreamConfig)
{
    UINT32 cost = 0;
    if (m_logicalCameraInfo[logicalCameraId].numPhysicalCameras == 0)
    {
        cost = 0; // torch
    }
    else
    {
        cost = GetMyLogicalCameraCost(logicalCameraId, pStreamConfig);
    }
    return cost;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::GetCameraIdfromDevice
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 ExtensionModule::GetCameraIdfromDevice(const camera3_device_t *pCamera3Device)
{
    UINT32 id = CDKInvalidId;

    for (UINT32 i = 0; i < MaxNumImageSensors; i++)
    {
        if ((NULL != m_logicalCameraInfo[i].m_pCamera3Device) &&
            (m_logicalCameraInfo[i].m_pCamera3Device == pCamera3Device))
        {
            id = i;
            break;
        }
    }
    return id;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::GetNumPCRsBeforeStreamOn
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 ExtensionModule::GetNumPCRsBeforeStreamOn(VOID* pMetaData) const
{
    CamxResult      result  = CamxResultSuccess;
    UINT32          metaTag = 0;
    CHITAGSOPS      m_vendorTagOps;
    static UINT8    isEarlyPCREnabledVendorTag = 0;

    // In android P the metadata pointer wil be part of session parameter. This logic will change accordingly.
    if (NULL != pMetaData)
    {
        GetInstance()->GetVendorTagOps(&m_vendorTagOps);

        result = m_vendorTagOps.pQueryVendorTagLocation("org.quic.camera.EarlyPCRenable", "EarlyPCRenable", &metaTag);

        if (CDKResultSuccess == result)
        {
            result = m_vendorTagOps.pGetMetaData(
                reinterpret_cast<camera_metadata_t*>(const_cast<VOID*>(pMetaData)),
                metaTag,
                &isEarlyPCREnabledVendorTag,
                sizeof(isEarlyPCREnabledVendorTag));

            if (CDKResultSuccess != result)
            {
                CHX_LOG_ERROR("pGetMetaData failed result %d", result);
            }
        }
        else
        {
            CHX_LOG_ERROR("pQueryVendorTagLocation failed result %d", result);
        }
    }

    if ((FALSE == isEarlyPCREnabledVendorTag) || (CDKResultSuccess != result))
    {
        CHX_LOG("EarlyPCR: chiextensionModule: EarlyPCR is disabled");
    }
    else
    {
        CHX_LOG("EarlyPCR: chiextensionModule %d", *m_pNumPCRsBeforeStreamOn);
    }

    return (FALSE == isEarlyPCREnabledVendorTag) ? 0 : *m_pNumPCRsBeforeStreamOn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::GetCameraType
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LogicalCameraType ExtensionModule::GetCameraType(UINT32 cameraId) const
{
    CHIHANDLE         staticMetaDataHandle = const_cast<camera_metadata_t*>(
            m_logicalCameraInfo[cameraId].m_cameraInfo.static_camera_characteristics);
    CHITAGSOPS        tagOps               = { 0 };
    LogicalCameraType logicalCameraType    = LogicalCameraType_Default;
    UINT32            tag;

    g_chiContextOps.pTagOps(&tagOps);

    if (CDKResultSuccess == tagOps.pQueryVendorTagLocation("org.codeaurora.qcamera3.logicalCameraType",
            "logical_camera_type", &tag))
    {
        tagOps.pGetMetaData(staticMetaDataHandle, tag, &logicalCameraType, sizeof(SBYTE));
    }

    return logicalCameraType;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::SortCameras
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ExtensionModule::SortCameras()
{
    CDKResult                   result = CDKResultSuccess;
    INT                         j      = 0;
    LogicalCameraConfiguration  temp;

    for (INT i = 1; i < static_cast<INT>(m_numOfLogicalCameraConfiguration); i++)
    {
        temp = m_pLogicalCameraConfigurationInfo[i];

        for (j = i - 1; (j >= 0) && (m_pLogicalCameraConfigurationInfo[j].logicalCameraId > temp.logicalCameraId); j--)
        {
            m_pLogicalCameraConfigurationInfo[j + 1] = m_pLogicalCameraConfigurationInfo[j];
        }

        m_pLogicalCameraConfigurationInfo[j + 1] = temp;
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtensionModule::MappingConfigSettings
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ExtensionModule::MappingConfigSettings(
    UINT32  numTokens,
    VOID*   pInputTokens)
{
    ChiOverrideToken*   pTokens = static_cast<ChiOverrideToken*>(pInputTokens);

    for (UINT i = 0; i < numTokens; i++)
    {
        switch (pTokens[i].id)
        {
            case static_cast<UINT32>(ChxSettingsToken::OverrideForceUsecaseId):
                m_pForceUsecaseId                 = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::OverrideDisableZSL):
                m_pDisableZSL                     = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::OverrideGPURotationUsecase):
                m_pGPUNodeRotateUsecase           = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::OverrideEnableMFNR):
                m_pEnableMFNRUsecase              = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::AnchorSelectionAlgoForMFNR):
                m_pAnchorSelectionAlgoForMFNR     = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::OverrideHFRNo3AUseCase):
                m_pHFRNo3AUsecase                 = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::OverrideForceSensorMode):
                m_pForceSensorMode                = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::DefaultMaxFPS):
                m_pDefaultMaxFPS                  = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::FovcEnable):
                m_pEnableFOVC                     = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::OverrideCameraClose):
                m_pOverrideCameraClose            = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::OverrideCameraOpen):
                m_pOverrideCameraOpen             = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::EISV2Enable):
                m_pEISV2Enable                    = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::EISV3Enable):
                m_pEISV3Enable                    = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::NumPCRsBeforeStreamOn):
                m_pNumPCRsBeforeStreamOn          = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::StatsProcessingSkipFactor):
                m_pStatsSkipPattern               = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::DumpDebugDataEveryProcessResult):
                m_pEnableDumpDebugData            = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::MultiCameraVREnable):
                m_pEnableMultiVRCamera            = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::OverrideGPUDownscaleUsecase):
                m_pGPUNodeDownscaleUsecase        = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::AdvanceFeatureMask):
                m_pAdvanceFeataureMask            = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::DisableASDStatsProcessing):
                m_pDisableASDProcessing           = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::MultiCameraFrameSync):
                m_pEnableMultiCameraFrameSync     = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::OutputFormat):
                m_pOutputFormat                   = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::EnableCHIPartialData):
                m_pCHIPartialDataSupport          = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::EnableFDStreamInRealTime):
                m_pFDStreamSupport                = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::SelectIHDRUsecase):
                m_pSelectIHDRUsecase              = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::EnableUnifiedBufferManager):
                m_pEnableUnifiedBufferManager     = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::EnableCHIImageBufferLateBinding):
                m_pEnableCHILateBinding           = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::EnableCHIPartialDataRecovery):
                m_pCHIEnablePartialDataRecovery   = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::UseFeatureForQCFA):
                m_pUseFeatureForQCFA              = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::EnableOfflineNoiseReprocess):
                m_pEnableOfflineNoiseReprocessing = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::EnableIHDRSnapshot):
                m_pEnableIHDRSnapshot             = &m_pConfigSettings[i];
                break;
            case static_cast<UINT32>(ChxSettingsToken::ForceEnableIHDRSnapshot):
                m_pForceEnableIHDRSnapshot        = &m_pConfigSettings[i];
                break;
            default:
                break;
        }
    }
}

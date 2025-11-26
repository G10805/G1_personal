////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxmulticamcontroller.cpp
/// @brief MultiCamController class implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "chxmulticamcontroller.h"
#include "chxzoomtranslator.h"
#include "chiifedefs.h"
#include "camxcommontypes.h"

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

#undef LOG_TAG
#define LOG_TAG "CHIMULTICAMCONTROLLER"

static const UINT32 MaxFileLen = 256;

ChiVendorTagsOps MultiCamControllerManager::s_vendorTagOps;
UINT32           MultiCamControllerManager::m_vendorTagOpticalZoomResultMeta;
UINT32           MultiCamControllerManager::m_vendorTagOpticalZoomInputMeta;
UINT32           MultiCamControllerManager::m_vendorTagBokehResultMeta;
UINT32           MultiCamControllerManager::m_vendorTagBokehInputMeta;
UINT32           MultiCamControllerManager::m_vendorTagMultiCameraRole;
UINT32           MultiCamControllerManager::m_vendorTagCropRegions;
UINT32           MultiCamControllerManager::m_vendorTagIFEAppliedCrop;
UINT32           MultiCamControllerManager::m_vendorTagSensorIFEAppliedCrop;
UINT32           MultiCamControllerManager::m_vendorTagIFEResidualCrop;
UINT32           MultiCamControllerManager::m_vendorTagLuxIndex;
UINT32           MultiCamControllerManager::m_vendorTagMasterInfo;
UINT32           MultiCamControllerManager::m_vendorTagLPMInfo;
UINT32           MultiCamControllerManager::m_vendorTagSyncInfo;
UINT32           MultiCamControllerManager::m_vendorTagCameraType;
UINT32           MultiCamControllerManager::m_vendorTagReferenceCropSize;
UINT32           MultiCamControllerManager::m_vendorTagMetadataOwner;

static const FLOAT MAX_USER_ZOOM = 8.0F;   ///< MAX digital(User) zoom

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiCamControllerManager::MultiCamControllerManager
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MultiCamControllerManager::MultiCamControllerManager()
{
    ChxUtils::Memset(&m_pController, 0, MaxNumImageSensors * sizeof(MultiCamController*));

    // Get CHI vendor tag ops
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
    OSLIBRARYHANDLE handle = ChxUtils::LibMap(pChiDriver);
    CHICONTEXTOPS   chiContextOps;
    PCHIENTRY       funcPChiEntry = reinterpret_cast<PCHIENTRY>(ChxUtils::LibGetAddr(handle, "ChiEntry"));
    if (NULL == funcPChiEntry)
    {
        CHX_LOG("Invalid pointer of ChiEntry");
        return;
    }
    funcPChiEntry(&chiContextOps);
    chiContextOps.pTagOps(&s_vendorTagOps);

    // Get the vendor tags needed by MultiCamControllers

    s_vendorTagOps.pQueryVendorTagLocation("com.qti.chi.multicameraoutputmetadata",
                                           "OutputMetadataOpticalZoom",
                                           &m_vendorTagOpticalZoomResultMeta);

    s_vendorTagOps.pQueryVendorTagLocation("com.qti.chi.multicamerainputmetadata",
                                           "InputMetadataOpticalZoom",
                                           &m_vendorTagOpticalZoomInputMeta);

    s_vendorTagOps.pQueryVendorTagLocation("com.qti.chi.multicameraoutputmetadata",
                                           "OutputMetadataBokeh",
                                           &m_vendorTagBokehResultMeta);

    s_vendorTagOps.pQueryVendorTagLocation("com.qti.chi.multicamerainputmetadata",
                                           "InputMetadataBokeh",
                                           &m_vendorTagBokehInputMeta);

    s_vendorTagOps.pQueryVendorTagLocation("com.qti.chi.multicamerainfo",
                                           "MultiCameraIdRole",
                                           &m_vendorTagMultiCameraRole);

    s_vendorTagOps.pQueryVendorTagLocation("com.qti.cropregions",
                                           "crop_regions",
                                           &m_vendorTagCropRegions);

    s_vendorTagOps.pQueryVendorTagLocation("org.quic.camera.ifecropinfo",
                                           "ResidualCrop",
                                           &m_vendorTagIFEResidualCrop);

    s_vendorTagOps.pQueryVendorTagLocation("org.quic.camera.ifecropinfo",
                                           "AppliedCrop",
                                           &m_vendorTagIFEAppliedCrop);

    s_vendorTagOps.pQueryVendorTagLocation("org.quic.camera.ifecropinfo",
                                           "SensorIFEAppliedCrop",
                                           &m_vendorTagSensorIFEAppliedCrop);

    s_vendorTagOps.pQueryVendorTagLocation("com.qti.chi.statsaec",
                                           "AecLux",
                                           &m_vendorTagLuxIndex);

    s_vendorTagOps.pQueryVendorTagLocation("com.qti.chi.multicamerainfo",
                                           "MasterCamera",
                                           &m_vendorTagMasterInfo);

    s_vendorTagOps.pQueryVendorTagLocation("com.qti.chi.multicamerainfo",
                                           "LowPowerMode",
                                           &m_vendorTagLPMInfo);

    s_vendorTagOps.pQueryVendorTagLocation("com.qti.chi.multicamerainfo",
                                           "SyncMode",
                                           &m_vendorTagSyncInfo);

    s_vendorTagOps.pQueryVendorTagLocation("org.codeaurora.qcamera3.logicalCameraType",
                                           "logical_camera_type",
                                           &m_vendorTagCameraType);

    s_vendorTagOps.pQueryVendorTagLocation("org.quic.camera2.ref.cropsize",
                                           "RefCropSize",
                                           &m_vendorTagReferenceCropSize);

    s_vendorTagOps.pQueryVendorTagLocation("com.qti.chi.metadataOwnerInfo",
                                           "MetadataOwner",
                                           &m_vendorTagMetadataOwner);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiCamControllerManager::GetController
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MultiCamController* MultiCamControllerManager::GetController(
    MccCreateData* pMccCreateData)
{
    CHX_ASSERT(NULL != pMccCreateData);
    CHX_ASSERT(pMccCreateData->logicalCameraType != LogicalCameraType_Default);
    CHX_ASSERT(pMccCreateData->logicalCameraId < MaxNumImageSensors);

    // Check if the current camera already has an active controller
    MultiCamController* pMultiCamController = m_pController[pMccCreateData->logicalCameraId];

    // If the controller type matches with the new initialize request, reconfigure the controller
    if ((NULL != pMultiCamController) &&
        (pMccCreateData->logicalCameraType == pMultiCamController->GetCameraType()))
    {
        // Reconfigure the controller
        pMultiCamController->Reconfigure(pMccCreateData);
    }
    else
    {
        // Destroy the active controller and create a new controller
        if (NULL != pMultiCamController)
        {
            pMultiCamController->Destroy();
            pMultiCamController = NULL;
        }

        switch (pMccCreateData->logicalCameraType)
        {
        case LogicalCameraType_SAT:
            pMultiCamController = MultiFovController::Create(pMccCreateData);
            break;
        case LogicalCameraType_RTB:
            pMultiCamController = MultiRTBController::Create(pMccCreateData);
            break;
        case LogicalCameraType_BAYERMONO:
            pMultiCamController = BayerMonoController::Create(pMccCreateData);
            break;
         case LogicalCameraType_VR:
            pMultiCamController = VRController::Create(pMccCreateData);
            break;
        default:
            pMultiCamController = NULL;
            CHX_LOG("Invalid controller type");
            break;
        }

        if (NULL != pMultiCamController)
        {
            m_pController[pMccCreateData->logicalCameraId] = pMultiCamController;
        }
        else
        {
            CHX_LOG("Error in creating multicamcontroller");
        }
    }

    return pMultiCamController;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiCamControllerManager::DestroyController
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiCamControllerManager::DestroyController(
    MultiCamController* pMultiCamController)
{
    if (NULL != pMultiCamController)
    {
        for (UINT32 i = 0; i < MaxNumImageSensors; ++i)
        {
            if (m_pController[i] == pMultiCamController)
            {
                m_pController[i]->Destroy();
                m_pController[i] = NULL;
                break;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiCamControllerManager::Destroy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiCamControllerManager::Destroy()
{
    for (UINT32 i = 0; i < MaxNumImageSensors; ++i)
    {
        if (NULL != m_pController[i])
        {
            m_pController[i]->Destroy();
            m_pController[i] = NULL;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiCamControllerManager::~MultiCamControllerManager
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MultiCamControllerManager::~MultiCamControllerManager()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiCamController::~MultiCamController
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MultiCamController::~MultiCamController()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiCamController::ConsolidateCameraInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiCamController::ConsolidateCameraInfo(
    LogicalCameraInfo *pLogicalCameraInfo)
{
    CDKResult result = CDKResultEFailed;
    CHX_ASSERT(NULL != pLogicalCameraInfo);

    LogicalCameraType logicalCameraType                    = pLogicalCameraInfo->logicalCameraType;
    UINT              numOfDevices                         = pLogicalCameraInfo->numPhysicalCameras;
    CHICAMERAINFO*    pConsolidatedCamInfo                 = &(pLogicalCameraInfo->m_cameraCaps);
    CHICAMERAINFO*    ppCamInfo[MaxDevicePerLogicalCamera] = { 0 };

    for (UINT32 i = 0 ; i < pLogicalCameraInfo->numPhysicalCameras ; i++)
    {
        ppCamInfo[i] = const_cast<CHICAMERAINFO*>(pLogicalCameraInfo->ppDeviceInfo[i]->m_pDeviceCaps);
    }

    if (LogicalCameraType_Default != logicalCameraType)
    {
        switch (logicalCameraType)
        {
        case LogicalCameraType_SAT:
             result = MultiFovController::ConsolidateCameraInfo(pLogicalCameraInfo);
            break;
        case LogicalCameraType_RTB:
            result = MultiRTBController::ConsolidateCameraInfo(pLogicalCameraInfo);
            break;
        case LogicalCameraType_BAYERMONO:
            result = BayerMonoController::ConsolidateCameraInfo(numOfDevices, ppCamInfo, pConsolidatedCamInfo);
            break;
        case LogicalCameraType_VR:
            result = VRController::ConsolidateCameraInfo(numOfDevices, ppCamInfo, pConsolidatedCamInfo);
            break;
        default:
            break;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiCamController::TranslateRequestSettings
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiCamController::TranslateRequestSettings(
    MulticamReqSettings* pMultiCamSettings)
{
    // If the concrete class hasn't implemented this method, return Success.
    (VOID)pMultiCamSettings;
    return CDKResultSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiCamController::Reconfigure
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiCamController::Reconfigure(
    MccCreateData* pMccCreateData)
{
    // If the concrete class hasn't implemented this method, return Success.
    (VOID)pMccCreateData;
    return CDKResultSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiCamController::TranslateFaceRegions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiCamController::TranslateFaceRegions(
    VOID* pResultMetadata)
{
    // If the concrete class hasn't implemented this method, do nothing.
    (VOID)pResultMetadata;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiCamController::TranslateResultMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiCamController::TranslateResultMetadata(
    camera_metadata_t* pResultMetadata)
{
    TranslateFaceRegions(pResultMetadata);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiCamController::ProcessResultMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiCamController::ProcessResultMetadata(
    ChiMetadata* pResultMetadata)
{
    // If the concrete class hasn't implemented this method, do nothing.
    (VOID)pResultMetadata;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiCamController::GetResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ControllerResult MultiCamController::GetResult(
    ChiMetadata* pMetadata)
{
    // If the concrete class hasn't implemented this method, return result as invalid
    (VOID)pMetadata;
    m_result.isValid = FALSE;
    return m_result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiCamController::UpdateResults
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiCamController::UpdateResults(
    ChiMetadata* pMetadata)
{
    (VOID)pMetadata;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiCamController::FillOfflinePipelineInputMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiCamController::FillOfflinePipelineInputMetadata(
    MulticamResultMetadata* pMultiCamResultMetadata,
    ChiMetadata*                pOfflineMetadata,
    BOOL                        isSnapshotPipeline)
{
    // If the concrete class hasn't implemented this method, return Success by default.
    (VOID)pMultiCamResultMetadata;
    (VOID)pOfflineMetadata;
    (VOID)isSnapshotPipeline;
    return CDKResultSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiCamController::UpdateVendorTags
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiCamController::UpdateVendorTags(
    MulticamReqSettings* pMultiCamSettings)
{
    CDKResult result = CDKResultEFailed;

    ChiMetadata* pMetadata           = pMultiCamSettings->ppSettings[0];
    ChiMetadata* pTranslatedMetadata = pMultiCamSettings->ppSettings[1];

    // Get master camera of this request
    CameraRoleType masterCameraRole = pMultiCamSettings->currentRequestMCCResult.masterCameraRole;

    // Add master camera info
    BOOL isMaster = FALSE;
    isMaster = (CameraRoleTypeWide == masterCameraRole) ? TRUE : FALSE;

    result = pMetadata->SetTag(MultiCamControllerManager::m_vendorTagMasterInfo, &isMaster, 1);

    isMaster = (CameraRoleTypeTele == masterCameraRole) ? TRUE : FALSE;

    result = pTranslatedMetadata->SetTag(MultiCamControllerManager::m_vendorTagMasterInfo, &isMaster, 1);

    CHX_LOG("masterCameraRole %d", masterCameraRole);

    // Add Sync info
    BOOL isSyncActive        = FALSE;
    BOOL isWideActive        = pMultiCamSettings->currentRequestMCCResult.activeCameras[0].isActive;
    BOOL isTeleActive        = pMultiCamSettings->currentRequestMCCResult.activeCameras[1].isActive;
    BOOL isDualZoneActive    = ((TRUE == isWideActive) && (TRUE == isTeleActive)) ? TRUE : FALSE;

    // Enable frame sync if override settings is enabled and if its in in DUAL ZONE
    if ((TRUE == pMultiCamSettings->kernelFrameSyncEnable) && (TRUE == isDualZoneActive))
    {
        isSyncActive = TRUE;
    }

    result = pMetadata->SetTag(MultiCamControllerManager::m_vendorTagSyncInfo, &isSyncActive, sizeof(SyncModeInfo));
    result = pTranslatedMetadata->SetTag(MultiCamControllerManager::m_vendorTagSyncInfo, &isSyncActive, sizeof(SyncModeInfo));
    CHX_LOG("isSyncActive %d", isSyncActive);

    // Add LPM info
    if (CDKResultSuccess == result)
    {
        LowPowerModeInfo lowPowerMode;
        lowPowerMode.isLPMEnabled               = (TRUE == isDualZoneActive) ? FALSE : TRUE;
        lowPowerMode.isSlaveOperational         = isDualZoneActive;
        lowPowerMode.lowPowerMode[0].isEnabled  = (TRUE == isWideActive) ? FALSE : TRUE;
        lowPowerMode.lowPowerMode[1].isEnabled  = (TRUE == isTeleActive) ? FALSE : TRUE;

        result = pMetadata->SetTag(
            MultiCamControllerManager::m_vendorTagLPMInfo,
            &lowPowerMode, sizeof(LowPowerModeInfo));
        result = pTranslatedMetadata->SetTag(
            MultiCamControllerManager::m_vendorTagLPMInfo,
            &lowPowerMode, sizeof(LowPowerModeInfo));

        CHX_LOG("WIDE LPM : %d , TELE LPM : %d ", lowPowerMode.lowPowerMode[0].isEnabled,
            lowPowerMode.lowPowerMode[1].isEnabled);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiCamController::ExtractCameraMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiCamController::ExtractCameraMetadata(
    ChiMetadata*    pMetadata,
    CameraMetadata* pExtractedCameraMetadata)
{
    camera_metadata_entry_t entry = { 0 };
    if (0 == pMetadata->FindTag(ANDROID_CONTROL_AF_REGIONS, &entry))
    {
        pExtractedCameraMetadata->afFocusROI.xMin   = entry.data.i32[0];
        pExtractedCameraMetadata->afFocusROI.yMin   = entry.data.i32[1];
        pExtractedCameraMetadata->afFocusROI.xMax   = entry.data.i32[2];
        pExtractedCameraMetadata->afFocusROI.yMax   = entry.data.i32[3];
        pExtractedCameraMetadata->afFocusROI.weight = entry.data.i32[4];
    }

    if (0 == pMetadata->FindTag(ANDROID_CONTROL_AF_STATE, &entry))
    {
        pExtractedCameraMetadata->afState = entry.data.u8[0];
    }

    if (0 == pMetadata->FindTag(ANDROID_SENSOR_SENSITIVITY, &entry))
    {
        pExtractedCameraMetadata->isoSensitivity = entry.data.i32[0];
    }

    if (0 == pMetadata->FindTag(ANDROID_SENSOR_EXPOSURE_TIME, &entry))
    {
        pExtractedCameraMetadata->exposureTime = entry.data.i64[0];
    }

    if (0 == pMetadata->FindTag(ANDROID_SENSOR_TIMESTAMP, &entry))
    {
        pExtractedCameraMetadata->sensorTimestamp = entry.data.i64[0];
    }

    if (0 == pMetadata->FindTag(ANDROID_STATISTICS_FACE_RECTANGLES, &entry))
    {
        UINT32 numElemsRect = sizeof(CHIRECT) / sizeof(UINT32);
        pExtractedCameraMetadata->fdMetadata.numFaces = entry.count / numElemsRect;

        UINT32 dataIndex = 0;
        for (INT32 i = 0; i < pExtractedCameraMetadata->fdMetadata.numFaces; ++i)
        {
            INT32 xMin = entry.data.i32[dataIndex++];
            INT32 yMin = entry.data.i32[dataIndex++];
            INT32 xMax = entry.data.i32[dataIndex++];
            INT32 yMax = entry.data.i32[dataIndex++];
            pExtractedCameraMetadata->fdMetadata.faceRect[i].left   = xMin;
            pExtractedCameraMetadata->fdMetadata.faceRect[i].top    = yMin;
            pExtractedCameraMetadata->fdMetadata.faceRect[i].width  = xMax - xMin + 1;
            pExtractedCameraMetadata->fdMetadata.faceRect[i].height = yMax - yMin + 1;
        }
    }

    if (0 == pMetadata->FindTag(ANDROID_STATISTICS_FACE_SCORES, &entry))
    {
        for (INT32 i = 0; i < pExtractedCameraMetadata->fdMetadata.numFaces; ++i)
        {
            pExtractedCameraMetadata->fdMetadata.faceScore[i] = entry.data.u8[i];
        }
    }

    IFECropInfo* pIfeResidualCrop = static_cast<IFECropInfo*>(pMetadata->GetTag(
        MultiCamControllerManager::m_vendorTagIFEResidualCrop));
    if (NULL != pIfeResidualCrop)
    {
        pExtractedCameraMetadata->ifeResidualCrop.left   = pIfeResidualCrop->fullPath.left;
        pExtractedCameraMetadata->ifeResidualCrop.top    = pIfeResidualCrop->fullPath.top;
        pExtractedCameraMetadata->ifeResidualCrop.width  = pIfeResidualCrop->fullPath.width;
        pExtractedCameraMetadata->ifeResidualCrop.height = pIfeResidualCrop->fullPath.height;
    }

    IFECropInfo* pIfeAppliedCrop = static_cast<IFECropInfo*>(pMetadata->GetTag(
        MultiCamControllerManager::m_vendorTagIFEAppliedCrop));
    if (NULL != pIfeAppliedCrop)
    {
        pExtractedCameraMetadata->ifeAppliedCrop.left   = pIfeAppliedCrop->fullPath.left;
        pExtractedCameraMetadata->ifeAppliedCrop.top    = pIfeAppliedCrop->fullPath.top;
        pExtractedCameraMetadata->ifeAppliedCrop.width  = pIfeAppliedCrop->fullPath.width;
        pExtractedCameraMetadata->ifeAppliedCrop.height = pIfeAppliedCrop->fullPath.height;
    }

    IFECropInfo* pSensorIFEAppliedCrop = static_cast<IFECropInfo*>(pMetadata->GetTag(
        MultiCamControllerManager::m_vendorTagSensorIFEAppliedCrop));
    if (NULL != pSensorIFEAppliedCrop)
    {
        pExtractedCameraMetadata->sensorIFEAppliedCrop.left   = pSensorIFEAppliedCrop->fullPath.left;
        pExtractedCameraMetadata->sensorIFEAppliedCrop.top    = pSensorIFEAppliedCrop->fullPath.top;
        pExtractedCameraMetadata->sensorIFEAppliedCrop.width  = pSensorIFEAppliedCrop->fullPath.width;
        pExtractedCameraMetadata->sensorIFEAppliedCrop.height = pSensorIFEAppliedCrop->fullPath.height;
    }

    FLOAT* pLux = static_cast<FLOAT*>(pMetadata->GetTag(MultiCamControllerManager::m_vendorTagLuxIndex));
    if (NULL != pLux)
    {
        pExtractedCameraMetadata->lux = pLux ? *pLux : 0.0f;
    }

    if (0 == pMetadata->FindTag(ANDROID_LENS_FOCUS_DISTANCE, &entry))
    {
        /* Unit of optimal focus distance in diopter, convert it to cm */
        if (entry.data.f[0] > 0.0f)
        {
            pExtractedCameraMetadata->focusDistCm = 100.0f / entry.data.f[0];
        }
        else
        {
            pExtractedCameraMetadata->focusDistCm = FocusDistanceCmMax;
        }
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiCamController::UpdateScalerCropForSnapshot
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiCamController::UpdateScalerCropForSnapshot(
    ChiMetadata* pMetadata)
{
    (VOID)pMetadata;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DualFovController::~DualFovController
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DualFovController::~DualFovController()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DualFovController::Destroy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID DualFovController::Destroy()
{
    if (NULL != m_pLock)
    {
        m_pLock->Destroy();
        m_pLock = NULL;
    }

    if (NULL != m_pZoomTranslator)
    {
        m_pZoomTranslator->Destroy();
        m_pZoomTranslator = NULL;
    }

    CHX_DELETE this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DualFovController::Create
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DualFovController* DualFovController::Create(
    MccCreateData* pMccCreateData)
{
    CHX_ASSERT(MaxDevicePerLogicalCamera >= pMccCreateData->numBundledCameras);

    DualFovController* pDualFovController = CHX_NEW DualFovController;

    if (NULL != pDualFovController)
    {
        pDualFovController->m_cameraType    = LogicalCameraType(pMccCreateData->logicalCameraType);

        // Parse the camera info to build the controller
        CamInfo* pCamInfoMain            = pMccCreateData->pBundledCamInfo[0];
        CamInfo* pCamInfoAux             = pMccCreateData->pBundledCamInfo[1];
        CamInfo* pCamInfoWide            = NULL;
        CamInfo* pCamInfoTele            = NULL;
        camera_info_t* pAppCameraInfoMain   = static_cast<camera_info_t*>(pCamInfoMain->pChiCamInfo->pLegacy);
        camera_info_t* pAppCameraInfoAux    = static_cast<camera_info_t*>(pCamInfoAux->pChiCamInfo->pLegacy);

        // @note: Remove the facing check after resolving Fovwider(false) case
        if ((pCamInfoMain->pChiCamInfo->lensCaps.focalLength < pCamInfoAux->pChiCamInfo->lensCaps.focalLength) ||
            (FacingFront == pAppCameraInfoAux->facing))
        {
            pDualFovController->m_isMainCamFovWider = TRUE;
            pCamInfoWide = pCamInfoMain;
            pCamInfoTele = pCamInfoAux;
        }
        else
        {
            pDualFovController->m_isMainCamFovWider = FALSE;
            pCamInfoWide = pCamInfoAux;
            pCamInfoTele = pCamInfoMain;
        }

        CHX_LOG("Main focal %f, Aux focal %f, Main camera id %d, Aux camera id %d",
            pCamInfoMain->pChiCamInfo->lensCaps.focalLength,
            pCamInfoAux->pChiCamInfo->lensCaps.focalLength,
            pCamInfoMain->camId, pCamInfoTele->camId);

        CHX_LOG("Main facing %d Aux facing %d",
            pAppCameraInfoMain->facing, pAppCameraInfoAux->facing);

        // Save wide, tele and logical camera ids
        pDualFovController->m_camIdWide    = pCamInfoWide->camId;
        pDualFovController->m_camIdTele    = pCamInfoTele->camId;
        pDualFovController->m_camIdLogical = pMccCreateData->logicalCameraId;
        pDualFovController->m_numOfDevices = pMccCreateData->numBundledCameras;
        // Get focal length, pixel size, sensor dimensions and active pixel array size
        pDualFovController->m_focalLengthWide      = pCamInfoWide->pChiCamInfo->lensCaps.focalLength;
        pDualFovController->m_focalLengthTele      = pCamInfoTele->pChiCamInfo->lensCaps.focalLength;
        pDualFovController->m_pixelPitchWide       = pCamInfoWide->pChiCamInfo->sensorCaps.pixelSize;
        pDualFovController->m_pixelPitchTele       = pCamInfoTele->pChiCamInfo->sensorCaps.pixelSize;
        pDualFovController->m_sensorDimensionWide  = pCamInfoWide->sensorOutDimension;
        pDualFovController->m_sensorDimensionTele  = pCamInfoTele->sensorOutDimension;
        pDualFovController->m_activeArraySizeWide  = pCamInfoWide->pChiCamInfo->sensorCaps.activeArray;
        pDualFovController->m_activeArraySizeTele  = pCamInfoTele->pChiCamInfo->sensorCaps.activeArray;
        pDualFovController->m_fovRectIFEWide       = pCamInfoWide->fovRectIFE;
        pDualFovController->m_fovRectIFETele       = pCamInfoTele->fovRectIFE;
        pDualFovController->m_pRawOTPData          = pCamInfoWide->pChiCamInfo->sensorCaps.pRawOTPData;
        pDualFovController->m_rawOTPDataSize       = pCamInfoWide->pChiCamInfo->sensorCaps.rawOTPDataSize;

        pDualFovController->m_isVideoStreamSelected = FALSE;
        for (UINT32 i = 0; i < pMccCreateData->pStreamConfig->numStreams; i++)
        {
            if (0 != (GRALLOC_USAGE_HW_VIDEO_ENCODER & pMccCreateData->pStreamConfig->pStreamInfo[i].usage))
            {
                pDualFovController->m_isVideoStreamSelected = TRUE;
            }
            if ((CAMERA3_STREAM_OUTPUT == pMccCreateData->pStreamConfig->pStreamInfo[i].streamType) &&
                (GRALLOC_USAGE_HW_COMPOSER == (GRALLOC_USAGE_HW_COMPOSER & pMccCreateData->pStreamConfig->pStreamInfo[i].usage)))
            {
                pDualFovController->m_previewDimensions = pMccCreateData->pStreamConfig->pStreamInfo[i].streamDimension;
            }
        }

        CDKResult result = pDualFovController->CalculateTransitionParams();

        if(CDKResultSuccess == result)
        {
            pDualFovController->m_pLock = Mutex::Create();

            if (NULL == pDualFovController->m_pLock)
            {
                result = CDKResultENoMemory;
            }
        }

        if (CDKResultSuccess == result)
        {
            // Create and initialize the zoom translator
            pDualFovController->m_pZoomTranslator = ZoomTranslator::Create();

            if (NULL != pDualFovController->m_pZoomTranslator)
            {
                ZoomTranslatorInitData zoomTranslatorInitData = { {0} };
                zoomTranslatorInitData.numLinkedSessions      = DualCamCount;
                zoomTranslatorInitData.previewDimension       = pDualFovController->m_previewDimensions;
                zoomTranslatorInitData.defaultAppCameraID     = pMccCreateData->primaryCameraId;
                zoomTranslatorInitData.maxZoom                = MAX_USER_ZOOM;

                // Wide camera Data
                zoomTranslatorInitData.linkedCameraInfo[0].adjustedFovRatio = 1.0;
                zoomTranslatorInitData.linkedCameraInfo[0].activeArraySize  = pDualFovController->m_activeArraySizeWide;
                zoomTranslatorInitData.linkedCameraInfo[0].sensorDimension  = pDualFovController->m_sensorDimensionWide;
                zoomTranslatorInitData.linkedCameraInfo[0].ifeFOVRect       = pDualFovController->m_fovRectIFEWide;
                zoomTranslatorInitData.linkedCameraInfo[0].cameraId         = pDualFovController->m_camIdWide;
                zoomTranslatorInitData.linkedCameraInfo[0].pixelSize        = pDualFovController->m_pixelPitchWide;

                // Tele camera Data
                zoomTranslatorInitData.linkedCameraInfo[1].sensorDimension  = pDualFovController->m_sensorDimensionTele;
                zoomTranslatorInitData.linkedCameraInfo[1].activeArraySize  = pDualFovController->m_activeArraySizeTele;
                zoomTranslatorInitData.linkedCameraInfo[1].ifeFOVRect       = pDualFovController->m_fovRectIFETele;
                zoomTranslatorInitData.linkedCameraInfo[1].adjustedFovRatio = pDualFovController->m_adjustedFovRatio;
                zoomTranslatorInitData.linkedCameraInfo[1].cameraId         = pDualFovController->m_camIdTele;
                zoomTranslatorInitData.linkedCameraInfo[1].pixelSize        = pDualFovController->m_pixelPitchTele;

                zoomTranslatorInitData.linkedCameraInfo[0].otpData[0].pRawOtpData    = pDualFovController->m_pRawOTPData;
                zoomTranslatorInitData.linkedCameraInfo[0].otpData[0].rawOtpDataSize = pDualFovController->m_rawOTPDataSize;
                zoomTranslatorInitData.linkedCameraInfo[0].otpData[0].refCameraId    = pDualFovController->m_camIdTele;

                zoomTranslatorInitData.linkedCameraInfo[1].otpData[0].pRawOtpData    = pDualFovController->m_pRawOTPData;
                zoomTranslatorInitData.linkedCameraInfo[1].otpData[0].rawOtpDataSize = pDualFovController->m_rawOTPDataSize;
                zoomTranslatorInitData.linkedCameraInfo[1].otpData[0].refCameraId    = pDualFovController->m_camIdWide;

                result = pDualFovController->m_pZoomTranslator->Init(&zoomTranslatorInitData);

                // Set initial result state
                pDualFovController->SetInitialResultState();
            }
            else
            {
                result = CDKResultEFailed;
            }
        }

        // Destroy the object in case of failure
        if (CDKResultSuccess != result)
        {
            pDualFovController->m_pZoomTranslator->Deinit();
            pDualFovController->Destroy();
            pDualFovController = NULL;
        }
    }

    return pDualFovController;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DualFovController::Reconfigure
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult DualFovController::Reconfigure(
    MccCreateData* pMccCreateData)
{
    CDKResult result = CDKResultSuccess;

    CHX_ASSERT(MaxDevicePerLogicalCamera >= pMccCreateData->numBundledCameras);

    // Parse the camera info to reconfigure the controller
    CamInfo* pCamInfoWide = NULL;
    CamInfo* pCamInfoTele = NULL;

    if (TRUE == m_isMainCamFovWider)
    {
        pCamInfoWide = pMccCreateData->pBundledCamInfo[0];
        pCamInfoTele = pMccCreateData->pBundledCamInfo[1];
    }
    else
    {
        pCamInfoWide = pMccCreateData->pBundledCamInfo[1];
        pCamInfoTele = pMccCreateData->pBundledCamInfo[0];
    }

    // Get sensor output dimensions
    m_sensorDimensionWide = pCamInfoWide->sensorOutDimension;
    m_sensorDimensionTele = pCamInfoTele->sensorOutDimension;

    result = CalculateTransitionParams();

    m_isVideoStreamSelected = FALSE;
    for (UINT32 i = 0; i < pMccCreateData->pStreamConfig->numStreams; i++)
    {
        if (0 != (GRALLOC_USAGE_HW_VIDEO_ENCODER & pMccCreateData->pStreamConfig->pStreamInfo[i].usage))
        {
            m_isVideoStreamSelected = TRUE;
        }
        if ((CAMERA3_STREAM_OUTPUT == pMccCreateData->pStreamConfig->pStreamInfo[i].streamType) &&
            (GRALLOC_USAGE_HW_COMPOSER == (GRALLOC_USAGE_HW_COMPOSER & pMccCreateData->pStreamConfig->pStreamInfo[i].usage)))
        {
            m_previewDimensions = pMccCreateData->pStreamConfig->pStreamInfo[i].streamDimension;
        }
    }

    if (CDKResultSuccess == result)
    {
        result = m_pZoomTranslator->Deinit();

        if (CDKResultSuccess == result)
        {
            // Initialize the zoom translator
            ZoomTranslatorInitData zoomTranslatorInitData = { {0} };
            zoomTranslatorInitData.numLinkedSessions      = DualCamCount;
            zoomTranslatorInitData.previewDimension       = m_previewDimensions;
            zoomTranslatorInitData.defaultAppCameraID     = pMccCreateData->primaryCameraId;

            // Wide camera Data
            zoomTranslatorInitData.linkedCameraInfo[0].adjustedFovRatio = 1.0;
            zoomTranslatorInitData.linkedCameraInfo[0].activeArraySize  = m_activeArraySizeWide;
            zoomTranslatorInitData.linkedCameraInfo[0].sensorDimension  = m_sensorDimensionWide;
            zoomTranslatorInitData.linkedCameraInfo[0].ifeFOVRect       = m_fovRectIFEWide;

            zoomTranslatorInitData.linkedCameraInfo[1].sensorDimension  = m_sensorDimensionTele;
            zoomTranslatorInitData.linkedCameraInfo[1].activeArraySize  = m_activeArraySizeTele;
            zoomTranslatorInitData.linkedCameraInfo[1].ifeFOVRect       = m_fovRectIFETele;
            zoomTranslatorInitData.linkedCameraInfo[1].adjustedFovRatio = m_adjustedFovRatio;

            // Tele camera Data
            zoomTranslatorInitData.linkedCameraInfo[0].otpData[0].pRawOtpData    = m_pRawOTPData;
            zoomTranslatorInitData.linkedCameraInfo[0].otpData[0].rawOtpDataSize = m_rawOTPDataSize;
            zoomTranslatorInitData.linkedCameraInfo[0].otpData[0].refCameraId    = m_camIdTele;

            zoomTranslatorInitData.linkedCameraInfo[1].otpData[0].pRawOtpData    = m_pRawOTPData;
            zoomTranslatorInitData.linkedCameraInfo[1].otpData[0].rawOtpDataSize = m_rawOTPDataSize;
            zoomTranslatorInitData.linkedCameraInfo[1].otpData[0].refCameraId    = m_camIdWide;

            result = m_pZoomTranslator->Init(&zoomTranslatorInitData);
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DualFovController::CalculateTransitionParams
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult DualFovController::CalculateTransitionParams()
{
    CDKResult result = CDKResultEFailed;

    FLOAT fovWide = m_sensorDimensionWide.width * m_pixelPitchWide / m_focalLengthWide;
    FLOAT fovTele = m_sensorDimensionTele.width * m_pixelPitchTele / m_focalLengthTele;

    if (fovTele > 0.0f)
    {
        m_adjustedFovRatio = fovWide / fovTele;

        // Calculate the transition parameters
        m_transitionTeleToWide = m_adjustedFovRatio;
        m_transitionWideToTele = m_transitionTeleToWide + m_adjustedFovRatio * PercentMarginHysterisis;
        m_transitionLow        = m_transitionWideToTele - m_adjustedFovRatio * PercentMarginTransitionZone;
        m_transitionHigh       = m_transitionTeleToWide + m_adjustedFovRatio * PercentMarginTransitionZone;

        // Adjust transitionLow and transitionHigh based on fusion thresholds
        m_transitionLow  = (SNAPSHOT_FUSION_ZOOM_MIN < m_transitionLow)  ? SNAPSHOT_FUSION_ZOOM_MIN : m_transitionLow;
        m_transitionHigh = (SNAPSHOT_FUSION_ZOOM_MAX > m_transitionHigh) ? SNAPSHOT_FUSION_ZOOM_MAX : m_transitionHigh;

        CHX_LOG("transition param: low : %f", m_transitionLow);
        CHX_LOG("transition param: t2w : %f", m_transitionTeleToWide);
        CHX_LOG("transition param: w2t : %f", m_transitionWideToTele);
        CHX_LOG("transition param: high: %f", m_transitionHigh);
        CHX_LOG("fovWide: %f fovTele: %f m_adjustedFovRatio %f", fovWide, fovTele, m_adjustedFovRatio);

        result = CDKResultSuccess;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DualFovController::SetInitialResultState
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID DualFovController::SetInitialResultState()
{
    m_result.activeCameras[0].cameraId = m_camIdWide;
    m_result.activeCameras[1].cameraId = m_camIdTele;

    if (m_zoomUser <= m_transitionWideToTele)
    {
        m_result.masterCameraId            = m_camIdWide;
        m_result.masterCameraRole          = CameraRoleTypeWide;
        m_recommendedMasterCameraRole      = CameraRoleTypeWide;
        m_result.activeCameras[0].isActive = TRUE;
        m_result.activeCameras[1].isActive = FALSE;
    }
    else
    {
        m_result.masterCameraId            = m_camIdTele;
        m_result.masterCameraRole          = CameraRoleTypeTele;
        m_recommendedMasterCameraRole      = CameraRoleTypeTele;
        m_result.activeCameras[0].isActive = FALSE;
        m_result.activeCameras[1].isActive = TRUE;
    }

    m_result.snapshotFusion = FALSE;
    // Activate both sensors if LPM is disabled

    if (FALSE == ENABLE_LPM)
    {
        m_result.activeCameras[0].isActive = TRUE;
        m_result.activeCameras[1].isActive = TRUE;
    }
    for (UINT32 i = 0 ; i < m_numOfDevices ; i++)
    {
        if (m_result.activeCameras[i].isActive)
        {
            m_result.activeMap |= 1 << i;
        }
    }

    if (TRUE == m_isVideoStreamSelected)
    {
        m_result.snapshotFusion = FALSE;
    }
    else
    {
        m_result.snapshotFusion = TRUE;
    }
    m_result.isValid        = TRUE;

    CHX_LOG("Active cameras Wide[%d], Tele[%d]", m_result.activeCameras[0].isActive, m_result.activeCameras[1].isActive);
    CHX_LOG("Master camera id: %d fusion: %d", m_result.masterCameraId, m_result.snapshotFusion);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DualFovController::UpdateResults
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VOID DualFovController::UpdateResults(
    ChiMetadata* pMetadata)
{
    if (NULL != pMetadata)
    {
        camera_metadata_entry_t entryCropRegion = { 0 };
        if (0 == pMetadata->FindTag(ANDROID_SCALER_CROP_REGION, &entryCropRegion))
        {
            CHIRECT userZoom;
            userZoom.left   = entryCropRegion.data.i32[0];
            userZoom.top    = entryCropRegion.data.i32[1];
            userZoom.width  = entryCropRegion.data.i32[2];
            userZoom.height = entryCropRegion.data.i32[3];

            m_zoomUser = m_activeArraySizeWide.width / (FLOAT)userZoom.width;
        }

        SetInitialResultState();
    }
    CHX_LOG("Updated Active cameras Wide[%d], Tele[%d]", m_result.activeCameras[0].isActive, m_result.activeCameras[1].isActive);
    CHX_LOG("Updated Master camera id: %d fusion: %d", m_result.masterCameraId, m_result.snapshotFusion);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DualFovController::ConsolidateCameraInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult DualFovController::ConsolidateCameraInfo(
    UINT32            numBundledCameras,
    CHICAMERAINFO**   ppCamInfo,
    CHICAMERAINFO*    pConsolidatedCamInfo)
{
    CHX_ASSERT(NULL != ppCamInfo);
    CHX_ASSERT(NULL != pConsolidatedCamInfo);
    CHX_ASSERT(MaxDevicePerLogicalCamera >= numBundledCameras);
    CHX_ASSERT(NULL != ppCamInfo[0]);
    CHX_ASSERT(NULL != ppCamInfo[1]);

    CDKResult result = CDKResultSuccess;

    camera_info_t* pCameraInfoMain = static_cast<camera_info_t*>(ppCamInfo[0]->pLegacy);
    const camera_metadata_t* pMetadataMain = pCameraInfoMain->static_camera_characteristics;

    camera_info_t* pCameraInfoAux = static_cast<camera_info_t*>(ppCamInfo[1]->pLegacy);
    const camera_metadata_t* pMetadataAux = pCameraInfoAux->static_camera_characteristics;

    camera_info_t* pCameraInfoConsolidated = static_cast<camera_info_t*>(pConsolidatedCamInfo->pLegacy);
    const camera_metadata_t* pMetadataConsolidated = pCameraInfoConsolidated->static_camera_characteristics;

    pConsolidatedCamInfo->lensCaps       = ppCamInfo[0]->lensCaps;
    pConsolidatedCamInfo->numSensorModes = ppCamInfo[0]->numSensorModes;
    pConsolidatedCamInfo->sensorCaps     = ppCamInfo[0]->sensorCaps;
    ChxUtils::Memcpy(pConsolidatedCamInfo->pLegacy, ppCamInfo[0]->pLegacy, sizeof(camera_info_t));
    pCameraInfoConsolidated->static_camera_characteristics = pMetadataConsolidated;

    pConsolidatedCamInfo->size = ppCamInfo[0]->size;

    if ((NULL != pMetadataMain) && (NULL != pMetadataAux))
    {
        camera_metadata_entry_t entryMain = { 0 };
        camera_metadata_entry_t entryAux  = { 0 };
        camera_metadata_entry_t entryConsolidated = { 0 };

        // update available stream configure for consolidate metadata
        // this is dual camera usecase. So availabe stream configure should be
        // compilation of main and aux camera.
        if ((0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataMain),
                ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entryMain)) &&
            (0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataAux),
                ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entryAux)) &&
            (0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated),
                ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entryConsolidated)))
        {
            UINT32 consolidatedEntryCount = 0;
            // 4 elements in the tuple of ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS
            UINT32 tupleSize      = 4;
            UINT32 numEntriesMain = entryMain.count / tupleSize;
            UINT32 numEntriesAux  = entryAux.count / tupleSize;

            INT32* pEntryData     = static_cast<INT32*>(ChxUtils::Calloc(
                (numEntriesAux + numEntriesMain) * tupleSize * sizeof(INT32)));
            if (NULL != pEntryData)
            {
                ChxUtils::Memcpy(pEntryData, entryMain.data.i32, entryMain.count * sizeof(INT32));
                consolidatedEntryCount = numEntriesMain;

                for (UINT32 j = 0; j < numEntriesAux; j++)
                {
                    UINT32 auxIndex  = j * tupleSize;
                    BOOL matchFound  = FALSE;

                    for (UINT32 i = 0; i < numEntriesMain; i++)
                    {
                        UINT32 mainIndex = i * tupleSize;
                        if ((entryMain.data.i32[mainIndex]     == entryAux.data.i32[auxIndex])     &&
                            (entryMain.data.i32[mainIndex + 1] == entryAux.data.i32[auxIndex + 1]) &&
                            (entryMain.data.i32[mainIndex + 2] == entryAux.data.i32[auxIndex + 2]) &&
                            (entryMain.data.i32[mainIndex + 3] == entryAux.data.i32[auxIndex + 3]))
                        {
                            matchFound = TRUE;
                            break;
                        }
                    }

                    // if this stream configure is not in main metadata, add it to consolidate metadata.
                    if (FALSE == matchFound)
                    {
                        UINT32 consolidatedIndex          = consolidatedEntryCount * tupleSize;

                        pEntryData[consolidatedIndex]     = entryAux.data.i32[auxIndex];
                        pEntryData[consolidatedIndex + 1] = entryAux.data.i32[auxIndex + 1];
                        pEntryData[consolidatedIndex + 2] = entryAux.data.i32[auxIndex + 2];
                        pEntryData[consolidatedIndex + 3] = entryAux.data.i32[auxIndex + 3];

                        consolidatedEntryCount++;
                    }
                }

                entryConsolidated.count = consolidatedEntryCount * tupleSize;

                if (0 != update_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated), entryConsolidated.index,
                        pEntryData, entryConsolidated.count, &entryConsolidated))
                {
                    CHX_LOG_WARN("update availablestreamconfigure for consolidate metadata failed!");
                }

                ChxUtils::Free(pEntryData);
                pEntryData = NULL;
            }
            else
            {
                CHX_LOG("No memory allocated for pEntryData");
                result = CDKResultENoMemory;
            }
        }
    }

    if ((NULL != pMetadataMain) && (NULL != pMetadataAux))
    {
        camera_metadata_entry_t entryMain = { 0 };
        camera_metadata_entry_t entryAux  = { 0 };
        camera_metadata_entry_t entryConsolidated = { 0 };
        if ((0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataMain),
                ANDROID_JPEG_MAX_SIZE, &entryMain)) &&
            (0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataAux),
                ANDROID_JPEG_MAX_SIZE, &entryAux)) &&
            (0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated),
                ANDROID_JPEG_MAX_SIZE, &entryConsolidated)))
        {
            INT32 maxJPEGSizeOfMain = *entryMain.data.i32;
            INT32 maxJPEGSizeOfAux  = *entryAux.data.i32;
            INT32 maxJPEGSize       = maxJPEGSizeOfMain;
            // we need to use big JPEG size for consolidate metadata
            if (maxJPEGSizeOfAux > maxJPEGSizeOfMain)
            {
                maxJPEGSize = maxJPEGSizeOfAux;
            }

            if (0 != update_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated),
                                                    entryConsolidated.index,
                                                    &maxJPEGSize,
                                                    entryConsolidated.count,
                                                    &entryConsolidated))
            {
                CHX_LOG_WARN("update MAXJPEGSize metadata for consolidate metadata failed!");
            }

        }

    }

    if ((NULL != pMetadataMain) && (NULL != pMetadataAux))
    {
        camera_metadata_entry_t entryMain = { 0 };
        camera_metadata_entry_t entryAux  = { 0 };
        camera_metadata_entry_t entryConsolidated = { 0 };
        if ((0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataMain),
                ANDROID_LENS_FACING, &entryMain)) &&
            (0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataAux),
                ANDROID_LENS_FACING, &entryAux)) &&
            (0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated),
                ANDROID_LENS_FACING, &entryConsolidated)))
        {
            // Use front face as consolidate metadata face,
            // app needs the aux info ( front facing ) to select the logical camera
            INT32 iLensFace = entryAux.data.i32[0];
            if (0 != update_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated), entryConsolidated.index,
                &iLensFace, entryConsolidated.count, &entryConsolidated))
            {
                CHX_LOG_WARN("update lence face metadata for consolidate metadata failed!");
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DualFovController::TranslateRequestSettings
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult DualFovController::TranslateRequestSettings(
    MulticamReqSettings* pMultiCamSettings)
{
    CDKResult result = CDKResultEFailed;

    if (NULL != pMultiCamSettings)
    {
        CHX_ASSERT(MaxDevicePerLogicalCamera >= pMultiCamSettings->numSettingsBuffers);

        ChiMetadata* pMetadata           = pMultiCamSettings->ppSettings[0];
        ChiMetadata* pTranslatedMetadata = pMultiCamSettings->ppSettings[1];

        if ((NULL != pMetadata) && (NULL != pTranslatedMetadata))
        {
            // Translate the reference crop window if app set this vendor tag for wide
            RefCropWindowSize refCropWindowWide = { 0 };
            RefCropWindowSize refCropWindowTele = { 0 };
            UINT32            refCropSizeTag    = 0;

            result = MultiCamControllerManager::s_vendorTagOps.pQueryVendorTagLocation(
                "org.quic.camera2.ref.cropsize", "RefCropSize", &refCropSizeTag);
            if (CDKResultSuccess == result)
            {
                RefCropWindowSize* pCropWindow = static_cast<RefCropWindowSize*>(pMetadata->GetTag(refCropSizeTag));
                if (NULL != pCropWindow)
                {
                    refCropWindowWide = *pCropWindow;
                }

                if ((CDKResultSuccess == result) && (refCropWindowWide.width > 0) && (refCropWindowWide.height > 0))
                {
                    refCropWindowTele.width  =
                        refCropWindowWide.width * m_activeArraySizeTele.width / m_activeArraySizeWide.width;
                    refCropWindowTele.height =
                        refCropWindowWide.height * m_activeArraySizeTele.height / m_activeArraySizeWide.height;
                    CHX_LOG("Translate RefCropSize Wide w:%d h:%d Tele w:%d h:%d", refCropWindowWide.width,
                        refCropWindowWide.height, refCropWindowTele.width, refCropWindowTele.height);
                    result = pTranslatedMetadata->SetTag(refCropSizeTag,
                        &refCropWindowTele, sizeof(refCropWindowTele));
                }
            }
            // Translate the zoom crop window
            camera_metadata_entry_t entryCropRegion      = { 0 };
            camera_metadata_entry_t entryCropRegionTrans = { 0 };
            if (0 == pMetadata->FindTag(ANDROID_SCALER_CROP_REGION, &entryCropRegion))
            {
                TranslatedZoom translatedZoom;
                ChxUtils::Memset(&translatedZoom, 0, sizeof(translatedZoom));

                CHIRECT userZoom;
                userZoom.left   = entryCropRegion.data.i32[0];
                userZoom.top    = entryCropRegion.data.i32[1];
                userZoom.width  = entryCropRegion.data.i32[2];
                userZoom.height = entryCropRegion.data.i32[3];

                if (CDKResultSuccess == m_pZoomTranslator->GetTranslatedZoom(&userZoom, &translatedZoom))
                {
                    // Update the zoom value for wide camera
                    ZoomRegions zoomPreview = translatedZoom.zoomPreview;
                    pMetadata->SetTag(ANDROID_SCALER_CROP_REGION,
                        &zoomPreview.totalZoom[0], 4);

                    /* Update the vendor tag for the CropRegions containing
                    user crop, pipeline crop and IFE crop limit */
                    CaptureRequestCropRegions cropRegionsWide;
                    CaptureRequestCropRegions cropRegionsTele;

                    cropRegionsWide.userCropRegion     = userZoom;
                    cropRegionsWide.pipelineCropRegion = zoomPreview.totalZoom[0];
                    cropRegionsWide.ifeLimitCropRegion = zoomPreview.ispZoom[0];

                    cropRegionsTele.userCropRegion     = userZoom;
                    cropRegionsTele.pipelineCropRegion = zoomPreview.totalZoom[1];
                    cropRegionsTele.ifeLimitCropRegion = zoomPreview.ispZoom[1];

                    CHX_LOG("inputcropregion:wide:%d,%d,%d,%d",zoomPreview.totalZoom[0].left,
                        zoomPreview.totalZoom[0].top,zoomPreview.totalZoom[0].width,zoomPreview.totalZoom[0].height);
                    ChxUtils::SetVendorTagValue(pMetadata, CropRegions,
                        sizeof(CaptureRequestCropRegions), &cropRegionsWide);
                    ChxUtils::SetVendorTagValue(pTranslatedMetadata, CropRegions,
                        sizeof(CaptureRequestCropRegions), &cropRegionsTele);

                    m_zoomWide = m_activeArraySizeWide.width / (FLOAT)translatedZoom.zoomSnapshot.totalZoom[0].width;
                    m_zoomTele = m_activeArraySizeTele.width / (FLOAT)translatedZoom.zoomSnapshot.totalZoom[1].width;
                    m_zoomUser = m_activeArraySizeWide.width / (FLOAT)userZoom.width;

                    CHX_LOG("Transalate Settings Zoom User: %.2f, Zoom Wide: %.2f, Zoom Tele: %.2f",
                        m_zoomUser, m_zoomWide, m_zoomTele);

                    if (0 == pTranslatedMetadata->FindTag(ANDROID_SCALER_CROP_REGION, &entryCropRegionTrans))
                    {
                        pTranslatedMetadata->SetTag(ANDROID_SCALER_CROP_REGION,
                            &zoomPreview.totalZoom[1], 4);
                        CHX_LOG("inputcropregion:tele:%d,%d,%d,%d",zoomPreview.totalZoom[1].left,
                                zoomPreview.totalZoom[1].top,zoomPreview.totalZoom[1].width,zoomPreview.totalZoom[1].height);

                    }
                }
            }

            // Translate the Focus ROI
            camera_metadata_entry_t entryAFRegionMain = { 0 };
            camera_metadata_entry_t entryAFRegionAux  = { 0 };

            if ((0 == pMetadata->FindTag(ANDROID_CONTROL_AF_REGIONS, &entryAFRegionMain)) &&
                (0 == pTranslatedMetadata->FindTag(ANDROID_CONTROL_AF_REGIONS, &entryAFRegionAux)))
            {
                // AF_REGION tag tuple has 5 elements [xMin, yMin, xMax, yMax, weight]
                UINT32 tupleSize = 5;
                for (UINT32 i = 0; i < entryAFRegionMain.count / tupleSize; i++)
                {
                    WeightedRegion afRegion = { 0 };
                    UINT32 index = i * tupleSize;

                    afRegion.xMin   = entryAFRegionMain.data.i32[index];
                    afRegion.yMin   = entryAFRegionMain.data.i32[index + 1];
                    afRegion.xMax   = entryAFRegionMain.data.i32[index + 2];
                    afRegion.yMax   = entryAFRegionMain.data.i32[index + 3];
                    afRegion.weight = entryAFRegionMain.data.i32[index + 4];

                    // Get the translated AF ROI for the wide camera
                    WeightedRegion afRegionTrans = TranslateMeteringRegion(&afRegion, CameraRoleTypeWide);

                    // Update the metadata
                    pMetadata->SetTag(ANDROID_CONTROL_AF_REGIONS,
                        &afRegionTrans, (sizeof(WeightedRegion) / sizeof(INT32)));

                    // Get the translated AF ROI for the tele camera
                    afRegionTrans = TranslateMeteringRegion(&afRegion, CameraRoleTypeTele);

                    // Update the metadata
                    pTranslatedMetadata->SetTag(ANDROID_CONTROL_AF_REGIONS,
                        &afRegionTrans, (sizeof(WeightedRegion) / sizeof(INT32)));
                }
            }

            // Translate the Metering ROI
            camera_metadata_entry_t entryAERegionMain = { 0 };
            camera_metadata_entry_t entryAERegionAux  = { 0 };

            if ((0 == pMetadata->FindTag(ANDROID_CONTROL_AE_REGIONS, &entryAERegionMain)) &&
                (0 == pTranslatedMetadata->FindTag(ANDROID_CONTROL_AE_REGIONS, &entryAERegionAux)))
            {
                // AE_REGION tag tuple has 5 elements [xMin, yMin, xMax, yMax, weight]
                UINT32 tupleSize = 5;
                for (UINT32 i = 0; i < entryAERegionMain.count / tupleSize; i++)
                {
                    WeightedRegion aeRegion = { 0 };
                    UINT32 index = i * tupleSize;

                    aeRegion.xMin   = entryAERegionMain.data.i32[index];
                    aeRegion.yMin   = entryAERegionMain.data.i32[index + 1];
                    aeRegion.xMax   = entryAERegionMain.data.i32[index + 2];
                    aeRegion.yMax   = entryAERegionMain.data.i32[index + 3];
                    aeRegion.weight = entryAERegionMain.data.i32[index + 4];

                    // Get the translated AE ROI for the wide camera
                    WeightedRegion aeRegionTrans = TranslateMeteringRegion(&aeRegion, CameraRoleTypeWide);

                    // Update the metadata
                    pMetadata->SetTag(ANDROID_CONTROL_AE_REGIONS,
                        &aeRegionTrans, (sizeof(WeightedRegion) / sizeof(INT32)));

                    // Get the translated AE ROI for the tele camera
                    aeRegionTrans = TranslateMeteringRegion(&aeRegion, CameraRoleTypeTele);

                    // Update the metadata
                    pTranslatedMetadata->SetTag(ANDROID_CONTROL_AE_REGIONS,
                        &aeRegionTrans, (sizeof(WeightedRegion) / sizeof(INT32)));
                }
            }

            // Get master camera of this request
            CameraRoleType masterCameraRole = pMultiCamSettings->currentRequestMCCResult.masterCameraRole;

            // Send the camera role and camera id for the first frame.
            if (0 == pMultiCamSettings->frameNum)
            {
                // Set the MultiCameraIdRole for the main camera
                MultiCameraIdRole inputMetadata;
                inputMetadata.currentCameraId   = m_isMainCamFovWider ? m_camIdWide : m_camIdTele;
                inputMetadata.currentCameraRole = m_isMainCamFovWider ? CameraRoleTypeWide : CameraRoleTypeTele;
                inputMetadata.logicalCameraId   = m_camIdLogical;
                inputMetadata.masterCameraRole  = masterCameraRole;

                result = pMetadata->SetTag(MultiCamControllerManager::m_vendorTagMultiCameraRole, &inputMetadata, sizeof(MultiCameraIdRole));

                 CHX_LOG("currentCameraId %d Camera role %d", inputMetadata.currentCameraId, inputMetadata.currentCameraRole);

                // Set the MultiCameraIdRole for the aux camera
                inputMetadata.currentCameraId   = m_isMainCamFovWider ? m_camIdTele : m_camIdWide;
                inputMetadata.currentCameraRole = m_isMainCamFovWider ? CameraRoleTypeTele : CameraRoleTypeWide;
                inputMetadata.logicalCameraId   = m_camIdLogical;
                inputMetadata.masterCameraRole  = masterCameraRole;

                result = pTranslatedMetadata->SetTag(MultiCamControllerManager::m_vendorTagMultiCameraRole, &inputMetadata, sizeof(MultiCameraIdRole));

                CHX_LOG("currentCameraId %d Camera role %d", inputMetadata.currentCameraId, inputMetadata.currentCameraRole);
            }

            // Update all the common vendor tags
            UpdateVendorTags(pMultiCamSettings);

            result = CDKResultSuccess;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DualFovController::ProcessResultMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID DualFovController::ProcessResultMetadata(
    ChiMetadata* pResultMetadata)
{
    OutputMetadataOpticalZoom* pMetadataOpticalZoom;
    MultiCameraIdRole* pMultiCameraRole;

    m_pLock->Lock();

    if (NULL != (pMetadataOpticalZoom = static_cast<OutputMetadataOpticalZoom*>(pResultMetadata->GetTag(
        MultiCamControllerManager::m_vendorTagOpticalZoomResultMeta))))
    {
        CHX_LOG("Current master: %d Recommended master: %d", pMetadataOpticalZoom->masterCamera,
            pMetadataOpticalZoom->recommendedMasterCamera);

        // Update the master camera info
        m_result.masterCameraId   = (CameraRoleTypeWide == pMetadataOpticalZoom->recommendedMasterCamera) ?
            m_camIdWide : m_camIdTele;
        m_result.masterCameraRole = pMetadataOpticalZoom->recommendedMasterCamera;

        m_recommendedMasterCameraRole = pMetadataOpticalZoom->recommendedMasterCamera;

        // Update LPM info
        m_result.activeCameras[0].cameraId = m_camIdWide;
        m_result.activeCameras[1].cameraId = m_camIdTele;

        if (CameraRoleTypeWide == pMetadataOpticalZoom->lowPowerMode[0].cameraRole)
        {
            m_result.activeCameras[0].isActive = pMetadataOpticalZoom->lowPowerMode[0].isEnabled ? 0 : 1;
            m_result.activeCameras[1].isActive = pMetadataOpticalZoom->lowPowerMode[1].isEnabled ? 0 : 1;
        }
        else
        {
            m_result.activeCameras[0].isActive = pMetadataOpticalZoom->lowPowerMode[1].isEnabled ? 0 : 1;
            m_result.activeCameras[1].isActive = pMetadataOpticalZoom->lowPowerMode[0].isEnabled ? 0 : 1;
        }

        if (FALSE == ENABLE_LPM)
        {
            m_result.activeCameras[0].isActive = TRUE;
            m_result.activeCameras[1].isActive = TRUE;
        }

        // Never put the master into LPM
        if (CameraRoleTypeWide == m_recommendedMasterCameraRole && FALSE == m_result.activeCameras[0].isActive)
        {
            CHX_LOG("Wide is master and inactive, setting wide to active");
            m_result.activeCameras[0].isActive = TRUE;
        }
        else if (CameraRoleTypeTele == m_recommendedMasterCameraRole && FALSE == m_result.activeCameras[1].isActive)
        {
            CHX_LOG("Tele is master and inactive, setting tele to active");
            m_result.activeCameras[1].isActive = TRUE;
        }

        if ((TRUE == m_result.activeCameras[0].isActive) && (TRUE == m_result.activeCameras[1].isActive) &&
            (TRUE != m_isVideoStreamSelected))
        {
            m_result.snapshotFusion = TRUE;
        }
        else
        {
            m_result.snapshotFusion = FALSE;
        }

        // Update spatial alignement pixel shift
        INT32 horzShiftPreview  = pMetadataOpticalZoom->outputShiftPreview.horizonalShift;
        INT32 vertShiftPreview  = pMetadataOpticalZoom->outputShiftPreview.verticalShift;
        INT32 horzShiftSnapshot = pMetadataOpticalZoom->outputShiftSnapshot.horizonalShift;
        INT32 vertShiftSnapshot = pMetadataOpticalZoom->outputShiftSnapshot.verticalShift;

        m_pixelShiftWidePreview  = { horzShiftPreview, vertShiftPreview };
        m_pixelShiftWideSnapshot = { horzShiftSnapshot, vertShiftSnapshot };

        m_result.isValid = TRUE;

        CHX_LOG("Zoom User: %.2f, Zoom Wide: %.2f, Zoom Tele: %.2f", m_zoomUser, m_zoomWide, m_zoomTele);
        CHX_LOG("Active cameras Wide[%d], Tele[%d]", m_result.activeCameras[0].isActive, m_result.activeCameras[1].isActive);
        CHX_LOG("Master camera id: %d, Fusion: %d", m_result.masterCameraId, m_result.snapshotFusion);
    }
    // Read LUX index and object focus distance for master camera
    else if (NULL != (pMultiCameraRole = static_cast<MultiCameraIdRole*>(pResultMetadata->GetTag(
        MultiCamControllerManager::m_vendorTagMultiCameraRole))))
    {
        if (m_recommendedMasterCameraRole == pMultiCameraRole->currentCameraRole)
        {
            FLOAT* pLux = static_cast<FLOAT*>(pResultMetadata->GetTag(
                    MultiCamControllerManager::m_vendorTagLuxIndex));
            if (NULL != pLux)
            {
                m_currentLuxIndex = *pLux;
            }
            m_currentFocusDistCm = 100;
            // Update the fusion enable flag
            if ( ENABLE_SNAPSHOT_FUSION                                  &&
                (m_zoomUser >= SNAPSHOT_FUSION_ZOOM_MIN)                 &&
                (m_zoomUser <= SNAPSHOT_FUSION_ZOOM_MAX)                 &&
                (m_currentLuxIndex >= SNAPSHOT_FUSION_LUX_IDX_THRESHOLD) &&
                (m_currentFocusDistCm >= SNAPSHOT_FUSION_FOCUS_DIST_CM_MIN))
            {
                m_result.snapshotFusion = TRUE;
            }
            else
            {
                m_result.snapshotFusion = FALSE;
            }
        }
    }

    m_pLock->Unlock();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DualFovController::GetResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ControllerResult DualFovController::GetResult(
    ChiMetadata* pMetadata)
{
    (VOID)pMetadata;
    m_pLock->Lock();
    ControllerResult result = m_result;

    m_result.activeMap = 0;
    for (UINT32 i = 0 ; i < m_numOfDevices ; i++)
    {
        if (m_result.activeCameras[i].isActive)
        {
            m_result.activeMap |= 1 << i;
        }
    }

    m_pLock->Unlock();

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DualFovController::TranslateMeteringRegion
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL DualFovController::isFusionEnabled()
{
    m_pLock->Lock();
    BOOL isFusionEnabled = m_result.snapshotFusion;
    m_pLock->Unlock();
    return isFusionEnabled;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DualFovController::TranslateMeteringRegion
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WeightedRegion DualFovController::TranslateMeteringRegion(
    WeightedRegion* pRegion,
    CameraRoleType  camRole)
{
    FLOAT fovRatio = 1.0f;

    WeightedRegion    regionInput = *pRegion;
    WeightedRegion    regionTrans = regionInput;
    PixelShift        pixelShift = { 0 };
    CHIRECT           activeArray = { 0 };

    if (0 != (regionInput.xMax * regionInput.yMax))
    {
        // Acquire mutex to read spatial alignment shifts which are written by other thread
        m_pLock->Lock();
        if (CameraRoleTypeWide == camRole)
        {
            fovRatio    = 1.0f;
            pixelShift  = m_pixelShiftWidePreview;
            activeArray = m_activeArraySizeWide;
        }
        else
        {
            fovRatio = (m_zoomTele / m_zoomWide) * m_adjustedFovRatio;
            pixelShift.xShift = (m_adjustedFovRatio / m_zoomUser) * m_pixelShiftWidePreview.xShift;
            pixelShift.yShift = (m_adjustedFovRatio / m_zoomUser) * m_pixelShiftWidePreview.yShift;
            activeArray = m_activeArraySizeTele;
        }
        m_pLock->Unlock();

        // ROI should be with respect to the corresponding active array
        FLOAT xScale = ((FLOAT)activeArray.width  / m_activeArraySizeWide.width);
        FLOAT yScale = ((FLOAT)activeArray.height / m_activeArraySizeWide.height);

        regionInput.xMin *= xScale;
        regionInput.yMin *= yScale;
        regionInput.xMax *= xScale;
        regionInput.yMax *= yScale;

        INT32 regionWidth       = regionInput.xMax - regionInput.xMin + 1;
        INT32 regionHeight      = regionInput.yMax - regionInput.yMin + 1;
        INT32 regionTransWidth  = regionWidth * fovRatio;
        INT32 regionTransHeight = regionHeight * fovRatio;

        INT32 regionHorzDelta = (regionTransWidth - regionWidth) / 2;
        INT32 regionVertDelta = (regionTransHeight - regionHeight) / 2;

        regionTrans.xMin = regionInput.xMin - regionHorzDelta - pixelShift.xShift;
        regionTrans.yMin = regionInput.yMin - regionVertDelta - pixelShift.yShift;

        regionTrans.xMax = regionTrans.xMin + regionTransWidth - 1;
        regionTrans.yMax = regionTrans.yMin + regionTransHeight - 1;

        INT32 activeArrayWidth  = activeArray.width;
        INT32 activeArrayHeight = activeArray.height;

        // Check ROI bounds and correct it if necessary
        if ((regionTrans.xMin < 0) ||
            (regionTrans.yMin < 0) ||
            (regionTrans.xMax > activeArrayWidth) ||
            (regionTrans.yMax > activeArrayHeight))
        {
            if (regionTrans.xMin < 0)
            {
                regionTrans.xMin = 0;
            }
            if (regionTrans.yMin < 0)
            {
                regionTrans.yMin = 0;
            }
            if (regionTrans.xMax >= activeArrayWidth)
            {
                regionTrans.xMax = activeArrayWidth - 1;
            }
            if (regionTrans.yMax >= activeArrayHeight)
            {
                regionTrans.yMax = activeArrayHeight - 1;
            }
        }
    }

    return regionTrans;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DualFovController::FillOfflinePipelineInputMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult DualFovController::FillOfflinePipelineInputMetadata(
    MulticamResultMetadata* pMultiCamResultMetadata,
    ChiMetadata*                pOfflineMetadata,
    BOOL                        isSnapshotPipeline)
{
    // Populate the vendor tag for struct InputMetadataSAT
    CDKResult result = CDKResultSuccess;

    ChiMetadata* pMetadataWide = NULL;
    ChiMetadata* pMetadataTele = NULL;

    // Identify wide and tele metadata
    MultiCameraIdRole* pMultiCameraRole;

    if (NULL != pMultiCamResultMetadata->ppResultMetadata[0])
    {
        pMultiCameraRole = static_cast<MultiCameraIdRole*>(pMultiCamResultMetadata->ppResultMetadata[0]->GetTag(
            MultiCamControllerManager::m_vendorTagMultiCameraRole));

        if (NULL != pMultiCameraRole)
        {
            if (CameraRoleTypeWide == pMultiCameraRole->currentCameraRole)
            {
                pMetadataWide = pMultiCamResultMetadata->ppResultMetadata[0];
            }
            else
            {
                pMetadataTele = pMultiCamResultMetadata->ppResultMetadata[0];
            }
        }
        else
        {
            CHX_LOG_ERROR("Multicamera role is missing for index 0:%p %p", pMetadataWide, pMetadataTele);
        }
    }
    else
    {
        CHX_LOG_INFO("NULL metadata for cam1");
    }

    if ((1 < pMultiCamResultMetadata->numResults) && (NULL != pMultiCamResultMetadata->ppResultMetadata[1]))
    {
        pMultiCameraRole = static_cast<MultiCameraIdRole*>(pMultiCamResultMetadata->ppResultMetadata[1]->GetTag(
            MultiCamControllerManager::m_vendorTagMultiCameraRole));

        if (NULL != pMultiCameraRole)
        {
            CHX_LOG("pMultiCameraRole->currentCameraRole %d, MasterCameraRole %d", pMultiCameraRole->currentCameraRole,
                         pMultiCameraRole->masterCameraRole);

            if (CameraRoleTypeWide == pMultiCameraRole->currentCameraRole)
            {
                if (NULL == pMetadataWide)
                {
                    pMetadataWide = pMultiCamResultMetadata->ppResultMetadata[1];
                }
                else
                {
                    CHX_LOG_ERROR("Invalid role for metadata index 1, should have been tele");
                }
            }
            else
            {
                if (NULL == pMetadataTele)
                {
                    pMetadataTele = pMultiCamResultMetadata->ppResultMetadata[1];
                }
                else
                {
                    CHX_LOG_ERROR("Invalid role for metadata index 1, should have been wide");
                }
            }
        }
        else
        {
            CHX_LOG_ERROR("Multicamera role is missing for index 1:%p %p", pMetadataWide, pMetadataTele);
        }
    }
    else
    {
        CHX_LOG_INFO("NULL metadata for cam2");
    }

    InputMetadataOpticalZoom metadataOpticalZoom;
    ChxUtils::Memset(&metadataOpticalZoom, 0, sizeof(InputMetadataOpticalZoom));

    m_pLock->Lock();
    PixelShift spatialShift  = m_pixelShiftWideSnapshot;
    m_pLock->Unlock();

    // Read the master camera info
    INT32 isWideMaster = 0;

    if (NULL != pMetadataWide)
    {
        INT32* pIsWideMaster = static_cast<INT32*>(pMetadataWide->GetTag(
            MultiCamControllerManager::m_vendorTagMasterInfo));

        if (NULL != pIsWideMaster)
        {
            isWideMaster = *pIsWideMaster;
        }
    }
    else if (NULL != pMetadataTele)
    {
        INT32 isTeleMaster = 0;

        INT32* pIsTeleMaster = static_cast<INT32*>(pMetadataTele->GetTag(
            MultiCamControllerManager::m_vendorTagMasterInfo));

        if (NULL != pIsTeleMaster)
        {
            isTeleMaster = *pIsTeleMaster;
        }

        isWideMaster = isTeleMaster ? 0 : 1;
    }

    if (((TRUE  == isWideMaster) && (NULL != pMetadataWide)) ||
        ((FALSE == isWideMaster) && (NULL != pMetadataTele)))
    {
        if (NULL != pMetadataWide)
        {
            ExtractCameraMetadata(pMetadataWide, &metadataOpticalZoom.cameraMetadata[0]);

            metadataOpticalZoom.cameraMetadata[0].isValid          = TRUE;
            metadataOpticalZoom.cameraMetadata[0].cameraRole       = CameraRoleTypeWide;
            metadataOpticalZoom.cameraMetadata[0].masterCameraRole =
                isWideMaster ? CameraRoleTypeWide : CameraRoleTypeTele;
            metadataOpticalZoom.cameraMetadata[0].fovRectIFE       = m_fovRectIFEWide;
            metadataOpticalZoom.cameraMetadata[0].fovRectIPE       = m_fovRectIFEWide;
            metadataOpticalZoom.cameraMetadata[0].activeArraySize  = m_activeArraySizeWide;

            CaptureRequestCropRegions* pCropRegions = static_cast<CaptureRequestCropRegions*>(pMetadataWide->GetTag(
                MultiCamControllerManager::m_vendorTagCropRegions));

            if (NULL != pCropRegions)
            {
                metadataOpticalZoom.cameraMetadata[0].userCropRegion     = pCropRegions->userCropRegion;
                metadataOpticalZoom.cameraMetadata[0].pipelineCropRegion = pCropRegions->pipelineCropRegion;
                metadataOpticalZoom.cameraMetadata[0].ifeLimitCropRegion = pCropRegions->ifeLimitCropRegion;
                CHX_LOG("wide cropregion:%d,%d,%d,%d",
                        pCropRegions->pipelineCropRegion.left,
                        pCropRegions->pipelineCropRegion.top,
                        pCropRegions->pipelineCropRegion.width,
                        pCropRegions->pipelineCropRegion.height);

                if (TRUE == isSnapshotPipeline)
                {
                    TranslatedZoom translatedZoom;
                    ChxUtils::Memset(&translatedZoom, 0, sizeof(translatedZoom));
                    m_pZoomTranslator->GetTranslatedZoom(&pCropRegions->userCropRegion, &translatedZoom);
                    ZoomRegions zoomSnapshot = translatedZoom.zoomSnapshot;

                    metadataOpticalZoom.cameraMetadata[0].userCropRegion     = pCropRegions->userCropRegion;
                    metadataOpticalZoom.cameraMetadata[0].pipelineCropRegion = zoomSnapshot.totalZoom[0];
                    metadataOpticalZoom.cameraMetadata[0].ifeLimitCropRegion = zoomSnapshot.ispZoom[0];

                    pCropRegions->pipelineCropRegion = zoomSnapshot.totalZoom[0];
                    pCropRegions->ifeLimitCropRegion = zoomSnapshot.ispZoom[0];
                    pMetadataWide->SetTag(
                        MultiCamControllerManager::m_vendorTagCropRegions, pCropRegions, sizeof(CaptureRequestCropRegions));

                }
            }
        }

        if (NULL != pMetadataTele)
        {
            ExtractCameraMetadata(pMetadataTele, &metadataOpticalZoom.cameraMetadata[1]);

            metadataOpticalZoom.cameraMetadata[1].isValid          = TRUE;
            metadataOpticalZoom.cameraMetadata[1].cameraRole       = CameraRoleTypeTele;
            metadataOpticalZoom.cameraMetadata[1].masterCameraRole =
                isWideMaster ? CameraRoleTypeWide : CameraRoleTypeTele;;
            metadataOpticalZoom.cameraMetadata[1].fovRectIFE       = m_fovRectIFETele;
            metadataOpticalZoom.cameraMetadata[1].fovRectIPE       = m_fovRectIFETele;
            metadataOpticalZoom.cameraMetadata[1].activeArraySize  = m_activeArraySizeTele;

            CaptureRequestCropRegions* pCropRegions = static_cast<CaptureRequestCropRegions*>(pMetadataTele->GetTag(
                MultiCamControllerManager::m_vendorTagCropRegions));
            if (NULL != pCropRegions)
            {
                CHX_LOG("tele cropregion:%d,%d,%d,%d",
                        pCropRegions->pipelineCropRegion.left,
                        pCropRegions->pipelineCropRegion.top,
                        pCropRegions->pipelineCropRegion.width,
                        pCropRegions->pipelineCropRegion.height);
                metadataOpticalZoom.cameraMetadata[1].userCropRegion     = pCropRegions->userCropRegion;
                metadataOpticalZoom.cameraMetadata[1].pipelineCropRegion = pCropRegions->pipelineCropRegion;
                metadataOpticalZoom.cameraMetadata[1].ifeLimitCropRegion = pCropRegions->ifeLimitCropRegion;

                if (TRUE == isSnapshotPipeline)
                {
                    TranslatedZoom translatedZoom;
                    ChxUtils::Memset(&translatedZoom, 0, sizeof(translatedZoom));
                    m_pZoomTranslator->GetTranslatedZoom(&pCropRegions->userCropRegion, &translatedZoom);
                    ZoomRegions zoomSnapshot = translatedZoom.zoomSnapshot;

                    metadataOpticalZoom.cameraMetadata[1].userCropRegion     = pCropRegions->userCropRegion;
                    metadataOpticalZoom.cameraMetadata[1].pipelineCropRegion = zoomSnapshot.totalZoom[1];
                    metadataOpticalZoom.cameraMetadata[1].ifeLimitCropRegion = zoomSnapshot.ispZoom[1];

                    pCropRegions->pipelineCropRegion = zoomSnapshot.totalZoom[1];
                    pCropRegions->ifeLimitCropRegion = zoomSnapshot.ispZoom[1];
                    pMetadataTele->SetTag(
                        MultiCamControllerManager::m_vendorTagCropRegions, pCropRegions, sizeof(CaptureRequestCropRegions));
                }
            }
        }

        metadataOpticalZoom.frameId   = pMultiCamResultMetadata->frameNum;
        metadataOpticalZoom.numInputs = pMultiCamResultMetadata->numResults;
        metadataOpticalZoom.outputShiftSnapshot.horizonalShift = spatialShift.xShift;
        metadataOpticalZoom.outputShiftSnapshot.verticalShift  = spatialShift.yShift;

        // Using role instead of cameraIds. Need to replace this once
        // recommended cameraId is published instead of role.
        ChiMetadata* pMasterMetadata = NULL;
        UINT32       metaIds[MaxCameras];
        ChiMetadata* pMetadataArray[MaxCameras];
        UINT32       primaryMetaId;

        if (TRUE == isWideMaster)
        {
            pMasterMetadata   = pMetadataWide;
            metaIds[0]        = static_cast<UINT32>(CameraRoleTypeWide);
            metaIds[1]        = static_cast<UINT32>(CameraRoleTypeTele);
            pMetadataArray[0] = pMetadataWide;
            pMetadataArray[1] = pMetadataTele;
            primaryMetaId     = metaIds[0];
        }
        else
        {
            pMasterMetadata   = pMetadataTele;
            metaIds[0]        = static_cast<UINT32>(CameraRoleTypeTele);
            metaIds[1]        = static_cast<UINT32>(CameraRoleTypeWide);
            pMetadataArray[0] = pMetadataTele;
            pMetadataArray[1] = pMetadataWide;
            primaryMetaId     = metaIds[0];
        }

        CHX_LOG("isWideMaster %d pMetadataWide %p pMetadataTele %p", isWideMaster, pMetadataWide, pMetadataTele);

        result = pOfflineMetadata->Invalidate();

        result = pOfflineMetadata->MergeMultiCameraMetadata(pMultiCamResultMetadata->numResults,
                                                            pMetadataArray, 
                                                            metaIds,
                                                            primaryMetaId);
        CHX_LOG("Merge multicamera meta isWideMaster %d pMetadataWide %p pMetadataTele %p",
            isWideMaster, pMetadataWide, pMetadataTele);

        if (CDKResultSuccess != result)
        {
            CHX_LOG("Failure in append meta, dst meta size %u src meta size %u",
                    pOfflineMetadata->Count(),
                    pMasterMetadata->Count());
        }

        if (CDKResultSuccess != pOfflineMetadata->SetTag(
            MultiCamControllerManager::m_vendorTagOpticalZoomInputMeta, &metadataOpticalZoom, sizeof(InputMetadataOpticalZoom)))
        {
            CHX_LOG("Failure in pSetMetaData of m_vendorTagOpticalZoomInputMeta");
        }

        if (CDKResultSuccess != pOfflineMetadata->SetTag(
            MultiCamControllerManager::m_vendorTagMultiCameraRole, pMultiCameraRole, sizeof(MultiCameraIdRole)))
        {
            CHX_LOG("Failure in pSetMetaData m_vendorTagMultiCameraRole");
        }

        if (TRUE == isWideMaster)
        {
            ChxUtils::FillCameraId(pOfflineMetadata,
                                   m_camIdWide);
        }
        else
        {
            ChxUtils::FillCameraId(pOfflineMetadata,
                                   m_camIdTele);
        }
    }
    else
    {
        CHAR metaFileName[MaxFileLen];
        if (NULL != pMultiCamResultMetadata->ppResultMetadata[0])
        {
            CdkUtils::SNPrintF(metaFileName, sizeof(metaFileName), "mcam_wide_%d.txt",
                               pMultiCamResultMetadata->ppResultMetadata[0]->GetFrameNumber());
            pMultiCamResultMetadata->ppResultMetadata[0]->DumpDetailsToFile(metaFileName);
        }
        if (NULL != pMultiCamResultMetadata->ppResultMetadata[1])
        {
            CdkUtils::SNPrintF(metaFileName, sizeof(metaFileName), "mcam_tele_%d.txt",
                               pMultiCamResultMetadata->ppResultMetadata[1]->GetFrameNumber());
            pMultiCamResultMetadata->ppResultMetadata[1]->DumpDetailsToFile(metaFileName);
        }
        CHX_LOG_ERROR("Invalid metadata tags iswideMaster %d wide %p tele %p",
                      isWideMaster, pMetadataWide, pMetadataTele);
        result = CamxResultEFailed;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DualFovController::UpdateScalerCropForSnapshot
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID DualFovController::UpdateScalerCropForSnapshot(
    ChiMetadata* pMetadata)
{
    CDKResult result = CDKResultSuccess;
    // Check if the metadata belongs to wide or tele
    BOOL isWideCamMetadata = TRUE;
    MultiCameraIdRole* pMultiCameraRole = static_cast<MultiCameraIdRole*>(pMetadata->GetTag(
        MultiCamControllerManager::m_vendorTagMultiCameraRole));

    if (CDKResultSuccess == result)
    {
        isWideCamMetadata = (CameraRoleTypeWide == pMultiCameraRole->currentCameraRole) ? TRUE : FALSE;
    }

    CaptureRequestCropRegions* pCropRegions = static_cast<CaptureRequestCropRegions*>(pMetadata->GetTag(
        MultiCamControllerManager::m_vendorTagCropRegions));
    if (NULL != pCropRegions)
    {
        TranslatedZoom translatedZoom;
        ChxUtils::Memset(&translatedZoom, 0, sizeof(translatedZoom));
        m_pZoomTranslator->GetTranslatedZoom(&pCropRegions->userCropRegion, &translatedZoom);
        ZoomRegions zoomSnapshot = translatedZoom.zoomSnapshot;

        CHIRECT zoomTotal;
        CHIRECT zoomIspLimit;
        CHIRECT activeArraySize;
        if (TRUE == isWideCamMetadata)
        {
            zoomTotal       = zoomSnapshot.totalZoom[0];
            zoomIspLimit    = zoomSnapshot.ispZoom[0];
            activeArraySize = m_activeArraySizeWide;
        }
        else
        {
            zoomTotal       = zoomSnapshot.totalZoom[1];
            zoomIspLimit    = zoomSnapshot.ispZoom[1];
            activeArraySize = m_activeArraySizeTele;
        }

        /* Update the vendor tag for the CropRegions containing
        user crop, pipeline crop and IFE crop limit */

        // Set the snapshot crop from zoom translator
        // Use the spatial shift to move the crop region
        INT32 updatedLeft = m_pixelShiftWideSnapshot.xShift + zoomTotal.left;
        INT32 updatedTop  = m_pixelShiftWideSnapshot.yShift + zoomTotal.top;

        if ((updatedLeft >= 0) &&
            ((updatedLeft + zoomTotal.width) < activeArraySize.width) &&
            (updatedTop >= 0)  &&
            ((updatedTop + zoomTotal.height) < activeArraySize.height))
        {
            zoomTotal.left = updatedLeft;
            zoomTotal.top  = updatedTop;
            pCropRegions->pipelineCropRegion = zoomTotal;
        }

        updatedLeft = m_pixelShiftWideSnapshot.xShift + zoomIspLimit.left;
        updatedTop  = m_pixelShiftWideSnapshot.yShift + zoomIspLimit.top;

        if ((updatedLeft >= 0) &&
            ((updatedLeft + zoomIspLimit.width) < activeArraySize.width) &&
            (updatedTop >= 0)  &&
            ((updatedTop + zoomIspLimit.height) < activeArraySize.height))
        {
            zoomIspLimit.left = updatedLeft;
            zoomIspLimit.top  = updatedTop;
            pCropRegions->ifeLimitCropRegion = zoomIspLimit;
        }

        pMetadata->SetTag(
            MultiCamControllerManager::m_vendorTagCropRegions, pCropRegions, sizeof(CaptureRequestCropRegions));

        // Update the scaler crop region
        camera_metadata_entry_t entryCropRegion = { 0 };
        if (0 == pMetadata->FindTag(
                ANDROID_SCALER_CROP_REGION, &entryCropRegion))
        {
            pMetadata->SetTag(ANDROID_SCALER_CROP_REGION,
                &zoomTotal, 4);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DualFovController::TranslateFaceRegions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID DualFovController::TranslateFaceRegions(
    VOID*           pResultMetadata)
{
    OutputMetadataOpticalZoom metadataOpticalZoom;
    camera_metadata_entry_t   entry              = { 0 };
    camera_metadata_t         *pMetadata         = static_cast<camera_metadata_t*>(pResultMetadata);

    if (CDKResultSuccess == MultiCamControllerManager::s_vendorTagOps.pGetMetaData(pResultMetadata,
        MultiCamControllerManager::m_vendorTagOpticalZoomResultMeta, &metadataOpticalZoom, sizeof(OutputMetadataOpticalZoom)))
    {
        CameraRoleType camRole = metadataOpticalZoom.masterCamera;

        if (0 == find_camera_metadata_entry(pMetadata, ANDROID_STATISTICS_FACE_RECTANGLES, &entry))
        {
            if (entry.count > 0)
            {
                UINT32  numElemsRect    = sizeof(CHIRECT) / sizeof(UINT32);
                INT32   numFaces        = entry.count / numElemsRect;
                UINT32  dataIndex       = 0;
                INT32   numVisibleFaces = 0;
                CHIRECT faceRegions[FDMaxFaces];

                // Get the user zoom to remove the faces which are outside of preview FOV
                CaptureRequestCropRegions cropRegions;
                MultiCamControllerManager::s_vendorTagOps.pGetMetaData(pMetadata,
                    MultiCamControllerManager::m_vendorTagCropRegions, &cropRegions, sizeof(CaptureRequestCropRegions));

                for (INT32 i = 0; i < numFaces; ++i)
                {
                    UINT32         xMin       = entry.data.i32[dataIndex++];
                    UINT32         yMin       = entry.data.i32[dataIndex++];
                    UINT32         xMax       = entry.data.i32[dataIndex++];
                    UINT32         yMax       = entry.data.i32[dataIndex++];

                    CHX_LOG("Face rectangle from camera_metadata: l_t_r_b_(%d, %d, %d, %d), ratio = %f.",
                            xMin, yMin, xMax, yMax,
                            (m_activeArraySizeWide.width * 1.0f / cropRegions.userCropRegion.width));

                    CHIRECTEXT pUserFace       = {xMin,yMin,xMax,yMax};
                    CHIRECTEXT pTranslatedFace = {0};

                    m_pZoomTranslator->GetTranslatedRect(metadataOpticalZoom,cropRegions.userCropRegion,pUserFace,pTranslatedFace);
                    xMin = pTranslatedFace.left;
                    yMin = pTranslatedFace.top;
                    xMax = pTranslatedFace.right;
                    yMax = pTranslatedFace.bottom;

                    CHX_LOG("Face rectangle after translated: l_t_r_b_(%d, %d, %d, %d)",
                            xMin, yMin, xMax, yMax);

                    CHIRECT      alignedCropRegion;
                    CHIDimension previewDim;
                    CHIDimension cropDim;

                    previewDim.width  = m_previewDimensions.width;
                    previewDim.height = m_previewDimensions.height;
                    cropDim.width     = cropRegions.userCropRegion.width;
                    cropDim.height    = cropRegions.userCropRegion.height;

                    ChxUtils::MatchAspectRatio(&previewDim, &cropDim);

                    alignedCropRegion.width  = cropDim.width;
                    alignedCropRegion.height = cropDim.height;
                    alignedCropRegion.left   = cropRegions.userCropRegion.left +
                                               ((cropRegions.userCropRegion.width  - alignedCropRegion.width)  / 2);
                    alignedCropRegion.top    = cropRegions.userCropRegion.top  +
                                               ((cropRegions.userCropRegion.height - alignedCropRegion.height) / 2);

                    // Reject FD ROIs outside the user cropped region
                    if ((xMin >= alignedCropRegion.left) && (xMax <= alignedCropRegion.left + alignedCropRegion.width) &&
                        (yMin >= alignedCropRegion.top)  && (yMax <= alignedCropRegion.top  + alignedCropRegion.height))
                    {
                        faceRegions[numVisibleFaces].left   = xMin;
                        faceRegions[numVisibleFaces].top    = yMin;
                        faceRegions[numVisibleFaces].width  = xMax;
                        faceRegions[numVisibleFaces].height = yMax;
                        numVisibleFaces++;
                    }
                }

                if (numVisibleFaces > 0)
                {
                    update_camera_metadata_entry(pMetadata, entry.index, faceRegions,
                                                 numVisibleFaces * sizeof(CHIRECT) / sizeof(INT32), NULL);
                }
                else
                {
                    INT32 status = delete_camera_metadata_entry(pMetadata, entry.index);

                    if (CDKResultSuccess == status)
                    {
                        status = add_camera_metadata_entry(pMetadata, ANDROID_STATISTICS_FACE_RECTANGLES, faceRegions, 0);
                    }

                    if (CDKResultSuccess != status)
                    {
                        CHX_LOG_ERROR("Failed to delete/ add metadata entry for no face case");
                    }
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// RTBController::~RTBController
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RTBController::~RTBController()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// RTBController::Destroy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID RTBController::Destroy()
{
    if (NULL != m_pLock)
    {
        m_pLock->Destroy();
        m_pLock = NULL;
    }

    CHX_DELETE this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// RTBController::Create
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RTBController* RTBController::Create(
    MccCreateData* pMccCreateData)
{
    CHX_ASSERT(MaxDevicePerLogicalCamera >= pMccCreateData->numBundledCameras);

    RTBController* pRTBController = CHX_NEW RTBController;

    if (NULL != pRTBController)
    {
        pRTBController->m_cameraType = LogicalCameraType_RTB;

        CDKResult result = CDKResultSuccess;

        pRTBController->m_pLock = Mutex::Create();
        if (NULL == pRTBController->m_pLock)
        {
            result = CDKResultENoMemory;
        }

        if (result == CDKResultSuccess)
        {
            // Parse the camera info to build the controller
            CamInfo* pCamInfoMain = pMccCreateData->pBundledCamInfo[0];
            CamInfo* pCamInfoAux  = pMccCreateData->pBundledCamInfo[1];
            CamInfo* pCamInfoWide = NULL;
            CamInfo* pCamInfoTele = NULL;
            camera_info_t* pAppCameraInfoMain   = static_cast<camera_info_t*>(pCamInfoMain->pChiCamInfo->pLegacy);
            camera_info_t* pAppCameraInfoAux    = static_cast<camera_info_t*>(pCamInfoAux->pChiCamInfo->pLegacy);

            if (pCamInfoMain->pChiCamInfo->lensCaps.focalLength <
                    pCamInfoAux->pChiCamInfo->lensCaps.focalLength)
            {
                pRTBController->m_isMainCamFovWider = TRUE;
                pCamInfoWide = pCamInfoMain;
                pCamInfoTele = pCamInfoAux;
            }
            else
            {
                pRTBController->m_isMainCamFovWider = FALSE;
                pCamInfoWide = pCamInfoAux;
                pCamInfoTele = pCamInfoMain;
            }

            CHX_LOG("Main focal %f, Aux focal %f, Main camera id %d, Aux camera id %d",
                pCamInfoMain->pChiCamInfo->lensCaps.focalLength,
                pCamInfoAux->pChiCamInfo->lensCaps.focalLength,
                pCamInfoMain->camId, pCamInfoTele->camId);

            CHX_LOG("Main facing %d Aux facing %d",
                pAppCameraInfoMain->facing, pAppCameraInfoAux->facing);

            // Save wide, tele and logical camera ids
            pRTBController->m_camIdWide    = pCamInfoWide->camId;
            pRTBController->m_camIdTele    = pCamInfoTele->camId;
            pRTBController->m_camIdLogical = pMccCreateData->logicalCameraId;
            pRTBController->m_numOfDevices = pMccCreateData->numBundledCameras;
            pRTBController->m_activeArraySizeWide = pCamInfoWide->pChiCamInfo->sensorCaps.activeArray;
            pRTBController->m_activeArraySizeTele = pCamInfoTele->pChiCamInfo->sensorCaps.activeArray;
            pRTBController->m_fovRectIFEWide      = pCamInfoWide->fovRectIFE;
            pRTBController->m_fovRectIFETele      = pCamInfoTele->fovRectIFE;

            pRTBController->m_pRawOTPData         = pCamInfoWide->pChiCamInfo->sensorCaps.pRawOTPData;
            pRTBController->m_rawOTPDataSize      = pCamInfoWide->pChiCamInfo->sensorCaps.rawOTPDataSize;

            // Calculate the FOV ratio between the two cameras
            FLOAT focalLengthWide = pCamInfoWide->pChiCamInfo->lensCaps.focalLength;
            FLOAT focalLengthTele = pCamInfoTele->pChiCamInfo->lensCaps.focalLength;
            FLOAT pixelPitchWide  = pCamInfoWide->pChiCamInfo->sensorCaps.pixelSize;
            FLOAT pixelPitchTele  = pCamInfoTele->pChiCamInfo->sensorCaps.pixelSize;
            CHIDIMENSION sensorDimensionWide = pCamInfoWide->sensorOutDimension;
            CHIDIMENSION sensorDimensionTele = pCamInfoTele->sensorOutDimension;

            FLOAT fovWide = sensorDimensionWide.width * pixelPitchWide / focalLengthWide;
            FLOAT fovTele = sensorDimensionTele.width * pixelPitchTele / focalLengthTele;

            if (fovTele > 0.0f)
            {
                pRTBController->m_adjustedFovRatio = fovWide / fovTele;
            }

            pRTBController->SetInitialResultState();
        }
        else
        {
            // Destroy the object in case of failure
            pRTBController->Destroy();
            pRTBController = NULL;
        }
    }

    return pRTBController;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// RTBController::TranslateRequestSettings
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult RTBController::TranslateRequestSettings(
    MulticamReqSettings* pMultiCamSettings)
{
    CDKResult result = CDKResultEFailed;

    if (NULL != pMultiCamSettings)
    {
        CHX_ASSERT(MaxDevicePerLogicalCamera >= pMultiCamSettings->numSettingsBuffers);

        ChiMetadata* pMetadataWide  = NULL;
        ChiMetadata* pMetadataTele  = NULL;

        if (TRUE == m_isMainCamFovWider)
        {
            pMetadataWide = pMultiCamSettings->ppSettings[0];
            pMetadataTele = pMultiCamSettings->ppSettings[1];
        }
        else
        {
            pMetadataWide = pMultiCamSettings->ppSettings[1];
            pMetadataTele = pMultiCamSettings->ppSettings[0];
        }

        if ((NULL != pMetadataWide) && (NULL != pMetadataTele))
        {
            // Set the reference crop window for both Wide and Tele
            RefCropWindowSize refCropWindowWide, refCropWindowTele;
            UINT32 refCropSizeTag = 0;

            result = MultiCamControllerManager::s_vendorTagOps.pQueryVendorTagLocation(
                "org.quic.camera2.ref.cropsize", "RefCropSize", &refCropSizeTag);
            if (CDKResultSuccess == result)
            {
                refCropWindowWide.width = m_activeArraySizeWide.width;
                refCropWindowWide.height = m_activeArraySizeWide.height;

                refCropWindowTele.width = m_activeArraySizeTele.width;
                refCropWindowTele.height = m_activeArraySizeTele.height;

                CHX_LOG("RefCropSize Wide w:%d h:%d Tele w:%d h:%d", refCropWindowWide.width,
                        refCropWindowWide.height, refCropWindowTele.width, refCropWindowTele.height);

                result = pMetadataWide->SetTag(refCropSizeTag,
                    &refCropWindowWide, sizeof(refCropWindowWide));
                result = pMetadataTele->SetTag(refCropSizeTag,
                    &refCropWindowTele, sizeof(refCropWindowTele));
            }

            camera_metadata_entry entryCropRegionWide;
            camera_metadata_entry entryCropRegionTele;
            if ((0 == pMetadataWide->FindTag(
                        ANDROID_SCALER_CROP_REGION, &entryCropRegionWide)) &&
                (0 == pMetadataTele->FindTag(
                        ANDROID_SCALER_CROP_REGION, &entryCropRegionTele)))
            {
                // ANDROID_SCALER_CROP_REGION rectangle should be in reference with sensor Active Array
                // excluding dummy pixels. So, top and left starts with (0, 0) for crop window.
                CHIRECT scalerCropWindowWide, scalerCropWindowTele;

                scalerCropWindowWide = m_activeArraySizeWide;
                scalerCropWindowWide.top  = 0;
                scalerCropWindowWide.left = 0;

                scalerCropWindowTele = m_activeArraySizeTele;
                scalerCropWindowTele.top  = 0;
                scalerCropWindowTele.left = 0;

                // Update the metadata
                pMetadataWide->SetTag(ANDROID_SCALER_CROP_REGION,
                    &scalerCropWindowWide, 4);

                pMetadataTele->SetTag(ANDROID_SCALER_CROP_REGION,
                    &scalerCropWindowTele, 4);

            }
            else
            {
                CHX_LOG_WARN("No crop region information in this metadata!");
            }
            // Translate the Focus ROI
            camera_metadata_entry_t entryAFRegionWide = { 0 };
            camera_metadata_entry_t entryAFRegionTele = { 0 };

            if ((0 == pMetadataWide->FindTag(ANDROID_CONTROL_AF_REGIONS, &entryAFRegionWide)) &&
                (0 == pMetadataTele->FindTag(ANDROID_CONTROL_AF_REGIONS, &entryAFRegionTele)))
            {
                // AF_REGION tag tuple has 5 elements [xMin, yMin, xMax, yMax, weight]
                UINT32 tupleSize = 5;
                for (UINT32 i = 0; i < entryAFRegionTele.count / tupleSize; i++)
                {
                    WeightedRegion afRegion = { 0 };
                    UINT32 index = i * tupleSize;

                    afRegion.xMin   = entryAFRegionTele.data.i32[index];
                    afRegion.yMin   = entryAFRegionTele.data.i32[index + 1];
                    afRegion.xMax   = entryAFRegionTele.data.i32[index + 2];
                    afRegion.yMax   = entryAFRegionTele.data.i32[index + 3];
                    afRegion.weight = entryAFRegionTele.data.i32[index + 4];

                    // Get the translated AF ROI for the wide camera
                    WeightedRegion afRegionWide = TranslateMeteringRegion(&afRegion, CameraRoleTypeWide);

                    // Update the metadata
                    pMetadataWide->SetTag(ANDROID_CONTROL_AF_REGIONS,
                        &afRegionWide, sizeof(WeightedRegion));
                }
            }

            // Translate the Metering ROI
            camera_metadata_entry_t entryAERegionWide = { 0 };
            camera_metadata_entry_t entryAERegionTele = { 0 };

            if ((0 == pMetadataWide->FindTag(ANDROID_CONTROL_AE_REGIONS, &entryAERegionWide)) &&
                (0 == pMetadataTele->FindTag(ANDROID_CONTROL_AE_REGIONS, &entryAERegionTele)))
            {
                // AE_REGION tag tuple has 5 elements [xMin, yMin, xMax, yMax, weight]
                UINT32 tupleSize = 5;
                for (UINT32 i = 0; i < entryAERegionTele.count / tupleSize; i++)
                {
                    WeightedRegion aeRegion = { 0 };
                    UINT32 index = i * tupleSize;

                    aeRegion.xMin   = entryAERegionTele.data.i32[index];
                    aeRegion.yMin   = entryAERegionTele.data.i32[index + 1];
                    aeRegion.xMax   = entryAERegionTele.data.i32[index + 2];
                    aeRegion.yMax   = entryAERegionTele.data.i32[index + 3];
                    aeRegion.weight = entryAERegionTele.data.i32[index + 4];

                    // Get the translated AE ROI for the wide camera
                    WeightedRegion aeRegionWide = TranslateMeteringRegion(&aeRegion, CameraRoleTypeWide);

                    // Update the metadata
                    pMetadataWide->SetTag(ANDROID_CONTROL_AE_REGIONS,
                        &aeRegionWide, sizeof(WeightedRegion));
                }
            }

            // Send the camera role and camera id for the first frame.
            if (0 == pMultiCamSettings->frameNum)
            {
                // Set the vendor tag for the MultiCameraIdRole
                UINT32 tag = 0;
                CDKResult result = MultiCamControllerManager::s_vendorTagOps.pQueryVendorTagLocation(
                    "com.qti.chi.multicamerainfo", "MultiCameraIdRole", &tag);

                if (CDKResultSuccess == result)
                {
                    // Get master camera of this request
                    CameraRoleType masterCameraRole = pMultiCamSettings->currentRequestMCCResult.masterCameraRole;

                    // Set the MultiCameraIdRole for the main camera
                    MultiCameraIdRole inputMetadata;
                    inputMetadata.currentCameraId   = m_camIdTele;
                    inputMetadata.currentCameraRole = CameraRoleTypeTele;
                    inputMetadata.logicalCameraId   = m_camIdLogical;
                    inputMetadata.masterCameraRole  = masterCameraRole;
                    result = pMetadataTele->SetTag(tag,
                        &inputMetadata, sizeof(MultiCameraIdRole));
                    CHX_LOG("m_camIdTele %d Camera role %d", m_camIdTele, inputMetadata.currentCameraRole);

                    // Set the MultiCameraIdRole for the aux camera
                    inputMetadata.currentCameraId   = m_camIdWide;
                    inputMetadata.currentCameraRole = CameraRoleTypeWide;
                    inputMetadata.logicalCameraId   = m_camIdLogical;
                    inputMetadata.masterCameraRole  = masterCameraRole;
                    result = pMetadataWide->SetTag(tag,
                        &inputMetadata, sizeof(MultiCameraIdRole));
                    CHX_LOG("m_camIdWide %d Camera role %d", m_camIdWide, inputMetadata.currentCameraRole);
                }
            }

            // Update all the common vendor tags
            UpdateVendorTags(pMultiCamSettings);

            result = CDKResultSuccess;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// RTBController::SetInitialResultState
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID RTBController::SetInitialResultState()
{
    // Set the active camera state
    m_result.activeCameras[0].cameraId = m_camIdWide;
    m_result.activeCameras[1].cameraId = m_camIdTele;
    m_result.activeCameras[0].isActive = TRUE;
    m_result.activeCameras[1].isActive = TRUE;

    // Set the master camera info
    m_result.masterCameraId   = m_camIdTele;
    m_result.masterCameraRole = CameraRoleTypeTele;
    m_recommendedMasterCameraRole = CameraRoleTypeTele;

    m_result.snapshotFusion = TRUE;
    m_result.isValid        = TRUE;

    CHX_LOG("Active cameras Wide[%d], Tele[%d]", m_result.activeCameras[0].isActive, m_result.activeCameras[1].isActive);
    CHX_LOG("Master camera id: %d", m_result.masterCameraId);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// RTBController::ConsolidateCameraInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult RTBController::ConsolidateCameraInfo(
    UINT32            numBundledCameras,
    CHICAMERAINFO**   ppCamInfo,
    CHICAMERAINFO*    pConsolidatedCamInfo)
{
    CHX_ASSERT(NULL != ppCamInfo);
    CHX_ASSERT(NULL != pConsolidatedCamInfo);
    CHX_ASSERT(MaxDevicePerLogicalCamera >= numBundledCameras);
    CHX_ASSERT(NULL != ppCamInfo[0]);
    CHX_ASSERT(NULL != ppCamInfo[1]);

    CDKResult result = CDKResultSuccess;
    UINT32    mainCameraId, auxCameraId;

    if (ppCamInfo[0]->lensCaps.focalLength < ppCamInfo[1]->lensCaps.focalLength)
    {
        mainCameraId = 0;
        auxCameraId  = 1;
    }
    else
    {
        mainCameraId = 1;
        auxCameraId  = 0;
    }

    camera_info_t* pCameraInfoMain = static_cast<camera_info_t*>(ppCamInfo[mainCameraId]->pLegacy);
    const camera_metadata_t* pMetadataMain = pCameraInfoMain->static_camera_characteristics;

    camera_info_t* pCameraInfoAux = static_cast<camera_info_t*>(ppCamInfo[auxCameraId]->pLegacy);
    const camera_metadata_t* pMetadataAux = pCameraInfoAux->static_camera_characteristics;

    camera_info_t* pCameraInfoConsolidated = static_cast<camera_info_t*>(pConsolidatedCamInfo->pLegacy);
    const camera_metadata_t* pMetadataConsolidated = pCameraInfoConsolidated->static_camera_characteristics;

    pConsolidatedCamInfo->lensCaps       = ppCamInfo[mainCameraId]->lensCaps;
    pConsolidatedCamInfo->numSensorModes = ppCamInfo[mainCameraId]->numSensorModes;
    pConsolidatedCamInfo->sensorCaps     = ppCamInfo[mainCameraId]->sensorCaps;
    ChxUtils::Memcpy(pConsolidatedCamInfo->pLegacy, ppCamInfo[mainCameraId]->pLegacy, sizeof(camera_info_t));
    pCameraInfoConsolidated->static_camera_characteristics = pMetadataConsolidated;

    pConsolidatedCamInfo->size = ppCamInfo[mainCameraId]->size;

    if ((NULL != pMetadataMain) && (NULL != pMetadataAux))
    {
        camera_metadata_entry_t entryMain = { 0 };
        camera_metadata_entry_t entryAux  = { 0 };
        camera_metadata_entry_t entryConsolidated = { 0 };

        // update available stream configure for consolidate metadata
        // this is dual camera usecase. So availabe stream configure should be
        // compilation of main and aux camera.
        if ((0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataMain),
                ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entryMain)) &&
            (0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataAux),
                ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entryAux)) &&
            (0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated),
                ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entryConsolidated)))
        {
            UINT32 consolidatedEntryCount = 0;
            // 4 elements in the tuple of ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS
            UINT32 tupleSize      = 4;
            UINT32 numEntriesMain = entryMain.count / tupleSize;
            UINT32 numEntriesAux  = entryAux.count / tupleSize;

            INT32* pEntryData     = static_cast<INT32*>(ChxUtils::Calloc(
                (numEntriesAux + numEntriesMain) * tupleSize * sizeof(INT32)));
            if (NULL != pEntryData)
            {
                ChxUtils::Memcpy(pEntryData, entryMain.data.i32, entryMain.count * sizeof(INT32));
                consolidatedEntryCount = numEntriesMain;

                for (UINT32 j = 0; j < numEntriesAux; j++)
                {
                    UINT32 auxIndex  = j * tupleSize;
                    BOOL matchFound  = FALSE;

                    for (UINT32 i = 0; i < numEntriesMain; i++)
                    {
                        UINT32 mainIndex = i * tupleSize;
                        if ((entryMain.data.i32[mainIndex]     == entryAux.data.i32[auxIndex])     &&
                            (entryMain.data.i32[mainIndex + 1] == entryAux.data.i32[auxIndex + 1]) &&
                            (entryMain.data.i32[mainIndex + 2] == entryAux.data.i32[auxIndex + 2]) &&
                            (entryMain.data.i32[mainIndex + 3] == entryAux.data.i32[auxIndex + 3]))
                        {
                            matchFound = TRUE;
                            break;
                        }
                    }

                    // if this stream configure is not in main metadata, add it to consolidate metadata.
                    if (FALSE == matchFound)
                    {
                        UINT32 consolidatedIndex          = consolidatedEntryCount * tupleSize;

                        pEntryData[consolidatedIndex]     = entryAux.data.i32[auxIndex];
                        pEntryData[consolidatedIndex + 1] = entryAux.data.i32[auxIndex + 1];
                        pEntryData[consolidatedIndex + 2] = entryAux.data.i32[auxIndex + 2];
                        pEntryData[consolidatedIndex + 3] = entryAux.data.i32[auxIndex + 3];

                        consolidatedEntryCount++;
                    }
                }

                entryConsolidated.count = consolidatedEntryCount * tupleSize;

                if (0 != update_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated), entryConsolidated.index,
                        pEntryData, entryConsolidated.count, &entryConsolidated))
                {
                    CHX_LOG_WARN("update availablestreamconfigure for consolidate metadata failed!");
                }

                ChxUtils::Free(pEntryData);
                pEntryData = NULL;
            }
            else
            {
                CHX_LOG("No memory allocated for pEntryData");
                result = CDKResultENoMemory;
            }
        }
    }

    if ((NULL != pMetadataMain) && (NULL != pMetadataAux))
    {
        camera_metadata_entry_t entryMain = { 0 };
        camera_metadata_entry_t entryAux  = { 0 };
        camera_metadata_entry_t entryConsolidated = { 0 };
        if ((0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataMain),
                ANDROID_JPEG_MAX_SIZE, &entryMain)) &&
            (0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataAux),
                ANDROID_JPEG_MAX_SIZE, &entryAux)) &&
            (0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated),
                ANDROID_JPEG_MAX_SIZE, &entryConsolidated)))
        {

            INT32 maxJPEGSizeOfMain = *entryMain.data.i32;
            INT32 maxJPEGSizeOfAux  = *entryAux.data.i32;
            INT32 maxJPEGSize       = maxJPEGSizeOfMain;
            // we need to use big JPEG size for consolidate metadata
            if (maxJPEGSizeOfAux > maxJPEGSizeOfMain)
            {
                maxJPEGSize = maxJPEGSizeOfAux;
            }

            if (0 != update_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated),
                                                    entryConsolidated.index,
                                                    &maxJPEGSize,
                                                    entryConsolidated.count,
                                                    &entryConsolidated))
            {
                CHX_LOG_WARN("update MAXJPEGSize metadata for consolidate metadata failed!");
            }

        }

    }

    if ((NULL != pMetadataMain) && (NULL != pMetadataAux))
    {
        camera_metadata_entry_t entryMain = { 0 };
        camera_metadata_entry_t entryAux  = { 0 };
        camera_metadata_entry_t entryConsolidated = { 0 };
        if ((0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataMain),
                ANDROID_LENS_FACING, &entryMain)) &&
            (0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataAux),
                ANDROID_LENS_FACING, &entryAux)) &&
            (0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated),
                ANDROID_LENS_FACING, &entryConsolidated)))
        {
            // Use front face as consolidate metadata face,
            // app needs the aux info ( front facing ) to select the logical camera
            INT32 iLensFace = entryAux.data.i32[0];
            if (0 != update_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated), entryConsolidated.index,
                &iLensFace, entryConsolidated.count, &entryConsolidated))
            {
                CHX_LOG_WARN("update lence face metadata for consolidate metadata failed!");
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// RTBController::ProcessResultMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID RTBController::ProcessResultMetadata(
    ChiMetadata* pResultMetadata)
{
    OutputMetadataBokeh* pMetadataBokeh = static_cast<OutputMetadataBokeh*>(pResultMetadata->GetTag(
        MultiCamControllerManager::m_vendorTagBokehResultMeta));

    if (NULL != pMetadataBokeh)
    {
        CHX_LOG("Current master: %d Recommended master: %d", pMetadataBokeh->masterCamera,
                pMetadataBokeh->recommendedMasterCamera);

        m_pLock->Lock();

        // Update the master camera info
        m_result.masterCameraId   = (CameraRoleTypeWide == pMetadataBokeh->masterCamera) ? m_camIdWide : m_camIdTele;
        m_result.masterCameraRole = pMetadataBokeh->masterCamera;

        m_recommendedMasterCameraRole = pMetadataBokeh->recommendedMasterCamera;

        m_pLock->Unlock();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// RTBController::GetResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ControllerResult RTBController::GetResult(
    ChiMetadata* pMetadata)
{
    (VOID)pMetadata;
    m_pLock->Lock();
    ControllerResult result = m_result;

    m_result.activeMap = 0;

    for (UINT32 i = 0 ; i < m_numOfDevices ; i++)
    {
        if (m_result.activeCameras[i].isActive)
        {
            m_result.activeMap |= 1 << i;
        }
    }

    m_pLock->Unlock();

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// RTBController::UpdateResults
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID RTBController::UpdateResults(
    ChiMetadata* pMetadata)
{
    (VOID)pMetadata;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// RTBController::TranslateMeteringRegion
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WeightedRegion RTBController::TranslateMeteringRegion(
    WeightedRegion* pRegion,
    CameraRoleType  camRole)
{
    FLOAT fovRatio = 1.0f;

    WeightedRegion    regionInput = *pRegion;
    WeightedRegion    regionTrans = regionInput;
    CHIRECT           activeArray = { 0 };

    if (0 != (regionInput.xMax * regionInput.yMax))
    {
        // Acquire mutex to read spatial alignment shifts which are written by other thread
        m_pLock->Lock();
        if (CameraRoleTypeWide == camRole)
        {
            fovRatio    = 1.0f / m_adjustedFovRatio;
            activeArray = m_activeArraySizeWide;
        }
        else
        {
            fovRatio    = m_adjustedFovRatio;
            activeArray = m_activeArraySizeTele;
        }
        m_pLock->Unlock();

        // ROI should be with respect to the corresponding active array
        FLOAT xScale = ((FLOAT)activeArray.width  / m_activeArraySizeTele.width);
        FLOAT yScale = ((FLOAT)activeArray.height / m_activeArraySizeTele.height);

        regionInput.xMin *= xScale;
        regionInput.yMin *= yScale;
        regionInput.xMax *= xScale;
        regionInput.yMax *= yScale;

        INT32 regionWidth       = regionInput.xMax - regionInput.xMin + 1;
        INT32 regionHeight      = regionInput.yMax - regionInput.yMin + 1;
        INT32 regionTransWidth  = regionWidth * fovRatio;
        INT32 regionTransHeight = regionHeight * fovRatio;

        INT32 regionHorzDelta = (regionTransWidth - regionWidth) / 2;
        INT32 regionVertDelta = (regionTransHeight - regionHeight) / 2;

        regionTrans.xMin = regionInput.xMin - regionHorzDelta;
        regionTrans.yMin = regionInput.yMin - regionVertDelta;

        regionTrans.xMax = regionTrans.xMin + regionTransWidth - 1;
        regionTrans.yMax = regionTrans.yMin + regionTransHeight - 1;

        INT32 activeArrayWidth  = activeArray.width;
        INT32 activeArrayHeight = activeArray.height;

        // Check ROI bounds and correct it if necessary
        if ((regionTrans.xMin < 0) ||
            (regionTrans.yMin < 0) ||
            (regionTrans.xMax > activeArrayWidth) ||
            (regionTrans.yMax > activeArrayHeight))
        {
            if (regionTrans.xMin < 0)
            {
                regionTrans.xMin = 0;
            }
            if (regionTrans.yMin < 0)
            {
                regionTrans.yMin = 0;
            }
            if (regionTrans.xMax >= activeArrayWidth)
            {
                regionTrans.xMax = activeArrayWidth - 1;
            }
            if (regionTrans.yMax >= activeArrayHeight)
            {
                regionTrans.yMax = activeArrayHeight - 1;
            }
        }
    }

    return regionTrans;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// RTBController::FillOfflinePipelineInputMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult RTBController::FillOfflinePipelineInputMetadata(
    MulticamResultMetadata* pMultiCamResultMetadata,
    ChiMetadata*                pOfflineMetadata,
    BOOL                        isSnapshotPipeline)
{
    (VOID)isSnapshotPipeline;
    // Populate the vendor tag for struct InputMetadataBokeh
    CDKResult result = CDKResultSuccess;

    ChiMetadata* pMetadataWide = NULL;
    ChiMetadata* pMetadataTele = NULL;

    // Identify wide and tele metadata
    MultiCameraIdRole* pMultiCameraRole;

    if (NULL != pMultiCamResultMetadata->ppResultMetadata[0])
    {
        pMultiCameraRole = static_cast<MultiCameraIdRole*>(pMultiCamResultMetadata->ppResultMetadata[0]->GetTag(
            MultiCamControllerManager::m_vendorTagMultiCameraRole));

        if (NULL != pMultiCameraRole)
        {
            if (CameraRoleTypeWide == pMultiCameraRole->currentCameraRole)
            {
                pMetadataWide = pMultiCamResultMetadata->ppResultMetadata[0];
            }
            else
            {
                pMetadataTele = pMultiCamResultMetadata->ppResultMetadata[0];
            }
            CHX_LOG_INFO("MC role for cam1 %d", pMultiCameraRole->currentCameraRole);
        }
        else
        {
            CHX_LOG_ERROR("NULL role for cam1");
        }
    }
    else
    {
        CHX_LOG_INFO("NULL metadata for cam1");
    }

    if (NULL != pMultiCamResultMetadata->ppResultMetadata[1])
    {
        pMultiCameraRole = static_cast<MultiCameraIdRole*>(pMultiCamResultMetadata->ppResultMetadata[1]->GetTag(
            MultiCamControllerManager::m_vendorTagMultiCameraRole));

        if (NULL != pMultiCameraRole)
        {
            if (CameraRoleTypeWide == pMultiCameraRole->currentCameraRole)
            {
                pMetadataWide = pMultiCamResultMetadata->ppResultMetadata[1];
            }
            else
            {
                pMetadataTele = pMultiCamResultMetadata->ppResultMetadata[1];
            }
            CHX_LOG_INFO("MC role for cam2 %d", pMultiCameraRole->currentCameraRole);
        }
        else
        {
            CHX_LOG_ERROR("NULL role for cam2");
        }
    }
    else
    {
        CHX_LOG_INFO("NULL metadata for cam2");
    }

    if((NULL == pMetadataWide) || (NULL == pMetadataTele))
    {
        CHX_LOG_INFO("NULL metadata wide %p tele %p", pMetadataWide, pMetadataTele);
    }

    InputMetadataBokeh metadataBokeh;
    ChxUtils::Memset(&metadataBokeh, 0, sizeof(InputMetadataBokeh));

    m_pLock->Lock();
    CameraRoleType masterCamera = m_recommendedMasterCameraRole;
    m_pLock->Unlock();

    // Read the master camera info to check if wide masterinfo is true else tele is master
    INT32 isWideMaster = 0;

    if (NULL != pMetadataWide)
    {
        INT32* pIsWideMaster = static_cast<INT32*>(pMetadataWide->GetTag(
            MultiCamControllerManager::m_vendorTagMasterInfo));

        if (NULL != pIsWideMaster)
        {
            isWideMaster = *pIsWideMaster;
        }
    }

    if (((TRUE  == isWideMaster) && (NULL != pMetadataWide)) ||
        ((FALSE == isWideMaster) && (NULL != pMetadataTele)))
    {
        if (NULL != pMetadataWide)
        {
            ExtractCameraMetadata(pMetadataWide, &metadataBokeh.cameraMetadata[0]);

            metadataBokeh.cameraMetadata[0].isValid          = TRUE;
            metadataBokeh.cameraMetadata[0].cameraRole       = CameraRoleTypeWide;
            metadataBokeh.cameraMetadata[0].masterCameraRole = masterCamera;
            metadataBokeh.cameraMetadata[0].fovRectIFE       = m_fovRectIFEWide;
            metadataBokeh.cameraMetadata[0].fovRectIPE       = m_fovRectIFEWide;
            metadataBokeh.cameraMetadata[0].activeArraySize  = m_activeArraySizeWide;

            CaptureRequestCropRegions* pCropRegions = static_cast<CaptureRequestCropRegions*>(pMetadataWide->GetTag(
                MultiCamControllerManager::m_vendorTagCropRegions));

            if (NULL != pCropRegions)
            {
                metadataBokeh.cameraMetadata[0].userCropRegion     = pCropRegions->userCropRegion;
                metadataBokeh.cameraMetadata[0].pipelineCropRegion = pCropRegions->pipelineCropRegion;
                metadataBokeh.cameraMetadata[0].ifeLimitCropRegion = pCropRegions->ifeLimitCropRegion;
            }
        }

        if (NULL != pMetadataTele)
        {
            ExtractCameraMetadata(pMetadataTele, &metadataBokeh.cameraMetadata[1]);

            metadataBokeh.cameraMetadata[1].isValid          = TRUE;
            metadataBokeh.cameraMetadata[1].cameraRole       = CameraRoleTypeTele;
            metadataBokeh.cameraMetadata[1].masterCameraRole = masterCamera;
            metadataBokeh.cameraMetadata[1].fovRectIFE       = m_fovRectIFETele;
            metadataBokeh.cameraMetadata[1].fovRectIPE       = m_fovRectIFETele;
            metadataBokeh.cameraMetadata[1].activeArraySize  = m_activeArraySizeTele;

            CaptureRequestCropRegions* pCropRegions = static_cast<CaptureRequestCropRegions*>(pMetadataTele->GetTag(
                MultiCamControllerManager::m_vendorTagCropRegions));

            if (CDKResultSuccess == result)
            {
                metadataBokeh.cameraMetadata[1].userCropRegion     = pCropRegions->userCropRegion;
                metadataBokeh.cameraMetadata[1].pipelineCropRegion = pCropRegions->pipelineCropRegion;
                metadataBokeh.cameraMetadata[1].ifeLimitCropRegion = pCropRegions->ifeLimitCropRegion;
            }
        }

        if (FALSE == m_isMainCamFovWider)
        {
            // Swap the camera meta data if Aux Camera is wider
            CameraMetadata tempMeta;
            tempMeta                        = metadataBokeh.cameraMetadata[0];
            metadataBokeh.cameraMetadata[0] = metadataBokeh.cameraMetadata[1];
            metadataBokeh.cameraMetadata[1] = tempMeta;
        }

        metadataBokeh.frameId      = pMultiCamResultMetadata->frameNum;
        metadataBokeh.blurLevel    = 0;
        metadataBokeh.blurMinValue = 0;
        metadataBokeh.blurMaxValue = 0;

        // Copy the master result metadata first and then add the vendor tag data
        ChiMetadata* pMasterMetadata = NULL;
        if (TRUE == isWideMaster)
        {
            pMasterMetadata = pMetadataWide;
        }
        else
        {
            pMasterMetadata = pMetadataTele;
        }

        CHX_LOG("isWideMaster %d pMetadataWide %p pMetadataTele %p", isWideMaster, pMetadataWide, pMetadataTele);

        pOfflineMetadata->Invalidate();

        if (CDKResultSuccess != pOfflineMetadata->Merge(*pMasterMetadata))
        {
            CHX_LOG("Failure in append meta, dst meta size %u src meta size %u",
                pOfflineMetadata->Count(),
                pMasterMetadata->Count());
        }

        if (CDKResultSuccess != pOfflineMetadata->SetTag(
            MultiCamControllerManager::m_vendorTagBokehInputMeta, &metadataBokeh, sizeof(InputMetadataBokeh)))
        {
            CHX_LOG("Failure in pSetMetaData of m_vendorTagBokehInputMeta");
        }
    }
    else
    {
        CHAR metaFileName[MaxFileLen];
        if (NULL != pMultiCamResultMetadata->ppResultMetadata[0])
        {
            CdkUtils::SNPrintF(metaFileName, sizeof(metaFileName), "mcam_wide_%d.txt",
                               pMultiCamResultMetadata->ppResultMetadata[0]->GetFrameNumber());
            pMultiCamResultMetadata->ppResultMetadata[0]->DumpDetailsToFile(metaFileName);
        }
        if (NULL != pMultiCamResultMetadata->ppResultMetadata[1])
        {
            CdkUtils::SNPrintF(metaFileName, sizeof(metaFileName), "mcam_tele_%d.txt",
                               pMultiCamResultMetadata->ppResultMetadata[1]->GetFrameNumber());
            pMultiCamResultMetadata->ppResultMetadata[1]->DumpDetailsToFile(metaFileName);
        }
        CHX_LOG_ERROR("Invalid metadata tags iswideMaster %d wide %p tele %p",
                      isWideMaster, pMetadataWide, pMetadataTele);

        result = CamxResultEFailed;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// RTBController::UpdateScalerCropForSnapshot
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID RTBController::UpdateScalerCropForSnapshot(
    ChiMetadata* pMetadata)
{
    (VOID)pMetadata;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// BayerMonoController::~BayerMonoController
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BayerMonoController::~BayerMonoController()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// BayerMonoController::Destroy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID BayerMonoController::Destroy()
{
    CHX_DELETE this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// BayerMonoController::Create
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BayerMonoController* BayerMonoController::Create(
    MccCreateData* pMccCreateData)
{
    (VOID)pMccCreateData;

    BayerMonoController* pBayerMonoController = CHX_NEW BayerMonoController;

    CHX_ASSERT(NULL != pBayerMonoController);
    pBayerMonoController->m_cameraType = LogicalCameraType_BAYERMONO;

    // Parse the camera info to build the controller.

    return pBayerMonoController;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// BayerMonoController::Reconfigure
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult BayerMonoController::Reconfigure(
    MccCreateData* pMccCreateData)
{
    (VOID)pMccCreateData;
    return CDKResultSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// BayerMonoController::ConsolidateCameraInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult BayerMonoController::ConsolidateCameraInfo(
    UINT32            numBundledCameras,
    CHICAMERAINFO**   ppCamInfo,
    CHICAMERAINFO*    pConsolidatedCamInfo)
{
    (VOID)numBundledCameras;
    CDKResult result                    = CDKResultSuccess;
    LogicalCameraType logicalCameraType = LogicalCameraType_BAYERMONO;

    // Consolidate the camera info with BayerMono controller logic
    ChxUtils::Memcpy(pConsolidatedCamInfo, ppCamInfo[0], sizeof(CHICAMERAINFO));

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// BayerMonoController::TranslateRequestSettings
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult BayerMonoController::TranslateRequestSettings(
    MulticamReqSettings* pMultiCamSettings)
{
    (VOID)pMultiCamSettings;
    return CDKResultSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// BayerMonoController::ProcessResultMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID BayerMonoController::ProcessResultMetadata(
    ChiMetadata* pResultMetadata)
{
    (VOID)pResultMetadata;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// VRController::~VRController
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VRController::~VRController()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// VRController::Destroy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID VRController::Destroy()
{
    CHX_DELETE this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// VRController::Create
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VRController* VRController::Create(
    MccCreateData* pMccCreateData)
{
    (VOID)pMccCreateData;

    VRController* pVRController = CHX_NEW VRController;
    CamInfo* pCamInfoMainVR  = pMccCreateData->pBundledCamInfo[0];
    CamInfo* pCamInfoSlaveVR = pMccCreateData->pBundledCamInfo[1];

    CHX_ASSERT(NULL != pVRController);
    if (pVRController != NULL)
    {
        pVRController->m_cameraType = LogicalCameraType_VR;
        pVRController->m_camIdVRMaster = pCamInfoMainVR->camId;
        pVRController->m_camIdVRMaster  = pCamInfoSlaveVR->camId;
        pVRController->m_recommendedMasterCameraRole = CameraRoleTypeVR;
    }
    return pVRController;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// VRController::Reconfigure
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult VRController::Reconfigure(
    MccCreateData* pMccCreateData)
{
    (VOID)pMccCreateData;
    return CDKResultSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// VRController::ConsolidateCameraInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult VRController::ConsolidateCameraInfo(
    UINT32            numBundledCameras,
    CHICAMERAINFO**   ppCamInfo,
    CHICAMERAINFO*    pConsolidatedCamInfo)
{
    CHX_ASSERT(NULL != ppCamInfo);
    CHX_ASSERT(NULL != pConsolidatedCamInfo);
    CHX_ASSERT(MaxDevicePerLogicalCamera >= numBundledCameras);
    CHX_ASSERT(NULL != ppCamInfo[0]);
    CHX_ASSERT(NULL != ppCamInfo[1]);
    CDKResult result = CDKResultSuccess;
    static const INT supportedCustomRes   = 2;
    static const INT availConfigTupleSize = 4;
    static const INT availConfigInfo      = supportedCustomRes * 12;
    // Make sure that this array is always sorted in descending order
    // while updating it
    static const INT32 customSupportedSize[availConfigInfo][availConfigTupleSize] = {
        { HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, 5760, 2880, 0 },
        { HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, 5760, 2880, 1 },
        { HAL_PIXEL_FORMAT_YCbCr_420_888, 5760, 2880, 0 },
        { HAL_PIXEL_FORMAT_YCbCr_420_888, 5760, 2880, 1 },
        { HAL_PIXEL_FORMAT_BLOB, 5760, 2880, 0 },
        { HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, 4096, 2048, 0 },
        { HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, 4096, 2048, 1 },
        { HAL_PIXEL_FORMAT_YCbCr_420_888, 4096, 2048, 0 },
        { HAL_PIXEL_FORMAT_YCbCr_420_888, 4096, 2048, 1 },
        { HAL_PIXEL_FORMAT_BLOB, 4096, 2048, 0 },
        { HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, 1920, 1080, 0 },
        { HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, 1920, 1080, 1 },
        { HAL_PIXEL_FORMAT_YCbCr_420_888, 1920, 1080, 0 },
        { HAL_PIXEL_FORMAT_YCbCr_420_888, 1920, 1080, 1 },
        { HAL_PIXEL_FORMAT_BLOB, 3840, 2160, 0 },
        { HAL_PIXEL_FORMAT_BLOB, 3264, 2448, 0 },
        { HAL_PIXEL_FORMAT_RAW10, 8112, 3040, 0 },
        { HAL_PIXEL_FORMAT_RAW10, 8112, 3040, 1 },
        { HAL_PIXEL_FORMAT_RAW12, 8112, 3040, 0 },
        { HAL_PIXEL_FORMAT_RAW12, 8112, 3040, 1 },
        { HAL_PIXEL_FORMAT_RAW16, 8112, 3040, 0 },
        { HAL_PIXEL_FORMAT_RAW16, 8112, 3040, 1 },
        { HAL_PIXEL_FORMAT_RAW_OPAQUE, 8112, 3040, 0 },
        { HAL_PIXEL_FORMAT_RAW_OPAQUE, 8112, 3040, 1 },
    };

    camera_info_t* pCameraInfoMain = static_cast<camera_info_t*>(ppCamInfo[0]->pLegacy);
    const camera_metadata_t* pMetadataMain = pCameraInfoMain->static_camera_characteristics;

    camera_info_t* pCameraInfoAux = static_cast<camera_info_t*>(ppCamInfo[1]->pLegacy);
    const camera_metadata_t* pMetadataAux = pCameraInfoAux->static_camera_characteristics;

    camera_info_t* pCameraInfoConsolidated = static_cast<camera_info_t*>(pConsolidatedCamInfo->pLegacy);
    const camera_metadata_t* pMetadataConsolidated = pCameraInfoConsolidated->static_camera_characteristics;

    pConsolidatedCamInfo->lensCaps       = ppCamInfo[0]->lensCaps;
    pConsolidatedCamInfo->numSensorModes = ppCamInfo[0]->numSensorModes;
    pConsolidatedCamInfo->sensorCaps     = ppCamInfo[0]->sensorCaps;
    ChxUtils::Memcpy(pConsolidatedCamInfo->pLegacy, ppCamInfo[0]->pLegacy, sizeof(camera_info_t));
    pCameraInfoConsolidated->static_camera_characteristics = pMetadataConsolidated;
    pConsolidatedCamInfo->size = ppCamInfo[0]->size;

    if ((NULL != pMetadataMain) && (NULL != pMetadataAux))
    {
        camera_metadata_entry_t entryMain = { 0 };
        camera_metadata_entry_t entryAux  = { 0 };
        camera_metadata_entry_t entryConsolidated = { 0 };
        UINT32 tupleSize = 4;
        INT ret;
        UINT32 consolidatedEntryCount = 0;

        if ((0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataMain),
            ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entryMain)) &&
            (0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataAux),
            ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entryAux)) &&
            (0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated),
            ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entryConsolidated)))
        {
            // 4 elements in the tuple of ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS
            UINT32 tupleSize = 4;
            UINT32 numEntriesMain = entryMain.count / tupleSize;
            UINT32 numEntriesAux  = entryAux.count / tupleSize;
            INT32* pEntryData       = static_cast<INT32*>(ChxUtils::Calloc(
                (numEntriesAux + numEntriesMain) * tupleSize * sizeof(INT32)));
            if (NULL != pEntryData)
            {
                ChxUtils::Memcpy(pEntryData, entryMain.data.i32, entryMain.count * sizeof(INT32));
                UINT32 consolidatedIndex;
                for (INT i = 0; i < availConfigInfo; i++)
                {
                    consolidatedIndex = consolidatedEntryCount * tupleSize;
                    pEntryData[consolidatedIndex]     = customSupportedSize[i][0];
                    pEntryData[consolidatedIndex + 1] = customSupportedSize[i][1];
                    pEntryData[consolidatedIndex + 2] = customSupportedSize[i][2];
                    pEntryData[consolidatedIndex + 3] = customSupportedSize[i][3];
                    consolidatedEntryCount++;
                }

                entryConsolidated.count = consolidatedEntryCount * tupleSize;

                ret = update_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated), entryConsolidated.index,
                        pEntryData, entryConsolidated.count, &entryConsolidated);
                if (ret != 0)
                {
                    CHX_LOG_WARN("update availablestreamconfigure for consolidate metadata failed!");
                }
                ChxUtils::Free(pEntryData);
                pEntryData = NULL;
            }
            else
            {
                CHX_LOG_ERROR("No memory allocated for pEntryData");
                result = CDKResultENoMemory;
            }
        }
   }

   if (NULL != pMetadataConsolidated)
   {
       camera_metadata_entry_t entryConsolidated   = { 0 };
       camera_metadata_entry_t entryMain           = { 0 };
       INT32                   maxJPEGSizeofMain   = 0;
       LogicalCameraType       logicalCameraType   = LogicalCameraType_VR;

       if ((0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataMain),
                                                      ANDROID_JPEG_MAX_SIZE, &entryMain)))
       {
           maxJPEGSizeofMain = *entryMain.data.i32;
       }
       if (0 == find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated),
                ANDROID_JPEG_MAX_SIZE, &entryConsolidated))
       {
           INT32 maxJPEGSize = 0;
           // we need to use big JPEG size for consolidate metadata
           // Calculate data as per the largest width and height
           maxJPEGSize = customSupportedSize[0][1]*customSupportedSize[0][2]*3 + sizeof(CamX::Camera3JPEGBlob);
           if (maxJPEGSize > maxJPEGSizeofMain)
           {
               if (0 != update_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated), entryConsolidated.index,
                            &maxJPEGSize, entryConsolidated.count, &entryConsolidated))
               {
                   CHX_LOG_WARN("update MAXJPEGSize metadata for consolidate metadata failed!");
               }
           }
       }

   }
   return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// VRController::TranslateRequestSettings
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult VRController::TranslateRequestSettings(
    MulticamReqSettings* pMultiCamSettings)
{
    CDKResult result = CDKResultEFailed;

    if (NULL != pMultiCamSettings)
    {
        CHX_ASSERT(MaxDevicePerLogicalCamera >= pMultiCamSettings->numSettingsBuffers);

        ChiMetadata* pMetadata           = pMultiCamSettings->ppSettings[0];
        ChiMetadata* pTranslatedMetadata = pMultiCamSettings->ppSettings[1];

        if ((NULL != pMetadata) && (NULL != pTranslatedMetadata))
        {
             if (0 == pMultiCamSettings->frameNum)
            {
                // Set the vendor tag for the MultiCameraIdRole
                UINT32 tag = 0;
                BOOL isSyncMode = FALSE;
                if (TRUE == pMultiCamSettings->kernelFrameSyncEnable)
                {
                    isSyncMode = TRUE;
                }
                CDKResult result = MultiCamControllerManager::s_vendorTagOps.pQueryVendorTagLocation(
                        "com.qti.chi.multicamerainfo", "MultiCameraIdRole", &tag);

                if (CDKResultSuccess == result)
                {
                    // Set the MultiCameraIdRole for the main camera
                    MultiCameraIdRole inputMetadata;
                    inputMetadata.currentCameraId   = m_camIdVRMaster;
                    inputMetadata.currentCameraRole = CameraRoleTypeVR;
                    inputMetadata.logicalCameraId   = m_camIdLogical;
                    result = pMetadata->SetTag(tag,
                        &inputMetadata, sizeof(MultiCameraIdRole));

                    // Set the MultiCameraIdRole for the aux camera
                    inputMetadata.currentCameraId   = m_camIdVRSlave;
                    inputMetadata.currentCameraRole = CameraRoleTypeVR;
                    inputMetadata.logicalCameraId   = m_camIdLogical;
                    result = pTranslatedMetadata->SetTag(tag,
                        &inputMetadata, sizeof(MultiCameraIdRole));

                    result = pMetadata->SetTag(
                        MultiCamControllerManager::m_vendorTagSyncInfo, &isSyncMode, sizeof(SyncModeInfo));
                    result = pTranslatedMetadata->SetTag(
                        MultiCamControllerManager::m_vendorTagSyncInfo, &isSyncMode, sizeof(SyncModeInfo));
                }
            }

            // Add the master camera info in vendor tag
            UINT32 tagMaster = 0;
            result = MultiCamControllerManager::s_vendorTagOps.pQueryVendorTagLocation(
                "com.qti.chi.multicamerainfo", "MasterCamera", &tagMaster);

            if (CDKResultSuccess == result)
            {
                // Get the master camera
                BOOL isMaster = FALSE;

                isMaster = TRUE;
                result = pMetadata->SetTag(tagMaster,
                    &isMaster, sizeof(BOOL));

                isMaster = FALSE;
                result = pTranslatedMetadata->SetTag(tagMaster,
                    &isMaster, sizeof(BOOL));
            }
            result = CDKResultSuccess;
        }
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// VRController::ProcessResultMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID VRController::ProcessResultMetadata(
    ChiMetadata* pResultMetadata)
{
    (VOID)pResultMetadata;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::~MultiFovController
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MultiFovController::~MultiFovController()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::Destroy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiFovController::Destroy()
{
    if (NULL != m_pLock)
    {
        m_pLock->Destroy();
        m_pLock = NULL;
    }

    if (NULL != m_pZoomTranslator)
    {
        m_pZoomTranslator->Destroy();
        m_pZoomTranslator = NULL;
    }

    CHX_DELETE this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::Create
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MultiFovController* MultiFovController::Create(
    MccCreateData* pMccCreateData)
{
    CHX_ASSERT(MaxDevicePerLogicalCamera >= pMccCreateData->numBundledCameras);
    CHX_ASSERT(InvalidPhysicalCameraId != pMccCreateData->primaryCameraId);

    MultiFovController* pMultiFovController = CHX_NEW MultiFovController;

    if (NULL != pMultiFovController)
    {
        pMultiFovController->m_cameraType               = LogicalCameraType_SAT;
        pMultiFovController->m_numOfLinkedSessions      = pMccCreateData->numBundledCameras;
        pMultiFovController->m_camIdLogical             = pMccCreateData->logicalCameraId;
        pMultiFovController->m_isVideoStreamSelected    = FALSE;
        pMultiFovController->m_smoothTransitionDisabled = FALSE;
        pMultiFovController->m_primaryCamId             = pMccCreateData->primaryCameraId;

        // Fill in per camera info
        for (UINT32 i = 0; i < pMultiFovController->m_numOfLinkedSessions; i++)
        {
            CamInfo *pCamInfo = pMccCreateData->pBundledCamInfo[i];

            // Get focal length, pixel size, sensor dimensions and active pixel array size
            pMultiFovController->m_camInfo[i].focalLength            = pCamInfo->pChiCamInfo->lensCaps.focalLength;
            pMultiFovController->m_camInfo[i].pixelPitch             = pCamInfo->pChiCamInfo->sensorCaps.pixelSize;
            pMultiFovController->m_camInfo[i].sensorOutDimension     = pCamInfo->sensorOutDimension;
            pMultiFovController->m_camInfo[i].activeArraySize        = pCamInfo->pChiCamInfo->sensorCaps.activeArray;
            pMultiFovController->m_camInfo[i].fovRectIFE             = pCamInfo->fovRectIFE;
            pMultiFovController->m_camInfo[i].cameraId               = pCamInfo->camId;
            pMultiFovController->m_camInfo[i].cameraRole             = pCamInfo->pChiCamConfig->cameraRole;
            pMultiFovController->m_camInfo[i].alwaysOn               = pCamInfo->pChiCamConfig->alwaysOn;
            pMultiFovController->m_camInfo[i].enableSmoothTransition = pCamInfo->pChiCamConfig->enableSmoothTransition;
            pMultiFovController->m_camInfo[i].horizontalBinning      = pCamInfo->horizontalBinning;
            pMultiFovController->m_camInfo[i].verticalBinning        = pCamInfo->verticalBinning;

            if (FALSE == pCamInfo->pChiCamConfig->enableSmoothTransition)
            {
                pMultiFovController->m_smoothTransitionDisabled = TRUE;
            }

            /// Get Max Zoom
            camera_metadata_t* pStaticMetadata = static_cast <camera_metadata_t*>(pCamInfo->pChiCamInfo->pLegacy);
            MultiCamControllerManager::s_vendorTagOps.pGetMetaData(pStaticMetadata, ANDROID_SCALER_AVAILABLE_MAX_DIGITAL_ZOOM,
                                                                   &pMultiFovController->m_maxUserZoom, sizeof(FLOAT));
            CHX_LOG_VERBOSE("Max user zoom %f", pMultiFovController->m_maxUserZoom);

            /// Update OTP Data
            for (UINT32 otpIndx = 0; otpIndx < pMultiFovController->m_numOfLinkedSessions; otpIndx++)
            {
                if (i == otpIndx)
                {
                    pMultiFovController->m_camInfo[i].otpData[otpIndx].pRawOtpData    = NULL;
                    pMultiFovController->m_camInfo[i].otpData[otpIndx].rawOtpDataSize = 0;
                }
                pMultiFovController->m_camInfo[i].otpData[otpIndx].pRawOtpData =
                    pCamInfo->pChiCamInfo->sensorCaps.pRawOTPData;
                pMultiFovController->m_camInfo[i].otpData[otpIndx].rawOtpDataSize =
                    pCamInfo->pChiCamInfo->sensorCaps.rawOTPDataSize;
                pMultiFovController->m_camInfo[i].otpData[otpIndx].refCameraId =
                    pMultiFovController->m_camInfo[otpIndx].cameraId;
            }
        }

        for (UINT32 i = 0; i < pMccCreateData->pStreamConfig->numStreams; i++)
        {
            if (0 != (GRALLOC_USAGE_HW_VIDEO_ENCODER & pMccCreateData->pStreamConfig->pStreamInfo[i].usage))
            {
                pMultiFovController->m_isVideoStreamSelected = TRUE;
            }
            if ((CAMERA3_STREAM_OUTPUT == pMccCreateData->pStreamConfig->pStreamInfo[i].streamType) &&
                (GRALLOC_USAGE_HW_COMPOSER == (GRALLOC_USAGE_HW_COMPOSER &
                                               pMccCreateData->pStreamConfig->pStreamInfo[i].usage)))
            {
                pMultiFovController->m_previewDimensions = pMccCreateData->pStreamConfig->pStreamInfo[i].streamDimension;
            }
        }

        CDKResult result = pMultiFovController->CalculateTransitionParams(pMccCreateData);

        if (CDKResultSuccess == result)
        {
            pMultiFovController->m_pLock = Mutex::Create();

            if (NULL == pMultiFovController->m_pLock)
            {
                result = CDKResultENoMemory;
            }
        }

        if (CDKResultSuccess == result)
        {
            // Create and initialize the zoom translator
            pMultiFovController->m_pZoomTranslator = ZoomTranslator::Create();

            if (NULL != pMultiFovController->m_pZoomTranslator)
            {
                result = pMultiFovController->InitZoomTranslator(pMultiFovController->m_pZoomTranslator);

                // Set initial result state
                ChxUtils::Memset(&pMultiFovController->m_result, 0, sizeof(ControllerResult));
            }
            else
            {
                result = CDKResultEFailed;
            }
        }

        // Destroy the object in case of failure
        if (CDKResultSuccess != result)
        {
            pMultiFovController->m_pZoomTranslator->Deinit();
            pMultiFovController->Destroy();
            pMultiFovController = NULL;
        }

    }

    return pMultiFovController;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::InitZoomTranslator
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiFovController::InitZoomTranslator(
    ZoomTranslator* pZoomTranslator)
{
    CDKResult result = CDKResultSuccess;

    if (NULL != pZoomTranslator)
    {
        ZoomTranslatorInitData zoomTranslatorInitData = { { 0 } };
        camera_metadata_entry_t zoomEntry;

        zoomTranslatorInitData.numLinkedSessions  = m_numOfLinkedSessions;
        zoomTranslatorInitData.previewDimension   = m_previewDimensions;
        zoomTranslatorInitData.maxZoom            = MAX_USER_ZOOM;
        zoomTranslatorInitData.defaultAppCameraID = m_primaryCamId;

        for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
        {
            zoomTranslatorInitData.linkedCameraInfo[i].cameraId         = m_camInfo[i].cameraId;
            zoomTranslatorInitData.linkedCameraInfo[i].activeArraySize  = m_camInfo[i].activeArraySize;
            zoomTranslatorInitData.linkedCameraInfo[i].sensorDimension  = m_camInfo[i].sensorOutDimension;
            zoomTranslatorInitData.linkedCameraInfo[i].ifeFOVRect       = m_camInfo[i].fovRectIFE;
            zoomTranslatorInitData.linkedCameraInfo[i].adjustedFovRatio = m_camInfo[i].adjustedFovRatio;
            zoomTranslatorInitData.linkedCameraInfo[i].pixelSize        = m_camInfo[i].pixelPitch;
            zoomTranslatorInitData.linkedCameraInfo[i].focalLength      = m_camInfo[i].focalLength;
            zoomTranslatorInitData.linkedCameraInfo[i].horizontalBinning= m_camInfo[i].horizontalBinning;
            zoomTranslatorInitData.linkedCameraInfo[i].verticalBinning  = m_camInfo[i].verticalBinning;

            memcpy(&zoomTranslatorInitData.linkedCameraInfo[i].otpData,
                   &m_camInfo[i].otpData, sizeof(CalibrationData));
        }

        result = m_pZoomTranslator->Init(&zoomTranslatorInitData);
    }
    else
    {
        result = CamxResultEFailed;
    }

    return result;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::Reconfigure
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiFovController::Reconfigure(
    MccCreateData* pMccCreateData)
{
    CDKResult  result   = CDKResultSuccess;

    CHX_ASSERT(MaxDevicePerLogicalCamera >= pMccCreateData->numBundledCameras);

    m_numOfLinkedSessions   = pMccCreateData->numBundledCameras;
    m_camIdLogical          = pMccCreateData->logicalCameraId;
    m_isVideoStreamSelected = FALSE;

    // Get sensor output dimensions
    for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
    {
        CamInfo *pCamInfo               = pMccCreateData->pBundledCamInfo[i];
        m_camInfo[i].sensorOutDimension = pCamInfo->sensorOutDimension;
    }

    result = CalculateTransitionParams(pMccCreateData);

    for (UINT32 i = 0; i < pMccCreateData->pStreamConfig->numStreams; i++)
    {
        if (0 != (GRALLOC_USAGE_HW_VIDEO_ENCODER & pMccCreateData->pStreamConfig->pStreamInfo[i].usage))
        {
            m_isVideoStreamSelected = TRUE;
        }
        if ((CAMERA3_STREAM_OUTPUT == pMccCreateData->pStreamConfig->pStreamInfo[i].streamType) &&
            (GRALLOC_USAGE_HW_COMPOSER == (GRALLOC_USAGE_HW_COMPOSER & pMccCreateData->pStreamConfig->pStreamInfo[i].usage)))
        {
            m_previewDimensions = pMccCreateData->pStreamConfig->pStreamInfo[i].streamDimension;
        }
    }

    if (CDKResultSuccess == result)
    {
        result = m_pZoomTranslator->Deinit();

        if (CDKResultSuccess == result)
        {
            result = InitZoomTranslator(m_pZoomTranslator);
        }
    }

    return result;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::CalculateFusionParams
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MultiFovController::CalculateFusionParams(TransitionZone *pTransitionZone)
{
    CHX_ASSERT(NULL != pTransitionZone);

    FLOAT transitionRatio  = pTransitionZone->transitionRatio;
    FLOAT rightTransition  = transitionRatio + (transitionRatio * PercentMarginHysterisis);

    pTransitionZone->fusionLow = rightTransition -
        transitionRatio * PercentMarginTransitionZone;
    pTransitionZone->fusionHigh = transitionRatio +
        transitionRatio * PercentMarginTransitionZone;

    // Adjust transitionLow and transitionHigh based on fusion thresholds
    //TODO: There can be more than one fusion zone. Need to update per camera. These variables are not used right now
    pTransitionZone->fusionLow  = (SNAPSHOT_FUSION_ZOOM_MIN < pTransitionZone->fusionLow) ?
        SNAPSHOT_FUSION_ZOOM_MIN : pTransitionZone->fusionLow;
    pTransitionZone->fusionHigh = (SNAPSHOT_FUSION_ZOOM_MAX > pTransitionZone->fusionHigh) ?
        SNAPSHOT_FUSION_ZOOM_MAX : pTransitionZone->fusionHigh;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::CalculateTransitionParams
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiFovController::CalculateTransitionParams(
    MccCreateData* pMccCreateData)
{
    CDKResult result        = CDKResultSuccess;
    UINT32    primaryCamIdx = 0;
    FLOAT     primaryFov    = 0;

    //Get Primary camera Index
    for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
    {
        if (m_camInfo[i].cameraId == m_primaryCamId)
        {
            primaryCamIdx = i;
            break;
        }
    }

    if (0 < m_camInfo[primaryCamIdx].focalLength)
    {
        primaryFov = m_camInfo[primaryCamIdx].activeArraySize.width * m_camInfo[primaryCamIdx].pixelPitch /
            m_camInfo[primaryCamIdx].focalLength;
    }
    else
    {
        CHX_LOG_ERROR("Invalid Focal length for primary camera id %d", m_camInfo[primaryCamIdx].cameraId);
        result = CDKResultEFailed;
    }

    for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
    {
        FLOAT fov = 0.0F;

        if (0 < m_camInfo[i].focalLength)
        {
            fov = m_camInfo[i].activeArraySize.width * m_camInfo[i].pixelPitch /
                m_camInfo[i].focalLength;
        }
        else
        {
            CHX_LOG_ERROR("Invalid Focal length for camera id %d", m_camInfo[i].cameraId);
        }

        if ((0.0F < fov) && (0.0F < primaryFov))
        {
            CamInfo *pCameraCfg           = pMccCreateData->pBundledCamInfo[i];
            m_camInfo[i].adjustedFovRatio = primaryFov / fov;

            CHX_LOG_INFO("camInfo cameraid %d, phy config sensor id %d, camera role %d", m_camInfo[i].cameraId,
                         pCameraCfg->pChiCamConfig->sensorId, pCameraCfg->pChiCamConfig->cameraRole);
            CHX_LOG("Max user zoom %f, adj FOV Ratio %f", m_maxUserZoom, m_camInfo[i].adjustedFovRatio);

            //Calculate Left and right transition params
            if (i > 0)
            {
                /// If i > 0, there is a camera to the left. left transition is valid. Calculate the params.
                m_camInfo[i].transitionLeft.isValid      = TRUE;
                m_camInfo[i - 1].transitionRight.isValid = TRUE;
                CamInfo *pAdjCameraCfg                = pMccCreateData->pBundledCamInfo[i - 1];

                if ((TRUE == pCameraCfg->pChiCamConfig->enableSmoothTransition) &&
                    (TRUE == pAdjCameraCfg->pChiCamConfig->enableSmoothTransition))
                {
                    m_camInfo[i].transitionLeft.smoothTransitionEnabled = TRUE;
                    m_camInfo[i].transitionLeft.transitionRatio        = m_camInfo[i].adjustedFovRatio;
                    m_camInfo[i].transitionLeft.low                    = m_camInfo[i].transitionLeft.transitionRatio -
                        (m_camInfo[i].transitionLeft.transitionRatio * PercentMarginOverlapZone);
                    m_camInfo[i].transitionLeft.high                   = m_camInfo[i].transitionLeft.transitionRatio +
                        (m_camInfo[i].transitionLeft.transitionRatio * PercentMarginOverlapZone);
                    CalculateFusionParams(&m_camInfo[i].transitionLeft);

                    m_camInfo[i - 1].transitionRight.smoothTransitionEnabled = TRUE;
                    m_camInfo[i - 1].transitionRight.transitionRatio   = m_camInfo[i].adjustedFovRatio;
                    m_camInfo[i - 1].transitionRight.low               = m_camInfo[i - 1].transitionRight.transitionRatio -
                        (m_camInfo[i].transitionRight.transitionRatio * PercentMarginOverlapZone);
                    m_camInfo[i - 1].transitionRight.high              = m_camInfo[i - 1].transitionRight.transitionRatio +
                        (m_camInfo[i].transitionRight.transitionRatio * PercentMarginOverlapZone);
                    CalculateFusionParams(&m_camInfo[i - 1].transitionRight);
                }
                else
                {
                    m_camInfo[i].transitionLeft.smoothTransitionEnabled = FALSE;
                    /// Transition ratio is from the camera that has smooth zoom disabled
                    m_camInfo[i].transitionLeft.transitionRatio = pCameraCfg->pChiCamConfig->enableSmoothTransition ?
                        pAdjCameraCfg->pChiCamConfig->transitionZoomRatioMax :
                        pCameraCfg->pChiCamConfig->transitionZoomRatioMin;

                    m_camInfo[i - 1].transitionRight.smoothTransitionEnabled = FALSE;
                    m_camInfo[i - 1].transitionRight.transitionRatio = pAdjCameraCfg->pChiCamConfig->enableSmoothTransition ?
                        pCameraCfg->pChiCamConfig->transitionZoomRatioMin :
                        pAdjCameraCfg->pChiCamConfig->transitionZoomRatioMax;
                }
            }
            else
            {
                //First Camera on the left
                m_camInfo[i].transitionLeft.isValid = FALSE;
                if (TRUE == pCameraCfg->pChiCamConfig->enableSmoothTransition)
                {
                    m_camInfo[i].transitionLeft.transitionRatio = m_camInfo[i].adjustedFovRatio;
                }
                else
                {
                    m_camInfo[i].transitionLeft.transitionRatio = pCameraCfg->pChiCamConfig->transitionZoomRatioMin;
                }
            }

            /// Last Camera on the right
            if (i == m_numOfLinkedSessions - 1)
            {
                m_camInfo[i].transitionRight.isValid = FALSE;
                if (TRUE == pCameraCfg->pChiCamConfig->enableSmoothTransition)
                {
                    m_camInfo[i].transitionRight.transitionRatio = MAX_USER_ZOOM;
                }
                else
                {
                    m_camInfo[i].transitionRight.transitionRatio =
                        pCameraCfg->pChiCamConfig->transitionZoomRatioMax;
                }
            }
        }
    }
    for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
    {
        CHX_ASSERT(m_camInfo[i].transitionLeft.transitionRatio < m_camInfo[i].transitionRight.transitionRatio);
        CHX_LOG_CONFIG("Camera Id %d, Left Transition Ratio %f, Smooth Zoom enabled %d, Right Transition Ratio %f,"
                      "Smooth Zoom enabled %d",
                     m_camInfo[i].cameraId, m_camInfo[i].transitionLeft.transitionRatio,
                     m_camInfo[i].transitionLeft.smoothTransitionEnabled,
                     m_camInfo[i].transitionRight.transitionRatio,
                     m_camInfo[i].transitionRight.smoothTransitionEnabled);
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::SetInitialResultState
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiFovController::SetInitialResultState()
{
    m_result.numOfActiveCameras = m_numOfLinkedSessions;

    for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
    {
        FLOAT minFovRatio = m_camInfo[i].transitionLeft.transitionRatio;
        FLOAT maxFovRatio = m_camInfo[i].transitionRight.transitionRatio;

        CHX_LOG_INFO("cameraid %d, MinFovRatio %f, MaxFovRatio %f, userZoom, %f",
                     m_camInfo[i].cameraId, minFovRatio, maxFovRatio, m_zoomUser);

        m_result.activeCameras[i].cameraId = m_camInfo[i].cameraId;

        if (((m_zoomUser >= minFovRatio) && (m_zoomUser < maxFovRatio))||
            ((i == (m_numOfLinkedSessions - 1)) && (fabs(m_zoomUser - maxFovRatio) < 0.001 )))
        {
            m_result.masterCameraId            = m_camInfo[i].cameraId;
            m_result.masterCameraRole          = m_camInfo[i].cameraRole;
            m_recommendedMasterCameraRole      = m_result.masterCameraRole;
            m_recommendedMasterCameraId        = m_result.masterCameraId;
            m_result.activeCameras[i].isActive = TRUE;
            m_result.activeMap                |= 1 << i;
        }
        else
        {
            m_result.activeCameras[i].isActive = FALSE;
        }

        // Override the active flag if always on.
        if (TRUE == m_camInfo[i].alwaysOn)
        {
            m_result.activeCameras[i].isActive = TRUE;
            m_result.activeMap                |= 1 << i;
        }
    }
    m_result.snapshotFusion = FALSE;

    // Activate all the sensors if LPM is disabled
    if ((FALSE == ENABLE_LPM) && (DualCamCount == m_numOfLinkedSessions))
    {
        for (UINT32 j = 0; j < m_numOfLinkedSessions; j++)
        {
            m_result.activeCameras[j].isActive = TRUE;
        }
    }

    if (TRUE == m_isVideoStreamSelected)
    {
        m_result.snapshotFusion = FALSE;
    }
    else
    {
        m_result.snapshotFusion = TRUE;
    }

    m_result.isValid = TRUE;

    for (UINT32 i = 0; i < m_result.numOfActiveCameras; i++)
    {
        CHX_LOG_INFO("Active cameras camera id[%d], index [%d] Active[%d]", m_result.activeCameras[i].cameraId, i,
                m_result.activeCameras[i].isActive);
    }
    CHX_LOG_INFO("Master camera id: %d fusion: %d, active map %d", m_result.masterCameraId, m_result.snapshotFusion,
                 m_result.activeMap);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::GetResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ControllerResult MultiFovController::GetResult(
    ChiMetadata* pMetadata)
{
    m_pLock->Lock();

    /// If smooth transition is disabled, check if results need to be updated by MCC
    if ((TRUE == m_smoothTransitionDisabled) && (NULL != pMetadata))
    {
        camera_metadata_entry_t entryCropRegion = { 0 };
        if (0 == pMetadata->FindTag(ANDROID_SCALER_CROP_REGION, &entryCropRegion))
        {
            CHIRECT userZoom;
            userZoom.left   = entryCropRegion.data.i32[0];
            userZoom.top    = entryCropRegion.data.i32[1];
            userZoom.width  = entryCropRegion.data.i32[2];
            userZoom.height = entryCropRegion.data.i32[3];

            FLOAT userZoomRatio = m_camInfo[GetCameraIndex(m_primaryCamId)].activeArraySize.width /
                (FLOAT)userZoom.width;

            CHX_LOG_INFO("User Zoom %f, User Zoom Crop rect l %d, T %d, w %d, h %d",
                         userZoomRatio, userZoom.left, userZoom.top, userZoom.width, userZoom.height);
            CheckOverrideMccResult(userZoomRatio);
        }
        else
        {
            CHX_LOG_ERROR("Cannot find metadata ANDROID_SCALER_CROP_REGION in meta pointer %p", pMetadata);
        }
    }

    m_result.activeMap = 0;
    for (UINT32 i = 0; i < m_result.numOfActiveCameras; i++)
    {
        if (TRUE == m_result.activeCameras[i].isActive)
        {
            m_result.activeMap |= 1 << i;
        }
    }

    ControllerResult result = m_result;
    m_pLock->Unlock();

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::TranslateMeteringRegion
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL MultiFovController::isFusionEnabled()
{
    m_pLock->Lock();
    BOOL isFusionEnabled = m_result.snapshotFusion;
    m_pLock->Unlock();
    return isFusionEnabled;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::GetCameraIdFromRole
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 MultiFovController::GetCameraIdFromRole(
    CameraRoleType camRole)
{
    UINT32 camID = InvalidPhysicalCameraId;

    for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
    {
        if (m_camInfo[i].cameraRole == camRole)
        {
            camID = m_camInfo[i].cameraId;
            break;
        }
    }
    return camID;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::GetCameraIdFromRole
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 MultiFovController::GetCameraIndexFromRole(
    CameraRoleType camRole)
{
    UINT32 camIndex = INVALID_INDEX;

    for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
    {
        if (m_camInfo[i].cameraRole == camRole)
        {
            camIndex = i;
            break;
        }
    }
    return camIndex;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::GetCameraIndex
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 MultiFovController::GetCameraIndex(
    UINT32 cameraID)
{
    UINT32 camIndex = INVALID_INDEX;
    for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
    {
        if (m_camInfo[i].cameraId == cameraID)
        {
            camIndex = i;
            break;
        }
    }
    return camIndex;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::GetCameraIndex
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 MultiFovController::GetZoomIndex(
    UINT32   *pCamIdList,
    UINT32   cameraID)
{
    UINT32 camIndex = INVALID_INDEX;

    for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
    {
        if (pCamIdList[i] == cameraID)
        {
            camIndex = i;
            break;
        }
    }
    return camIndex;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::IsSmoothZoomEnabled
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL MultiFovController::IsSmoothZoomEnabled(
    UINT32 cameraID)
{
    UINT32 camIdx            = GetCameraIndex(cameraID);
    BOOL   smoothZoomEnabled = TRUE;

    if (INVALID_INDEX != camIdx)
    {
        smoothZoomEnabled = m_camInfo[camIdx].enableSmoothTransition;
    }

    return smoothZoomEnabled;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::ProcessResultMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiFovController::ProcessResultMetadata(
    ChiMetadata* pResultMetadata)
{
    OutputMetadataOpticalZoom* pMetadataOpticalZoom = NULL;
    MultiCameraIdRole*         pMultiCameraRole = NULL;

    m_pLock->Lock();

    pMetadataOpticalZoom = static_cast <OutputMetadataOpticalZoom*>(
        pResultMetadata->GetTag(MultiCamControllerManager::m_vendorTagOpticalZoomResultMeta));

    if ((NULL != pMetadataOpticalZoom) && (FALSE == m_overrideMode) &&
        (TRUE == IsSmoothZoomEnabled(pMetadataOpticalZoom->masterCameraId)))
    {
        CHX_LOG_INFO("Current master Role: %d Recommended master Role: %d", pMetadataOpticalZoom->masterCamera,
                     pMetadataOpticalZoom->recommendedMasterCamera);

        // Update the master camera info
        m_result.masterCameraId = GetCameraIdFromRole(pMetadataOpticalZoom->recommendedMasterCamera);
        CHX_ASSERT(InvalidPhysicalCameraId != m_result.masterCameraId);

        m_result.masterCameraRole   = pMetadataOpticalZoom->recommendedMasterCamera;
        m_result.numOfActiveCameras = m_numOfLinkedSessions;

        m_recommendedMasterCameraRole = pMetadataOpticalZoom->recommendedMasterCamera;
        m_recommendedMasterCameraId   = m_result.masterCameraId;

        // Update LPM info
        for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
        {
            //Todo : Update when metadata is updated with camera id
            m_result.activeCameras[i].cameraId = m_camInfo[i].cameraId;
            /// The order of active cameras in MCC result info should be in the same order as physical configuration.
            /// Need to remap the camera order from SAT result tip SAT recives physical cam config
            for (UINT32 lpmIdx = 0; lpmIdx < m_numOfLinkedSessions; lpmIdx++)
            {
                /// Todo: Update to camera id once metadata is updated
                if (pMetadataOpticalZoom->lowPowerMode[lpmIdx].cameraRole == m_camInfo[i].cameraRole)
                {
                    m_result.activeCameras[i].isActive = pMetadataOpticalZoom->lowPowerMode[lpmIdx].isEnabled ? 0 : 1;
                }
            }
            CHX_LOG_INFO("Camera id: %d, is Active %d", m_result.activeCameras[i].cameraId,
                         m_result.activeCameras[i].isActive);
        }

        if ((FALSE == ENABLE_LPM) && (DualCamCount == m_numOfLinkedSessions))
        {
            for (UINT32 i = 0; i < m_result.numOfActiveCameras; i++)
            {
                m_result.activeCameras[i].isActive = TRUE;
            }
        }

        // Never put the master into LPM
        for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
        {
            if (((m_recommendedMasterCameraId == m_result.activeCameras[i].cameraId) &&
                (FALSE == m_result.activeCameras[i].isActive)) ||
                ((TRUE == m_camInfo[i].alwaysOn)))
            {
                CHX_LOG_INFO("Camera Id %d is master and inactive, setting it to active", m_recommendedMasterCameraId);
                m_result.activeCameras[i].isActive = TRUE;
            }
        }

        m_result.snapshotFusion = FALSE;
        for (UINT32 i = 0; i < m_result.numOfActiveCameras; i++)
        {
            //Fusion is enabled if adajacent cameras are active at the same time
            if (((i + 1) < m_result.numOfActiveCameras) && (TRUE == m_result.activeCameras[i].isActive) &&
                (TRUE == m_result.activeCameras[i + 1].isActive) &&
                (TRUE != m_isVideoStreamSelected))
            {
                m_result.snapshotFusion = TRUE;
                break;
            }
        }

        // Update spatial alignement pixel shift
        INT32 horzShiftPreview  = pMetadataOpticalZoom->outputShiftPreview.horizonalShift;
        INT32 vertShiftPreview  = pMetadataOpticalZoom->outputShiftPreview.verticalShift;
        INT32 horzShiftSnapshot = pMetadataOpticalZoom->outputShiftSnapshot.horizonalShift;
        INT32 vertShiftSnapshot = pMetadataOpticalZoom->outputShiftSnapshot.verticalShift;

        m_pixelShiftPreview  = { horzShiftPreview, vertShiftPreview };
        m_pixelShiftSnapshot = { horzShiftSnapshot, vertShiftSnapshot };

        m_result.isValid = TRUE;

        CHX_LOG_INFO("Zoom User: %.2f ", m_zoomUser);
        for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
        {
            CHX_LOG_INFO("CameraID %d, Zoom : %.2f", m_camInfo[i].cameraId, m_camInfo[i].zoom);
            CHX_LOG_INFO("Active cameras CameraID %d. Active[%d]", m_result.activeCameras[i].cameraId,
                         m_result.activeCameras[i].isActive);
        }
        CHX_LOG_INFO("Master camera id: %d, Fusion: %d", m_result.masterCameraId, m_result.snapshotFusion);
    }
    // Read LUX index and object focus distance for master camera
    else if (NULL != (pMultiCameraRole = static_cast<MultiCameraIdRole *>(pResultMetadata->GetTag(
        MultiCamControllerManager::m_vendorTagMultiCameraRole))))
    {
        if (m_recommendedMasterCameraRole == pMultiCameraRole->currentCameraRole)
        {
            FLOAT* pLux = NULL;
            if (NULL != (pLux = static_cast<FLOAT*>(pResultMetadata->GetTag(MultiCamControllerManager::m_vendorTagLuxIndex))))
            {
                m_currentLuxIndex = *pLux;
            }
            else
            {
                CHX_LOG_ERROR("Metadata tag LuxIndex is NULL");
            }
            m_currentFocusDistCm = 100;

            // Update the fusion enable flag
            if (ENABLE_SNAPSHOT_FUSION &&
                (m_zoomUser >= SNAPSHOT_FUSION_ZOOM_MIN) &&
                (m_zoomUser <= SNAPSHOT_FUSION_ZOOM_MAX) &&
                (m_currentLuxIndex >= SNAPSHOT_FUSION_LUX_IDX_THRESHOLD) &&
                (m_currentFocusDistCm >= SNAPSHOT_FUSION_FOCUS_DIST_CM_MIN))
            {
                m_result.snapshotFusion = TRUE;
            }
            else
            {
                m_result.snapshotFusion = FALSE;
            }
        }
    }
    else
    {
        if (m_overrideMode)
        {
            CHX_LOG_INFO("MCC in override mode. Not updating result");
        }
        else
        {
            CHX_LOG_ERROR("Metadata tags OpticalZoomResultMeta and MultiCameraRole are NULL!");
        }
    }

    m_pLock->Unlock();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::ConsolidateMinFpsStallDuration
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiFovController::ConsolidateMinFpsStallDuration(
    camera_metadata_t   *pConsolidatedMetadata,
    camera_metadata_t   *pPrimaryCamSettings,
    camera_metadata_t   *pAuxCamSettings[],
    UINT32              numAuxSettings)
{
    CDKResult result = CDKResultSuccess;

    camera_metadata_entry_t entryMain = { 0 };
    camera_metadata_entry_t entryConsolidated = { 0 };
    camera_metadata_entry_t entryAux[MaxDevicePerLogicalCamera - 1];

    INT isPrimaryMetaPresent = 1;
    INT isAuxMetaPresent     = 1;
    INT isConsMetaPresent    = 1;

    // Update available min frame durations for consolidate metadata
    // Since this is multi camera usecase, availabe min frame durations should be
    // compilation of main and aux cameras
    isPrimaryMetaPresent = find_camera_metadata_entry(pPrimaryCamSettings,
        ANDROID_SCALER_AVAILABLE_MIN_FRAME_DURATIONS, &entryMain);

    for (UINT32 i = 0; i < numAuxSettings; i++)
    {
        isAuxMetaPresent = find_camera_metadata_entry(pAuxCamSettings[i],
            ANDROID_SCALER_AVAILABLE_MIN_FRAME_DURATIONS, &entryAux[i]);

        if (0 != isAuxMetaPresent)
        {
            break;
        }
    }

    isConsMetaPresent = find_camera_metadata_entry(pConsolidatedMetadata,
        ANDROID_SCALER_AVAILABLE_MIN_FRAME_DURATIONS, &entryConsolidated);

    if ((0 == isPrimaryMetaPresent) && (0 == isAuxMetaPresent) && (0 == isConsMetaPresent))
    {
        UINT32 numEntriesConsolidated = 0;
        // 4 elements in the tuple of ANDROID_SCALER_AVAILABLE_MIN_FRAME_DURATIONS
        UINT32 tupleSize                                    = 4;
        UINT32 numEntriesMain                               = entryMain.count / tupleSize;
        UINT32 numEntriesAux[MaxDevicePerLogicalCamera - 1] = {0};
        UINT32 numEntriesTotalAux                           = 0;

        for (UINT32 i = 0; i < numAuxSettings; i++)
        {
            numEntriesAux[i]    = entryAux[i].count / tupleSize;
            numEntriesTotalAux += numEntriesAux[i];
        }

        INT64* pEntryData = static_cast<INT64*>(ChxUtils::Calloc(
            (numEntriesTotalAux + numEntriesMain) * tupleSize * sizeof(INT64)));

        if (NULL != pEntryData)
        {
            /// Copy data from Primary cam meta to consolidated meta
            ChxUtils::Memcpy(pEntryData, entryMain.data.i64, entryMain.count * sizeof(INT64));
            numEntriesConsolidated = numEntriesMain;

            for (UINT32 j = 0; j < numAuxSettings; j++)
            {
                for (UINT32 k = 0; k < numEntriesAux[j]; k++)
                {
                    UINT32 auxIndex   = k * tupleSize;
                    BOOL   matchFound = FALSE;

                    /// Consolidated Meta already has main meta. Check if entry is in consolidated meta before adding.
                    for (UINT32 i = 0; i < numEntriesConsolidated; i++)
                    {
                        UINT32 idx = i * tupleSize;
                        if ((pEntryData[idx]     == entryAux[j].data.i64[auxIndex]) &&
                            (pEntryData[idx + 1] == entryAux[j].data.i64[auxIndex + 1]) &&
                            (pEntryData[idx + 2] == entryAux[j].data.i64[auxIndex + 2]) &&
                            (pEntryData[idx + 3] == entryAux[j].data.i64[auxIndex + 3]))
                        {
                            matchFound = TRUE;
                            break;
                        }
                    }

                    // if this min frame duration is not in main metadata, add it to consolidate metadata.
                    if (FALSE == matchFound)
                    {
                        UINT32 consolidatedIndex = numEntriesConsolidated * tupleSize;

                        pEntryData[consolidatedIndex]     = entryAux[j].data.i64[auxIndex];
                        pEntryData[consolidatedIndex + 1] = entryAux[j].data.i64[auxIndex + 1];
                        pEntryData[consolidatedIndex + 2] = entryAux[j].data.i64[auxIndex + 2];
                        pEntryData[consolidatedIndex + 3] = entryAux[j].data.i64[auxIndex + 3];
                        numEntriesConsolidated++;
                    }
                }
            }

            entryConsolidated.count = numEntriesConsolidated * tupleSize;

            if (0 != UpdateMetadata(pConsolidatedMetadata, entryConsolidated.index,
                    pEntryData, entryConsolidated.count, &entryConsolidated))
            {
                CHX_LOG_ERROR("update availableMinFrameDurations for consolidate metadata failed!");
                result = CDKResultEFailed;
            }

            ChxUtils::Free(pEntryData);
            pEntryData = NULL;
        }
        else
        {
            CHX_LOG("No memory allocated for pEntryData");
            result = CDKResultENoMemory;
        }
    }

    if (result == CDKResultSuccess)
    {
        camera_metadata_entry_t entryMain = { 0 };
        camera_metadata_entry_t entryConsolidated = { 0 };
        camera_metadata_entry_t entryAux[MaxDevicePerLogicalCamera - 1];

        INT isPrimaryMetaPresent = 1;
        INT isAuxMetaPresent     = 1;
        INT isConsMetaPresent    = 1;

        // Update available stall durations for consolidate metadata
        // Since this is multi camera usecase, availabe stall durations should be
        // compilation of main and aux cameras
        isPrimaryMetaPresent = find_camera_metadata_entry(pPrimaryCamSettings,
            ANDROID_SCALER_AVAILABLE_STALL_DURATIONS, &entryMain);

        for (UINT32 i = 0; i < numAuxSettings; i++)
        {
            isAuxMetaPresent = find_camera_metadata_entry(pAuxCamSettings[i],
                ANDROID_SCALER_AVAILABLE_STALL_DURATIONS, &entryAux[i]);

            if (0 != isAuxMetaPresent)
            {
                break;
            }
        }

        isConsMetaPresent = find_camera_metadata_entry(pConsolidatedMetadata,
            ANDROID_SCALER_AVAILABLE_STALL_DURATIONS, &entryConsolidated);

        if ((0 == isPrimaryMetaPresent) && (0 == isAuxMetaPresent) && (0 == isConsMetaPresent))
        {
            UINT32 numEntriesConsolidated = 0;
            // 4 elements in the tuple of ANDROID_SCALER_AVAILABLE_STALL_DURATIONS
            UINT32 tupleSize                                    = 4;
            UINT32 numEntriesMain                               = entryMain.count / tupleSize;
            UINT32 numEntriesAux[MaxDevicePerLogicalCamera - 1] = {0};
            UINT32 numEntriesTotalAux                           = 0;

            for (UINT32 i = 0; i < numAuxSettings; i++)
            {
                numEntriesAux[i]    = entryAux[i].count / tupleSize;
                numEntriesTotalAux += numEntriesAux[i];
            }

            INT64* pEntryData = static_cast<INT64*>(ChxUtils::Calloc(
                (numEntriesTotalAux + numEntriesMain) * tupleSize * sizeof(INT64)));

            if (NULL != pEntryData)
            {
                /// Copy data from Primary cam meta to consolidated meta
                ChxUtils::Memcpy(pEntryData, entryMain.data.i64, entryMain.count * sizeof(INT64));
                numEntriesConsolidated = numEntriesMain;

                for (UINT32 j = 0; j < numAuxSettings; j++)
                {
                    for (UINT32 k = 0; k < numEntriesAux[j]; k++)
                    {
                        UINT32 auxIndex   = k * tupleSize;
                        BOOL   matchFound = FALSE;

                        /// Consolidated Meta already has main meta. Check if entry is in consolidated meta before adding.
                        for (UINT32 i = 0; i < numEntriesConsolidated; i++)
                        {
                            UINT32 idx = i * tupleSize;
                            if ((pEntryData[idx]     == entryAux[j].data.i64[auxIndex]) &&
                                (pEntryData[idx + 1] == entryAux[j].data.i64[auxIndex + 1]) &&
                                (pEntryData[idx + 2] == entryAux[j].data.i64[auxIndex + 2]) &&
                                (pEntryData[idx + 3] == entryAux[j].data.i64[auxIndex + 3]))
                            {
                                matchFound = TRUE;
                                break;
                            }
                        }

                        // if this stall durations is not in main metadata, add it to consolidate metadata.
                        if (FALSE == matchFound)
                        {
                            UINT32 consolidatedIndex = numEntriesConsolidated * tupleSize;

                            pEntryData[consolidatedIndex]     = entryAux[j].data.i64[auxIndex];
                            pEntryData[consolidatedIndex + 1] = entryAux[j].data.i64[auxIndex + 1];
                            pEntryData[consolidatedIndex + 2] = entryAux[j].data.i64[auxIndex + 2];
                            pEntryData[consolidatedIndex + 3] = entryAux[j].data.i64[auxIndex + 3];
                            numEntriesConsolidated++;
                        }
                    }
                }

                entryConsolidated.count = numEntriesConsolidated * tupleSize;

                if (0 != UpdateMetadata(pConsolidatedMetadata, entryConsolidated.index,
                        pEntryData, entryConsolidated.count, &entryConsolidated))
                {
                    CHX_LOG_ERROR("update availableStallDurations for consolidate metadata failed!");
                    result = CDKResultEFailed;
                }

                ChxUtils::Free(pEntryData);
                pEntryData = NULL;
            }
            else
            {
                CHX_LOG("No memory allocated for pEntryData");
                result = CDKResultENoMemory;
            }
        }
    }

    return result;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::ConsolidateCameraInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiFovController::ConsolidateStreamConfig(
    camera_metadata_t   *pConsolidatedMetadata,
    camera_metadata_t   *pPrimaryCamSettings,
    camera_metadata_t   **pAuxCamSettings,
    UINT32              numAuxSettings)
{
    CDKResult result = CDKResultSuccess;

    camera_metadata_entry_t entryMain         = { 0 };
    camera_metadata_entry_t entryConsolidated = { 0 };
    camera_metadata_entry_t entryAux[MaxDevicePerLogicalCamera - 1];

    INT isPrimaryMetaPresent = 1;
    INT isAuxMetaPresent     = 1;
    INT isConsMetaPresent    = 1;

    // Update available stream configure for consolidate metadata
    // Since this is multi camera usecase, availabe stream configure should be
    // compilation of main and aux cameras
    isPrimaryMetaPresent = find_camera_metadata_entry(pPrimaryCamSettings,
                                                   ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entryMain);

    for (UINT32 i = 0; i < numAuxSettings; i++)
    {
        isAuxMetaPresent = find_camera_metadata_entry((pAuxCamSettings[i]),
                                                      ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entryAux[i]);
        if (0 != isAuxMetaPresent)
        {
            break;
        }
    }

    isConsMetaPresent = find_camera_metadata_entry(pConsolidatedMetadata,
                                                   ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entryConsolidated);

    if ((0 == isPrimaryMetaPresent) && (0 == isAuxMetaPresent) && (0 == isConsMetaPresent))
    {
        UINT32 numEntriesConsolidated = 0;
        // 4 elements in the tuple of ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS
        UINT32 tupleSize                                    = 4;
        UINT32 numEntriesMain                               = entryMain.count / tupleSize;
        UINT32 numEntriesAux[MaxDevicePerLogicalCamera - 1] = {0};
        UINT32 numEntriesTotalAux                           = 0;

        for (UINT32 i = 0; i < numAuxSettings; i++)
        {
            numEntriesAux[i]    = entryAux[i].count / tupleSize;
            numEntriesTotalAux += numEntriesAux[i];
        }

        INT32* pEntryData = static_cast<INT32*>(ChxUtils::Calloc(
            (numEntriesTotalAux + numEntriesMain) * tupleSize * sizeof(INT32)));

        if (NULL != pEntryData)
        {
            /// Copy data from Primary cam meta to consolidated meta
            ChxUtils::Memcpy(pEntryData, entryMain.data.i32, entryMain.count * sizeof(INT32));
            numEntriesConsolidated = numEntriesMain;

            for (UINT32 j = 0; j < numAuxSettings; j++)
            {
                for (UINT32 k = 0; k < numEntriesAux[j]; k++)
                {
                    UINT32 auxIndex   = k * tupleSize;
                    BOOL   matchFound = FALSE;
                    /// Consolidated Meta already has main meta. Check if entry is in consolidated meta before adding.
                    for (UINT32 i = 0; i < numEntriesConsolidated; i++)
                    {
                        UINT32 idx = i * tupleSize;
                        if ((pEntryData[idx]     == entryAux[j].data.i32[auxIndex])     &&
                            (pEntryData[idx + 1] == entryAux[j].data.i32[auxIndex + 1]) &&
                            (pEntryData[idx + 2] == entryAux[j].data.i32[auxIndex + 2]) &&
                            (pEntryData[idx + 3] == entryAux[j].data.i32[auxIndex + 3]))
                        {
                            matchFound = TRUE;
                            break;
                        }
                    }

                    // if this stream configure is not in main metadata, add it to consolidate metadata.
                    if (FALSE == matchFound)
                    {
                        UINT32 consolidatedIndex = numEntriesConsolidated * tupleSize;

                        pEntryData[consolidatedIndex]     = entryAux[j].data.i32[auxIndex];
                        pEntryData[consolidatedIndex + 1] = entryAux[j].data.i32[auxIndex + 1];
                        pEntryData[consolidatedIndex + 2] = entryAux[j].data.i32[auxIndex + 2];
                        pEntryData[consolidatedIndex + 3] = entryAux[j].data.i32[auxIndex + 3];
                        numEntriesConsolidated++;
                    }
                }
            }

            entryConsolidated.count = numEntriesConsolidated * tupleSize;

            if (0 != UpdateMetadata(pConsolidatedMetadata, entryConsolidated.index,
                                    pEntryData, entryConsolidated.count, &entryConsolidated))
            {
                CHX_LOG_ERROR("update availablestreamconfigure for consolidate metadata failed!");
                result = CDKResultEFailed;
            }

            ChxUtils::Free(pEntryData);
            pEntryData = NULL;
        }
        else
        {
            CHX_LOG("No memory allocated for pEntryData");
            result = CDKResultENoMemory;
        }
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::ConsolidateCameraInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiFovController::ConsolidateCameraInfo(
    LogicalCameraInfo *pLogicalCameraInfo)
{
    CHICAMERAINFO*     ppCamInfo[MaxDevicePerLogicalCamera]          = { 0 };
    CHICAMERAINFO*     pConsolidatedCamInfo;
    camera_metadata_t* pPrimaryMetadata                              = NULL;
    camera_metadata_t* pAuxMetadata[MaxDevicePerLogicalCamera - 1]   = { 0 };

    UINT      numBundledCameras = 0;;
    UINT32    primaryCamIdx     = 0;
    UINT32    primaryCameraId   = 0;
    UINT32    auxCameraCount    = 0;
    CDKResult result            = CDKResultSuccess;

    CHX_ASSERT(NULL != pLogicalCameraInfo);
    CHX_ASSERT(InvalidPhysicalCameraId != pLogicalCameraInfo->primaryCameraId);

    numBundledCameras    = pLogicalCameraInfo->numPhysicalCameras;
    pConsolidatedCamInfo = &(pLogicalCameraInfo->m_cameraCaps);
    primaryCameraId      = pLogicalCameraInfo->primaryCameraId;

    CHX_ASSERT(MaxDevicePerLogicalCamera == numBundledCameras);

    /// Extract Main and Aux cameras metadata
    for (UINT32 i = 0; i < pLogicalCameraInfo->numPhysicalCameras; i++)
    {
        ppCamInfo[i] = const_cast<CHICAMERAINFO*>(pLogicalCameraInfo->ppDeviceInfo[i]->m_pDeviceCaps);
        CHX_ASSERT(NULL != ppCamInfo[i]);

        if (primaryCameraId == pLogicalCameraInfo->ppDeviceInfo[i]->cameraId)
        {
            camera_info_t* pCameraInfoMain  = static_cast<camera_info_t*>(ppCamInfo[i]->pLegacy);
            pPrimaryMetadata = const_cast<camera_metadata_t *>(pCameraInfoMain->static_camera_characteristics);
            primaryCamIdx    = i;
        }
        else
        {
            camera_info_t* pCameraInfo     = static_cast<camera_info_t*>(ppCamInfo[i]->pLegacy);
            pAuxMetadata[auxCameraCount] = const_cast<camera_metadata_t *>(pCameraInfo->static_camera_characteristics);
            if (NULL == pAuxMetadata[auxCameraCount])
            {
                result = CDKResultEFailed;
            }
            auxCameraCount++;
        }
    }
    if (NULL == pPrimaryMetadata)
    {
        result = CDKResultEFailed;
    }
    CHX_LOG("Primary camera id %d, Auxmeta count[%d]", primaryCameraId, auxCameraCount);

    camera_info_t* pCameraInfoConsolidated         = static_cast<camera_info_t*>(pConsolidatedCamInfo->pLegacy);
    const camera_metadata_t* pMetadataConsolidated = pCameraInfoConsolidated->static_camera_characteristics;

    if (NULL != ppCamInfo[primaryCamIdx])
    {
        pConsolidatedCamInfo->lensCaps       = ppCamInfo[primaryCamIdx]->lensCaps;
        pConsolidatedCamInfo->numSensorModes = ppCamInfo[primaryCamIdx]->numSensorModes;
        pConsolidatedCamInfo->sensorCaps     = ppCamInfo[primaryCamIdx]->sensorCaps;

        /// Copy the primary cameras metadata to the consolidated meta
        ChxUtils::Memcpy(pConsolidatedCamInfo->pLegacy, ppCamInfo[primaryCamIdx]->pLegacy, sizeof(camera_info_t));
        pCameraInfoConsolidated->static_camera_characteristics = pMetadataConsolidated;

        pConsolidatedCamInfo->size = ppCamInfo[primaryCamIdx]->size;
    }
    else
    {
        CHX_LOG_ERROR("Primary camera info is NULL");
        result = CDKResultEFailed;
    }

    if (CDKResultSuccess == result)
    {
        result = ConsolidateStreamConfig(const_cast<camera_metadata_t*>(pMetadataConsolidated),
                                         pPrimaryMetadata, pAuxMetadata, auxCameraCount);
    }

    if (CDKResultSuccess == result)
    {
        result = ConsolidateMinFpsStallDuration(const_cast<camera_metadata_t*>(pMetadataConsolidated),
                                                pPrimaryMetadata, pAuxMetadata, auxCameraCount);
    }

    if (CDKResultSuccess == result)
    {
        camera_metadata_entry_t entryMain         = { 0 };
        camera_metadata_entry_t entryConsolidated = { 0 };
        camera_metadata_entry_t entryAux[MaxDevicePerLogicalCamera - 1];


        INT isPrimaryMetaPresent = 1;
        INT isAuxMetaPresent     = 1;
        INT isConsMetaPresent    = 1;

        isPrimaryMetaPresent = find_camera_metadata_entry(pPrimaryMetadata, ANDROID_JPEG_MAX_SIZE, &entryMain);

        for (UINT32 i = 0; i < auxCameraCount; i++)
        {
            isAuxMetaPresent = find_camera_metadata_entry(pAuxMetadata[i], ANDROID_JPEG_MAX_SIZE, &entryAux[i]);
            if (0 != isAuxMetaPresent)
            {
                break;
            }
        }

        isConsMetaPresent = find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated),
                                         ANDROID_JPEG_MAX_SIZE, &entryConsolidated);

        if((0 == isPrimaryMetaPresent) && (0 == isAuxMetaPresent) && (0 == isConsMetaPresent))
        {
            INT32 maxJPEGSizeOfMain = *entryMain.data.i32;
            INT32 maxJPEGSizeOfAux  = 0;
            INT32 maxJPEGSize       = maxJPEGSizeOfMain;
            for (UINT32 i = 0; i < auxCameraCount; i++)
            {
                maxJPEGSizeOfAux = ChxUtils::MaxINT32(maxJPEGSizeOfAux, *entryAux[i].data.i32);
            }

            /// Choose the largest JPEG size for consolidate metadata
            maxJPEGSize = ChxUtils::MaxINT32(maxJPEGSize, maxJPEGSizeOfAux);

            if (0 != UpdateMetadata(const_cast<camera_metadata_t*>(pMetadataConsolidated),
                                    entryConsolidated.index,
                                    &maxJPEGSize,
                                    entryConsolidated.count,
                                    &entryConsolidated))
            {
                CHX_LOG_WARN("update MAXJPEGSize metadata for consolidate metadata failed!");
            }

        }

        /// Update ANDROID_LENS_FACING Tag
        isPrimaryMetaPresent = find_camera_metadata_entry(pPrimaryMetadata, ANDROID_LENS_FACING, &entryMain);

        for (UINT32 i = 0; i < auxCameraCount; i++)
        {
            isAuxMetaPresent = find_camera_metadata_entry(pAuxMetadata[i], ANDROID_LENS_FACING, &entryAux[i]);
            if (0 != isAuxMetaPresent)
            {
                break;
            }
        }
        isConsMetaPresent = find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated),
                                         ANDROID_LENS_FACING, &entryConsolidated);

        if ((0 == isPrimaryMetaPresent) &&
            (0 == isAuxMetaPresent) &&
            (0 == isConsMetaPresent))
        {
            // Use front face as consolidate metadata face,
            // App needs the aux info ( front facing ) to select the logical camera
            INT32 iLensFace = entryAux[0].data.i32[0];
            if (0 != UpdateMetadata(const_cast<camera_metadata_t*>(pMetadataConsolidated), entryConsolidated.index,
                                    &iLensFace, entryConsolidated.count, &entryConsolidated))
            {
                CHX_LOG_WARN("update lence face metadata for consolidate metadata failed!");
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::TranslateReferenceCropWindow
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiFovController::TranslateReferenceCropWindow(
    CameraSettings *primarySettings,
    CameraSettings *ptranslatedSettings,
    UINT32         numOfTranslatedSettings,
    UINT32         primaryCamIdx)
{
    CDKResult result = CDKResultSuccess;

    /// Translate the reference crop window if app set this vendor tag for primary
    RefCropWindowSize* pRefCropWindowPrimary;
    RefCropWindowSize  refCropWindowTranslated[MaxDevicePerLogicalCamera - 1];

    pRefCropWindowPrimary = static_cast<RefCropWindowSize *>(primarySettings->pMetadata->GetTag(
        MultiCamControllerManager::m_vendorTagReferenceCropSize));

    if ((NULL != pRefCropWindowPrimary) && (pRefCropWindowPrimary->width > 0) && (pRefCropWindowPrimary->height > 0))
    {
        for (UINT32 i = 0; i < numOfTranslatedSettings; i++)
        {
            if (NULL != ptranslatedSettings[i].pMetadata)
            {
                UINT32 camIdx = GetCameraIndex(ptranslatedSettings[i].cameraId);
                CHX_ASSERT(INVALID_INDEX != camIdx);

                if ((primaryCamIdx < MaxDevicePerLogicalCamera) && (camIdx < MaxDevicePerLogicalCamera))
                {
                    refCropWindowTranslated[i].width =
                        pRefCropWindowPrimary->width * m_camInfo[camIdx].activeArraySize.width /
                        m_camInfo[primaryCamIdx].activeArraySize.width;
                    refCropWindowTranslated[i].height =
                        pRefCropWindowPrimary->height * m_camInfo[camIdx].activeArraySize.height /
                        m_camInfo[primaryCamIdx].activeArraySize.height;

                    CHX_LOG_INFO("Translate RefCropSize Wide w:%d h:%d Tele w:%d h:%d", pRefCropWindowPrimary->width,
                            pRefCropWindowPrimary->height, refCropWindowTranslated[i].width,
                            refCropWindowTranslated[i].height);
                    result = SetMetadata(ptranslatedSettings[i].pMetadata, MultiCamControllerManager::m_vendorTagReferenceCropSize,
                        &refCropWindowTranslated[i], sizeof(RefCropWindowSize));
                }
                else
                {
                    result = CDKResultEFailed;
                }

                if (CDKResultSuccess != result)
                {
                    CHX_LOG_ERROR("Failed to set metadata for tag Reference Crop size");
                }
            }
        }
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::TranslateROI
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiFovController::TranslateROI(
    CameraSettings* primarySettings,
    CameraSettings* ptranslatedSettings,
    UINT32          numOfTranslatedSettings)
{
    CDKResult result              = CDKResultSuccess;
    INT       isPrimayMetaPresent = 1;
    INT       isAuxMetaPresent    = 1;

    // Translate the Focus ROI
    camera_metadata_entry_t entryAFRegionMain                               = { 0 };
    camera_metadata_entry_t entryAFRegionAux[MaxDevicePerLogicalCamera - 1] = { {0} };

    isPrimayMetaPresent = FindMetadata(primarySettings->pMetadata,
                                      ANDROID_CONTROL_AF_REGIONS, &entryAFRegionMain);

    for (UINT i = 0; i < numOfTranslatedSettings; i++)
    {
        isAuxMetaPresent = FindMetadata(ptranslatedSettings[i].pMetadata,
                                         ANDROID_CONTROL_AF_REGIONS, &entryAFRegionAux[i]);
        if (0 != isAuxMetaPresent)
        {
            break;
        }
    }

    if ((0 == isPrimayMetaPresent) && (0 == isAuxMetaPresent))
    {
        // AF_REGION tag tuple has 5 elements [xMin, yMin, xMax, yMax, weight]
        UINT32 tupleSize = 5;
        for (UINT32 i = 0; i < entryAFRegionMain.count / tupleSize; i++)
        {
            WeightedRegion afRegion = { 0 };
            UINT32 index = i * tupleSize;

            afRegion.xMin   = entryAFRegionMain.data.i32[index];
            afRegion.yMin   = entryAFRegionMain.data.i32[index + 1];
            afRegion.xMax   = entryAFRegionMain.data.i32[index + 2];
            afRegion.yMax   = entryAFRegionMain.data.i32[index + 3];
            afRegion.weight = entryAFRegionMain.data.i32[index + 4];

            // Get the translated AF ROI for the primary camera
            WeightedRegion afRegionTrans = TranslateMeteringRegion(&afRegion, primarySettings->cameraId);

            result = SetMetadata(primarySettings->pMetadata, ANDROID_CONTROL_AF_REGIONS,
                                 &afRegionTrans, 5);

            if (CDKResultSuccess != result)
            {
                CHX_LOG_ERROR("SetMetadata failed for tag AFRegion for camera id %d", primarySettings->cameraId);
            }

            // Get the translated AF ROI for the aux cameras and update metadata
            for (UINT32 i = 0; i < numOfTranslatedSettings; i++)
            {
                afRegionTrans = TranslateMeteringRegion(&afRegion, ptranslatedSettings[i].cameraId);

                /// Update the metadata
                result = SetMetadata(ptranslatedSettings[i].pMetadata,
                                     ANDROID_CONTROL_AF_REGIONS,
                                    &afRegionTrans, 5);

                if (CDKResultSuccess != result)
                {
                    CHX_LOG_ERROR("SetMetadata failed for tag AFRegion for camera id %d",
                                  ptranslatedSettings[i].cameraId);
                }
            }
        }
    }

    // Translate the Metering ROI
    camera_metadata_entry_t entryAERegionMain = { 0 };
    camera_metadata_entry_t entryAERegionAux[MaxDevicePerLogicalCamera - 1] = { {0} };

    isPrimayMetaPresent = FindMetadata(primarySettings->pMetadata,
                                      ANDROID_CONTROL_AE_REGIONS, &entryAERegionMain);
    for (UINT i = 0; i < numOfTranslatedSettings; i++)
    {
        isAuxMetaPresent = FindMetadata(ptranslatedSettings[i].pMetadata,
                                         ANDROID_CONTROL_AE_REGIONS, &entryAERegionAux[i]);
        if (0 != isAuxMetaPresent)
        {
            break;
        }
    }
    if ((0 == isPrimayMetaPresent) && (0 == isAuxMetaPresent))
    {
        // AE_REGION tag tuple has 5 elements [xMin, yMin, xMax, yMax, weight]
        UINT32 tupleSize = 5;
        for (UINT32 i = 0; i < entryAERegionMain.count / tupleSize; i++)
        {
            WeightedRegion aeRegion = { 0 };
            UINT32 index = i * tupleSize;

            aeRegion.xMin   = entryAERegionMain.data.i32[index];
            aeRegion.yMin   = entryAERegionMain.data.i32[index + 1];
            aeRegion.xMax   = entryAERegionMain.data.i32[index + 2];
            aeRegion.yMax   = entryAERegionMain.data.i32[index + 3];
            aeRegion.weight = entryAERegionMain.data.i32[index + 4];

            // Get the translated AE ROI for the wide camera
            WeightedRegion aeRegionTrans = TranslateMeteringRegion(&aeRegion, primarySettings->cameraId);

            // Update the metadata
            result = SetMetadata(primarySettings->pMetadata,
                        ANDROID_CONTROL_AE_REGIONS,
                        &aeRegionTrans, 5);
            if (CDKResultSuccess != result)
            {
                CHX_LOG_ERROR("Set Metadata failed for tag AERegion for camera ID %d",
                              primarySettings->cameraId);
            }

            // Get the translated AE ROI for the aux cameras and update metadata
            for (UINT32 i = 0; i < numOfTranslatedSettings; i++)
            {
                aeRegionTrans = TranslateMeteringRegion(&aeRegion, ptranslatedSettings[i].cameraId);

                // Update the metadata
                SetMetadata(ptranslatedSettings[i].pMetadata,
                            ANDROID_CONTROL_AE_REGIONS,
                             &aeRegionTrans, 5);
                if (CDKResultSuccess != result)
                {
                    CHX_LOG_ERROR("Set Metadata failed for tag AERegion for camera id %d",
                                  ptranslatedSettings[i].cameraId);
                }
            }
        }
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::TranslateRequestSettings
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiFovController::TranslateRequestSettings(
    MulticamReqSettings* pMultiCamSettings)
{
    CDKResult result = CDKResultSuccess;

    CameraSettings primarySettings;
    CameraSettings translatedSettings[MaxDevicePerLogicalCamera - 1];
    UINT32         primaryCamIdx = 0;
    ChxUtils::Memset(&primarySettings, 0, sizeof(primarySettings));
    ChxUtils::Memset(translatedSettings, 0, sizeof(translatedSettings));

    if (NULL != pMultiCamSettings)
    {
        CHX_ASSERT(m_numOfLinkedSessions == pMultiCamSettings->numSettingsBuffers);

        UINT32 translatedMetacount = 0;
        for (UINT32 i = 0; i < pMultiCamSettings->numSettingsBuffers; i++)
        {
            ChiMetadata *pMetadata = pMultiCamSettings->ppSettings[i];

            //Get CameraID from metadata
            UINT32* pCameraId = static_cast <UINT32*>(pMetadata->GetTag(
                    MultiCamControllerManager::m_vendorTagMetadataOwner));

            if (NULL != pCameraId)
            {
                if (m_primaryCamId == *pCameraId)
                {
                    primarySettings.cameraId = *pCameraId;
                    primarySettings.pMetadata = pMetadata;
                    primaryCamIdx = GetCameraIndex(primarySettings.cameraId);
                    CHX_ASSERT(INVALID_INDEX != primaryCamIdx);
                }
                else
                {
                    translatedSettings[translatedMetacount].cameraId = *pCameraId;
                    translatedSettings[translatedMetacount].pMetadata = pMetadata;
                    translatedMetacount++;
                }
            }
            else
            {
                CHX_LOG_ERROR("Metadata tag MetadataOwner is NULL. Cannot get cameraId");
                result = CDKResultEFailed;
            }
        }
        if(NULL == primarySettings.pMetadata)
        {
            CHX_LOG_ERROR("Primary setting metadata is NULL");
            result = CDKResultEFailed;
        }

        if (CDKResultSuccess == result)
        {
            result = TranslateReferenceCropWindow(&primarySettings, translatedSettings,
                                                  translatedMetacount, primaryCamIdx);
        }

         // Translate the zoom crop window
         camera_metadata_entry_t entryCropRegion                                     = { 0 };
         camera_metadata_entry_t entryCropRegionTrans[MaxDevicePerLogicalCamera - 1] = { {0} };

         if ((CDKResultSuccess == result) && (0 == FindMetadata(primarySettings.pMetadata,
                               ANDROID_SCALER_CROP_REGION, &entryCropRegion)))
        {
            TranslatedZoom translatedZoom;
            ChxUtils::Memset(&translatedZoom, 0, sizeof(translatedZoom));

            CHIRECT userZoom;
            userZoom.left   = entryCropRegion.data.i32[0];
            userZoom.top    = entryCropRegion.data.i32[1];
            userZoom.width  = entryCropRegion.data.i32[2];
            userZoom.height = entryCropRegion.data.i32[3];

            if (CDKResultSuccess == m_pZoomTranslator->GetTranslatedZoom(&userZoom, &translatedZoom))
            {
                // Update the zoom value for primary camera
                ZoomRegions zoomPreview = translatedZoom.zoomPreview;
                UINT32 zoomIdx = GetZoomIndex(zoomPreview.cameraID, primarySettings.cameraId);
                CHX_LOG_INFO("cameraid %d, zoomIdx %d", primarySettings.cameraId, zoomIdx);
                SetMetadata(primarySettings.pMetadata,
                            ANDROID_SCALER_CROP_REGION,
                            &zoomPreview.totalZoom[zoomIdx], 4);


                /* Update the vendor tag for the CropRegions containing
                user crop, pipeline crop and IFE crop limit */
                CaptureRequestCropRegions cropRegions[MaxDevicePerLogicalCamera];

                for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
                {
                    cropRegions[i].userCropRegion     = userZoom;

                    CHX_LOG_INFO("Usercropregion: Cameraid:%d [%d,%d,%d,%d]", zoomPreview.cameraID[i],
                                 userZoom.left, userZoom.top,
                                 userZoom.width, userZoom.height);

                    cropRegions[i].pipelineCropRegion = zoomPreview.totalZoom[i];
                    cropRegions[i].ifeLimitCropRegion = zoomPreview.ispZoom[i];
                    CHX_LOG_INFO("inputcropregion: Cameraid:%d [%d,%d,%d,%d]", zoomPreview.cameraID[i],
                            zoomPreview.totalZoom[i].left, zoomPreview.totalZoom[i].top,
                            zoomPreview.totalZoom[i].width, zoomPreview.totalZoom[i].height);
                }

                for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
                {
                     ChxUtils::SetVendorTagValue(pMultiCamSettings->ppSettings[i], CropRegions,
                                                    sizeof(CaptureRequestCropRegions), &cropRegions[i]);

                     /// Update zoom for each camera from translated zoom
                     m_camInfo[i].zoom = m_camInfo[i].activeArraySize.width /
                         (FLOAT)translatedZoom.zoomSnapshot.totalZoom[i].width;
                     CHX_LOG_INFO("Transalate Settings cameraID %d Zoom: %.2f",
                             m_camInfo[i].cameraId, m_camInfo[i].zoom);
                }

                m_zoomUser = m_camInfo[m_primaryCamId].activeArraySize.width / (FLOAT)userZoom.width;
                CHX_LOG_INFO("User Zoom: %.2f", m_zoomUser);

                for (UINT32 i = 0; i < translatedMetacount; i++)
                {
                    if (0 == FindMetadata(translatedSettings[i].pMetadata,
                                          ANDROID_SCALER_CROP_REGION, &entryCropRegionTrans[i]))
                    {
                        UINT32 idx = GetCameraIndex(translatedSettings[i].cameraId);
                        CHX_ASSERT(INVALID_INDEX != idx);

                        SetMetadata(translatedSettings[i].pMetadata,
                                    ANDROID_SCALER_CROP_REGION,
                                    &zoomPreview.totalZoom[idx], 4);
                        CHX_LOG_INFO("Input crop region:tele:%d,%d,%d,%d", zoomPreview.totalZoom[i].left,
                                zoomPreview.totalZoom[i].top, zoomPreview.totalZoom[i].width,
                                zoomPreview.totalZoom[i].height);
                    }
                }
            }
        }
         else
         {
             CHX_LOG_ERROR("Cannot Translate Zoom Crop window");
         }

         /// Translate 3A ROI
         if (CDKResultSuccess == result)
         {
             result = TranslateROI(&primarySettings, translatedSettings, translatedMetacount);
         }
         else
         {
             CHX_LOG_ERROR("Cannot Translate ROI");
         }

        // Update all the common vendor tags
        UpdateVendorTags(pMultiCamSettings);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::UpdateVendorTags
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiFovController::UpdateVendorTags(
    MulticamReqSettings* pMultiCamSettings)
{
    CDKResult result = CDKResultSuccess;

    CameraSettings primarySettings;
    CameraSettings translatedSettings[MaxDevicePerLogicalCamera - 1];
    UINT32         primaryCamIdx = 0;
    ChxUtils::Memset(&primarySettings, 0, sizeof(primarySettings));
    ChxUtils::Memset(translatedSettings, 0, sizeof(translatedSettings));

    if (NULL != pMultiCamSettings)
    {
        CHX_ASSERT(m_numOfLinkedSessions == pMultiCamSettings->numSettingsBuffers);

        UINT32 translatedMetacount = 0;
        for (UINT32 i = 0; i < pMultiCamSettings->numSettingsBuffers; i++)
        {
            ChiMetadata *pMetadata = pMultiCamSettings->ppSettings[i];

            //Get CameraID from metadata
            UINT32* pCameraId = static_cast <UINT32*>(pMetadata->GetTag(
                    MultiCamControllerManager::m_vendorTagMetadataOwner));

            if (NULL != pCameraId)
            {
                if (m_primaryCamId == *pCameraId)
                {
                    primarySettings.cameraId = *pCameraId;
                    primarySettings.pMetadata = pMetadata;
                    primaryCamIdx = GetCameraIndex(primarySettings.cameraId);
                    CHX_ASSERT(INVALID_INDEX != primaryCamIdx);
                }
                else
                {
                    translatedSettings[translatedMetacount].cameraId = *pCameraId;
                    translatedSettings[translatedMetacount].pMetadata = pMetadata;
                    translatedMetacount++;
                }
            }
            else
            {
                CHX_LOG_ERROR("Metadata tag MetadataOwner is NULL. Cannot get cameraId");
                result = CDKResultEFailed;
            }
        }
        if(NULL == primarySettings.pMetadata)
        {
            CHX_LOG_ERROR("Primary setting metadata is NULL");
            result = CDKResultEFailed;
        }

        // Get the SW master camera of this request
        CameraRoleType masterCameraRole = pMultiCamSettings->currentRequestMCCResult.masterCameraRole;
        CHX_LOG_INFO("Master camera role from MCC result %d", masterCameraRole);

        // Set the camera role and camera id for the first frame.
        if ((CDKResultSuccess == result) && (0 == pMultiCamSettings->frameNum))
        {
            // Set the MultiCameraIdRole for the main camera
            MultiCameraIdRole inputMetadata;
            inputMetadata.currentCameraId   = m_camInfo[primaryCamIdx].cameraId;
            inputMetadata.currentCameraRole = m_camInfo[primaryCamIdx].cameraRole;
            inputMetadata.logicalCameraId   = m_camIdLogical;
            inputMetadata.masterCameraRole  = masterCameraRole;
            result = SetMetadata(primarySettings.pMetadata,
                                 MultiCamControllerManager::m_vendorTagMultiCameraRole,
                                 &inputMetadata, sizeof(MultiCameraIdRole));
            CHX_LOG_INFO("primary CameraId %d Camera role %d", inputMetadata.currentCameraId,
                         inputMetadata.currentCameraRole);

            if (CDKResultSuccess != result)
            {
                CHX_LOG_ERROR("Failed to set metadata tag  MultiCameraRole for camera id %d ", inputMetadata.currentCameraId);
            }

            for (UINT32 i = 0; i < translatedMetacount; i++)
            {
                // Set the MultiCameraIdRole for the aux cameras
                inputMetadata.currentCameraId   = translatedSettings[i].cameraId;
                inputMetadata.currentCameraRole = m_camInfo[GetCameraIndex(translatedSettings[i].cameraId)].cameraRole;
                inputMetadata.logicalCameraId   = m_camIdLogical;
                inputMetadata.masterCameraRole  = masterCameraRole;

                result = SetMetadata(translatedSettings[i].pMetadata,
                                     MultiCamControllerManager::m_vendorTagMultiCameraRole,
                                     &inputMetadata, sizeof(MultiCameraIdRole));
                CHX_LOG_INFO("Aux CameraId %d Camera role %d", inputMetadata.currentCameraId,
                             inputMetadata.currentCameraRole);

                if (CDKResultSuccess != result)
                {
                    CHX_LOG_ERROR("Failed to set metadata tag  MultiCameraRole for camera id %d ",
                                 inputMetadata.currentCameraId);
                }

            }
        }

        // Add master camera info
        if (CDKResultSuccess == result)
        {
            BOOL isMaster = FALSE;
            isMaster = (m_camInfo[primaryCamIdx].cameraRole == masterCameraRole) ? TRUE : FALSE;
            result = SetMetadata(primarySettings.pMetadata,
                                 MultiCamControllerManager::m_vendorTagMasterInfo,
                                 &isMaster, 1);
            CHX_LOG_INFO("MasterCameraRole %d, Current Camera id %d,Current Camera role %d, isMaster %d", masterCameraRole,
                         m_camInfo[primaryCamIdx].cameraId, m_camInfo[primaryCamIdx].cameraRole, isMaster);

            if (CDKResultSuccess != result)
            {
                CHX_LOG_ERROR("Failed to set metadata tag for MasterInfo for camera id %d ",
                              m_camInfo[primaryCamIdx].cameraId);
            }

            for (UINT32 i = 0; i < translatedMetacount; i++)
            {
                UINT32 camIdx = GetCameraIndex(translatedSettings[i].cameraId);
                CHX_ASSERT(INVALID_INDEX != camIdx);
                isMaster = (m_camInfo[camIdx].cameraRole == masterCameraRole) ? TRUE : FALSE;
                result = SetMetadata(translatedSettings[i].pMetadata,
                                     MultiCamControllerManager::m_vendorTagMasterInfo,
                                     &isMaster, 1);
                CHX_LOG_INFO("MasterCameraRole %d, Current Camera id %d, Current Camera role %d, isMaster %d",
                             masterCameraRole, m_camInfo[camIdx].cameraId, m_camInfo[camIdx].cameraRole, isMaster);

                if (CDKResultSuccess != result)
                {
                    CHX_LOG_ERROR("Failed to set metadata tag for MasterInfo for camera id %d ",
                                  m_camInfo[camIdx].cameraId);
                }
            }
        }

        // Add Sync info
        BOOL isSyncActive     = FALSE;
        BOOL isDualZoneActive = FALSE;
        BOOL isLPMEnabled     = FALSE;

        for (UINT32 i = 0; i < pMultiCamSettings->currentRequestMCCResult.numOfActiveCameras; i++)
        {
            //If 2 adjacent cameras are active, we are in Dual zone
            if(((i+1) < pMultiCamSettings->currentRequestMCCResult.numOfActiveCameras) &&
                (pMultiCamSettings->currentRequestMCCResult.activeCameras[i].isActive) &&
                (pMultiCamSettings->currentRequestMCCResult.activeCameras[i + 1].isActive))
            {
                isDualZoneActive = TRUE;
            }
            if (FALSE == pMultiCamSettings->currentRequestMCCResult.activeCameras[i].isActive)
            {
                isLPMEnabled = TRUE;
            }
        }

        // Enable frame sync if override settings is enabled and if its in in DUAL ZONE
        if ((TRUE == pMultiCamSettings->kernelFrameSyncEnable) && (TRUE == isDualZoneActive))
        {
            isSyncActive = TRUE;
        }

        LowPowerModeInfo lowPowerMode;
        lowPowerMode.isLPMEnabled       = isLPMEnabled;
        lowPowerMode.isSlaveOperational = isDualZoneActive;

        CHX_LOG_INFO("isSyncActive %d", isSyncActive);

        for (UINT32 i = 0; i < pMultiCamSettings->currentRequestMCCResult.numOfActiveCameras; i++)
        {
            UINT32 camIdx = GetCameraIndex(pMultiCamSettings->currentRequestMCCResult.activeCameras[i].cameraId);
            CHX_ASSERT(INVALID_INDEX != camIdx);
            lowPowerMode.lowPowerMode[i].cameraRole = m_camInfo[camIdx].cameraRole;
            lowPowerMode.lowPowerMode[i].isEnabled  = pMultiCamSettings->currentRequestMCCResult.activeCameras[i].isActive ?
                FALSE: TRUE;

            CHX_LOG("LPM camera[%d] - %d, cameraRole %d ", i,
                    lowPowerMode.lowPowerMode[i].isEnabled,
                    lowPowerMode.lowPowerMode[i].cameraRole);
        }

        for (UINT32 i = 0; i < pMultiCamSettings->numSettingsBuffers; i++)
        {
            ChiMetadata* pMetadata = pMultiCamSettings->ppSettings[i];
            CHX_ASSERT(NULL != pMetadata);
            result = SetMetadata(pMetadata, MultiCamControllerManager::m_vendorTagSyncInfo,
                                 &isSyncActive, sizeof(SyncModeInfo));

            result = SetMetadata(pMetadata, MultiCamControllerManager::m_vendorTagLPMInfo,
                                 &lowPowerMode, sizeof(LowPowerModeInfo));
            CHX_LOG_INFO("LPM camera[%d] - %d ",i, lowPowerMode.lowPowerMode[i].isEnabled);
        }
    }
    return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::TranslateMeteringRegion
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WeightedRegion MultiFovController::TranslateMeteringRegion(
    WeightedRegion* pRegion,
    UINT32          cameraId)
{
    FLOAT fovRatio = 1.0f;

    WeightedRegion    regionInput = *pRegion;
    WeightedRegion    regionTrans = regionInput;
    PixelShift        pixelShift  = { 0 };
    CHIRECT           activeArray = { 0 };
    UINT32            camIdx;
    UINT32            primaryCamIdx;

    if (0 != (regionInput.xMax * regionInput.yMax))
    {
        // Acquire mutex to read spatial alignment shifts which are written by other thread
        m_pLock->Lock();

        primaryCamIdx = GetCameraIndex(m_primaryCamId);
        CHX_ASSERT(INVALID_INDEX != primaryCamIdx);

        if (m_primaryCamId == cameraId)
        {
            fovRatio    = 1.0f;
            pixelShift  = m_pixelShiftPreview;
            activeArray = m_camInfo[primaryCamIdx].activeArraySize;
        }
        else
        {
            camIdx = GetCameraIndex(cameraId);
            CHX_ASSERT(INVALID_INDEX != camIdx);
            fovRatio          = (m_camInfo[camIdx].zoom / m_camInfo[primaryCamIdx].zoom) * m_camInfo[camIdx].adjustedFovRatio;
            pixelShift.xShift = (m_camInfo[camIdx].adjustedFovRatio / m_zoomUser) * m_pixelShiftPreview.xShift;
            pixelShift.yShift = (m_camInfo[camIdx].adjustedFovRatio / m_zoomUser) * m_pixelShiftPreview.yShift;
            activeArray       = m_camInfo[camIdx].activeArraySize;;
        }
        m_pLock->Unlock();

        // ROI should be with respect to the corresponding active array
        FLOAT xScale = ((FLOAT)activeArray.width / m_camInfo[primaryCamIdx].activeArraySize.width);
        FLOAT yScale = ((FLOAT)activeArray.height / m_camInfo[primaryCamIdx].activeArraySize.height);

        regionInput.xMin *= xScale;
        regionInput.yMin *= yScale;
        regionInput.xMax *= xScale;
        regionInput.yMax *= yScale;

        INT32 regionWidth       = regionInput.xMax - regionInput.xMin + 1;
        INT32 regionHeight      = regionInput.yMax - regionInput.yMin + 1;
        INT32 regionTransWidth  = regionWidth * fovRatio;
        INT32 regionTransHeight = regionHeight * fovRatio;

        INT32 regionHorzDelta = (regionTransWidth - regionWidth) / 2;
        INT32 regionVertDelta = (regionTransHeight - regionHeight) / 2;

        regionTrans.xMin = regionInput.xMin - regionHorzDelta - pixelShift.xShift;
        regionTrans.yMin = regionInput.yMin - regionVertDelta - pixelShift.yShift;

        regionTrans.xMax = regionTrans.xMin + regionTransWidth - 1;
        regionTrans.yMax = regionTrans.yMin + regionTransHeight - 1;

        INT32 activeArrayWidth  = activeArray.width;
        INT32 activeArrayHeight = activeArray.height;

        // Check ROI bounds and correct it if necessary
        if ((regionTrans.xMin < 0) ||
            (regionTrans.yMin < 0) ||
            (regionTrans.xMax > activeArrayWidth) ||
            (regionTrans.yMax > activeArrayHeight))
        {
            if (regionTrans.xMin < 0)
            {
                regionTrans.xMin = 0;
            }
            if (regionTrans.yMin < 0)
            {
                regionTrans.yMin = 0;
            }
            if (regionTrans.xMax >= activeArrayWidth)
            {
                regionTrans.xMax = activeArrayWidth - 1;
            }
            if (regionTrans.yMax >= activeArrayHeight)
            {
                regionTrans.yMax = activeArrayHeight - 1;
            }
        }
    }

    return regionTrans;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::SetMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiFovController::SetMetadata(
    ChiMetadata *pMetaData,
    UINT32      tag,
    VOID*       pData,
    SIZE_T      count)
{
    return pMetaData->SetTag(tag, pData, count);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::FillOfflinePipelineInputMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiFovController::FillOfflinePipelineInputMetadata(
    MulticamResultMetadata* pMultiCamResultMetadata,
    ChiMetadata*                pOfflineMetadata,
    BOOL                        isSnapshotPipeline)
{
    CDKResult      result = CDKResultSuccess;
    CameraSettings cameraSettings[MaxDevicePerLogicalCamera];

    // Identify camera ID and metadata
    MultiCameraIdRole* pMultiCameraRole    = NULL;
    UINT32*            pCameraId;
    INT32*             pIsMaster;
    CameraRoleType     masterRole          = CameraRoleTypeMax;
    UINT32             masterCamId         = 0;
    UINT32             masterMetaIdx       = 0;
    UINT32             metadataIds[MaxCameras];
    ChiMetadata*       pMetadataArray[MaxCameras];
    UINT32             primaryMetaId;
    UINT32             metaIndex = 0;

    if (NULL == pMultiCamResultMetadata)
    {
        result = CDKResultEInvalidArg;
    }

    if (CDKResultSuccess == result)
    {
        for (UINT32 i = 0; i < pMultiCamResultMetadata->numResults; i++)
        {
            cameraSettings[i].pMetadata  = pMultiCamResultMetadata->ppResultMetadata[i];

            if (NULL != cameraSettings[i].pMetadata)
            {
                //Get camera ID from metadata
                pCameraId = static_cast <UINT32*>(cameraSettings[i].pMetadata->GetTag(
                    MultiCamControllerManager::m_vendorTagMetadataOwner));

                /// Get Current camera Role
                /// Todo: Not needed when metadata is updated with cameraid
                pMultiCameraRole = static_cast <MultiCameraIdRole*>(cameraSettings[i].pMetadata->GetTag(
                    MultiCamControllerManager::m_vendorTagMultiCameraRole));

                /// Check if current camera is SW master
                pIsMaster = static_cast<INT32*>(cameraSettings[i].pMetadata->GetTag(
                    MultiCamControllerManager::m_vendorTagMasterInfo));

                /// Extract Master camera info from metadata
                if ((NULL != pMultiCameraRole) && (NULL != pIsMaster) && (NULL != pCameraId))
                {
                    cameraSettings[i].cameraId = *pCameraId;
                    CHX_LOG_INFO("Current Camera Id %d, cuurent camera role %d, is SW master %d",
                        cameraSettings[i].cameraId, pMultiCameraRole->currentCameraRole, *pIsMaster);

                    metadataIds[metaIndex]    = static_cast<UINT32>(pMultiCameraRole->currentCameraId);
                    pMetadataArray[metaIndex] = cameraSettings[i].pMetadata;

                    if (TRUE == *pIsMaster)
                    {
                        masterRole    = pMultiCameraRole->currentCameraRole;
                        masterCamId   = cameraSettings[i].cameraId;
                        masterMetaIdx = i;
                        primaryMetaId = metadataIds[metaIndex];
                        CHX_LOG_INFO("Master Camera Role %d, Id %d, Meta Index %d primaryMetaId %d",
                                     masterRole, masterCamId, masterMetaIdx, primaryMetaId);
                    }

                    metaIndex++;
                }
                else
                {
                    CHX_LOG_ERROR("Metadata tag MultiCameraIdRole/ MetadataOwner/ isMaster is NULL");
                    result = CDKResultEFailed;
                }
            }
        }
    }

    if (CameraRoleTypeMax == masterRole)
    {
        CHX_LOG_ERROR("Get master camera role failed");
        result = CDKResultEFailed;
    }

    m_pLock->Lock();
    PixelShift spatialShift = m_pixelShiftSnapshot;
    m_pLock->Unlock();

    InputMetadataOpticalZoom metadataOpticalZoom;
    ChxUtils::Memset(&metadataOpticalZoom, 0, sizeof(InputMetadataOpticalZoom));

    if (NULL != pMultiCamResultMetadata)
    {
        if (CDKResultSuccess == result)
        {
            for (UINT32 i = 0; i < pMultiCamResultMetadata->numResults; i++)
            {
                if (NULL != cameraSettings[i].pMetadata)
                {
                    ExtractCameraMetadata(cameraSettings[i].pMetadata, &metadataOpticalZoom.cameraMetadata[i]);

                    UINT32 camIdx = GetCameraIndex(cameraSettings[i].cameraId);
                    CHX_ASSERT(INVALID_INDEX != camIdx);

                    metadataOpticalZoom.cameraMetadata[i].isValid          = TRUE;
                    metadataOpticalZoom.cameraMetadata[i].cameraRole       = m_camInfo[camIdx].cameraRole;
                    metadataOpticalZoom.cameraMetadata[i].cameraId         = m_camInfo[camIdx].cameraId;
                    metadataOpticalZoom.cameraMetadata[i].masterCameraRole = masterRole;
                    metadataOpticalZoom.cameraMetadata[i].masterCameraId   = masterCamId;
                    metadataOpticalZoom.cameraMetadata[i].fovRectIFE       = m_camInfo[camIdx].fovRectIFE;
                    metadataOpticalZoom.cameraMetadata[i].fovRectIPE       = m_camInfo[camIdx].fovRectIFE;
                    metadataOpticalZoom.cameraMetadata[i].activeArraySize  = m_camInfo[camIdx].activeArraySize;

                    CaptureRequestCropRegions* pCropRegions;
                    pCropRegions = static_cast <CaptureRequestCropRegions *>(cameraSettings[i].pMetadata->GetTag(
                                         MultiCamControllerManager::m_vendorTagCropRegions));

                    if (NULL != pCropRegions)
                    {
                        metadataOpticalZoom.cameraMetadata[i].userCropRegion     = pCropRegions->userCropRegion;
                        metadataOpticalZoom.cameraMetadata[i].pipelineCropRegion = pCropRegions->pipelineCropRegion;
                        metadataOpticalZoom.cameraMetadata[i].ifeLimitCropRegion = pCropRegions->ifeLimitCropRegion;
                        CHX_LOG("Camera id %d Pipeline cropregion:%d,%d,%d,%d", cameraSettings[i].cameraId,
                                pCropRegions->pipelineCropRegion.left, pCropRegions->pipelineCropRegion.top,
                                pCropRegions->pipelineCropRegion.width, pCropRegions->pipelineCropRegion.height);

                        if (TRUE == isSnapshotPipeline)
                        {
                            TranslatedZoom translatedZoom;
                            ChxUtils::Memset(&translatedZoom, 0, sizeof(translatedZoom));
                            m_pZoomTranslator->GetTranslatedZoom(&pCropRegions->userCropRegion, &translatedZoom);
                            ZoomRegions zoomSnapshot = translatedZoom.zoomSnapshot;

                            metadataOpticalZoom.cameraMetadata[i].userCropRegion = pCropRegions->userCropRegion;

                            UINT32 zoomIdx = GetZoomIndex(zoomSnapshot.cameraID, cameraSettings[i].cameraId);
                            CHX_ASSERT(INVALID_INDEX != zoomIdx);
                            metadataOpticalZoom.cameraMetadata[i].pipelineCropRegion = zoomSnapshot.totalZoom[zoomIdx];
                            metadataOpticalZoom.cameraMetadata[i].ifeLimitCropRegion = zoomSnapshot.ispZoom[zoomIdx];

                            pCropRegions->pipelineCropRegion = zoomSnapshot.totalZoom[zoomIdx];
                            pCropRegions->ifeLimitCropRegion = zoomSnapshot.ispZoom[zoomIdx];
                            SetMetadata(cameraSettings[i].pMetadata,
                                        MultiCamControllerManager::m_vendorTagCropRegions,
                                        pCropRegions, sizeof(CaptureRequestCropRegions));

                        }
                    }
                    else
                    {
                        CHX_LOG_ERROR("Metadata is NULL for tag CropRegions");
                    }
                }
                else
                {
                    CHX_LOG_ERROR("Camera metadata for cameraid %d is NULL", cameraSettings[i].cameraId);
                }
            }

            metadataOpticalZoom.frameId   = pMultiCamResultMetadata->frameNum;
            metadataOpticalZoom.numInputs = pMultiCamResultMetadata->numResults;
            metadataOpticalZoom.outputShiftSnapshot.horizonalShift = spatialShift.xShift;
            metadataOpticalZoom.outputShiftSnapshot.verticalShift  = spatialShift.yShift;

            result = pOfflineMetadata->Invalidate();
            result = pOfflineMetadata->MergeMultiCameraMetadata(pMultiCamResultMetadata->numResults,
                                                                pMetadataArray, metadataIds, primaryMetaId);
            CHX_LOG_INFO("Merge multicamera meta count %d primary %d", metaIndex, primaryMetaId);

            if (CDKResultSuccess != result)
            {
                CHX_LOG("Failure in append meta, dst meta size %ul src meta size %ul",
                        pOfflineMetadata->Count(),
                        cameraSettings[masterMetaIdx].pMetadata->Count());
            }

            if (CDKResultSuccess != SetMetadata(pOfflineMetadata,
                                                MultiCamControllerManager::m_vendorTagOpticalZoomInputMeta,
                                                &metadataOpticalZoom, sizeof(InputMetadataOpticalZoom)))
            {
                CHX_LOG("Failure in pSetMetaData of m_vendorTagOpticalZoomInputMeta");
            }

            /// Update master camera ID
            ChxUtils::FillCameraId(pOfflineMetadata, masterCamId);
        }
        else
        {
            CHAR metaFileName[MaxFileLen];
            for (UINT32 i = 0; i < pMultiCamResultMetadata->numResults; i++)
            {
                if (NULL != pMultiCamResultMetadata->ppResultMetadata[i])
                {
                    CdkUtils::SNPrintF(metaFileName, sizeof(metaFileName), "mcam_%d_%d.txt",
                                       cameraSettings[i].cameraId,
                                       pMultiCamResultMetadata->ppResultMetadata[i]->GetFrameNumber());
                    pMultiCamResultMetadata->ppResultMetadata[i]->DumpDetailsToFile(metaFileName);
                }
            }
            result = CamxResultEFailed;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::UpdateScalerCropForSnapshot
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiFovController::UpdateScalerCropForSnapshot(
    ChiMetadata* pMetadata)
{
    CDKResult result   = CDKResultSuccess;
    UINT32    cameraId;
    UINT32    camIdx;

    CaptureRequestCropRegions* pCropRegions;
    MultiCameraIdRole*         pMultiCameraRole;

    /// Get Camera Id for the metadata
    /// ToDo: Replace with GetCameraid from Metadata when cameraid is added to metadata instead of Role
    pMultiCameraRole = static_cast <MultiCameraIdRole*>(pMetadata->GetTag(
                         MultiCamControllerManager::m_vendorTagMultiCameraRole));

    if (NULL != pMultiCameraRole)
    {
        camIdx   = GetCameraIndexFromRole(pMultiCameraRole->currentCameraRole);
        CHX_ASSERT(INVALID_INDEX != camIdx);
        cameraId = m_camInfo[camIdx].cameraId;
    }
    else
    {
        CHX_LOG_ERROR("Metadata tag MultiCameraRole is NULL");
        result = CDKResultEFailed;
    }

    pCropRegions = static_cast <CaptureRequestCropRegions*>(pMetadata->GetTag(
        MultiCamControllerManager::m_vendorTagCropRegions));


    if (CDKResultSuccess == result)
    {
        //Get Translated zoom from zoom translator
        TranslatedZoom translatedZoom;
        ChxUtils::Memset(&translatedZoom, 0, sizeof(translatedZoom));
        m_pZoomTranslator->GetTranslatedZoom(&pCropRegions->userCropRegion, &translatedZoom);

        ZoomRegions zoomSnapshot = translatedZoom.zoomSnapshot;
        CHIRECT zoomTotal;
        CHIRECT zoomIspLimit;
        CHIRECT activeArraySize;

        //Get translated zoom values corresponsing to the cameraId
        UINT32 zoomIdx = GetZoomIndex(&zoomSnapshot.cameraID[0], cameraId);
        CHX_ASSERT(INVALID_INDEX != zoomIdx);

        zoomTotal       = zoomSnapshot.totalZoom[zoomIdx];
        zoomIspLimit    = zoomSnapshot.ispZoom[zoomIdx];
        activeArraySize = m_camInfo[camIdx].activeArraySize;

        /// Set the snapshot crop from zoom translator using the spatial shift to move the crop region
        INT32 updatedLeft = m_pixelShiftSnapshot.xShift + zoomTotal.left;
        INT32 updatedTop  = m_pixelShiftSnapshot.yShift + zoomTotal.top;

        if ((updatedLeft >= 0) &&
            ((updatedLeft + zoomTotal.width) < activeArraySize.width) &&
            (updatedTop >= 0) &&
            ((updatedTop + zoomTotal.height) < activeArraySize.height))
        {
            zoomTotal.left                   = updatedLeft;
            zoomTotal.top                    = updatedTop;
            pCropRegions->pipelineCropRegion = zoomTotal;
        }

        updatedLeft = m_pixelShiftSnapshot.xShift + zoomIspLimit.left;
        updatedTop  = m_pixelShiftSnapshot.yShift + zoomIspLimit.top;

        if ((updatedLeft >= 0) &&
            ((updatedLeft + zoomIspLimit.width) < activeArraySize.width) &&
            (updatedTop >= 0) &&
            ((updatedTop + zoomIspLimit.height) < activeArraySize.height))
        {
            zoomIspLimit.left                = updatedLeft;
            zoomIspLimit.top                 = updatedTop;
            pCropRegions->ifeLimitCropRegion = zoomIspLimit;
        }

        SetMetadata(pMetadata,
                    MultiCamControllerManager::m_vendorTagCropRegions, pCropRegions,
                    sizeof(CaptureRequestCropRegions));

        ///  Update the scaler crop region
        camera_metadata_entry_t entryCropRegion = { 0 };
        if (0 == FindMetadata(pMetadata, ANDROID_SCALER_CROP_REGION, &entryCropRegion))
        {
            CHIRECT cropRect;
            cropRect = { zoomTotal.left, zoomTotal.top, zoomTotal.width, zoomTotal.height };
            SetMetadata(pMetadata, ANDROID_SCALER_CROP_REGION, &cropRect, 4);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::UpdateResults
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiFovController::UpdateResults(
    ChiMetadata* pMetadata)
{
    if (NULL != pMetadata)
    {
        camera_metadata_entry_t entryCropRegion = { 0 };
        if (0 == pMetadata->FindTag(ANDROID_SCALER_CROP_REGION, &entryCropRegion))
        {
            CHIRECT userZoom;
            userZoom.left   = entryCropRegion.data.i32[0];
            userZoom.top    = entryCropRegion.data.i32[1];
            userZoom.width  = entryCropRegion.data.i32[2];
            userZoom.height = entryCropRegion.data.i32[3];

            m_zoomUser = m_camInfo[GetCameraIndex(m_primaryCamId)].activeArraySize.width /
                (FLOAT)userZoom.width;

            CHX_LOG_INFO("User Zoom %f, User Zoom Crop rect l %d, T %d, w %d, h %d",
                         m_zoomUser, userZoom.left, userZoom.top, userZoom.width, userZoom.height);
        }
        else
        {
            CHX_LOG_ERROR("Cannot find metadata ANDROID_SCALER_CROP_REGION in meta pointer %p", pMetadata);
        }
        SetInitialResultState();
    }
    CHX_LOG_INFO("Updated Master camera id: %d fusion: %d", m_result.masterCameraId, m_result.snapshotFusion);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::TranslateFaceRegions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiFovController::TranslateFaceRegions(
    VOID* pResultMetadata)
{
    OutputMetadataOpticalZoom metadataOpticalZoom;
    camera_metadata_entry_t   entry      = { 0 };
    camera_metadata_t         *pMetadata = static_cast<camera_metadata_t*>(pResultMetadata);
    CDKResult                 result     = CDKResultSuccess;

    if (CDKResultSuccess ==
        MultiCamControllerManager::s_vendorTagOps.pGetMetaData(pResultMetadata,
                                                               MultiCamControllerManager::m_vendorTagOpticalZoomResultMeta,
                                                               &metadataOpticalZoom, sizeof(OutputMetadataOpticalZoom)))
    {
        ///Todo Remove role once metadata is updated with camera id
        CameraRoleType camRole  = metadataOpticalZoom.masterCamera;
        UINT32         cameraId = GetCameraIdFromRole(camRole);
        if (InvalidPhysicalCameraId == cameraId)
        {
            CHX_LOG_ERROR("Invalid Camera ID %d, face translation is invalid", cameraId);
            result = CDKResultEFailed;
        }

        if (0 == find_camera_metadata_entry(pMetadata, ANDROID_STATISTICS_FACE_RECTANGLES, &entry) &&
            (CDKResultSuccess == result))
        {
            if (entry.count > 0)
            {
                UINT32  numElemsRect = sizeof(CHIRECT) / sizeof(UINT32);
                INT32   numFaces = entry.count / numElemsRect;
                UINT32  dataIndex = 0;
                INT32   numVisibleFaces = 0;
                CHIRECT faceRegions[FDMaxFaces];

                // Get the user zoom to remove the faces which are outside of preview FOV
                CaptureRequestCropRegions cropRegions;
                MultiCamControllerManager::s_vendorTagOps.pGetMetaData(pMetadata,
                                                                       MultiCamControllerManager::m_vendorTagCropRegions,
                                                                       &cropRegions, sizeof(CaptureRequestCropRegions));

                for (INT32 i = 0; i < numFaces; ++i)
                {
                    UINT32 xMin = entry.data.i32[dataIndex++];
                    UINT32 yMin = entry.data.i32[dataIndex++];
                    UINT32 xMax = entry.data.i32[dataIndex++];
                    UINT32 yMax = entry.data.i32[dataIndex++];

                    CHX_LOG("Face rectangle from camera_metadata: l_t_r_b_(%d, %d, %d, %d), ratio = %f.",
                            xMin, yMin, xMax, yMax,
                            (m_camInfo[GetCameraIndex(m_primaryCamId)].activeArraySize.width * 1.0f /
                             cropRegions.userCropRegion.width));

                    CHIRECTEXT pUserFace       = {xMin,yMin,xMax,yMax};
                    CHIRECTEXT pTranslatedFace = {0};

                    m_pZoomTranslator->GetTranslatedRect(metadataOpticalZoom,cropRegions.userCropRegion,pUserFace,pTranslatedFace);
                    xMin = pTranslatedFace.left;
                    yMin = pTranslatedFace.top;
                    xMax = pTranslatedFace.right;
                    yMax = pTranslatedFace.bottom;

                    CHX_LOG("Face rectangle after translated: l_t_r_b_(%d, %d, %d, %d)",
                            xMin, yMin, xMax, yMax);

                    CHIRECT      alignedCropRegion;
                    CHIDimension previewDim;
                    CHIDimension cropDim;

                    previewDim.width  = m_previewDimensions.width;
                    previewDim.height = m_previewDimensions.height;
                    cropDim.width     = cropRegions.userCropRegion.width;
                    cropDim.height    = cropRegions.userCropRegion.height;

                    ChxUtils::MatchAspectRatio(&previewDim, &cropDim);

                    alignedCropRegion.width  = cropDim.width;
                    alignedCropRegion.height = cropDim.height;
                    alignedCropRegion.left   = cropRegions.userCropRegion.left +
                        ((cropRegions.userCropRegion.width - alignedCropRegion.width) / 2);
                    alignedCropRegion.top    = cropRegions.userCropRegion.top +
                        ((cropRegions.userCropRegion.height - alignedCropRegion.height) / 2);

                    // Reject FD ROIs outside the user cropped region
                    if ((xMin >= alignedCropRegion.left) && (xMax <= alignedCropRegion.left + alignedCropRegion.width) &&
                        (yMin >= alignedCropRegion.top) && (yMax <= alignedCropRegion.top + alignedCropRegion.height))
                    {
                        faceRegions[numVisibleFaces].left   = xMin;
                        faceRegions[numVisibleFaces].top    = yMin;
                        faceRegions[numVisibleFaces].width  = xMax;
                        faceRegions[numVisibleFaces].height = yMax;
                        numVisibleFaces++;
                    }
                }

                if (numVisibleFaces > 0)
                {
                    UpdateMetadata(pMetadata, entry.index, faceRegions,
                                   numVisibleFaces * sizeof(CHIRECT) / sizeof(INT32), NULL);
                }
                else
                {
                    INT32 status = delete_camera_metadata_entry(pMetadata, entry.index);

                    if (CDKResultSuccess == status)
                    {
                        status = add_camera_metadata_entry(pMetadata, ANDROID_STATISTICS_FACE_RECTANGLES, faceRegions, 0);
                    }

                    if (CDKResultSuccess != status)
                    {
                        CHX_LOG_ERROR("Failed to delete/ add metadata entry for no face case");
                    }
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::TranslateResultMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiFovController::TranslateResultMetadata(
    camera_metadata_t* pResultMetadata)
{
    PGETMETADATA    pGetMetaData = MultiCamControllerManager::s_vendorTagOps.pGetMetaData;
    CDKResult       result       = CDKResultSuccess;

    OutputMetadataOpticalZoom metadataOpticalZoom;

    result = pGetMetaData(pResultMetadata,
                          MultiCamControllerManager::m_vendorTagOpticalZoomResultMeta,
                          &metadataOpticalZoom, sizeof(OutputMetadataOpticalZoom));

    if (CDKResultSuccess == result)
    {
        CaptureRequestCropRegions cropRegions;
        result = pGetMetaData(pResultMetadata,
                              MultiCamControllerManager::m_vendorTagCropRegions,
                              &cropRegions, sizeof(CaptureRequestCropRegions));

        if (CDKResultSuccess == result)
        {
            camera_metadata_entry_t entryCropRegion = { 0 };

            // Translate Crop Regions
            if (0 == find_camera_metadata_entry(pResultMetadata, ANDROID_SCALER_CROP_REGION, &entryCropRegion))
            {
                CHIRECT androidCrop;
                androidCrop.left   = entryCropRegion.data.i32[0];
                androidCrop.top    = entryCropRegion.data.i32[1];
                androidCrop.width  = entryCropRegion.data.i32[2];
                androidCrop.height = entryCropRegion.data.i32[3];

                // Convert CHIRECT to CHIRECTEXT
                CHIRECTEXT srcRect;
                srcRect.left   = androidCrop.left;
                srcRect.top    = androidCrop.top;
                srcRect.right  = androidCrop.width  + androidCrop.left;
                srcRect.bottom = androidCrop.height + androidCrop.top;

                CHIRECTEXT dstRect = { 0 };

                m_pZoomTranslator->GetTranslatedRect(metadataOpticalZoom, cropRegions.userCropRegion, srcRect, dstRect);

                CHX_LOG("EntryCrop [w:%d h:%d] srcRect [l:%d t:%d r:%d b:%d] dstRect [l:%d t:%d r:%d b:%d]",
                        androidCrop.width,
                        androidCrop.height,
                        srcRect.left,
                        srcRect.top,
                        srcRect.right,
                        srcRect.bottom,
                        dstRect.left,
                        dstRect.top,
                        dstRect.right,
                        dstRect.bottom);

                // Convert back to framework format
                androidCrop.left   = dstRect.left;
                androidCrop.top    = dstRect.top;
                androidCrop.width  = dstRect.right  - dstRect.left;
                androidCrop.height = dstRect.bottom - dstRect.top;

                // Update Android Tag
                UpdateMetadata(pResultMetadata, entryCropRegion.index, &androidCrop, sizeof(CHIRECT) / sizeof(INT32), NULL);

            }

            // Translate 3A Stats
            for (UINT32 tag : { ANDROID_CONTROL_AF_REGIONS, ANDROID_CONTROL_AWB_REGIONS, ANDROID_CONTROL_AE_REGIONS })
            {
                camera_metadata_entry_t entryStatRegion;

                if (0 == find_camera_metadata_entry(pResultMetadata, tag, &entryStatRegion))
                {
                    // 3A Stat tag tuple has 5 elements [xMin, yMin, xMax, yMax, weight]
                    const SIZE_T& tupleSize = sizeof(WeightedRegion);
                    CHIRECTEXT    dstRect;

                    for (UINT32 index = 0; index < entryStatRegion.count; index += tupleSize)
                    {
                        WeightedRegion* pStatRegion     = reinterpret_cast<WeightedRegion*>(&entryStatRegion.data.i32[index]);
                        CHIRECTEXT*     pSrcRect        = reinterpret_cast<CHIRECTEXT*>(&entryStatRegion.data.i32[index]);
                        const CHAR*     tagName         = get_camera_metadata_tag_name(tag);

                        m_pZoomTranslator->GetTranslatedRect(metadataOpticalZoom,
                                                             cropRegions.userCropRegion,
                                                             *pSrcRect,
                                                             dstRect);

                        CHX_LOG("Translated 3A tag:%s srcRect [l:%d t:%d r:%d b:%d] dstRect [l:%d t:%d r:%d b:%d]",
                                tagName,
                                pSrcRect->left,
                                pSrcRect->top,
                                pSrcRect->right,
                                pSrcRect->bottom,
                                dstRect.left,
                                dstRect.top,
                                dstRect.right,
                                dstRect.bottom);

                        pStatRegion->xMin = dstRect.left;
                        pStatRegion->yMin = dstRect.top;
                        pStatRegion->xMax = dstRect.right;
                        pStatRegion->yMax = dstRect.bottom;

                        // Update the metadata
                        UpdateMetadata(pResultMetadata,
                                       entryStatRegion.index,
                                       pStatRegion,
                                       sizeof(WeightedRegion) / sizeof(INT32),
                                       NULL);
                    }
                }
            }
        }

        MultiCamController::TranslateResultMetadata(pResultMetadata);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiFovController::CheckOverrideMccResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiFovController::CheckOverrideMccResult(
    FLOAT userzoom)
{
    UINT32    newMasterId;
    UINT32    newMasterIdx;
    CDKResult result           = CDKResultSuccess;
    UINT32    currentMasterId  = m_result.masterCameraId;
    UINT32    currentmasterIdx = GetCameraIndex(currentMasterId);
    BOOL      masterUpdated    = FALSE;

    if (INVALID_INDEX == currentmasterIdx)
    {
        result = CDKResultEFailed;
        CHX_LOG_ERROR("Invalid camera index for camera id %d", currentMasterId);
    }

    if (CDKResultSuccess == result)
    {
        for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
        {
            FLOAT minFovRatio = m_camInfo[i].transitionLeft.transitionRatio;
            FLOAT maxFovRatio = m_camInfo[i].transitionRight.transitionRatio;

            if ((minFovRatio <= userzoom) && (maxFovRatio > userzoom))
            {
                CHX_LOG_INFO("MinFov %f Max Fov %f, userzoom %f, camera %d, current master %d",
                             minFovRatio, maxFovRatio, userzoom, m_camInfo[i].cameraId,
                             currentMasterId);
                if (currentMasterId != m_camInfo[i].cameraId)
                {
                    /// Transition into camera with smooth transition disabled
                    if ((FALSE == m_camInfo[i].transitionLeft.smoothTransitionEnabled) &&
                        (FALSE == m_camInfo[i].transitionRight.smoothTransitionEnabled))
                    {
                        newMasterId    = m_camInfo[i].cameraId;
                        newMasterIdx   = i;
                        masterUpdated  = TRUE;
                        m_overrideMode = TRUE;
                        CHX_LOG_CONFIG("Transitioning from camera %d to camera %d, Override mode %d",
                                     currentMasterId, newMasterId, m_overrideMode);

                    }
                    /// Transitioning out of camera with smooth transition disabled
                    else if ((FALSE == m_camInfo[currentmasterIdx].transitionLeft.smoothTransitionEnabled) &&
                        (FALSE == m_camInfo[currentmasterIdx].transitionRight.smoothTransitionEnabled))
                    {
                        newMasterId    = m_camInfo[i].cameraId;
                        newMasterIdx   = i;
                        masterUpdated  = TRUE;
                        m_overrideMode = TRUE;
                        CHX_LOG_CONFIG("Transitioning from camera %d to camera %d, Override mode %d",
                                     currentMasterId, newMasterId, m_overrideMode);
                    }
                    else
                    {
                        CHX_LOG_VERBOSE("Master not updated though zoom region mismatch");
                        m_overrideMode = FALSE;
                    }
                }
                else
                {
                    m_overrideMode = FALSE;
                }
            }
        }

        if (TRUE == masterUpdated)
        {
            m_result.masterCameraId       = newMasterId;
            m_result.masterCameraRole     = m_camInfo[newMasterIdx].cameraRole;
            m_recommendedMasterCameraRole = m_result.masterCameraRole;
            m_recommendedMasterCameraId   = m_result.masterCameraId;

            CHX_LOG_INFO("Overrding MCC result, Camera switch camera %d -> camera %d",
                              -currentMasterId, newMasterId);

            /// Reset the shift values since they are no longer valid
            m_pixelShiftPreview  = { 0 };
            m_pixelShiftSnapshot = { 0 };

            //Update activ cameras
            for (UINT32 i = 0; i < m_result.numOfActiveCameras; i++)
            {
                if ((m_result.activeCameras[i].cameraId == newMasterId)  ||
                    (TRUE == m_camInfo[i].alwaysOn))
                {
                    m_result.activeCameras[i].isActive = 1;
                }
                else
                {
                    m_result.activeCameras[i].isActive = 0;
                }
                CHX_LOG("Master Updated! Camera id %d, active %d", m_result.activeCameras[i].cameraId,
                         -m_result.activeCameras[i].isActive);
            }

            if ((FALSE == ENABLE_LPM) && (DualCamCount == m_numOfLinkedSessions))
            {
                for (UINT32 i = 0; i < m_result.numOfActiveCameras; i++)
                {
                    m_result.activeCameras[i].isActive = TRUE;
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::~MultiRTBController
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MultiRTBController::~MultiRTBController()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::Destroy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiRTBController::Destroy()
{
    if (NULL != m_pLock)
    {
        m_pLock->Destroy();
        m_pLock = NULL;
    }

    CHX_DELETE this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::Create
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MultiRTBController* MultiRTBController::Create(
    MccCreateData* pMccCreateData)
{
    CDKResult result = CDKResultSuccess;
    CHX_ASSERT(MaxDevicePerLogicalCamera >= pMccCreateData->numBundledCameras);
    CHX_ASSERT(InvalidPhysicalCameraId   != pMccCreateData->primaryCameraId);

    MultiRTBController* pMultiRTBController = CHX_NEW MultiRTBController;

    if (NULL != pMultiRTBController)
    {
        pMultiRTBController->m_pLock = Mutex::Create();

        if (NULL == pMultiRTBController->m_pLock)
        {
            result = CDKResultENoMemory;
            CHX_LOG_ERROR("Out of memory");
        }

        if (CDKResultSuccess == result)
        {
            pMultiRTBController->m_cameraType          = LogicalCameraType_RTB;
            pMultiRTBController->m_numOfLinkedSessions = pMccCreateData->numBundledCameras;
            pMultiRTBController->m_camIdLogical        = pMccCreateData->logicalCameraId;
            pMultiRTBController->m_primaryCamId        = pMccCreateData->primaryCameraId;

            // Fill in per camera info
            for (UINT32 i = 0; i < pMultiRTBController->m_numOfLinkedSessions; i++)
            {
                CamInfo *pCamInfo = pMccCreateData->pBundledCamInfo[i];

                // Get focal length, pixel size, sensor dimensions and active pixel array size
                pMultiRTBController->m_camInfo[i].focalLength                  = pCamInfo->pChiCamInfo->lensCaps.focalLength;
                pMultiRTBController->m_camInfo[i].pixelPitch                   = pCamInfo->pChiCamInfo->sensorCaps.pixelSize;
                pMultiRTBController->m_camInfo[i].sensorOutDimension           = pCamInfo->sensorOutDimension;
                pMultiRTBController->m_camInfo[i].activeArraySize              = pCamInfo->pChiCamInfo->sensorCaps.activeArray;
                pMultiRTBController->m_camInfo[i].fovRectIFE                   = pCamInfo->fovRectIFE;
                pMultiRTBController->m_camInfo[i].cameraId                     = pCamInfo->camId;
                pMultiRTBController->m_camInfo[i].cameraRole                   = pCamInfo->pChiCamConfig->cameraRole;
                pMultiRTBController->m_camInfo[i].transitionZoomRatioMin       = pCamInfo->pChiCamConfig->transitionZoomRatioMin;
                pMultiRTBController->m_camInfo[i].transitionZoomRatioMax       = pCamInfo->pChiCamConfig->transitionZoomRatioMax;

                for (UINT32 otpIndx = 0; otpIndx < pMultiRTBController->m_numOfLinkedSessions; otpIndx++)
                {
                    if (i == otpIndx)
                    {
                        pMultiRTBController->m_camInfo[i].otpData[otpIndx].pRawOtpData    = NULL;
                        pMultiRTBController->m_camInfo[i].otpData[otpIndx].rawOtpDataSize = 0;
                    }
                    pMultiRTBController->m_camInfo[i].otpData[otpIndx].pRawOtpData =
                        pCamInfo->pChiCamInfo->sensorCaps.pRawOTPData;
                    pMultiRTBController->m_camInfo[i].otpData[otpIndx].rawOtpDataSize =
                        pCamInfo->pChiCamInfo->sensorCaps.rawOTPDataSize;
                    pMultiRTBController->m_camInfo[i].otpData[otpIndx].refCameraId =
                        pMultiRTBController->m_camInfo[otpIndx].cameraId;
                }
            }

            result = pMultiRTBController->CalculateTransitionParams();
        }

        if (CDKResultSuccess != result)
        {
            // Destroy the object in case of failure
            pMultiRTBController->Destroy();
            pMultiRTBController = NULL;
        }
    }

    return pMultiRTBController;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::CalculateTransitionParams
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiRTBController::CalculateTransitionParams()
{
    CDKResult result        = CDKResultSuccess;
    UINT32    primaryCamIdx = 0;
    FLOAT     primaryFov    = 0;
    //Get Primary camera index
    for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
    {
        if (m_camInfo[i].cameraId == m_primaryCamId)
        {
            primaryCamIdx = i;
            break;
        }
    }

    if(0 < m_camInfo[primaryCamIdx].focalLength)
    {
        primaryFov = m_camInfo[primaryCamIdx].sensorOutDimension.width * m_camInfo[primaryCamIdx].pixelPitch /
            m_camInfo[primaryCamIdx].focalLength;
    }
    else
    {
        CHX_LOG_ERROR("Invalid Focal length for primary camera id %d", m_camInfo[primaryCamIdx].cameraId);
        result = CDKResultEFailed;
    }

    //Calculate per camera FOV
    for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
    {
        FLOAT fov = 0;

        if (0 < m_camInfo[i].focalLength)
        {
            fov = m_camInfo[i].sensorOutDimension.width * m_camInfo[i].pixelPitch /
                  m_camInfo[i].focalLength;
        }
        else
        {
            CHX_LOG_ERROR("Invalid Focal length for camera id %d", m_camInfo[i].cameraId);
        }

        if ((fov > 0.0f)&& (0.0F < primaryFov))
        {
            m_camInfo[i].adjustedFovRatio = primaryFov / fov;
        }
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::SetInitialResultState //////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiRTBController::SetInitialResultState()
{
    m_result.numOfActiveCameras = m_numOfLinkedSessions;

    for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
    {
        m_result.activeCameras[i].cameraId = m_camInfo[i].cameraId;
        m_result.activeCameras[i].isActive = FALSE;
    }

    if (DualCamCount == m_numOfLinkedSessions)
    {
        if (m_camInfo[0].transitionZoomRatioMin < m_camInfo[1].transitionZoomRatioMin)
        {
            m_result.masterCameraId              = m_camInfo[1].cameraId;
            m_result.masterCameraRole            = m_camInfo[1].cameraRole;
        }
        else
        {
            m_result.masterCameraId              = m_camInfo[0].cameraId;
            m_result.masterCameraRole            = m_camInfo[0].cameraRole;
        }
        m_result.activeCameras[0].isActive   = TRUE;
        m_result.activeCameras[1].isActive   = TRUE;
    }
    else if (TripleCamCount == m_numOfLinkedSessions)
    {
        // Fov sort
        UINT32 FovOderIndexArry[3] = {0};
        for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
        {
            FovOderIndexArry[i] = i;
        }
        for (UINT32 i = 0; i < m_numOfLinkedSessions-1; i++)
        {
            for (UINT32 j = 0; j < m_numOfLinkedSessions-1-i; j++)
            {
                if (m_camInfo[j].transitionZoomRatioMin > m_camInfo[j+1].transitionZoomRatioMin)
                {
                    UINT32 temp           = FovOderIndexArry[j];
                    FovOderIndexArry[j]   = FovOderIndexArry[j+1];
                    FovOderIndexArry[j+1] = temp;
                }
            }
        }

        //set init master
        if (m_zoomUser < 2)
        {
            m_result.masterCameraId                                = m_camInfo[FovOderIndexArry[1]].cameraId;
            m_result.masterCameraRole                              = m_camInfo[FovOderIndexArry[1]].cameraRole;
            m_result.activeCameras[FovOderIndexArry[0]].isActive   = TRUE;
            m_result.activeCameras[FovOderIndexArry[1]].isActive   = TRUE;
        }
        else
        {
            m_result.masterCameraId                                = m_camInfo[FovOderIndexArry[2]].cameraId;
            m_result.masterCameraRole                              = m_camInfo[FovOderIndexArry[2]].cameraRole;
            m_result.activeCameras[FovOderIndexArry[1]].isActive   = TRUE;
            m_result.activeCameras[FovOderIndexArry[2]].isActive   = TRUE;
        }
    }
    else
    {
        CHX_LOG_ERROR("Not support more than 3 camera now");
    }


    m_result.activeMap = 0;
    //Update the Active map
    for (UINT32 i = 0; i < m_result.numOfActiveCameras; i++)
    {
        if (TRUE == m_result.activeCameras[i].isActive)
        {
            m_result.activeMap |= 1 << i;
        }
    }

    m_recommendedMasterCameraRole      = m_result.masterCameraRole;
    m_recommendedMasterCameraId        = m_result.masterCameraId;

    m_result.snapshotFusion = TRUE;
    m_result.isValid        = TRUE;

    for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
    {
        CHX_LOG_INFO("Active cameras camera id[%d], index [%d] Active[%d]", m_result.activeCameras[i].cameraId, i,
            m_result.activeCameras[i].isActive);
    }
    CHX_LOG_INFO("Master camera id: %d fusion: %d", m_result.masterCameraId, m_result.snapshotFusion);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::Reconfigure
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiRTBController::Reconfigure(
    MccCreateData* pMccCreateData)
{
    CDKResult   result    = CDKResultSuccess;
    CamInfo    *pCamInfo  = NULL;

    CHX_ASSERT(MaxDevicePerLogicalCamera >= pMccCreateData->numBundledCameras);

    m_numOfLinkedSessions = pMccCreateData->numBundledCameras;
    m_camIdLogical        = pMccCreateData->logicalCameraId;

    // Get sensor output dimensions
    for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
    {
        CamInfo    *pCamInfo            = pMccCreateData->pBundledCamInfo[i];
        m_camInfo[i].sensorOutDimension = pCamInfo->sensorOutDimension;
    }

    result = CalculateTransitionParams();

    return result;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::GetResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ControllerResult MultiRTBController::GetResult(
    ChiMetadata* pMetadata)
{
    (void)pMetadata;
    m_pLock->Lock();

    ControllerResult result = m_result;
    if (m_result.masterCameraId != m_recommendedMasterCameraId)
    {
        m_result.masterCameraId   = m_recommendedMasterCameraId;
        m_result.masterCameraRole = m_recommendedMasterCameraRole;
        //Update the Active camera
        for (UINT32 i = 0; i < m_result.numOfActiveCameras; i++)
        {
            CHX_LOG_VERBOSE("activemap : %d", m_result.activeMap);
            if (TRUE == ChxUtils::IsBitSet(m_result.activeMap, i))
            {
                m_result.activeCameras[i].isActive = TRUE;
            }
            else
            {
                m_result.activeCameras[i].isActive = FALSE;
            }
        }
    }

    for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
    {
        CHX_LOG_INFO("Active cameras camera id[%d], index [%d] Active[%d]", m_result.activeCameras[i].cameraId, i,
            m_result.activeCameras[i].isActive);
    }
    m_pLock->Unlock();

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::GetCameraIdFromRole
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 MultiRTBController::GetCameraIdFromRole(
    CameraRoleType camRole)
{
    UINT32 camID = InvalidPhysicalCameraId;

    for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
    {
        if (m_camInfo[i].cameraRole == camRole)
        {
            camID = m_camInfo[i].cameraId;
            break;
        }
    }
    return camID;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::GetCameraIdFromRole
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 MultiRTBController::GetCameraIndexFromRole(
    CameraRoleType camRole)
{
    UINT32 camIndex = INVALID_INDEX;

    for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
    {
        if (m_camInfo[i].cameraRole == camRole)
        {
            camIndex = i;
            break;
        }
    }
    return camIndex;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::GetCameraIndex
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 MultiRTBController::GetCameraIndex(
    UINT32 cameraID)
{
    UINT32 camIndex = INVALID_INDEX;
    for (UINT32 i = 0; i < m_numOfLinkedSessions; i++)
    {
        if (m_camInfo[i].cameraId == cameraID)
        {
            camIndex = i;
            break;
        }
    }
    return camIndex;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::TranslateRequestSettings
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiRTBController::TranslateRequestSettings(
    MulticamReqSettings* pMultiCamSettings)
{
    CDKResult result = CDKResultSuccess;

    CameraSettings primarySettings;
    CameraSettings translatedSettings[MaxDevicePerLogicalCamera - 1];
    UINT32         primaryCamIdx = 0;
    ChxUtils::Memset(&primarySettings, 0, sizeof(primarySettings));
    ChxUtils::Memset(translatedSettings, 0, sizeof(translatedSettings));

    if (NULL != pMultiCamSettings)
    {
        CHX_ASSERT(m_numOfLinkedSessions == pMultiCamSettings->numSettingsBuffers);

        UINT32 translatedMetacount = 0;
        for (UINT32 i = 0; i < pMultiCamSettings->numSettingsBuffers; i++)
        {
            ChiMetadata *pMetadata = pMultiCamSettings->ppSettings[i];

            if(NULL != pMetadata)
            {
                //Get CameraID from metadata
                UINT32      *pCameraId = static_cast <UINT32*>(pMetadata->GetTag(
                    MultiCamControllerManager::m_vendorTagMetadataOwner));

                if (NULL != pCameraId)
                {
                    if (m_primaryCamId == *pCameraId)
                    {
                        primarySettings.cameraId  = *pCameraId;
                        primarySettings.pMetadata =  pMetadata;
                        primaryCamIdx             =  GetCameraIndex(primarySettings.cameraId);
                        if (NULL == primarySettings.pMetadata)
                        {
                            CHX_LOG_ERROR("Primary metadata is null");
                            result = CDKResultEFailed;
                        }
                        CHX_ASSERT(INVALID_INDEX != primaryCamIdx);
                    }
                    else
                    {
                        translatedSettings[translatedMetacount].cameraId  = *pCameraId;
                        translatedSettings[translatedMetacount].pMetadata =  pMetadata;
                        if (NULL == translatedSettings[translatedMetacount].pMetadata)
                        {
                            CHX_LOG_ERROR("translatedSettings[%d] metadata is null", translatedMetacount);
                            result = CDKResultEFailed;
                        }
                        translatedMetacount++;
                    }
                }
                else
                {
                    CHX_LOG_ERROR("Metadata tag MetadataOwner is NULL. Cannot get cameraId");
                    result = CDKResultEFailed;
                }
            }
            else
            {
                CHX_LOG_ERROR("Metadata from setting buffers is NULL");
                result = CDKResultEFailed;
            }
        }

        if(NULL == primarySettings.pMetadata)
        {
            CHX_LOG_ERROR("Primary setting metadata is NULL");
            result = CDKResultEFailed;
        }

        if (CDKResultSuccess == result)
        {
            result = TranslateReferenceCropWindow(&primarySettings, translatedSettings,
                                                   translatedMetacount, primaryCamIdx);
        }

        camera_metadata_entry_t entryCropRegion = { 0 };
        camera_metadata_entry_t entryCropRegionTrans[MaxDevicePerLogicalCamera - 1] = { { 0 } };
        INT isPrimaryMetaFound = 1;
        INT isAuxMetaFound     = 1;
        if (CDKResultSuccess == result)
        {
            isPrimaryMetaFound = FindMetadata(primarySettings.pMetadata,
                                              ANDROID_SCALER_CROP_REGION, &entryCropRegion);

            for (UINT32 i = 0; i < translatedMetacount; i++)
            {
                isAuxMetaFound = FindMetadata(translatedSettings[i].pMetadata,
                                              ANDROID_SCALER_CROP_REGION, &entryCropRegionTrans[i]);
                if (0 != isAuxMetaFound)
                {
                    break;
                }
            }
            if ((0 == isPrimaryMetaFound) && (0 == isAuxMetaFound))
            {
                // ANDROID_SCALER_CROP_REGION rectangle should be in reference with sensor Active Array
                // excluding dummy pixels. So, top and left starts with (0, 0) for crop window.
                CHIRECT pScalerCropWindowPrimary;
                CHIRECT scalerCropWindows[MaxDevicePerLogicalCamera - 1];

                pScalerCropWindowPrimary      = m_camInfo[primaryCamIdx].activeArraySize;
                pScalerCropWindowPrimary.top  = 0;
                pScalerCropWindowPrimary.left = 0;

                SetMetadata(primarySettings.pMetadata, ANDROID_SCALER_CROP_REGION,
                            &pScalerCropWindowPrimary, 4);

                for (UINT i = 0; i < translatedMetacount; i++)
                {
                    UINT32 camIdx = GetCameraIndex(translatedSettings[i].cameraId);
                    CHX_ASSERT(INVALID_INDEX != camIdx);
                    scalerCropWindows[i]      = m_camInfo[camIdx].activeArraySize;
                    scalerCropWindows[i].top  = 0;
                    scalerCropWindows[i].left = 0;

                    // Update the metadata
                    SetMetadata(translatedSettings[i].pMetadata, ANDROID_SCALER_CROP_REGION,
                                &scalerCropWindows[i], 4);
                }
            }
            else
            {
                CHX_LOG_WARN("No crop region information in this metadata!");
            }
        }
        else
        {
            CHX_LOG_ERROR("Translate Reference CropWindow failed");
        }

        if (CDKResultSuccess == result)
        {
            result = TranslateROI(&primarySettings, translatedSettings, translatedMetacount);
        }
        else
        {
            CHX_LOG_ERROR("Cannot Translate ROI");
        }

        // Update all the common vendor tags
        UpdateVendorTags(pMultiCamSettings);

    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::TranslateReferenceCropWindow
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiRTBController::TranslateReferenceCropWindow(
    CameraSettings *primarySettings,
    CameraSettings *ptranslatedSettings,
    UINT32         numOfTranslatedSettings,
    UINT32         primaryCamIdx)
{
    CDKResult result = CDKResultSuccess;

    RefCropWindowSize pRefCropWindowPrimary;
    RefCropWindowSize refCropWindowTranslated[MaxDevicePerLogicalCamera - 1];

    if (primaryCamIdx >= MaxDevicePerLogicalCamera)
    {
        result = CDKResultEOutOfBounds;
        return result;
    }

    pRefCropWindowPrimary.width  = m_camInfo[primaryCamIdx].activeArraySize.width;
    pRefCropWindowPrimary.height = m_camInfo[primaryCamIdx].activeArraySize.height;
    CHX_LOG("Primary RefCropSize w:%d h:%d", pRefCropWindowPrimary.width, pRefCropWindowPrimary.height);

    for (UINT32 i = 0; i < numOfTranslatedSettings; i++)
    {
        UINT32 camIdx = GetCameraIndex(ptranslatedSettings[i].cameraId);
        CHX_ASSERT(INVALID_INDEX != camIdx);

        refCropWindowTranslated[i].width  = m_camInfo[camIdx].activeArraySize.width;
        refCropWindowTranslated[i].height = m_camInfo[camIdx].activeArraySize.height;
        CHX_LOG("Translate RefCropSize w:%d h:%d", refCropWindowTranslated[i].width, refCropWindowTranslated[i].height);
        if (NULL != ptranslatedSettings[i].pMetadata)
        {
            result = SetMetadata(ptranslatedSettings[i].pMetadata, MultiCamControllerManager::m_vendorTagReferenceCropSize,
                                &refCropWindowTranslated[i], sizeof(RefCropWindowSize));
        }
        else
        {
            CHX_LOG_ERROR("translated camera Metadata is NULL");
        }
        if (CDKResultSuccess != result)
        {
            CHX_LOG_ERROR("Failed to set metadata for tag Reference Crop size");
        }
    }

    if (NULL != primarySettings->pMetadata)
    {
        result = SetMetadata(primarySettings->pMetadata, MultiCamControllerManager::m_vendorTagReferenceCropSize,
                             &pRefCropWindowPrimary, sizeof(RefCropWindowSize));
    }
    else
    {
        CHX_LOG_ERROR("Primary camera Metadata is NULL");
    }

    if (CDKResultSuccess != result)
    {
        CHX_LOG_ERROR("Failed to set metadata for tag Reference Crop size");
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::TranslateROI
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiRTBController::TranslateROI(
    CameraSettings *primarySettings,
    CameraSettings *ptranslatedSettings,
    UINT32         numOfTranslatedSettings)
{
    CDKResult result = CDKResultSuccess;
    INT isPrimayMetaPresent = 1, isAuxMetaPresent = 1;

    // Translate the Focus ROI
    camera_metadata_entry_t entryAFRegionMain = { 0 };
    camera_metadata_entry_t entryAFRegionAux[MaxDevicePerLogicalCamera - 1] = { { 0 } };

    isPrimayMetaPresent = FindMetadata(primarySettings->pMetadata,
                                        ANDROID_CONTROL_AF_REGIONS, &entryAFRegionMain);
    for (UINT i = 0; i < numOfTranslatedSettings; i++)
    {
        isAuxMetaPresent = FindMetadata(ptranslatedSettings[i].pMetadata,
                                         ANDROID_CONTROL_AF_REGIONS, &entryAFRegionAux[i]);
        if (0 != isAuxMetaPresent)
        {
            break;
        }
    }
    if ((0 == isPrimayMetaPresent) && (0 == isAuxMetaPresent))
    {
        // AF_REGION tag tuple has 5 elements [xMin, yMin, xMax, yMax, weight]
        UINT32 tupleSize = 5;
        for (UINT32 i = 0; i < entryAFRegionMain.count / tupleSize; i++)
        {
            WeightedRegion afRegion = { 0 };
            UINT32 index = i * tupleSize;

            afRegion.xMin   = entryAFRegionMain.data.i32[index];
            afRegion.yMin   = entryAFRegionMain.data.i32[index + 1];
            afRegion.xMax   = entryAFRegionMain.data.i32[index + 2];
            afRegion.yMax   = entryAFRegionMain.data.i32[index + 3];
            afRegion.weight = entryAFRegionMain.data.i32[index + 4];

            // Get the translated AF ROI for the primary camera
            WeightedRegion afRegionTrans = TranslateMeteringRegion(&afRegion, primarySettings->cameraId);

            // Update the Primay metadata
            result = SetMetadata(primarySettings->pMetadata, ANDROID_CONTROL_AF_REGIONS,
                                 &afRegionTrans, 5);
            if (CDKResultSuccess != result)
            {
                CHX_LOG_ERROR("SetMetadata failed for tag AFRegion for camera id %d",
                    primarySettings->cameraId);
            }

        }
    }

    // Translate the Metering ROI
    camera_metadata_entry_t entryAERegionMain = { 0 };
    camera_metadata_entry_t entryAERegionAux[MaxDevicePerLogicalCamera - 1] = { { 0 } };

    isPrimayMetaPresent = FindMetadata(primarySettings->pMetadata,
                                        ANDROID_CONTROL_AE_REGIONS, &entryAERegionMain);
    for (UINT i = 0; i < numOfTranslatedSettings; i++)
    {
        isAuxMetaPresent = FindMetadata(ptranslatedSettings[i].pMetadata,
                                         ANDROID_CONTROL_AE_REGIONS, &entryAERegionAux[i]);
        if (0 != isAuxMetaPresent)
        {
            break;
        }
    }
    if ((0 == isPrimayMetaPresent) && (0 == isAuxMetaPresent))
    {
        // AE_REGION tag tuple has 5 elements [xMin, yMin, xMax, yMax, weight]
        UINT32 tupleSize = 5;
        for (UINT32 i = 0; i < entryAERegionMain.count / tupleSize; i++)
        {
            WeightedRegion aeRegion = { 0 };
            UINT32 index = i * tupleSize;

            aeRegion.xMin   = entryAERegionMain.data.i32[index];
            aeRegion.yMin   = entryAERegionMain.data.i32[index + 1];
            aeRegion.xMax   = entryAERegionMain.data.i32[index + 2];
            aeRegion.yMax   = entryAERegionMain.data.i32[index + 3];
            aeRegion.weight = entryAERegionMain.data.i32[index + 4];

            // Get the translated AE ROI for the primary camera
            WeightedRegion aeRegionTrans = TranslateMeteringRegion(&aeRegion, primarySettings->cameraId);

            // Update the metadata
            result = SetMetadata(primarySettings->pMetadata,
                                 ANDROID_CONTROL_AF_REGIONS,
                                 &aeRegionTrans, 5);
            if (CDKResultSuccess != result)
            {
                CHX_LOG_ERROR("Set Metadata failed for tag AERegion for camera ID %d",
                    primarySettings->cameraId);
            }
        }
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::TranslateMeteringRegion
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WeightedRegion MultiRTBController::TranslateMeteringRegion(
    WeightedRegion* pRegion,
    UINT32          cameraId)
{
    FLOAT fovRatio = 1.0f;

    WeightedRegion    regionInput = *pRegion;
    WeightedRegion    regionTrans = regionInput;
    CHIRECT           activeArray = { 0 };
    UINT32            camIdx;
    UINT32            primaryCamIdx;

    if (0 != (regionInput.xMax * regionInput.yMax))
    {
        // Acquire mutex to read spatial alignment shifts which are written by other thread
        m_pLock->Lock();

        primaryCamIdx = GetCameraIndex(m_primaryCamId);
        CHX_ASSERT(INVALID_INDEX != primaryCamIdx);
        camIdx = GetCameraIndex(cameraId);
        CHX_ASSERT(INVALID_INDEX != camIdx);

        if (m_primaryCamId == cameraId)
        {
            if(m_camInfo[primaryCamIdx].adjustedFovRatio>0)
            {
                fovRatio    = 1.0f / m_camInfo[primaryCamIdx].adjustedFovRatio;
                activeArray = m_camInfo[primaryCamIdx].activeArraySize;
            }
            else
            {
                CHX_LOG_ERROR("Invalid adjustedFovRatio for primary camera id %d", m_camInfo[primaryCamIdx].cameraId);
            }
        }
        else
        {
            fovRatio    = m_camInfo[camIdx].adjustedFovRatio;
            activeArray = m_camInfo[camIdx].activeArraySize;
        }
        m_pLock->Unlock();

        // ROI should be with respect to the corresponding active array
        FLOAT xScale = ((FLOAT)activeArray.width / m_camInfo[camIdx].activeArraySize.width);
        FLOAT yScale = ((FLOAT)activeArray.height / m_camInfo[camIdx].activeArraySize.height);

        regionInput.xMin *= xScale;
        regionInput.yMin *= yScale;
        regionInput.xMax *= xScale;
        regionInput.yMax *= yScale;

        INT32 regionWidth       = regionInput.xMax - regionInput.xMin + 1;
        INT32 regionHeight      = regionInput.yMax - regionInput.yMin + 1;
        INT32 regionTransWidth  = regionWidth * fovRatio;
        INT32 regionTransHeight = regionHeight * fovRatio;

        INT32 regionHorzDelta = (regionTransWidth - regionWidth) / 2;
        INT32 regionVertDelta = (regionTransHeight - regionHeight) / 2;

        regionTrans.xMin = regionInput.xMin - regionHorzDelta;
        regionTrans.yMin = regionInput.yMin - regionVertDelta;

        regionTrans.xMax = regionTrans.xMin + regionTransWidth - 1;
        regionTrans.yMax = regionTrans.yMin + regionTransHeight - 1;

        INT32 activeArrayWidth  = activeArray.width;
        INT32 activeArrayHeight = activeArray.height;

        // Check ROI bounds and correct it if necessary
        if ((regionTrans.xMin < 0) ||
            (regionTrans.yMin < 0) ||
            (regionTrans.xMax > activeArrayWidth) ||
            (regionTrans.yMax > activeArrayHeight))
        {
            if (regionTrans.xMin < 0)
            {
                regionTrans.xMin = 0;
            }
            if (regionTrans.yMin < 0)
            {
                regionTrans.yMin = 0;
            }
            if (regionTrans.xMax >= activeArrayWidth)
            {
                regionTrans.xMax = activeArrayWidth - 1;
            }
            if (regionTrans.yMax >= activeArrayHeight)
            {
                regionTrans.yMax = activeArrayHeight - 1;
            }
        }
    }

    return regionTrans;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::SetMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiRTBController::SetMetadata(
    ChiMetadata *pMetaData,
    UINT32      tag,
    VOID*       pData,
    SIZE_T      count)
{
    return pMetaData->SetTag(tag, pData, count);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::ConsolidateCameraInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiRTBController::ConsolidateCameraInfo(
    LogicalCameraInfo *pLogicalCameraInfo)
{
    CHICAMERAINFO  *ppCamInfo[MaxDevicePerLogicalCamera]             = {0};
    CHICAMERAINFO  *pConsolidatedCamInfo;
    camera_metadata_t* pPrimaryMetadata                              = NULL;;
    camera_metadata_t* pAuxMetadata[MaxDevicePerLogicalCamera - 1]   = { 0 };;

    UINT      numBundledCameras = 0;;
    UINT32    primaryCamIdx     = 0;
    UINT32    primaryCameraId   = 0;
    UINT32    auxCameraCount    = 0;
    CDKResult result = CDKResultSuccess;

    CHX_ASSERT(NULL != pLogicalCameraInfo);
    CHX_ASSERT(InvalidPhysicalCameraId != pLogicalCameraInfo->primaryCameraId);

    numBundledCameras    = pLogicalCameraInfo->numPhysicalCameras;
    pConsolidatedCamInfo = &(pLogicalCameraInfo->m_cameraCaps);
    primaryCameraId      = pLogicalCameraInfo->primaryCameraId;

    CHX_ASSERT(MaxDevicePerLogicalCamera == numBundledCameras);

    /// Extract primary and Aux cameras metadata
    for (UINT32 i = 0; i < pLogicalCameraInfo->numPhysicalCameras; i++)
    {
        ppCamInfo[i] = const_cast<CHICAMERAINFO*>(pLogicalCameraInfo->ppDeviceInfo[i]->m_pDeviceCaps);
        CHX_ASSERT(NULL != ppCamInfo[i]);

        if (primaryCameraId == pLogicalCameraInfo->ppDeviceInfo[i]->cameraId)
        {
            camera_info_t* pCameraInfoMain = static_cast<camera_info_t*>(ppCamInfo[i]->pLegacy);
            pPrimaryMetadata = const_cast<camera_metadata_t *>(pCameraInfoMain->static_camera_characteristics);
            primaryCamIdx    = i;
        }
        else
        {
            camera_info_t* pCameraInfo   = static_cast<camera_info_t*>(ppCamInfo[i]->pLegacy);
            pAuxMetadata[auxCameraCount] = const_cast<camera_metadata_t *>(pCameraInfo->static_camera_characteristics);

            if (NULL == pAuxMetadata[auxCameraCount])
            {
                result = CDKResultEFailed;
            }
            auxCameraCount++;
        }
    }

    if (NULL == pPrimaryMetadata)
    {
        result = CDKResultEFailed;
    }

    CHX_LOG("Primary camera id %d, Auxmeta count[%d]", primaryCameraId, auxCameraCount);

    camera_info_t*           pCameraInfoConsolidated    = static_cast<camera_info_t*>(pConsolidatedCamInfo->pLegacy);
    const camera_metadata_t* pMetadataConsolidated      = pCameraInfoConsolidated->static_camera_characteristics;

    if (NULL != ppCamInfo[primaryCamIdx])
    {
        pConsolidatedCamInfo->lensCaps       = ppCamInfo[primaryCamIdx]->lensCaps;
        pConsolidatedCamInfo->numSensorModes = ppCamInfo[primaryCamIdx]->numSensorModes;
        pConsolidatedCamInfo->sensorCaps     = ppCamInfo[primaryCamIdx]->sensorCaps;

        /// Copy the primary cameras metadata to the consolidated meta
        ChxUtils::Memcpy(pConsolidatedCamInfo->pLegacy, ppCamInfo[primaryCamIdx]->pLegacy, sizeof(camera_info_t));
        pCameraInfoConsolidated->static_camera_characteristics = pMetadataConsolidated;

        pConsolidatedCamInfo->size = ppCamInfo[primaryCamIdx]->size;
    }
    else
    {
        CHX_LOG_ERROR("Primary camera info is NULL");
        result = CDKResultEFailed;
    }

    if (CDKResultSuccess == result)
    {
        result = ConsolidateStreamConfig(const_cast<camera_metadata_t*>(pMetadataConsolidated),
            pPrimaryMetadata, pAuxMetadata, auxCameraCount);
    }

    if (CDKResultSuccess == result)
    {
        result = ConsolidateMinFpsStallDuration(const_cast<camera_metadata_t*>(pMetadataConsolidated),
                                                pPrimaryMetadata, pAuxMetadata, auxCameraCount);
    }

    if (CDKResultSuccess == result)
    {
        camera_metadata_entry_t entryMain = { 0 };
        camera_metadata_entry_t entryConsolidated = { 0 };
        camera_metadata_entry_t entryAux[MaxDevicePerLogicalCamera - 1];


        INT isPrimaryMetaPresent = 1;
        INT isAuxMetaPresent     = 1;
        INT isConsMetaPresent    = 1;

        isPrimaryMetaPresent = find_camera_metadata_entry(pPrimaryMetadata, ANDROID_JPEG_MAX_SIZE, &entryMain);

        for (UINT32 i = 0; i < auxCameraCount; i++)
        {
            isAuxMetaPresent = find_camera_metadata_entry(pAuxMetadata[i], ANDROID_JPEG_MAX_SIZE, &entryAux[i]);
            if (0 != isAuxMetaPresent)
            {
                break;
            }
        }

        isConsMetaPresent = find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated),
            ANDROID_JPEG_MAX_SIZE, &entryConsolidated);

        if ((0 == isPrimaryMetaPresent) && (0 == isAuxMetaPresent) && (0 == isConsMetaPresent))
        {
            INT32 maxJPEGSizeOfMain = *entryMain.data.i32;
            INT32 maxJPEGSizeOfAux  =  0;
            INT32 maxJPEGSize       =  maxJPEGSizeOfMain;
            for (UINT32 i = 0; i < auxCameraCount; i++)
            {
                maxJPEGSizeOfAux = ChxUtils::MaxINT32(maxJPEGSizeOfAux, *entryAux[i].data.i32);
            }

            /// Choose the largest JPEG size for consolidate metadata
            maxJPEGSize = ChxUtils::MaxINT32(maxJPEGSize, maxJPEGSizeOfAux);

            if (0 != UpdateMetadata(const_cast<camera_metadata_t*>(pMetadataConsolidated),
                entryConsolidated.index,
                &maxJPEGSize,
                entryConsolidated.count,
                &entryConsolidated))
            {
                CHX_LOG_WARN("update MAXJPEGSize metadata for consolidate metadata failed!");
            }

        }

        /// Update ANDROID_LENS_FACING Tag
        isPrimaryMetaPresent = find_camera_metadata_entry(pPrimaryMetadata, ANDROID_LENS_FACING, &entryMain);

        for (UINT32 i = 0; i < auxCameraCount; i++)
        {
            isAuxMetaPresent = find_camera_metadata_entry(pAuxMetadata[i], ANDROID_LENS_FACING, &entryAux[i]);
            if (0 != isAuxMetaPresent)
            {
                break;
            }
        }
        isConsMetaPresent = find_camera_metadata_entry(const_cast<camera_metadata_t*>(pMetadataConsolidated),
            ANDROID_LENS_FACING, &entryConsolidated);

        if ((0 == isPrimaryMetaPresent) && (0 == isAuxMetaPresent) && (0 == isConsMetaPresent))
        {
            // Use front face as consolidate metadata face,
            // App needs the aux info ( front facing ) to select the logical camera
            INT32 iLensFace = entryAux[0].data.i32[0];
            if (0 != UpdateMetadata(const_cast<camera_metadata_t*>(pMetadataConsolidated), entryConsolidated.index,
                &iLensFace, entryConsolidated.count, &entryConsolidated))
            {
                CHX_LOG_WARN("update lence face metadata for consolidate metadata failed!");
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::ConsolidateMinFpsStallDuration
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiRTBController::ConsolidateMinFpsStallDuration(
    camera_metadata_t   *pConsolidatedMetadata,
    camera_metadata_t   *pPrimaryCamSettings,
    camera_metadata_t   *pAuxCamSettings[],
    UINT32              numAuxSettings)
{
    CDKResult result = CDKResultSuccess;

    camera_metadata_entry_t entryMain = { 0 };
    camera_metadata_entry_t entryConsolidated = { 0 };
    camera_metadata_entry_t entryAux[MaxDevicePerLogicalCamera - 1];

    INT isPrimaryMetaPresent = 1;
    INT isAuxMetaPresent     = 1;
    INT isConsMetaPresent    = 1;

    // Update available min frame durations for consolidate metadata
    // Since this is multi camera usecase, availabe min frame durations should be
    // compilation of main and aux cameras
    isPrimaryMetaPresent = find_camera_metadata_entry(pPrimaryCamSettings,
        ANDROID_SCALER_AVAILABLE_MIN_FRAME_DURATIONS, &entryMain);

    for (UINT32 i = 0; i < numAuxSettings; i++)
    {
        isAuxMetaPresent = find_camera_metadata_entry(pAuxCamSettings[i],
            ANDROID_SCALER_AVAILABLE_MIN_FRAME_DURATIONS, &entryAux[i]);

        if (0 != isAuxMetaPresent)
        {
            break;
        }
    }

    isConsMetaPresent = find_camera_metadata_entry(pConsolidatedMetadata,
        ANDROID_SCALER_AVAILABLE_MIN_FRAME_DURATIONS, &entryConsolidated);

    if ((0 == isPrimaryMetaPresent) && (0 == isAuxMetaPresent) && (0 == isConsMetaPresent))
    {
        UINT32 numEntriesConsolidated = 0;
        // 4 elements in the tuple of ANDROID_SCALER_AVAILABLE_MIN_FRAME_DURATIONS
        UINT32 tupleSize                                    = 4;
        UINT32 numEntriesMain                               = entryMain.count / tupleSize;
        UINT32 numEntriesAux[MaxDevicePerLogicalCamera - 1] = {0};
        UINT32 numEntriesTotalAux                           = 0;

        for (UINT32 i = 0; i < numAuxSettings; i++)
        {
            numEntriesAux[i]    = entryAux[i].count / tupleSize;
            numEntriesTotalAux += numEntriesAux[i];
        }

        INT64* pEntryData = static_cast<INT64*>(ChxUtils::Calloc(
            (numEntriesTotalAux + numEntriesMain) * tupleSize * sizeof(INT64)));

        if (NULL != pEntryData)
        {
            /// Copy data from Primary cam meta to consolidated meta
            ChxUtils::Memcpy(pEntryData, entryMain.data.i64, entryMain.count * sizeof(INT64));
            numEntriesConsolidated = numEntriesMain;

            for (UINT32 j = 0; j < numAuxSettings; j++)
            {
                for (UINT32 k = 0; k < numEntriesAux[j]; k++)
                {
                    UINT32 auxIndex   = k * tupleSize;
                    BOOL   matchFound = FALSE;

                    /// Consolidated Meta already has main meta. Check if entry is in consolidated meta before adding.
                    for (UINT32 i = 0; i < numEntriesConsolidated; i++)
                    {
                        UINT32 idx = i * tupleSize;
                        if ((pEntryData[idx]     == entryAux[j].data.i64[auxIndex]) &&
                            (pEntryData[idx + 1] == entryAux[j].data.i64[auxIndex + 1]) &&
                            (pEntryData[idx + 2] == entryAux[j].data.i64[auxIndex + 2]) &&
                            (pEntryData[idx + 3] == entryAux[j].data.i64[auxIndex + 3]))
                        {
                            matchFound = TRUE;
                            break;
                        }
                    }

                    // if this min frame duration is not in main metadata, add it to consolidate metadata.
                    if (FALSE == matchFound)
                    {
                        UINT32 consolidatedIndex = numEntriesConsolidated * tupleSize;

                        pEntryData[consolidatedIndex]     = entryAux[j].data.i64[auxIndex];
                        pEntryData[consolidatedIndex + 1] = entryAux[j].data.i64[auxIndex + 1];
                        pEntryData[consolidatedIndex + 2] = entryAux[j].data.i64[auxIndex + 2];
                        pEntryData[consolidatedIndex + 3] = entryAux[j].data.i64[auxIndex + 3];
                        numEntriesConsolidated++;
                    }
                }
            }

            entryConsolidated.count = numEntriesConsolidated * tupleSize;

            if (0 != UpdateMetadata(pConsolidatedMetadata, entryConsolidated.index,
                    pEntryData, entryConsolidated.count, &entryConsolidated))
            {
                CHX_LOG_ERROR("update availableMinFrameDurations for consolidate metadata failed!");
                result = CDKResultEFailed;
            }

            ChxUtils::Free(pEntryData);
            pEntryData = NULL;
        }
        else
        {
            CHX_LOG("No memory allocated for pEntryData");
            result = CDKResultENoMemory;
        }
    }

    if (result == CDKResultSuccess)
    {
        camera_metadata_entry_t entryMain = { 0 };
        camera_metadata_entry_t entryConsolidated = { 0 };
        camera_metadata_entry_t entryAux[MaxDevicePerLogicalCamera - 1];

        INT isPrimaryMetaPresent = 1;
        INT isAuxMetaPresent     = 1;
        INT isConsMetaPresent    = 1;

        // Update available stall durations for consolidate metadata
        // Since this is multi camera usecase, availabe stall durations should be
        // compilation of main and aux cameras
        isPrimaryMetaPresent = find_camera_metadata_entry(pPrimaryCamSettings,
            ANDROID_SCALER_AVAILABLE_STALL_DURATIONS, &entryMain);

        for (UINT32 i = 0; i < numAuxSettings; i++)
        {
            isAuxMetaPresent = find_camera_metadata_entry(pAuxCamSettings[i],
                ANDROID_SCALER_AVAILABLE_STALL_DURATIONS, &entryAux[i]);

            if (0 != isAuxMetaPresent)
            {
                break;
            }
        }

        isConsMetaPresent = find_camera_metadata_entry(pConsolidatedMetadata,
            ANDROID_SCALER_AVAILABLE_STALL_DURATIONS, &entryConsolidated);

        if ((0 == isPrimaryMetaPresent) && (0 == isAuxMetaPresent) && (0 == isConsMetaPresent))
        {
            UINT32 numEntriesConsolidated = 0;
            // 4 elements in the tuple of ANDROID_SCALER_AVAILABLE_STALL_DURATIONS
            UINT32 tupleSize                                    = 4;
            UINT32 numEntriesMain                               = entryMain.count / tupleSize;
            UINT32 numEntriesAux[MaxDevicePerLogicalCamera - 1] = {0};
            UINT32 numEntriesTotalAux                           = 0;

            for (UINT32 i = 0; i < numAuxSettings; i++)
            {
                numEntriesAux[i]    = entryAux[i].count / tupleSize;
                numEntriesTotalAux += numEntriesAux[i];
            }

            INT64* pEntryData = static_cast<INT64*>(ChxUtils::Calloc(
                (numEntriesTotalAux + numEntriesMain) * tupleSize * sizeof(INT64)));

            if (NULL != pEntryData)
            {
                /// Copy data from Primary cam meta to consolidated meta
                ChxUtils::Memcpy(pEntryData, entryMain.data.i64, entryMain.count * sizeof(INT64));
                numEntriesConsolidated = numEntriesMain;

                for (UINT32 j = 0; j < numAuxSettings; j++)
                {
                    for (UINT32 k = 0; k < numEntriesAux[j]; k++)
                    {
                        UINT32 auxIndex   = k * tupleSize;
                        BOOL   matchFound = FALSE;

                        /// Consolidated Meta already has main meta. Check if entry is in consolidated meta before adding.
                        for (UINT32 i = 0; i < numEntriesConsolidated; i++)
                        {
                            UINT32 idx = i * tupleSize;
                            if ((pEntryData[idx]     == entryAux[j].data.i64[auxIndex]) &&
                                (pEntryData[idx + 1] == entryAux[j].data.i64[auxIndex + 1]) &&
                                (pEntryData[idx + 2] == entryAux[j].data.i64[auxIndex + 2]) &&
                                (pEntryData[idx + 3] == entryAux[j].data.i64[auxIndex + 3]))
                            {
                                matchFound = TRUE;
                                break;
                            }
                        }

                        // if this stall durations is not in main metadata, add it to consolidate metadata.
                        if (FALSE == matchFound)
                        {
                            UINT32 consolidatedIndex = numEntriesConsolidated * tupleSize;

                            pEntryData[consolidatedIndex]     = entryAux[j].data.i64[auxIndex];
                            pEntryData[consolidatedIndex + 1] = entryAux[j].data.i64[auxIndex + 1];
                            pEntryData[consolidatedIndex + 2] = entryAux[j].data.i64[auxIndex + 2];
                            pEntryData[consolidatedIndex + 3] = entryAux[j].data.i64[auxIndex + 3];
                            numEntriesConsolidated++;
                        }
                    }
                }

                entryConsolidated.count = numEntriesConsolidated * tupleSize;

                if (0 != UpdateMetadata(pConsolidatedMetadata, entryConsolidated.index,
                        pEntryData, entryConsolidated.count, &entryConsolidated))
                {
                    CHX_LOG_ERROR("update availableStallDurations for consolidate metadata failed!");
                    result = CDKResultEFailed;
                }

                ChxUtils::Free(pEntryData);
                pEntryData = NULL;
            }
            else
            {
                CHX_LOG("No memory allocated for pEntryData");
                result = CDKResultENoMemory;
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::ConsolidateStreamConfig
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiRTBController::ConsolidateStreamConfig(
    camera_metadata_t   *pConsolidatedMetadata,
    camera_metadata_t   *pPrimaryCamSettings,
    camera_metadata_t   *pAuxCamSettings[],
    UINT32              numAuxSettings)
{
    CDKResult result = CDKResultSuccess;

    camera_metadata_entry_t entryMain = { 0 };
    camera_metadata_entry_t entryConsolidated = { 0 };
    camera_metadata_entry_t entryAux[MaxDevicePerLogicalCamera - 1];

    INT isPrimaryMetaPresent = 1;
    INT isAuxMetaPresent     = 1;
    INT isConsMetaPresent    = 1;

    // Update available stream configure for consolidate metadata
    // Since this is multi camera usecase, availabe stream configure should be
    // compilation of main and aux cameras
    isPrimaryMetaPresent = find_camera_metadata_entry(pPrimaryCamSettings,
        ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entryMain);

    for (UINT32 i = 0; i < numAuxSettings; i++)
    {
        isAuxMetaPresent = find_camera_metadata_entry(pAuxCamSettings[i],
            ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entryAux[i]);

        if (0 != isAuxMetaPresent)
        {
            break;
        }
    }

    isConsMetaPresent = find_camera_metadata_entry(pConsolidatedMetadata,
        ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entryConsolidated);

    if ((0 == isPrimaryMetaPresent) && (0 == isAuxMetaPresent) && (0 == isConsMetaPresent))
    {
        UINT32 numEntriesConsolidated = 0;
        // 4 elements in the tuple of ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS
        UINT32 tupleSize                                    = 4;
        UINT32 numEntriesMain                               = entryMain.count / tupleSize;
        UINT32 numEntriesAux[MaxDevicePerLogicalCamera - 1] = {0};
        UINT32 numEntriesTotalAux                           = 0;

        for (UINT32 i = 0; i < numAuxSettings; i++)
        {
            numEntriesAux[i]    = entryAux[i].count / tupleSize;
            numEntriesTotalAux += numEntriesAux[i];
        }

        INT32* pEntryData = static_cast<INT32*>(ChxUtils::Calloc(
            (numEntriesTotalAux + numEntriesMain) * tupleSize * sizeof(INT32)));

        if (NULL != pEntryData)
        {
            /// Copy data from Primary cam meta to consolidated meta
            ChxUtils::Memcpy(pEntryData, entryMain.data.i32, entryMain.count * sizeof(INT32));
                numEntriesConsolidated = numEntriesMain;

            for (UINT32 j = 0; j < numAuxSettings; j++)
            {
                for (UINT32 k = 0; k < numEntriesAux[j]; k++)
                {
                    UINT32 auxIndex   = k * tupleSize;
                    BOOL   matchFound = FALSE;

                    /// Consolidated Meta already has main meta. Check if entry is in consolidated meta before adding.
                    for (UINT32 i = 0; i < numEntriesConsolidated; i++)
                    {
                        UINT32 idx = i * tupleSize;
                        if ((pEntryData[idx]     == entryAux[j].data.i32[auxIndex]) &&
                            (pEntryData[idx + 1] == entryAux[j].data.i32[auxIndex + 1]) &&
                            (pEntryData[idx + 2] == entryAux[j].data.i32[auxIndex + 2]) &&
                            (pEntryData[idx + 3] == entryAux[j].data.i32[auxIndex + 3]))
                        {
                            matchFound = TRUE;
                            break;
                        }
                    }

                    // if this stream configure is not in main metadata, add it to consolidate metadata.
                    if (FALSE == matchFound)
                    {
                        UINT32 consolidatedIndex = numEntriesConsolidated * tupleSize;

                        pEntryData[consolidatedIndex]     = entryAux[j].data.i32[auxIndex];
                        pEntryData[consolidatedIndex + 1] = entryAux[j].data.i32[auxIndex + 1];
                        pEntryData[consolidatedIndex + 2] = entryAux[j].data.i32[auxIndex + 2];
                        pEntryData[consolidatedIndex + 3] = entryAux[j].data.i32[auxIndex + 3];
                        numEntriesConsolidated++;
                    }
                }
            }

            entryConsolidated.count = numEntriesConsolidated * tupleSize;

            if (0 != UpdateMetadata(pConsolidatedMetadata, entryConsolidated.index,
                    pEntryData, entryConsolidated.count, &entryConsolidated))
            {
                CHX_LOG_ERROR("update availablestreamconfigure for consolidate metadata failed!");
                result = CDKResultEFailed;
            }

            ChxUtils::Free(pEntryData);
            pEntryData = NULL;
        }
        else
        {
            CHX_LOG("No memory allocated for pEntryData");
            result = CDKResultENoMemory;
        }
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::ProcessResultMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiRTBController::ProcessResultMetadata(
    ChiMetadata* pResultMetadata)
{
    OutputMetadataBokeh* pMetadataBokeh = static_cast<OutputMetadataBokeh*>(pResultMetadata->GetTag(
        MultiCamControllerManager::m_vendorTagBokehResultMeta));

    if (NULL != pMetadataBokeh)
    {
        CHX_LOG_INFO("Current master: %d Recommended master: %d", pMetadataBokeh->masterCamera,
            pMetadataBokeh->recommendedMasterCamera);

        m_pLock->Lock();
        // Update the master camera info
        m_result.masterCameraId       = pMetadataBokeh->masterCameraId;
        CHX_ASSERT(InvalidPhysicalCameraId != m_result.masterCameraId);

        m_result.masterCameraRole     = pMetadataBokeh->masterCamera;
        m_result.numOfActiveCameras   = m_numOfLinkedSessions;

        m_recommendedMasterCameraRole = pMetadataBokeh->recommendedMasterCamera;
        m_recommendedMasterCameraId   = pMetadataBokeh->recommendedMasterCameraId;
        m_result.activeMap            = pMetadataBokeh->activeMap;
        m_pLock->Unlock();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::UpdateResults
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiRTBController::UpdateResults(
    ChiMetadata* pMetadata)
{
    if (NULL != pMetadata)
    {
        camera_metadata_entry_t entryCropRegion = { 0 };
        if (0 == pMetadata->FindTag(ANDROID_SCALER_CROP_REGION, &entryCropRegion))
        {
            CHIRECT userZoom;
            userZoom.left   = entryCropRegion.data.i32[0];
            userZoom.top    = entryCropRegion.data.i32[1];
            userZoom.width  = entryCropRegion.data.i32[2];
            userZoom.height = entryCropRegion.data.i32[3];

            m_zoomUser = m_camInfo[GetCameraIndex(m_primaryCamId)].activeArraySize.width /
                (FLOAT)userZoom.width;

            CHX_LOG_INFO("User Zoom %f, User Zoom Crop rect l %d, T %d, w %d, h %d",
                m_zoomUser, userZoom.left, userZoom.top, userZoom.width, userZoom.height);
        }
        else
        {
            CHX_LOG_ERROR("Cannot find metadata ANDROID_SCALER_CROP_REGION in meta pointer %p", pMetadata);
        }

        SetInitialResultState();
    }
    CHX_LOG_INFO("Updated Master camera id: %d fusion: %d", m_result.masterCameraId, m_result.snapshotFusion);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::UpdateScalerCropForSnapshot
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiRTBController::UpdateScalerCropForSnapshot(
    ChiMetadata* pMetadata)
{
    (VOID)pMetadata;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::UpdateVendorTags
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID MultiRTBController::UpdateVendorTags(
    MulticamReqSettings* pMultiCamSettings)
{
    CDKResult result = CDKResultSuccess;

    CameraSettings primarySettings;
    CameraSettings translatedSettings[MaxDevicePerLogicalCamera - 1];
    UINT32         primaryCamIdx = 0;
    ChxUtils::Memset(&primarySettings, 0, sizeof(primarySettings));
    ChxUtils::Memset(translatedSettings, 0, sizeof(translatedSettings));

    if (NULL != pMultiCamSettings)
    {
        CHX_ASSERT(m_numOfLinkedSessions == pMultiCamSettings->numSettingsBuffers);

        UINT32 translatedMetacount = 0;
        for (UINT32 i = 0; i < pMultiCamSettings->numSettingsBuffers; i++)
        {
            ChiMetadata *pMetadata = pMultiCamSettings->ppSettings[i];

            if(NULL != pMetadata)
            {
                //Get CameraID from metadata
                UINT32      *pCameraId = static_cast <UINT32*>(pMetadata->GetTag(
                    MultiCamControllerManager::m_vendorTagMetadataOwner));

                if (NULL != pCameraId)
                {
                    if (m_primaryCamId == *pCameraId)
                    {
                        primarySettings.cameraId  = *pCameraId;
                        primarySettings.pMetadata =  pMetadata;
                        primaryCamIdx             =  GetCameraIndex(primarySettings.cameraId);
                        if (NULL == primarySettings.pMetadata)
                        {
                            CHX_LOG_ERROR("Primary metadata is null");
                            result = CDKResultEFailed;
                        }
                        CHX_ASSERT(INVALID_INDEX != primaryCamIdx);
                    }
                    else
                    {
                        translatedSettings[translatedMetacount].cameraId  = *pCameraId;
                        translatedSettings[translatedMetacount].pMetadata =  pMetadata;
                        if (NULL == translatedSettings[translatedMetacount].pMetadata)
                        {
                            CHX_LOG_ERROR("translatedSettings[%d] metadata is null", translatedMetacount);
                            result = CDKResultEFailed;
                        }
                        translatedMetacount++;
                    }
                }
                else
                {
                    CHX_LOG_ERROR("Metadata tag MetadataOwner is NULL. Cannot get cameraId");
                    result = CDKResultEFailed;
                }
            }
            else
            {
                CHX_LOG_ERROR("Metadata from setting buffers is NULL");
                result = CDKResultEFailed;
            }
        }

        if(NULL == primarySettings.pMetadata)
        {
            CHX_LOG_ERROR("Primary setting metadata is NULL");
            result = CDKResultEFailed;
        }

        // Get the SW master camera of this request
        CameraRoleType masterCameraRole = pMultiCamSettings->currentRequestMCCResult.masterCameraRole;
        CHX_LOG_INFO("Master camera role from MCC result %d", masterCameraRole);

        // Set the camera role and camera id for the first frame.
        if ((CDKResultSuccess == result) && (0 == pMultiCamSettings->frameNum))
        {
            // Set the MultiCameraIdRole for the main camera
            MultiCameraIdRole inputMetadata;
            inputMetadata.currentCameraId   = m_camInfo[primaryCamIdx].cameraId;
            inputMetadata.currentCameraRole = m_camInfo[primaryCamIdx].cameraRole;
            inputMetadata.logicalCameraId   = m_camIdLogical;
            inputMetadata.masterCameraRole  = masterCameraRole;
            result = SetMetadata(primarySettings.pMetadata,
                                 MultiCamControllerManager::m_vendorTagMultiCameraRole,
                                 &inputMetadata, sizeof(MultiCameraIdRole));
            CHX_LOG_INFO("primary CameraId %d Camera role %d", inputMetadata.currentCameraId,
                inputMetadata.currentCameraRole);
            if (CDKResultSuccess != result)
            {
                CHX_LOG_ERROR("Failed to set metadata tag  MultiCameraRole for camera id %d ",
                    inputMetadata.currentCameraId);
            }

            for (UINT32 i = 0; i < translatedMetacount; i++)
            {
                // Set the MultiCameraIdRole for the aux cameras
                inputMetadata.currentCameraId   = translatedSettings[i].cameraId;
                inputMetadata.currentCameraRole = m_camInfo[GetCameraIndex(translatedSettings[i].cameraId)].cameraRole;
                inputMetadata.logicalCameraId   = m_camIdLogical;
                inputMetadata.masterCameraRole  = masterCameraRole;

                result = SetMetadata(translatedSettings[i].pMetadata,
                                     MultiCamControllerManager::m_vendorTagMultiCameraRole,
                                     &inputMetadata, sizeof(MultiCameraIdRole));
                CHX_LOG_INFO("Aux CameraId %d Camera role %d", inputMetadata.currentCameraId,
                    inputMetadata.currentCameraRole);
                if (CDKResultSuccess != result)
                {
                    CHX_LOG_ERROR("Failed to set metadata tag  MultiCameraRole for camera id %d ",
                        inputMetadata.currentCameraId);
                }
            }
        }

        if (CDKResultSuccess == result)
        {
            // Add master camera info
            BOOL isMaster = FALSE;
            isMaster      = (m_camInfo[primaryCamIdx].cameraRole == masterCameraRole) ? TRUE : FALSE;
            result        = SetMetadata(primarySettings.pMetadata,
                                        MultiCamControllerManager::m_vendorTagMasterInfo,
                                        &isMaster, 1);
            CHX_LOG_INFO("MasterCameraRole %d, Camera id %d, isMaster %d", masterCameraRole,
                m_camInfo[primaryCamIdx].cameraId, isMaster);
            if (CDKResultSuccess != result)
            {
                CHX_LOG_ERROR("Failed to set metadata tag for MasterInfo for camera id %d ",
                    m_camInfo[primaryCamIdx].cameraId);
            }
            for (UINT32 i = 0; i < translatedMetacount; i++)
            {
                UINT32 camIdx = GetCameraIndex(translatedSettings[i].cameraId);
                CHX_ASSERT(INVALID_INDEX != camIdx);
                isMaster = (m_camInfo[camIdx].cameraRole == masterCameraRole) ? TRUE : FALSE;
                result = SetMetadata(translatedSettings[i].pMetadata,
                                    MultiCamControllerManager::m_vendorTagMasterInfo,
                                    &isMaster, 1);
                CHX_LOG_INFO("MasterCameraRole %d, Camera id %d, isMaster %d",
                    masterCameraRole, m_camInfo[camIdx].cameraId, isMaster);
                if (CDKResultSuccess != result)
                {
                    CHX_LOG_ERROR("Failed to set metadata tag for MasterInfo for camera id %d ",
                        m_camInfo[camIdx].cameraId);
                }
            }
        }

        // Add Sync info
        BOOL isSyncActive     = FALSE;
        BOOL isDualZoneActive = TRUE;

        // Enable frame sync if override settings is enabled and if its in in DUAL ZONE
        if ((TRUE == pMultiCamSettings->kernelFrameSyncEnable) && (TRUE == isDualZoneActive))
        {
            isSyncActive = TRUE;
        }

        CHX_LOG_INFO("isSyncActive %d", isSyncActive);

        for (UINT32 i = 0; i < pMultiCamSettings->numSettingsBuffers; i++)
        {
            ChiMetadata* pMetadata = pMultiCamSettings->ppSettings[i];
            CHX_ASSERT(NULL != pMetadata);
            result = SetMetadata(pMetadata, MultiCamControllerManager::m_vendorTagSyncInfo,
                &isSyncActive, sizeof(SyncModeInfo));
        }
    }
    return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MultiRTBController::FillOfflinePipelineInputMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult MultiRTBController::FillOfflinePipelineInputMetadata(
    MulticamResultMetadata* pMultiCamResultMetadata,
    ChiMetadata*                pOfflineMetadata,
    BOOL                        isSnapshotPipeline)
{
    (VOID)isSnapshotPipeline;
    // Populate the vendor tag for struct InputMetadataBokeh
    CDKResult result = CDKResultSuccess;

    CameraSettings cameraSettings[MaxDevicePerLogicalCamera];

    // Identify camera ID and metadata
    MultiCameraIdRole* pMultiCameraRole;
    UINT32*            pCameraId;
    INT32*             pIsMaster;
    CameraRoleType     masterRole          = CameraRoleTypeMax;
    UINT32             masterCamId;
    UINT32             masterMetaIdx;

    for (UINT32 i = 0; i < pMultiCamResultMetadata->numResults; i++)
    {
        cameraSettings[i].pMetadata = pMultiCamResultMetadata->ppResultMetadata[i];

        //Get camera ID from metadata
        pCameraId = static_cast <UINT32*>(cameraSettings[i].pMetadata->GetTag(
            MultiCamControllerManager::m_vendorTagMetadataOwner));

        /// Get Current camera Role
        /// Todo: Not needed when metadata is updated with cameraid
        pMultiCameraRole = static_cast <MultiCameraIdRole*>(cameraSettings[i].pMetadata->GetTag(
            MultiCamControllerManager::m_vendorTagMultiCameraRole));

        /// Check if current camera is SW master
        pIsMaster = static_cast<INT32*>(cameraSettings[i].pMetadata->GetTag(
            MultiCamControllerManager::m_vendorTagMasterInfo));

        /// Extract Master camera info from metadata
        if ((NULL != pMultiCameraRole) && (NULL != pIsMaster) && (NULL != pCameraId))
        {
            cameraSettings[i].cameraId = *pCameraId;
            CHX_LOG_INFO("Current Camera Id %d, cuurent camera role %d, is SW master %d", pMultiCameraRole->currentCameraId,
                pMultiCameraRole->currentCameraRole, *pIsMaster);
            if (TRUE == *pIsMaster)
            {
                masterRole    = pMultiCameraRole->currentCameraRole;
                masterCamId   = cameraSettings[i].cameraId;
                masterMetaIdx = i;
                CHX_LOG_INFO("Master Camera Role %d, Id %d, Meta Index %d",
                    masterRole, masterCamId, masterMetaIdx);
            }
        }
        else
        {
            CHX_LOG_ERROR("Metadata tag MultiCameraIdRole is NULL");
            result = CDKResultEFailed;
        }
    }

    if (CameraRoleTypeMax == masterRole)
    {
        CHX_LOG_ERROR("Get master camera role failed");
        result = CDKResultEFailed;
    }

    InputMetadataBokeh metadataBokeh;
    ChxUtils::Memset(&metadataBokeh, 0, sizeof(InputMetadataBokeh));

    if (CDKResultSuccess == result)
    {
        for (UINT32 i = 0; i < pMultiCamResultMetadata->numResults; i++)
        {
            if (NULL != cameraSettings[i].pMetadata)
            {
                ExtractCameraMetadata(cameraSettings[i].pMetadata, &metadataBokeh.cameraMetadata[i]);

                UINT32 camIdx = GetCameraIndex(cameraSettings[i].cameraId);
                CHX_ASSERT(INVALID_INDEX != camIdx);

                metadataBokeh.cameraMetadata[i].isValid          = TRUE;
                metadataBokeh.cameraMetadata[i].cameraRole       = m_camInfo[camIdx].cameraRole;
                metadataBokeh.cameraMetadata[i].cameraId         = m_camInfo[camIdx].cameraId;
                metadataBokeh.cameraMetadata[i].masterCameraRole = masterRole;
                metadataBokeh.cameraMetadata[i].masterCameraId   = masterCamId;
                metadataBokeh.cameraMetadata[i].fovRectIFE       = m_camInfo[camIdx].fovRectIFE;
                metadataBokeh.cameraMetadata[i].fovRectIPE       = m_camInfo[camIdx].fovRectIFE;
                metadataBokeh.cameraMetadata[i].activeArraySize  = m_camInfo[camIdx].activeArraySize;

                CaptureRequestCropRegions* pCropRegions;
                pCropRegions = static_cast <CaptureRequestCropRegions *>(cameraSettings[i].pMetadata->GetTag(
                    MultiCamControllerManager::m_vendorTagCropRegions));

                if (NULL != pCropRegions)
                {
                    metadataBokeh.cameraMetadata[i].userCropRegion     = pCropRegions->userCropRegion;
                    metadataBokeh.cameraMetadata[i].pipelineCropRegion = pCropRegions->pipelineCropRegion;
                    metadataBokeh.cameraMetadata[i].ifeLimitCropRegion = pCropRegions->ifeLimitCropRegion;
                    CHX_LOG("Camera id %d Pipeline cropregion:%d,%d,%d,%d", cameraSettings[i].cameraId,
                        pCropRegions->pipelineCropRegion.left, pCropRegions->pipelineCropRegion.top,
                        pCropRegions->pipelineCropRegion.width, pCropRegions->pipelineCropRegion.height);
                }
                else
                {
                    CHX_LOG_ERROR("Metadata is NULL for tag CropRegions");
                }
            }
            else
            {
                CHX_LOG_ERROR("Camera metadata for cameraid %d is NULL", cameraSettings[i].cameraId);
            }
        }

        metadataBokeh.frameId      = pMultiCamResultMetadata->frameNum;
        metadataBokeh.blurLevel    = 0;
        metadataBokeh.blurMinValue = 0;
        metadataBokeh.blurMaxValue = 0;

        pOfflineMetadata->Invalidate();

        if (CDKResultSuccess != pOfflineMetadata->Merge(*cameraSettings[masterMetaIdx].pMetadata))
        {
            CHX_LOG("Failure in append meta, dst meta size %ul src meta size %ul",
                pOfflineMetadata->Count(),
                cameraSettings[masterMetaIdx].pMetadata->Count());
        }

        if (CDKResultSuccess != SetMetadata(pOfflineMetadata,
            MultiCamControllerManager::m_vendorTagBokehInputMeta,
            &metadataBokeh, sizeof(InputMetadataBokeh)))
        {
            CHX_LOG("Failure in pSetMetaData of m_vendorTagBokehInputMeta");
        }
    }
    else
    {
        CHAR metaFileName[MaxFileLen];
        for (UINT32 i = 0; i < pMultiCamResultMetadata->numResults; i++)
        {
            if (NULL != pMultiCamResultMetadata->ppResultMetadata[i])
            {
                CdkUtils::SNPrintF(metaFileName, sizeof(metaFileName), "mcam_%d_%d.txt",
                    cameraSettings[i].cameraId,
                    pMultiCamResultMetadata->ppResultMetadata[i]->GetFrameNumber());
                pMultiCamResultMetadata->ppResultMetadata[i]->DumpDetailsToFile(metaFileName);
            }
        }
        result = CamxResultEFailed;
    }

    return result;
}

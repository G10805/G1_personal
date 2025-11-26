////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxextensionmodule.h
/// @brief CHX Extension Module
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CHXEXTENSIONMODULE_H
#define CHXEXTENSIONMODULE_H

#include <assert.h>

#include "camxdefs.h"
#include "chi.h"
#include "chioverride.h"
#include "chxdefs.h"
#include "camxcdktypes.h"
#include "chxutils.h"
#include <stdio.h>
#include "chxperf.h"

#if defined(_LINUX) && !defined(CAMERA_UNITTEST)
#include <log/log.h>
#undef LOG_TAG
#define LOG_TAG "CHIUSECASE"
#endif

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Forward Declarations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Mutex;
class Session;
class UsecaseFactory;
class UsecaseSelector;
class Usecase;
class PerfLockManager;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Constant Declarations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const UINT   MaxSessions                  = 32;           ///< Max sessions
static const UINT   MaxSensorModes               = 50;           ///< Max number of sensor modes
static const UINT   MaxHFRConfigs                = 64;           ///< Max number of HFR configurations
static const UINT   MaxCameras                   = 8;            ///< Max number of cameras supported
static const UINT   MaxDevicePerLogicalCamera    = 8;            ///< Maximum number of related cameras
static const UINT32 INVALID_INDEX                = 0xFFFFFFFF;   ///< Invalid Index
static const UINT32 InvalidPhysicalCameraId      = 0XFFFFFFFF;   ///< Invalid Physical Camera ID
static const UINT32 InvalidSessionId             = 0xFFFFFFFF; ///< Invalid session id

/// @brief Pipeline create data
struct PipelineCreateData
{
    const CHAR*                   pPipelineName;                     ///< Pipeline name
    CHIPIPELINECREATEDESCRIPTOR*  pPipelineCreateDescriptor;         ///< Pipeline create descriptor
    UINT32                        numOutputs;                        ///< Number of output buffers of this pipeline
    CHIPORTBUFFERDESCRIPTOR*      pOutputDescriptors;                ///< Output buffer descriptors
    UINT32                        numInputs;                         ///< Number of inputs
    CHIPIPELINEINPUTOPTIONS*      pInputOptions;                     ///< Input buffer requirements for this pipeline
};

/// @brief Create session data
struct SessionCreateData
{
    UINT32                          numPipelines;                   ///< Number of pipelines in this session
    CHIPIPELINEINFO*                pPipelineInfo;                  ///< Pipeline info
    CHICALLBACKS*                   pCallbacks;                     ///< Callbacks
    VOID*                           pPrivateCallbackData;           ///< Private callbacks
    CHISESSIONFLAGS                 flags;                          ///< Flags
};

/// @brief Submit request data
struct SubmitRequestData
{
    UINT64                hSession;         ///< Session handle
    UINT64                hPipeline;        ///< Pipeline handle
    UINT32                numPipelines;     ///< Number of pipelines
    CHIPIPELINEREQUEST*   pChiRequest;      ///< Capture request descriptor
};

/// @brief PhysicalCameraConfiguration
struct PhysicalCameraConfiguration
{
    UINT32         sensorId;                ///< Sensor Id, this is slot id of sensor.
    CameraRoleType cameraRole;              ///< Camera Role
    FLOAT          transitionZoomRatioMin;  ///< Min Zoom ratio at which the camera will be active
    FLOAT          transitionZoomRatioMax;  ///< Max Zoom ratio at which the camera will be active
    BOOL           enableSmoothTransition;  ///< Enable Smooth Transition for the camera during SAT
    BOOL           alwaysOn;                ///< Indicate if this camera always on
};

/// @bried Camera info
struct DeviceInfo
{
    UINT32                        cameraId;         ///< Physical device cameraID
    const CHICAMERAINFO*          m_pDeviceCaps;    ///< device capability.
    const CHISENSORMODEINFO*      pSensorModeInfo;  ///< sensor mode table array.
    PhysicalCameraConfiguration*  pDeviceConfig;    ///< Physical device configuration
};

/// @brief Logical Camera type
enum LogicalCameraType
{
    LogicalCameraType_Default  =0,     ///< Single physical camera
    LogicalCameraType_RTB,             ///< RTB mode
    LogicalCameraType_SAT,             ///< SAT mode
    LogicalCameraType_BAYERMONO,       ///< Dual camera with Bayer and MONO sensor
    LogicalCameraType_VR,              ///< VR camera
    LogicalCameraType_DualApp,         ///< Logical camera type for application dual camera solution
    LogicalCameraType_MAX,
};

/// @brief Logical Camera info
struct LogicalCameraInfo
{
    UINT32                   cameraId;                        ///< Logical cameraID
    camera_info              m_cameraInfo;                    ///< Application data
    CHICAMERAINFO            m_cameraCaps;                    ///< camera sub device capability
    CHISENSORMODEINFO*       pSensorModeInfo;                 ///< sensor mode table array.
    UINT32                   numPhysicalCameras;              ///< Number of physical device attached to this logical
                                                              /// camera
    DeviceInfo**             ppDeviceInfo;                    ///< Array of physical device info related to this logical camera
    const camera3_device_t*  m_pCamera3Device;                ///< Camera3 device pointer from application.
    LogicalCameraType        logicalCameraType;               ///< Type of this logical camera
    UINT32                   primaryCameraId;                 ///< Primary Camera from the App's perspective from the list of
                                                              ///  conected physical cameras in the logical camera
    BOOL                     publicVisiblity;                 ///< Is this visible
};

/// @brief Vendor Tags
struct VendorTagInfo
{
    const CHAR*  pSectionName;                ///< Vendor Tag Section Name
    const CHAR*  pTagName;                    ///< Vendor Tag Name
    UINT32       tagId;                       ///< Vendor Tag Id used to query
};

/// @brief HFR configuration
struct HFRConfigurationParams
{
    INT32 width;                ///< Width
    INT32 height;               ///< Height
    INT32 minFPS;               ///< minimum preview FPS
    INT32 maxFPS;               ///< maximum video FPS
    INT32 batchSizeMax;         ///< maximum batch size
};

/// @brief Logical camera configuration
struct LogicalCameraConfiguration
{
    UINT16                      logicalCameraId;                       ///< Logical camera Id exposed to APP
    UINT32                      logicalCameraType;                     ///< Logical camera type
    BOOL                        publicVisibility;                      ///< Indicate if this camera id is exposed to APP
    UINT16                      numOfDevices;                          ///< Number of physical device
    PhysicalCameraConfiguration deviceInfo[MaxDevicePerLogicalCamera]; ///< Physical device information array
    UINT32                      primarySensorID;                       ///< Primary camera from App's perspective.
                                                                       /// The primary camera's zoom values maps to the user zoom
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief The ExtensionModule class provides entry/landing implementation of CHX
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ExtensionModule
{
public:

    /// Return the singleton instance of ExtensionModule
    static ExtensionModule* GetInstance();

    /// Get info from CHI about a given logical camera
    CHX_INLINE const LogicalCameraInfo* GetCameraInfo(
        uint32_t logicalCameraId) const
    {
        return &m_logicalCameraInfo[logicalCameraId];
    }

    /// Generic, extensible interface for retrieving info from CHI
    CDKResult GetInfo(
       CDKGetInfoCmd       infoCmd,
       void*               pInputParams,
       void*               pOutputParams);

    /// Get the index of logical camera information by logical camera id
    UINT32 GetCameraIdIndex(
        UINT32 logicalCameraId) const;

    /// Get the primary camera index in device info array
    UINT32 GetPrimaryCameraIndex(
        const LogicalCameraInfo *pLogicalCameraInfo);

    /// Get physical camera information
    const LogicalCameraInfo* GetPhysicalCameraInfo(
        UINT32 physicalCameraId
        )const;

    /// Build logical camera information
    CDKResult BuildLogicalCameraInfo(
        UINT32* sensorIdMap,
        BOOL    isMultiCameraEnabled,
        BOOL    isForExposed);

    UINT32 GetVendorTagId(
        VendorTag tag
        ) const
    {
        return m_pvtVendorTags[tag].tagId;
    }

    /// Get sensor mode info
    CDKResult GetPhysicalCameraSensorModes(
        UINT32              physicalCameraId,       ///< Physical camera id
        UINT32*             pNumSensorModes,        ///< Number of sensor modes
        CHISENSORMODEINFO** ppAllSensorModes);      ///< Sensor mode list

    /// Create a pipeline
    CHIPIPELINEDESCRIPTOR CreatePipelineDescriptor(
        PipelineCreateData* pPipelineCreateData);   ///< Pipeline create data

    /// Destroy a pipeline
    VOID DestroyPipelineDescriptor(
        CHIPIPELINEDESCRIPTOR pipelineHandle);       ///< Pipeline handle

    /// Create a session
    CHIHANDLE CreateSession(
        SessionCreateData* pSessionCreateData);     ///< Session create data

    /// Destroy a session
    VOID DestroySession(
        CHIHANDLE sessionHandle,
        BOOL isForced);                             ///< Pipeline handle

    /// Flush a session
    CDKResult Flush(
        CHIHANDLE sessionHandle);                   ///< Pipeline handle

    /// Submit request
    CDKResult SubmitRequest(
        CHIPIPELINEREQUEST* pSubmitRequestData);     ///< Submit request data

    /// Activate pipeline in a session
    CDKResult ActivatePipeline(
        CHIHANDLE             sessionHandle,        ///< Session handle
        CHIPIPELINEDESCRIPTOR pipelineHandle);      ///< Pipeline handle

    /// Deactivate pipeline in a session
    CDKResult DeactivatePipeline(
        CHIHANDLE                 sessionHandle,        ///< Session handle
        CHIPIPELINEDESCRIPTOR     pipelineHandle,       ///< Pipeline handle
        CHIDEACTIVATEPIPELINEMODE mode);                ///< Deactivate pipeline mode

    /// Create a CHI fence
    CDKResult CreateChiFence(
        CHIFENCECREATEPARAMS* pInfo,
        CHIFENCEHANDLE*       phChiFence);

    /// Release a Chi fence
    CDKResult ReleaseChiFence(
        CHIFENCEHANDLE hChiFence);

    /// Wait on a CHI fence asynchronously
    CDKResult WaitChiFenceAsync(
        PFNCHIFENCECALLBACK pCallbackFn,
        CHIFENCEHANDLE      hChiFence,
        VOID*               pData);

    /// Signal a CHI fence
    CDKResult SignalChiFence(
        CHIFENCEHANDLE hChiFence,
        CDKResult      statusResult);

    /// Obtain a CHI fence status
    CDKResult GetChiFenceStatus(
        CHIFENCEHANDLE hChiFence,
        CDKResult*     pFenceResult);

    /// Query metadata information from the pipeline
    CDKResult QueryPipelineMetadataInfo(
        CHIHANDLE                   sessionHandle,          ///< Session handle
        CHIPIPELINEDESCRIPTOR       pipelineHandle,         ///< Pipeline handle
        CHIPIPELINEMETADATAINFO*    pPipelineMetadataInfo); ///< metadata information

    /// Enumerate sensor modes given the physical camera ID
    CDKResult EnumerateSensorModes(
        UINT32             physCameraId,
        UINT32             numSensorModes,
        CHISENSORMODEINFO* pSensorModeInfo);         ///< Enumerate sensor modes

    /// Interface function invoked by the CHI driver to give an opportunity to the override module to take control of a
    /// usecase on configure streams

    /// @brief Called by the driver to allow for override to detect special camera IDs for additional processing
    UINT32 RemapCameraId(
        UINT32              frameworkCameraId,
        CameraIdRemapMode   mode);

    /// @brief Called by the driver to allow for additional override processing during open()
    CDKResult ExtendOpen(
        uint32_t    logicalCameraId,        ///< Logical camera ID.
        VOID*       pPriv);                 ///< private data for the client to make use of during HAL open

                                            /// @brief Called by the driver to allow for additional override processing during open()
    VOID ExtendClose(
        uint32_t    logicalCameraId,        ///< Logical camera ID.
        VOID*       pPriv);                 ///< private data for the client to make use of during HAL open

    VOID GetNumCameras(
        UINT32* pNumFwCameras,              ///< return the number of cameras to expose to the framework
        UINT32* pNumLogicalCameras);        ///< return the number of logical cameras created by the override

    CDKResult GetCameraInfo(
        uint32_t     logicalCameraId,       ///< Camera Id
        camera_info* cameraInfo);           ///< Camera Info

    CDKResult InitializeOverrideSession(
        uint32_t                        logicalCameraId,    ///< Camera Id
        const camera3_device_t*         camera3_device,     ///< Camera device
        const chi_hal_ops_t*            chiHalOps,          ///< Callbacks into the HAL driver
        camera3_stream_configuration_t* pStreamConfig,      ///< Stream config
        int*                            pIsOverride,        ///< TRUE to take control of the usecase
        VOID**                          pPriv);             ///< Private data

    /// Interface function invoked by the CHI driver to allow the override module to finalize session creation
    CDKResult FinalizeOverrideSession(
        const camera3_device_t* camera3_device, ///< Camera device
        VOID*                   pPriv);         ///< Private data

    /// Interface function invoked by the CHI driver to destroy/teardown a session
    VOID TeardownOverrideSession(
        const camera3_device_t* camera3_device, ///< Camera device
        UINT64                  session,        ///< Session Handle
        VOID*                   pPriv);         ///< Private data

    /// Interface function invoked by the CHI driver to pass along the process capture request to the override module
    CDKResult OverrideProcessRequest(
        const camera3_device_t*     camera3_device,     ///< Camera device
        camera3_capture_request_t*  pCaptureRequest,    ///< Capture request
        VOID*                       pPriv);             ///< Private data


    VOID HandleProcessRequestErrorAllPCRs(
        camera3_capture_request_t* pRequest,    ///< Framework request
        UINT32 logicalCameraId);                ///< Logical camera id

    /// For a given Logical Id, Calculate Cost Based On SensorMode For Each Physical Camera
    UINT32 GetMyLogicalCameraCost(
        UINT32                          logicalId,       ///< logical camera id
        camera3_stream_configuration_t* pStreamConfig);  ///< Stream config

    /// Calculate Resource cost of logical camera id
    UINT32 CostOfLogicalCamera(
        UINT32                          logicalId,       ///< logical camera id
        camera3_stream_configuration_t* pStreamConfig);  ///< Stream config

    /// Calculate  Sensor Resolution for a given physical id
    UINT32 GetSelectedResolutionForActiveSensorMode(
        UINT32                          physCameraId,    ///< physical Camera Id
        camera3_stream_configuration_t* pStreamConfig);  ///< Stream config

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// SignalRecoveryCondition
    ///
    /// @brief  Wake up recovery thread
    ///
    /// @param  cameraId        CameraId with which the usecase is associated with
    ///
    /// @return  None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID SignalRecoveryCondition(
        UINT32 cameraId);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// RecoveryThread
    ///
    /// @brief  Create recovery thread
    ///
    /// @param  pThread Data        Handle to context
    ///
    /// @return  None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static VOID* RecoveryThread(
        VOID* pThreadData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// RequestThreadProcessing
    ///
    /// @brief  Where we do recovery
    ///
    /// @return  None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID RequestThreadProcessing();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CheckAndSetRecovery
    ///
    /// @brief  Determine whether recovery can be set in progress
    ///
    /// @return  BOOL
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    BOOL CheckAndSetRecovery();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// TriggerRecovery
    ///
    /// @brief Trigger recovery
    ///
    /// @param  logicalCameraId  The logical camera associated with the usecase to teardown
    ///
    /// @return  None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID TriggerRecovery(
        UINT32 logicalCameraId);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// TeardownOverrideUsecase
    ///
    /// @brief Interface function invoked by the CHI driver to destroy/teardown a Usecase
    ///
    /// @param  camera3_device The logical camera associated with the usecase to teardown
    /// @param  isForced       If true, then initiate flush while the Usecase is destroyed
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID TeardownOverrideUsecase(
        const camera3_device_t* camera3_device,
        BOOL                    isForced);

    /// Interface function invoked by the CHI driver to flush
    CDKResult OverrideFlush(
        const camera3_device_t*     camera3_device);     ///< Camera device

    /// Interface function invoked by the CHI driver to dump
    CDKResult OverrideDump(
        const camera3_device_t*     camera3_device,
        int                         fd);     ///< Camera device

    /// @brief Allows implementation-specific settings to be toggled in the override at runtime
    VOID ModifySettings(
        VOID*       pPriv);                 ///< private data for the client to make use of at runtime in the override

    /// @brief Add any vendor tag specific request template settings
    VOID DefaultRequestSettings(
        uint32_t                  cameraId,
        int                       requestTemplate,
        const camera_metadata_t** settings);

    /// Sets the HAL callbacks
    VOID SetHALOps(
        const chi_hal_ops_t* pHalOps,
        uint32_t logicalCameraId);                  ///< Callback functions

    /// Sets the CHI context ops from the CHI driver
    VOID SetCHIContextOps();

    CHX_INLINE const chi_hal_ops_t* GetHalOps(uint32_t logicalCameraId) const
    {
        if (logicalCameraId < m_numLogicalCameras)
        {
            return m_pHALOps[logicalCameraId];
        }
        else
        {
            return NULL;
        }
    }

    /// Setting to force-disable ZSL...otherwise on by default
    CHX_INLINE BOOL DisableZSL() const
    {
        return *m_pDisableZSL;
    }

    /// Setting to select IHDR usecase supporting in-sensor HDR...otherwise on by default
    BOOL SelectIHDRUsecase() const
    {
        return *m_pSelectIHDRUsecase;
    }

    /// Setting to override the use case by user
    CHX_INLINE UINT OverrideUseCase() const
    {
        return *m_pForceUsecaseId;
    }

    /// Setting to force-use GPU Rotation Node
    CHX_INLINE BOOL UseGPURotationUsecase() const
    {
        return *m_pGPUNodeRotateUsecase;
    }

    /// Setting to force-use GPU Downscale Node
    CHX_INLINE BOOL UseGPUDownscaleUsecase() const
    {
        return *m_pGPUNodeDownscaleUsecase;
    }

    /// Setting to Enable MFNR Usecase
    CHX_INLINE BOOL EnableMFNRUsecase() const
    {
        return *m_pEnableMFNRUsecase;
    }

    /// Setting to Enable MFNR Anchor Selection Algorithm
    CHX_INLINE UINT32 EnableMFNRAnchorSelectionAlgorithm() const
    {
        return *m_pAnchorSelectionAlgoForMFNR;
    }

    /// Setting to Enable MFSR Usecase
    CHX_INLINE BOOL EnableMFSRUsecase() const
    {
        if (NULL != m_pEnableMFSRUsecase)
        {
            return *m_pEnableMFSRUsecase;
        }
        return FALSE;
    }

    /// Setting to Enable HFR Usecase
    CHX_INLINE BOOL EnableHFRNo3AUsecas() const
    {
        return *m_pHFRNo3AUsecase;
    }

    /// Setting to check if unified buffer manager is enabled
    CHX_INLINE BOOL EnableUnifiedBufferManager() const
    {
        return (*m_pEnableUnifiedBufferManager == 0) ? FALSE : TRUE;
    }

    /// Setting to check if CHI late binding is enabled
    CHX_INLINE BOOL EnableCHILateBinding() const
    {
        return (*m_pEnableCHILateBinding == 0) ? FALSE : TRUE;
    }

    CHX_INLINE UINT32 GetForceSensorMode() const
    {
        return *m_pForceSensorMode;
    }

    CHX_INLINE UINT32 EnableFDStreamSupport() const
    {
        return *m_pFDStreamSupport;
    }

    CHX_INLINE BOOL EnableIHDRSnapshot() const
    {
        return *m_pEnableIHDRSnapshot;
    }

    CHX_INLINE BOOL ForceEnableIHDRSnapshot() const
    {
        return static_cast<BOOL>(*m_pForceEnableIHDRSnapshot);
    }

    CHX_INLINE BOOL GetDCVRMode() const
    {
         return *m_pEnableMultiVRCamera;
    }

    CHX_INLINE UINT32 GetStatsSkipPattern() const
    {
        return *m_pStatsSkipPattern;
    }

    CHX_INLINE UINT32 EnableDumpDebugData() const
    {
        return *m_pEnableDumpDebugData;
    }

    /// Setting to Enable EIS V2 Usecase
    CHX_INLINE BOOL EnableEISV2Usecase() const
    {
        return *m_pEISV2Enable;
    }

    /// Setting to Enable EIS V3 Usecase
    CHX_INLINE BOOL EnableEISV3Usecase() const
    {
        return *m_pEISV3Enable;
    }

    CHX_INLINE BOOL UseFeatureForQuadCFA() const
    {
        return *m_pUseFeatureForQCFA;
    }

    CHX_INLINE UINT32 GetAdvanceFeatureMask() const
    {
        return *m_pAdvanceFeataureMask;
    }

    /// Setting to Disable ASD Processing
    CHX_INLINE UINT32 DisableASDProcessing() const
    {
        return *m_pDisableASDProcessing;
    }

    LogicalCameraType GetCameraType(UINT32 cameraId) const;

    CHX_INLINE UINT32 GetDCFrameSyncMode() const
    {
        return *m_pEnableMultiCameraFrameSync;
    }

    CHX_INLINE UINT32 GetOutputFormat() const
    {
        return *m_pOutputFormat;
    }

    CHX_INLINE UINT32 IsCameraClose() const
    {
        BOOL isClosed = FALSE;

        if (NULL != m_pOverrideCameraClose)
        {
            isClosed = *m_pOverrideCameraClose;
        }

        return isClosed;
    }

    CHX_INLINE UINT32 IsCameraOpen() const
    {
        return *m_pOverrideCameraOpen;
    }

    // Get number of PCRs before stream on
    UINT32 GetNumPCRsBeforeStreamOn(VOID* pMetaData) const;

    /// Setting to override the FOVC Usecase settings
    CHX_INLINE UINT EnableFOVCUseCase() const
    {
        return *m_pEnableFOVC;
    }

    /// Setting to Enable Usecase Handling of Partial Data
    CHX_INLINE PartialMetaSupport EnableCHIPartialData() const
    {
        return static_cast<PartialMetaSupport>(*m_pCHIPartialDataSupport);
    }

    /// Setting to Enable Recovery for Usecase Handling of Partial Data
    CHX_INLINE BOOL EnableCHIPartialDataRecovery() const
    {
        return static_cast<BOOL>(*m_pCHIEnablePartialDataRecovery);
    }

    /// Setting to Enable Offline Noise Reprocessing pipeline
    CHX_INLINE BOOL EnableOfflineNoiseReprocessing() const
    {
        return static_cast<BOOL>(*m_pEnableOfflineNoiseReprocessing);
    }

    VOID GetVendorTagOps(
        CHITAGSOPS* pVendorTagOps);

    // Gets the metadata ops table
    VOID GetMetadataOps(
        CHIMETADATAOPS* pMetadataOps);

    // get available request keys for the framework
    CDKResult GetAvailableRequestKeys(
        UINT32   logicalCameraId,
        UINT32*  pTagList,
        UINT32   maxTagListCount,
        UINT32*  pTagCount);

    VOID SearchNumBatchedFrames(
        uint32_t                        cameraId,
        camera3_stream_configuration_t* pStreamConfigs,
        UINT*                           pBatchSize,
        UINT*                           pFPSValue,
        UINT                            maxSessionFps);

    struct CameraDeviceInfo
    {
        const camera3_device_t *m_pCamera3Device; ///< Camera3 device
    };


    Usecase* GetSelectedUsecase(const camera3_device*);

    CHX_INLINE UINT GetUsecaseMaxFPS()
    {
        return m_usecaseMaxFPS;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetPreviewFPS
    ///
    /// @brief  Get the preview stream FPS
    ///
    /// @param  None
    ///
    /// @return FPS value for preview stream
    /////////////////////////////////////////////////////////////////////////////////////////////////
    CHX_INLINE UINT GetPreviewFPS()
    {
        return m_previewFPS;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetVideoFPS
    ///
    /// @brief   Get the video stream FPS
    ///
    /// @param   None
    ///
    /// @return  FPS value for video stream
    /////////////////////////////////////////////////////////////////////////////////////////////////
    CHX_INLINE UINT GetVideoFPS()
    {
        return m_videoFPS;
    }

    CHX_INLINE UINT GetVideoHDRMode()
    {
        return m_VideoHDRMode;
    }

    CHX_INLINE UINT GetNumBatchedFrames()
    {
        return m_usecaseNumBatchedFrames;
    }


    CHX_INLINE UINT32 GetOpMode(UINT32 cameraId)
    {
        return m_operationMode[cameraId];
    }

    VOID ReturnFrameworkResult(
        const camera3_capture_result_t* pResult, UINT32 cameraID);

    VOID ReturnFrameworkMessage(
        const camera3_notify_msg_t *msg, UINT32 cameraID);

    VOID DumpDebugData(UINT32 cameraID);

    UINT32 GetCameraIdfromDevice(const camera3_device_t * pCamera3Device);

    CHX_INLINE UINT IsTorchWidgetUsecase()
    {
        return m_torchWidgetUsecase;
    }

    CHX_INLINE UINT32 GetNumMetadataResults()
    {
        return m_numMetadataResults;
    }

    CHX_INLINE OSLIBRARYHANDLE GetPerfLibHandle()
    {
        return m_perfLibHandle;
    }

    /// Get info Cost Of Any Currently Open Logical Cameras
    CHX_INLINE UINT32 CostOfAnyCurrentlyOpenLogicalCameras()
    {
        return m_resourcesUsed;
    }

    // Mapping the config settings pointer to each variable
    VOID MappingConfigSettings(
        UINT32  numTokens,
        VOID*   pInputTokens);

private:

    /// Sort the Logical Camera configuration by their logical IDs
    CDKResult SortCameras();

    /// Enumerate the cameras
    CDKResult EnumerateCameras();

#if defined(CAMX_ANDROID_API) && (CAMX_ANDROID_API >= 28) //Android-P or better
    VOID updateCameraIdsInCaps();
#endif
    /// Constructor
    ExtensionModule();
    /// Destructor
    ~ExtensionModule();

    /// Do not support the copy constructor or assignment operator
    ExtensionModule(const ExtensionModule& rExtensionModule) = delete;
    ExtensionModule& operator= (const ExtensionModule& rExtensionModule) = delete;
    VOID FreeLastKnownRequestSetting(UINT32 logicalCameraId);

    CHIHANDLE               m_hCHIContext;                          ///< CHI context handle
    ChiFenceOps             m_chiFenceOps;                          ///< CHI Fence Operations
    const chi_hal_ops_t*    m_pHALOps[MaxNumImageSensors];          ///< HAL ops
    UsecaseSelector*        m_pUsecaseSelector;                     ///< Usecase selector
    UsecaseFactory*         m_pUsecaseFactory;                      ///< Usecase factory
    Usecase*                m_pSelectedUsecase[MaxNumImageSensors]; ///< Selected usecase
    UINT32                  m_numPhysicalCameras;                   ///< Number of physical camera sensors in the system
    UINT32                  m_numExposedLogicalCameras;             ///< Number of logical camera exposed to application
    UINT32                  m_numLogicalCameras;                    ///< Number of logical camera in configuration table
    LogicalCameraInfo       m_logicalCameraInfo[MaxNumImageSensors];///< Logical camera information
    UINT32                  m_cameraMap[MaxNumImageSensors];        ///< Camera ID map
    UINT32                  m_cameraReverseMap[MaxNumImageSensors];
    UINT32*                 m_pConfigSettings;                      ///< Configuration settings
    UINT32*                 m_pDisableZSL;
    UINT32*                 m_pForceUsecaseId;
    UINT32*                 m_pEnableFOVC;
    UINT32*                 m_pGPUNodeRotateUsecase;            ///< Select GPU Node Rotate usecase
    UINT32*                 m_pGPUNodeDownscaleUsecase;         ///< Select GPU Node Downscale usecase
    UINT32*                 m_pEnableMFNRUsecase;               ///< Select MFNR usecase
    UINT32*                 m_pAnchorSelectionAlgoForMFNR;      ///< Select MFNR Anchor Selection Algorithm
    UINT32*                 m_pEnableMFSRUsecase;               ///< Select MFSR usecase
    UINT32*                 m_pHFRNo3AUsecase;                  ///< Select HFR without 3A usecase
    UINT32*                 m_pForceSensorMode;                 ///< Select a specific sensor mode
    UINT32*                 m_pEISV2Enable;                     ///< Enable EIS V2
    UINT32*                 m_pEISV3Enable;                     ///< Enable EIS V3
    UINT32*                 m_pDisableASDProcessing;            ///< Disable ASD processing
    UINT32*                 m_pEnableMultiVRCamera;             ///< Enbale VR DC
    UINT32*                 m_pOverrideCameraClose;             ///< Camera close indicator
    UINT32*                 m_pOverrideCameraOpen;              ///< Camera open indicator
    UINT32*                 m_pNumPCRsBeforeStreamOn;           ///< Number of PCRs before stream on
    UINT32*                 m_pStatsSkipPattern;                ///< Stats skip pattern
    UINT32*                 m_pEnableMultiCameraFrameSync;      ///< DC Frame sync enabled or not
    UINT32*                 m_pEnableDumpDebugData;             ///< Dump debug-data into a file, enable or not
    UINT32*                 m_pOutputFormat;                    ///< Output format for IMPL_Defined
    UINT32*                 m_pCHIPartialDataSupport;           ///< CHI Partial Data Handling
    UINT32*                 m_pCHIEnablePartialDataRecovery;    ///< CHI Partial Data Recovery
    UINT32*                 m_pFDStreamSupport;                 ///< Support FD Stream in Real Time
    UINT32*                 m_pSelectIHDRUsecase;               ///< Select IHDR usecase
    UINT32*                 m_pEnableIHDRSnapshot;              ///< Enable IHDR Snapshot
    UINT32*                 m_pEnableUnifiedBufferManager;      ///< Enable Unified Buffer Manager
    UINT32*                 m_pEnableCHILateBinding;            ///< Enable CHI image buffer late binding
    UINT32*                 m_pEnableOfflineNoiseReprocessing;  ///< Enable Offline Noise Reprocessing
    UINT32*                 m_pForceEnableIHDRSnapshot;         ///< Force enable IHDR snapshot for test
    VendorTagInfo           m_pvtVendorTags[NumVendorTags];     ///< List of private vendor tags
    static const UINT       DefaultSettingsNumEntries = 32;     ///< Num of entries
    static const UINT       DefaultSettingsDataSize   = 512;    ///< Data size bytes
    camera_metadata_t*      m_pDefaultSettings;                 ///< Default settings
    UINT*                   m_pUseFeatureForQCFA;               ///< Use FeatureQuadCFA or UsecaseQuadCFA
    UINT*                   m_pDefaultMaxFPS;                   ///< Default MaxFPS
    UINT*                   m_pAdvanceFeataureMask;             ///< Advance Features Mask
    UINT                    m_usecaseMaxFPS;                    ///< Max FPS required for high speed mode
    UINT                    m_previewFPS;                       ///< FPS of the preview stream set by App
    UINT                    m_videoFPS;                         ///< FPS of the video stream set by App
    UINT                    m_VideoHDRMode;                     ///< video HDR mode
    UINT                    m_usecaseNumBatchedFrames;          ///< Number of framework frames batched together if
                                                                ///  batching is enabled
    BOOL                    m_torchWidgetUsecase;               ///< Indicate torch widget usecase.

    PerfLockManager*        m_pPerfLockManager[MaxNumImageSensors];                 ///< PerfLock Manager
    PerfLockType            m_CurrentpowerHint;                 ///< Current power Hint
    PerfLockType            m_previousPowerHint;                ///< Previous power Hint
    UINT                    m_numMetadataResults;               ///< number of metadata results expected from the driver

    UINT32                  m_originalFrameWorkNumber;          ///< Original framework number
    Mutex*                  m_pPCRLock;                         ///< Lock for process capture request
    Mutex*                  m_pRecoveryLock;                    ///< Lock for process capture request
    Mutex*                  m_pDestroyLock;                     ///< Lock for destroying usecase
    Condition*              m_pRecoveryCondition;               ///< Condition to wait for recovery done.
    BOOL                    m_TeardownInProgress;               ///< Flag to indicate teardown is in progress
    BOOL                    m_RecoveryInProgress;               ///< Flag to indicate recovery is in progress
    BOOL                    m_isFlushLocked;                    ///< Is Flush executing with PCR lock
    UINT32                  m_operationMode[MaxNumImageSensors];///< op_mode sent from Frameworks

    camera_metadata_t*      m_pLastKnownRequestSettings[MaxNumImageSensors];        ///< Save last known metadata to send aftet recovery
    UINT32                  m_firstFrameAfterRecovery;          ///< First frame after recovery to send settings
    UINT32                  m_longExposureFrame;                ///< Long exposure snapshot frameNumber
    volatile UINT32         m_aFlushInProgress;                 ///< Is flush in progress
    volatile UINT32         m_aLongExposureInProgress;          ///< Is long exposure in progress

    camera3_stream_configuration_t* m_pStreamConfig[MaxNumImageSensors];        ///< Stream configuration array to use in recovery
    UINT32                          m_SelectedUsecaseId[MaxNumImageSensors];    ///< Selected usecase id to use in recoverys
    CameraDeviceInfo                m_pCameraDeviceInfo[MaxNumImageSensors];    ///< Device info for all logical camera Id

    BOOL                    m_firstResult;                      ///< First result after configure streams
    OSLIBRARYHANDLE         m_perfLibHandle;                    ///< PerfLock Library handle
    BOOL                    m_isUsecaseInBadState;              ///< Flag to indicate if recovery is required
    UINT32                  m_logicalCameraId;                  ///< Logical camera id for this session
    LogicalCameraConfiguration*     m_pLogicalCameraConfigurationInfo;  ///< Array of Logical Cameras
    UINT                            m_numOfLogicalCameraConfiguration;  ///< Number of Logical Cameras
    Mutex*                          m_pTriggerRecoveryLock;             ///< Lock to trigger recovery
    Condition*                      m_pTriggerRecoveryCondition;        ///< Condition to trigger recovery
    BOOL                            m_IsRecoverySignaled;               ///< Is recovery in progress
    PerThreadData                   m_pRecoveryThread;                  ///< Recovery thread
    BOOL                            m_terminateRecoveryThread;          ///< Do we want to terminate the recovery thread

    BOOL                            m_bTeardownRequired[MaxNumImageSensors]; ///< Keep track of usecase to tear down
    UINT32                          m_hasFlushOccurred;                      ///< Keep track of whether flush has happened
    UINT32                          m_singleISPResourceCost;                 ///< Single ISP resource cost
    UINT32                          m_totalResourceBudget;                   ///< Total resource cost
    UINT32                          m_resourcesUsed;                         ///< Total number of resource used
    UINT32                          m_IFEResourceCost[MaxNumImageSensors];   ///< IFE resource per logical camera Id
    Mutex*                          m_pResourcesUsedLock;                    ///< Mutex for access to m_resourcesUsed
};

#endif // CHXEXTENSIONMODULE_H

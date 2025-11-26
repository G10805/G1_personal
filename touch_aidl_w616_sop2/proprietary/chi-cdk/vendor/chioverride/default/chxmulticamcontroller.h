////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxmulticamcontroller.h
/// @brief CHX Multi Cam Controller related class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CHXMULTICAMCONTROLLER_H
#define CHXMULTICAMCONTROLLER_H

#include "chi.h"
#include "chxdefs.h"
#include "chxutils.h"
#include "chivendortag.h"
#include "chxextensionmodule.h"
#include "chxmetadata.h"
#include "cdkutils.h"

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

/// Static constants
static const UINT32 DualCamCount       = 2;
static const UINT32 TripleCamCount     = 3;
static const INT32  FocusDistanceCmMax = 1000;

static const FLOAT PercentMarginHysterisis     = 0.05f;  ///< Threshold for hysteriS in the transition zone
static const FLOAT PercentMarginTransitionZone = 0.20f;  ///< Margin threshold to calculate transition zone
static const FLOAT PercentMarginOverlapZone    = 0.20f;  ///< Margin threshold to calculate the overlap zone

/// Multi-cam controller settings
/// @todo: Move these to CHIOverride settings xml
/* This setting will enable the snapshot fusion.
If set to 1, it will enable capturing snapshots from both the cameras and feeding those
to snapshot fusion algorithm.
If set to 0, a snapshot is captured only from the master camera session at any time. */
#define ENABLE_SNAPSHOT_FUSION 1

/* This setting enables/disables Low Power Mode(LPM). The system is in power optimized state when
in LPM(1) where only one camera- the master camera is streaming. The other camera doesn't stream.
At the time of master role switch, both the cameras stream to switch the master and after
successful role switch, the non-master can be put into LPM.
When disabled(0), both the cameras will stream all the time.
This setting is available only for dual cameras */
#define ENABLE_LPM             1

/*This setting indicates the mechanism used to sync the multiple cameras.
 Various sync mechanisms:
 NO_SYNC    : Absence of any sync mechanism between the two cameras.
 HW_SYNC    : Sensor turns on hw-sync.
 SW_SYNC    : Sensor ensures the phase difference is kept close to zero. (This
              mode is currently not supported)
 HYBRID_SYNC: Sensor turns on hw-sync and also injects phase as needed. */
#define SYNC_MECHANSIM         0

 /* This setting indicates low power modes for the two cameras
 Various low power modes:
 LPM_NONE             : No/Invalid mode
 LPM_SENSOR_SUSPEND   : Sensor sleep/suspend.
 LPM_ISPIF_FRAME_DROP : Sensor streams but ISPIF drops the frames.
 LPM_ISPIF_FRAME_SKIP : Sensor streams and ISPIF skips the frame to control frame rate. (This
 mode is currently not supported)
 LPM_STATS_FPS_CONTROL : Sensor and ISPIF stream but stats module controls the frame rate. */
//#define LPM_MAIN               (LPM_ISPIF_FRAME_DROP)
//#define LPM_AUX                (LPM_SENSOR_SUSPEND)

 /* This setting indicates the minimum zoom value for the snapshot fusion.
 Snapshot fusion is only enabled for zoom equal to and higher than this value. */
#define SNAPSHOT_FUSION_ZOOM_MIN           (1.5)

 /* This setting indicates the maximum zoom value for the snapshot fusion.
 Snapshot fusion is only enabled for zoom equal to and lower than this value. */
#define SNAPSHOT_FUSION_ZOOM_MAX           (1.9)

/* These values indicates the thresholds for the max lux index amd min focus distance in cm
 to enable / disable snapshot fusion. If the current lux index is lower than the threshold
 or if the focus distance is lower than the threshold, snapshot fusion will be disabled.
 The LUX index value will be lower for dark lit scene than bright scene. */
#define SNAPSHOT_FUSION_LUX_IDX_THRESHOLD  (1.0)

#define SNAPSHOT_FUSION_FOCUS_DIST_CM_MIN  (15)

/* This value indicates the threshhold view angle to determine 360 deg camera
 from normal camera. Achieving 360 full view each camera's view angle in usecase
 needs to be more than 180 deg */
#define CAMERA_VIEW_ANGLE_THRESHOLD  (180)

/// @brief Struct to hold spatial alignment shifts
typedef struct PixelShift
{
    INT32       xShift;                 ///< Horizontal shift
    INT32       yShift;                 ///< Vertical shift
} PixelShift;

/// @brief Struct to hold camera info
typedef struct CamInfo {
    UINT32                             camId;               ///< Camera Id
    CHIDIMENSION                       sensorOutDimension;  ///< Sensor out dimension
    CHIRECT                            fovRectIFE;          ///< IFE FOV wrt to active sensor array
    const CHICAMERAINFO*               pChiCamInfo;         ///< Pointer to ChiCameraInfo struct
    const PhysicalCameraConfiguration* pChiCamConfig;       ///< Camera configuration
    UINT32                             horizontalBinning;   ///< Horizontal binning value
    UINT32                             verticalBinning;     ///< Vertical binning value
} CamInfo;

/// @brief Struct to hold stream info
typedef struct StreamInfo {
    UINT32               streamType;                ///< Stream type
    UINT32               usage;                     ///< Stream gralloc usage flag
    CHIDIMENSION         streamDimension;           ///< Stream dimension
} StreamInfo;

/// @brief Struct to hold stream configuration
typedef struct StreamConfig {
    UINT32             numStreams;               ///< Number of streams in the configuration
    StreamInfo*        pStreamInfo;              ///< Pointer to the stream info
} StreamConfig;

/// @brief Struct to hold the data to create multicamcontroller
typedef struct MccCreateData {
    UINT32           logicalCameraId;                       ///< Logical camera Id
    UINT32           numBundledCameras;                     ///< Number of  bundled cameras
    CamInfo*         pBundledCamInfo[MaxNumImageSensors];   ///< Array of pointers for bundled camera info
    StreamConfig*    pStreamConfig;                         ///< Pointer to stream config info
    UINT32           logicalCameraType;                     ///< Logical camera type
    UINT32           primaryCameraId;                       ///< Primary Camera wrt App from the set of
                                                            ///  physical cameras in the logical camera
} MccCreateData;

/// @brief Struct for per camera settings
typedef struct CameraSettings
{
    UINT32       cameraId;      ///< Camera Id
    ChiMetadata* pMetadata;     ///< Pointer to associated metadata
} CameraSettings;

/// @brief Struct for result metadata for multiple cameras
typedef struct MulticamResultMetadata {
    UINT32         frameNum;                                  ///< Current frame number
    UINT32         numResults;                                ///< Number of results
    ChiMetadata**  ppResultMetadata;                          ///< Pointer of pointers for settings buffers
} MulticamResultMetadata;

/// @brief Struct for active camera info
typedef struct ActiveCameras
{
    UINT32      cameraId;                           ///< Camera Id
    BOOL        isActive;                           ///< Boolean indicating if the camera is active
} ActiveCameras;

/// @brief Struct to hold controller result
typedef struct ControllerResult
{
    BOOL             isValid;                                    ///< Boolean indicating if the result is valid
    BOOL             snapshotFusion;                             ///< Boolean indicating if the snapshot fusion is enabled
    UINT32           masterCameraId;                             ///< Camera Id for the master camera
    CameraRoleType   masterCameraRole;                           ///< Camera role for the master camera
    ActiveCameras    activeCameras[MaxDevicePerLogicalCamera];   ///< Camera info indicating the LPM status
    UINT32           numOfActiveCameras;                         ///< Number of active Cameras in the activeCameras array
    UINT32           activeMap;                                  ///< Active map to indicate which pipeline is active
} ControllerResult;

/// @brief Struct for request settings for multiple cameras
typedef struct MulticamReqSettings {
    UINT32                  frameNum;                                ///< Current frame number
    UINT32                  numSettingsBuffers;                      ///< Number of settings buffers
    ControllerResult        currentRequestMCCResult;                 ///< MMC result of current request
    ChiMetadata**           ppSettings;                              ///< Pointer of pointers for settings buffers
    BOOL                    kernelFrameSyncEnable;                   ///< Frame sync enable
} MulticamReqSettings;

/// @brief Calibration data definition
typedef struct CalibrationData
{
    VOID*        pRawOtpData;         ///< Pointer to raw OTP data
    UINT32       rawOtpDataSize;      ///< Size of raw OTP data
    UINT32       refCameraId;         ///< OPT Data wrt reference cameraID
} CalibrationData;

/// @brief Struct for transition between cameras for multiple cameras
typedef struct {
    FLOAT fusionLow;                   ///< Fusion zone low zoom ratio
    FLOAT fusionHigh;                  ///< Fusion zone high zoom ratio
    FLOAT low;                         ///< Transition zone low zoom ratio that determines LPM on/off
    FLOAT high;                        ///< Transition zone high zoom ratio that determines LPM on/off
    FLOAT transitionRatio;             ///< Zoom ratio at which cameras transition
    BOOL  isValid;                     ///< Flag indicating if transition is valid relative to the camera position
    BOOL  smoothTransitionEnabled;     ///< Flag indicating if smooth transition is enabled for the transition zone
} TransitionZone;

/// @brief Struct for per camera settings for multiple cameras
typedef struct MccCamInfo {
    UINT32          cameraId;                           ///< Camera Id
    CHIDIMENSION    sensorOutDimension;                 ///< Sensor out dimension
    CHIRECT         fovRectIFE;                         ///< IFE FOV wrt to active sensor array
    CHIRECT         activeArraySize;                    ///< Sensor active pixel array dimension
    FLOAT           focalLength;                        ///< Focal Length
    FLOAT           pixelPitch;                         ///< Pixel size of camera
    FLOAT           totalZoom;                          ///< Total Zoom
    FLOAT           adjustedFovRatio;                   ///< Adjusted FOV ratio wrt the default camera
    TransitionZone  transitionLeft;                     ///< Transition zone to transition to the left adjacent camera
    TransitionZone  transitionRight;                    ///< Transition zone to transition to the right adjacent camera
    CameraRoleType  cameraRole;                         ///< Camera Role
    CalibrationData otpData[MaxDevicePerLogicalCamera]; ///< OTP Data
    FLOAT           zoom;                               ///< Translated zoom value of the camera
    FLOAT           transitionZoomRatioMin;             ///< Min Zoom ratio at which the camera will be active
    FLOAT           transitionZoomRatioMax;             ///< Max Zoom ratio at which the camera will be active
    BOOL            alwaysOn;                           ///< Indicate if this camera always on
    BOOL            enableSmoothTransition;             ///< Flag indicating if smooth transition is enabled for the camera
    UINT32          horizontalBinning;                  ///< Horizontal binning value
    UINT32          verticalBinning;                    ///< Vertical binning value
} MccCamInfo;

class ZoomTranslator;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief MultiCamController abstract base class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MultiCamController
{
public:
    /// Consolidate the camera info
    static CDKResult ConsolidateCameraInfo(
        LogicalCameraInfo *pLogicalCameraInfo);

    /// Reconfigure the controller
    virtual CDKResult Reconfigure(
        MccCreateData* pMccCreateData);

    /// Translate request settings
    virtual CDKResult TranslateRequestSettings(
        MulticamReqSettings* pMultiCamSettings);

    /// Translate face regions
    virtual VOID TranslateFaceRegions(
        VOID* pResultMetadata);

    /// Translate metadata
    virtual VOID TranslateResultMetadata(
        camera_metadata_t* pResultMetadata);

    /// Process the result metadata
    virtual VOID ProcessResultMetadata(
        ChiMetadata* pResultMetadata);

    /// Destroy the MultiCamController object
    virtual VOID Destroy() = 0;

    /// Get controller result
    virtual ControllerResult GetResult(
        ChiMetadata* pMetadata);

    /// Update controller result
    virtual VOID UpdateResults(
        ChiMetadata* pMetadata);

    /// Function to get the logical camera type
    CHX_INLINE LogicalCameraType GetCameraType() const
    {
        return m_cameraType;
    }

    /// Function to fill the offline metadata
    virtual CDKResult FillOfflinePipelineInputMetadata(
        MulticamResultMetadata* pMultiCamResultMetadata,
        ChiMetadata*                pOfflineMetadata,
        BOOL                        isSnapshotPipeline);

    /// Function to extract the subset of camera metadata from the result metadata
    VOID ExtractCameraMetadata(
        ChiMetadata*    pResultMetadata,
        CameraMetadata* pExtractedCameraMetadata);

    /// Function to set all the dual camera vendor tags in real time requests
    virtual VOID UpdateVendorTags(
        MulticamReqSettings* pMultiCamSettings);

    /// Function to update the scaler crop for snapshot
    virtual VOID UpdateScalerCropForSnapshot(
        ChiMetadata* pMetadata);

protected:

    MultiCamController() = default;
    virtual ~MultiCamController();

    LogicalCameraType          m_cameraType;        ///< Type of the camera
    ControllerResult           m_result;            ///< Controller result
    Mutex*                     m_pLock;             ///< Mutex to protect the result state
    UINT32                     m_frameId;           ///< Current frame id
    VOID*                      m_pRawOTPData;       ///< Pointer to raw OTP data
    UINT32                     m_rawOTPDataSize;    ///< Size of raw OTP data
    UINT32                     m_numOfDevices;      ///< How many realtime devices

private:

    /// Do not allow the copy constructor or assignment operator
    MultiCamController(const MultiCamController& rMultiCamController) = delete;
    MultiCamController& operator= (const MultiCamController& rMultiCamController) = delete;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief DualFovController for optical zoom usecase
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class DualFovController final : public MultiCamController
{
public:

    /// Create controller object
    static DualFovController* Create(
        MccCreateData* pMccCreateData);

    /// Consolidate the camera info
    static CDKResult ConsolidateCameraInfo(
        UINT32            numBundledCameras,
        CHICAMERAINFO**   ppCamInfo,
        CHICAMERAINFO*    pConsolidatedCamInfo);

    /// Reconfigure the controller
    virtual CDKResult Reconfigure(
        MccCreateData* pMccCreateData);

    /// Translate request settings
    virtual CDKResult TranslateRequestSettings(
        MulticamReqSettings* pMultiCamSettings);

    /// Translate face regions
    virtual VOID TranslateFaceRegions(
        VOID* pResultMetadata);

    /// Process the result metadata
    virtual VOID ProcessResultMetadata(
        ChiMetadata* pResultMetadata);

    /// Destroy/Cleanup the object
    virtual VOID Destroy();

    /// Get controller result
    ControllerResult GetResult(
        ChiMetadata* pMetadata);

    /// Update controller result
    VOID UpdateResults(
        ChiMetadata* pMetadata);

    /// Function to fill the offline metadata
    CDKResult FillOfflinePipelineInputMetadata(
        MulticamResultMetadata* pMultiCamResultMetadata,
        ChiMetadata*                pOfflineMetadata,
        BOOL                        isSnapshotPipeline);

    /// Function to update the scaler crop for snapshot
    VOID UpdateScalerCropForSnapshot(
        ChiMetadata* pMetadata);

protected:

private:

    DualFovController() = default;
    virtual ~DualFovController();

    /// Do not allow the copy constructor or assignment operator
    DualFovController(const DualFovController& rDualFovController) = delete;
    DualFovController& operator= (const DualFovController& rDualFovController) = delete;

    CDKResult CalculateTransitionParams();
    VOID SetInitialResultState();
    WeightedRegion TranslateMeteringRegion(WeightedRegion* pRegion, CameraRoleType camRole);
    BOOL isFusionEnabled();

    UINT32              m_camIdWide;                                ///< Wide camera Id
    UINT32              m_camIdTele;                                ///< Tele camera Id
    UINT32              m_camIdLogical;                             ///< Logical camera Id
    FLOAT               m_zoomUser;                                 ///< User zoom value
    FLOAT               m_zoomWide;                                 ///< Total zoom on wide camera
    FLOAT               m_zoomTele;                                 ///< Total zoom on tele camera
    FLOAT               m_adjustedFovRatio;                         ///< Adjusted FOV ratio between cameras
    FLOAT               m_transitionLow;                            ///< Lower transition point to toggle LPM
    FLOAT               m_transitionWideToTele;                     ///< Approx. zoom value for master switch from Wide to Tele
    FLOAT               m_transitionTeleToWide;                     ///< Approx. zoom value for master switch from Tele to Wide
    FLOAT               m_transitionHigh;                           ///< Higher transition point to toggle LPM
    FLOAT               m_focalLengthWide;                          ///< Focal length of wide camera lens
    FLOAT               m_focalLengthTele;                          ///< Focal length of tele camera lens
    FLOAT               m_pixelPitchWide;                           ///< Pixel size of wide camera
    FLOAT               m_pixelPitchTele;                           ///< Pixel size of tele camera
    FLOAT               m_currentLuxIndex;                          ///< Current lux index value
    UINT32              m_currentFocusDistCm;                       ///< Current focus distance in cm
    BOOL                m_isMainCamFovWider;                        ///< Boolean to indicate if main camera has wider fov than aux
    BOOL                m_isVideoStreamSelected;                    ///< Boolean to indicate if the video stream is selected
    CHIDIMENSION        m_sensorDimensionWide;                      ///< Wide sensor output dimension
    CHIDIMENSION        m_sensorDimensionTele;                      ///< Tele sensor output dimension
    CHIDIMENSION        m_previewDimensions;                        ///< Preview dimension
    CHIRECT             m_activeArraySizeWide;                      ///< Wide sensor active pixel array dimension
    CHIRECT             m_activeArraySizeTele;                      ///< Tele sensor active pixel array dimension
    CHIRECT             m_fovRectIFEWide;                           ///< IFE FOV wrt to active sensor array for wide camera
    CHIRECT             m_fovRectIFETele;                           ///< IFE FOV wrt to active sensor array for tele camera
    ZoomTranslator*     m_pZoomTranslator;                          ///< Pointer to zoom translator
    CameraRoleType      m_recommendedMasterCameraRole;              ///< Recommended master camera role
    PixelShift          m_pixelShiftWidePreview;                    ///< Pixel shift applied by SAT on wide camera preview
    PixelShift          m_pixelShiftWideSnapshot;                   ///< Pixel shift applied by SAT on wide camera snapshot
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief RTBController for Real Time Bokeh usecase
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RTBController final : public MultiCamController
{
public:

    /// Create controller object
    static RTBController* Create(
        MccCreateData* pMccCreateData);

    /// Consolidate the camera info
    static CDKResult ConsolidateCameraInfo(
        UINT32            numBundledCameras,
        CHICAMERAINFO**   ppCamInfo,
        CHICAMERAINFO*    pConsolidatedCamInfo);

    /// Translate request settings
    virtual CDKResult TranslateRequestSettings(
        MulticamReqSettings* pMultiCamSettings);

    /// Process the result metadata
    virtual VOID ProcessResultMetadata(
        ChiMetadata* pResultMetadata);

    /// Destroy/Cleanup the object
    virtual VOID Destroy();

    /// Get controller result
    virtual ControllerResult GetResult(
        ChiMetadata* pMetadata);

    /// Update controller result
    VOID UpdateResults(
        ChiMetadata* pMetadata);

    /// Function to fill the offline metadata
    CDKResult FillOfflinePipelineInputMetadata(
        MulticamResultMetadata* pMultiCamResultMetadata,
        ChiMetadata*                pOfflineMetadata,
        BOOL                        isSnapshotPipeline);

    /// Function to update the scaler crop for snapshot
    VOID UpdateScalerCropForSnapshot(
        ChiMetadata* pMetadata);

private:

    RTBController() = default;
    virtual ~RTBController();

    /// Do not allow the copy constructor or assignment operator
    RTBController(const RTBController& rRTBController) = delete;
    RTBController& operator= (const RTBController& rRTBController) = delete;

    VOID SetInitialResultState();
    WeightedRegion TranslateMeteringRegion(WeightedRegion* pRegion, CameraRoleType camRole);

    UINT32              m_camIdWide;                                ///< Wide camera Id
    UINT32              m_camIdTele;                                ///< Tele camera Id
    UINT32              m_camIdLogical;                             ///< Logical camera Id
    FLOAT               m_adjustedFovRatio;                         ///< Adjusted FOV ratio between cameras
    BOOL                m_isMainCamFovWider;                        ///< Boolean to indicate if main camera has wider fov than aux
    CHIRECT             m_activeArraySizeWide;                      ///< Wide sensor active pixel array dimension
    CHIRECT             m_activeArraySizeTele;                      ///< Tele sensor active pixel array dimension
    CameraRoleType      m_recommendedMasterCameraRole;              ///< Recommended master camera role
    CHIRECT             m_fovRectIFEWide;                           ///< IFE FOV wrt to active sensor array for wide camera
    CHIRECT             m_fovRectIFETele;                           ///< IFE FOV wrt to active sensor array for tele camera
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief BayerMonoController for Bayer-Mono use case
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BayerMonoController final : public MultiCamController
{
public:

    /// Create controller object
    static BayerMonoController* Create(
        MccCreateData* pMccCreateData);

    /// Consolidate the camera info
    static CDKResult ConsolidateCameraInfo(
        UINT32            numBundledCameras,
        CHICAMERAINFO**   ppCamInfo,
        CHICAMERAINFO*    pConsolidatedCamInfo);

    /// Reconfigure the controller
    virtual CDKResult Reconfigure(
        MccCreateData* pMccCreateData);

    /// Translate request settings
    virtual CDKResult TranslateRequestSettings(
        MulticamReqSettings* pMultiCamSettings);

    /// Process the result metadata
    virtual VOID ProcessResultMetadata(
        ChiMetadata* pResultMetadata);

    /// Destroy/Cleanup the object
    virtual VOID Destroy();

private:

    BayerMonoController() = default;
    virtual ~BayerMonoController();

    /// Do not allow the copy constructor or assignment operator
    BayerMonoController(const BayerMonoController& rBayerMonoController) = delete;
    BayerMonoController& operator= (const BayerMonoController& rBayerMonoController) = delete;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief VRController for VR use case
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class VRController final : public MultiCamController
{
public:

    /// Create controller object
    static VRController* Create(
        MccCreateData* pMccCreateData);

    /// Consolidate the camera info
    static CDKResult ConsolidateCameraInfo(
        UINT32            numBundledCameras,
        CHICAMERAINFO**   ppCamInfo,
        CHICAMERAINFO*    pConsolidatedCamInfo);

    /// Reconfigure the controller
    virtual CDKResult Reconfigure(
        MccCreateData* pMccCreateData);

    /// Translate request settings
    virtual CDKResult TranslateRequestSettings(
        MulticamReqSettings* pMultiCamSettings);

    /// Process the result metadata
    virtual VOID ProcessResultMetadata(
        ChiMetadata* pResultMetadata);

    /// Destroy/Cleanup the object
    virtual VOID Destroy();

private:

    VRController() = default;
    virtual ~VRController();

    /// Do not allow the copy constructor or assignment operator
    VRController(const BayerMonoController& rVRController) = delete;
    VRController& operator= (const VRController& rVRController) = delete;

    UINT32              m_camIdVRMaster;                           ///< VR Master camera Id
    UINT32              m_camIdVRSlave;                            ///< VR Slave camera Id
    UINT32              m_camIdLogical;                            ///< Logical camera Id
    CameraRoleType      m_recommendedMasterCameraRole;             ///< Recommended master camera role
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief MultiFovController for optical zoom usecase
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MultiFovController final : public MultiCamController
{
public:

    /// Create controller object
    static MultiFovController* Create(
        MccCreateData* pMccCreateData);

    /// Consolidate the camera info
    static CDKResult ConsolidateCameraInfo(
        LogicalCameraInfo *pLogicalCameraInfo);

    /// Reconfigure the controller
    virtual CDKResult Reconfigure(
        MccCreateData* pMccCreateData);

    /// Translate request settings
    virtual CDKResult TranslateRequestSettings(
        MulticamReqSettings* pMultiCamSettings);

    /// Process the result metadata
    virtual VOID ProcessResultMetadata(
        ChiMetadata* pResultMetadata);

    /// Translate face regions
    virtual VOID TranslateFaceRegions(
        VOID* pResultMetadata);

    /// Translate result metadata
    virtual VOID TranslateResultMetadata(
        camera_metadata_t* pResultMetadata);

    /// Destroy/Cleanup the object
    virtual VOID Destroy();

    /// Get controller result
    ControllerResult GetResult(
        ChiMetadata* pMetadata);

    /// Function to fill the offline metadata
    CDKResult FillOfflinePipelineInputMetadata(
        MulticamResultMetadata* pMultiCamResultMetadata,
        ChiMetadata*                pOfflineMetadata,
        BOOL                        isSnapshotPipeline);

    /// Function to update the scaler crop for snapshot
    VOID UpdateScalerCropForSnapshot(
        ChiMetadata* pMetadata);


    /// Function to set all the dual camera vendor tags in real time requests
    VOID UpdateVendorTags(
        MulticamReqSettings* pMultiCamSettings);

    /// Update controller result
    VOID UpdateResults(
        ChiMetadata* pMetadata);

    /// Function to find Metadata
    CHX_INLINE static INT FindMetadata(
        ChiMetadata            *pMetadata,
        UINT32                  tag,
        camera_metadata_entry_t *entry)
    {
        //return find_camera_metadata_entry(meta, tag, entry);
        return pMetadata->FindTag(tag, entry);
    }

    /// Function to Update Metadata
    CHX_INLINE static INT UpdateMetadata(
        camera_metadata_t       *meta,
        size_t                  index,
        const void              *data,
        size_t                  data_count,
        camera_metadata_entry_t *entry)
    {
        return update_camera_metadata_entry(meta, index, data, data_count, entry);
    }

    /// Function to Set Metadata
    static CDKResult SetMetadata(
        ChiMetadata    *pMetaData,
        UINT32         tag,
        VOID*          pData,
        SIZE_T         count);

protected:

private:

    MultiFovController() = default;
    virtual ~MultiFovController();

    /// Do not allow the copy constructor or assignment operator
    MultiFovController(const MultiFovController& rMultiFovController) = delete;
    MultiFovController& operator= (const MultiFovController& rMultiFovController) = delete;

    /// Function to calculate defualt transition params
    CDKResult CalculateTransitionParams(MccCreateData* pMccCreateData);

    /// Function to calculate defualt transition params
    VOID SetInitialResultState();

    /// Function to Translate metering region
    WeightedRegion TranslateMeteringRegion(WeightedRegion* pRegion, UINT32 cameraId);

    /// Function to check if Fusion is enabled
    BOOL isFusionEnabled();

    /// Function to Initialize the Zoom translator
    CDKResult InitZoomTranslator(ZoomTranslator* pZoomTranslator);

    /// Helper Function to get Camera ID from Role
    UINT32 GetCameraIdFromRole(CameraRoleType camRole);

    /// Helper Function to get Camera Index from Role
    UINT32 GetCameraIndexFromRole(CameraRoleType camRole);

    /// Helper Function to get Camera Index from camera ID
    UINT32 GetCameraIndex(UINT32 cameraID);

    /// Helper Function to get zoom index from camera ID
    UINT32 GetZoomIndex(
        UINT32   *pCamIdList,
        UINT32   cameraID);

    ///Function to Translate Reference Crop window
    CDKResult TranslateReferenceCropWindow(
        CameraSettings *primarySettings,
        CameraSettings *ptranslatedSettings,
        UINT32         numOfTranslatedSettings,
        UINT32         primaryCamIdx);

    /// Function to translate ROI
    CDKResult TranslateROI(
        CameraSettings *primarySettings,
        CameraSettings *ptranslatedSettings,
        UINT32         numOfTranslatedSettings);

    /// Function to Check and override MCC result
    VOID CheckOverrideMccResult(FLOAT userzoom);

    /// Function to consolidate Stream Config
    static CDKResult ConsolidateStreamConfig(
        camera_metadata_t   *pConsolidatedMetadata,
        camera_metadata_t   *pPrimaryCamSettings,
        camera_metadata_t   **pAuxCamSettings,
        UINT32              numAuxSettings);

    /// Function to consolidate min fps range and stall durations.
    static CDKResult ConsolidateMinFpsStallDuration(
        camera_metadata_t   *pConsolidatedMetadata,
        camera_metadata_t   *pPrimaryCamSettings,
        camera_metadata_t   **pAuxCamSettings,
        UINT32              numAuxSettings);

    /// Function to calculate Fusion params
    void CalculateFusionParams(TransitionZone *pTransitionZone);

    /// Function to check if smoothZoom is enabled for a given camera ID
    BOOL IsSmoothZoomEnabled(UINT32 cameraID);

    MccCamInfo       m_camInfo[MaxDevicePerLogicalCamera];  ///< Per camera Info
    UINT32           m_primaryCamId;                        ///< Default camera wrt App
    UINT32           m_numOfLinkedSessions;                 ///< Number of linked sessions
    UINT32           m_camIdLogical;                        ///< Logical camera Id
    FLOAT            m_zoomUser;                            ///< User zoom value
    FLOAT            m_currentLuxIndex;                     ///< Current lux index value
    UINT32           m_currentFocusDistCm;                  ///< Current focus distance in cm
    BOOL             m_isVideoStreamSelected;               ///< Boolean to indicate if the video stream is selected
    CHIDIMENSION     m_previewDimensions;                   ///< Preview dimension
    ZoomTranslator*  m_pZoomTranslator;                     ///< Pointer to zoom translator
    CameraRoleType   m_recommendedMasterCameraRole;         ///< Recommended master camera role
    UINT32           m_recommendedMasterCameraId;           ///< Recommended master camera id
    PixelShift       m_pixelShiftPreview;                   ///< Pixel shift applied by SAT on primary camera preview
    PixelShift       m_pixelShiftSnapshot;                  ///< Pixel shift applied by SAT on primary camera snapshot
    BOOL             m_smoothTransitionDisabled;            ///< Flag to indicate if smooth transition has been disabled
                                                            ///  on any of the cameras
    FLOAT            m_maxUserZoom;                         ///< Max user Zoom
    BOOL             m_overrideMode;                        ///< MCC is in override mode and is making transition decisions
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief MultiRTBController for optical zoom usecase
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MultiRTBController final : public MultiCamController
{
public:

    /// Create controller object
    static MultiRTBController* Create(
        MccCreateData* pMccCreateData);

    /// Consolidate the camera info
    static CDKResult ConsolidateCameraInfo(
        LogicalCameraInfo* pLogicalCameraInfo);

    /// Reconfigure the controller
    virtual CDKResult Reconfigure(
        MccCreateData* pMccCreateData);

    /// Translate request settings
    virtual CDKResult TranslateRequestSettings(
        MulticamReqSettings* pMultiCamSettings);

    /// Process the result metadata
    virtual VOID ProcessResultMetadata(
        ChiMetadata* pResultMetadata);

    /// Destroy/Cleanup the object
    virtual VOID Destroy();

    /// Get controller result
    ControllerResult GetResult(
        ChiMetadata* pMetadata);

    /// Function to fill the offline metadata
    CDKResult FillOfflinePipelineInputMetadata(
        MulticamResultMetadata     *pMultiCamResultMetadata,
        ChiMetadata                *pOfflineMetadata,
        BOOL                        isSnapshotPipeline);

    /// Function to set all the dual camera vendor tags in real time requests
    VOID UpdateVendorTags(MulticamReqSettings* pMultiCamSettings);

    /// Function to update the scaler crop for snapshot
    VOID UpdateScalerCropForSnapshot(
        ChiMetadata* pMetadata);

    /// Update controller result
    VOID UpdateResults(
        ChiMetadata* pMetadata);

    /// Function to find Metadata
    CHX_INLINE static INT FindMetadata(
        ChiMetadata             *pMetadata,
        UINT32                  tag,
        camera_metadata_entry_t *entry)
    {
        return pMetadata->FindTag(tag, entry);
    }

    /// Function to Update Metadata
    CHX_INLINE static INT UpdateMetadata(
        camera_metadata_t       *meta,
        size_t                  index,
        const void              *data,
        size_t                  data_count,
        camera_metadata_entry_t *entry)
    {
        return update_camera_metadata_entry(meta, index, data, data_count, entry);
    }

    /// Function to Set Metadata
    static CDKResult SetMetadata(
        ChiMetadata    *pMetaData,
        UINT32         tag,
        VOID*          pData,
        SIZE_T         count);

protected:

private:

    MultiRTBController() = default;
    virtual ~MultiRTBController();

    /// Do not allow the copy constructor or assignment operator
    MultiRTBController(const MultiFovController& rMultiFovController) = delete;
    MultiRTBController& operator= (const MultiFovController& rMultiFovController) = delete;

    /// Function to calculate defualt transition params
    CDKResult CalculateTransitionParams();

    /// Function to calculate defualt transition params
    VOID SetInitialResultState();

    /// Function to Translate metering region
    WeightedRegion TranslateMeteringRegion(WeightedRegion* pRegion, UINT32 cameraId);

    /// Helper Function to get Camera ID from Role
    UINT32 GetCameraIdFromRole(CameraRoleType camRole);

    /// Helper Function to get Camera Index from Role
    UINT32 GetCameraIndexFromRole(CameraRoleType camRole);

    /// Helper Function to get Camera Index from camera ID
    UINT32 GetCameraIndex(UINT32 cameraID);

    ///Function to Translate Reference Crop window
    CDKResult TranslateReferenceCropWindow(
        CameraSettings *primarySettings,
        CameraSettings *ptranslatedSettings,
        UINT32         numOfTranslatedSettings,
        UINT32         primaryCamIdx);

    /// Function to translate ROI
    CDKResult TranslateROI(
        CameraSettings *primarySettings,
        CameraSettings *ptranslatedSettings,
        UINT32         numOfTranslatedSettings);

    /// Function to consolidate Stream Config
    static CDKResult ConsolidateStreamConfig(
        camera_metadata_t   *pConsolidatedMetadata,
        camera_metadata_t   *pPrimaryCamSettings,
        camera_metadata_t   *pAuxCamSettings[],
        UINT32              numAuxSettings);

    /// Function to consolidate min fps range and stall durations.
    static CDKResult ConsolidateMinFpsStallDuration(
        camera_metadata_t   *pConsolidatedMetadata,
        camera_metadata_t   *pPrimaryCamSettings,
        camera_metadata_t   *pAuxCamSettings[],
        UINT32              numAuxSettings);

    MccCamInfo       m_camInfo[MaxDevicePerLogicalCamera];   ///< Per camera Info
    UINT32           m_primaryCamId;                         ///< Default camera wrt App
    UINT32           m_numOfLinkedSessions;                  ///< Number of linked sessions
    UINT32           m_camIdLogical;                         ///< Logical camera Id
    FLOAT            m_zoomUser;                             ///< User zoom value
    UINT32           m_currentFocusDistCm;                   ///< Current focus distance in cm
    CHIDIMENSION     m_previewDimensions;                    ///< Preview dimension
    ZoomTranslator*  m_pZoomTranslator;                      ///< Pointer to zoom translator
    CameraRoleType   m_recommendedMasterCameraRole;          ///< Recommended master camera role
    UINT32           m_recommendedMasterCameraId;            ///< Recommended master camera id
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief MultiCamControllerManager is a singleton class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MultiCamControllerManager
{
public:

    /// Get the instance of manager object
    static MultiCamControllerManager* GetInstance()
    {
        static MultiCamControllerManager s_controllerManagerSingleton;

        return &s_controllerManagerSingleton;
    }

    /// Get MultiCamController object
    MultiCamController* GetController(
        MccCreateData* pMccCreateData);

    /// Destroy MultiCamController object
    VOID DestroyController(
        MultiCamController* pMultiCamController);

    /// Destroy MultiCamControllerManager object
    VOID Destroy();

    static ChiVendorTagsOps     s_vendorTagOps;                   ///< CHI vendor tag ops
    static UINT32               m_vendorTagOpticalZoomResultMeta; ///< Vendor tag of OZ result metadata
    static UINT32               m_vendorTagOpticalZoomInputMeta;  ///< Vendor tag of OZ input metadata
    static UINT32               m_vendorTagBokehResultMeta;       ///< Vendor tag of Bokeh result metadata
    static UINT32               m_vendorTagBokehInputMeta;        ///< Vendor tag of Bokeh input metadata
    static UINT32               m_vendorTagMultiCameraRole;       ///< Vendor tag of camera role
    static UINT32               m_vendorTagCropRegions;           ///< Vendor tag of capture request crop regions
    static UINT32               m_vendorTagIFEAppliedCrop;        ///< Vendor tag of IFE applied crop
    static UINT32               m_vendorTagSensorIFEAppliedCrop;  ///< Vendor tag of IFE applied crop
    static UINT32               m_vendorTagIFEResidualCrop;       ///< Vendor tag of IFE residual crop
    static UINT32               m_vendorTagLuxIndex;              ///< Vendor tag of LUX index
    static UINT32               m_vendorTagMasterInfo;            ///< Vendor tag of master camera
    static UINT32               m_vendorTagLPMInfo;               ///< Vendor tag for LPM info
    static UINT32               m_vendorTagSyncInfo;              ///< Vendor tag for SYNC info
    static UINT32               m_vendorTagCameraType;            ///< Vendor tag for camera type
    static UINT32               m_vendorTagReferenceCropSize;     ///< Vendor tag for camera type
    static UINT32               m_vendorTagMetadataOwner;         ///< Vendor tag for metadata owner

protected:
private:

    MultiCamController* m_pController[MaxNumImageSensors];          ///< Array to hold per camera controller

    MultiCamControllerManager();
    virtual ~MultiCamControllerManager();

    /// Do not allow the copy constructor or assignment operator
    MultiCamControllerManager(const MultiCamControllerManager& MultiCamControllerManager) = delete;
    MultiCamControllerManager& operator= (const MultiCamControllerManager& MultiCamControllerManager) = delete;
};

#endif // CHXMULTICAMCONTROLLER_H

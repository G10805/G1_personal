////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file chimulticamera.h
/// @brief Define Qualcomm Technologies, Inc. Multicamera input / output Meta data information
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CHIMULTICAMERA_H
#define CHIMULTICAMERA_H

#include "camxcdktypes.h"
#include "chicommontypes.h"
#include "chi.h"

#ifdef __cplusplus

extern "C"
{
#endif // __cplusplus

/// @brief Data structures that are exposed to OEM must be packed to have the expected layout.
#pragma pack(push, CDK_PACK_SIZE)

/// @brief Defines maximum number of nodes for FOVs
#define MAX_NODES_NUMBER              4

/// @brief Defines maximum number of streams for a camera
#define MAX_NUM_STREAMS               11

/// @brief Max number of faces detected
static const UINT8 FDMaxFaces        = 10;

/// @brief Defines maximum number of streams for a camera
static const UINT32 MaxLinkedCameras = 4;

///@brief Enumeration describing sensor mode
typedef enum
{
    NoSync = 0,      ///< No Hardware Sync
    MasterMode,      ///< Master Mode
    SlaveMode,       ///< Slave Mode
} SensorSyncMode;

///@brief Enumeration describing sensor status
typedef enum
{
    SensorNoPerf = 0,///< No Perf Ctrl
    SensorActive,    ///< Sensor In Active Status
    SensorStandby,   ///< Sensor In Sleeping Status
} SensorPerfCtrl;

///@brief Enumeration describing Buffer downscale type
typedef enum
{
    BUFFER_FULL,                ///< Buffer is full sized - no downscaling
    BUFFER_DS4,                 ///< Buffer is downscaled by 4
    BUFFER_DS16,                ///< Buffer is downscaled by 16
    MAX_BUFFER_DOWNSCALE_TYPES  ///< Max number of downscaling types
} BufferDownscaleType;

///@brief Structure describing sensor hardware sync mode configure
///Vendor tag section name:com.qti.chi.multicamerasensorconfig
///Vendor tag tag name:sensorsyncmodeconfig
typedef struct
{
    BOOL           isValid;        ///< flag to indicate if sync mode meta is valid
    SensorSyncMode sensorSyncMode; ///< sensor sync mode
} SensorSyncModeMetadata;

///@brief Enumeration describing sensor IHDR state
typedef enum
{
    IHDR_NONE   = 0,
    IHDR_START,
    IHDR_ENABLED,
    IHDR_STOP
}IHDRSnapshotState;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Structure describing the image size
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    INT                   width;                        ///< Image size width value
    INT                   height;                       ///< Image size height value
} ImageSizeData;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief structure for refernce crop window size
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    INT32 width;       ///< width for reference crop window
    INT32 height;      ///< height for reference crop window
} RefCropWindowSize;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Structure describing shift offset information
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    INT32                horizonalShift;               ///< x cordinate of the pixel
    INT32                verticalShift;                ///< y cordinate of the pixel
} ImageShiftData;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Struct for a weighted region used for focusing/metering areas
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    INT32                xMin;                           ///< Top-left x-coordinate of the region
    INT32                yMin;                           ///< Top-left y-coordinate of the region
    INT32                xMax;                           ///< Width of the region
    INT32                yMax;                           ///< Height of the region
    INT32                weight;                         ///< Weight of the region
} WeightedRegion;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Struct for a weighted region used for focusing/metering areas
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    UINT8                numFaces;                       ///< Number of faces detected
    UINT8                faceScore[FDMaxFaces];          ///< List of face confidence scores for detected faces
    CHIRECT              faceRect[FDMaxFaces];           ///< List of face rect information for detected faces
} FDMetadataResults;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Enumeration describing Camera Role
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
    CameraRoleTypeDefault                = 0,            ///< Camera type Default
    CameraRoleTypeUltraWide,                             ///< Camera type ultra wide
    CameraRoleTypeWide,                                  ///< Camera type wide
    CameraRoleTypeTele,                                  ///< Camera type tele
    CameraRoleTypeVR,                                    ///< Camera type VR 360
    CameraRoleTypeMax,                                   ///< Max role to indicate error
} CameraRoleType;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Structure describing Low Power mode
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    CameraRoleType        cameraRole;                   ///< Camera role of the chosen frame
    UINT32                cameraId;                     ///< Camera type of the chosen frame
    BOOL                  isEnabled;                    ///< Flag to check if Camera is active
} LowPowerModeData;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Structure describing camera metadata needed by SAT node
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    BOOL                  isValid;                      ///< If this metadata is valid
    UINT32                cameraId;                     ///< Camera Id this metadata belongs to
    CameraRoleType        cameraRole;                   ///< Camera role this metadata belongs to
    UINT32                masterCameraId;               ///< Master camera Id
    CameraRoleType        masterCameraRole;             ///< Master camera role
    CHIRECT               userCropRegion;               ///< Overall user crop region
    CHIRECT               pipelineCropRegion;           ///< Pipeline specific crop region
    CHIRECT               ifeLimitCropRegion;           ///< Max Crop that IFE can apply
    CHIRECT               fovRectIFE;                   ///< IFE FOV wrt to active sensor array
    CHIRECT               fovRectIPE;                   ///< IPE FOV wrt to active sensor array
    CHIRECT               ifeAppliedCrop;               ///< Crop applied by IFE
    CHIRECT               sensorIFEAppliedCrop;         ///< Crop applied by Sensor+IFE
    CHIRECT               ifeResidualCrop;              ///< Crop remaining after IFE processing
    WeightedRegion        afFocusROI;                   ///< AF focus ROI
    CHIRECT               activeArraySize;              ///< Wide sensor active array size
    FLOAT                 lux;                          ///< LUX value
    INT32                 focusDistCm;                  ///< Focus distance in cm
    UINT32                afState;                      ///< AF state
    INT32                 isoSensitivity;               ///< ISO value
    INT64                 exposureTime;                 ///< Exposure time
    UINT64                sensorTimestamp;              ///< Sensor timestamp
    FDMetadataResults     fdMetadata;                   ///< Face detection metadata
} CameraMetadata;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Structure mapping physicalCameraId to a Node input port
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    CHILINKNODEDESCRIPTOR nodeDescriptor;
    UINT                  physicalCameraId;    ///< The physical cameraId associated with this nodeDescriptor
    BufferDownscaleType   bufferDownscaleType; ///< The buffer downscale type
} PhysicalCameraInputConfiguration;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Metadata structure containing a list of physicalCameraId to node input mappings
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    UINT                             numConfigurations;                 ///< The number of physical camera input configurations
    PhysicalCameraInputConfiguration configuration[MaxNumImageSensors]; ///< Array of physicalCameraId to Node Input properties
} ChiPhysicalCameraConfig;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Structure describing Multi camera SAT input meta data structure
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    UINT32                frameId;                              ///< Frame ID
    UINT8                 numInputs;                            ///< Number of input metadata
    ImageShiftData        outputShiftSnapshot;                  ///< Snapshot frame shift due to spatial alignment
    CameraMetadata        cameraMetadata[MaxLinkedCameras];     ///< Camera metdata for SAT
} InputMetadataOpticalZoom;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Structure describing Multi camera SAT output meta data structure
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    UINT32                masterCameraId;                   ///< Current master camera id
    CameraRoleType        masterCamera;                     ///< Current master camera role
    UINT32                recommendedMasterCameraId;        ///< Recommended master camera id
    CameraRoleType        recommendedMasterCamera;          ///< Current master camera role
    ImageShiftData        outputShiftPreview;               ///< Preview frame shift due to spatial alignment
    ImageShiftData        outputShiftSnapshot;              ///< Snapshot frame shift due to spatial alignment
    ImageSizeData         refResForOutputShift;             ///< Reference resolution for the output shift
    CHIRECT               outputCrop;                       ///< Output Crop information
    UINT8                 numLinkedCameras;                 ///< Number of linked physical cameras
    LowPowerModeData      lowPowerMode[MaxLinkedCameras];   ///< output lower power mode information
    BOOL                  hasProcessingOccured;             ///< True if the SAT algorithm has processed this frame
                                                            ///  False if SAT bypasses or copys its input to output
} OutputMetadataOpticalZoom;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Structure describing physical camera configuration
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct CameraConfiguration
{
    UINT32              cameraId;                ///< Camera Id
    CameraRoleType      cameraRole;              ///< Camera Role
    FLOAT               transitionZoomRatioLow;  ///< Min Transition Zoom ratio at which the camera will be active
    FLOAT               transitionZoomRatioHigh; ///< Max Transition Zoom ratio at which the camera will be active
    BOOL                enableSmoothTransition;  ///< Enable Smooth Transition for the camera during SAT
    CHISENSORCAPS       sensorCaps;              ///< Capabilities related to the device's imaging characteristics
    CHILENSCAPS         lensCaps;                ///< Capabilities related to the device's lens characteristics
    CHISENSORMODEINFO   sensorModeInfo;          ///< Sensor mode information
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Structure describing set of physical configs
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct CameraConfigs
{
    UINT32              numPhysicalCameras;                ///< Number of physical cameras
    CameraConfiguration cameraConfigs[MaxLinkedCameras];   ///< Physical configuration of cameras
    UINT32              primaryCameraId;                   ///< Primary camera in the given Logical camera
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Structure describing Multi camera RTB input meta data structure
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    UINT32                frameId;                            ///< Frame ID
    UINT32                blurLevel;                          ///< Blur intensity value to be applied to the Bokeh
    UINT32                blurMinValue;                       ///< Blur minimum value
    UINT32                blurMaxValue;                       ///< Blur maximum value
    CameraMetadata        cameraMetadata[MaxLinkedCameras];   ///< Camera metdata for RTB
    UINT8                 numLinkedCameras;                   ///< Number of linked physical cameras
} InputMetadataBokeh;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Structure describing Multi camera RTB output meta data structure
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    UINT32                masterCameraId;                   ///< Current master camera id
    CameraRoleType        masterCamera;                     ///< Current master camera role
    UINT32                recommendedMasterCameraId;        ///< Recommended master camera id
    CameraRoleType        recommendedMasterCamera;          ///< Recommended master camera role
    UINT32                activeMap;                        ///< Active map to indicate which pipeline is active
    UINT32                depthEffectInfo;                  ///< Depth Effect status information
} OutputMetadataBokeh;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Structure for Dual camera role, Id
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    CameraRoleType        currentCameraRole;             ///< Current camera role - wide / tele
    UINT32                currentCameraId;               ///< Current camera id
    UINT32                logicalCameraId;               ///< Logical camera id
    CameraRoleType        masterCameraRole;              ///< Master camera role - wide / tele
} MultiCameraIdRole;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Structure for Dual camera low power mode information
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    UINT32                isLPMEnabled;                      ///< Low power mode status
    UINT32                isSlaveOperational;                ///< Is Slave operational when LPM is enabled
    LowPowerModeData      lowPowerMode[MaxLinkedCameras];    ///< LPM info for all the real time pipelines
    UINT8                 numLinkedCameras;                  ///< Number of linked physical cameras
} LowPowerModeInfo;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Structure for Dual camera sync mode information
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    BOOL                  isSyncModeEnabled;                    ///< Sync mode info to sync both the frames
} SyncModeInfo;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Structure for crop regions for capture request
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    CHIRECT               userCropRegion;                ///< Overall user crop region
    CHIRECT               pipelineCropRegion;            ///< Pipeline specific crop region
    CHIRECT               ifeLimitCropRegion;            ///< Max Crop that IFE can apply
} CaptureRequestCropRegions;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Structure for Gamma information
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    BOOL    isGammaValid;              ///< Is Valid Gamma Entries
    UINT32  gammaG[65];                ///< Gamma Green Entries
} GAMMAINFO;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Structure describing the OEM JPEG EXIF App header Data
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    SIZE_T  size;   ///< Size in bytes of the EXIF App header data buffer
    VOID*   pData;  ///< Pointer to the EXIF App header data buffer
} OEMJPEGEXIFAppData;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Structure for Metadata Owner information
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct MetadataOwnerInfo
{
    UINT32                  cameraId;                    ///< Camera Id who owns this metadata
} METADATAOWNERINFO;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // CHIMULTICAMERA_H

//*************************************************************************************************
// Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//*************************************************************************************************

#ifndef __CHIPIPELINE_H__
#define __CHIPIPELINE_H__

#include <map>

#include "CameraResult.h"
#include "chi.h"

// Forward declarations
//
class ChiModule;
class ChiSession;

// Pipeline create data
//
typedef struct _PipelineCreateData
{
    const CHAR*                   pPipelineName;                     ///< Pipeline name
    CHIPIPELINECREATEDESCRIPTOR*  pPipelineCreateDescriptor;         ///< Pipeline create descriptor
    uint32_t                      numOutputs;                        ///< Number of output buffers of this pipeline
    CHIPORTBUFFERDESCRIPTOR*      pOutputDescriptors;                ///< Output buffer descriptors
    uint32_t                      numInputs;                         ///< Number of inputs
    CHIPORTBUFFERDESCRIPTOR*      pInputDescriptors;                 ///< Input buffer descriptors
    CHIPIPELINEINPUTOPTIONS*      pInputBufferRequirements;          ///< Input buffer requirements for this pipeline

} PipelineCreateData;

// This enum defines supported pipeline types.
//
typedef enum _PipelineType
{
    OfflineBPS = 0,
    OfflineSHDR,
    OfflineBPSStats,
    OfflineShdrBps,
    OfflineShdrBpsStatsProc,
    OfflineBPSAWBBGStats,
    OfflineIPE,
    OfflineBPSBGBHISTAWB,
    OfflineShdrBpsAECAWB,
    OfflineBPSAECAWB,
    OfflineBpsIpe,
    OfflineBpsIpeAECAWB,
    OfflineShdrBpsIpeAECAWB,
    PipelineTypeMax
} PipelineType;

const UINT ChiExternalNode              = 255;
typedef enum _NodeId
{
    IFE = (1 << 16),
    JPEG,
    IPE,
    BPS,
    FDHw,
    LRME,
    RANSAC
} NodeId;

typedef enum _BpsPortId
{
    BPSInputPort = 0,               // Full size input
    BPSOutputPortIdFull,            // BPS Image full size
    BPSOutputPortIdDS4,             // BPS down scale 4
    BPSOutputPortIdDS16,            // BPS down scale 16
    BPSOutputPortIdDS64,            // BPS down scale 64
    BPSOutputPortIdStatsBG,         // BPS Bayer Grid stats
    BPSOutputPortIdStatsHDRBHIST,   // BPS HDR Bayer histogram stats
    BPSOutputPortIdReg1,            // BPS Image registration1
    BPSOutputPortIdReg2,            // BPS Image registration2
    BPSOutputPortIdMax              // To use as the number of port resources
} BpsPortId;

typedef enum _ShdrPortId
{
    SHDRInputPort = 0,               // Full size input
    SHDROutputPortIdFull,            // SHDR Image full size
    SHDROutputPortIdMax              // To use as the number of port resources
} SHDRPortId;

/// @brief IpePorts
typedef enum _IpePortId
{
    IPEInputPortFull = 0,
    IPEInputPortDS4,
    IPEInputPortDS16,
    IPEInputPortDS64,
    IPEInputPortFullRef,
    IPEInputPortDS4Ref,
    IPEInputPortDS16Ref,
    IPEInputPortDS64Ref,
    IPEOutputPortDisplay,
    IPEOutputPortVideo,
    IPEOutputPortFullRef,
    IPEOutputPortDS4Ref,
    IPEOutputPortDS16Ref,
    IPEOutputPortDS64Ref
}IpePortId;

// This class implements pipeline configuration functionality
// and holds related data.
//
class ChiPipeline
{
public:
    // This methods creates chi pipeline object.
    //
    // param [in]: pStreams Pointer to stream objects.
    // param [in]: numStream Number of streams in pStream.
    // param [in]: cameraId camera data to be used in the test
    // param [in]: pChiModule Pointer to ChiModule object
    // param [in]: type tells what pipeline type to create.
    //
    // returns: ChiPipeline* Pointer to ChiPipeline instance if successful
    //                     or NULL otherwise.
    //
    static ChiPipeline*     Create(ChiStream*       pStreams,
                                     int            numStreams,
                                     UINT32         cameraId,
                                     ChiModule*     pChiModule,
                                     PipelineType   type);

    // This method sends request to camx to destroy pipeline.
    //
    // returns: none.
    //
    void                    DestroyPipeline();

    // This method retrieves pipeline info
    //
    // param [out]: pPipelineInfo Pointer to pipeline info.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult            GetPipelineInfo(CHIPIPELINEINFO* pPipelineInfo);

    // This method activates pipeline.
    //
    // param [in]: hSession Chi handle to session
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult            ActivatePipeline(ChiSession* pChiSession);

    // This method de-activates pipeline.
    //
    // param [in]: hSession Chi handle to session
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult            DeactivatePipeline(ChiSession* pChiSession);

    // This method returns handle to created pipeline
    //
    // returns: CHIPIPELINEHANDLE
    //
    CHIPIPELINEHANDLE       GetPipelineHandle() { return reinterpret_cast<CHIPIPELINEHANDLE>(m_createdPipeline); }

    // This method query pipeline metadata info.
    //
    // returns: none.
    //
    void       QueryPipelineMetadataInfo(ChiSession* pChiSession);
    // This method check the pipeline for tag.
    //
    // returns: true if its found otherwise false.
    //
    bool       PipelineHasPopulatedTag(UINT32 tagId);

private:

    // This method initializes internal data of chi pipeline object.
    //
    // param [in]: pStreams Pointer to stream objects.
    // param [in]: numStream Number of streams in pStream.
    // param [in]: cameraId camera data to be used in the test
    // param [in]: type tells what pipeline type to create.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult            Initialize(ChiStream*       pStreams,
                                          int           numStreams,
                                          UINT32        cameraId,
                                          PipelineType  type);

    // This method sends request to camx to create pipeline.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult            CreatePipeline();

    CameraResult            CreateMetadata();



    // This method queries camx software to get camera info for all sensor
    // modules discovered during probe operation during camx initialization.
    //
    // The camera info comes from sensor module binary files on the target.
    // These files are created by user with relevant sensor information and
    // are read during camx initializaton. The camera info is stored in the
    // m_sensorInfoTable in the camx. User uses index to this table to specify
    // camera to be used in a pipeline. One of the camera info parameters is
    // sensorId set by user. This api creates mapping between user specified
    // sensorId and index in m_sensorInfoTable that is used by camx.
    // camera id used with camx apis repersents index in the m_sensorInfoTable.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult            DetectCameras();

    // Class constants
    static const unsigned int   m_MaxOutputBufs = 3;
    static const unsigned int   m_MaxCameras = 8;

    ChiModule*                  m_pChiModule;                       // pointer to the ChiModule
    PipelineCreateData          m_pipelineCreateData;               // pipeline data required for creating pipeline
    CHIPORTBUFFERDESCRIPTOR     m_pipelineInputBuffer;              // pipeline input buffer configuration
    CHIPORTBUFFERDESCRIPTOR     m_pipelineOutputBuffer[m_MaxOutputBufs]; // pipeline output buffer configuration
    CHIPIPELINEINPUTOPTIONS     m_pipelineInputBufferRequirements;  // pipeline input options
    CHIPIPELINEDESCRIPTOR       m_createdPipeline;                  // pipeline descriptor of created pipeline
    CHIPIPELINEINFO             m_pipelineInfo;                     // pipeline info
    CHIMETADATAOPS              m_metadataOps;                      // function pointers to all metadata related api
    CHICAMERAINFO               m_chiCameraInfo[m_MaxCameras];      // array to store camera info
    CHICAMERAINFO               m_legacyCameraInfo;                 // legacy camera information
    CHIMETAHANDLE               m_pAndroidDefaultMetaHandle;        // handle to Android metadata
    ChiLinkNodeDescriptor       m_pipelineLinks[4];
    CHIPIPELINEMETADATAINFO     m_pipelineMetadataInfo;             // Metadata info
    UINT32                      m_numDetectedCameras;
    std::map<UINT32, UINT32>    m_cameraUserIdToCamxId;              // mapping between user camera id and camx.

    ChiPipeline(ChiModule*    pChiModule);
    ~ChiPipeline();

    // Do not allow the copy constructor or assignment operator
    ChiPipeline(const ChiPipeline& ) = delete;
    ChiPipeline& operator= (const ChiPipeline& ) = delete;
};


#endif // __CHIPIPELINE_H__

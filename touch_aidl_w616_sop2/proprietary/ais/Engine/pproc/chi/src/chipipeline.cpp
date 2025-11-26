//******************************************************************************************************************************
// Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//******************************************************************************************************************************

#include <string.h>
#include <cstdlib>

#include "camxcdktypes.h"
#include "chituningmodeparam.h"
#include "chipipeline.h"
#include "chisession.h"
#include "chimodule.h"
#include "ais_log.h"
#include "chipipelines_def.h"

#define ISP_LOG(lvl, fmt...) AIS_LOG(PPROC_ISP, lvl, fmt)

ChiPipeline* ChiPipeline::Create(ChiStream*     pStreams,
                                   int          numStreams,
                                   UINT32       cameraId,
                                   ChiModule*   pChiModule,
                                   PipelineType type)
{
    // Sanity check input params
    if ((NULL == pStreams) || (NULL == pChiModule) || (type >= PipelineTypeMax))
    {
        ISP_LOG(ERROR,
            "Invalid params: pStreams=%p, pChiModule=%p, type=%d",
            pStreams, pChiModule, type);
        return NULL;
    }

    ChiPipeline* pChiPipeline = new ChiPipeline(pChiModule);
    if (pChiPipeline != NULL)
    {
        if (pChiPipeline->Initialize(pStreams, numStreams, cameraId, type) != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR,
                "Failed to initialize ChiPipeline object.");
            delete pChiPipeline;
            return NULL;
        }
    }
    return pChiPipeline;
}

ChiPipeline::ChiPipeline(ChiModule* pChiModule) :
    m_pChiModule(pChiModule), m_createdPipeline(NULL), m_pAndroidDefaultMetaHandle(NULL)
{
    memset(&m_pipelineCreateData, 0, sizeof(m_pipelineCreateData));
    memset(&m_pipelineInputBuffer, 0, sizeof(m_pipelineInputBuffer));
    memset(m_pipelineOutputBuffer, 0, sizeof(m_pipelineOutputBuffer[0]) * m_MaxOutputBufs);
    memset(&m_pipelineInputBufferRequirements, 0, sizeof(m_pipelineInputBufferRequirements));
    memset(&m_pipelineInfo, 0, sizeof(m_pipelineInfo));
    memset(&m_metadataOps, 0, sizeof(m_metadataOps));
    memset(&m_chiCameraInfo, 0, sizeof(m_chiCameraInfo));
    memset(&m_legacyCameraInfo, 0, sizeof(m_legacyCameraInfo));
    memset(&m_pipelineMetadataInfo, 0, sizeof(m_pipelineMetadataInfo));
    for (unsigned int i = 0; i < m_MaxCameras; i++)
    {
        m_chiCameraInfo[i].pLegacy = &m_legacyCameraInfo;
    }
}

ChiPipeline::~ChiPipeline()
{
}

CameraResult ChiPipeline::Initialize(ChiStream*       pStreams,
                                        int           numStreams,
                                        UINT32        cameraId,
                                        PipelineType  type)
{
    CameraResult rc = CAMERA_SUCCESS;

    ISP_LOG(DBG, "numStreams %d, cameraId %d type %d",
        numStreams, cameraId, type);

    // Detect cameras and populate map
    rc = DetectCameras();
    if (rc != CAMERA_SUCCESS)
    {
        ISP_LOG(ERROR,"Failed to detect cameras");
        return rc;
    }

    // Get camx camera id for user specified id
    UINT32 camxId;
    std::map<UINT32,UINT32>::iterator it;
    it = m_cameraUserIdToCamxId.find(cameraId);
    if (it != m_cameraUserIdToCamxId.end())
    {
        camxId = it->second;
    }
    else
    {
        ISP_LOG(ERROR,"Cannot find camxId for user cameraId %d", cameraId);
        return CAMERA_ENOSUCH;
    }
    ISP_LOG(WARN, "cameraId %d corresponds to camxId %d",
        cameraId, camxId);

    if (OfflineBPS == type)
    {
        // Check if we have all streams to create pipeline
        if (numStreams != 2)
        {
            ISP_LOG(ERROR,
                "Invalid number = %d of chi streams, expected 2 stream",
                numStreams);
            return CAMERA_EBADPARM;
        }

        m_pipelineLinks[0].nodeId = BPS;
        m_pipelineLinks[0].nodeInstanceId = 0;
        m_pipelineLinks[0].nodePortId = BPSInputPort;

        m_pipelineLinks[1].nodeId = BPS;
        m_pipelineLinks[1].nodeInstanceId = 0;
        m_pipelineLinks[1].nodePortId = BPSOutputPortIdFull;

        // Configure pipeline input buffer
        m_pipelineInputBuffer.size = sizeof(m_pipelineInputBuffer);
        m_pipelineInputBuffer.numNodePorts = 1;
        m_pipelineInputBuffer.pNodePort = &m_pipelineLinks[0];
        m_pipelineInputBuffer.pStream = &(pStreams[0]);

        // Configure pipeline output buffer
        m_pipelineOutputBuffer[0].size = sizeof(m_pipelineOutputBuffer);
        m_pipelineOutputBuffer[0].numNodePorts = 1;
        m_pipelineOutputBuffer[0].pNodePort = &m_pipelineLinks[1];
        m_pipelineOutputBuffer[0].pStream = &(pStreams[1]);

        // Configure pipeline input options
        m_pipelineInputBufferRequirements.nodePort.nodeId = BPS;
        m_pipelineInputBufferRequirements.nodePort.nodeInstanceId = 0;
        m_pipelineInputBufferRequirements.nodePort.nodePortId = BPSInputPort;
        m_pipelineInputBufferRequirements.bufferOptions.size = sizeof(CHIBUFFEROPTIONS);
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.width = 0;
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.height = 0;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.width = 3840;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.height = 2160;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.width = pStreams[0].width;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.height = pStreams[0].height;

        // Configufre pipeline creation data
        m_pipelineCreateData.pPipelineName =
            IspUsecases[UsecaseOfflineBpsId].pPipelineTargetCreateDesc[OfflineBPSFull].pPipelineName;
        m_pipelineCreateData.pPipelineCreateDescriptor = (CHIPIPELINECREATEDESCRIPTOR*)malloc(sizeof(ChiPipelineCreateDescriptor));
        if (m_pipelineCreateData.pPipelineCreateDescriptor == NULL)
        {
            ISP_LOG(ERROR, "malloc ChiPipelineCreateDescriptor failed");
            return CAMERA_EFAILED;
        }

        *m_pipelineCreateData.pPipelineCreateDescriptor =
            IspUsecases[UsecaseOfflineBpsId].pPipelineTargetCreateDesc[OfflineBPSFull].pipelineCreateDesc;
        m_pipelineCreateData.pPipelineCreateDescriptor->cameraId = camxId;
        m_pipelineCreateData.numOutputs = 1;
        m_pipelineCreateData.pOutputDescriptors = m_pipelineOutputBuffer;
        m_pipelineCreateData.numInputs  = 1;
        m_pipelineCreateData.pInputDescriptors = &m_pipelineInputBuffer;
        m_pipelineCreateData.pInputBufferRequirements = &m_pipelineInputBufferRequirements;

        // Create default metadata
        rc = CreateMetadata();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreateMetadata() failed");
            return CAMERA_EFAILED;
        }

        // Send pipeline create request to camx
        rc = CreatePipeline();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreatePipeline() failed");
            return CAMERA_EFAILED;
        }
    } else if (OfflineSHDR == type)
    {
        // Check if we have all streams to create pipeline
        if (numStreams != 2)
        {
            ISP_LOG(ERROR,
                "Invalid number = %d of chi streams, expected 2 stream",
                numStreams);
            return CAMERA_EBADPARM;
        }

        m_pipelineLinks[0].nodeId = ChiExternalNode;
        m_pipelineLinks[0].nodeInstanceId = 0;
        m_pipelineLinks[0].nodePortId = SHDRInputPort;

        m_pipelineLinks[1].nodeId = ChiExternalNode;
        m_pipelineLinks[1].nodeInstanceId = 0;
        m_pipelineLinks[1].nodePortId = SHDROutputPortIdFull;

        // Configure pipeline input buffer
        m_pipelineInputBuffer.size = sizeof(m_pipelineInputBuffer);
        m_pipelineInputBuffer.numNodePorts = 1;
        m_pipelineInputBuffer.pNodePort = &m_pipelineLinks[0];
        m_pipelineInputBuffer.pStream = &(pStreams[0]);

        // Configure pipeline output buffer
        m_pipelineOutputBuffer[0].size = sizeof(m_pipelineOutputBuffer);
        m_pipelineOutputBuffer[0].numNodePorts = 1;
        m_pipelineOutputBuffer[0].pNodePort = &m_pipelineLinks[1];
        m_pipelineOutputBuffer[0].pStream = &(pStreams[1]);

        // Configure pipeline input options
        m_pipelineInputBufferRequirements.nodePort.nodeId = ChiExternalNode;
        m_pipelineInputBufferRequirements.nodePort.nodeInstanceId = 0;
        m_pipelineInputBufferRequirements.nodePort.nodePortId = SHDRInputPort;
        m_pipelineInputBufferRequirements.bufferOptions.size = sizeof(CHIBUFFEROPTIONS);
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.width = 0;
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.height = 0;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.width = 3840;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.height = 2160;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.width = pStreams[0].width;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.height = pStreams[0].height;

        // Configufre pipeline creation data
        m_pipelineCreateData.pPipelineName =
            IspUsecases[UsecaseOfflineSHDRId].pPipelineTargetCreateDesc[OfflineSHDRFull].pPipelineName;
        m_pipelineCreateData.pPipelineCreateDescriptor = (CHIPIPELINECREATEDESCRIPTOR*)malloc(sizeof(ChiPipelineCreateDescriptor));
        if (m_pipelineCreateData.pPipelineCreateDescriptor == NULL)
        {
            ISP_LOG(ERROR, "malloc ChiPipelineCreateDescriptor failed");
            return CAMERA_EFAILED;
        }

        *m_pipelineCreateData.pPipelineCreateDescriptor =
            IspUsecases[UsecaseOfflineSHDRId].pPipelineTargetCreateDesc[OfflineSHDRFull].pipelineCreateDesc;
        m_pipelineCreateData.pPipelineCreateDescriptor->cameraId = camxId;
        m_pipelineCreateData.numOutputs = 1;
        m_pipelineCreateData.pOutputDescriptors = m_pipelineOutputBuffer;
        m_pipelineCreateData.numInputs  = 1;
        m_pipelineCreateData.pInputDescriptors = &m_pipelineInputBuffer;
        m_pipelineCreateData.pInputBufferRequirements = &m_pipelineInputBufferRequirements;

        // Create default metadata
        rc = CreateMetadata();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreateMetadata() failed");
            return CAMERA_EFAILED;
        }
        ISP_LOG(DBG, "CreateMetadata() succeeded");

        // Send pipeline create request to camx
        rc = CreatePipeline();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreatePipeline() failed");
            return CAMERA_EFAILED;
        }
        ISP_LOG(DBG, "CreatePipeline() succeeded");
    }
    else if (OfflineBPSStats == type)
    {
        // Check if we have all streams to create pipeline
        if (numStreams != 4)
        {
            ISP_LOG(ERROR,
                "Invalid number = %d of chi streams, expected 2 stream",
                numStreams);
            return CAMERA_EBADPARM;
        }

        m_pipelineLinks[0].nodeId = BPS;
        m_pipelineLinks[0].nodeInstanceId = 0;
        m_pipelineLinks[0].nodePortId = BPSInputPort;

        m_pipelineLinks[1].nodeId = BPS;
        m_pipelineLinks[1].nodeInstanceId = 0;
        m_pipelineLinks[1].nodePortId = BPSOutputPortIdFull;

        m_pipelineLinks[2].nodeId = BPS;
        m_pipelineLinks[2].nodeInstanceId = 0;
        m_pipelineLinks[2].nodePortId = BPSOutputPortIdStatsBG;

        m_pipelineLinks[3].nodeId = BPS;
        m_pipelineLinks[3].nodeInstanceId = 0;
        m_pipelineLinks[3].nodePortId = BPSOutputPortIdStatsHDRBHIST;

        // Configure pipeline input buffer
        m_pipelineInputBuffer.size = sizeof(m_pipelineInputBuffer);
        m_pipelineInputBuffer.numNodePorts = 1;
        m_pipelineInputBuffer.pNodePort = &m_pipelineLinks[0];
        m_pipelineInputBuffer.pStream = &(pStreams[0]);

        // Configure pipeline output buffer
        m_pipelineOutputBuffer[0].size = sizeof(m_pipelineOutputBuffer);
        m_pipelineOutputBuffer[0].numNodePorts = 1;
        m_pipelineOutputBuffer[0].pNodePort = &m_pipelineLinks[1];
        m_pipelineOutputBuffer[0].pStream = &(pStreams[1]);

        m_pipelineOutputBuffer[1].size = sizeof(m_pipelineOutputBuffer);
        m_pipelineOutputBuffer[1].numNodePorts = 1;
        m_pipelineOutputBuffer[1].pNodePort = &m_pipelineLinks[2];
        m_pipelineOutputBuffer[1].pStream = &(pStreams[2]);

        m_pipelineOutputBuffer[2].size = sizeof(m_pipelineOutputBuffer);
        m_pipelineOutputBuffer[2].numNodePorts = 1;
        m_pipelineOutputBuffer[2].pNodePort = &m_pipelineLinks[3];
        m_pipelineOutputBuffer[2].pStream = &(pStreams[3]);

        // Configure pipeline input options
        //To do: confirm whether this is necessary
        m_pipelineInputBufferRequirements.nodePort.nodeId = BPS;
        m_pipelineInputBufferRequirements.nodePort.nodeInstanceId = 0;
        m_pipelineInputBufferRequirements.nodePort.nodePortId = BPSInputPort;

        m_pipelineInputBufferRequirements.bufferOptions.size = sizeof(CHIBUFFEROPTIONS);
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.width = 0;
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.height = 0;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.width = 3840;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.height = 2160;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.width = pStreams[0].width;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.height = pStreams[0].height;

        // Configufre pipeline creation data
        m_pipelineCreateData.pPipelineName =
            IspUsecases[UsecaseOfflineBPSStatsId].pPipelineTargetCreateDesc[BPSStats].pPipelineName;
        m_pipelineCreateData.pPipelineCreateDescriptor = (CHIPIPELINECREATEDESCRIPTOR*)malloc(sizeof(ChiPipelineCreateDescriptor));
        if (m_pipelineCreateData.pPipelineCreateDescriptor == NULL)
        {
            ISP_LOG(ERROR, "malloc ChiPipelineCreateDescriptor failed");
            return CAMERA_EFAILED;
        }

        *m_pipelineCreateData.pPipelineCreateDescriptor =
            IspUsecases[UsecaseOfflineBPSStatsId].pPipelineTargetCreateDesc[BPSStats].pipelineCreateDesc;
        m_pipelineCreateData.pPipelineCreateDescriptor->cameraId = camxId;
        m_pipelineCreateData.numOutputs = 3;
        m_pipelineCreateData.pOutputDescriptors = m_pipelineOutputBuffer;
        m_pipelineCreateData.numInputs  = 1;
        m_pipelineCreateData.pInputDescriptors = &m_pipelineInputBuffer;
        m_pipelineCreateData.pInputBufferRequirements = &m_pipelineInputBufferRequirements;

        // Create default metadata
        rc = CreateMetadata();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreateMetadata() failed");
            return CAMERA_EFAILED;
        }

        // Send pipeline create request to camx
        rc = CreatePipeline();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreatePipeline() failed");
            return CAMERA_EFAILED;
        }
    }
    else if ((OfflineShdrBps == type) ||
             (OfflineShdrBpsStatsProc == type) ||
             (OfflineShdrBpsAECAWB == type))
    {
        // Check if we have all streams to create pipeline
        if (numStreams != 2)
        {
            ISP_LOG(ERROR,
                "Invalid number = %d of chi streams, expected 2 stream",
                numStreams);
            return CAMERA_EBADPARM;
        }

        m_pipelineLinks[0].nodeId = ChiExternalNode;
        m_pipelineLinks[0].nodeInstanceId = 0;
        m_pipelineLinks[0].nodePortId = SHDRInputPort;

        m_pipelineLinks[1].nodeId = BPS;
        m_pipelineLinks[1].nodeInstanceId = 0;
        m_pipelineLinks[1].nodePortId = BPSOutputPortIdFull;

        // Configure pipeline input buffer
        m_pipelineInputBuffer.size = sizeof(m_pipelineInputBuffer);
        m_pipelineInputBuffer.numNodePorts = 1;
        m_pipelineInputBuffer.pNodePort = &m_pipelineLinks[0];
        m_pipelineInputBuffer.pStream = &(pStreams[0]);

        // Configure pipeline output buffer
        m_pipelineOutputBuffer[0].size = sizeof(m_pipelineOutputBuffer);
        m_pipelineOutputBuffer[0].numNodePorts = 1;
        m_pipelineOutputBuffer[0].pNodePort = &m_pipelineLinks[1];
        m_pipelineOutputBuffer[0].pStream = &(pStreams[1]);

        // Configure pipeline input options
        m_pipelineInputBufferRequirements.nodePort.nodeId = ChiExternalNode;
        m_pipelineInputBufferRequirements.nodePort.nodeInstanceId = 0;
        m_pipelineInputBufferRequirements.nodePort.nodePortId = SHDRInputPort;
        m_pipelineInputBufferRequirements.bufferOptions.size = sizeof(CHIBUFFEROPTIONS);
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.width = 0;
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.height = 0;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.width = 3840;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.height = 2160;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.width = pStreams[0].width;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.height = pStreams[0].height;

        // Configufre pipeline creation data
        UsecaseId usecaseId = UsecaseMaxId;
        int pipelineId = -1;
        if (OfflineShdrBps == type)
        {
            usecaseId = UsecaseOfflineShdrBpsId;
            pipelineId = ShdrBPSFull;
        }
        else if (OfflineShdrBpsStatsProc == type)
        {
            usecaseId = UsecaseOfflineShdrBpsStatsProcId;
            pipelineId = ShdrBPSFullStatsProc;
        }
        else if (OfflineShdrBpsAECAWB == type)
        {
            usecaseId = UsecaseOfflineShdrBpsAECAWB;
            pipelineId = SHDRBPSAECAWB;
        }
        m_pipelineCreateData.pPipelineName =
            IspUsecases[usecaseId].pPipelineTargetCreateDesc[pipelineId].pPipelineName;
        m_pipelineCreateData.pPipelineCreateDescriptor = (CHIPIPELINECREATEDESCRIPTOR*)malloc(sizeof(ChiPipelineCreateDescriptor));
        if (m_pipelineCreateData.pPipelineCreateDescriptor == NULL)
        {
            ISP_LOG(ERROR, "malloc ChiPipelineCreateDescriptor failed");
            return CAMERA_EFAILED;
        }

        *m_pipelineCreateData.pPipelineCreateDescriptor =
            IspUsecases[usecaseId].pPipelineTargetCreateDesc[pipelineId].pipelineCreateDesc;
        m_pipelineCreateData.pPipelineCreateDescriptor->cameraId = camxId;
        m_pipelineCreateData.numOutputs = 1;
        m_pipelineCreateData.pOutputDescriptors = m_pipelineOutputBuffer;
        m_pipelineCreateData.numInputs  = 1;
        m_pipelineCreateData.pInputDescriptors = &m_pipelineInputBuffer;
        m_pipelineCreateData.pInputBufferRequirements = &m_pipelineInputBufferRequirements;

        // Create default metadata
        rc = CreateMetadata();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreateMetadata() failed");
            return CAMERA_EFAILED;
        }

        // Send pipeline create request to camx
        rc = CreatePipeline();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreatePipeline() failed");
            return CAMERA_EFAILED;
        }
    }
    else if (OfflineBPSAWBBGStats == type ||
             OfflineBPSBGBHISTAWB == type ||
             OfflineBPSAECAWB == type)
    {
        if (numStreams != 2)
        {
            ISP_LOG(ERROR,
                "Invalid number = %d of chi streams, expected 2 stream",
                numStreams);
            return CAMERA_EBADPARM;
        }
        m_pipelineLinks[0].nodeId = BPS;
        m_pipelineLinks[0].nodeInstanceId = 0;
        m_pipelineLinks[0].nodePortId = BPSInputPort;

        m_pipelineLinks[1].nodeId = BPS;
        m_pipelineLinks[1].nodeInstanceId = 0;
        m_pipelineLinks[1].nodePortId = BPSOutputPortIdFull;

        // Configure pipeline input buffer
        m_pipelineInputBuffer.size = sizeof(m_pipelineInputBuffer);
        m_pipelineInputBuffer.numNodePorts = 1;
        m_pipelineInputBuffer.pNodePort = &m_pipelineLinks[0];
        m_pipelineInputBuffer.pStream = &(pStreams[0]);

        // Configure pipeline output buffer
        m_pipelineOutputBuffer[0].size = sizeof(m_pipelineOutputBuffer);
        m_pipelineOutputBuffer[0].numNodePorts = 1;
        m_pipelineOutputBuffer[0].pNodePort = &m_pipelineLinks[1];
        m_pipelineOutputBuffer[0].pStream = &(pStreams[1]);

        // Configure pipeline input options
        //To do: confirm whether this is necessary
        m_pipelineInputBufferRequirements.nodePort.nodeId = BPS;
        m_pipelineInputBufferRequirements.nodePort.nodeInstanceId = 0;
        m_pipelineInputBufferRequirements.nodePort.nodePortId = BPSInputPort;

        m_pipelineInputBufferRequirements.bufferOptions.size = sizeof(CHIBUFFEROPTIONS);
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.width = 0;
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.height = 0;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.width = 3840;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.height = 2160;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.width = pStreams[0].width;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.height = pStreams[0].height;

        // Configufre pipeline creation data
        UsecaseId usecaseId = UsecaseMaxId;
        int pipelineId = -1;
        if (OfflineBPSAWBBGStats == type)
        {
            usecaseId = UsecaseOfflineBPSBGStatsParseId;
            pipelineId = BPSBGStatsParse;
        }
        else if (OfflineBPSBGBHISTAWB == type)
        {
            usecaseId = UsecaseOfflineBPSBGBHistId;
            pipelineId = BPSBGBHIST;
        }
        else if (OfflineBPSAECAWB == type)
        {
            usecaseId = UsecaseOfflineBpsAECAWB;
            pipelineId = BPSAECAWB;
        }

        m_pipelineCreateData.pPipelineName =
            IspUsecases[usecaseId].pPipelineTargetCreateDesc[pipelineId].pPipelineName;
        m_pipelineCreateData.pPipelineCreateDescriptor = (CHIPIPELINECREATEDESCRIPTOR*)malloc(sizeof(ChiPipelineCreateDescriptor));
        if (m_pipelineCreateData.pPipelineCreateDescriptor == NULL)
        {
            ISP_LOG(ERROR, "malloc ChiPipelineCreateDescriptor failed");
            return CAMERA_EFAILED;
        }

        *m_pipelineCreateData.pPipelineCreateDescriptor =
            IspUsecases[usecaseId].pPipelineTargetCreateDesc[pipelineId].pipelineCreateDesc;
        m_pipelineCreateData.pPipelineCreateDescriptor->cameraId = camxId;
        m_pipelineCreateData.numOutputs = 1;
        m_pipelineCreateData.pOutputDescriptors = m_pipelineOutputBuffer;
        m_pipelineCreateData.numInputs  = 1;
        m_pipelineCreateData.pInputDescriptors = &m_pipelineInputBuffer;
        m_pipelineCreateData.pInputBufferRequirements = &m_pipelineInputBufferRequirements;

        // Create default metadata
        rc = CreateMetadata();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreateMetadata() failed");
            return CAMERA_EFAILED;
        }

        // Send pipeline create request to camx
        rc = CreatePipeline();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreatePipeline() failed");
            return CAMERA_EFAILED;
        }
    }
    else if(OfflineIPE == type)
    {
        // Check if we have all streams to create pipeline
        if (numStreams != 2)
        {
            ISP_LOG(ERROR,
                    "Invalid number = %d of chi streams , expected 2 stream",
                    numStreams);
            return CAMERA_EBADPARM;
        }

        m_pipelineLinks[0].nodeId = IPE;
        m_pipelineLinks[0].nodeInstanceId = 0;
        m_pipelineLinks[0].nodePortId = IPEInputPortFull;

        m_pipelineLinks[1].nodeId = IPE;
        m_pipelineLinks[1].nodeInstanceId = 0;
        m_pipelineLinks[1].nodePortId = IPEOutputPortDisplay;

        // Configure pipeline input buffer
        m_pipelineInputBuffer.size = sizeof(m_pipelineInputBuffer);
        m_pipelineInputBuffer.numNodePorts = 1;
        m_pipelineInputBuffer.pNodePort = &m_pipelineLinks[0];
        m_pipelineInputBuffer.pStream = &(pStreams[0]);

        // Configure pipeline output buffer
        m_pipelineOutputBuffer[0].size = sizeof(m_pipelineOutputBuffer);
        m_pipelineOutputBuffer[0].numNodePorts = 1;
        m_pipelineOutputBuffer[0].pNodePort = &m_pipelineLinks[1];
        m_pipelineOutputBuffer[0].pStream = &(pStreams[1]);

        // Configure pipeline input options
        m_pipelineInputBufferRequirements.nodePort.nodeId = IPE;
        m_pipelineInputBufferRequirements.nodePort.nodeInstanceId = 0;
        m_pipelineInputBufferRequirements.nodePort.nodePortId = IPEInputPortFull;
        m_pipelineInputBufferRequirements.bufferOptions.size = sizeof(CHIBUFFEROPTIONS);
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.width = 0;
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.height = 0;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.width = 3840;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.height = 2160;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.width = pStreams[0].width;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.height = pStreams[0].height;

        // Configufre pipeline creation data
        m_pipelineCreateData.pPipelineName =
        IspUsecases[UsecaseOfflineIpeId].pPipelineTargetCreateDesc[IpeDisp].pPipelineName;
        m_pipelineCreateData.pPipelineCreateDescriptor = (CHIPIPELINECREATEDESCRIPTOR*)malloc(sizeof(ChiPipelineCreateDescriptor));
        if (m_pipelineCreateData.pPipelineCreateDescriptor == NULL)
        {
            ISP_LOG(ERROR, "malloc ChiPipelineCreateDescriptor failed");
            return CAMERA_EFAILED;
        }

        *m_pipelineCreateData.pPipelineCreateDescriptor =
            IspUsecases[UsecaseOfflineIpeId].pPipelineTargetCreateDesc[IpeDisp].pipelineCreateDesc;
        m_pipelineCreateData.pPipelineCreateDescriptor->cameraId = camxId;
        m_pipelineCreateData.numOutputs = 1;
        m_pipelineCreateData.pOutputDescriptors = m_pipelineOutputBuffer;
        m_pipelineCreateData.numInputs  = 1;
        m_pipelineCreateData.pInputDescriptors = &m_pipelineInputBuffer;
        m_pipelineCreateData.pInputBufferRequirements = &m_pipelineInputBufferRequirements;

        // Create default metadata
        rc = CreateMetadata();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreateMetadata() failed");
            return CAMERA_EFAILED;
        }

        // Send pipeline create request to camx
        rc = CreatePipeline();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreatePipeline() failed");
            return CAMERA_EFAILED;
        }
    }
    else if (OfflineBpsIpe == type)
    {

        // Check if we have all streams to create pipeline
        if (numStreams != 2)
        {
            ISP_LOG(ERROR,
                "Invalid number = %d of chi streams , expected 2 stream",
                numStreams);
            return CAMERA_EBADPARM;
        }

        m_pipelineLinks[0].nodeId = BPS;
        m_pipelineLinks[0].nodeInstanceId = 0;
        m_pipelineLinks[0].nodePortId = BPSInputPort;

        m_pipelineLinks[1].nodeId = IPE;
        m_pipelineLinks[1].nodeInstanceId = 0;
        m_pipelineLinks[1].nodePortId = IPEOutputPortDisplay;

        // Configure pipeline input buffer
        m_pipelineInputBuffer.size = sizeof(m_pipelineInputBuffer);
        m_pipelineInputBuffer.numNodePorts = 1;
        m_pipelineInputBuffer.pNodePort = &m_pipelineLinks[0];
        m_pipelineInputBuffer.pStream = &(pStreams[0]);

        // Configure pipeline output buffer
        m_pipelineOutputBuffer[0].size = sizeof(m_pipelineOutputBuffer);
        m_pipelineOutputBuffer[0].numNodePorts = 1;
        m_pipelineOutputBuffer[0].pNodePort = &m_pipelineLinks[1];
        m_pipelineOutputBuffer[0].pStream = &(pStreams[1]);

        // Configure pipeline input options
        m_pipelineInputBufferRequirements.nodePort.nodeId = BPS;
        m_pipelineInputBufferRequirements.nodePort.nodeInstanceId = 0;
        m_pipelineInputBufferRequirements.nodePort.nodePortId = BPSInputPort;
        m_pipelineInputBufferRequirements.bufferOptions.size = sizeof(CHIBUFFEROPTIONS);
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.width = 0;
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.height = 0;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.width = 3840;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.height = 2160;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.width = pStreams[0].width;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.height = pStreams[0].height;

        // Configufre pipeline creation data
        m_pipelineCreateData.pPipelineName =
            IspUsecases[UsecaseOfflineBpsIpe].pPipelineTargetCreateDesc[OfflineBpsIpeDisp].pPipelineName;
        m_pipelineCreateData.pPipelineCreateDescriptor = (CHIPIPELINECREATEDESCRIPTOR*)malloc(sizeof(ChiPipelineCreateDescriptor));
        if (m_pipelineCreateData.pPipelineCreateDescriptor == NULL)
        {
            ISP_LOG(ERROR, "malloc ChiPipelineCreateDescriptor failed");
            return CAMERA_EFAILED;
        }

        *m_pipelineCreateData.pPipelineCreateDescriptor =
            IspUsecases[UsecaseOfflineBpsIpe].pPipelineTargetCreateDesc[OfflineBpsIpeDisp].pipelineCreateDesc;
        m_pipelineCreateData.pPipelineCreateDescriptor->cameraId = camxId;
        m_pipelineCreateData.numOutputs = 1;
        m_pipelineCreateData.pOutputDescriptors = m_pipelineOutputBuffer;
        m_pipelineCreateData.numInputs  = 1;
        m_pipelineCreateData.pInputDescriptors = &m_pipelineInputBuffer;
        m_pipelineCreateData.pInputBufferRequirements = &m_pipelineInputBufferRequirements;

        // Create default metadata
        rc = CreateMetadata();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreateMetadata() failed");
            return CAMERA_EFAILED;
        }

        // Send pipeline create request to camx
        rc = CreatePipeline();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreatePipeline() failed");
            return CAMERA_EFAILED;
        }

    }
    else if (OfflineBpsIpeAECAWB== type)
    {
        // Check if we have all streams to create pipeline
        if (numStreams != 2)
        {
            ISP_LOG(ERROR,
                "Invalid number = %d of chi streams , expected 2 stream",
                numStreams);
            return CAMERA_EBADPARM;
        }

        m_pipelineLinks[0].nodeId = BPS;
        m_pipelineLinks[0].nodeInstanceId = 0;
        m_pipelineLinks[0].nodePortId = BPSInputPort;

        m_pipelineLinks[1].nodeId = IPE;
        m_pipelineLinks[1].nodeInstanceId = 0;
        m_pipelineLinks[1].nodePortId = IPEOutputPortDisplay;

        // Configure pipeline input buffer
        m_pipelineInputBuffer.size = sizeof(m_pipelineInputBuffer);
        m_pipelineInputBuffer.numNodePorts = 1;
        m_pipelineInputBuffer.pNodePort = &m_pipelineLinks[0];
        m_pipelineInputBuffer.pStream = &(pStreams[0]);

        // Configure pipeline output buffer
        m_pipelineOutputBuffer[0].size = sizeof(m_pipelineOutputBuffer);
        m_pipelineOutputBuffer[0].numNodePorts = 1;
        m_pipelineOutputBuffer[0].pNodePort = &m_pipelineLinks[1];
        m_pipelineOutputBuffer[0].pStream = &(pStreams[1]);

        // Configure pipeline input options
        m_pipelineInputBufferRequirements.nodePort.nodeId = BPS;
        m_pipelineInputBufferRequirements.nodePort.nodeInstanceId = 0;
        m_pipelineInputBufferRequirements.nodePort.nodePortId = BPSInputPort;
        m_pipelineInputBufferRequirements.bufferOptions.size = sizeof(CHIBUFFEROPTIONS);
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.width = 0;
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.height = 0;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.width = 3840;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.height = 2160;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.width = pStreams[0].width;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.height = pStreams[0].height;

        // Configufre pipeline creation data
        m_pipelineCreateData.pPipelineName =
            IspUsecases[UsecaseOfflineBpsIpeAECAWB].pPipelineTargetCreateDesc[OfflineBPSAECAWBIPEPipelineId].pPipelineName;
        m_pipelineCreateData.pPipelineCreateDescriptor = (CHIPIPELINECREATEDESCRIPTOR*)malloc(sizeof(ChiPipelineCreateDescriptor));
        if (m_pipelineCreateData.pPipelineCreateDescriptor == NULL)
        {
            ISP_LOG(ERROR, "malloc ChiPipelineCreateDescriptor failed");
            return CAMERA_EFAILED;
        }

        *m_pipelineCreateData.pPipelineCreateDescriptor =
            IspUsecases[UsecaseOfflineBpsIpeAECAWB].pPipelineTargetCreateDesc[OfflineBPSAECAWBIPEPipelineId].pipelineCreateDesc;
        m_pipelineCreateData.pPipelineCreateDescriptor->cameraId = camxId;
        m_pipelineCreateData.numOutputs = 1;
        m_pipelineCreateData.pOutputDescriptors = m_pipelineOutputBuffer;
        m_pipelineCreateData.numInputs  = 1;
        m_pipelineCreateData.pInputDescriptors = &m_pipelineInputBuffer;
        m_pipelineCreateData.pInputBufferRequirements = &m_pipelineInputBufferRequirements;

        // Create default metadata
        rc = CreateMetadata();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreateMetadata() failed");
            return CAMERA_EFAILED;
        }

        // Send pipeline create request to camx
        rc = CreatePipeline();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreatePipeline() failed");
            return CAMERA_EFAILED;
        }
    }
#ifdef AIS_ISP_ENABLE_JPEG
    else if (OfflineShdrBpsIpeAECAWB == type)
    {
        // Check if we have all streams to create pipeline
        if (numStreams != 3)
        {
            ISP_LOG(ERROR,
                "Invalid number = %d of chi streams , expected 2 stream",
                numStreams);
            return CAMERA_EBADPARM;
        }

        m_pipelineLinks[0].nodeId = ChiExternalNode;
        m_pipelineLinks[0].nodeInstanceId = 0;
        m_pipelineLinks[0].nodePortId = SHDRInputPort;

        m_pipelineLinks[1].nodeId = IPE;
        m_pipelineLinks[1].nodeInstanceId = 0;
        m_pipelineLinks[1].nodePortId = IPEOutputPortDisplay;

        m_pipelineLinks[2].nodeId = 6;
        m_pipelineLinks[2].nodeInstanceId = 0;
        m_pipelineLinks[2].nodePortId = 2;

        // Configure pipeline input buffer
        m_pipelineInputBuffer.size = sizeof(m_pipelineInputBuffer);
        m_pipelineInputBuffer.numNodePorts = 1;
        m_pipelineInputBuffer.pNodePort = &m_pipelineLinks[0];
        m_pipelineInputBuffer.pStream = &(pStreams[0]);

        // Configure pipeline output buffer
        m_pipelineOutputBuffer[0].size = sizeof(m_pipelineOutputBuffer);
        m_pipelineOutputBuffer[0].numNodePorts = 1;
        m_pipelineOutputBuffer[0].pNodePort = &m_pipelineLinks[1];
        m_pipelineOutputBuffer[0].pStream = &(pStreams[1]);

        m_pipelineOutputBuffer[1].size = sizeof(m_pipelineOutputBuffer);
        m_pipelineOutputBuffer[1].numNodePorts = 1;
        m_pipelineOutputBuffer[1].pNodePort = &m_pipelineLinks[2];
        m_pipelineOutputBuffer[1].pStream = &(pStreams[2]);

        // Configure pipeline input options
        m_pipelineInputBufferRequirements.nodePort.nodeId = ChiExternalNode;
        m_pipelineInputBufferRequirements.nodePort.nodeInstanceId = 0;
        m_pipelineInputBufferRequirements.nodePort.nodePortId = SHDRInputPort;
        m_pipelineInputBufferRequirements.bufferOptions.size = sizeof(CHIBUFFEROPTIONS);
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.width = 0;
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.height = 0;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.width = 3840;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.height = 2160;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.width = pStreams[0].width;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.height = pStreams[0].height;

        // Configufre pipeline creation data
        m_pipelineCreateData.pPipelineName =
            IspUsecases[UsecaseOfflineShdrBpsIpeAECAWB].pPipelineTargetCreateDesc[OfflineShdrBpsAECAWBIPEJPEG].pPipelineName;
        m_pipelineCreateData.pPipelineCreateDescriptor = (CHIPIPELINECREATEDESCRIPTOR*)malloc(sizeof(ChiPipelineCreateDescriptor));
        if (m_pipelineCreateData.pPipelineCreateDescriptor == NULL)
        {
            ISP_LOG(ERROR, "malloc ChiPipelineCreateDescriptor failed");
            return CAMERA_EFAILED;
        }

        *m_pipelineCreateData.pPipelineCreateDescriptor =

            IspUsecases[UsecaseOfflineShdrBpsIpeAECAWB].pPipelineTargetCreateDesc[OfflineShdrBpsAECAWBIPEJPEG].pipelineCreateDesc;
        m_pipelineCreateData.pPipelineCreateDescriptor->cameraId = camxId;
        m_pipelineCreateData.numOutputs = 2;
        m_pipelineCreateData.pOutputDescriptors = m_pipelineOutputBuffer;
        m_pipelineCreateData.numInputs  = 1;
        m_pipelineCreateData.pInputDescriptors = &m_pipelineInputBuffer;
        m_pipelineCreateData.pInputBufferRequirements = &m_pipelineInputBufferRequirements;
        // Create default metadata
        rc = CreateMetadata();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreateMetadata() failed");
            return CAMERA_EFAILED;
        }
        if (m_pAndroidDefaultMetaHandle) {
            UINT64 Temp [2]{0xff,0x10};
            UINT8 res = m_metadataOps.pSetVendorTag(m_pAndroidDefaultMetaHandle,"org.quic.camera.tuningdata","TuningDataDump",Temp ,sizeof(Temp));
        }

        // Send pipeline create request to camx
        rc = CreatePipeline();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreatePipeline() failed");
            return CAMERA_EFAILED;
        }
    }
#else
     else if (OfflineShdrBpsIpeAECAWB == type)
    {
        // Check if we have all streams to create pipeline
        if (numStreams != 2)
        {
            ISP_LOG(ERROR,
                "Invalid number = %d of chi streams , expected 2 stream",
                numStreams);
            return CAMERA_EBADPARM;
        }

        m_pipelineLinks[0].nodeId = ChiExternalNode;
        m_pipelineLinks[0].nodeInstanceId = 0;
        m_pipelineLinks[0].nodePortId = SHDRInputPort;

        m_pipelineLinks[1].nodeId = IPE;
        m_pipelineLinks[1].nodeInstanceId = 0;
        m_pipelineLinks[1].nodePortId = IPEOutputPortDisplay;

        // Configure pipeline input buffer
        m_pipelineInputBuffer.size = sizeof(m_pipelineInputBuffer);
        m_pipelineInputBuffer.numNodePorts = 1;
        m_pipelineInputBuffer.pNodePort = &m_pipelineLinks[0];
        m_pipelineInputBuffer.pStream = &(pStreams[0]);

        // Configure pipeline output buffer
        m_pipelineOutputBuffer[0].size = sizeof(m_pipelineOutputBuffer);
        m_pipelineOutputBuffer[0].numNodePorts = 1;
        m_pipelineOutputBuffer[0].pNodePort = &m_pipelineLinks[1];
        m_pipelineOutputBuffer[0].pStream = &(pStreams[1]);

        // Configure pipeline input options
        m_pipelineInputBufferRequirements.nodePort.nodeId = ChiExternalNode;
        m_pipelineInputBufferRequirements.nodePort.nodeInstanceId = 0;
        m_pipelineInputBufferRequirements.nodePort.nodePortId = SHDRInputPort;
        m_pipelineInputBufferRequirements.bufferOptions.size = sizeof(CHIBUFFEROPTIONS);
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.width = 0;
        m_pipelineInputBufferRequirements.bufferOptions.minDimension.height = 0;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.width = 3840;
        m_pipelineInputBufferRequirements.bufferOptions.maxDimension.height = 2160;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.width = pStreams[0].width;
        m_pipelineInputBufferRequirements.bufferOptions.optimalDimension.height = pStreams[0].height;

        // Configufre pipeline creation data
        m_pipelineCreateData.pPipelineName =
            IspUsecases[UsecaseOfflineShdrBpsIpeAECAWB].pPipelineTargetCreateDesc[OfflineShdrBpsAECAWBIPE].pPipelineName;
        m_pipelineCreateData.pPipelineCreateDescriptor = (CHIPIPELINECREATEDESCRIPTOR*)malloc(sizeof(ChiPipelineCreateDescriptor));
        if (m_pipelineCreateData.pPipelineCreateDescriptor == NULL)
        {
            ISP_LOG(ERROR, "malloc ChiPipelineCreateDescriptor failed");
            return CAMERA_EFAILED;
        }

        *m_pipelineCreateData.pPipelineCreateDescriptor =

            IspUsecases[UsecaseOfflineShdrBpsIpeAECAWB].pPipelineTargetCreateDesc[OfflineShdrBpsAECAWBIPE].pipelineCreateDesc;
        m_pipelineCreateData.pPipelineCreateDescriptor->cameraId = camxId;
        m_pipelineCreateData.numOutputs = 1;
        m_pipelineCreateData.pOutputDescriptors = m_pipelineOutputBuffer;
        m_pipelineCreateData.numInputs  = 1;
        m_pipelineCreateData.pInputDescriptors = &m_pipelineInputBuffer;
        m_pipelineCreateData.pInputBufferRequirements = &m_pipelineInputBufferRequirements;
        // Create default metadata
        rc = CreateMetadata();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreateMetadata() failed");
            return CAMERA_EFAILED;
        }

        // Send pipeline create request to camx
        rc = CreatePipeline();
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "CreatePipeline() failed");
            return CAMERA_EFAILED;
        }
    }
#endif
    return rc;
}

CameraResult ChiPipeline::DetectCameras()
{
    CameraResult rc = CAMERA_SUCCESS;

    m_numDetectedCameras = m_pChiModule->GetChiOps()->pGetNumCameras(m_pChiModule->GetContext());
    if (0 == m_numDetectedCameras)
    {
        ISP_LOG(ERROR, "no cameras detected");
        rc = CAMERA_ERESOURCENOTFOUND;
        goto exit;
    }
    if (m_numDetectedCameras > m_MaxCameras)
    {
        ISP_LOG(ERROR, "Not sufficient memory to store info for %d detected cameras");
        rc = CAMERA_ENOMEMORY;
        goto exit;
    }

    for (UINT32 i = 0 ; i < m_numDetectedCameras; i++)
    {
        if (m_pChiModule->GetChiOps()->pGetCameraInfo(
                    m_pChiModule->GetContext(),
                    i,
                    &m_chiCameraInfo[i]) != CDKResultSuccess)
        {
            ISP_LOG(ERROR, "[%d] Failed to get camera info", i);
            rc = CAMERA_EFAILED;
            goto exit;
        }

        m_cameraUserIdToCamxId.insert(
            std::pair<UINT32,UINT32>(m_chiCameraInfo[i].sensorCaps.sensorId, i));
        ISP_LOG(WARN,"sensorId %d mapped to camxCameraId %d",
            m_chiCameraInfo[i].sensorCaps.sensorId, i);

        ISP_LOG(WARN,"m_chiCameraInfo[%d]: numSensorModes %d",i, m_chiCameraInfo[i].numSensorModes);
        ISP_LOG(WARN,"sensorCaps[%d]: sensorId %d positionType %d "
            "activeArray.left %d activeArray.top %d activeArray.width %d activeArray.height %d",
            i, m_chiCameraInfo[i].sensorCaps.sensorId, m_chiCameraInfo[i].sensorCaps.positionType,
            m_chiCameraInfo[i].sensorCaps.activeArray.left, m_chiCameraInfo[i].sensorCaps.activeArray.top,
            m_chiCameraInfo[i].sensorCaps.activeArray.width, m_chiCameraInfo[i].sensorCaps.activeArray.height);
    }

exit:
    if (rc != CAMERA_SUCCESS)
    {
        m_cameraUserIdToCamxId.clear();
    }

    return rc;
}

CameraResult ChiPipeline::CreateMetadata()
{
    CameraResult rc = CAMERA_SUCCESS;

    // Update metadata ops
    //
    m_pChiModule->GetChiOps()->pMetadataOps(&m_metadataOps);

    // Populate default Android metadata structure
    //
    CHIMETAHANDLE  h_AndroidDefaultMetaHandle;
    if (m_metadataOps.pGetDefaultAndroidMeta(0, (const VOID**)&h_AndroidDefaultMetaHandle) != CDKResultSuccess)
    {
        ISP_LOG(ERROR, "Failed to get default Android metadata");
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG,
            "Succeeded to get Android default metadata handl=%p", h_AndroidDefaultMetaHandle);

    if (m_metadataOps.pCreateWithAndroidMetadata(h_AndroidDefaultMetaHandle, &m_pAndroidDefaultMetaHandle, NULL))
    {
        ISP_LOG(ERROR, "Failed to create Android metadata");
        return CAMERA_EFAILED;
    }
    else
        ISP_LOG(DBG, "Successfully created Android metadata");

    ISP_LOG(ERROR,
            "Succeeded to get Android default metadata handl=%p", h_AndroidDefaultMetaHandle);

    if (m_metadataOps.pSetAndroidMetadata(m_pAndroidDefaultMetaHandle, h_AndroidDefaultMetaHandle))
    {
        ISP_LOG(ERROR, "Failed to set default Android android metadata");
        return CAMERA_EFAILED;
    }
    else
        ISP_LOG(DBG, "Successfully set default Android metadata");

    ChiTuningModeParameter tagValue;
    memset(&tagValue, 0, sizeof(tagValue));
    if (m_metadataOps.pSetVendorTag(
                m_pAndroidDefaultMetaHandle,
                "org.quic.camera2.tuning.mode",
                "TuningMode",
                &tagValue,
                sizeof(ChiTuningModeParameter)) != CDKResultSuccess)
    {
        ISP_LOG(ERROR, "Failed to set vendor tag");
        return CAMERA_EFAILED;
    }

    m_pipelineCreateData.pPipelineCreateDescriptor->hPipelineMetadata = m_pAndroidDefaultMetaHandle;
    return rc;
}


CameraResult ChiPipeline::CreatePipeline()
{
    CameraResult rc = CAMERA_SUCCESS;

    m_createdPipeline = m_pChiModule->GetChiOps()->pCreatePipelineDescriptor(
        m_pChiModule->GetContext(),
        m_pipelineCreateData.pPipelineName,
        m_pipelineCreateData.pPipelineCreateDescriptor,
        m_pipelineCreateData.numOutputs,
        m_pipelineCreateData.pOutputDescriptors,
        m_pipelineCreateData.numInputs,
        m_pipelineCreateData.pInputBufferRequirements);
    if (NULL == m_createdPipeline)
    {
        ISP_LOG(ERROR,
            "Failed to create pipeline descriptor");
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG,
        "Successfully created pipeline descriptor=%p", m_createdPipeline);

    // Populate pipeline info structure
    m_pipelineInfo.hPipelineDescriptor = m_createdPipeline;
    m_pipelineInfo.pipelineInputInfo.isInputSensor = false;
    m_pipelineInfo.pipelineInputInfo.inputBufferInfo.numInputBuffers = m_pipelineCreateData.numInputs;
    m_pipelineInfo.pipelineInputInfo.inputBufferInfo.pInputBufferDescriptors = m_pipelineCreateData.pInputDescriptors;
    m_pipelineInfo.pipelineOutputInfo.hPipelineHandle = reinterpret_cast<CHIPIPELINEHANDLE>(m_createdPipeline);

    return rc;
}

void ChiPipeline::DestroyPipeline()
{
    if (m_pAndroidDefaultMetaHandle)
    {
        m_metadataOps.pDestroy(m_pAndroidDefaultMetaHandle, true);
        ISP_LOG(DBG,
            "Successfully destroyed Android metadata handle=%p", m_pAndroidDefaultMetaHandle);
        m_pAndroidDefaultMetaHandle = NULL;
    }

    if (m_createdPipeline != NULL)
    {
        m_pChiModule->GetChiOps()->pDestroyPipelineDescriptor(
            m_pChiModule->GetContext(), m_createdPipeline);
        ISP_LOG(DBG,
            "Successfully destroyed pipeline descriptor=%p", m_createdPipeline);
        m_createdPipeline = NULL;
    }

    if (m_pipelineCreateData.pPipelineCreateDescriptor)
    {
        free(m_pipelineCreateData.pPipelineCreateDescriptor);
        m_pipelineCreateData.pPipelineCreateDescriptor = NULL;
    }

    delete this;
}

CameraResult ChiPipeline::GetPipelineInfo(CHIPIPELINEINFO* pPipelineInfo)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (NULL == pPipelineInfo)
    {
        ISP_LOG(ERROR, "pPipelineInfo is NULL");
        return CAMERA_EBADPARM;
    }

    *pPipelineInfo = m_pipelineInfo;

    return rc;
}

CameraResult ChiPipeline::ActivatePipeline(ChiSession* pChiSession)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (m_pChiModule->GetChiOps()->pActivatePipeline(
            m_pChiModule->GetContext(),
            pChiSession->GetSessionHandle(),
            m_createdPipeline,
            NULL) != CDKResultSuccess)
    {
        ISP_LOG(ERROR,
            "Failed to activate pipeline for hSession=%p.",
            pChiSession->GetSessionHandle());
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG,
        "Successfully activated pipeline for hSession=%p.",
        pChiSession->GetSessionHandle());

    return rc;
}

CameraResult ChiPipeline::DeactivatePipeline(ChiSession* pChiSession)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (m_pChiModule->GetChiOps()->pDeactivatePipeline(
            m_pChiModule->GetContext(),
            pChiSession->GetSessionHandle(),
            m_createdPipeline,
            CHIDeactivateModeDefault) != CDKResultSuccess)
    {
        ISP_LOG(ERROR,
            "Failed to deactivate pipeline for hSession=%p.",
            pChiSession->GetSessionHandle());
        return CAMERA_EFAILED;
    }

    ISP_LOG(DBG,
        "Successfully deactivated pipeline for hSession=%p.",
        pChiSession->GetSessionHandle());
    return rc;
}

void ChiPipeline::QueryPipelineMetadataInfo(ChiSession* pChiSession)
{
    if (m_pChiModule->GetChiOps()->pQueryPipelineMetadataInfo(
            m_pChiModule->GetContext(),
            pChiSession->GetSessionHandle(),
            m_createdPipeline,
            &m_pipelineMetadataInfo) != CDKResultSuccess)
    {
        ISP_LOG(ERROR,
            "Failed to query metadata from pipeline for hSession=%p.",
            pChiSession->GetSessionHandle());
    }
    else
    {
        ISP_LOG(DBG,
            "metadata info of pipeline for session=%p",
            pChiSession->GetSessionHandle());
        ISP_LOG(DBG,
            "maxNumMetaBuffers=%u", m_pipelineMetadataInfo.maxNumMetaBuffers);
        ISP_LOG(DBG,
            "publishTagCount=%u", m_pipelineMetadataInfo.publishTagCount);
        ISP_LOG(DBG,
            "publishPartialTagCount=%u", m_pipelineMetadataInfo.publishPartialTagCount);
        for (UINT32 i=0;i<m_pipelineMetadataInfo.publishTagCount;i++)
            ISP_LOG(DBG,
            "publishTagArray[%u]=%x",i, m_pipelineMetadataInfo.publishTagArray[i]);
    }
}

bool ChiPipeline::PipelineHasPopulatedTag(UINT32 tagId)
{
    for (UINT32 i=0;i<m_pipelineMetadataInfo.publishTagCount;i++)
        if (tagId == m_pipelineMetadataInfo.publishTagArray[i])
            return true;
    return false;
}

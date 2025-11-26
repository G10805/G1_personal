//*************************************************************************************************
// Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//*************************************************************************************************

#ifndef __CHIPIPELINES_DEF_H__
#define __CHIPIPELINES_DEF_H__

#include "chi.h"
#include "g_pipelines.h"

#define SHDR_AEC_ENABLE_PROPERTY NodePropertyVendorStart + 1 // Property to wait for AEC dependence

#define StatsInputPortBPSAWBBG 15
#define StatsParseInputPortBPSAWBBG 15

#define BufferQueueDepth 12 //max buffers that will ever exist on link


#ifndef USECASEDEFS_H
/// @brief ranges of buffer sizes supported
struct BufferDimension
{
    UINT minWidth;
    UINT minHeight;
    UINT maxWidth;
    UINT maxHeight;
};

/// @brief Collection of information describing how a buffer is used
struct ChiTarget
{
    ChiStreamType       direction;
    BufferDimension     dimension;
    UINT                numFormats;
    ChiBufferFormat*    pBufferFormats;
    ChiStream*          pChiStream;
};

/// @brief Information regarding where a port interacts with a buffer directly
struct ChiTargetPortDescriptor
{
    const CHAR*            pTargetName;
    ChiTarget*             pTarget;
    UINT                   numNodePorts;
    ChiLinkNodeDescriptor* pNodePort;
};

/// @brief List of port->buffer information
struct ChiTargetPortDescriptorInfo
{
    UINT                     numTargets;
    ChiTargetPortDescriptor* pTargetPortDesc;
};

/// @brief Combination of pipeline information with buffer information
struct ChiPipelineTargetCreateDescriptor
{
    const CHAR*                 pPipelineName;
    ChiPipelineCreateDescriptor pipelineCreateDesc;
    ChiTargetPortDescriptorInfo sinkTarget;
    ChiTargetPortDescriptorInfo sourceTarget;
};

/// @brief Collection of information summarizing a usecase
struct ChiUsecase
{
    const CHAR*                        pUsecaseName;
    UINT                               streamConfigMode;
    UINT                               numTargets;
    ChiTarget**                        ppChiTargets;
    UINT                               numPipelines;
    ChiPipelineTargetCreateDescriptor* pPipelineTargetCreateDesc;
};

/// @brief Collection of usecases with matching properties (target count at this point)
struct ChiTargetUsecases
{
    UINT        numUsecases;
    ChiUsecase* pChiUsecases;
};
#endif
/*==================== USECASE: OfflineBps =======================*/

static ChiBufferFormat OfflineBps_TARGET_BUFFER_RAW_IN_formats[] =
{
    ChiFormatRawMIPI,
    ChiFormatRawPlain16,
};

static ChiBufferFormat OfflineBps_TARGET_BUFFER_SNAPSHOT_formats[] =
{
    ChiFormatUBWCNV12,
    ChiFormatUBWCTP10,
    ChiFormatYUV420NV12,
};

static ChiTarget OfflineBps_TARGET_BUFFER_RAW_IN_target =
{
    ChiStreamTypeInput,
    {
        0,
        0,
        8000,
        6000
    },
    2,
    OfflineBps_TARGET_BUFFER_RAW_IN_formats,
    NULL
}; // TARGET_BUFFER_RAW_IN

static ChiTarget OfflineBps_TARGET_BUFFER_SNAPSHOT_target =
{
    ChiStreamTypeOutput,
    {
        0,
        0,
        8000,
        6000
    },
    3,
    OfflineBps_TARGET_BUFFER_SNAPSHOT_formats,
    NULL
}; // TARGET_BUFFER_SNAPSHOT

static ChiTarget* OfflineBps_Targets[] =
{
    &OfflineBps_TARGET_BUFFER_RAW_IN_target,
    &OfflineBps_TARGET_BUFFER_SNAPSHOT_target
};

/*****************************Pipeline OfflineBPSFull***************************/

static ChiLinkNodeDescriptor OfflineBps_OfflineBPSFullLink0DestDescriptors[] =
{
    {2, 0, 0, 0}, // SinkBuffer
};

static ChiLinkNodeDescriptor OfflineBps_OfflineBPSFull_source_NodeDescriptors[] =
{
     {65539, 0, 0, 0}
};

static ChiLinkNodeDescriptor OfflineBps_OfflineBPSFull_sink_NodeDescriptors[] =
{
     {65539, 0, 1, 0}
};

static ChiTargetPortDescriptor OfflineBps_OfflineBPSFull_sink_TargetDescriptors[] =
{
    {"TARGET_BUFFER_SNAPSHOT", &OfflineBps_TARGET_BUFFER_SNAPSHOT_target, 1, OfflineBps_OfflineBPSFull_sink_NodeDescriptors }, // TARGET_BUFFER_SNAPSHOT
};

static ChiTargetPortDescriptor OfflineBps_OfflineBPSFull_source_TargetDescriptors[] =
{
    {"TARGET_BUFFER_RAW_IN", &OfflineBps_TARGET_BUFFER_RAW_IN_target, 1, OfflineBps_OfflineBPSFull_source_NodeDescriptors}, // TARGET_BUFFER_RAW_IN
};

static ChiInputPortDescriptor OfflineBps_OfflineBPSFullNode65539_0InputPortDescriptors[] =
{
    {0, 1, 0}, // BPSInputPort
};

static ChiOutputPortDescriptor OfflineBps_OfflineBPSFullNode65539_0OutputPortDescriptors[] =
{
    {1, 1, 1, 0, 0, NULL}, // BPSOutputPortFull
};

static ChiNode OfflineBps_OfflineBPSFullNodes[] =
{
    {NULL, 65539, 0, { 1, OfflineBps_OfflineBPSFullNode65539_0InputPortDescriptors, 1, OfflineBps_OfflineBPSFullNode65539_0OutputPortDescriptors }, 0},
};

static ChiNodeLink OfflineBps_OfflineBPSFullLinks[] =
{
    {{65539, 0, 1, 0}, 1, OfflineBps_OfflineBPSFullLink0DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
};

typedef enum _OfflineBpsPipelineIds
{
    OfflineBPSFull = 0,
} OfflineBpsPipelineIds;

static ChiPipelineTargetCreateDescriptor OfflineBps_pipelines[] =
{
    {"OfflineBPSFull", { 0, 1, OfflineBps_OfflineBPSFullNodes, 1, OfflineBps_OfflineBPSFullLinks, 0, 0, 0, 0, NULL}, {1, OfflineBps_OfflineBPSFull_sink_TargetDescriptors}, {1, OfflineBps_OfflineBPSFull_source_TargetDescriptors}},  // OfflineBPSFull
};

/*==================== USECASE: OfflineSHDR =======================*/

static ChiBufferFormat OfflineSHDRIPE_TARGET_BUFFER_RAW_IN_formats[] =
{
    ChiFormatRawMIPI,
    ChiFormatRawPlain16,
};

static ChiBufferFormat OfflineSHDRIPE_TARGET_BUFFER_SNAPSHOT_formats[] =
{
    ChiFormatUBWCNV12,
    ChiFormatUBWCTP10,
    ChiFormatYUV420NV12,
};

static ChiTarget OfflineSHDRIPE_TARGET_BUFFER_RAW_IN_target =
{
    ChiStreamTypeInput,
    {
        0,
        0,
        8000,
        6000
    },
    2,
    OfflineSHDRIPE_TARGET_BUFFER_RAW_IN_formats,
    NULL
}; // TARGET_BUFFER_RAW_IN

static ChiTarget OfflineSHDRIPE_TARGET_BUFFER_SNAPSHOT_target =
{
    ChiStreamTypeOutput,
    {
        0,
        0,
        8000,
        6000
    },
    3,
    OfflineSHDRIPE_TARGET_BUFFER_SNAPSHOT_formats,
    NULL
}; // TARGET_BUFFER_SNAPSHOT

static ChiTarget* OfflineSHDRIPE_Targets[] =
{
    &OfflineSHDRIPE_TARGET_BUFFER_RAW_IN_target,
    &OfflineSHDRIPE_TARGET_BUFFER_SNAPSHOT_target
};

/*****************************Pipeline OfflineSHDR***************************/

static ChiLinkNodeDescriptor OfflineSHDRIPE_OfflineSHDRFullLink0DestDescriptors[] =
{
    {2, 0, 0, 0}, // SinkBuffer
};

static ChiLinkNodeDescriptor OfflineSHDRIPE_OfflineSHDRFull_source_NodeDescriptors[] =
{
     {255, 0, 0, 0}
};

static ChiLinkNodeDescriptor OfflineSHDRIPE_OfflineSHDRFull_sink_NodeDescriptors[] =
{
     {255, 0, 1, 0}
};

static ChiTargetPortDescriptor OfflineSHDRIPE_OfflineSHDRFull_sink_TargetDescriptors[] =
{
    {"TARGET_BUFFER_SNAPSHOT", &OfflineSHDRIPE_TARGET_BUFFER_SNAPSHOT_target, 1, OfflineSHDRIPE_OfflineSHDRFull_sink_NodeDescriptors }, // TARGET_BUFFER_SNAPSHOT
};

static ChiTargetPortDescriptor OfflineSHDRIPE_OfflineSHDRFull_source_TargetDescriptors[] =
{
    {"TARGET_BUFFER_RAW_IN", &OfflineSHDRIPE_TARGET_BUFFER_RAW_IN_target, 1, OfflineSHDRIPE_OfflineSHDRFull_source_NodeDescriptors }, // TARGET_BUFFER_RAW_IN
};

static ChiInputPortDescriptor OfflineSHDRIPE_OfflineSHDRFullNode65539_0InputPortDescriptors[] =
{
    {0, 1, 0}, // SHDRInputPort
};

static ChiOutputPortDescriptor OfflineSHDRIPE_OfflineSHDRFullNode65539_0OutputPortDescriptors[] =
{
    {1, 1, 1, 0, 0, NULL}, // SHDROutputPortFull
};


static ChiNodeProperty SHDR_properties[] =
{
    {1, "com.qti.node.shdr"},
    {SHDR_AEC_ENABLE_PROPERTY, "0"} // disable AEC dependency.
};

static ChiNode OfflineSHDRIPE_OfflineSHDRFullNodes[] =
{
    {SHDR_properties, 255, 0, { 1, OfflineSHDRIPE_OfflineSHDRFullNode65539_0InputPortDescriptors, 1, OfflineSHDRIPE_OfflineSHDRFullNode65539_0OutputPortDescriptors }, 2},
};

static ChiNodeLink OfflineSHDRIPE_OfflineSHDRFullLinks[] =
{
    {{255, 0, 1, 0}, 1, OfflineSHDRIPE_OfflineSHDRFullLink0DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
};

typedef enum _OfflineSHDRIPEPipelineIds
{
    OfflineSHDRFull = 0,
} OfflineSHDRIPEPipelineIds;

static ChiPipelineTargetCreateDescriptor OfflineSHDRIPE_pipelines[] =
{
    {"OfflineSHDRFull", { 0, 1, OfflineSHDRIPE_OfflineSHDRFullNodes, 1, OfflineSHDRIPE_OfflineSHDRFullLinks, 0, 0, 0, 0, NULL}, {1, OfflineSHDRIPE_OfflineSHDRFull_sink_TargetDescriptors}, {1, OfflineSHDRIPE_OfflineSHDRFull_source_TargetDescriptors}},  // OfflineSHDRFull
};

/*==================== USECASE: OfflineBPSStats =======================*/

static ChiBufferFormat OfflineBPSStats_TARGET_BUFFER_RAW_IN_formats[] =
{
    ChiFormatRawMIPI,
    ChiFormatRawPlain16,
};

static ChiBufferFormat OfflineBPSStats_TARGET_BUFFER_BPS_FULL_formats[] =
{
    ChiFormatUBWCTP10,
};

static ChiBufferFormat OfflineBPSStats_TARGET_BUFFER_BPS_BG_formats[] =
{
    ChiFormatBlob,
};

static ChiBufferFormat OfflineBPSStats_TARGET_BUFFER_BPS_BHIST_formats[] =
{
    ChiFormatBlob,
};

static ChiTarget OfflineBPSStats_TARGET_BUFFER_RAW_IN_target =
{
    ChiStreamTypeInput,
    {
        0,
        0,
        4096,
        2448
    },
    2,
    OfflineBPSStats_TARGET_BUFFER_RAW_IN_formats,
    NULL
}; // TARGET_BUFFER_RAW_IN

static ChiTarget OfflineBPSStats_TARGET_BUFFER_BPS_FULL_target =
{
    ChiStreamTypeOutput,
    {
        0,
        0,
        4096,
        2448
    },
    1,
    OfflineBPSStats_TARGET_BUFFER_BPS_FULL_formats,
    NULL
}; // TARGET_BUFFER_BPS_FULL

static ChiTarget OfflineBPSStats_TARGET_BUFFER_BPS_BG_target =
{
    ChiStreamTypeOutput,
    {
        0,
        0,
        691200,
        1
    },
    1,
    OfflineBPSStats_TARGET_BUFFER_BPS_BG_formats,
    NULL
}; // TARGET_BUFFER_BPS_BG

static ChiTarget OfflineBPSStats_TARGET_BUFFER_BPS_BHIST_target =
{
    ChiStreamTypeOutput,
    {
        0,
        0,
        3072,
        1
    },
    1,
    OfflineBPSStats_TARGET_BUFFER_BPS_BHIST_formats,
    NULL
}; // TARGET_BUFFER_BPS_BHIST

static ChiTarget* OfflineBPSStats_Targets[] =
{
    &OfflineBPSStats_TARGET_BUFFER_RAW_IN_target,
    &OfflineBPSStats_TARGET_BUFFER_BPS_FULL_target,
    &OfflineBPSStats_TARGET_BUFFER_BPS_BG_target,
    &OfflineBPSStats_TARGET_BUFFER_BPS_BHIST_target
};

/*****************************Pipeline BPSStats***************************/

static ChiInputPortDescriptor OfflineBPSStats_BPSStatsNode65539_0InputPortDescriptors[] =
{
    {0, 1, 0}, // BPSInputPort
};
static ChiLinkNodeDescriptor OfflineBPSStats_BPSStats_source_NodeDescriptors[] =
{
    {65539, 0, 0, 0},
};

static ChiLinkNodeDescriptor OfflineBPSStats_BPSStats_TARGET_BUFFER_BPS_FULL_sink_NodeDescriptors[] =
{
    {65539, 0, 1, 0},
};

static ChiLinkNodeDescriptor OfflineBPSStats_BPSStats_TARGET_BUFFER_BPS_BG_sink_NodeDescriptors[] =
{
    {65539, 0, 5, 0},
};

static ChiLinkNodeDescriptor OfflineBPSStats_BPSStats_TARGET_BUFFER_BPS_BHIST_sink_NodeDescriptors[] =
{
    {65539, 0, 6, 0},
};

static ChiOutputPortDescriptor OfflineBPSStats_BPSStatsNode65539_0OutputPortDescriptors[] =
{
    {1, 1, 1, 0, 0, NULL}, // BPSOutputPortFull
    {5, 1, 1, 0, 0, NULL}, // BPSOutputPortStatsBG
    {6, 1, 1, 0, 0, NULL}, // BPSOutputPortStatsBHist
};

static ChiNode OfflineBPSStats_BPSStatsNodes[] =
{
    {NULL, 65539, 0, { 1, OfflineBPSStats_BPSStatsNode65539_0InputPortDescriptors, 3, OfflineBPSStats_BPSStatsNode65539_0OutputPortDescriptors }, 0},
};

static ChiLinkNodeDescriptor OfflineBPSStats_BPSStatsLink0DestDescriptors[] =
{
    {2, 0, 0, 0}, // SinkBuffer
};

static ChiLinkNodeDescriptor OfflineBPSStats_BPSStatsLink1DestDescriptors[] =
{
    {2, 1, 0, 0}, // SinkBuffer
};

static ChiLinkNodeDescriptor OfflineBPSStats_BPSStatsLink2DestDescriptors[] =
{
    {2, 2, 0, 0}, // SinkBuffer
};

static ChiNodeLink OfflineBPSStats_BPSStatsLinks[] =
{
    {{65539, 0, 1, 0}, 1, OfflineBPSStats_BPSStatsLink0DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
    {{65539, 0, 5, 0}, 1, OfflineBPSStats_BPSStatsLink1DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
    {{65539, 0, 6, 0}, 1, OfflineBPSStats_BPSStatsLink2DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
};

static ChiTargetPortDescriptor OfflineBPSStats_BPSStats_sink_TargetDescriptors[] =
{
    {"TARGET_BUFFER_BPS_FULL", &OfflineBPSStats_TARGET_BUFFER_BPS_FULL_target, 1, OfflineBPSStats_BPSStats_TARGET_BUFFER_BPS_FULL_sink_NodeDescriptors}, // TARGET_BUFFER_BPS_FULL
    {"TARGET_BUFFER_BPS_BG", &OfflineBPSStats_TARGET_BUFFER_BPS_BG_target, 1, OfflineBPSStats_BPSStats_TARGET_BUFFER_BPS_BG_sink_NodeDescriptors}, // TARGET_BUFFER_BPS_BG
    {"TARGET_BUFFER_BPS_BHIST", &OfflineBPSStats_TARGET_BUFFER_BPS_BHIST_target, 1, OfflineBPSStats_BPSStats_TARGET_BUFFER_BPS_BHIST_sink_NodeDescriptors}, // TARGET_BUFFER_BPS_BHIST
};

static ChiTargetPortDescriptor OfflineBPSStats_BPSStats_source_TargetDescriptors[] =
{
    {"TARGET_BUFFER_RAW_IN", &OfflineBPSStats_TARGET_BUFFER_RAW_IN_target, 1, OfflineBPSStats_BPSStats_source_NodeDescriptors }, // TARGET_BUFFER_RAW_IN
};

enum OfflineBPSStatsPipelineIds
{
    BPSStats = 0,
};

static ChiPipelineTargetCreateDescriptor OfflineBPSStats_pipelines[] =
{
    {"BPSStats", { 0, 1, OfflineBPSStats_BPSStatsNodes, 3, OfflineBPSStats_BPSStatsLinks, 0, 0, 0, 0, NULL}, {3, OfflineBPSStats_BPSStats_sink_TargetDescriptors}, {1, OfflineBPSStats_BPSStats_source_TargetDescriptors}},  // BPSStats
};

/*==================== USECASE: OfflineShdrBps =======================*/

static ChiBufferFormat OfflineShdrBps_TARGET_BUFFER_RAW_IN_formats[] =
{
    ChiFormatRawMIPI,
    ChiFormatRawPlain16,
};

static ChiBufferFormat OfflineShdrBps_TARGET_BUFFER_SNAPSHOT_formats[] =
{
    ChiFormatUBWCNV12,
    ChiFormatUBWCTP10,
    ChiFormatYUV420NV12,
};

static ChiTarget OfflineShdrBps_TARGET_BUFFER_RAW_IN_target =
{
    ChiStreamTypeInput,
    {
        0,
        0,
        8000,
        6000
    },
    2,
    OfflineShdrBps_TARGET_BUFFER_RAW_IN_formats,
    NULL
}; // TARGET_BUFFER_RAW_IN

static ChiTarget OfflineShdrBps_TARGET_BUFFER_SNAPSHOT_target =
{
    ChiStreamTypeOutput,
    {
        0,
        0,
        8000,
        6000
    },
    3,
    OfflineShdrBps_TARGET_BUFFER_SNAPSHOT_formats,
    NULL
}; // TARGET_BUFFER_SNAPSHOT

static ChiTarget* OfflineShdrBps_Targets[] =
{
    &OfflineShdrBps_TARGET_BUFFER_RAW_IN_target,
    &OfflineShdrBps_TARGET_BUFFER_SNAPSHOT_target
};

/*****************************Pipeline OfflineShdrBpsFull***************************/

static ChiNodeProperty OfflineShdrBps_ShdrBPSFullNode255_0_properties[] =
{
    {1, "com.qti.node.shdr"},
    {SHDR_AEC_ENABLE_PROPERTY, "0"} // Disable AEC dependency.
};

static ChiInputPortDescriptor OfflineShdrBps_ShdrBPSFullNode255_0InputPortDescriptors[] =
{
    {0, 1, 0}, // ChiNodeInputPort
};

static ChiLinkNodeDescriptor OfflineShdrBps_ShdrBPSFull_source_NodeDescriptors[] =
{
    {255, 0, 0, 0},
};

static ChiLinkNodeDescriptor OfflineShdrBps_ShdrBPSFull_sink_NodeDescriptors[] =
{
    {65539, 0, 1, 0},
};

static ChiOutputPortDescriptor OfflineShdrBps_ShdrBPSFullNode255_0OutputPortDescriptors[] =
{
    {0, 0, 0, 0, 0, NULL}, // ChiNodeOutputPort
};

static ChiInputPortDescriptor OfflineShdrBps_ShdrBPSFullNode65539_0InputPortDescriptors[] =
{
    {0, 0, 0}, // BPSInputPort
};

static ChiOutputPortDescriptor OfflineShdrBps_ShdrBPSFullNode65539_0OutputPortDescriptors[] =
{
    {1, 1, 1, 0, 0, NULL}, // BPSOutputPortFull
};

static ChiNode OfflineShdrBps_ShdrBPSFullNodes[] =
{
    {OfflineShdrBps_ShdrBPSFullNode255_0_properties, 255, 0, { 1, OfflineShdrBps_ShdrBPSFullNode255_0InputPortDescriptors, 1, OfflineShdrBps_ShdrBPSFullNode255_0OutputPortDescriptors}, 2},
    {NULL, 65539, 0, { 1, OfflineShdrBps_ShdrBPSFullNode65539_0InputPortDescriptors, 1, OfflineShdrBps_ShdrBPSFullNode65539_0OutputPortDescriptors}, 0},
};

static ChiLinkNodeDescriptor OfflineShdrBps_ShdrBPSFullLink0DestDescriptors[] =
{
    {65539, 0, 0, 0}, // BPS
};

static ChiLinkNodeDescriptor OfflineShdrBps_ShdrBPSFullLink1DestDescriptors[] =
{
    {2, 0, 0, 0}, // SinkBuffer
};

static ChiNodeLink OfflineShdrBps_ShdrBPSFullLinks[] =
{
    {{255, 0, 0, 0}, 1, OfflineShdrBps_ShdrBPSFullLink0DestDescriptors, {ChiFormatRawPlain16, 0, 8, BufferQueueDepth, BufferHeapIon, BufferMemFlagHw|BufferMemFlagLockable|BufferMemFlagCache}, {0, 0}},
    {{65539, 0, 1, 0}, 1, OfflineShdrBps_ShdrBPSFullLink1DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
};

static ChiTargetPortDescriptor OfflineBps_ShdrBPSFull_sink_TargetDescriptors[] =
{
    {"TARGET_BUFFER_SNAPSHOT", &OfflineShdrBps_TARGET_BUFFER_SNAPSHOT_target, 1,OfflineShdrBps_ShdrBPSFull_sink_NodeDescriptors } , // TARGET_BUFFER_SNAPSHOT
};

static ChiTargetPortDescriptor OfflineBps_ShdrBPSFull_source_TargetDescriptors[] =
{
    {"TARGET_BUFFER_RAW_IN", &OfflineShdrBps_TARGET_BUFFER_RAW_IN_target, 1, OfflineShdrBps_ShdrBPSFull_source_NodeDescriptors }, // TARGET_BUFFER_RAW_IN
};

typedef enum _OfflineShdrBpsPipelineIds
{
    ShdrBPSFull = 0,
} OfflineShdrBpsPipelineIds;

static ChiPipelineTargetCreateDescriptor OfflineShdrBps_pipelines[] =
{
    {"ShdrBPSFull", { 0, 2, OfflineShdrBps_ShdrBPSFullNodes, 2, OfflineShdrBps_ShdrBPSFullLinks, 0, 0, 0, 0, NULL}, {1, OfflineBps_ShdrBPSFull_sink_TargetDescriptors}, {1, OfflineBps_ShdrBPSFull_source_TargetDescriptors}},  // ShdrBPSFull
};


/*==================== USECASE: OfflineBPSBGStatsParse =======================*/
static ChiBufferFormat OfflineBPSBGStatsParse_TARGET_BUFFER_RAW_IN_formats[] =
{
    ChiFormatRawMIPI,
    ChiFormatRawPlain16,
};

static ChiBufferFormat OfflineBPSBGStatsParse_TARGET_BUFFER_BPS_FULL_formats[] =
{
    ChiFormatYUV420NV12,
};

static ChiBufferFormat OfflineBPSBGStatsParse_TARGET_BUFFER_PREVIEW_formats[] =
{
    ChiFormatUBWCNV12,
    ChiFormatUBWCTP10,
    ChiFormatYUV420NV12,
};

static ChiTarget OfflineBPSBGStatsParse_TARGET_BUFFER_RAW_IN_target =
{
    ChiStreamTypeInput,
    {
        0,
        0,
        4096,
        2448
    },
    2,
    OfflineBPSBGStatsParse_TARGET_BUFFER_RAW_IN_formats,
    NULL
}; // TARGET_BUFFER_RAW_IN

static ChiTarget OfflineBPSBGStatsParse_TARGET_BUFFER_BPS_FULL_target =
{
    ChiStreamTypeOutput,
    {
        0,
        0,
        4096,
        2448
    },
    1,
    OfflineBPSBGStatsParse_TARGET_BUFFER_BPS_FULL_formats,
    NULL
}; // TARGET_BUFFER_BPS_FULL

static ChiTarget OfflineBPSBGStatsParse_TARGET_BUFFER_PREVIEW_target =
{
    ChiStreamTypeOutput,
    {
        0,
        0,
        4096,
        2448
    },
    3,
    OfflineBPSBGStatsParse_TARGET_BUFFER_PREVIEW_formats,
    NULL
}; // TARGET_BUFFER_PREVIEW

static ChiTarget* OfflineBPSBGStatsParse_Targets[] =
{
    &OfflineBPSBGStatsParse_TARGET_BUFFER_RAW_IN_target,
    &OfflineBPSBGStatsParse_TARGET_BUFFER_BPS_FULL_target,
    &OfflineBPSBGStatsParse_TARGET_BUFFER_PREVIEW_target
};

/*****************************Pipeline BPSBGStatsParse***************************/

static ChiLinkNodeDescriptor OfflineBPSBGStatsParse_BPSBGStatsParse_source_Link_0NodeDescriptors[] =
{
    {65539, 0, 0, 0},
};
static ChiLinkNodeDescriptor OfflineBPSBGStatsParse_BPSBGStatsParse_sink_Link_1NodeDescriptors[] =
{
    {65539, 0, 1, 0},
};
static ChiLinkNodeDescriptor OfflineBPSBGStatsParse_BPSBGStatsParseLink0DestDescriptors[] =
{
    {2, 0, 0, 0}, // SinkBuffer
};

static ChiLinkNodeDescriptor OfflineBPSBGStatsParse_BPSBGStatsParseLink1DestDescriptors[] =
{
    {9, 0, StatsParseInputPortBPSAWBBG, 0}, // StatsParse
    {12, 0, StatsInputPortBPSAWBBG, 0}, // AutoWhiteBalance
};

static ChiLinkNodeDescriptor OfflineBPSBGStatsParse_BPSBGStatsParseLink2DestDescriptors[] =
{
    {3, 0, 0, 0}, // SinkNoBuffer
};

static ChiLinkNodeDescriptor OfflineBPSBGStatsParse_BPSBGStatsParseLink3DestDescriptors[] =
{
    {3, 0, 0, 0}, // SinkNoBuffer
};

static ChiTargetPortDescriptor OfflineBPSBGStatsParse_BPSBGStatsParse_sink_TargetDescriptors[] =
{
    {"TARGET_BUFFER_BPS_FULL", &OfflineBPSBGStatsParse_TARGET_BUFFER_BPS_FULL_target, 1, OfflineBPSBGStatsParse_BPSBGStatsParse_sink_Link_1NodeDescriptors}, // TARGET_BUFFER_BPS_FULL
};

static ChiTargetPortDescriptor OfflineBPSBGStatsParse_BPSBGStatsParse_source_TargetDescriptors[] =
{
    {"TARGET_BUFFER_RAW_IN", &OfflineBPSBGStatsParse_TARGET_BUFFER_RAW_IN_target, 1, OfflineBPSBGStatsParse_BPSBGStatsParse_source_Link_0NodeDescriptors }, // TARGET_BUFFER_RAW_IN
};

static ChiInputPortDescriptor OfflineBPSBGStatsParse_BPSBGStatsParseNode12_0InputPortDescriptors[] =
{
    {StatsInputPortBPSAWBBG, 0, 0},
};

static ChiOutputPortDescriptor OfflineBPSBGStatsParse_BPSBGStatsParseNode12_0OutputPortDescriptors[] =
{
    {0, 1, 0, 0, 0, NULL}, // AWBOutputPort0
};

static ChiInputPortDescriptor OfflineBPSBGStatsParse_BPSBGStatsParseNode65539_0InputPortDescriptors[] =
{
    {0, 1, 0}, // BPSInputPort
};

static ChiOutputPortDescriptor OfflineBPSBGStatsParse_BPSBGStatsParseNode65539_0OutputPortDescriptors[] =
{
    {1, 1, 1, 0, 0, NULL}, // BPSOutputPortFull
    {5, 0, 0, 0, 0, NULL}, // BPSOutputPortStatsBG
};

static ChiInputPortDescriptor OfflineBPSBGStatsParse_BPSBGStatsParseNode9_0InputPortDescriptors[] =
{
    {StatsParseInputPortBPSAWBBG, 0, 0},
};

static ChiOutputPortDescriptor OfflineBPSBGStatsParse_BPSBGStatsParseNode9_0OutputPortDescriptors[] =
{
    {0, 1, 0, 0, 0, NULL}, // StatsParseOutputPort0
};

static ChiNodeProperty OfflineBPSBGStatsParse_BPSBGStatsParse_node12_0_properties[] =
{
    {1, "com.qti.stats.awb"},
};

static ChiNodeProperty OfflineBPSBGStatsParse_BPSBGStatsParse_node9_0_properties[] =
{
    {6, "1"},
};

static ChiNode OfflineBPSBGStatsParse_BPSBGStatsParseNodes[] =
{
    {OfflineBPSBGStatsParse_BPSBGStatsParse_node12_0_properties, 12, 0, { 1, OfflineBPSBGStatsParse_BPSBGStatsParseNode12_0InputPortDescriptors, 1, OfflineBPSBGStatsParse_BPSBGStatsParseNode12_0OutputPortDescriptors }, 1},
    {NULL, 65539, 0, { 1, OfflineBPSBGStatsParse_BPSBGStatsParseNode65539_0InputPortDescriptors, 2, OfflineBPSBGStatsParse_BPSBGStatsParseNode65539_0OutputPortDescriptors }, 0},
    {NULL, 9, 0, { 1, OfflineBPSBGStatsParse_BPSBGStatsParseNode9_0InputPortDescriptors, 1, OfflineBPSBGStatsParse_BPSBGStatsParseNode9_0OutputPortDescriptors }, 0},
};

static ChiNodeLink OfflineBPSBGStatsParse_BPSBGStatsParseLinks[] =
{
    {{65539, 0, 1, 0}, 1, OfflineBPSBGStatsParse_BPSBGStatsParseLink0DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
    {{65539, 0, 5, 0}, 2, OfflineBPSBGStatsParse_BPSBGStatsParseLink1DestDescriptors, {ChiFormatBlob, 691200, 6, BufferQueueDepth, BufferHeapIon, BufferMemFlagLockable|BufferMemFlagHw|BufferMemFlagCache}, {0, 0}},
    {{9, 0, 0, 0}, 1, OfflineBPSBGStatsParse_BPSBGStatsParseLink2DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
    {{12, 0, 0, 0}, 1, OfflineBPSBGStatsParse_BPSBGStatsParseLink3DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
};

enum OfflineBPSBGStatsParsePipelineIds
{
    BPSBGStatsParse = 0,
};

static ChiPipelineTargetCreateDescriptor OfflineBPSBGStatsParse_pipelines[] =
{
    {"BPSBGStatsParse", { 0, 3, OfflineBPSBGStatsParse_BPSBGStatsParseNodes, 4, OfflineBPSBGStatsParse_BPSBGStatsParseLinks, 0, 0, 0, 0, NULL}, {1, OfflineBPSBGStatsParse_BPSBGStatsParse_sink_TargetDescriptors}, {1, OfflineBPSBGStatsParse_BPSBGStatsParse_source_TargetDescriptors}},  // BPSBGStatsParse
};

/*==================== USECASE: OfflineShdrBpsStatsProc =======================*/

static ChiBufferFormat OfflineShdrBpsStatsProc_TARGET_BUFFER_RAW_IN_formats[] =
{
    ChiFormatRawMIPI,
    ChiFormatRawPlain16,
};

static ChiBufferFormat OfflineShdrBpsStatsProc_TARGET_BUFFER_SNAPSHOT_formats[] =
{
    ChiFormatUBWCNV12,
    ChiFormatUBWCTP10,
    ChiFormatYUV420NV12,
};

static ChiTarget OfflineShdrBpsStatsProc_TARGET_BUFFER_RAW_IN_target =
{
    ChiStreamTypeInput,
    {
        0,
        0,
        8000,
        6000
    },
    2,
    OfflineShdrBpsStatsProc_TARGET_BUFFER_RAW_IN_formats,
    NULL
}; // TARGET_BUFFER_RAW_IN

static ChiTarget OfflineShdrBpsStatsProc_TARGET_BUFFER_SNAPSHOT_target =
{
    ChiStreamTypeOutput,
    {
        0,
        0,
        8000,
        6000
    },
    3,
    OfflineShdrBpsStatsProc_TARGET_BUFFER_SNAPSHOT_formats,
    NULL
}; // TARGET_BUFFER_SNAPSHOT

static ChiTarget* OfflineShdrBpsStatsProc_Targets[] =
{
    &OfflineShdrBpsStatsProc_TARGET_BUFFER_RAW_IN_target,
    &OfflineShdrBpsStatsProc_TARGET_BUFFER_SNAPSHOT_target,
    &OfflineBPSBGStatsParse_TARGET_BUFFER_BPS_FULL_target
};
/*
static ChiTarget* OfflineBPSBGStatsParse_Targets[] =
{
    &OfflineBPSBGStatsParse_TARGET_BUFFER_RAW_IN_target,
    &OfflineBPSBGStatsParse_TARGET_BUFFER_BPS_FULL_target,
    &OfflineBPSBGStatsParse_TARGET_BUFFER_PREVIEW_target
};

*/

/*****************************Pipeline OfflineShdrBpsStatsProcFull***************************/

static ChiNodeProperty OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcNode255_0_properties[] =
{
    {1, "com.qti.node.shdr"},
    {SHDR_AEC_ENABLE_PROPERTY, "1"} // Enable AEC dependency.
};

static ChiNodeProperty OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcNode1_0_properties[] =
{
    {1, "com.qti.stats.aec"},
    {6, "1"},// AEC Skip pattern, used to skip AEC processing, 1 = never skip
};

static ChiLinkNodeDescriptor OfflineShdrBpsStatsProcFull_source_Link_0NodeDescriptors[] =
{
    {65539, 0, 1, 0},
};
static ChiLinkNodeDescriptor OfflineShdrBpsStatsProcFull_sink_Link_1NodeDescriptors[] =
{
    {255, 0, 0, 0},
};

static ChiInputPortDescriptor OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcNode255_0InputPortDescriptors[] =
{
    {0, 1, 0}, // ChiNodeInputPort
};

static ChiOutputPortDescriptor OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcNode255_0OutputPortDescriptors[] =
{
    {0, 0, 0, 0, 0, NULL}, // ChiNodeOutputPort
};

static ChiInputPortDescriptor OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcNode65539_0InputPortDescriptors[] =
{
    {0, 0, 0}, // BPSInputPort
};

static ChiOutputPortDescriptor OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcNode65539_0OutputPortDescriptors[] =
{
    {1, 1, 1, 0, 0, NULL}, // BPSOutputPortFull
    {5, 0, 0, 0, 0, NULL}, // BPSOutputPortStatsBG
};

static ChiOutputPortDescriptor OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcNode1_0OutputPortDescriptors[] =
{
    {0, 1, 0, 0, 0, NULL}, // StatsProcessingOutputPort0
};

static ChiNode OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcNodes[] =
{
    {OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcNode255_0_properties, 255, 0, { 1, OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcNode255_0InputPortDescriptors, 1, OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcNode255_0OutputPortDescriptors}, 2},
    {NULL, 65539, 0, { 1, OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcNode65539_0InputPortDescriptors, 2, OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcNode65539_0OutputPortDescriptors}, 0},
    {OfflineBPSBGStatsParse_BPSBGStatsParse_node12_0_properties, 12, 0, { 1, OfflineBPSBGStatsParse_BPSBGStatsParseNode12_0InputPortDescriptors, 1, OfflineBPSBGStatsParse_BPSBGStatsParseNode12_0OutputPortDescriptors }, 1},
    {NULL, 9, 0, { 1, OfflineBPSBGStatsParse_BPSBGStatsParseNode9_0InputPortDescriptors, 1, OfflineBPSBGStatsParse_BPSBGStatsParseNode9_0OutputPortDescriptors }, 0},
    {OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcNode1_0_properties, 1, 0, { 0, NULL, 1, OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcNode1_0OutputPortDescriptors}, 2}
};


static ChiLinkNodeDescriptor OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcLink0DestDescriptors[] =
{
    {65539, 0, 0, 0}, // BPS
};

static ChiLinkNodeDescriptor OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcLink1DestDescriptors[] =
{
    {2, 0, 0, 0}, // SinkBuffer
};

static ChiLinkNodeDescriptor OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcLink2DestDescriptors[] =
{
    {3, 0, 0, 0}, // SinkNoBuffer
};

static ChiNodeLink OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcLinks[] =
{
    {{255, 0, 0, 0}, 1, OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcLink0DestDescriptors, {ChiFormatRawPlain16, 0, 8, BufferQueueDepth, BufferHeapIon, BufferMemFlagHw|BufferMemFlagLockable|BufferMemFlagCache}, {0, 0}},
    {{65539, 0, 1, 0}, 1, OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcLink1DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
    {{65539, 0, 5, 0}, 2, OfflineBPSBGStatsParse_BPSBGStatsParseLink1DestDescriptors, {ChiFormatBlob, 691200, 6, BufferQueueDepth, BufferHeapIon, BufferMemFlagLockable|BufferMemFlagHw|BufferMemFlagCache}, {0, 0}},
    {{9, 0, 0, 0}, 1, OfflineBPSBGStatsParse_BPSBGStatsParseLink2DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
    {{12, 0, 0, 0}, 1, OfflineBPSBGStatsParse_BPSBGStatsParseLink3DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
    {{1, 0, 0, 0}, 1, OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcLink2DestDescriptors, {0,0,0,0,0,0}, {0, 0}}
};

static ChiTargetPortDescriptor OfflineBps_ShdrBPSFullStatsProc_sink_TargetDescriptors[] =
{
    {"TARGET_BUFFER_SNAPSHOT", &OfflineBPSBGStatsParse_TARGET_BUFFER_BPS_FULL_target, 1, OfflineShdrBpsStatsProcFull_source_Link_0NodeDescriptors }, // TARGET_BUFFER_SNAPSHOT
};

static ChiTargetPortDescriptor OfflineBps_ShdrBPSFullStatsProc_source_TargetDescriptors[] =
{
    {"TARGET_BUFFER_RAW_IN", &OfflineShdrBpsStatsProc_TARGET_BUFFER_RAW_IN_target, 1, OfflineShdrBpsStatsProcFull_sink_Link_1NodeDescriptors}, // TARGET_BUFFER_RAW_IN
};

typedef enum _OfflineShdrBpsStatsProcPipelineIds
{
    ShdrBPSFullStatsProc = 0,
} OfflineShdrBpsStatsProcPipelineIds;

static ChiPipelineTargetCreateDescriptor OfflineShdrBpsStatsProc_pipelines[] =
{
    {"ShdrBPSFullStatsProc", { 0, 5, OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcNodes, 6, OfflineShdrBpsStatsProc_ShdrBPSFullStatsProcLinks, 0, 0, 0, 0, NULL}, {1, OfflineBps_ShdrBPSFullStatsProc_sink_TargetDescriptors}, {1, OfflineBps_ShdrBPSFullStatsProc_source_TargetDescriptors}},  // ShdrBPSFullStatsProc
};


/*****************************Pipeline OfflineIPE***************************/

//IPE0 Input Port Descriptors
static ChiInputPortDescriptor OfflineIPE_IpeDispNode65538_0InputPortDescriptors[] =
{
    {0, 1, 0}, // IPEInputPortFull
};
static ChiLinkNodeDescriptor OfflineIPE_source_Link_0NodeDescriptors[] =
{
    {65539, 0, 0, 0},
};
static ChiLinkNodeDescriptor OfflineIPE_sink_Link_1NodeDescriptors[] =
{
    {65538, 0, 8, 0},
};

//  IPE0 Output Port Descriptors
static ChiOutputPortDescriptor
OfflineIPE_IpeDispNode65538_0OutputPortDescriptors[] =
{
    {8, 1, 1, 0, 0, NULL}, // IPEOutputPortDisplay
};

static ChiNode OfflineIPE_IpeDispNodes[] =
{
    {NULL, 65538, 0, {1, OfflineIPE_IpeDispNode65538_0InputPortDescriptors, 1, OfflineIPE_IpeDispNode65538_0OutputPortDescriptors}, 0},
};

static ChiLinkNodeDescriptor OfflineIPE_IpeDisp_Link0_IPE0_Out8Desc[] =
{
    {2, 0, 0, 0}, // SinkBuffer ]-> TARGET_BUFFER_OUTPUT
};

static ChiNodeLink OfflineIPE_IpeDispLinks[] =
{
    {{65538, 0, 8, 0}, 1, OfflineIPE_IpeDisp_Link0_IPE0_Out8Desc, {0,0,0,0,0,0}, {0, 0}},
};

static ChiBufferFormat OfflineIPE_TARGET_BUFFER_OUTPUT_formats[] =
{
    ChiFormatUBWCNV12,
    ChiFormatUBWCTP10,
    ChiFormatYUV420NV12,
};

static ChiTarget OfflineIPE_TARGET_BUFFER_OUTPUT_target =
{
    ChiStreamTypeOutput,
    {   // (MinW, MinH, MaxW, MaxH)
        0, 0, 4096, 2448
    },
    3,
    OfflineIPE_TARGET_BUFFER_OUTPUT_formats,
    NULL
};

static ChiTargetPortDescriptor OfflineIPE_IpeDisp_sink_TargetDescriptors[] =
{
    {"TARGET_BUFFER_OUTPUT", &OfflineIPE_TARGET_BUFFER_OUTPUT_target, 1, OfflineIPE_sink_Link_1NodeDescriptors}, // TARGET_BUFFER_OUTPUT
};

static ChiBufferFormat OfflineIPE_TARGET_BUFFER_IPE_INPUT_formats[] =
{
    ChiFormatUBWCNV12,
    ChiFormatUBWCTP10,
    ChiFormatYUV420NV12,
};

static ChiTarget OfflineIPE_TARGET_BUFFER_IPE_INPUT_target =
{
    ChiStreamTypeInput,
    {   // (MinW, MinH, MaxW, MaxH)
        0, 0, 4096, 2448
    },
    3,
    OfflineIPE_TARGET_BUFFER_IPE_INPUT_formats,
    NULL
};
static ChiTargetPortDescriptor OfflineIPE_IpeDisp_source_TargetDescriptors[] =
{
    {"TARGET_BUFFER_IPE_INPUT", &OfflineIPE_TARGET_BUFFER_IPE_INPUT_target, 1, OfflineIPE_source_Link_0NodeDescriptors }, // TARGET_BUFFER_IPE_INPUT
};

static ChiTarget* OfflineIPE_Targets[] =
{
    &OfflineIPE_TARGET_BUFFER_IPE_INPUT_target,
    &OfflineIPE_TARGET_BUFFER_OUTPUT_target
};

typedef enum OfflineIPEPipelineIds
{
    IpeDisp = 0,
} OfflineIPEPipelineIds;

static ChiPipelineTargetCreateDescriptor OfflineIPE_pipelines[] =
{
    {"IpeDisp", { 0, 1, OfflineIPE_IpeDispNodes, 1, OfflineIPE_IpeDispLinks, 0, 0, 0, 0, NULL}, {1, OfflineIPE_IpeDisp_sink_TargetDescriptors}, {1, OfflineIPE_IpeDisp_source_TargetDescriptors}},  // IpeDisp
};

/*==================== USECASE: OfflineBPSBGBHist =======================*/
static ChiBufferFormat OfflineBPSBGBHist_TARGET_BUFFER_RAW_IN_formats[] =
{
    ChiFormatRawMIPI,
    ChiFormatRawPlain16,
};

static ChiBufferFormat OfflineBPSBGBHist_TARGET_BUFFER_BPS_FULL_formats[] =
{
    ChiFormatYUV420NV12,
};

static ChiBufferFormat OfflineBPSBGBHist_TARGET_BUFFER_PREVIEW_formats[] =
{
    ChiFormatUBWCNV12,
    ChiFormatUBWCTP10,
    ChiFormatYUV420NV12,
};

static ChiTarget OfflineBPSBGBHist_TARGET_BUFFER_RAW_IN_target =
{
    ChiStreamTypeInput,
    {
        0,
        0,
        4096,
        2448
    },
    2,
    OfflineBPSBGBHist_TARGET_BUFFER_RAW_IN_formats,
    NULL
}; // TARGET_BUFFER_RAW_IN

static ChiTarget OfflineBPSBGBHist_TARGET_BUFFER_BPS_FULL_target =
{
    ChiStreamTypeOutput,
    {
        0,
        0,
        4096,
        2448
    },
    1,
    OfflineBPSBGBHist_TARGET_BUFFER_BPS_FULL_formats,
    NULL
}; // TARGET_BUFFER_BPS_FULL

static ChiTarget OfflineBPSBGBHist_TARGET_BUFFER_PREVIEW_target =
{
    ChiStreamTypeOutput,
    {
        0,
        0,
        4096,
        2448
    },
    3,
    OfflineBPSBGBHist_TARGET_BUFFER_PREVIEW_formats,
    NULL
}; // TARGET_BUFFER_PREVIEW

static ChiTarget* OfflineBPSBGBHist_Targets[] =
{
    &OfflineBPSBGBHist_TARGET_BUFFER_RAW_IN_target,
    &OfflineBPSBGBHist_TARGET_BUFFER_BPS_FULL_target,
    &OfflineBPSBGBHist_TARGET_BUFFER_PREVIEW_target
};

/*****************************Pipeline BPSBGBHIST***************************/

static ChiLinkNodeDescriptor OfflineBPSBGBHist_BPSBGBHIST_source_Link_0NodeDescriptors[] =
{
     {65539, 0, 0, 0 } ,
};
static ChiLinkNodeDescriptor OfflineBPSBGBHist_BPSBGBHIST_sink_Link_1NodeDescriptors[] =
{
    {65539, 0, 1, 0 },
};
static ChiLinkNodeDescriptor OfflineBPSBGBHist_BPSBGBHISTLink0DestDescriptors[] =
{
    {2, 0, 0, 0}, // SinkBuffer
};

static ChiLinkNodeDescriptor OfflineBPSBGBHist_BPSBGBHISTLink1DestDescriptors[] =
{
    {9, 0, 2, 0}, // StatsParse
};

static ChiLinkNodeDescriptor OfflineBPSBGBHist_BPSBGBHISTLink2DestDescriptors[] =
{
    {9, 0, StatsParseInputPortBPSAWBBG, 0}, // StatsParse
    {12, 0, StatsInputPortBPSAWBBG, 0}, // AutoWhiteBalance
};

static ChiLinkNodeDescriptor OfflineBPSBGBHist_BPSBGBHISTLink3DestDescriptors[] =
{
    {3, 0, 0, 0}, // SinkNoBuffer
};

static ChiLinkNodeDescriptor OfflineBPSBGBHist_BPSBGBHISTLink4DestDescriptors[] =
{
    {3, 0, 0, 0}, // SinkNoBuffer
};

static ChiTargetPortDescriptor OfflineBPSBGBHist_BPSBGBHIST_sink_TargetDescriptors[] =
{
    {"TARGET_BUFFER_BPS_FULL", &OfflineBPSBGBHist_TARGET_BUFFER_BPS_FULL_target, 1, OfflineBPSBGBHist_BPSBGBHIST_sink_Link_1NodeDescriptors}, // TARGET_BUFFER_BPS_FULL
};

static ChiTargetPortDescriptor OfflineBPSBGBHist_BPSBGBHIST_source_TargetDescriptors[] =
{
    {"TARGET_BUFFER_RAW_IN", &OfflineBPSBGBHist_TARGET_BUFFER_RAW_IN_target, 1,OfflineBPSBGBHist_BPSBGBHIST_source_Link_0NodeDescriptors }, // TARGET_BUFFER_RAW_IN
};

static ChiInputPortDescriptor OfflineBPSBGBHist_BPSBGBHISTNode12_0InputPortDescriptors[] =
{
    {StatsInputPortBPSAWBBG, 0, 0},
};

static ChiOutputPortDescriptor OfflineBPSBGBHist_BPSBGBHISTNode12_0OutputPortDescriptors[] =
{
    {0, 1, 0, 0, 0, NULL}, // AWBOutputPort0
};

static ChiInputPortDescriptor OfflineBPSBGBHist_BPSBGBHISTNode65539_0InputPortDescriptors[] =
{
    {0, 1, 0}, // BPSInputPort
};

static ChiOutputPortDescriptor OfflineBPSBGBHist_BPSBGBHISTNode65539_0OutputPortDescriptors[] =
{
    {1, 1, 1, 0, 0, NULL}, // BPSOutputPortFull
    {6, 0, 0, 0, 0, NULL}, // BPSOutputPortStatsBHIST
    {5, 0, 0, 0, 0, NULL}, // BPSOutputPortStatsBG
};

static ChiInputPortDescriptor OfflineBPSBGBHist_BPSBGBHISTNode9_0InputPortDescriptors[] =
{
    {2, 0, 0}, // StatsParseInputPortHDRBHist
    {StatsParseInputPortBPSAWBBG, 0, 0},
};

static ChiOutputPortDescriptor OfflineBPSBGBHist_BPSBGBHISTNode9_0OutputPortDescriptors[] =
{
    {0, 1, 0, 0, 0, NULL}, // StatsParseOutputPort0
};

static ChiNodeProperty OfflineBPSBGBHist_BPSBGBHIST_node12_0_properties[] =
{
    {1, "com.qti.stats.awb"},
};

static ChiNode OfflineBPSBGBHist_BPSBGBHISTNodes[] =
{
    {OfflineBPSBGBHist_BPSBGBHIST_node12_0_properties, 12, 0, { 1, OfflineBPSBGBHist_BPSBGBHISTNode12_0InputPortDescriptors, 1, OfflineBPSBGBHist_BPSBGBHISTNode12_0OutputPortDescriptors }, 1},
    {NULL, 65539, 0, { 1, OfflineBPSBGBHist_BPSBGBHISTNode65539_0InputPortDescriptors, 3, OfflineBPSBGBHist_BPSBGBHISTNode65539_0OutputPortDescriptors }, 0},
    {NULL, 9, 0, { 2, OfflineBPSBGBHist_BPSBGBHISTNode9_0InputPortDescriptors, 1, OfflineBPSBGBHist_BPSBGBHISTNode9_0OutputPortDescriptors }, 0},
};

static ChiNodeLink OfflineBPSBGBHist_BPSBGBHISTLinks[] =
{
    {{65539, 0, 1, 0}, 1, OfflineBPSBGBHist_BPSBGBHISTLink0DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
    {{65539, 0, 6, 0}, 1, OfflineBPSBGBHist_BPSBGBHISTLink1DestDescriptors, {ChiFormatBlob, 3072, 6, BufferQueueDepth, BufferHeapIon, BufferMemFlagLockable|BufferMemFlagHw|BufferMemFlagCache}, {0, 0}},
    {{65539, 0, 5, 0}, 2, OfflineBPSBGBHist_BPSBGBHISTLink2DestDescriptors, {ChiFormatBlob, 691200, 6, BufferQueueDepth, BufferHeapIon, BufferMemFlagLockable|BufferMemFlagHw|BufferMemFlagCache}, {0, 0}},
    {{9, 0, 0, 0}, 1, OfflineBPSBGBHist_BPSBGBHISTLink3DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
    {{12, 0, 0, 0}, 1, OfflineBPSBGBHist_BPSBGBHISTLink4DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
};

enum OfflineBPSBGBHistPipelineIds
{
    BPSBGBHIST = 0,
};

static ChiPipelineTargetCreateDescriptor OfflineBPSBGBHist_pipelines[] =
{
    {"BPSBGBHIST", { 0, 3, OfflineBPSBGBHist_BPSBGBHISTNodes, 5, OfflineBPSBGBHist_BPSBGBHISTLinks, 0, 0, 0, 0, NULL}, {1, OfflineBPSBGBHist_BPSBGBHIST_sink_TargetDescriptors}, {1, OfflineBPSBGBHist_BPSBGBHIST_source_TargetDescriptors}},  // BPSBGBHIST
};

/*==================== USECASE: OfflineShdrBpsAECAWB =======================*/

static ChiLinkNodeDescriptor OfflineShdrBpsAECAWB_source_Link_0NodeDescriptors[] =
{
     {65539, 0, 0, 0 } ,
};
static ChiLinkNodeDescriptor OfflineShdrBpsAECAWB_sink_Link_1NodeDescriptors[] =
{
    {65539, 0, 1, 0 },
};

static ChiBufferFormat OfflineShdrBpsAECAWB_TARGET_BUFFER_RAW_IN_formats[] =
{
    ChiFormatRawMIPI,
    ChiFormatRawPlain16,
};

static ChiBufferFormat OfflineShdrBpsAECAWB_TARGET_BUFFER_PREVIEW_formats[] =
{
    ChiFormatUBWCNV12,
    ChiFormatUBWCTP10,
    ChiFormatYUV420NV12,
};

static ChiTarget OfflineShdrBpsAECAWB_TARGET_BUFFER_RAW_IN_target =
{
    ChiStreamTypeInput,
    {
        0,
        0,
        8000,
        6000
    },
    2,
    OfflineShdrBpsAECAWB_TARGET_BUFFER_RAW_IN_formats,
    NULL
}; // TARGET_BUFFER_RAW_IN

static ChiTarget OfflineShdrBpsAECAWB_TARGET_BUFFER_PREVIEW_target =
{
    ChiStreamTypeOutput,
    {
        0,
        0,
        8000,
        6000
    },
    3,
    OfflineShdrBpsAECAWB_TARGET_BUFFER_PREVIEW_formats,
    NULL
}; // TARGET_BUFFER_PREVIEW

static ChiTarget* OfflineShdrBpsAECAWB_Targets[] =
{
    &OfflineShdrBpsAECAWB_TARGET_BUFFER_RAW_IN_target,
    &OfflineShdrBpsAECAWB_TARGET_BUFFER_PREVIEW_target
};

/*****************************Pipeline SHDRBPSAECAWB***************************/

static ChiLinkNodeDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWB_source_Link_0NodeDescriptors[] =
{
     {255, 0, 0, 0 } ,
};
static ChiLinkNodeDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWBLink0DestDescriptors[] =
{
    {65539, 0, 0, 0}, // BPS
};

static ChiLinkNodeDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWB_sink_Link_2NodeDescriptors[] =
{
    {65539, 0, 1, 0 },
};
static ChiLinkNodeDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWBLink1DestDescriptors[] =
{
    {2, 0, 0, 0}, // SinkBuffer
};

static ChiLinkNodeDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWBLink2DestDescriptors[] =
{
    {9, 0, 2, 0}, // StatsParse
    {1, 0, 2, 0}, // StatsProcessing
};

static ChiLinkNodeDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWBLink3DestDescriptors[] =
{
    {9, 0, StatsParseInputPortBPSAWBBG, 0}, // StatsParse
    {12, 0, StatsInputPortBPSAWBBG, 0}, // AutoWhiteBalance
    {1, 0, 0, 0}, // StatsProcessing
};

static ChiLinkNodeDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWBLink4DestDescriptors[] =
{
    {3, 0, 0, 0}, // SinkNoBuffer
};

static ChiLinkNodeDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWBLink5DestDescriptors[] =
{
    {3, 0, 0, 0}, // SinkNoBuffer
};

static ChiLinkNodeDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWBLink6DestDescriptors[] =
{
    {3, 0, 0, 0}, // SinkNoBuffer
};

static ChiTargetPortDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWB_sink_TargetDescriptors[] =
{
    {"TARGET_BUFFER_PREVIEW", &OfflineShdrBpsAECAWB_TARGET_BUFFER_PREVIEW_target, 1, OfflineShdrBpsAECAWB_SHDRBPSAECAWB_sink_Link_2NodeDescriptors }, // TARGET_BUFFER_PREVIEW
};

static ChiTargetPortDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWB_source_TargetDescriptors[] =
{
    {"TARGET_BUFFER_RAW_IN", &OfflineShdrBpsAECAWB_TARGET_BUFFER_RAW_IN_target, 1, OfflineShdrBpsAECAWB_SHDRBPSAECAWB_source_Link_0NodeDescriptors}, // TARGET_BUFFER_RAW_IN
};

static ChiInputPortDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode1_0InputPortDescriptors[] =
{
    {2, 0, 0}, // StatsInputPortHDRBHist
    {0, 0, 0}, // StatsInputPortHDRBE
};

static ChiOutputPortDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode1_0OutputPortDescriptors[] =
{
    {0, 1, 0, 0, 0, NULL}, // StatsProcessingOutputPort0
};

static ChiInputPortDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode12_0InputPortDescriptors[] =
{
    {StatsInputPortBPSAWBBG, 0, 0},
};

static ChiOutputPortDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode12_0OutputPortDescriptors[] =
{
    {0, 1, 0, 0, 0, NULL}, // AWBOutputPort0
};

static ChiInputPortDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode255_0InputPortDescriptors[] =
{
    {0, 1, 0}, // SHDRInputPort
};

static ChiOutputPortDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode255_0OutputPortDescriptors[] =
{
    {0, 0, 0, 0, 0, NULL}, // SHDROutputPort
};

static ChiInputPortDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode65539_0InputPortDescriptors[] =
{
    {0, 0, 0}, // BPSInputPort
};

static ChiOutputPortDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode65539_0OutputPortDescriptors[] =
{
    {1, 1, 1, 0, 0, NULL}, // BPSOutputPortFull
    {6, 0, 0, 0, 0, NULL}, // BPSOutputPortStatsBHIST
    {5, 0, 0, 0, 0, NULL}, // BPSOutputPortStatsBG
};

static ChiInputPortDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode9_0InputPortDescriptors[] =
{
    {2, 0, 0}, // StatsParseInputPortHDRBHist
    {StatsParseInputPortBPSAWBBG, 0, 0},
};

static ChiOutputPortDescriptor OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode9_0OutputPortDescriptors[] =
{
    {0, 1, 0, 0, 0, NULL}, // StatsParseOutputPort0
};


static ChiNodeProperty OfflineShdrBpsAECAWB_SHDRBPSAECAWB_node1_0_properties[] =
{
    {1, "com.qti.stats.aec"},
    {6, "1"},// AEC Skip pattern, used to skip AEC processing, 1 = never skip
};

static ChiNodeProperty OfflineShdrBpsAECAWB_SHDRBPSAECAWB_node12_0_properties[] =
{
    {1, "com.qti.stats.awb"},
};

static ChiNodeProperty OfflineShdrBpsAECAWB_SHDRBPSAECAWB_node255_0_properties[] =
{
    {1, "com.qti.node.shdr"},
    {1024, "1"},
};

static ChiNode OfflineShdrBpsAECAWB_SHDRBPSAECAWBNodes[] =
{
    {OfflineShdrBpsAECAWB_SHDRBPSAECAWB_node1_0_properties, 1, 0, { 2, OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode1_0InputPortDescriptors, 1, OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode1_0OutputPortDescriptors }, 2},
    {OfflineShdrBpsAECAWB_SHDRBPSAECAWB_node12_0_properties, 12, 0, { 1, OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode12_0InputPortDescriptors, 1, OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode12_0OutputPortDescriptors }, 1},
    {OfflineShdrBpsAECAWB_SHDRBPSAECAWB_node255_0_properties, 255, 0, { 1, OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode255_0InputPortDescriptors, 1, OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode255_0OutputPortDescriptors }, 2},
    {NULL, 65539, 0, { 1, OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode65539_0InputPortDescriptors, 3, OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode65539_0OutputPortDescriptors }, 0},
    {NULL, 9, 0, { 2, OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode9_0InputPortDescriptors, 1, OfflineShdrBpsAECAWB_SHDRBPSAECAWBNode9_0OutputPortDescriptors }, 0},
};

static ChiNodeLink OfflineShdrBpsAECAWB_SHDRBPSAECAWBLinks[] =
{
    {{255, 0, 0, 0}, 1, OfflineShdrBpsAECAWB_SHDRBPSAECAWBLink0DestDescriptors, {ChiFormatRawPlain16, 0, 8, BufferQueueDepth, BufferHeapIon, BufferMemFlagHw|BufferMemFlagLockable|BufferMemFlagCache}, {0, 0}},
    {{65539, 0, 1, 0}, 1, OfflineShdrBpsAECAWB_SHDRBPSAECAWBLink1DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
    {{65539, 0, 6, 0}, 2, OfflineShdrBpsAECAWB_SHDRBPSAECAWBLink2DestDescriptors, {ChiFormatBlob, 3072, 6, BufferQueueDepth, BufferHeapIon, BufferMemFlagLockable|BufferMemFlagHw|BufferMemFlagCache}, {0, 0}},
    {{65539, 0, 5, 0}, 3, OfflineShdrBpsAECAWB_SHDRBPSAECAWBLink3DestDescriptors, {ChiFormatBlob, 691200, 6, BufferQueueDepth, BufferHeapIon, BufferMemFlagLockable|BufferMemFlagHw|BufferMemFlagCache}, {0, 0}},
    {{9, 0, 0, 0}, 1, OfflineShdrBpsAECAWB_SHDRBPSAECAWBLink4DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
    {{12, 0, 0, 0}, 1, OfflineShdrBpsAECAWB_SHDRBPSAECAWBLink5DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
    {{1, 0, 0, 0}, 1, OfflineShdrBpsAECAWB_SHDRBPSAECAWBLink6DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
};

enum OfflineShdrBpsAECAWBPipelineIds
{
    SHDRBPSAECAWB = 0,
};

static ChiPipelineTargetCreateDescriptor OfflineShdrBpsAECAWB_pipelines[] =
{
    {"SHDRBPSAECAWB", { 0, 5, OfflineShdrBpsAECAWB_SHDRBPSAECAWBNodes, 7, OfflineShdrBpsAECAWB_SHDRBPSAECAWBLinks, 0, 0, 0, 0, NULL}, {1, OfflineShdrBpsAECAWB_SHDRBPSAECAWB_sink_TargetDescriptors}, {1, OfflineShdrBpsAECAWB_SHDRBPSAECAWB_source_TargetDescriptors}},  // SHDRBPSAECAWB
};

/*==================== USECASE: OfflineBpsAECAWB =======================*/

static ChiBufferFormat OfflineBpsAECAWB_TARGET_BUFFER_RAW_IN_formats[] =
{
    ChiFormatRawMIPI,
    ChiFormatRawPlain16,
};

static ChiBufferFormat OfflineBpsAECAWB_TARGET_BUFFER_PREVIEW_formats[] =
{
    ChiFormatUBWCNV12,
    ChiFormatUBWCTP10,
    ChiFormatYUV420NV12,
};

static ChiTarget OfflineBpsAECAWB_TARGET_BUFFER_RAW_IN_target =
{
    ChiStreamTypeInput,
    {
        0,
        0,
        8000,
        6000
    },
    2,
    OfflineBpsAECAWB_TARGET_BUFFER_RAW_IN_formats,
    NULL
}; // TARGET_BUFFER_RAW_IN

static ChiTarget OfflineBpsAECAWB_TARGET_BUFFER_PREVIEW_target =
{
    ChiStreamTypeOutput,
    {
        0,
        0,
        8000,
        6000
    },
    3,
    OfflineBpsAECAWB_TARGET_BUFFER_PREVIEW_formats,
    NULL
}; // TARGET_BUFFER_PREVIEW

static ChiTarget* OfflineBpsAECAWB_Targets[] =
{
    &OfflineBpsAECAWB_TARGET_BUFFER_RAW_IN_target,
    &OfflineBpsAECAWB_TARGET_BUFFER_PREVIEW_target
};

/*****************************Pipeline BPSAECAWB***************************/

static ChiLinkNodeDescriptor BPSAECAWB_source_Link_0NodeDescriptors[] =
{
     {65539, 0, 0, 0 } ,
};
static ChiLinkNodeDescriptor BPSAECAWB_sink_Link_1NodeDescriptors[] =
{
    {65539, 0, 1, 0 },
};

static ChiLinkNodeDescriptor OfflineBpsAECAWB_BPSAECAWBLink0DestDescriptors[] =
{
    {2, 0, 0, 0}, // SinkBuffer
};

static ChiLinkNodeDescriptor OfflineBpsAECAWB_BPSAECAWBLink1DestDescriptors[] =
{
    {9, 0, 2, 0}, // StatsParse
    {1, 0, 2, 0}, // StatsProcessing
};

static ChiLinkNodeDescriptor OfflineBpsAECAWB_BPSAECAWBLink2DestDescriptors[] =
{
    {9, 0, StatsParseInputPortBPSAWBBG, 0}, // StatsParse
    {12, 0, StatsInputPortBPSAWBBG, 0}, // AutoWhiteBalance
    {1, 0, 0, 0}, // StatsProcessing
};

static ChiLinkNodeDescriptor OfflineBpsAECAWB_BPSAECAWBLink3DestDescriptors[] =
{
    {3, 0, 0, 0}, // SinkNoBuffer
};

static ChiLinkNodeDescriptor OfflineBpsAECAWB_BPSAECAWBLink4DestDescriptors[] =
{
    {3, 0, 0, 0}, // SinkNoBuffer
};

static ChiLinkNodeDescriptor OfflineBpsAECAWB_BPSAECAWBLink5DestDescriptors[] =
{
    {3, 0, 0, 0}, // SinkNoBuffer
};

static ChiTargetPortDescriptor OfflineBpsAECAWB_BPSAECAWB_sink_TargetDescriptors[] =
{
    {"TARGET_BUFFER_PREVIEW", &OfflineBpsAECAWB_TARGET_BUFFER_PREVIEW_target, 1, BPSAECAWB_sink_Link_1NodeDescriptors}, // TARGET_BUFFER_PREVIEW
};

static ChiTargetPortDescriptor OfflineBpsAECAWB_BPSAECAWB_source_TargetDescriptors[] =
{
    {"TARGET_BUFFER_RAW_IN", &OfflineBpsAECAWB_TARGET_BUFFER_RAW_IN_target, 1, BPSAECAWB_source_Link_0NodeDescriptors}, // TARGET_BUFFER_RAW_IN
};

static ChiInputPortDescriptor OfflineBpsAECAWB_BPSAECAWBNode1_0InputPortDescriptors[] =
{
    {2, 0, 0}, // StatsInputPortHDRBHist
    {0, 0, 0}, // StatsInputPortHDRBE
};

static ChiOutputPortDescriptor OfflineBpsAECAWB_BPSAECAWBNode1_0OutputPortDescriptors[] =
{
    {0, 1, 0, 0, 0, NULL}, // StatsProcessingOutputPort0
};

static ChiInputPortDescriptor OfflineBpsAECAWB_BPSAECAWBNode12_0InputPortDescriptors[] =
{
    {StatsInputPortBPSAWBBG, 0, 0},
};

static ChiOutputPortDescriptor OfflineBpsAECAWB_BPSAECAWBNode12_0OutputPortDescriptors[] =
{
    {0, 1, 0, 0, 0, NULL}, // AWBOutputPort0
};

static ChiInputPortDescriptor OfflineBpsAECAWB_BPSAECAWBNode65539_0InputPortDescriptors[] =
{
    {0, 1, 0}, // BPSInputPort
};

static ChiOutputPortDescriptor OfflineBpsAECAWB_BPSAECAWBNode65539_0OutputPortDescriptors[] =
{
    {1, 1, 1, 0, 0, NULL}, // BPSOutputPortFull
    {6, 0, 0, 0, 0, NULL}, // BPSOutputPortStatsBHIST
    {5, 0, 0, 0, 0, NULL}, // BPSOutputPortStatsBG
};

static ChiInputPortDescriptor OfflineBpsAECAWB_BPSAECAWBNode9_0InputPortDescriptors[] =
{
    {2, 0, 0}, // StatsParseInputPortHDRBHist
    {StatsParseInputPortBPSAWBBG, 0, 0},
};

static ChiOutputPortDescriptor OfflineBpsAECAWB_BPSAECAWBNode9_0OutputPortDescriptors[] =
{
    {0, 1, 0, 0, 0, NULL}, // StatsParseOutputPort0
};

static ChiNodeProperty OfflineBpsAECAWB_BPSAECAWB_node1_0_properties[] =
{
    {1, "com.qti.stats.aec"},
    {6, "1"},// AEC Skip pattern, used to skip AEC processing, 1 = never skip
};

static ChiNodeProperty OfflineBpsAECAWB_BPSAECAWB_node12_0_properties[] =
{
    {1, "com.qti.stats.awb"},
};

static ChiNode OfflineBpsAECAWB_BPSAECAWBNodes[] =
{
    {OfflineBpsAECAWB_BPSAECAWB_node1_0_properties, 1, 0, { 2, OfflineBpsAECAWB_BPSAECAWBNode1_0InputPortDescriptors, 1, OfflineBpsAECAWB_BPSAECAWBNode1_0OutputPortDescriptors }, 2},
    {OfflineBpsAECAWB_BPSAECAWB_node12_0_properties, 12, 0, { 1, OfflineBpsAECAWB_BPSAECAWBNode12_0InputPortDescriptors, 1, OfflineBpsAECAWB_BPSAECAWBNode12_0OutputPortDescriptors }, 1},
    {NULL, 65539, 0, { 1, OfflineBpsAECAWB_BPSAECAWBNode65539_0InputPortDescriptors, 3, OfflineBpsAECAWB_BPSAECAWBNode65539_0OutputPortDescriptors }, 0},
    {NULL, 9, 0, { 2, OfflineBpsAECAWB_BPSAECAWBNode9_0InputPortDescriptors, 1, OfflineBpsAECAWB_BPSAECAWBNode9_0OutputPortDescriptors }, 0},
};

static ChiNodeLink OfflineBpsAECAWB_BPSAECAWBLinks[] =
{
    {{65539, 0, 1, 0}, 1, OfflineBpsAECAWB_BPSAECAWBLink0DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
    {{65539, 0, 6, 0}, 2, OfflineBpsAECAWB_BPSAECAWBLink1DestDescriptors, {ChiFormatBlob, 3072, 6, BufferQueueDepth, BufferHeapIon, BufferMemFlagLockable|BufferMemFlagHw|BufferMemFlagCache}, {0, 0}},
    {{65539, 0, 5, 0}, 3, OfflineBpsAECAWB_BPSAECAWBLink2DestDescriptors, {ChiFormatBlob, 691200, 6, BufferQueueDepth, BufferHeapIon, BufferMemFlagLockable|BufferMemFlagHw|BufferMemFlagCache}, {0, 0}},
    {{9, 0, 0, 0}, 1, OfflineBpsAECAWB_BPSAECAWBLink3DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
    {{12, 0, 0, 0}, 1, OfflineBpsAECAWB_BPSAECAWBLink4DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
    {{1, 0, 0, 0}, 1, OfflineBpsAECAWB_BPSAECAWBLink5DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
};

enum OfflineBpsAECAWBPipelineIds
{
    BPSAECAWB = 0,
};

static ChiPipelineTargetCreateDescriptor OfflineBpsAECAWB_pipelines[] =
{
    {"BPSAECAWB", { 0, 4, OfflineBpsAECAWB_BPSAECAWBNodes, 6, OfflineBpsAECAWB_BPSAECAWBLinks, 0, 0, 0, 0, NULL}, {1, OfflineBpsAECAWB_BPSAECAWB_sink_TargetDescriptors}, {1, OfflineBpsAECAWB_BPSAECAWB_source_TargetDescriptors}},  // BPSAECAWB
};

/*****************************Pipeline OfflineBPSIPE***************************/

static ChiBufferFormat OfflineBPSIPE_TARGET_BUFFER_RAW_IN_formats[] =
{
    ChiFormatRawMIPI,
    ChiFormatRawPlain16,
};

static ChiTarget OfflineBPSIPE_TARGET_BUFFER_RAW_IN_target =
{
    ChiStreamTypeInput,
    {
        0,
        0,
        8000,
        6000
    },
    2,
    OfflineBPSIPE_TARGET_BUFFER_RAW_IN_formats,
    NULL
}; // TARGET_BUFFER_RAW_IN

static ChiBufferFormat OfflineBPSIPE_TARGET_BUFFER_OUTPUT_formats[] =
{
    ChiFormatUBWCNV12,
    ChiFormatUBWCTP10,
    ChiFormatYUV420NV12,
};

static ChiTarget OfflineBPSIPE_TARGET_BUFFER_OUTPUT_target =
{
    ChiStreamTypeOutput,
    {   // (MinW, MinH, MaxW, MaxH)
        0, 0, 4096, 2448
    },
    3,
    OfflineBPSIPE_TARGET_BUFFER_OUTPUT_formats,
    NULL
};


static ChiTarget* OfflineBPSIPE_Targets[] =
{
    &OfflineBPSIPE_TARGET_BUFFER_RAW_IN_target,
    &OfflineBPSIPE_TARGET_BUFFER_OUTPUT_target
};

typedef enum OfflineBPSIPEPipelineIds
{
    OfflineBpsIpeDisp = 0,
} OfflineBPSIPEPipelineIds;

static ChiLinkNodeDescriptor OfflineBPSIPE_source_Link_0NodeDescriptors[] =
{
     {65539, 0, 0, 0 } ,
};
static ChiLinkNodeDescriptor OfflineBPSIPE_sink_Link_1NodeDescriptors[] =
{
    {65538, 0, 8, 0 },
};


static ChiLinkNodeDescriptor OfflineBPSIPE_OfflineBPSIPEDisp_Link0_BPS0_Out1Desc[] =
{
    {65538, 0, 0, 0}, // IPE
};

static ChiLinkNodeDescriptor OfflineBPSIPE_OfflineBPSIPEDisp_Link1_IPE0_Out8Desc[] =
{
    {2, 0, 0, 0}, // SinkBuffer ]-> TARGET_BUFFER_OUTPUT
};

static ChiTargetPortDescriptor OfflineBPSIPE_OfflineBPSIPEDisp_sink_TargetDescriptors[] =
{
    {"TARGET_BUFFER_OUTPUT", &OfflineBPSIPE_TARGET_BUFFER_OUTPUT_target, 1, OfflineBPSIPE_sink_Link_1NodeDescriptors}, // TARGET_BUFFER_OUTPUT
};

static ChiTargetPortDescriptor OfflineBPSIPE_OfflineBPSIPEDisp_source_TargetDescriptors[] =
{
    {"TARGET_BUFFER_RAW_IN", &OfflineBPSIPE_TARGET_BUFFER_RAW_IN_target, 1, OfflineBPSIPE_source_Link_0NodeDescriptors}, // TARGET_BUFFER_RAW_IN
};

// IPE0 Input Port Descriptors
static ChiInputPortDescriptor OfflineBPSIPE_OfflineBPSIPEDispNode65538_0InputPortDescriptors[] =
{
    {0, 0, 0}, // IPEInputPortFull
};

// IPE0 Output Port Descriptors
static ChiOutputPortDescriptor OfflineBPSIPE_OfflineBPSIPEDispNode65538_0OutputPortDescriptors[] =
{
    {8, 1, 1, 0, 0, NULL}, // IPEOutputPortDisplay
};

// BPS0 Input Port Descriptors
static ChiInputPortDescriptor OfflineBPSIPE_OfflineBPSIPEDispNode65539_0InputPortDescriptors[] =
{
    {0, 1, 0}, // BPSInputPort
};

// BPS0 Output Port Descriptors
static ChiOutputPortDescriptor OfflineBPSIPE_OfflineBPSIPEDispNode65539_0OutputPortDescriptors[] =
{
    {1, 0, 0, 0, 0, NULL}, // BPSOutputPortFull
};

static ChiNode OfflineBPSIPE_OfflineBPSIPEDispNodes[] =
{
    {NULL, 65539, 0, {1, OfflineBPSIPE_OfflineBPSIPEDispNode65539_0InputPortDescriptors, 1, OfflineBPSIPE_OfflineBPSIPEDispNode65539_0OutputPortDescriptors}, 0 },
    {NULL, 65538, 0, {1, OfflineBPSIPE_OfflineBPSIPEDispNode65538_0InputPortDescriptors, 1, OfflineBPSIPE_OfflineBPSIPEDispNode65538_0OutputPortDescriptors}, 0 },
};

static ChiNodeLink OfflineBPSIPE_OfflineBPSIPEDispLinks[] =
{
    {{65539, 0, 1, 0}, 1, OfflineBPSIPE_OfflineBPSIPEDisp_Link0_BPS0_Out1Desc, { ChiFormatYUV420NV12, 0, 8, BufferQueueDepth, BufferHeapIon, BufferMemFlagHw }, {0, 0}},
    {{65538, 0, 8, 0}, 1, OfflineBPSIPE_OfflineBPSIPEDisp_Link1_IPE0_Out8Desc, {0,0,0,0,0,0}, {0, 0}},
};

static ChiPipelineTargetCreateDescriptor OfflineBPSIPE_pipelines[] =
{
    {"OfflineBpsIpeDisp", { 0, 2, OfflineBPSIPE_OfflineBPSIPEDispNodes, 2, OfflineBPSIPE_OfflineBPSIPEDispLinks, 0, 0, 0, 0, NULL}, {1, OfflineBPSIPE_OfflineBPSIPEDisp_sink_TargetDescriptors}, {1, OfflineBPSIPE_OfflineBPSIPEDisp_source_TargetDescriptors}},  // BpsIpeDisp
};

/*==================== USECASE: OfflineBPSAECAWBIPE =======================*/

static ChiBufferFormat OfflineBPSAECAWBIPE_TARGET_BUFFER_RAW_IN_formats[] =
{
    ChiFormatRawMIPI,
    ChiFormatRawPlain16,
};

static ChiBufferFormat OfflineBPSAECAWBIPE_TARGET_BUFFER_OUTPUT_formats[] =
{
    ChiFormatUBWCNV12,
    ChiFormatUBWCTP10,
    ChiFormatYUV420NV12,
};

static ChiTarget OfflineBPSAECAWBIPE_TARGET_BUFFER_RAW_IN_target =
{
    ChiStreamTypeInput,
    {
        0,
        0,
        8000,
        6000
    },
    2,
    OfflineBPSAECAWBIPE_TARGET_BUFFER_RAW_IN_formats,
    NULL
}; // TARGET_BUFFER_RAW_IN

static ChiTarget OfflineBPSAECAWBIPE_TARGET_BUFFER_OUTPUT_target =
{
    ChiStreamTypeOutput,
    {
        0,
        0,
        4096,
        2448
    },
    3,
    OfflineBPSAECAWBIPE_TARGET_BUFFER_OUTPUT_formats,
    NULL
}; // TARGET_BUFFER_PREVIEW
static ChiTarget* OfflineBPSAECAWBIPE_Targets[] =
{
    &OfflineBPSAECAWBIPE_TARGET_BUFFER_RAW_IN_target,
    &OfflineBPSAECAWBIPE_TARGET_BUFFER_OUTPUT_target
};

/*****************************Pipeline OfflineBPSAECAWBIPE***************************/
static ChiLinkNodeDescriptor OfflineBPSAECAWBIPE_source_Link_0NodeDescriptors[] =
{
     {65539, 0, 0, 0 } ,
};
static ChiLinkNodeDescriptor OfflineBPSAECAWBIPE_sink_Link_1NodeDescriptors[] =
{
    {65538, 0, 8, 0 },
};


static ChiTargetPortDescriptor OfflineBPSAECAWBIPE_sink_TargetDescriptors[] =
{
    {"TARGET_BUFFER_OUTPUT", &OfflineBPSAECAWBIPE_TARGET_BUFFER_OUTPUT_target, 1,OfflineBPSAECAWBIPE_sink_Link_1NodeDescriptors}, // TARGET_BUFFER_OUTPUT
};

static ChiTargetPortDescriptor OfflineBPSAECAWBIPE_source_TargetDescriptors[] =
{
    {"TARGET_BUFFER_RAW_IN", &OfflineBPSAECAWBIPE_TARGET_BUFFER_RAW_IN_target, 1 ,OfflineBPSAECAWBIPE_source_Link_0NodeDescriptors}, // TARGET_BUFFER_RAW_IN
};

static ChiNodeProperty OfflineBPSAECAWBIPE_node1_0_properties[] =
{
    {1, "com.qti.stats.aec"},
    {6, "1"},// AEC Skip pattern, used to skip AEC processing, 1 = never skip
};

static ChiInputPortDescriptor OfflineBPSAECAWBIPE_Node1_0InputPortDescriptors[] =
{
    {2, 0, 0}, // StatsInputPortHDRBHist
    {0, 0, 0}, // StatsInputPortHDRBE
};

static ChiOutputPortDescriptor OfflineBPSAECAWBIPE_Node1_0OutputPortDescriptors[] =
{
    {0, 1, 0, 0, 0, NULL}, // StatsProcessingOutputPort0
};

static ChiNodeProperty OfflineBPSAECAWBIPE_node12_0_properties[] =
{
    {1, "com.qti.stats.awb"},
};

static ChiInputPortDescriptor OfflineBPSAECAWBIPE_Node12_0InputPortDescriptors[] =
{
    {StatsInputPortBPSAWBBG, 0, 0},
};

static ChiOutputPortDescriptor OfflineBPSAECAWBIPE_Node12_0OutputPortDescriptors[] =
{
    {0, 1, 0, 0, 0, NULL}, // AWBOutputPort0
};

static ChiInputPortDescriptor OfflineBPSAECAWBIPE_Node65539_0InputPortDescriptors[] =
{
    {0, 1, 0}, // BPSInputPort
};

static ChiOutputPortDescriptor OfflineBPSAECAWBIPE_Node65539_0OutputPortDescriptors[] =
{
    {1, 0, 0, 0, 0, NULL}, // BPSOutputPortFull
    {6, 0, 0, 0, 0, NULL}, // BPSOutputPortStatsBHIST
    {5, 0, 0, 0, 0, NULL}, // BPSOutputPortStatsBG
};


static ChiInputPortDescriptor OfflineBPSAECAWBIPE_Node9_0InputPortDescriptors[] =
{
    {2, 0, 0}, // StatsParseInputPortHDRBHist
    {StatsParseInputPortBPSAWBBG, 0, 0},
};

static ChiOutputPortDescriptor OfflineBPSAECAWBIPE_Node9_0OutputPortDescriptors[] =
{
    {0, 1, 0, 0, 0, NULL}, // StatsParseOutputPort0
};

// IPE0 Input Port Descriptors
static ChiInputPortDescriptor OfflineBPSAECAWBIPE_Node65538_0InputPortDescriptors[] =
{
    {0, 0, 0}, // IPEInputPortFull
};

// IPE0 Output Port Descriptors
static ChiOutputPortDescriptor OfflineBPSAECAWBIPE_Node65538_0OutputPortDescriptors[] =
{
    {8, 1, 1, 0, 0, NULL}, // IPEOutputPortDisplay
};

static ChiLinkNodeDescriptor OfflineBPSAECAWBIPE_Link0DestDescriptors[] =
{
    {65538, 0, 0, 0}, // IPE
};

static ChiLinkNodeDescriptor OfflineBPSAECAWBIPE_Link1DestDescriptors[] =
{
    {9, 0, 2, 0}, // StatsParse
    {1, 0, 2, 0}, // StatsProcessing
};

static ChiLinkNodeDescriptor OfflineBPSAECAWBIPE_Link2DestDescriptors[] =
{
    {9, 0, StatsParseInputPortBPSAWBBG, 0}, // StatsParse
    {12, 0, StatsInputPortBPSAWBBG, 0}, // AutoWhiteBalance
    {1, 0, 0, 0}, // StatsProcessing
};


static ChiLinkNodeDescriptor OfflineBPSAECAWBIPE_Link3DestDescriptors[] =
{
    {3, 0, 0, 0}, // SinkNoBuffer
};

static ChiLinkNodeDescriptor OfflineBPSAECAWBIPE_Link4DestDescriptors[] =
{
    {3, 0, 0, 0}, // SinkNoBuffer
};

static ChiLinkNodeDescriptor OfflineBPSAECAWBIPE_Link5DestDescriptors[] =
{
    {3, 0, 0, 0}, // SinkNoBuffer
};

static ChiLinkNodeDescriptor OfflineBPSAECAWBIPE_Link6DestDescriptors[] =
{
    {2, 0, 0, 0}, // SinkBuffer ]> TARGET_BUFFER_OUTPUT
};

static ChiNodeLink OfflineBPSAECAWBIPE_ISPLinks[] =
{
    {{65539, 0, 1, 0}, 1, OfflineBPSAECAWBIPE_Link0DestDescriptors, { ChiFormatYUV420NV12, 0, 8, BufferQueueDepth, BufferHeapIon, BufferMemFlagHw }, {0, 0}},
    {{65539, 0, 6, 0}, 2, OfflineBPSAECAWBIPE_Link1DestDescriptors, {ChiFormatBlob, 3072, 6, BufferQueueDepth, BufferHeapIon, BufferMemFlagLockable|BufferMemFlagHw|BufferMemFlagCache}, {0, 0}},
    {{65539, 0, 5, 0}, 3, OfflineBPSAECAWBIPE_Link2DestDescriptors, {ChiFormatBlob, 691200, 6, BufferQueueDepth, BufferHeapIon, BufferMemFlagLockable|BufferMemFlagHw|BufferMemFlagCache}, {0, 0}},
    {{9, 0, 0, 0}, 1, OfflineBPSAECAWBIPE_Link3DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
    {{12, 0, 0, 0}, 1, OfflineBPSAECAWBIPE_Link4DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
    {{1, 0, 0, 0}, 1, OfflineBPSAECAWBIPE_Link5DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
    {{65538, 0, 8, 0}, 1, OfflineBPSAECAWBIPE_Link6DestDescriptors, {0,0,0,0,0,0}, {0, 0}},
};

static ChiNode OfflineBPSAECAWBIPE_ISPNodes[] =
{
    {OfflineBPSAECAWBIPE_node1_0_properties, 1, 0, { 2, OfflineBPSAECAWBIPE_Node1_0InputPortDescriptors, 1, OfflineBPSAECAWBIPE_Node1_0OutputPortDescriptors }, 2},
    {OfflineBPSAECAWBIPE_node12_0_properties, 12, 0, { 1, OfflineBPSAECAWBIPE_Node12_0InputPortDescriptors, 1, OfflineBPSAECAWBIPE_Node12_0OutputPortDescriptors }, 1},
    {NULL, 65539, 0, { 1, OfflineBPSAECAWBIPE_Node65539_0InputPortDescriptors, 3, OfflineBPSAECAWBIPE_Node65539_0OutputPortDescriptors }, 0},
    {NULL, 9, 0, { 2, OfflineBPSAECAWBIPE_Node9_0InputPortDescriptors, 1, OfflineBPSAECAWBIPE_Node9_0OutputPortDescriptors }, 0},
    {NULL, 65538, 0, {1, OfflineBPSAECAWBIPE_Node65538_0InputPortDescriptors, 1, OfflineBPSAECAWBIPE_Node65538_0OutputPortDescriptors}, 0 },
};

static ChiPipelineTargetCreateDescriptor OfflineBPSAECAWBIPE_pipelines[] =
{
    {"BPSAECAWBIPE", { 0, 5, OfflineBPSAECAWBIPE_ISPNodes, 7, OfflineBPSAECAWBIPE_ISPLinks, 0, 0, 0, 0, NULL}, {1, OfflineBPSAECAWBIPE_sink_TargetDescriptors}, {1, OfflineBPSAECAWBIPE_source_TargetDescriptors}},  // FullPipeline
};


typedef enum OfflineBPSAECAWBIPEPipelineIds
{
    OfflineBPSAECAWBIPEPipelineId = 0,
} OfflineBPSAECAWBIPEPipelineIds;


typedef enum _UsecaseId
{
    UsecaseOfflineBpsId = 0,
    UsecaseOfflineSHDRId,
    UsecaseOfflineBPSStatsId,
    UsecaseOfflineShdrBpsId,
    UsecaseOfflineShdrBpsStatsProcId,
    UsecaseOfflineBPSBGStatsParseId,
    UsecaseOfflineIpeId,
    UsecaseOfflineBPSBGBHistId,
    UsecaseOfflineShdrBpsAECAWB,
    UsecaseOfflineBpsAECAWB,
    UsecaseOfflineBpsIpe,
    UsecaseOfflineBpsIpeAECAWB,
    UsecaseOfflineShdrBpsIpeAECAWB,
    UsecaseMaxId
} UsecaseId;

static ChiUsecase IspUsecases[UsecaseMaxId] =
{
    {"UsecaseOfflineBps" , 0, 2, OfflineBps_Targets, 1, OfflineBps_pipelines},
    {"UsecaseOfflineSHDRIPE" , 0, 2, OfflineSHDRIPE_Targets, 3, OfflineSHDRIPE_pipelines},
    {"UsecaseOfflineBPSStats", 0, 4, OfflineBPSStats_Targets, 1, OfflineBPSStats_pipelines},
    {"UsecaseOfflineShdrBpsId" , 0, 2, OfflineShdrBps_Targets, 1, OfflineShdrBps_pipelines},
    {"UsecaseOfflineShdrBpsStatsProcId" , 0, 3, OfflineShdrBpsStatsProc_Targets, 1, OfflineShdrBpsStatsProc_pipelines},
    {"UsecaseOfflineBPSBGStatsParse", 0, 3, OfflineBPSBGStatsParse_Targets, 1, OfflineBPSBGStatsParse_pipelines},
    {"UsecaseOfflineIPE"    , 0, 2, OfflineIPE_Targets,    2, OfflineIPE_pipelines},
    {"UsecaseOfflineBPSBGBHist", 0, 3, OfflineBPSBGBHist_Targets, 1, OfflineBPSBGBHist_pipelines},
    {"UsecaseOfflineShdrBpsAECAWB", 0, 2, OfflineShdrBpsAECAWB_Targets, 1, OfflineShdrBpsAECAWB_pipelines},
    {"UsecaseOfflineBpsAECAWB", 0, 2, OfflineBpsAECAWB_Targets, 1, OfflineBpsAECAWB_pipelines},
    {"UsecaseOfflineBpsIpe" , 0, 2, OfflineBPSIPE_Targets, 1, OfflineBPSIPE_pipelines},
    {"UsecaseOfflineBpsIpeAECAWB", 0, 2, OfflineBPSAECAWBIPE_Targets, 1, OfflineBPSAECAWBIPE_pipelines},
#ifdef AIS_ISP_ENABLE_JPEG
    *g_pOfflineShdrBpsAECAWBIPEJPEG,
#else
    *g_pOfflineShdrBpsAECAWBIPE,
#endif
};

#endif // __CHIPIPELINES_DEF_H__

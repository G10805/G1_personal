/*!
 * Copyright (c) 2016-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ais_proc_chain.h"

#if !defined(__INTEGRITY)
#include "ais_pproc_gpu.h"
#include "ais_pproc_isp.h"
#include "ais_pproc_framesync.h"
#include "ais_pproc_rgbir.h"
#include "ais_pproc_memcpy.h"
#include "ais_pproc_ext.h"
#endif
#include "ais_pproc_usrdone.h"
#include "CameraPlatform.h"

//////////////////////////////////////////////////////////////////////////////////
/// MACRO DEFINITIONS
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
/// TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
/// FORWARD DECLARE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////
static CameraResult BuflistAllocDefault(AisUsrCtxt* pUsrCtxt, const AisBuflistDefType* pBuflistDef);

static AisBuffer* GetFreeBufDefault(AisUsrCtxt* pUsrCtxt, AisBufferList* pBufferList);
static CameraResult ReturnBufDefault(AisUsrCtxt* pUsrCtxt, AisBufferList* pBufferList, uint32 idx);

static AisBuffer* BuflistDeqIfe(AisUsrCtxt* pUsrCtxt, AisBufferList* pBufferList);
static CameraResult BuflistEnqIfe(AisUsrCtxt* pUsrCtxt, AisBufferList* pBufferList, uint32 idx);

//////////////////////////////////////////////////////////////////////////////////
/// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////////
/**
 * RAW Dump usecase
 */
static const AisBuflistDefType RawdumpBuflist[] =
{
    {
        AIS_BUFLIST_USR_0,            /* id */
        GetFreeBufDefault,            /* GetFreeBuf */
        BuflistEnqIfe,                /* ReturnBuf */
        NULL,                         /* AllocBuf */
        {                             /* allocParams */
            AIS_BUFLIST_ALLOC_NONE,   /* allocType */
            QCARCAM_MAX_NUM_BUFFERS,  /* maxBuffers */
            0,                        /* matchBuflistId */
            (qcarcam_color_fmt_t)0,   /* fmt */
            0,                        /* width */
            0,                        /* height */
            0,                        /* stride */
            0                         /* align */
        }
    },
};

static const AisProcChainType RawdumpPProc[] =
{
    {
        .id = AIS_PPROC_USR_DONE,
        .pprocFunction = 0,
        .inBuflistId = {AIS_BUFLIST_USR_0},
        .outBuflistId = {},
        .pLibName = "",
        .instanceId = 0,
        .numIn = 1,
        .numOut = 0,
        .stepType = AIS_PPROC_STEP_REQUIRED
    }
};

static const AisProcChainDefType RawdumpProcChainDef =
{
    RawdumpPProc,                     /* pProcChain */
    STD_ARRAY_SIZE(RawdumpPProc),     /* nProc */
    RawdumpBuflist,                   /* pBufferlistDef */
    STD_ARRAY_SIZE(RawdumpBuflist),   /* nBuflist */
    1,                                /* nStreams */
    { {AIS_STREAM_TYPE_IFE, AIS_BUFLIST_OUTPUT_USR}, {}} /* streams */
};

#if !defined(__INTEGRITY)
/**
 * 2 streams RAW Dump usecase (e.g. image + embedded data)
 */
static const AisBuflistDefType DualStreamRawdumpBuflist[] =
{
    {
        AIS_BUFLIST_USR_0,            /* id */
        GetFreeBufDefault,            /* GetFreeBuf */
        BuflistEnqIfe,                /* ReturnBuf */
        NULL,                         /* AllocBuf */
        {                             /* allocParams */
            AIS_BUFLIST_ALLOC_NONE,   /* allocType */
            QCARCAM_MAX_NUM_BUFFERS,  /* maxBuffers */
            0,                        /* matchBuflistId */
            (qcarcam_color_fmt_t)0,   /* fmt */
            0,                        /* width */
            0,                        /* height */
            0,                        /* stride */
            0                         /* align */
        }
    },
    {
        AIS_BUFLIST_USR_1,            /* id */
        GetFreeBufDefault,            /* GetFreeBuf */
        BuflistEnqIfe,                /* ReturnBuf */
        NULL,                         /* AllocBuf */
        {                             /* allocParams */
            AIS_BUFLIST_ALLOC_NONE,   /* allocType */
            QCARCAM_MAX_NUM_BUFFERS,  /* maxBuffers */
            0,                        /* matchBuflistId */
            (qcarcam_color_fmt_t)0,   /* fmt */
            0,                        /* width */
            0,                        /* height */
            0,                        /* stride */
            0                         /* align */
        }
    },
};

static const AisProcChainType DualStreamRawdumpPProc[] =
{
    {
        .id = AIS_PPROC_USR_DONE,
        .pprocFunction = 0,
        .inBuflistId = {AIS_BUFLIST_USR_0, AIS_BUFLIST_USR_1},
        .outBuflistId = {},
        .pLibName = "",
        .instanceId = 0,
        .numIn = 2,
        .numOut = 0,
        .stepType = AIS_PPROC_STEP_REQUIRED
    }
};

static const AisProcChainDefType DualStreamRawdumpProcChainDef =
{
    DualStreamRawdumpPProc,                     /* pProcChain */
    STD_ARRAY_SIZE(DualStreamRawdumpPProc),     /* nProc */
    DualStreamRawdumpBuflist,                   /* pBufferlistDef */
    STD_ARRAY_SIZE(DualStreamRawdumpBuflist),   /* nBuflist */
    2,                                        /* nStreams */
    { {AIS_STREAM_TYPE_IFE, AIS_BUFLIST_USR_0}, {AIS_STREAM_TYPE_IFE, AIS_BUFLIST_USR_1}} /* streams */
};


/**
 * Inject ISP PPROC usecase
 */
static const AisBuflistDefType InjectRawdumpBuflist[] =
{
    {
        AIS_BUFLIST_INPUT_USR,        /* id */
        GetFreeBufDefault,            /* GetFreeBuf */
        ReturnBufDefault,             /* ReturnBuf */
        NULL,                         /* AllocBuf */
        {                             /* allocParams */
            AIS_BUFLIST_ALLOC_NONE,   /* allocType */
            QCARCAM_MAX_NUM_BUFFERS,  /* maxBuffers */
            0,                        /* matchBuflistId */
            (qcarcam_color_fmt_t)0,   /* fmt */
            0,                        /* width */
            0,                        /* height */
            0,                        /* stride */
            0                         /* align */
        }
    },
    {
        AIS_BUFLIST_OUTPUT_USR,       /* id */
        GetFreeBufDefault,            /* GetFreeBuf */
        ReturnBufDefault,             /* ReturnBuf */
        NULL,                         /* AllocBuf */
        {                             /* allocParams */
            AIS_BUFLIST_ALLOC_NONE,   /* allocType */
            QCARCAM_MAX_NUM_BUFFERS,  /* maxBuffers */
            0,                        /* matchBuflistId */
            (qcarcam_color_fmt_t)0,   /* fmt */
            0,                        /* width */
            0,                        /* height */
            0,                        /* stride */
            0                         /* align */
        }
    }
};

static const AisProcChainType InjectRawdumpPProc[] =
{
    {
        .id = AIS_PPROC_ISP,
        .pprocFunction = 0,
        .inBuflistId = {AIS_BUFLIST_INPUT_USR},
        .outBuflistId = {AIS_BUFLIST_OUTPUT_USR},
        .pLibName = "",
        .instanceId = 0,
        .numIn = 1,
        .numOut = 1,
        .stepType = AIS_PPROC_STEP_REQUIRED
    },
    {
        .id = AIS_PPROC_USR_DONE,
        .pprocFunction = 0,
        .inBuflistId = {AIS_BUFLIST_OUTPUT_USR},
        .outBuflistId = {},
        .pLibName = "",
        .instanceId = 0,
        .numIn = 1,
        .numOut = 0,
        .stepType = AIS_PPROC_STEP_REQUIRED
    }
};

static const AisProcChainDefType InjectProcChainDef =
{
    InjectRawdumpPProc,                     /* pProcChain */
    STD_ARRAY_SIZE(InjectRawdumpPProc),     /* nProc */
    InjectRawdumpBuflist,                   /* pBufferlistDef */
    STD_ARRAY_SIZE(InjectRawdumpBuflist),   /* nBuflist */
    1,                                      /* nStreams */
    { {AIS_STREAM_TYPE_USER, AIS_BUFLIST_INPUT_USR}, {}}    /* streams */
};


/**
 * SHDR usecase
 */
static const AisBuflistDefType ISP_Buflist[] =
{
    {
        AIS_BUFLIST_VFE_RAW_OUT,       /* id */
        BuflistDeqIfe,                /* GetFreeBuf */
        BuflistEnqIfe,                 /* ReturnBuf */
        BuflistAllocDefault,           /* AllocBuf */
        {                              /* allocParams */
            AIS_BUFLIST_ALLOC_MATCH_INPUT_SIZE,   /* allocType */
            6,  /* maxBuffers */
            0,                        /* matchBuflistId */
            (qcarcam_color_fmt_t)QCARCAM_COLOR_FMT(QCARCAM_BAYER_GRBG, QCARCAM_BITDEPTH_12, QCARCAM_PACK_PLAIN16),   /* fmt */
            0,                        /* width */
            0,                        /* height */
            0,                        /* stride */
            0                         /* align */
        }
    },
    {
        AIS_BUFLIST_OUTPUT_USR,       /* id */
        GetFreeBufDefault,            /* GetFreeBuf */
        ReturnBufDefault,             /* ReturnBuf */
        NULL,                         /* AllocBuf */
        {                             /* allocParams */
            AIS_BUFLIST_ALLOC_NONE,   /* allocType */
            QCARCAM_MAX_NUM_BUFFERS,  /* maxBuffers */
            0,                        /* matchBuflistId */
            (qcarcam_color_fmt_t)0,   /* fmt */
            0,                        /* width */
            0,                        /* height */
            0,                        /* stride */
            0                         /* align */
        }
    },
    {
        AIS_BUFLIST_OUTPUT_JPEG,      /* id */
        GetFreeBufDefault,            /* GetFreeBuf */
        ReturnBufDefault,             /* ReturnBuf */
        BuflistAllocDefault,          /* AllocBuf */
        {                             /* allocParams */
            AIS_BUFLIST_ALLOC_FIXED,  /* allocType */
            5,                        /* maxBuffers */
            1,                        /* matchBuflistId */
            QCARCAM_FMT_MIPIRAW_8,    /* fmt */
            4320,                     /* width */
            1080,                     /* height */
            4576,                     /* stride */
            0                         /* align */
        }
    },
};

static const AisProcChainType ISP_PProc[] =
{
    {
        .id = AIS_PPROC_ISP,
        .pprocFunction = 0,
        .inBuflistId = {AIS_BUFLIST_VFE_RAW_OUT},
        .outBuflistId = {AIS_BUFLIST_OUTPUT_USR, AIS_BUFLIST_OUTPUT_JPEG},
        .pLibName = "",
        .instanceId = 0,
        .numIn = 1,
        .numOut = 2,
        .stepType = AIS_PPROC_STEP_REQUIRED
    },
    {
        .id = AIS_PPROC_USR_DONE,
        .pprocFunction = 0,
        .inBuflistId = {AIS_BUFLIST_OUTPUT_USR},
        .outBuflistId = {},
        .pLibName = "",
        .instanceId = 0,
        .numIn = 1,
        .numOut = 0,
        .stepType = AIS_PPROC_STEP_REQUIRED
    }
};

static const AisProcChainDefType ISP_ProcChainDef =
{
    ISP_PProc,                      /* pProcChain */
    STD_ARRAY_SIZE(ISP_PProc),      /* nProc */
    ISP_Buflist,                    /* pBufferlistDef */
    STD_ARRAY_SIZE(ISP_Buflist),    /* nBuflist */
    1,                              /* nStreams */
    { {AIS_STREAM_TYPE_IFE, AIS_BUFLIST_VFE_RAW_OUT}, {}} /* streams */
};

#endif // Include Paired Input in GHS

#ifdef AIS_PAIRED_INPUT_GPU_SYNC
/**
 * Paired Input usecase with GPU node to sync and stitch
 */
static const AisBuflistDefType PairedInput_Buflist[] =
{
    {
        AIS_BUFLIST_VFE_RAW_OUT,       /* id */
        BuflistDeqIfe,            /* GetFreeBuf */
        BuflistEnqIfe,             /* ReturnBuf */
        BuflistAllocDefault,                         /* AllocBuf */
        {                             /* allocParams */
            AIS_BUFLIST_ALLOC_MATCH_INPUT,   /* allocType */
            9,  /* maxBuffers */
            0,                        /* matchBuflistId */
            (qcarcam_color_fmt_t)0,   /* fmt */
            0,                        /* width */
            0,                        /* height */
            0,                        /* stride */
            0                         /* align */
        }
    },
    {
        AIS_BUFLIST_OUTPUT_USR,       /* id */
        BuflistDeqIfe,            /* GetFreeBuf */
        BuflistEnqIfe,             /* ReturnBuf */
        NULL,                         /* AllocBuf */
        {                             /* allocParams */
            AIS_BUFLIST_ALLOC_NONE,   /* allocType */
            QCARCAM_MAX_NUM_BUFFERS, /* maxBuffers */
            0,                           /* matchBuflistId */
            (qcarcam_color_fmt_t)0,   /* fmt */
            0,                        /* width */
            0,                        /* height */
            0,                        /* stride */
            0                         /* align */
        }
    },
};

static const AisProcChainType PairedInput_PProc[] =
{
    {
        .id = AIS_PPROC_GPU,
        .pprocFunction = AIS_PPROC_GPU_FUNCTION_PAIRED_INPUT,
        .inBuflistId = {AIS_BUFLIST_OUTPUT_USR, AIS_BUFLIST_VFE_RAW_OUT},
        .outBuflistId = {AIS_BUFLIST_OUTPUT_USR},
        .pLibName = "",
        .instanceId = 0,
        .numIn = 2,
        .numOut = 1,
        .stepType = AIS_PPROC_STEP_REQUIRED
    },
    {
        .id = AIS_PPROC_USR_DONE,
        .pprocFunction = 0,
        .inBuflistId = {AIS_BUFLIST_OUTPUT_USR},
        .outBuflistId = {},
        .pLibName = "",
        .instanceId = 0,
        .numIn = 1,
        .numOut = 0,
        .stepType = AIS_PPROC_STEP_REQUIRED
    }
};
#else
/**
 * Paired Input usecase with dual IFE synchronization
 */
static const AisBuflistDefType PairedInput_Buflist[] =
{
    {
        AIS_BUFLIST_OUTPUT_USR,       /* id */
        GetFreeBufDefault,            /* GetFreeBuf */
        BuflistEnqIfe,                /* ReturnBuf */
        NULL,                         /* AllocBuf */
        {                             /* allocParams */
            AIS_BUFLIST_ALLOC_NONE,   /* allocType */
            QCARCAM_MAX_NUM_BUFFERS,  /* maxBuffers */
            0,                        /* matchBuflistId */
            (qcarcam_color_fmt_t)0,   /* fmt */
            0,                        /* width */
            0,                        /* height */
            0,                        /* stride */
            0                         /* align */
        }
    },
};

static const AisProcChainType PairedInput_PProc[] =
{
    {
        .id = AIS_PPROC_FRAMESYNC,
        .pprocFunction = 0,
        .inBuflistId = {AIS_BUFLIST_OUTPUT_USR, AIS_BUFLIST_OUTPUT_USR},
        .outBuflistId = {AIS_BUFLIST_OUTPUT_USR},
        .pLibName = "",
        .instanceId = 0,
        .numIn = 2,
        .numOut = 1,
        .stepType = AIS_PPROC_STEP_REQUIRED
    },
    {
        .id = AIS_PPROC_USR_DONE,
        .pprocFunction = 0,
        .inBuflistId = {AIS_BUFLIST_OUTPUT_USR},
        .outBuflistId = {},
        .pLibName = "",
        .instanceId = 0,
        .numIn = 1,
        .numOut = 0,
        .stepType = AIS_PPROC_STEP_REQUIRED
    }
};
#endif // AIS_PAIRED_INPUT_GPU_SYNC

static const AisProcChainDefType PairedInput_ProcChainDef =
{
    PairedInput_PProc,                     /* pProcChain */
    STD_ARRAY_SIZE(PairedInput_PProc),     /* nProc */
    PairedInput_Buflist,                   /* pBufferlistDef */
    STD_ARRAY_SIZE(PairedInput_Buflist),   /* nBuflist */
    2,                                     /* nStreams */
    {                                      /* streams */
        {AIS_STREAM_TYPE_IFE, AIS_BUFLIST_OUTPUT_USR},
#ifdef AIS_PAIRED_INPUT_GPU_SYNC
        {AIS_STREAM_TYPE_IFE, AIS_BUFLIST_VFE_RAW_OUT}
#else
        {AIS_STREAM_TYPE_IFE, AIS_BUFLIST_OUTPUT_USR}
#endif
    }
};

#if !defined(__INTEGRITY)
/**
 * GPU PPROC bufferlist definitions
 */
static const AisBuflistDefType GPU_PPROC_Buflist[] =
{
    {
        AIS_BUFLIST_VFE_RAW_OUT,      /* id */
        BuflistDeqIfe,               /* GetFreeBuf */
        BuflistEnqIfe,                /* ReturnBuf */
        BuflistAllocDefault,          /* AllocBuf */
        {                             /* allocParams */
            AIS_BUFLIST_ALLOC_MATCH_INPUT,   /* allocType */
            5,  /* maxBuffers */
            0,                        /* matchBuflistId */
            (qcarcam_color_fmt_t)0,   /* fmt */
            0,                        /* width */
            0,                        /* height */
            0,                        /* stride */
            64                        /* align */
        }
    },
    {
        AIS_BUFLIST_OUTPUT_USR,       /* id */
        GetFreeBufDefault,            /* GetFreeBuf */
        ReturnBufDefault,             /* ReturnBuf */
        NULL,                         /* AllocBuf */
        {                             /* allocParams */
            AIS_BUFLIST_ALLOC_NONE,   /* allocType */
            QCARCAM_MAX_NUM_BUFFERS,  /* maxBuffers */
            0,                        /* matchBuflistId */
            (qcarcam_color_fmt_t)0,   /* fmt */
            0,                        /* width */
            0,                        /* height */
            0,                        /* stride */
            0                         /* align */
        }
    },
};

/**
 * Deinterlace usecase
 */
static const AisProcChainType Deinterlace_PProc[] =
{
    {
        .id = AIS_PPROC_GPU,
        .pprocFunction = AIS_PPROC_GPU_FUNCTION_DEINTERLACE,
        .inBuflistId = {AIS_BUFLIST_VFE_RAW_OUT},
        .outBuflistId = {AIS_BUFLIST_OUTPUT_USR},
        .pLibName = "",
        .instanceId = 0,
        .numIn = 1,
        .numOut = 1,
        .stepType = AIS_PPROC_STEP_REQUIRED
    },
    {
        .id = AIS_PPROC_USR_DONE,
        .pprocFunction = 0,
        .inBuflistId = {AIS_BUFLIST_OUTPUT_USR},
        .outBuflistId = {},
        .pLibName = "",
        .instanceId = 0,
        .numIn = 1,
        .numOut = 0,
        .stepType = AIS_PPROC_STEP_REQUIRED
    }
};

static const AisProcChainDefType DeinterlaceProcChainDef =
{
    Deinterlace_PProc,                     /* pProcChain */
    STD_ARRAY_SIZE(Deinterlace_PProc),     /* nProc */
    GPU_PPROC_Buflist,                     /* pBufferlistDef */
    STD_ARRAY_SIZE(GPU_PPROC_Buflist),     /* nBuflist */
    1,                                     /* nStreams */
    { {AIS_STREAM_TYPE_IFE, AIS_BUFLIST_VFE_RAW_OUT}, {}} /* streams */
};

/**
 * QCARCAM_OPMODE_TRANSFORMER usecase
 */

static const AisProcChainType Transformer_PProc[] =
{
    {
        .id = AIS_PPROC_GPU,
        .pprocFunction = AIS_PPROC_GPU_FUNCTION_TRANSFORMER,
        .inBuflistId = {AIS_BUFLIST_VFE_RAW_OUT},
        .outBuflistId = {AIS_BUFLIST_OUTPUT_USR},
        .pLibName = "",
        .instanceId = 0,
        .numIn = 1,
        .numOut = 1,
        .stepType = AIS_PPROC_STEP_REQUIRED
    },
    {
        .id = AIS_PPROC_USR_DONE,
        .pprocFunction = 0,
        .inBuflistId = {AIS_BUFLIST_OUTPUT_USR},
        .outBuflistId = {},
        .pLibName = "",
        .instanceId = 0,
        .numIn = 1,
        .numOut = 0,
        .stepType = AIS_PPROC_STEP_REQUIRED
    }
};

static const AisProcChainDefType TransformerProcChainDef =
{
    Transformer_PProc,                     /* pProcChain */
    STD_ARRAY_SIZE(Transformer_PProc),     /* nProc */
    GPU_PPROC_Buflist,                     /* pBufferlistDef */
    STD_ARRAY_SIZE(GPU_PPROC_Buflist),     /* nBuflist */
    1,                                     /* nStreams */
    { {AIS_STREAM_TYPE_IFE, AIS_BUFLIST_VFE_RAW_OUT}, {}} /* streams */
};

/**
 * QCARCAM_OPMODE_RGBIR usecase
 */
static const AisBuflistDefType RGBIR_PPROC_Buflist[] =
{
    {
        AIS_BUFLIST_VFE_RAW_OUT,            /* id */
        BuflistDeqIfe,                     /* GetFreeBuf */
        BuflistEnqIfe,                      /* ReturnBuf */
        BuflistAllocDefault,                /* AllocBuf */
        {                                   /* allocParams */
            AIS_BUFLIST_ALLOC_MATCH_INPUT_SIZE, /* allocType */
            6,                              /* maxBuffers */
            // a minimum of 5 buffers is required:
            //- 3 for CamX (one for each node that requires a buffer: SHDR, BPS and IPE)
            //- 2 for the IFE
            0,                              /* matchBuflistId */
            (qcarcam_color_fmt_t)QCARCAM_COLOR_FMT(QCARCAM_BAYER_GRBG, QCARCAM_BITDEPTH_12, QCARCAM_PACK_PLAIN16), /* fmt */
            0,                              /* width */
            0,                              /* height */
            0,                              /* stride */
            0                               /* align */
        }
    },
    {
        /*bayer output of RGBIR extract*/
        AIS_BUFLIST_PPROC_1,                /* id */
        GetFreeBufDefault,                  /* GetFreeBuf */
        ReturnBufDefault,                   /* ReturnBuf */
        BuflistAllocDefault,                /* AllocBuf */
        {                                   /* allocParams */
            AIS_BUFLIST_ALLOC_MATCH_BUFLIST, /* allocType */
            5,                              /* maxBuffers */
            AIS_BUFLIST_VFE_RAW_OUT,        /* matchBuflistId */
            (qcarcam_color_fmt_t)0,         /* fmt */
            0,                              /* width */
            0,                              /* height */
            0,                              /* stride */
            64                              /* align */
        }
    },
    {
        /*IR output of RGBIR extract*/
        AIS_BUFLIST_PPROC_2,                /* id */
        GetFreeBufDefault,                  /* GetFreeBuf */
        ReturnBufDefault,                   /* ReturnBuf */
        BuflistAllocDefault,                /* AllocBuf */
        {                                   /* allocParams */
            AIS_BUFLIST_ALLOC_MATCH_BUFLIST, /* allocType */
            5,                              /* maxBuffers */
            AIS_BUFLIST_VFE_RAW_OUT,        /* matchBuflistId */
            (qcarcam_color_fmt_t)0,         /* fmt */
            0,                              /* width */
            0,                              /* height */
            0,                              /* stride */
            64                              /* align */
        }
    },
    {
        AIS_BUFLIST_USR_0,                  /* id */
        GetFreeBufDefault,                  /* GetFreeBuf */
        ReturnBufDefault,                   /* ReturnBuf */
        NULL,                               /* AllocBuf */
        {                                   /* allocParams */
            AIS_BUFLIST_ALLOC_NONE,         /* allocType */
            QCARCAM_MAX_NUM_BUFFERS,        /* maxBuffers */
            0,                              /* matchBuflistId */
            (qcarcam_color_fmt_t)0,         /* fmt */
            0,                              /* width */
            0,                              /* height */
            0,                              /* stride */
            0                               /* align */
        }
    },
    {
        AIS_BUFLIST_USR_1,                  /* id */
        GetFreeBufDefault,                  /* GetFreeBuf */
        ReturnBufDefault,                   /* ReturnBuf */
        NULL,                               /* AllocBuf */
        {                                   /* allocParams */
            AIS_BUFLIST_ALLOC_NONE,         /* allocType */
            QCARCAM_MAX_NUM_BUFFERS,        /* maxBuffers */
            0,                              /* matchBuflistId */
            (qcarcam_color_fmt_t)0,         /* fmt */
            0,                              /* width */
            0,                              /* height */
            0,                              /* stride */
            0                               /* align */
        }
    },
    {
        AIS_BUFLIST_OUTPUT_JPEG,            /* id */
        GetFreeBufDefault,                  /* GetFreeBuf */
        ReturnBufDefault,                   /* ReturnBuf */
        BuflistAllocDefault,                /* AllocBuf */
        {                                   /* allocParams */
            AIS_BUFLIST_ALLOC_FIXED,        /* allocType */
            5,                              /* maxBuffers */
            1,                              /* matchBuflistId */
            //Allocate enough buffer to align with CAMX Plane size calcuation for JPEG format
            QCARCAM_FMT_MIPIRAW_8,          /* fmt */
            4320,                           /* width */
            1080,                           /* height */
            4576,                           /* stride */
            0                               /* align */
        }
    },
};

static const AisProcChainType RGBIR_PProc[] =
{
    {
        .id = AIS_PPROC_EXT,
        .pprocFunction = 0,
        .inBuflistId = {AIS_BUFLIST_VFE_RAW_OUT},
        .outBuflistId = {AIS_BUFLIST_PPROC_1, AIS_BUFLIST_PPROC_2},
        .pLibName = "libpproc_rgbir.so",
        .instanceId = 0,
        .numIn = 1,
        .numOut = 2,
        .stepType = AIS_PPROC_STEP_REQUIRED
    },
    {
        /*BAYER - RGB*/
        .id = AIS_PPROC_ISP,
        .pprocFunction = 0,
        .inBuflistId = {AIS_BUFLIST_PPROC_1},
        .outBuflistId = {AIS_BUFLIST_USR_0, AIS_BUFLIST_OUTPUT_JPEG},
        .pLibName = NULL,
        .instanceId = 0,
        .numIn = 1,
        .numOut = 2,
        .stepType = AIS_PPROC_STEP_OPTIONAL_INPUT_BUFFERS
    },
    {
        /*IR*/
        .id = AIS_PPROC_ISP,
        .pprocFunction = 0,
        .inBuflistId = {AIS_BUFLIST_PPROC_2},
        .outBuflistId = {AIS_BUFLIST_USR_1},
        .pLibName = NULL,
        .instanceId = 1,
        .numIn = 1,
        .numOut = 1,
        .stepType = AIS_PPROC_STEP_OPTIONAL_INPUT_BUFFERS
    },
    {
        /*USRDONE - BAYER*/
        .id = AIS_PPROC_USR_DONE,
        .pprocFunction = 0,
        .inBuflistId = {AIS_BUFLIST_USR_0},
        .outBuflistId = {},
        .pLibName = NULL,
        .instanceId = 0,
        .numIn = 1,
        .numOut = 0,
        .stepType = AIS_PPROC_STEP_OPTIONAL_INPUT_BUFFERS
    },
    {
        /*USRDONE - IR*/
        .id = AIS_PPROC_USR_DONE,
        .pprocFunction = 0,
        .inBuflistId = {AIS_BUFLIST_USR_1},
        .outBuflistId = {},
        .pLibName = NULL,
        .instanceId = 1,
        .numIn = 1,
        .numOut = 0,
        .stepType = AIS_PPROC_STEP_OPTIONAL_INPUT_BUFFERS
    }
};

static const AisProcChainDefType RGBIRProcChainDef =
{
    RGBIR_PProc,                            /* pProcChain */
    STD_ARRAY_SIZE(RGBIR_PProc),            /* nProc */
    RGBIR_PPROC_Buflist,                    /* pBuffer */
    STD_ARRAY_SIZE(RGBIR_PPROC_Buflist),    /* nBuflist */
    1,                                      /* nStreams */
    { {AIS_STREAM_TYPE_IFE, AIS_BUFLIST_VFE_RAW_OUT}, {}} /* streams */
};
#endif //__INTEGRITY

/**
 * Proc Chain Definitions
 */
#if !defined(__INTEGRITY)
static const AisProcChainDefType* g_ProcChainDefs[QCARCAM_OPMODE_MAX] =
{
    &RawdumpProcChainDef,       /* QCARCAM_OPMODE_RAW_DUMP */
    &ISP_ProcChainDef,          /* QCARCAM_OPMODE_SHDR */
    &InjectProcChainDef,        /* QCARCAM_OPMODE_INJECT */
    &PairedInput_ProcChainDef,  /* QCARCAM_OPMODE_PAIRED_INPUT */
    &DeinterlaceProcChainDef,   /* QCARCAM_OPMODE_DEINTERLACE */
    &TransformerProcChainDef,   /* QCARCAM_OPMODE_TRANSFORMER */
    &RGBIRProcChainDef,         /* QCARCAM_OPMODE_RGBIR */
    &ISP_ProcChainDef,          /* QCARCAM_OPMODE_ISP */
    &DualStreamRawdumpProcChainDef, /*QCARCAM_OPMODE_2_STREAMS*/
    &PairedInput_ProcChainDef, /* QCARCAM_OPMODE_RDI_CONVERSION */
};
#else
static const AisProcChainDefType* g_ProcChainDefs[QCARCAM_OPMODE_MAX] =
{
    &RawdumpProcChainDef,       /* QCARCAM_OPMODE_RAW_DUMP */
    nullptr,                    /* QCARCAM_OPMODE_SHDR */
    nullptr,                    /* QCARCAM_OPMODE_INJECT */
    &PairedInput_ProcChainDef,  /* QCARCAM_OPMODE_PAIRED_INPUT */
};
#endif

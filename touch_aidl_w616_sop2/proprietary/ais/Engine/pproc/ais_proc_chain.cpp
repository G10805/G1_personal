/*!
 * Copyright (c) 2016-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ais_i.h"
#include "ais_engine.h"
#include "ais_ife_configurer.h"
#include "ais_proc_chain_def.h"
#include "ais_pproc_framesync.h"

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

//////////////////////////////////////////////////////////////////////////////////
/// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
/// FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////
class AisProcChainManagerPrivate : public AisProcChainManager
{
public:
    AisProcChainManagerPrivate() {
        memset(m_pPProc, 0x0, sizeof(m_pPProc));
    }

    CameraResult Init(void);
    CameraResult Deinit(void);

    virtual CameraResult GetProcChain(AisUsrCtxt* pUsrCtxt);
    virtual CameraResult ReleaseProcChain(AisUsrCtxt* pUsrCtxt);

    virtual AisPProc* GetPProc(AisPProcIdType id)
    {
        return (id < AIS_PPROC_MAX) ? m_pPProc[id] : nullptr;
    }

private:
    AisPProc* m_pPProc[AIS_PPROC_MAX];
};


///@brief AisProcChainManager singleton
AisProcChainManager* AisProcChainManager::m_pProcChainManagerInstance = nullptr;

AisProcChainManager* AisProcChainManager::CreateInstance()
{
    if(m_pProcChainManagerInstance == nullptr)
    {
        m_pProcChainManagerInstance = new AisProcChainManagerPrivate();
        if (m_pProcChainManagerInstance)
        {
            CameraResult rc = ((AisProcChainManagerPrivate*)m_pProcChainManagerInstance)->Init();
            if (rc)
            {
                DestroyInstance();
            }
        }
    }

    return m_pProcChainManagerInstance;
}

AisProcChainManager* AisProcChainManager::GetInstance()
{
    return m_pProcChainManagerInstance;
}

void AisProcChainManager::DestroyInstance()
{
    if(m_pProcChainManagerInstance != nullptr)
    {
        ((AisProcChainManagerPrivate*)m_pProcChainManagerInstance)->Deinit();

        delete m_pProcChainManagerInstance;
        m_pProcChainManagerInstance = nullptr;
    }
}

/**
 * Returns number of streams for an OPMODE
 */
CameraResult AisProcChainManager::GetNumStreams(qcarcam_opmode_type opMode, uint32* pNumStreams)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 i;

    if (opMode >= QCARCAM_OPMODE_MAX)
    {
        AIS_LOG(PPROC_MGR, ERROR, "Invalid operation mode %d", opMode);
        rc = CAMERA_EFAILED;
    }
    else
    {
        if (g_ProcChainDefs[opMode] == NULL)
        {
            AIS_LOG(PPROC_MGR, ERROR, "No proc chain assigned to opMode %d", opMode);
            rc = CAMERA_EFAILED;
        }
        else
        {
            *pNumStreams = g_ProcChainDefs[opMode]->nStreams;
        }
    }

    return rc;
}

CameraResult AisProcChainManagerPrivate::Init(void)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 i;

    m_pPProc[AIS_PPROC_USR_DONE] = AisPProcUsrDone::Create();
    m_pPProc[AIS_PPROC_FRAMESYNC] = AisPprocFrameSync::Create();
#if defined(__INTEGRITY)
    if (!m_pPProc[AIS_PPROC_USR_DONE] || !m_pPProc[AIS_PPROC_FRAMESYNC])
    {
        AIS_LOG(PPROC_MGR, FATAL, "Failed to create AIS_PPROC_USR_DONE or AIS_PPROC_FRAMESYNC");
        rc = CAMERA_EFAILED;
    }
#else
    m_pPProc[AIS_PPROC_ISP] = AisPProcIsp::Create();
    m_pPProc[AIS_PPROC_GPU] = AisPProcGpu::Create();
    m_pPProc[AIS_PPROC_MEMCPY] = AisPProcMemcpy::Create();
    m_pPProc[AIS_PPROC_RGBIR] = AisPProcRgbIR::Create();

    for (i = 0; i < AIS_PPROC_MAX; i++)
    {
        if (AIS_PPROC_EXT == i)
        {
            continue;
        }

        if (!m_pPProc[i])
        {
            AIS_LOG(PPROC_MGR, FATAL, "Failed to create PPROC %d", i);
            rc = CAMERA_EFAILED;
            break;
        }
    }
#endif
    return rc;
}

CameraResult AisProcChainManagerPrivate::Deinit(void)
{
    AisPProcUsrDone::Destroy();
    AisPprocFrameSync::Destroy();
#if !defined(__INTEGRITY)
    AisPProcIsp::Destroy();
    AisPProcGpu::Destroy();
    AisPProcMemcpy::Destroy();
    AisPProcRgbIR::Destroy();
#endif

    return CAMERA_SUCCESS;
}

/**
 * Copies proc chain definition to a user context
 */
CameraResult AisProcChainManagerPrivate::GetProcChain(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (pUsrCtxt->m_opMode >= QCARCAM_OPMODE_MAX)
    {
        AIS_LOG(PPROC_MGR, ERROR, "Invalid operation mode %d", pUsrCtxt->m_opMode);
        rc = CAMERA_EFAILED;
    }
    else
    {
        if (g_ProcChainDefs[pUsrCtxt->m_opMode] == NULL)
        {
            AIS_LOG(PPROC_MGR, ERROR, "No proc chain assigned to opMode %d", pUsrCtxt->m_opMode);
            rc = CAMERA_EUNSUPPORTED;
        }
        else if (g_ProcChainDefs[pUsrCtxt->m_opMode]->nProc > AIS_PPROC_MAX_NUM_PROCS)
        {
            AIS_LOG(PPROC_MGR, ERROR, "proc chain for opMode %d exceeds max num procs", pUsrCtxt->m_opMode);
            rc = CAMERA_EFAILED;
        }
        else
        {
            pUsrCtxt->m_pProcChainDef = g_ProcChainDefs[pUsrCtxt->m_opMode];
            pUsrCtxt->m_numStreams = STD_MIN(pUsrCtxt->m_pProcChainDef->nStreams, AIS_USER_CTXT_MAX_STREAMS);

            for (uint32 i = 0; i < pUsrCtxt->m_numStreams; i++)
            {
                pUsrCtxt->m_streams[i].type = pUsrCtxt->m_pProcChainDef->streams[i].type;
            }

            //fill pNode
            for (uint32 i = 0; i < pUsrCtxt->m_pProcChainDef->nProc; i++)
            {
                AisPProcIdType      id = pUsrCtxt->m_pProcChainDef->pProcChain[i].id;
#if !defined(__INTEGRITY)
                if (AIS_PPROC_EXT == id)
                {
                    pUsrCtxt->m_pPProc[i] = AisPProcNode::Create();
                }
                else
#endif
                {
                    pUsrCtxt->m_pPProc[i] = m_pPProc[id];
                }

                if (!pUsrCtxt->m_pPProc[i])
                {
                    AIS_LOG(PPROC_MGR, ERROR, "proc chain node%d (id %d) for opMode %d not available",
                            i, id, pUsrCtxt->m_opMode);
                    rc = CAMERA_EFAILED;
                    break;
                }
            }
        }
    }

    return rc;
}

/**
 * Copies proc chain definition to a user context
 */
CameraResult AisProcChainManagerPrivate::ReleaseProcChain(AisUsrCtxt* pUsrCtxt)
{
    //fill pNode
    for (uint32 i = 0; i < pUsrCtxt->m_pProcChainDef->nProc; i++)
    {
        if (AIS_PPROC_EXT == pUsrCtxt->m_pProcChainDef->pProcChain[i].id)
        {
#if !defined(__INTEGRITY)
            ((AisPProcNode*)pUsrCtxt->m_pPProc[i])->Destroy();
#endif
        }
        else
        {
            pUsrCtxt->m_pPProc[i] = NULL;
        }
    }

    pUsrCtxt->m_pProcChainDef = NULL;
    memset(pUsrCtxt->m_pPProc, 0x0, sizeof(pUsrCtxt->m_pPProc));

    return CAMERA_SUCCESS;
}


/**
 * GetBufferDefault
 *
 * @brief Default get_buf implementation. Find first available buffer index and return it
 *
 * @param pUsrCtxt
 * @param pBufferList
 * @param [out] idx - available buffer index
 *
 * @return CameraResult
 */
static AisBuffer* GetFreeBufDefault(AisUsrCtxt* pUsrCtxt, AisBufferList* pBufferList)
{
    AisBuffer* pBuffer = NULL;
    uint32 i = 0;

    pBufferList->Lock();
    for (i = 0; i < pBufferList->m_nBuffers; i++)
    {
        if (pBufferList->m_pBuffers[i].state == AIS_BUFFER_INITIALIZED)
        {
            pBufferList->m_pBuffers[i].state = AIS_USER_ACQUIRED;
            pBuffer = &pBufferList->m_pBuffers[i];
            break;
        }
    }
    pBufferList->Unlock();

    return pBuffer;
}

/**
 * PutBufferDefault
 *
 * @brief Default put_buf implementation. Set buffer to available
 *
 * @param pUsrCtxt
 * @param pBufferList
 * @param idx - buffer index
 *
 * @return CameraResult
 */
static CameraResult ReturnBufDefault(AisUsrCtxt* pUsrCtxt, AisBufferList* pBufferList, uint32 idx)
{
    return pBufferList->SetBufferState(idx, AIS_BUFFER_INITIALIZED);
}

/**
 * BuflistDeqIfe
 *
 * @return CameraResult
 */
static AisBuffer* BuflistDeqIfe(AisUsrCtxt* pUsrCtxt, AisBufferList* pBufferList)
{
    return NULL;
}

/**
 * BuflistEnqIfe
 *
 * @brief Enqueue buffer to VFE
 *
 * @return CameraResult
 */
static CameraResult BuflistEnqIfe(AisUsrCtxt* pUsrCtxt, AisBufferList* pBufferList, uint32 idx)
{
    CameraResult rc = CAMERA_EBADSTATE;

    AisBuffer* pBuffer = pBufferList->GetBuffer(idx);
    if (!pBuffer)
    {
        AIS_LOG(PPROC_MGR, ERROR, "No Buffer %d in buffer list %d", idx, pBufferList->GetId());
        return CAMERA_EBADPARM;
    }

    AisBufferStateType current_state = pBuffer->state;
    if (current_state == AIS_USER_ACQUIRED || current_state == AIS_BUFFER_INITIALIZED)
    {
        rc = AisIFEConfigurer::GetInstance()->QueueBuffer(pUsrCtxt, pBufferList->GetId(), pBuffer);
        if( rc != CAMERA_SUCCESS)
        {
            AIS_LOG(PPROC_MGR, ERROR, "queue_buffer fail %u", idx);
        }
    }
    else
    {
        AIS_LOG(PPROC_MGR, ERROR, "buffer %d is not in good state %u", idx, current_state);
    }

    return rc;
}

/**
 * get_fmt_component_pp
 *
 * @brief Helper function that returns number of channels components per pixel
 * for a qcarcam color format.
 *
 * @param fmt - qcarcam_color_fmt_t
 *
 * @return number of color channels per pixel
 */
static uint32 get_fmt_component_pp(qcarcam_color_fmt_t fmt)
{
    qcarcam_color_pattern_t pattern = (qcarcam_color_pattern_t)QCARCAM_COLOR_GET_PATTERN(fmt);
    uint32 component_pp = 1;

    switch(pattern)
    {
        case QCARCAM_RAW:
        case QCARCAM_BAYER_GBRG:
        case QCARCAM_BAYER_GRBG:
        case QCARCAM_BAYER_RGGB:
        case QCARCAM_BAYER_BGGR:
        {
            component_pp = 1;
            break;
        }
        case QCARCAM_YUV_YUYV:
        case QCARCAM_YUV_YVYU:
        case QCARCAM_YUV_UYVY:
        case QCARCAM_YUV_VYUY:
        {
            component_pp = 2;
            break;
        }
        case QCARCAM_RGB_RGB888:
        case QCARCAM_RGB_BGR888:
        {
            component_pp = 3;
            break;
        }
        default:
            AIS_LOG(PPROC_MGR, ERROR, "unsupported color format %d use default 1 byte pp", fmt);
            break;
    }

    return component_pp;
}

/**
 * get_bytes_per_line
 *
 * @brief Number of bytes per line for a qcarcam color format and width.
 *
 * @param width
 * @param fmt - qcarcam_color_fmt_t
 *
 * @return number of bytes per line
 */
static uint32 get_bytes_per_line(uint32 width, qcarcam_color_fmt_t fmt)
{
    qcarcam_color_bitdepth_t bitdepth = (qcarcam_color_bitdepth_t)QCARCAM_COLOR_GET_BITDEPTH(fmt);
    qcarcam_color_pack_t pack = QCARCAM_COLOR_GET_PACK(fmt);
    uint32 M, N;

    if (QCARCAM_PACK_MIPI == pack)
    {
    switch(bitdepth)
    {
        case QCARCAM_BITDEPTH_8:
        {
            M = 1;
            N = 1;
            break;
        }
        case QCARCAM_BITDEPTH_10:
        {
            M = 5;
            N = 4;
            break;
        }
        case QCARCAM_BITDEPTH_12:
        {
            M = 3;
            N = 2;
            break;
        }
        case QCARCAM_BITDEPTH_14:
        {
            M = 7;
            N = 4;
            break;
        }
        case QCARCAM_BITDEPTH_16:
        {
            M = 2;
            N = 1;
            break;
        }
        case QCARCAM_BITDEPTH_20:
        {
            M = 5;
            N = 2;
            break;
        }
        default:
            AIS_LOG(PPROC_MGR, ERROR, "unsupported color format 0x%x use default MIPI8", fmt);
            M = 1;
            N = 1;
            break;
    }

        return (width * get_fmt_component_pp(fmt) * M + (N-1)) / N;
    }
    else if (QCARCAM_PACK_PLAIN8 == pack)
    {
        return width * get_fmt_component_pp(fmt);
    }
    else if (QCARCAM_PACK_PLAIN16 == pack)
    {
        //2Bytes per pixel
        return width * get_fmt_component_pp(fmt) * 2;
    }
    else if (QCARCAM_PACK_PLAIN32 == pack)
    {
        //4Bytes per pixel
        return width * get_fmt_component_pp(fmt) * 4;
    }
    else if (QCARCAM_PACK_FOURCC == pack)
    {
        qcarcam_color_pattern_t pattern = QCARCAM_COLOR_GET_PATTERN(fmt);
        if (pattern >= QCARCAM_RGB)
        {
            return width * bitdepth / 8;
        }
        else
        {
            return width * get_fmt_component_pp(fmt);
        }
    }

    AIS_LOG(PPROC_MGR, ERROR, "Unsupported color format 0x%x", fmt);
    return width * get_fmt_component_pp(fmt);
}


static CameraResult BuflistAllocDefault(AisUsrCtxt* pUsrCtxt, const AisBuflistDefType* pBuflistDef)
{
    CameraResult rc = CAMERA_SUCCESS;
    CameraHwBlockType hwBlock = CAMERA_HWBLOCK_IFE;
    AisBufferList* pBufferList = pUsrCtxt->m_bufferList[pBuflistDef->id];
    AisBuflistAllocParams  allocParams = pBuflistDef->allocParams;
    uint32 i = 0;
    int size = 0;

    switch (allocParams.allocType)
    {
    case AIS_BUFLIST_ALLOC_MATCH_INPUT:
    case AIS_BUFLIST_ALLOC_MATCH_INPUT_SIZE:
    {
        AisInputModeInfoType* pInputModeInfo = NULL;

        for (uint32 stream = 0; stream < pUsrCtxt->m_numStreams; stream++)
        {
            if (pUsrCtxt->m_streams[stream].resources.ifeStream.bufferListIdx == pBuflistDef->id)
            {
                pInputModeInfo = &pUsrCtxt->m_streams[stream].inputCfg.inputModeInfo;
            }
        }

        if (!pInputModeInfo)
        {
            AIS_LOG(PPROC_MGR, HIGH, "Could not find ife stream for bufferlist %d, will match stream 0", pBuflistDef->id);
            pInputModeInfo = &pUsrCtxt->m_streams[0].inputCfg.inputModeInfo;
        }

        if (AIS_BUFLIST_ALLOC_MATCH_INPUT == allocParams.allocType)
        {
            allocParams.fmt = pInputModeInfo->fmt;
        }

        allocParams.width = pInputModeInfo->width;
        allocParams.height = pInputModeInfo->height;
        allocParams.stride = CAM_ALIGN_SIZE(get_bytes_per_line(allocParams.width, allocParams.fmt), allocParams.align);

        break;
    }
    case AIS_BUFLIST_ALLOC_MATCH_BUFLIST:
    {
        if (allocParams.matchBuflistId < AIS_BUFLIST_MAX)
        {
            AisBufferList* pMatchBufferList = pUsrCtxt->m_bufferList[allocParams.matchBuflistId];
            if (pMatchBufferList)
            {
                allocParams.fmt = pMatchBufferList->GetColorFmt();
                allocParams.width = pMatchBufferList->GetWidth();
                allocParams.height = pMatchBufferList->GetHeight();
                allocParams.stride = CAM_ALIGN_SIZE(get_bytes_per_line(allocParams.width, allocParams.fmt), allocParams.align);
            }
            else
            {
                rc = CAMERA_EBADPARM;
            }
        }
        else
        {
            rc = CAMERA_EBADPARM;
        }
        break;
    }
    case AIS_BUFLIST_ALLOC_FIXED:
    {
        /*Do nothing*/
        break;
    }
    case AIS_BUFLIST_ALLOC_QUERY:
    default:
        rc = CAMERA_EBADPARM;
        break;
    }

    if (rc != CAMERA_SUCCESS)
    {
        AIS_LOG(PPROC_MGR, ERROR, "op_mode(%d) bufferlist(%d) failed(%d) to parse internal params",
            pUsrCtxt->m_opMode, pBuflistDef->id, rc);
        return rc;
    }

    size = allocParams.height * allocParams.stride;
    uint32 numBuffers = pBufferList->GetMaxNumBuffers();

    AIS_LOG(PPROC_MGR, HIGH, "fmt 0x%08x stride %d width %d height %d",
        allocParams.fmt, allocParams.stride, allocParams.width, allocParams.height);

    for (i = 0; i < numBuffers; i++)
    {
        AisBuffer* pBuffer = &pBufferList->m_pBuffers[i];

        pBuffer->bufferInfo.planes[0].width = allocParams.width;
        pBuffer->bufferInfo.planes[0].height = allocParams.height;
        pBuffer->bufferInfo.planes[0].stride = allocParams.stride;
        pBuffer->bufferInfo.planes[0].size = size;
        pBuffer->bufferInfo.n_planes = 1;

        pBuffer->idx = i;
        pBuffer->size = size;

#if defined(__ANDROID__)|| defined(__AGL__)
        rc = CameraBufferAlloc(pBuffer,
            CAMERA_BUFFER_FLAG_CONTIGOUS,
            CAMERA_BUFFER_ALIGN_4K,
            &hwBlock, 1);
#else
        rc = CameraBufferAlloc(pBuffer,
            CAMERA_BUFFER_FLAG_CONTIGOUS,
            CAMERA_BUFFER_ALIGN_4K,
            NULL, 0);

        if (CAMERA_SUCCESS == rc)
        {
            rc = CameraBufferMap(
                pBuffer,
                CAMERA_BUFFER_FLAG_HNDL,
                0x0,
                &hwBlock,
                1);

            if (CAMERA_SUCCESS == rc)
            {
                pBuffer->pDa = CameraBufferGetDeviceAddress(pBuffer, hwBlock);
            }
            else
            {
                CameraBufferFree(pBuffer);
            }
        }
#endif
        if (CAMERA_SUCCESS == rc)
        {
            pBuffer->state = AIS_BUFFER_INITIALIZED;
        }
        else
        {
            while (i > 0)
            {
                i--;
                CameraBufferFree(&pBufferList->m_pBuffers[i]);
                memset(&pBufferList->m_pBuffers[i], 0x0, sizeof(pBufferList->m_pBuffers[i]));
            }
            pBufferList->m_nBuffers = 0;
            break;
        }
    }

    if (CAMERA_SUCCESS == rc)
    {
        pBufferList->SetProperties(numBuffers, allocParams.width, allocParams.height, allocParams.fmt);

        AIS_LOG(PPROC_MGR, LOW, "op_mode %d allocated %d for internal bufferlist rc = %d",
            pUsrCtxt->m_opMode, pBufferList->m_nBuffers, rc);
    }
    else
    {
        AIS_LOG(PPROC_MGR, ERROR, "failed to alloc for op_mode %d internal bufferlist %d rc = %d",
            pUsrCtxt->m_opMode, pBuflistDef->id, rc);
    }

    return rc;
}


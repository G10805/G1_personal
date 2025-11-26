/*!
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ais_pproc_gpu.h"

#include <stdio.h>
#if defined(__ANDROID__)|| defined(__AGL__)
#include <cutils/native_handle.h>
#endif
#include "c2d2.h"

#include "ais_log.h"
#include "ais_engine.h"
#include "ais_ife_configurer.h"

#include "CameraPlatform.h"

//////////////////////////////////////////////////////////////////////////////////
/// MACRO DEFINITIONS
//////////////////////////////////////////////////////////////////////////////////
#define PPROC_LOG(lvl, fmt...) AIS_LOG(PPROC_GPU, lvl, fmt)

#if defined(__ANDROID__)|| defined(__AGL__)
#define KGSL_USER_MEM_TYPE_ION 3
#endif

/*from experiment most frames were within 50us of each other so 500 should be sufficiently large*/
#define EPSILON_QTIME 501

//////////////////////////////////////////////////////////////////////////////////
/// TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
/// FORWARD DECLARE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
/// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////////
AisPProcGpu* AisPProcGpu::m_pNodeInstance = nullptr;

//////////////////////////////////////////////////////////////////////////////////
/// FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////
AisPProcGpu::AisPProcGpu()
{
    m_isC2dInitialized = FALSE;
    m_gpuMutex = NULL;
    m_gpuEventHandlerTid = NULL;
}

/**
 * ~AisPProcGpu
 *
 * @brief Destructor
 */
AisPProcGpu::~AisPProcGpu()
{
    if (m_isC2dInitialized)
    {
        c2dDriverDeInit();
    }
}

/**
 * Create
 *
 * @brief Creates an instance of AisPProcGpu
 *
 * @param None
 *
 * @return m_pNodeInstance
 */
AisPProcGpu* AisPProcGpu::Create()
{
    CameraResult rc = CAMERA_SUCCESS;
    char name[64];
    AisPProcGpu* pProcGpu = new AisPProcGpu();
    /*create the thread for the gpu handler*/
    snprintf(name, sizeof(name), "ais_gpu_procthread");

    if (CAMERA_SUCCESS == rc)
    {
        rc = CameraCreateMutex(&pProcGpu->m_gpuMutex);
        AIS_LOG_ON_ERR(PPROC_GPU, rc, "Failed to create gpu mutex: %d", rc);
    }

    if (CAMERA_SUCCESS == rc)
    {
        rc = CameraCreateThread(CAMERA_THREAD_PRIO_HIGH_REALTIME,
                0,
                GPUEventHandler,
                (void*)pProcGpu,
                0,
                name,
                &pProcGpu->m_gpuEventHandlerTid);
        AIS_LOG_ON_ERR(PPROC_GPU, rc, "CameraCreateThread failed: %d", rc);
    }

    if (CAMERA_SUCCESS == rc)
    {
        m_pNodeInstance = pProcGpu;
    }
    else
    {
        if (pProcGpu->m_gpuMutex)
        {
            CameraDestroyMutex(pProcGpu->m_gpuMutex);
            pProcGpu->m_gpuMutex = NULL;
        }
        delete pProcGpu;
        m_pNodeInstance = NULL;
    }

    return m_pNodeInstance;
}

/**
 * Destroy
 *
 * @brief Destroy Node
 *
 * @param None
 *
 * @return None
 */
void AisPProcGpu::Destroy(void)
{
    if (m_pNodeInstance)
    {
        if (m_pNodeInstance->m_gpuEventHandlerTid)
        {
            CameraJoinThread(m_pNodeInstance->m_gpuEventHandlerTid, NULL);
            CameraReleaseThread(m_pNodeInstance->m_gpuEventHandlerTid);
        }
        if (m_pNodeInstance->m_gpuMutex)
        {
            CameraDestroyMutex(m_pNodeInstance->m_gpuMutex);
            m_pNodeInstance->m_gpuMutex = NULL;
        }
        delete m_pNodeInstance;
        m_pNodeInstance = nullptr;
    }
}

/**
 * AddSession
 *
 * @brief Add session to map
 *
 * @param pUsrCtxt
 * @param pSession
 *
 * @return CameraResult
 */
CameraResult AisPProcGpu::AddSession(AisUsrCtxt* pUsrCtxt, AisPProcGpuSession* pSession)
{
    CameraResult rc = CAMERA_SUCCESS;

    std::pair<AisPProcGpuSessionMap::iterator, bool> ret;
    ret = m_sessionMap.insert(std::pair<AisUsrCtxt*, AisPProcGpuSession*>(pUsrCtxt, pSession));

    if (ret.second == false)
    {
        PPROC_LOG(ERROR, "Failed to AddSession (%p %p)", pUsrCtxt, pSession);
        rc =  CAMERA_EFAILED;
    }

    return rc;
}

/**
 * RemoveSession
 *
 * @brief Remove session from map
 *
 * @param pUsrCtxt
 *
 * @return none
 */
void AisPProcGpu::RemoveSession(AisUsrCtxt* pUsrCtxt)
{
    m_sessionMap.erase(pUsrCtxt);
}

/**
 * GetSession
 *
 * @brief Get session from map
 *
 * @param pUsrCtxt
 *
 * @return AisPProcCombineSession pointer
 */
AisPProcGpuSession* AisPProcGpu::GetSession(AisUsrCtxt* pUsrCtxt)
{
    std::map<AisUsrCtxt*, AisPProcGpuSession*>::iterator it;
    it = m_sessionMap.find(pUsrCtxt);
    if (it == m_sessionMap.end())
    {
        PPROC_LOG(ERROR, "can't find the session for %p", pUsrCtxt);
        return NULL;
    }

    return (*it).second;
}

/**
 * CreateSession
 *
 * @brief Register buffers with Combine PPROC
 *
 * @param pUsrCtxt
 * @param pProcChain
 *
 * @return CameraResult
 */
CameraResult AisPProcGpu::CreateSession(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisPProcGpuSession* pSession = NULL;

    CameraLockMutex(m_gpuMutex);
    if (m_isC2dInitialized)
    {
        switch ((AisPProcGpuFunction)pProcChain->pprocFunction)
        {
        case AIS_PPROC_GPU_FUNCTION_PAIRED_INPUT:
            pSession = new AisPProcGpuPairedInputSession(pUsrCtxt, pProcChain);
            break;
        case AIS_PPROC_GPU_FUNCTION_DEINTERLACE:
            pSession = new AisPProcGpuDeinterlaceSession(pUsrCtxt, pProcChain);
            break;
        case AIS_PPROC_GPU_FUNCTION_TRANSFORMER:
            pSession = new AisPProcGpuTransformSession(pUsrCtxt, pProcChain);
            break;
        default:
            PPROC_LOG(ERROR, "Unsupported PProc GPU function %d", pProcChain->pprocFunction);
            break;
        }
    }
    CameraUnlockMutex(m_gpuMutex);

    if (pSession)
    {
        m_sessionMap.insert(std::pair<AisUsrCtxt*, AisPProcGpuSession*>(pUsrCtxt, pSession));
    }
    else
    {
        rc = CAMERA_EFAILED;
    }

    return rc;
}

/**
 * DestroySession
 *
 * @brief DeRegister buffers with Combine PPROC
 *
 * @param pUsrCtxt
 * @param pProcChain
 *
 * @return CameraResult
 */
CameraResult AisPProcGpu::DestroySession(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain)
{
    AisPProcGpuSession* pSession = GetSession(pUsrCtxt);
    if (pSession)
    {
        RemoveSession(pUsrCtxt);
        delete pSession;
    }

    return CAMERA_SUCCESS;
}

/**
 * GPUEventHandler
 *
 * @brief Performs c2dDriverInit
 *
 * @param NULL
 *
 * returns 0
 */
int AisPProcGpu::GPUEventHandler(void* pArg)
{
    AisPProcGpu* pGpuCtxt = (AisPProcGpu*)pArg;

    if (pGpuCtxt)
    {
        PPROC_LOG(HIGH, "Starting GPUEventHandler");

        CameraLockMutex(pGpuCtxt->m_gpuMutex);
        C2D_STATUS c2d_status = c2dDriverInit(NULL);
        if (c2d_status != C2D_STATUS_OK)
        {
            PPROC_LOG(ERROR, "C2DDriverInit failed %d", c2d_status);
        }
        else
        {
            PPROC_LOG(HIGH, "C2DDriverInit success!");
            pGpuCtxt->m_isC2dInitialized = TRUE;
        }
        CameraUnlockMutex(pGpuCtxt->m_gpuMutex);
    }
    return 0;
}

/**
 * ProcessEvent
 *
 * @brief Process ISP Frame
 *
 * @param pUsrCtxt
 * @param pEvent
 *
 * @return CameraResult
 */
CameraResult AisPProcGpu::ProcessEvent(AisUsrCtxt* pUsrCtxt, AisEventMsgType* pEvent)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisEventPProcJobType* pJob = &pEvent->payload.pprocJob;
    AisPProcGpuSession* pSession = GetSession(pUsrCtxt);

    if (pSession)
    {
        rc = pSession->ProcessEvent(pJob);

        //only move to next hop if success
        if (CAMERA_SUCCESS == rc)
        {
            pJob->status = rc;
            pEvent->eventId = AIS_EVENT_PPROC_JOB_DONE;
            AisEngine::GetInstance()->QueueEvent(pEvent);
        }
    }

    return rc;
}

static uint32 GetC2DColorFormat(qcarcam_color_pattern_t pattern)
{
    uint32 c2dFormat = C2D_COLOR_FORMAT_422_UYVY;

    switch(pattern)
    {
    case QCARCAM_YUV_YUYV:
        c2dFormat = C2D_COLOR_FORMAT_422_YUYV;
        break;
    case QCARCAM_YUV_YVYU:
        c2dFormat = C2D_COLOR_FORMAT_422_YVYU;
        break;
    case QCARCAM_YUV_UYVY:
        c2dFormat = C2D_COLOR_FORMAT_422_UYVY;
        break;
    case QCARCAM_YUV_VYUY:
        c2dFormat = C2D_COLOR_FORMAT_422_VYUY;
        break;
    case QCARCAM_YUV_NV12:
        c2dFormat = C2D_COLOR_FORMAT_420_NV12;
        PPROC_LOG(HIGH, "NV12 format");
        break;
    case QCARCAM_YUV_NV21:
        c2dFormat = C2D_COLOR_FORMAT_420_NV21;
        PPROC_LOG(HIGH, "NV21 format");
        break;
#if defined(__ANDROID__)|| defined(__AGL__)
    case QCARCAM_YUV_YU12:
        c2dFormat = C2D_COLOR_FORMAT_420_Y_U_V;
        break;
    case QCARCAM_YUV_YV12:
        c2dFormat = C2D_COLOR_FORMAT_420_Y_V_U ;
        break;
#endif
    default:
        break;
    }

    return c2dFormat;
}

static void FillC2DSurfaceDefPlaneInfo(qcarcam_color_pattern_t pattern, C2D_YUV_SURFACE_DEF& yuv_def,
    const qcarcam_buffer_v2_t& bufferInfo, uint8* pVa, uint8* gpuaddr)
{
    switch(pattern)
    {
    case QCARCAM_YUV_YUYV:
    case QCARCAM_YUV_YVYU:
    case QCARCAM_YUV_UYVY:
    case QCARCAM_YUV_VYUY:
        yuv_def.plane0  = pVa;
        yuv_def.phys0   = (void *)gpuaddr;
        break;
    case QCARCAM_YUV_NV12:
    case QCARCAM_YUV_NV21:
        // Y
        yuv_def.plane0  = pVa;
        yuv_def.phys0   = (void *)gpuaddr;

        // UV
        yuv_def.plane1  = &pVa[bufferInfo.planes[0].size];
        yuv_def.phys1   = &gpuaddr[bufferInfo.planes[0].size];

        PPROC_LOG(HIGH, "Fill_C2DSurfaceDefPlaneInfo NV12/NV21 format stride0 = %u stride1=%u offset1=%u",
            yuv_def.stride0, yuv_def.stride1, bufferInfo.planes[0].size);
        break;
#if defined(__ANDROID__)|| defined(__AGL__)
    case QCARCAM_YUV_YU12:
    case QCARCAM_YUV_YV12:
        // Y
        yuv_def.plane0  = pVa;
        yuv_def.phys0   = (void *)gpuaddr ;
        yuv_def.stride0 = bufferInfo.planes[0].stride;

        // U/V
        yuv_def.plane1  = &pVa[bufferInfo.planes[0].size];
        yuv_def.phys1   = &gpuaddr[bufferInfo.planes[0].size];
        yuv_def.stride1 = bufferInfo.planes[1].stride;

        // V/U
        yuv_def.plane2  = &pVa[bufferInfo.planes[0].size + bufferInfo.planes[1].size];
        yuv_def.phys2  = &gpuaddr[bufferInfo.planes[0].size + bufferInfo.planes[1].size];
        yuv_def.stride2 = bufferInfo.planes[2].stride;
        break;
#endif

    default:
        break;
    }
}

/**
 * MapBuffers
 *
 * @brief Map buffers to C2D surfaces
 *
 * @return CameraResult
 */
CameraResult AisPProcGpuSession::MapBuffers(AisPProcGpuBufferType type, AisPProcGpuFunction function)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (m_pBuffers[type])
    {
        PPROC_LOG(ERROR, "Already mapped buffers for %d", type);
        rc = CAMERA_EALREADY;
    }
    else
    {
        uint32 i = 0;
        C2D_STATUS c2d_status = C2D_STATUS_OK;
        C2D_SURFACE_TYPE surface_type;
        AisBufferList* pBufferList = m_pUsrCtxt->m_bufferList[m_bufferlistIdx[type]];
        AisPProcGpuBuffer* pBuffers = (AisPProcGpuBuffer*)CameraAllocate(CAMERA_ALLOCATE_ID_UNASSIGNED, sizeof(AisPProcGpuBuffer)*pBufferList->m_nBuffers);
        if (pBuffers == NULL)
        {
            PPROC_LOG(ERROR, "Falied to allocate memory to pBuffers");
            rc = CAMERA_ENOMEMORY;
            return rc;
        }

        qcarcam_color_pattern_t pattern = QCARCAM_COLOR_GET_PATTERN(pBufferList->GetColorFmt());

        if (QCARCAM_RGB == pattern)
        {
            C2D_RGB_SURFACE_DEF c2d_rgb_surface_def = {};
            surface_type = (C2D_SURFACE_TYPE)(C2D_SURFACE_RGB_HOST | C2D_SURFACE_WITH_PHYS);
            c2d_rgb_surface_def.format = C2D_COLOR_FORMAT_888_RGB;
            c2d_rgb_surface_def.width = pBufferList->GetWidth();
            c2d_rgb_surface_def.height = pBufferList->GetHeight();
            c2d_rgb_surface_def.stride = pBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;

            // adjust output dimensions for deinterlace weaving
            if (function == AIS_PPROC_GPU_FUNCTION_DEINTERLACE &&
                type == PPROC_GPU_BUFFER_OUT)
            {
                c2d_rgb_surface_def.width *= 2;
                c2d_rgb_surface_def.height /= 2;
                c2d_rgb_surface_def.stride *= 2;
            }

            for (i = 0; i < pBufferList->m_nBuffers; i++)
            {
#if defined(__ANDROID__)|| defined(__AGL__)
                c2d_status = c2dMapAddr((int)(intptr_t)pBufferList->m_pBuffers[i].pMemHndl,
                        pBufferList->m_pBuffers[i].pVa,
                        pBufferList->m_pBuffers[i].size,
                        0,
                        KGSL_USER_MEM_TYPE_ION,
                        &pBuffers[i].c2dPhysAddr[0]);
#else
                pBuffers[i].c2dPhysAddr[0] = (void*)CameraBufferGetPhysicalAddress(&pBufferList->m_pBuffers[i]);
#endif

                if (c2d_status == C2D_STATUS_OK)
                {
                    c2d_rgb_surface_def.buffer = pBufferList->m_pBuffers[i].pVa;
                    c2d_rgb_surface_def.phys = pBuffers[i].c2dPhysAddr[0];
                    c2d_status = c2dCreateSurface(&pBuffers[i].c2dSurfaceId,
                            C2D_SOURCE | C2D_TARGET,
                            surface_type,
                            &c2d_rgb_surface_def);
                }

                if (c2d_status != C2D_STATUS_OK)
                {
                    PPROC_LOG(ERROR, "c2dMapAddr | c2dCreateSurface buf %d failed %d", i, c2d_status);
                    rc = CAMERA_EFAILED;
                }

                PPROC_LOG(MED, "c2dCreateSurface RGB %d[%d]: id %u, %u x %u, stride %d, size %u",
                    type, i,
                    pBuffers[i].c2dSurfaceId,
                    c2d_rgb_surface_def.width, c2d_rgb_surface_def.height,
                    c2d_rgb_surface_def.stride,
                    pBufferList->m_pBuffers[i].size);
            }
        }
        else if (pattern >= QCARCAM_YUV_YUYV && pattern < QCARCAM_BAYER_GBRG)
        {
            C2D_YUV_SURFACE_DEF c2d_yuv_surface_def = {};
            surface_type = (C2D_SURFACE_TYPE) (C2D_SURFACE_YUV_HOST | C2D_SURFACE_WITH_PHYS);
            c2d_yuv_surface_def.format = GetC2DColorFormat(pattern);
            c2d_yuv_surface_def.width = pBufferList->GetWidth();
            c2d_yuv_surface_def.height = pBufferList->GetHeight();
            c2d_yuv_surface_def.stride0 = pBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;
            c2d_yuv_surface_def.stride1 = pBufferList->m_pBuffers[0].bufferInfo.planes[1].stride;
            c2d_yuv_surface_def.stride2 = pBufferList->m_pBuffers[0].bufferInfo.planes[2].stride;

            // adjust output dimensions for deinterlace weaving
            if (function == AIS_PPROC_GPU_FUNCTION_DEINTERLACE &&
                type == PPROC_GPU_BUFFER_OUT)
            {
                c2d_yuv_surface_def.width *= 2;
                c2d_yuv_surface_def.height /= 2;
                c2d_yuv_surface_def.stride0 *= 2;
                c2d_yuv_surface_def.stride1 *= 2;
                c2d_yuv_surface_def.stride2 *= 2;
            }

            for (i = 0; i < pBufferList->m_nBuffers; i++)
            {
#if defined(__ANDROID__)|| defined(__AGL__)
                c2d_status = c2dMapAddr((int)(intptr_t)pBufferList->m_pBuffers[i].pMemHndl,
                        pBufferList->m_pBuffers[i].pVa,
                        pBufferList->m_pBuffers[i].size,
                        0,
                        KGSL_USER_MEM_TYPE_ION,
                        &pBuffers[i].c2dPhysAddr[0]);
#else
                pBuffers[i].c2dPhysAddr[0] = (void*)CameraBufferGetPhysicalAddress(&pBufferList->m_pBuffers[i]);
#endif
                if (c2d_status == C2D_STATUS_OK)
                {
                    FillC2DSurfaceDefPlaneInfo(pattern, c2d_yuv_surface_def, pBufferList->m_pBuffers[i].bufferInfo,
                        (uint8*)pBufferList->m_pBuffers[i].pVa, (uint8*)pBuffers[i].c2dPhysAddr[0]);

                    c2d_status = c2dCreateSurface(&pBuffers[i].c2dSurfaceId,
                            C2D_SOURCE | C2D_TARGET,
                            surface_type,
                            &c2d_yuv_surface_def);
                }

                if (c2d_status != C2D_STATUS_OK)
                {
                    PPROC_LOG(ERROR, "c2dMapAddr | c2dCreateSurface buf %d failed %d size %u", i, c2d_status, pBufferList->m_pBuffers[i].size);
                    rc = CAMERA_EFAILED;
                }

                PPROC_LOG(MED, "c2dCreateSurface YUV %d[%d]: id %u, %u x %u, stride %d, size %u",
                    type, i,
                    pBuffers[i].c2dSurfaceId,
                    c2d_yuv_surface_def.width, c2d_yuv_surface_def.height,
                    c2d_yuv_surface_def.stride0,
                    pBufferList->m_pBuffers[i].size);
            }
        }
        else
        {
            PPROC_LOG(ERROR, "Unsupported color format %d", pBufferList->GetColorFmt());
            rc = CAMERA_EINVALIDFORMAT;
        }

        m_pBuffers[type] = pBuffers;
    }

    return rc;
}

/**
 * ReturnBuffer
 *
 * @brief Put back buffer to its bufferlist
 */
void AisPProcGpuSession::ReturnBuffer(AisPProcGpuBufferType type, uint32 idx)
{
    AisBufferList* pBufferList = m_pUsrCtxt->m_bufferList[m_bufferlistIdx[type]];
    pBufferList->SetBufferState(idx, AIS_BUFFER_INITIALIZED);
    pBufferList->ReturnBuffer(m_pUsrCtxt, idx);
}

CameraResult AisPProcGpuSession::UnmapAllBuffers()
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 bufType;

    for (bufType = 0; bufType < PPROC_GPU_BUFFER_MAX; bufType++)
    {
        if (m_pBuffers[bufType])
        {
            PPROC_LOG(HIGH, "unmap buffers %u", bufType);
            uint32 numBuffers = m_pUsrCtxt->m_bufferList[m_bufferlistIdx[bufType]]->m_nBuffers;
            uint32 bufIdx;
            C2D_STATUS c2d_status;

            for (bufIdx = 0; bufIdx < numBuffers; bufIdx++)
            {
                c2d_status = c2dDestroySurface(m_pBuffers[bufType][bufIdx].c2dSurfaceId);
#if defined(__ANDROID__)|| defined(__AGL__)
                if (c2d_status == C2D_STATUS_OK)
                {
                    c2d_status = c2dUnMapAddr(m_pBuffers[bufType][bufIdx].c2dPhysAddr[0]);
                }
#endif
                if (c2d_status != C2D_STATUS_OK)
                {
                    PPROC_LOG(ERROR, "c2dDestroySurface | c2dUnMapAddr buf %d failed %d", bufIdx, c2d_status);
                    rc = CAMERA_EFAILED;
                }
            }

            CameraFree(CAMERA_ALLOCATE_ID_UNASSIGNED, m_pBuffers[bufType]);
        }
    }

    return rc;
}


//////////////////////////////////////////////////////////////////////////////////
/// Paired Input Session
//////////////////////////////////////////////////////////////////////////////////

/**
 * AisPProcGpuPairedInputSession
 *
 * @brief Constructor
 *
 * @param pUsrCtxt
 * @param pProcChain
 */
AisPProcGpuPairedInputSession::AisPProcGpuPairedInputSession(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain)
{
    m_pUsrCtxt = pUsrCtxt;
    memset(m_pBuffers, 0x0, sizeof(m_pBuffers));

    m_bufferlistIdx[PPROC_GPU_BUFFER_OUT] = pProcChain->inBuflistId[0];
    m_bufferlistIdx[PPROC_GPU_BUFFER_RIGHT] = pProcChain->inBuflistId[1];
    MapBuffers(PPROC_GPU_BUFFER_OUT, (AisPProcGpuFunction)pProcChain->pprocFunction);
    MapBuffers(PPROC_GPU_BUFFER_RIGHT, (AisPProcGpuFunction)pProcChain->pprocFunction);
}

/**
 * AisPProcGpuPairedInputSession
 *
 * @brief Destructor
 */
AisPProcGpuPairedInputSession::~AisPProcGpuPairedInputSession()
{
    uint32 bufType;

    for (bufType = 0; bufType < PPROC_GPU_BUFFER_MAX; bufType++)
    {
        if (m_pBuffers[bufType])
        {
            uint32 numBuffers = m_pUsrCtxt->m_bufferList[m_bufferlistIdx[bufType]]->m_nBuffers;
            uint32 bufIdx;

            for (bufIdx = 0; bufIdx < numBuffers; bufIdx++)
            {
#if defined(__ANDROID__)|| defined(__AGL__)
                c2dUnMapAddr(m_pBuffers[bufType][bufIdx].c2dPhysAddr[0]);
#endif
                c2dDestroySurface(m_pBuffers[bufType][bufIdx].c2dSurfaceId);
            }

            CameraFree(CAMERA_ALLOCATE_ID_UNASSIGNED, m_pBuffers[bufType]);
        }
    }
}

/**
 * Paired Input ProcessEvent
 *
 * @brief Process new buffer
 */
CameraResult AisPProcGpuPairedInputSession::ProcessEvent(AisEventPProcJobType* pJob)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (pJob->streamIdx >= AIS_PPROC_MAX_BUFLISTS)
    {
        //Bail out if bad timestamp
        PPROC_LOG(ERROR, "Bad stream idx %d", pJob->streamIdx);
        return CAMERA_EBADPARM;
    }

    m_pUsrCtxt->Lock();

    AisPProcGpuBufferType bufType = (AisPProcGpuBufferType)pJob->streamIdx;
    AisBufferList *pInBufferList = m_pUsrCtxt->m_bufferList[pJob->pProcChain->inBuflistId[pJob->streamIdx]];
    AisBuffer* pInBuf = pInBufferList->GetReadyBuffer(pJob->jobId);
    if (!pInBuf)
    {
        AIS_LOG(ENGINE, ERROR, "Context %p cannot find input buffer for jobId 0x%llx", m_pUsrCtxt, pJob->jobId);
        m_pUsrCtxt->Unlock();
        return CAMERA_ENOREADYBUFFER;
    }

    //Check if valid qtimer timestamp
    if (pJob->frameInfo.sof_qtimestamp[0])
    {
        //add to queue
        m_bufsInFlight[bufType].push_back(pInBuf);
        m_pBuffers[bufType][pInBuf->idx].frameInfo = pJob->frameInfo;

        //Attempt to stitch buffers
        if(!m_bufsInFlight[PPROC_GPU_BUFFER_OUT].empty() &&
           !m_bufsInFlight[PPROC_GPU_BUFFER_RIGHT].empty())
        {
            rc = CombineBuffers(pJob);
        }
        else
        {
            //if not combining set as need more to abort job
            rc = CAMERA_ENEEDMORE;
        }

    }
    else
    {
        //Bail out if bad timestamp
        PPROC_LOG(ERROR, "Bad timestamp stream %d - bail out", bufType);
        ReturnBuffer(bufType, pInBuf->idx);
        rc = CAMERA_EBADPARM;
    }

    m_pUsrCtxt->Unlock();

    return rc;
}

/**
 * CombineBuffers
 *
 * @brief Find matching timestamp for left/right buffers and blit using C2D onto output buffer
 *
 * @param pJob
 *
 * @return CameraResult
 */
CameraResult AisPProcGpuPairedInputSession::CombineBuffers(AisEventPProcJobType* pJob)
{
    CameraResult rc = CAMERA_SUCCESS;
    boolean bFound = FALSE;
    boolean bExit = FALSE;
    AisBuffer* pBufferOut = m_bufsInFlight[PPROC_GPU_BUFFER_OUT].front();
    AisBuffer* pBufferRight = m_bufsInFlight[PPROC_GPU_BUFFER_RIGHT].front();
    uint64 out_qtime = m_pBuffers[PPROC_GPU_BUFFER_OUT][pBufferOut->idx].frameInfo.sof_qtimestamp[0];
    uint64 right_qtime = m_pBuffers[PPROC_GPU_BUFFER_RIGHT][pBufferRight->idx].frameInfo.sof_qtimestamp[0];

    //Loop through available buffers to find matching pair
    while (!bExit)
    {
        PPROC_LOG(MED, "%llu | %llu (%llu)", out_qtime, right_qtime, ABS_DIFF(out_qtime, right_qtime));

        //If absolute time difference is within epsilon, we have converged
        if (ABS_DIFF(out_qtime, right_qtime) <  EPSILON_QTIME)
        {
            bFound = TRUE;
            break;
        }
        else if (out_qtime < right_qtime)
        {
            PPROC_LOG(HIGH, "Drop Left buffer");

            //if left out of date, pop left
            ReturnBuffer(PPROC_GPU_BUFFER_OUT, pBufferOut->idx);
            m_bufsInFlight[PPROC_GPU_BUFFER_OUT].pop_front();
            if (!m_bufsInFlight[PPROC_GPU_BUFFER_OUT].empty())
            {
                pBufferOut = m_bufsInFlight[PPROC_GPU_BUFFER_OUT].front();
                out_qtime = m_pBuffers[PPROC_GPU_BUFFER_OUT][pBufferOut->idx].frameInfo.sof_qtimestamp[0];
            }
            else
            {
                //couldn't find match if q empty
                bExit = TRUE;
            }
        }
        else // (out_qtime > right_qtime)
        {
            PPROC_LOG(HIGH, "Drop Right buffer");

            //if right out of date, pop left
            ReturnBuffer(PPROC_GPU_BUFFER_RIGHT, pBufferRight->idx);
            m_bufsInFlight[PPROC_GPU_BUFFER_RIGHT].pop_front();
            if (!m_bufsInFlight[PPROC_GPU_BUFFER_RIGHT].empty())
            {
                pBufferRight = m_bufsInFlight[PPROC_GPU_BUFFER_RIGHT].front();
                right_qtime = m_pBuffers[PPROC_GPU_BUFFER_RIGHT][pBufferRight->idx].frameInfo.sof_qtimestamp[0];
            }
            else
            {
                //couldn't find match if q empty
                bExit = TRUE;
            }
        }
    }

    //If found, stitch left|right using c2d operations
    if (bFound)
    {
        C2D_STATUS c2d_status;
        C2D_OBJECT c2dObject[1];
        memset(c2dObject, 0x0, sizeof(c2dObject));
        unsigned int target_id = m_pBuffers[PPROC_GPU_BUFFER_OUT][pBufferOut->idx].c2dSurfaceId;

        PPROC_LOG(HIGH, "Combine %d | %d",
                pBufferOut->idx, pBufferRight->idx);

        c2dObject[0].surface_id = m_pBuffers[PPROC_GPU_BUFFER_RIGHT][pBufferRight->idx].c2dSurfaceId;
        c2dObject[0].config_mask = C2D_TARGET_RECT_BIT;
        //right rectangle offset by width of stream 0
        c2dObject[0].target_rect.x = pJob->pUsrCtxt->m_streams[0].inputCfg.inputModeInfo.width << 16;
        c2dObject[0].target_rect.y = 0;
        c2dObject[0].target_rect.width = pBufferRight->bufferInfo.planes[0].width << 16;
        c2dObject[0].target_rect.height = pBufferRight->bufferInfo.planes[0].height << 16;

        c2d_status = c2dDraw(target_id, C2D_TARGET_ROTATE_0, 0x0, 0, 0, c2dObject, 1);
        if (c2d_status == C2D_STATUS_OK)
        {
            c2d_status = c2dFinish(target_id);
        }

        //return right buffer
        ReturnBuffer(PPROC_GPU_BUFFER_RIGHT, pBufferRight->idx);

        //remove both buffers from Q
        m_bufsInFlight[PPROC_GPU_BUFFER_OUT].pop_front();
        m_bufsInFlight[PPROC_GPU_BUFFER_RIGHT].pop_front();

        //if failed, also return out buffer
        if (c2d_status != C2D_STATUS_OK)
        {
            PPROC_LOG(ERROR, "c2d merge failed with error %d", c2d_status);

            ReturnBuffer(PPROC_GPU_BUFFER_OUT, pBufferOut->idx);

            rc = CAMERA_EFAILED;
        }

        if (CAMERA_SUCCESS == rc)
        {
            AisBufferList* pBufferList = m_pUsrCtxt->m_bufferList[m_bufferlistIdx[PPROC_GPU_BUFFER_OUT]];
            pBufferList->QueueReadyBuffer(pJob->jobId, pBufferOut);
        }
    }
    else
    {
        //no match yet
        rc = CAMERA_ENEEDMORE;
    }

    return rc;
}

//////////////////////////////////////////////////////////////////////////////////
/// Deinterlace Session
//////////////////////////////////////////////////////////////////////////////////

/**
 * AisPProcGpuDeinterlaceSession
 *
 * @brief Constructor
 *
 * @param pUsrCtxt
 * @param pProcChain
 */
AisPProcGpuDeinterlaceSession::AisPProcGpuDeinterlaceSession(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain)
{
    m_pUsrCtxt = pUsrCtxt;
    memset(m_pBuffers, 0x0, sizeof(m_pBuffers));

    m_bufferlistIdx[PPROC_GPU_BUFFER_IN] = pProcChain->inBuflistId[0];
    m_bufferlistIdx[PPROC_GPU_BUFFER_OUT] = pProcChain->outBuflistId[0];
    MapBuffers(PPROC_GPU_BUFFER_IN, (AisPProcGpuFunction)pProcChain->pprocFunction);
    MapBuffers(PPROC_GPU_BUFFER_OUT, (AisPProcGpuFunction)pProcChain->pprocFunction);
}

/**
 * AisPProcGpuDeinterlaceSession
 *
 * @brief Destructor
 */
AisPProcGpuDeinterlaceSession::~AisPProcGpuDeinterlaceSession()
{
    uint32 bufType;

    for (bufType = 0; bufType < PPROC_GPU_BUFFER_MAX; bufType++)
    {
        if (m_pBuffers[bufType])
        {
            uint32 numBuffers = m_pUsrCtxt->m_bufferList[m_bufferlistIdx[bufType]]->m_nBuffers;
            uint32 bufIdx;

            for (bufIdx = 0; bufIdx < numBuffers; bufIdx++)
            {
#if defined(__ANDROID__)|| defined(__AGL__)
                c2dUnMapAddr(m_pBuffers[bufType][bufIdx].c2dPhysAddr[0]);
#endif
                c2dDestroySurface(m_pBuffers[bufType][bufIdx].c2dSurfaceId);
            }

            CameraFree(CAMERA_ALLOCATE_ID_UNASSIGNED, m_pBuffers[bufType]);
        }
    }
}

/**
 * Deinterlace ProcessEvent
 *
 * @brief Process new buffer
 */
CameraResult AisPProcGpuDeinterlaceSession::ProcessEvent(AisEventPProcJobType* pJob)
{
    CameraResult rc = CAMERA_SUCCESS;

    m_pUsrCtxt->Lock();

    AisBufferList *pInBufferList = m_pUsrCtxt->m_bufferList[m_bufferlistIdx[PPROC_GPU_BUFFER_IN]];
    AisBuffer* pInBuf = pInBufferList->GetReadyBuffer(pJob->jobId);
    if (!pInBuf)
    {
        AIS_LOG(ENGINE, ERROR, "Context %p cannot find input buffer for jobId 0x%llx", m_pUsrCtxt, pJob->jobId);
        m_pUsrCtxt->Unlock();
        return CAMERA_ENOREADYBUFFER;
    }

    qcarcam_field_t curField = pJob->frameInfo.field_type;

    // add frame to appropriate queue
    if (curField == QCARCAM_FIELD_ODD)
    {
        m_bufsInFlight[PPROC_GPU_BUFFER_LEFT].push_back(pInBuf);
    }
    else if (curField == QCARCAM_FIELD_EVEN)
    {
        m_bufsInFlight[PPROC_GPU_BUFFER_RIGHT].push_back(pInBuf);
    }
    else
    {
        PPROC_LOG(ERROR, "Unsupported field type %d", curField);
        ReturnBuffer(PPROC_GPU_BUFFER_IN, pInBuf->idx);
        m_pUsrCtxt->Unlock();
        return CAMERA_EFAILED;
    }

    // attempt to deinterlace two frames with weaving
    if(!m_bufsInFlight[PPROC_GPU_BUFFER_LEFT].empty() &&
        !m_bufsInFlight[PPROC_GPU_BUFFER_RIGHT].empty())
    {
        rc = WeaveBuffers(pJob, curField);
    }
    else
    {
        // waiting for both odd and even frames
        rc = CAMERA_ENEEDMORE;
    }

    m_pUsrCtxt->Unlock();

    return rc;
}

/**
 * Deinterlace WeaveBuffers
 *
 * @brief Weaves even and old fields together into output frame
 *
 * @param pJob
 * @param curField
 *
 * @return CameraResult
 */
CameraResult AisPProcGpuDeinterlaceSession::WeaveBuffers(AisEventPProcJobType* pJob, qcarcam_field_t curField)
{
    CameraResult rc = CAMERA_SUCCESS;

    // reserve output buffer
    AisBufferList *pOutBufferList = m_pUsrCtxt->m_bufferList[m_bufferlistIdx[PPROC_GPU_BUFFER_OUT]];
    AisBuffer* pOutAisBuf = pOutBufferList->GetFreeBuffer(m_pUsrCtxt);
    if (pOutAisBuf)
    {
        AisBuffer* pOddAisBuf = m_bufsInFlight[PPROC_GPU_BUFFER_LEFT].front();
        AisBuffer* pEvenAisBuf = m_bufsInFlight[PPROC_GPU_BUFFER_RIGHT].front();
        C2D_STATUS c2d_status;
        C2D_OBJECT c2dObject[2] = {};
        unsigned int target_id = m_pBuffers[PPROC_GPU_BUFFER_OUT][pOutAisBuf->idx].c2dSurfaceId;

        // odd frame
        c2dObject[0].surface_id = m_pBuffers[PPROC_GPU_BUFFER_IN][pOddAisBuf->idx].c2dSurfaceId;
        c2dObject[0].config_mask = C2D_TARGET_RECT_BIT;
        c2dObject[0].target_rect.x = 0;
        c2dObject[0].target_rect.y = 0;
        c2dObject[0].target_rect.width = pOutAisBuf->bufferInfo.planes[0].width << 16;
        c2dObject[0].target_rect.height = (pOutAisBuf->bufferInfo.planes[0].height / 2) << 16;
        c2dObject[0].next = &c2dObject[1];

        // even frame
        c2dObject[1].surface_id = m_pBuffers[PPROC_GPU_BUFFER_IN][pEvenAisBuf->idx].c2dSurfaceId;
        c2dObject[1].config_mask = C2D_TARGET_RECT_BIT;
        c2dObject[1].target_rect.x = pOutAisBuf->bufferInfo.planes[0].width << 16;
        c2dObject[1].target_rect.y = 0;
        c2dObject[1].target_rect.width = pOutAisBuf->bufferInfo.planes[0].width << 16;
        c2dObject[1].target_rect.height = (pOutAisBuf->bufferInfo.planes[0].height / 2) << 16;

        c2d_status = c2dDraw(target_id, C2D_TARGET_ROTATE_0, 0x0, 0, 0, c2dObject, 2);

        if (c2d_status == C2D_STATUS_OK)
        {
            c2d_status = c2dFinish(target_id);
        }

        if (c2d_status != C2D_STATUS_OK)
        {
            PPROC_LOG(ERROR, "c2dDraw | c2dFinish failed %d", c2d_status);
            rc = CAMERA_EFAILED;

            // return out buffer if failed
            ReturnBuffer(PPROC_GPU_BUFFER_OUT, pOutAisBuf->idx);
            PPROC_LOG(ERROR, "c2d deinterlace failed");
        }
        else
        {
            //Q output buffer as ready
            pOutBufferList->QueueReadyBuffer(pJob->jobId, pOutAisBuf);
            rc = CAMERA_SUCCESS;
        }

        // for 60fps weave, return older and opposite buffer, keep current buffer
        if (curField == QCARCAM_FIELD_ODD)
        {
            ReturnBuffer(PPROC_GPU_BUFFER_IN, pEvenAisBuf->idx);
            m_bufsInFlight[PPROC_GPU_BUFFER_RIGHT].pop_front();
        }
        else
        {
            ReturnBuffer(PPROC_GPU_BUFFER_IN, pOddAisBuf->idx);
            m_bufsInFlight[PPROC_GPU_BUFFER_LEFT].pop_front();
        }
    }
    else
    {
        PPROC_LOG(ERROR, "No free buffers (error %d), dropped frame!", rc);
        rc = CAMERA_EFAILED;
    }

    return rc;
}


AisPProcGpuTransformSession::AisPProcGpuTransformSession(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain)
{
    m_pUsrCtxt = pUsrCtxt;
    memset(m_pBuffers, 0x0, sizeof(m_pBuffers));

    m_bufferlistIdx[PPROC_GPU_BUFFER_IN] = pProcChain->inBuflistId[0];
    m_bufferlistIdx[PPROC_GPU_BUFFER_OUT] = pProcChain->outBuflistId[0];
    MapBuffers(PPROC_GPU_BUFFER_IN, (AisPProcGpuFunction)pProcChain->pprocFunction);
    MapBuffers(PPROC_GPU_BUFFER_OUT, (AisPProcGpuFunction)pProcChain->pprocFunction);
}

AisPProcGpuTransformSession::~AisPProcGpuTransformSession()
{
    UnmapAllBuffers();
}

CameraResult AisPProcGpuTransformSession::ProcessEvent(AisEventPProcJobType* pJob)
{
    CameraResult rc = CAMERA_SUCCESS;

    m_pUsrCtxt->Lock();

    AisBufferList *pInBufferList = m_pUsrCtxt->m_bufferList[m_bufferlistIdx[PPROC_GPU_BUFFER_IN]];
    AisBuffer* pInBuf = pInBufferList->GetReadyBuffer(pJob->jobId);
    if (!pInBuf)
    {
        AIS_LOG(ENGINE, ERROR, "Context %p cannot find input buffer for jobId 0x%llx", m_pUsrCtxt, pJob->jobId);
        m_pUsrCtxt->Unlock();
        return CAMERA_ENOREADYBUFFER;
    }

    // transform
    rc = Transform(pInBuf, pJob);

    m_pUsrCtxt->Unlock();

    return rc;
}

CameraResult AisPProcGpuTransformSession::Transform(AisBuffer* pInBuffer, AisEventPProcJobType* pJob)
{
    CameraResult rc = CAMERA_SUCCESS;

    // reserve output buffer
    AisBufferList* pOutBufferList = m_pUsrCtxt->m_bufferList[m_bufferlistIdx[PPROC_GPU_BUFFER_OUT]];
    AisBuffer* pOutBuffer = pOutBufferList->GetFreeBuffer(m_pUsrCtxt);
    if (pOutBuffer)
    {
        C2D_STATUS c2d_status;
        C2D_OBJECT c2dObject[2] = {};
        unsigned int target_id = m_pBuffers[PPROC_GPU_BUFFER_OUT][pOutBuffer->idx].c2dSurfaceId;

        c2dObject[0].surface_id = m_pBuffers[PPROC_GPU_BUFFER_IN][pInBuffer->idx].c2dSurfaceId;
        c2dObject[0].config_mask = C2D_SOURCE_RECT_BIT | C2D_TARGET_RECT_BIT;
        c2dObject[0].source_rect.x = 0;
        c2dObject[0].source_rect.y = 0;
        c2dObject[0].source_rect.width = pInBuffer->bufferInfo.planes[0].width << 16;
        c2dObject[0].source_rect.height = pInBuffer->bufferInfo.planes[0].height << 16;
        c2dObject[0].target_rect.x = 0;
        c2dObject[0].target_rect.y = 0;
        c2dObject[0].target_rect.width = pOutBuffer->bufferInfo.planes[0].width << 16;
        c2dObject[0].target_rect.height = (pOutBuffer->bufferInfo.planes[0].height) << 16;
        c2dObject[0].next = &c2dObject[0];

        PPROC_LOG(MED, "begin transform");
        c2d_status = c2dDraw(target_id, C2D_TARGET_ROTATE_0, 0x0, 0, 0, c2dObject, 1);

        if (c2d_status == C2D_STATUS_OK)
        {
            c2d_status = c2dFinish(target_id);
        }

        PPROC_LOG(MED, "end transform");

        if (c2d_status != C2D_STATUS_OK)
        {
            PPROC_LOG(ERROR, "c2dDraw | c2dFinish failed %d", c2d_status);
            rc = CAMERA_EFAILED;

            // return out buffer if failed
            ReturnBuffer(PPROC_GPU_BUFFER_OUT, pOutBuffer->idx);
            PPROC_LOG(ERROR, "c2d transform failed");
        }
        else
        {
            //Q output buffer as ready
            pOutBufferList->QueueReadyBuffer(pJob->jobId, pOutBuffer);

            rc = CAMERA_SUCCESS;
        }

        ReturnBuffer(PPROC_GPU_BUFFER_IN, pInBuffer->idx);
    }
    else
    {
        PPROC_LOG(ERROR, "No free buffers (error %d), dropped frame!", rc);
        rc = CAMERA_EFAILED;
    }

    return rc;
}

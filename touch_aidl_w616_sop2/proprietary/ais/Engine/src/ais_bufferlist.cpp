/* ===========================================================================
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @file ais_bufferlist.cpp
 * @brief AIS bufferlist implementation
 *
=========================================================================== */

#include "ais_bufferlist.h"
#include "ais_i.h"
#include "ais_engine.h"
#include "pthread.h"


AisBufferList::AisBufferList(AisBuflistIdType id,
        uint32 maxBuffers)
{
    m_id = id;
    m_maxBuffers = STD_MIN(maxBuffers, QCARCAM_MAX_NUM_BUFFERS);

    m_colorFmt = (qcarcam_color_fmt_t)0;
    m_width = 0;
    m_height = 0;
    m_nBuffers = 0;
    memset(m_pBuffers, 0x0, sizeof(m_pBuffers));

    m_GetFreeBuf = NULL;
    m_ReturnBuf = NULL;
    m_AllocBuf = NULL;

    m_mutex = NULL;
    m_bufferDoneQ = NULL;
    m_isBufferDoneQInit = FALSE;
};

CameraResult AisBufferList::Initialize()
{
    CameraResult rc;

    rc = CameraCreateMutex(&m_mutex);
    if (CAMERA_SUCCESS == rc)
    {
        /*create frame done Q*/
        CameraQueueCreateParamType sCreateParams = {};
        sCreateParams.nCapacity = m_maxBuffers;
        sCreateParams.nDataSizeInBytes = sizeof(qcarcam_frame_info_v2_t);
        sCreateParams.eLockType = CAMERAQUEUE_LOCK_THREAD;
        rc = CameraQueueCreate(&m_bufferDoneQ, &sCreateParams);
    }

    if (CAMERA_SUCCESS == rc)
    {
        pthread_condattr_t attr;

        if (EOK != (rc = pthread_condattr_init(&attr)))
        {
            AIS_LOG(ENGINE, ERROR, "pthread_cond_attr_init failed: %s", strerror(rc));
        }
        else
        {
#ifndef __INTEGRITY
            if (EOK != (rc = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC)))
            {
                AIS_LOG(ENGINE, ERROR, "pthread_cond_attr_setclock failed: %s", strerror(rc));
            }
            else
#endif
            if (EOK != (rc = pthread_cond_init(&m_bufferDoneQCond, &attr)))
            {
                AIS_LOG(ENGINE, ERROR, "pthread_cond_init(cond_buffer_done_q) failed: %s", strerror(rc));
            }
            else if (EOK != (rc = pthread_mutex_init(&m_bufferDoneQMutex, NULL)))
            {
                pthread_cond_destroy(&m_bufferDoneQCond);
            }

            // clean up - ignore non-fatal error
            int rc1 = EOK;
            if (EOK != (rc1 = pthread_condattr_destroy(&attr)))
            {
                AIS_LOG(ENGINE, ERROR, "pthread_condattr_destroy() failed: %s", strerror(rc1));
            }

            rc = ERRNO_TO_RESULT(rc);
            if (CAMERA_SUCCESS == rc)
            {
                m_isBufferDoneQInit = TRUE;
            }
        }
    }

    return rc;
}

/**
 * Create
 *
 * @brief Creates bufferlist and initializes it
 *
 * @param id               buffer list ID
 * @param maxBuffers       max number of buffers in list
 *
 * @return AisBufferList*
 */
AisBufferList* AisBufferList::Create(AisBuflistIdType id,
        uint32 maxBuffers)
{
    CameraResult rc;
    AisBufferList* pNewBufferList = new AisBufferList(id, maxBuffers);

    rc = pNewBufferList->Initialize();

    if (CAMERA_SUCCESS != rc)
    {
        AIS_LOG(ENGINE, ERROR, "Failed to initialize bufferlist %d (%d)", id, rc);
        pNewBufferList->Destroy();
        pNewBufferList = NULL;
    }

    return pNewBufferList;
}

/**
 * Destroy
 *
 * @brief Destroys bufferlist. Uninitializes object and invokes destructor
 *
 * @return void
 */
void AisBufferList::Destroy()
{
    if (m_mutex)
    {
        CameraDestroyMutex(m_mutex);
    }
    if (m_bufferDoneQ)
    {
        CameraQueueDestroy(m_bufferDoneQ);
    }

    if (m_isBufferDoneQInit)
    {
        pthread_cond_destroy(&m_bufferDoneQCond);
        pthread_mutex_destroy(&m_bufferDoneQMutex);
    }

    delete this;
}

/**
 * Lock
 *
 * @brief Locks bufferlist mutex
 *
 * @return void
 */
void AisBufferList::Lock()
{
    CameraLockMutex(m_mutex);
}

/**
 * Unlock
 *
 * @brief Unlocks bufferlist mutex
 *
 * @return void
 */
void AisBufferList::Unlock()
{
    CameraUnlockMutex(m_mutex);
}

/**
 * Init
 *
 * @brief Initializes bufferlist with function pointers
 */
void AisBufferList::Init(GetFreeBufFcnType GetFreeBufFcn,
        ReturnBufFcnType ReturnBufFcn,
        AllocBufType AllocBufFcn)
{
    m_GetFreeBuf = GetFreeBufFcn;
    m_ReturnBuf = ReturnBufFcn;
    m_AllocBuf = AllocBufFcn;
    for(uint32 idx = 0; idx < m_nBuffers; idx++)
    {
        m_pBuffers[idx].state = AIS_BUFFER_INITIALIZED;
    }
}


void AisBufferList::Reset()
{
    Lock();

    m_readyQ.clear();

    for(uint32 idx = 0; idx < m_nBuffers; idx++)
    {
        m_pBuffers[idx].state = AIS_BUFFER_INITIALIZED;
    }

    pthread_mutex_lock(&m_bufferDoneQMutex);
    CameraQueueClear(m_bufferDoneQ);
    pthread_cond_signal(&m_bufferDoneQCond);
    pthread_mutex_unlock(&m_bufferDoneQMutex);

    Unlock();
}

void AisBufferList::SetMaxBuffers(uint32 maxBuffers)
{
    m_maxBuffers = STD_MIN(maxBuffers, QCARCAM_MAX_NUM_BUFFERS);
}

void AisBufferList::SetProperties(uint32 numBuffers, uint32 width, uint32 height, qcarcam_color_fmt_t colorFmt)
{
    m_nBuffers = numBuffers;
    m_width = width;
    m_height = height;
    m_colorFmt = colorFmt;
}

/**
 * GetReadyBuffer
 *
 * @brief Get ready buffer associated with job ID
 *
 * @param jobId
 *
 * @return AisBuffer
 */
AisBuffer* AisBufferList::GetReadyBuffer(uint64 jobId)
{
    AisBuffer* pBuffer = NULL;
    std::list<AisBuflistReadyQType>::iterator  it;

    Lock();
    for(it = m_readyQ.begin(); it != m_readyQ.end(); ++it)
    {
        if (it->jobId == jobId)
        {
            pBuffer = it->pBuffer;
            m_readyQ.erase(it);
            break;
        }
    }
    Unlock();

    AIS_LOG(ENGINE, DBG, "DQ 0x%llx %p", jobId, pBuffer);

    return pBuffer;
}

/**
 * QueueReadyBuffer
 *
 * @brief Queues buffer associated with job ID to buffer ready list
 *
 * @param jobId      job ID
 * @param pBuffer    buffer
 *
 * @return CameraResult
 */
CameraResult AisBufferList::QueueReadyBuffer(uint64 jobId, AisBuffer* pBuffer)
{
    AisBuflistReadyQType readyQ = {jobId, pBuffer};

    Lock();
    m_readyQ.push_back(readyQ);
    Unlock();

    AIS_LOG(ENGINE, DBG, "Q 0x%llx %p", jobId, pBuffer);

    return CAMERA_SUCCESS;
}


/**
 * GetBuffer
 *
 * @brief Get ptr to buffer if valid
 *
 * @param idx
 *
 * @return AisBuffer
 */
AisBuffer* AisBufferList::GetBuffer(uint32 idx)
{
    return (idx >= m_nBuffers) ? NULL : &m_pBuffers[idx];
}

AisBuffer* AisBufferList::GetFreeBuffer(AisUsrCtxt* pUsrCtxt)
{
    return m_GetFreeBuf ? m_GetFreeBuf(pUsrCtxt, this) : NULL;
}

CameraResult AisBufferList::ReturnBuffer(AisUsrCtxt* pUsrCtxt, uint32 idx)
{
    return m_ReturnBuf ? m_ReturnBuf(pUsrCtxt, this, idx) : CAMERA_EBADSTATE;
}

CameraResult AisBufferList::AllocBuffers(AisUsrCtxt* pUsrCtxt, const struct AisBuflistDefType* pBuflistDef)
{
    return m_AllocBuf ? m_AllocBuf(pUsrCtxt, pBuflistDef) : CAMERA_EFAILED;
}

/**
 * DumpBuffer
 *
 * @brief Dump buffer to file
 *
 * @param idx        buffer index
 * @param frameId    frame Id
 *
 * @return void
 */
void AisBufferList::DumpBuffer(uint32 idx, uint32 frameId)
{
    AisBuffer* pBuffer = GetBuffer(idx);

    if (pBuffer && !(pBuffer->flags & CAMERA_BUFFER_FLAG_SECURE))
    {
        char filename[128] = "";

        if (m_id == AIS_BUFLIST_OUTPUT_JPEG)
        {
            snprintf(filename, sizeof(filename), AIS_DEFAULT_DUMP_LOCATION "frame_%llu.jpg", (unsigned long long)frameId);
        }
        else
        {
            snprintf(filename, sizeof(filename), AIS_DEFAULT_DUMP_LOCATION "frame_%llu_%u.raw", (unsigned long long)frameId, m_id);
        }

        CameraWriteFile(filename, pBuffer->pVa, pBuffer->size);
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Cannot dump buflist %u buffer %u", m_id, idx);
    }
}

/**
 * GetBufferState
 *
 * @brief Get state of a buffer in buffer list
 *
 * @param idx
 *
 * @return ais_buffer_state_t
 */
AisBufferStateType AisBufferList::GetBufferState(uint32 idx)
{
    AisBufferStateType state;
    if (idx >= m_nBuffers)
    {
        AIS_LOG(ENGINE, ERROR, "Invalid buf idx %d (list %d)", idx, m_id);
        state = AIS_BUFFER_UNITIALIZED;
    }
    else
    {
        state = m_pBuffers[idx].state;
    }

    return state;
}

/**
 * SetBufferState
 *
 * @brief Set state of a buffer in a buffer list
 *
 * @param idx
 * @param ais_buffer_state_t
 *
 * @return CameraResult
 */
CameraResult AisBufferList::SetBufferState(uint32 idx, AisBufferStateType state)
{
    if (idx >= m_nBuffers)
    {
        AIS_LOG(ENGINE, ERROR, "Invalid buf idx %d (list %d)", idx, m_id);
        return CAMERA_EBADPARM;
    }

    Lock();
    m_pBuffers[idx].state = state;
    Unlock();

    return CAMERA_SUCCESS;
}

AisBuflistIdType AisBufferList::GetId()
{
    return m_id;
}

qcarcam_color_fmt_t AisBufferList::GetColorFmt()
{
    return m_colorFmt;
}

uint32 AisBufferList::GetWidth()
{
    return m_width;
}

uint32 AisBufferList::GetHeight()
{
    return m_height;
}

uint32 AisBufferList::GetMaxNumBuffers()
{
    return m_maxBuffers;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  ais_pproc_framesync.cpp
/// @brief AisPProcFrameSync class implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ais_pproc_framesync.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "ais_log.h"
#include "ais_engine.h"
#include "CameraPlatform.h"

// QTime delta threshold in nsec between left and right buffers
#define EPSILON_QTIME 500000

AisPprocFrameSync* AisPprocFrameSync::m_pNodeInstance = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AisPprocFrameSyncSession::AisPprocFrameSyncSession
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AisPprocFrameSyncSession::AisPprocFrameSyncSession(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain)
: m_pUsrCtxt(pUsrCtxt), m_ErrorReportCnt(0), m_TotalSyncCnt(0), m_SuccessSyncCnt(0),
  m_FailSyncCnt(0)
{
    m_bufferlistIdx[FrameSyncBufLeft] = pProcChain->inBuflistId[0];
    m_bufferlistIdx[FrameSyncBufRight] = pProcChain->inBuflistId[1];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AisPprocFrameSyncSession::~AisPprocFrameSyncSession
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AisPprocFrameSyncSession::~AisPprocFrameSyncSession()
{
    for (int i = FrameSyncBufLeft; i < FrameSyncBufMax; i++)
    {
        m_inFlightBufs[i].clear();
    }
    m_retBufs.clear();

    AIS_LOG(PPROC_ISP, HIGH,
        "FramSync Stats [totalsync=%llu]: successync=%llu failsync=%llu",
        m_TotalSyncCnt, m_SuccessSyncCnt, m_FailSyncCnt);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AisPprocFrameSyncSession::ProcessEvent
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CameraResult AisPprocFrameSyncSession::ProcessEvent(AisEventMsgType* pEvent)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisEventPProcJobType* pJob = &pEvent->payload.pprocJob;
    FrameSyncBufType bufType = (FrameSyncBufType)pJob->streamIdx;

    AisBufferList* pBufferList = m_pUsrCtxt->m_bufferList[m_bufferlistIdx[bufType]];

    m_pUsrCtxt->Lock();

    AisBuffer* pBuffer = pBufferList->GetReadyBuffer(pJob->jobId);
    if (!pBuffer || pJob->frameInfo.idx != pBuffer->idx)
    {
        AIS_LOG(ENGINE, ERROR, "Could not get ready buffer matching jobId %llx idx %d",
                pJob->jobId, pJob->frameInfo.idx);
        m_pUsrCtxt->Unlock();
        return CAMERA_ENOREADYBUFFER;
    }

    // Add to inflight list
    m_inFlightBufs[bufType].push_back(pJob->frameInfo);

    // Run sync check
    rc = Synchronize();

    m_pUsrCtxt->Unlock();

    if (CAMERA_SUCCESS == rc)
    {
        pBufferList->QueueReadyBuffer(pJob->jobId, pBuffer);

        AIS_LOG(PPROC_ISP, DBG,
            "[totalsync=%llu]: successync=%llu failsync=%llu",
            m_TotalSyncCnt, m_SuccessSyncCnt, m_FailSyncCnt);

        pEvent->eventId = AIS_EVENT_PPROC_JOB_DONE;
        AisEngine::GetInstance()->QueueEvent(pEvent);
    }
    else if (rc != CAMERA_ENEEDMORE)
    {
        pEvent->eventId = AIS_EVENT_PPROC_JOB_FAIL;
        pEvent->payload.pprocJob.error = QCARCAM_FRAMESYNC_ERROR;
        AisEngine::GetInstance()->QueueEvent(pEvent);
    }

    return rc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AisPprocFrameSyncSession::Synchronize
// This function verifies that both left and right buffers of the same frame have
// SOF timestamps are within tolerance level and reports error if they are not.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CameraResult AisPprocFrameSyncSession::Synchronize()
{
    CameraResult rc = CAMERA_SUCCESS;
    boolean bExit = FALSE;

    while (!bExit)
    {
        if ((!m_inFlightBufs[FrameSyncBufLeft].empty()) &&
            (!m_inFlightBufs[FrameSyncBufRight].empty()))
        {
            m_TotalSyncCnt++;
            qcarcam_frame_info_v2_t leftFrameInfo = m_inFlightBufs[FrameSyncBufLeft].front();
            qcarcam_frame_info_v2_t rightFrameInfo = m_inFlightBufs[FrameSyncBufRight].front();
            uint64_t absDiff = ABS_DIFF(leftFrameInfo.sof_qtimestamp[0],
                    rightFrameInfo.sof_qtimestamp[0]);

            if ((leftFrameInfo.sof_qtimestamp[0] + absDiff) < leftFrameInfo.sof_qtimestamp[0] ||
                    (rightFrameInfo.sof_qtimestamp[0] + absDiff) < rightFrameInfo.sof_qtimestamp[0])
            {
                AIS_LOG(PPROC_ISP, WARN, "sofTs may overflow... [%llu,%llu,%llu] idx [%d,%d] seqNo [%d,%d]",
                        leftFrameInfo.sof_qtimestamp[0], rightFrameInfo.sof_qtimestamp[0],
                        absDiff,
                        leftFrameInfo.idx, rightFrameInfo.idx,
                        leftFrameInfo.seq_no[0], rightFrameInfo.seq_no[0]);
            }

            AIS_LOG(PPROC_ISP, LOW,
                    "sofTs [%llu,%llu,%llu] idx [%d,%d] seqNo [%d,%d]",
                    leftFrameInfo.sof_qtimestamp[0], rightFrameInfo.sof_qtimestamp[0],
                    absDiff,
                    leftFrameInfo.idx, rightFrameInfo.idx,
                    leftFrameInfo.seq_no[0], rightFrameInfo.seq_no[0]);

            if (absDiff < EPSILON_QTIME)
            {
                if (leftFrameInfo.idx == rightFrameInfo.idx)
                {
                    m_inFlightBufs[FrameSyncBufLeft].pop_front();
                    m_inFlightBufs[FrameSyncBufRight].pop_front();

                    AIS_LOG(PPROC_ISP, LOW,
                            "Match idx %d seqNum [%d,%d]",
                            leftFrameInfo.idx, leftFrameInfo.seq_no[0], rightFrameInfo.seq_no[0]);

                    // Both SOF timestamps and buffers indexes match.
                    // Sync check succeeded.
                    m_SuccessSyncCnt++;
                    if (m_ErrorReportCnt)
                        m_ErrorReportCnt--;
                    bExit = TRUE;
                }
                else
                {
                    AIS_LOG(PPROC_ISP, ERROR,
                            "SOF timestamp [%llu,%llu,%llu] match but idx [%d,%d] don't",
                            leftFrameInfo.sof_qtimestamp[0], rightFrameInfo.sof_qtimestamp[0],
                            absDiff,
                            leftFrameInfo.idx, rightFrameInfo.idx);
                    bExit = TRUE;
                    rc = CAMERA_EFAILED;
                }
            }
            else if (leftFrameInfo.idx == rightFrameInfo.idx)
            {
                AIS_LOG(PPROC_ISP, WARN,
                        "SOF delta %llu > %d for idx %d seqNo [%d,%d] (%llu %llu)",
                        absDiff,
                        EPSILON_QTIME, leftFrameInfo.idx,
                        leftFrameInfo.seq_no[0], rightFrameInfo.seq_no[0],
                        leftFrameInfo.sof_qtimestamp[0], rightFrameInfo.sof_qtimestamp[0]);
                m_inFlightBufs[FrameSyncBufLeft].pop_front();
                m_inFlightBufs[FrameSyncBufRight].pop_front();

                // Buffers indexes match but SOF timestamps don't.
                // Return buffer back to IFE and notify user.
                if (ReturnBuffer(FrameSyncBufLeft,leftFrameInfo.idx,
                                 leftFrameInfo.seq_no[0], FALSE) != CAMERA_SUCCESS)
                {
                    AIS_LOG(PPROC_ISP, ERROR,
                        "Failed to return buffer: type %d idx %d seqNum %d",
                        FrameSyncBufLeft, leftFrameInfo.idx, leftFrameInfo.seq_no[0]);
                }
                m_FailSyncCnt++;
                m_ErrorReportCnt++;
                bExit = TRUE;
                if (m_ErrorReportCnt >= m_ErrorReportThreshold)
                {
                    AIS_LOG(PPROC_ISP, ERROR, "FrameSync errors reached threshold %d. Bail out.",
                        m_ErrorReportThreshold);
                    rc = CAMERA_EFAILED;
                }
                else
                {
                    rc = CAMERA_ENEEDMORE;
                }
            }
            else
            {
                AIS_LOG(PPROC_ISP, WARN,
                        "SOF delta %llu > %d and buf [%d,%d] don't match",
                        absDiff, EPSILON_QTIME,
                        leftFrameInfo.idx, rightFrameInfo.idx,
                        leftFrameInfo.sof_qtimestamp[0], rightFrameInfo.sof_qtimestamp[0]);

                // Both SOF timestamps and buffer indexes don't match.
                // This can happen when buffer done events from different
                // frames were delayed in IFE IST and came as a group to
                // AIS engine. AIS engine has multiple event handling threads
                // and the order they come to PProc nodes can be random.
                // Return buffer with older timestamp to IFE and search
                // next available buffer.
                int retBufIdx = rightFrameInfo.idx;
                FrameSyncBufType retBufType = FrameSyncBufRight;
                unsigned int retBufSeqNum = rightFrameInfo.seq_no[0];
                if (leftFrameInfo.sof_qtimestamp[0] < rightFrameInfo.sof_qtimestamp[0])
                {
                    retBufType = FrameSyncBufLeft;
                    retBufIdx = leftFrameInfo.idx;
                    retBufSeqNum = leftFrameInfo.seq_no[0];
                }

                m_inFlightBufs[retBufType].pop_front();
                if (ReturnBuffer(retBufType, retBufIdx,
                                 retBufSeqNum, TRUE) != CAMERA_SUCCESS)
                {
                    AIS_LOG(PPROC_ISP, ERROR,
                        "Failed to return buffer: type %d idx %d seqNum %d",
                        retBufType, retBufIdx, retBufSeqNum);
                    bExit = TRUE;
                    rc = CAMERA_EFAILED;
                }
                m_FailSyncCnt++;
                m_ErrorReportCnt++;
            }
        }
        else
        {
            // Not enough buffers, abort the Pproc job
            bExit = TRUE;
            rc = CAMERA_ENEEDMORE;
        }
    }

    return rc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AisPprocFrameSyncSession::ReturnBuffer
// This function returns buffer only once to its source because two buffers (left and right)
// belong to the same big buffer that represent one frame.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CameraResult AisPprocFrameSyncSession::ReturnBuffer(FrameSyncBufType        bufType,
                                                            int             idx,
                                                            unsigned int    seqNum,
                                                            boolean         addToMap)
{
    CameraResult rc = CAMERA_SUCCESS;
    boolean bReturnBuf = TRUE;

    AisPProcFrameSyncRetBufMap::iterator it;
    it = m_retBufs.find(idx);
    if (it != m_retBufs.end())
    {
        unsigned int mapbufType = it->second;
        if (mapbufType == bufType)
        {
            AIS_LOG(PPROC_ISP, WARN, "Found matching buffer already returned %d %d?!", idx, bufType);

            // Remove stale buffer that has same index and same type
            m_retBufs.erase(it);

            if (addToMap)
            {
                m_retBufs.insert(std::pair<int,unsigned int>(idx, bufType));
            }
        }
        else
        {
            AIS_LOG(PPROC_ISP, WARN, "Found matching buf %d %d %d already dropped...", idx, bufType, seqNum);

            // This buffer has been already returned because it has same
            // index and of the other type. Just remove it from the map.
            m_retBufs.erase(it);
            bReturnBuf = FALSE;
        }
    }
    else
    {
        // Add buffer to the map if required
        if (addToMap)
        {
            AIS_LOG(PPROC_ISP, WARN, "Add buf %d %d %d to drop list...", idx, bufType, seqNum);

            m_retBufs.insert(std::pair<int,unsigned int>(idx, bufType));
        }
    }

    if (bReturnBuf)
    {
        AIS_LOG(PPROC_ISP, WARN, "Dropped buffer: idx %d %d seqNum %d", idx, bufType, seqNum);

        AisBufferList* pBufferList = m_pUsrCtxt->m_bufferList[m_bufferlistIdx[bufType]];
        rc = pBufferList->SetBufferState(idx, AIS_BUFFER_INITIALIZED);
        if (CAMERA_SUCCESS == rc)
        {
            rc = pBufferList->ReturnBuffer(m_pUsrCtxt, idx);
        }
    }

    return rc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AisPprocFrameSync::Create
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AisPprocFrameSync* AisPprocFrameSync::Create()
{
    m_pNodeInstance = new AisPprocFrameSync();
    return m_pNodeInstance;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AisPprocFrameSync::Destroy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AisPprocFrameSync::Destroy()
{
    if (m_pNodeInstance)
    {
        delete m_pNodeInstance;
        m_pNodeInstance = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AisPprocFrameSync::Destroy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AisPprocFrameSync* AisPprocFrameSync::GetInstance()
{
    return m_pNodeInstance;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AisPprocFrameSync::AisPprocFrameSync
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AisPprocFrameSync::AisPprocFrameSync()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AisPprocFrameSync::~AisPprocFrameSync
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AisPprocFrameSync::~AisPprocFrameSync()
{
    // clean up the session map
    std::map<AisUsrCtxt*, IFrameSyncSession*>::iterator it;
    for (it = m_SessionMap.begin(); it != m_SessionMap.end(); ++it)
    {
        IFrameSyncSession* pSession = it->second;
        delete pSession;
    }
    m_SessionMap.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AisPprocFrameSync::CreateSession
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CameraResult AisPprocFrameSync::CreateSession(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain)
{
    IFrameSyncSession* pSession = new AisPprocFrameSyncSession(pUsrCtxt, pProcChain);

    CameraResult rc = AddSession(pUsrCtxt, pSession);
    if (rc != CAMERA_SUCCESS)
        delete pSession;

    return rc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AisPprocFrameSync::DestroySession
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CameraResult AisPprocFrameSync::DestroySession(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain)
{
    CAM_UNUSED(pProcChain);
    return RemoveSession(pUsrCtxt);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AisPprocFrameSync::ProcessEvent
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CameraResult AisPprocFrameSync::ProcessEvent(AisUsrCtxt* pUsrCtxt, AisEventMsgType* pEvent)
{
    CameraResult rc = CAMERA_SUCCESS;

    IFrameSyncSession* pSession = GetSession(pUsrCtxt);
    if (pSession)
    {
        rc = pSession->ProcessEvent(pEvent);
    }
    else
    {
        rc = CAMERA_ERESOURCENOTFOUND;
    }
    return rc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AisPprocFrameSync::SetParams
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CameraResult AisPprocFrameSync::SetParams(AisUsrCtxt* pUsrCtxt)
{
    CAM_UNUSED(pUsrCtxt);
    CameraResult rc = CAMERA_SUCCESS;
    return rc;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AisPprocFrameSync::ProcessEvent
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CameraResult AisPprocFrameSync::GetParams(AisUsrCtxt* pUsrCtxt, void *pParamOut)
{
    CAM_UNUSED(pUsrCtxt); CAM_UNUSED(pParamOut);
    CameraResult rc = CAMERA_SUCCESS;
    return rc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AisPprocFrameSync::AddSession
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CameraResult AisPprocFrameSync::AddSession(AisUsrCtxt* pUsrCtxt, IFrameSyncSession* pSession)
{
    CameraResult rc = CAMERA_SUCCESS;

    std::pair<AisPProcFrameSyncSessionMap::iterator, boolean> ret;
    ret = m_SessionMap.insert(std::pair<AisUsrCtxt*, IFrameSyncSession*>(pUsrCtxt, pSession));
    if (FALSE == ret.second)
    {
        AIS_LOG(PPROC_ISP, ERROR, "Failed to AddSession (%p %p)", pUsrCtxt, pSession);
        rc = CAMERA_EFAILED;
    }

    return rc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AisPprocFrameSync::RemoveSession
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CameraResult AisPprocFrameSync::RemoveSession(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;

    std::map<AisUsrCtxt*, IFrameSyncSession*>::iterator it;
    it = m_SessionMap.find(pUsrCtxt);
    if (it != m_SessionMap.end())
    {
        IFrameSyncSession* pSession = it->second;
        m_SessionMap.erase(it);
        if (pSession)
            delete pSession;
    }
    else
    {
        AIS_LOG(PPROC_ISP, ERROR, "can't find the session for %p", pUsrCtxt);
        rc = CAMERA_ERESOURCENOTFOUND;
    }

    return rc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AisPprocFrameSync::GetSession
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IFrameSyncSession* AisPprocFrameSync::GetSession(AisUsrCtxt* pUsrCtxt)
{
    std::map<AisUsrCtxt*, IFrameSyncSession*>::iterator it;
    it = m_SessionMap.find(pUsrCtxt);
    if (it == m_SessionMap.end())
    {
        AIS_LOG(PPROC_ISP, ERROR, "can't find the session for %p", pUsrCtxt);
        return NULL;
    }
    return it->second;
}


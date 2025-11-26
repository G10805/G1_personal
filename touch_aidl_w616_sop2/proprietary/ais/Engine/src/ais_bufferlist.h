#ifndef _AIS_BUFFERLIST_H_
#define _AIS_BUFFERLIST_H_

/* ===========================================================================
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @file ais_bufferlist.h
 * @brief AIS bufferlist definition
 *
=========================================================================== */

#include <string.h>
#include <list>
#include <pthread.h>
#include "AEEstd.h"
#include "CameraResult.h"
#include "CameraCommonTypes.h"
#include "CameraOSServices.h"
#include "CameraQueue.h"
#include "CameraConfig.h"

class AisUsrCtxt;
class AisBufferList;

typedef enum
{
    AIS_BUFLIST_USR_FIRST,
    AIS_BUFLIST_USR_0 = AIS_BUFLIST_USR_FIRST,
    AIS_BUFLIST_USR_1,
    AIS_BUFLIST_USR_2,
    AIS_BUFLIST_USR_3,
    AIS_BUFLIST_OUTPUT_USR = AIS_BUFLIST_USR_0,
    AIS_BUFLIST_INPUT_USR = AIS_BUFLIST_USR_1,
    AIS_BUFLIST_USR_LAST = AIS_BUFLIST_USR_3,
    AIS_BUFLIST_VFE_RAW_OUT,
    AIS_BUFLIST_VFE_RAW_OUT2,
    AIS_BUFLIST_PPROC_1,
    AIS_BUFLIST_PPROC_2,
    AIS_BUFLIST_PPROC_3,
    AIS_BUFLIST_OUTPUT_JPEG,
    AIS_BUFLIST_MAX
}AisBuflistIdType;

typedef struct
{
    uint64 jobId;
    AisBuffer* pBuffer;
}AisBuflistReadyQType;

/**
 * Get available buffer from buffer list
 */
typedef AisBuffer* (*GetFreeBufFcnType)(AisUsrCtxt* pUsrCtxt, AisBufferList* pBufferList);

/**
 * Return buffer to buffer list
 */
typedef CameraResult (*ReturnBufFcnType)(AisUsrCtxt* pUsrCtxt, AisBufferList* pBufferList, uint32 idx);

/**
 * Allocate buffers for bufferlist
 */
typedef CameraResult (*AllocBufType)(AisUsrCtxt* pUsrCtxt, const struct AisBuflistDefType* pBuflistDef);

/**
 * Buffer list for inputs/outputs associated with a user context.
 */
class AisBufferList
{
public:
    CameraResult Initialize();

    /**
     * Create
     *
     * @brief Creates bufferlist
     *
     * @param id             bufferlist ID
     * @param maxBuffers     max number of buffers in list
     */
    static AisBufferList* Create(AisBuflistIdType id,
            uint32 maxBuffers);

    /**
     * Destroy
     *
     * @brief Destroys bufferlist
     */
    void Destroy();

    /**
     * Lock
     *
     * @brief Locks bufferlist mutex
     */
    void Lock();

    /**
     * Unlock
     *
     * @brief Unlocks bufferlist mutex
     */
    void Unlock();

    /**
     * Init
     *
     * @brief Initializes bufferlist with function pointers
     *
     * @param GetFreeBufFcn    Get free buffer function pointer
     * @param ReturnBufFcn     Return buffer function pointer
     * @param AllocBufFcn      Allocate buffer function
     */
    void Init(GetFreeBufFcnType GetFreeBufFcn,
            ReturnBufFcnType ReturnBufFcn,
            AllocBufType AllocBufFcn);

    /**
     * Reset
     *
     * @brief Resets buffer states and clears ready Q
     */
    void Reset();

    /**
     * SetMaxBuffers
     *
     * @brief Sets maximum number of buffers in list
     */
    void SetMaxBuffers(uint32 maxBuffers);

    /**
     * SetProperties
     *
     * @brief sets bufferlist properties (number of buffers, resolution and format)
     */
    void SetProperties(uint32 numBuffers, uint32 width, uint32 height, qcarcam_color_fmt_t colorFmt);


    /**
     * GetReadyBuffer
     *
     * @brief Get ready buffer associated with job ID
     *
     * @param jobId
     *
     * @return AisBuffer
     */
    AisBuffer* GetReadyBuffer(uint64 jobId);

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
    CameraResult QueueReadyBuffer(uint64 jobId, AisBuffer* pBuffer);

    /**
     * GetBuffer
     *
     * @brief Get ptr to buffer if valid index
     *
     * @param idx
     *
     * @return AisBuffer
     */
    AisBuffer* GetBuffer(uint32 idx);

    /**
     * GetFreeBuffer
     *
     * @brief Get free buffer to consume
     *
     * @param pUsrCtxt   user context
     *
     * @return AisBuffer*
     */
    AisBuffer* GetFreeBuffer(AisUsrCtxt* pUsrCtxt);


    /**
     * ReturnBuffer
     *
     * @brief Return buffer
     *
     * @param pUsrCtxt   user context
     * @param idx        buffer index
     *
     * @return CameraResult
     */
    CameraResult ReturnBuffer(AisUsrCtxt* pUsrCtxt, uint32 idx);

    /**
     * AllocBuffers
     *
     * @brief Allocate buffers based on buffer list definition
     *
     * @param pUsrCtxt   user context
     * @param pBuflistDef        buffer list definition
     *
     * @return CameraResult
     */
    CameraResult AllocBuffers(AisUsrCtxt* pUsrCtxt, const struct AisBuflistDefType* pBuflistDef);

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
    void DumpBuffer(uint32 idx, uint32 frameId);

    /**
     * GetBufferState
     *
     * @brief Get state of a buffer in buffer list
     *
     * @param idx
     *
     * @return ais_buffer_state_t
     */
    AisBufferStateType GetBufferState(uint32 idx);

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
    CameraResult SetBufferState(uint32 idx, AisBufferStateType state);

    AisBuflistIdType GetId();

    qcarcam_color_fmt_t GetColorFmt();

    uint32 GetWidth();

    uint32 GetHeight();

    uint32 GetMaxNumBuffers();

private:
    ~AisBufferList() = default;

    AisBufferList(AisBuflistIdType id,
            uint32 maxBuffers);

    AisBuflistIdType m_id;              /**< bufferlist ID */
    qcarcam_color_fmt_t m_colorFmt;     /**< color format of buffers */
    uint32 m_width;                     /**< width of buffers */
    uint32 m_height;                    /**< height of buffers */

    CameraMutex m_mutex;        /**< bufferlist mutex */

    std::list<AisBuflistReadyQType> m_readyQ; /**< buffer ready Q list between pproc stages*/

    uint32          m_isBufferDoneQInit;      /**< is buffer done Q initialized (used for cleanup) */

    GetFreeBufFcnType m_GetFreeBuf;
    ReturnBufFcnType m_ReturnBuf;
    AllocBufType m_AllocBuf;

    uint32 m_maxBuffers;  /**< max number of buffers that can be set */

public:
    AisBuffer m_pBuffers[QCARCAM_MAX_NUM_BUFFERS]; /**< array of buffers */
    uint32 m_nBuffers;                             /**< number of buffers set */

    pthread_cond_t  m_bufferDoneQCond; /**< conditional variable for buffer done Q
                                     signaled by usr_done node and waited by ais_get_frame */
    pthread_mutex_t m_bufferDoneQMutex;    /**< mutex to protect buffer done Q */
    CameraQueue m_bufferDoneQ;  /**< buffer done queue */
};


#endif /* _AIS_BUFFERLIST_H_ */


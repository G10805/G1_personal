/*===========================================================================

*//** @file hyp_queue_utility.c
This file implements utility functions for enqueue & dequeue

Copyright (c) 2016-2017, 2019, 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/


/*===========================================================================
                             Edit History

$Header: $

when(mm/dd/yy)     who         what, where, why
-------------      --------    -------------------------------------------------------
08/30/23           nb          Fix compilation errors due to additon of new compiler flags
04/11/19           sm          Generic queue interface
06/28/17           aw          Unify and update all logs in hyp-video
05/08/17           sm          Update for new hyp-video architecture
07/26/16           rz          Add debug message
07/15/16           hl          Add code to support LA target
06/01/16           hl          Add FE and BE to support Hypervisor interface

============================================================================*/
#include <pthread.h>
#include "hyp_queue_utility.h"
#include "hyp_debug.h"

/**===========================================================================

FUNCTION habmm_init_queue

@brief  Initialize queue for messages received from HAB

@param [in] pQueueInfo
@param [in] nMsgBufferCount
@param [in] nQueue
@param [in] nSignal

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type habmm_init_queue(IoctlHabmmQueueInfo* pQueueInfo, unsigned int nMsgBufferCount, int nQueue, int nSignal)
{
    int nQsize, i;
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    if ((nQueue > MAX_NUM_MSG_QUEUE) || (nSignal > MAX_NUM_SIGNAL))
    {
        HYP_VIDEO_MSG_ERROR("invalid input nQueue %d nSignal %d", nQueue, nSignal);
        rc = HYPV_STATUS_FAIL;
    }
    if ((0 < nMsgBufferCount) && (HYPV_STATUS_SUCCESS == rc))
    {
        for (i = 0; i < nQueue; i++)
        {
            pQueueInfo->nTotalMsgBufferCount[i] = nMsgBufferCount + 1;/** (+1): index handling for when the queue is full */

            nQsize = pQueueInfo->nTotalMsgBufferCount[i] * sizeof( BufferList);

            pQueueInfo->pBufferList[i] = ( BufferList *)HABMM_MALLOC(nQsize);

            if ( NULL ==  pQueueInfo->pBufferList[i] )
            {
                HYP_VIDEO_MSG_ERROR("message buffer queue alloc failed, out of memory");
                rc = HYPV_STATUS_FAIL;
            }
            else
            {
                HABMM_MEMSET(pQueueInfo->pBufferList[i], 0, nQsize);
                pQueueInfo->nMsgQueueInIndex[i] = 0;
                pQueueInfo->nMsgQueueOutIndex[i] = 0;
            }
        }
        for (i = 0; ( i < nSignal) && (rc == HYPV_STATUS_SUCCESS); i++)
        {
            /** Create signalQ and signal for I/O and callback message handling */
            if (MM_SignalQ_Create ( &pQueueInfo->MsgSignalQ[i].stateSignalQ ) != 0)
            {
                HYP_VIDEO_MSG_ERROR("failed to create SignalQ idx %d", i);
                rc = HYPV_STATUS_FAIL;
            }
            else if (MM_Signal_Create (pQueueInfo->MsgSignalQ[i].stateSignalQ,
                &pQueueInfo->MsgSignalQ[i].stateChangeSignal,
                NULL,
                &pQueueInfo->MsgSignalQ[i].stateChangeSignal) != 0)
            {
                HYP_VIDEO_MSG_ERROR("failed to create Signal idx %d", i);
                rc = HYPV_STATUS_FAIL;
            }
        }
    }
    else
    {
        HYP_VIDEO_MSG_ERROR("bad parameter, zero message buffer count");
        rc = HYPV_STATUS_FAIL;
    }
    if (rc != HYPV_STATUS_SUCCESS)
    {
        habmm_deinit_queue(pQueueInfo);
    }
    return rc;
}

/**===========================================================================

FUNCTION habmm_enqueue_buffer_list

@brief  Enqueue messages to queue

@param [in] pQueueInfo
@param [in] mode
@param [in] pNode

@dependencies
  None

@return
  Returns boolean

===========================================================================*/
boolean habmm_enqueue_buffer_list(IoctlHabmmQueueInfo* pQueueInfo, int mode, habmm_msg_desc_t *pNode)
{
    boolean bRet = TRUE;
    BufferList *pTmpMsgQ = pQueueInfo->pBufferList[mode];

    if ( ( ( pQueueInfo->nMsgQueueInIndex[mode] + 1) % pQueueInfo->nTotalMsgBufferCount[mode] ) == pQueueInfo->nMsgQueueOutIndex[mode] )
    {

       HYP_VIDEO_MSG_ERROR("failed %d is empty in index %u out index %u total %u",
                       mode, pQueueInfo->nMsgQueueInIndex[mode], pQueueInfo->nMsgQueueOutIndex[mode], pQueueInfo->nTotalMsgBufferCount[mode] );
       bRet = FALSE;
    }
    else
    {
        HABMM_MEMCPY(&pTmpMsgQ[pQueueInfo->nMsgQueueInIndex[mode]], pNode, sizeof(habmm_msg_desc_t));
        pQueueInfo->nMsgQueueInIndex[mode] = (pQueueInfo->nMsgQueueInIndex[mode] + 1) % pQueueInfo->nTotalMsgBufferCount[mode];
        HYP_VIDEO_MSG_LOW("nMsgQueueInIndex %u", pQueueInfo->nMsgQueueInIndex[mode]);
    }
    return bRet;
}

/**===========================================================================

FUNCTION habmm_dequeue_buffer_list

@brief  Dequeue messages from queue

@param [in] pQueueInfo
@param [in] mode

@dependencies
  None

@return
  Returns bmm_msg_desc_t pointer

===========================================================================*/
habmm_msg_desc_t* habmm_dequeue_buffer_list(IoctlHabmmQueueInfo* pQueueInfo, int mode)
{
    habmm_msg_desc_t *pTmpBufferNode = NULL;
    BufferList *pTmpMsgQ = pQueueInfo->pBufferList[mode];


    if ( pQueueInfo->nMsgQueueInIndex[mode] == pQueueInfo->nMsgQueueOutIndex[mode] )
    {
        HYP_VIDEO_MSG_LOW("message buffer queue %d is empty index %u", mode,
                      pQueueInfo->nMsgQueueInIndex[mode]);
    }
    else
    {
        pTmpBufferNode = &pTmpMsgQ[pQueueInfo->nMsgQueueOutIndex[mode]];
        pQueueInfo->nMsgQueueOutIndex[mode] = (pQueueInfo->nMsgQueueOutIndex[mode] + 1) % pQueueInfo->nTotalMsgBufferCount[mode];
        HYP_VIDEO_MSG_LOW("%d In %u Out %u", mode,
                      pQueueInfo->nMsgQueueInIndex[mode], pQueueInfo->nMsgQueueOutIndex[mode]);
    }
    return pTmpBufferNode;
}

/**===========================================================================

FUNCTION habmm_deinit_queue

@brief  Deinit HAM message queue

@param [in] pQueueInfo

@dependencies
  None

@return
  Returns void

===========================================================================*/
void habmm_deinit_queue(IoctlHabmmQueueInfo* pQueueInfo)
{
    int i = 0;

    for (i = 0; i < MAX_NUM_MSG_QUEUE; i++)
    {
        if ( NULL != *(pQueueInfo->pBufferList+ i) )
        {
            HYP_VIDEO_MSG_HIGH("free message buffer queue");
            HABMM_FREE( *(pQueueInfo->pBufferList+ i) );
            *(pQueueInfo->pBufferList+ i) = NULL;
        }
    }
    for (i = 0; i < MAX_NUM_SIGNAL; i++)
    {
        if (NULL != pQueueInfo->MsgSignalQ[i].stateChangeSignal)
        {
            MM_Signal_Reset (pQueueInfo->MsgSignalQ[i].stateChangeSignal);
            MM_Signal_Release (pQueueInfo->MsgSignalQ[i].stateChangeSignal);
            pQueueInfo->MsgSignalQ[i].stateChangeSignal = 0;
        }

        if (NULL != pQueueInfo->MsgSignalQ[i].stateSignalQ)
        {
            MM_SignalQ_Release ( pQueueInfo->MsgSignalQ[i].stateSignalQ );
            pQueueInfo->MsgSignalQ[i].stateSignalQ = 0;
        }
    }

}

/**===========================================================================

FUNCTION hyp_queue_init

@brief  Initialize hyp video generic queue data stucture

@param [in] queue pointer
@param [in] data_size
@param [in] queue_size

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type hyp_queue_init(hyp_queue_type *queue, uint32 data_size,
                                uint32 queue_size)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    queue->data = malloc(data_size * queue_size);
    if (NULL == queue->data)
    {
        HYP_VIDEO_MSG_ERROR("queue malloc failure");
        rc = HYPV_STATUS_FAIL;
    }
    else
    {
        if (pthread_mutex_init(&queue->lock, NULL))
        {
            free(queue->data);
            HYP_VIDEO_MSG_ERROR("error in pthread mutex init");
            rc = HYPV_STATUS_FAIL;
        }
        else
        {
            queue->front = queue->rear = 0;
            queue->queue_size = queue_size;
            queue->data_size = data_size;
            HYP_VIDEO_MSG_INFO("queue init success. queue %p data_size = %u queue_size = %u",
                                queue, data_size, queue_size);
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION hyp_enqueue

@brief  Enqueue data into a queue

@param [in] queue pointer
@param [in] data

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type hyp_enqueue(hyp_queue_type *queue, void *data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    pthread_mutex_lock(&queue->lock);
    if (NULL == queue->data)
    {
        HYP_VIDEO_MSG_ERROR("failed to enqueue %p. queue data is NULL", queue);
        rc = HYPV_STATUS_FAIL;
    }
    else if (queue->front == ((queue->rear + 1) % queue->queue_size))
    {
        HYP_VIDEO_MSG_ERROR("failed to enqueue %p. queue is full", queue);
        rc = HYPV_STATUS_FAIL;
    }
    else
    {
        HYP_VIDEO_MSG_INFO("enqueue %p  to index = %u size = %u",
                            queue, queue->rear, queue->data_size);
        memcpy((void *)((uint8 *)queue->data + (queue->data_size * queue->rear)),
               data, queue->data_size);
        ++queue->rear %= queue->queue_size;
    }
    pthread_mutex_unlock(&queue->lock);

    return rc;
}

/**===========================================================================

FUNCTION hyp_dequeue

@brief  Dequeue data from a queue

@param [in] queue pointer
@param [out] data

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type hyp_dequeue(hyp_queue_type *queue, void *data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    pthread_mutex_lock(&queue->lock);
    if (NULL == queue->data)
    {
        HYP_VIDEO_MSG_ERROR("failed to dequeue %p. queue data is NULL", queue);
        rc = HYPV_STATUS_FAIL;
    }
    else if (queue->front == queue->rear)
    {
        HYP_VIDEO_MSG_HIGH("failed to dequeue %p. queue is empty", queue);
        rc = HYPV_STATUS_FAIL;
    }
    else
    {
        HYP_VIDEO_MSG_INFO("dequeue %p from index = %u size = %u",
                            queue, queue->front, queue->data_size);
        memcpy(data, (void *)((uint8 *)queue->data + (queue->data_size * queue->front)),
               queue->data_size);
        ++queue->front %= queue->queue_size;
    }
    pthread_mutex_unlock(&queue->lock);

    return rc;
}

/**===========================================================================

FUNCTION hyp_queue_deinit

@brief  Deinit hyp queue

@param [in] queue pointer

@dependencies
  None

@return
  Returns void

===========================================================================*/
void hyp_queue_deinit(hyp_queue_type *queue)
{
    HYP_VIDEO_MSG_INFO("Deinit queue %p data_size = %u queue_size = %u",
                        queue, queue->data_size, queue->queue_size);

    pthread_mutex_lock(&queue->lock);
    if (NULL != queue->data)
    {
        free(queue->data);
    }
    pthread_mutex_unlock(&queue->lock);

    if (pthread_mutex_destroy(&queue->lock))
    {
        HYP_VIDEO_MSG_ERROR("error in pthread mutex destroy");
    }

    return;
}

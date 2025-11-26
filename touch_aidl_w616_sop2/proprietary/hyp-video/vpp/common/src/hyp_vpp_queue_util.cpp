/*===========================================================================

*//** @file hyp_queue_utility.c
This file implements utility functions for enqueue & dequeue

Copyright (c) 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/

#include <pthread.h>
#include "hyp_vpp_queue_util.h"
#include "hyp_vpppriv.h"

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
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type habmm_init_queue(habmm_queue_info_type* queue_info,
                                  unsigned int msg_buf_count, int num_queue, int num_signal)
{
    int nQsize, i;
    hypvpp_status_type rc = HYPVPP_STATUS_SUCCESS;

    if ((num_queue > MAX_NUM_MSG_QUEUE) || (num_signal > MAX_NUM_SIGNAL))
    {
        HYP_VPP_MSG_ERROR("invalid input nQueue %d nSignal %d", num_queue, num_signal);
        rc = HYPVPP_STATUS_FAIL;
    }

    if ((0 < msg_buf_count) && (HYPVPP_STATUS_SUCCESS == rc))
    {
        for (i = 0; i < num_queue; i++)
        {
            /** (+1): index handling for when the queue is full */
            queue_info->total_msg_buf_count[i] = msg_buf_count + 1;

            nQsize = queue_info->total_msg_buf_count[i] * sizeof( BufferList);

            queue_info->p_buffer_list[i] = ( BufferList *)HABMM_MALLOC(nQsize);

            if (NULL ==  queue_info->p_buffer_list[i])
            {
                HYP_VPP_MSG_ERROR("message buffer queue alloc failed, out of memory");
                rc = HYPVPP_STATUS_FAIL;
            }
            else
            {
                HABMM_MEMSET(queue_info->p_buffer_list[i], 0, nQsize);
                queue_info->msg_queue_in_index[i] = 0;
                queue_info->msg_queue_out_index[i] = 0;
            }
        }
        for (i = 0; ( i < num_signal) && (rc == HYPVPP_STATUS_SUCCESS); i++)
        {
            /** Create signalQ and signal for I/O and callback message handling */
            if (MM_SignalQ_Create ( &queue_info->msg_signal_queue[i].state_signal_queue) != 0)
            {
                HYP_VPP_MSG_ERROR("failed to create SignalQ idx %d", i);
                rc = HYPVPP_STATUS_FAIL;
            }
            else if (MM_Signal_Create (queue_info->msg_signal_queue[i].state_signal_queue,
                        &queue_info->msg_signal_queue[i].state_change_signal,
                        NULL,
                        &queue_info->msg_signal_queue[i].state_change_signal) != 0)
            {
                HYP_VPP_MSG_ERROR("failed to create Signal idx %d", i);
                rc = HYPVPP_STATUS_FAIL;
            }
        }
    }
    else
    {
        HYP_VPP_MSG_ERROR("bad parameter, zero message buffer count");
        rc = HYPVPP_STATUS_FAIL;
    }

    if (rc != HYPVPP_STATUS_SUCCESS)
    {
        habmm_deinit_queue(queue_info);
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
boolean habmm_enqueue_buffer_list(habmm_queue_info_type* queue_info, int mode, habmm_msg_desc_t *node)
{
    boolean ret = TRUE;
    BufferList *pTmpMsgQ = queue_info->p_buffer_list[mode];

    if (((queue_info->msg_queue_in_index[mode] + 1) % queue_info->total_msg_buf_count[mode])
            == queue_info->msg_queue_out_index[mode])
    {

        HYP_VPP_MSG_ERROR("failed %d is empty in index %u out index %u total %u",
                          mode, queue_info->msg_queue_in_index[mode], queue_info->msg_queue_out_index[mode],
                          queue_info->total_msg_buf_count[mode] );
        ret = FALSE;
    }
    else
    {
        HABMM_MEMCPY(&pTmpMsgQ[queue_info->msg_queue_in_index[mode]], node, sizeof(habmm_msg_desc_t));
        queue_info->msg_queue_in_index[mode] = (queue_info->msg_queue_in_index[mode] + 1)
                                               % queue_info->total_msg_buf_count[mode];
        HYP_VPP_MSG_LOW("nMsgQueueInIndex %u", queue_info->msg_queue_in_index[mode]);
    }

    return ret;
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
habmm_msg_desc_t* habmm_dequeue_buffer_list(habmm_queue_info_type* queue_info, int mode)
{
    habmm_msg_desc_t *buf_node = NULL;
    BufferList *buf_list = queue_info->p_buffer_list[mode];


    if (queue_info->msg_queue_in_index[mode] == queue_info->msg_queue_out_index[mode])
    {
        HYP_VPP_MSG_LOW("message buffer queue %d is empty index %u", mode,
                        queue_info->msg_queue_in_index[mode]);
    }
    else
    {
        buf_node = &buf_list[queue_info->msg_queue_out_index[mode]];
        queue_info->msg_queue_out_index[mode] = (queue_info->msg_queue_out_index[mode] + 1)
                                                % queue_info->total_msg_buf_count[mode];
        HYP_VPP_MSG_LOW("%d In %u Out %u", mode,
                queue_info->msg_queue_in_index[mode], queue_info->msg_queue_out_index[mode]);
    }

    return buf_node;
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
void habmm_deinit_queue(habmm_queue_info_type* queue_info)
{
    int i = 0;

    for (i = 0; i < MAX_NUM_MSG_QUEUE; i++)
    {
        if (NULL != *(queue_info->p_buffer_list+ i))
        {
            HYP_VPP_MSG_HIGH("free message buffer queue");
            HABMM_FREE( *(queue_info->p_buffer_list+ i) );
            *(queue_info->p_buffer_list+ i) = NULL;
        }
    }

    for (i = 0; i < MAX_NUM_SIGNAL; i++)
    {
        if (NULL != queue_info->msg_signal_queue[i].state_change_signal)
        {
            MM_Signal_Reset (queue_info->msg_signal_queue[i].state_change_signal);
            MM_Signal_Release (queue_info->msg_signal_queue[i].state_change_signal);
            queue_info->msg_signal_queue[i].state_change_signal = 0;
        }

        if (NULL != queue_info->msg_signal_queue[i].state_signal_queue)
        {
            MM_SignalQ_Release ( queue_info->msg_signal_queue[i].state_signal_queue );
            queue_info->msg_signal_queue[i].state_signal_queue = 0;
        }
    }
}

/**===========================================================================

  FUNCTION hyp_queue_init

  @brief  Initialize hyp VPP generic queue data stucture

  @param [in] queue pointer
  @param [in] data_size
  @param [in] queue_size

  @dependencies
  None

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type hyp_queue_init(hyp_queue_type *queue, uint32 data_size,
        uint32 queue_size)
{
    hypvpp_status_type rc = HYPVPP_STATUS_SUCCESS;

    queue->data = malloc(data_size * queue_size);
    if (NULL == queue->data)
    {
        HYP_VPP_MSG_ERROR("queue malloc failure");
        rc = HYPVPP_STATUS_FAIL;
    }
    else
    {
        if (pthread_mutex_init(&queue->lock, NULL))
        {
            free(queue->data);
            HYP_VPP_MSG_ERROR("error in pthread mutex init");
            rc = HYPVPP_STATUS_FAIL;
        }
        else
        {
            queue->front = queue->rear = 0;
            queue->queue_size = queue_size;
            queue->data_size = data_size;
            HYP_VPP_MSG_INFO("queue init success. queue %p data_size = %u queue_size = %u",
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
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type hyp_enqueue(hyp_queue_type *queue, void *data)
{
    hypvpp_status_type rc = HYPVPP_STATUS_SUCCESS;

    pthread_mutex_lock(&queue->lock);
    if (NULL == queue->data)
    {
        HYP_VPP_MSG_ERROR("failed to enqueue %p. queue data is NULL", queue);
        rc = HYPVPP_STATUS_FAIL;
    }
    else if (queue->front == ((queue->rear + 1) % queue->queue_size))
    {
        HYP_VPP_MSG_ERROR("failed to enqueue %p. queue is full", queue);
        rc = HYPVPP_STATUS_FAIL;
    }
    else
    {
        HYP_VPP_MSG_INFO("enqueue %p  to index = %u size = %u",
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
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type hyp_dequeue(hyp_queue_type *queue, void *data)
{
    hypvpp_status_type rc = HYPVPP_STATUS_SUCCESS;

    pthread_mutex_lock(&queue->lock);
    if (NULL == queue->data)
    {
        HYP_VPP_MSG_ERROR("failed to dequeue %p. queue data is NULL", queue);
        rc = HYPVPP_STATUS_FAIL;
    }
    else if (queue->front == queue->rear)
    {
        HYP_VPP_MSG_HIGH("failed to dequeue %p. queue is empty", queue);
        rc = HYPVPP_STATUS_FAIL;
    }
    else
    {
        HYP_VPP_MSG_INFO("dequeue %p from index = %u size = %u",
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
    HYP_VPP_MSG_INFO("Deinit queue %p data_size = %u queue_size = %u",
                     queue, queue->data_size, queue->queue_size);

    pthread_mutex_lock(&queue->lock);
    if (NULL != queue->data)
    {
        free(queue->data);
    }
    pthread_mutex_unlock(&queue->lock);

    if (pthread_mutex_destroy(&queue->lock))
    {
        HYP_VPP_MSG_ERROR("error in pthread mutex destroy");
    }

    return;
}

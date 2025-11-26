/*===========================================================================

*//** @file hyp_queue_utility.h
This file declares utility functions for enqueue & dequeue

Copyright (c) 2016-2019, 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/


/*===========================================================================
                             Edit History

$Header:$

when(mm/dd/yy)     who         what, where, why
-------------      --------    -------------------------------------------------------
06/02/23           mm          Fix stop time out issue
04/11/19           sm          Generic queue interface
02/05/19           rz          Bringup changes for 8155
07/30/18           sm          Increase message queue size
05/08/17           sm          Update for new hyp-video architecture
08/16/16           rz          Update memset/memcpy Macros
07/26/16           rz          Check memory ptr before memeory free
06/01/16           hl          Add FE and BE to support Hypervisor interface

============================================================================*/

#ifndef __HYP_QUEUE_UTILITY_H__
#define __HYP_QUEUE_UTILITY_H__

#include <stdlib.h> /** For malloc/free */
#include <stdio.h>
#ifdef __QNXNTO__
#include "AEEStdDef.h"
#endif
#include "MMSignal.h"
#include "hyp_video.h"

#ifdef __cplusplus
extern "C" {
#endif

/*========================================================================
 Defines Macro
 ========================================================================*/
#define MAX_MSG_QUEUE_SIZE   50

/** I/O send: FE and BE
    I/O response: FE and BE
    Callback message: FE and mmi
 */
#define MAX_NUM_MSG_QUEUE  3
#define MAX_NUM_SIGNAL  4

// HABMM queues and signals
#define HABMM_IO_SEND_INDEX     0
#define HABMM_IO_RESPONSE_INDEX 1
#define NUM_HABMM_MSG_QUEUE     2

#define HABMM_SIGNAL_IO_SEND        0
#define HABMM_SIGNAL_IO_RESPONSE    1
#define NUM_HABMM_SIGNAL            2

// HVFE queues and signals
#define HABMM_IO_CALLBACK_INDEX     0
#define NUM_HVFE_MSG_QUEUE          1

#define HABMM_SIGNAL_RESPONSE_ACK       0
#define NUM_HVFE_SIGNAL                 1

 /*========================================================================
 Enumerations
 ========================================================================*/

/*========================================================================
 Data Types
 ========================================================================*/
/** @struct habmm_msg_desc_t
 *  @brief  Structure containing the descriptor of message type
 */


#define habmm_msg_desc_t hypvideo_msg_type

/*========================================================================
 Defines structure
 ========================================================================*/
/*
typedef struct _BufferList
{
    habmm_msg_desc_t msg_desc;
} BufferList;
*/
#define BufferList habmm_msg_desc_t
typedef struct
{
    MM_HANDLE stateSignalQ;
    MM_HANDLE stateChangeSignal;
} habmmSignalQ;

typedef struct _IoctlHabmmQueueInfo
{
    BufferList* pBufferList[MAX_NUM_MSG_QUEUE];
    unsigned int nMsgQueueInIndex[MAX_NUM_MSG_QUEUE];
    unsigned int nMsgQueueOutIndex[MAX_NUM_MSG_QUEUE];
    unsigned int nTotalMsgBufferCount[MAX_NUM_MSG_QUEUE];
    habmmSignalQ MsgSignalQ[MAX_NUM_SIGNAL];
} IoctlHabmmQueueInfo;

typedef struct {
    void *data;
    uint32 data_size;
    uint32 queue_size;
    uint32 front;
    uint32 rear;
    pthread_mutex_t lock;
} hyp_queue_type;

/*========================================================================
 Defines Macros
 ========================================================================*/

/*========================================================================
 Defines function
 ========================================================================*/
hypv_status_type habmm_init_queue(IoctlHabmmQueueInfo* pQueueInfo, unsigned int nMsgBufferCount, int nQueue, int nSignal);
boolean habmm_enqueue_buffer_list(IoctlHabmmQueueInfo* pQueueInfo, int mode, habmm_msg_desc_t *pNode);
habmm_msg_desc_t* habmm_dequeue_buffer_list(IoctlHabmmQueueInfo* pQueueInfo, int mode);
void habmm_deinit_queue(IoctlHabmmQueueInfo* pQueueInfo);
hypv_status_type hyp_queue_init(hyp_queue_type *queue, uint32 data_size, uint32 queue_size);
hypv_status_type hyp_enqueue(hyp_queue_type *queue, void *data);
hypv_status_type hyp_dequeue(hyp_queue_type *queue, void *data);
void hyp_queue_deinit(hyp_queue_type *queue);

#ifdef __cplusplus
}
#endif

#endif /* __HYP_QUEUE_UTILITY_H__ */

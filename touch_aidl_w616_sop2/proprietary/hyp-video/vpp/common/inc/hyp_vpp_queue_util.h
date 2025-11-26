/*===========================================================================

*//** @file hyp_vpp_queue_util.h
This file declares utility functions for enqueue & dequeue

Copyright (c) 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/

#ifndef __HYP_VPP_QUEUE_UTILITY_H__
#define __HYP_VPP_QUEUE_UTILITY_H__

#include <stdlib.h>
#include <stdio.h>

#include <MMSignal.h>
#include "hyp_vppinf.h"
#include "hyp_vpp_debug.h"


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

#define BufferList habmm_msg_desc_t
typedef struct
{
    MM_HANDLE state_signal_queue;
    MM_HANDLE state_change_signal;
} habmm_signal_queue;

typedef struct _habmm_queue_info
{
    BufferList*  p_buffer_list[MAX_NUM_MSG_QUEUE];
    unsigned int msg_queue_in_index[MAX_NUM_MSG_QUEUE];
    unsigned int msg_queue_out_index[MAX_NUM_MSG_QUEUE];
    unsigned int total_msg_buf_count[MAX_NUM_MSG_QUEUE];
    habmm_signal_queue msg_signal_queue[MAX_NUM_SIGNAL];
} habmm_queue_info_type;

typedef struct {
    void *data;
    uint32 data_size;
    uint32 queue_size;
    uint32 front;
    uint32 rear;
    pthread_mutex_t lock;
} hyp_queue_type;

/*========================================================================
 Defines function
 ========================================================================*/
hypvpp_status_type habmm_init_queue(habmm_queue_info_type* p_queue_info, unsigned int msg_buf_count, int num_queue, int num_signal);
boolean habmm_enqueue_buffer_list(habmm_queue_info_type* p_queue_info, int mode, habmm_msg_desc_t *p_node);
habmm_msg_desc_t* habmm_dequeue_buffer_list(habmm_queue_info_type* p_queue_info, int mode);
void habmm_deinit_queue(habmm_queue_info_type* p_queue_info);
hypvpp_status_type hyp_queue_init(hyp_queue_type *p_queue, uint32 data_size, uint32 queue_size);
hypvpp_status_type hyp_enqueue(hyp_queue_type *p_queue, void *data);
hypvpp_status_type hyp_dequeue(hyp_queue_type *p_queue, void *data);
void hyp_queue_deinit(hyp_queue_type *p_queue);

#ifdef __cplusplus
}
#endif

#endif /* __HYP_VPP_QUEUE_UTILITY_H__ */

/*===========================================================================

*//** @file hyp_vppinf.h
This file declares datatypes and prototypes for vpp hypervisor

Copyright (c) 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/

#ifndef __HYP_VPP_INF_H__
#define __HYP_VPP_INF_H__

#include "hyp_vpp.h"
#include "vpp.h"
#include <map>

typedef struct hypvpp_return_data
{
    hypvpp_status_type          return_status;
} hypvpp_return_data_type;

typedef struct hypvpp_flush_data
{
    enum vpp_port             port;
} hypvpp_flush_data_type;

typedef struct hypvpp_reconfig_data
{
    struct vpp_port_param     input_param;
    struct vpp_port_param     output_param;
} hypvpp_reconfig_data_type;

typedef struct hypvpp_vid_prop_data
{
    struct video_property     prop;
} hypvpp_vid_prop_data_type;

typedef struct hypvpp_init_data
{
    hypvpp_status_type        return_status;
    int64                     return_io_handle;
} hypvpp_init_data_type;

typedef struct hypvpp_buf_req_data
{
    hypvpp_status_type        return_status;
    vpp_requirements          buf_req_playload;
} hypvpp_buf_req_data_type;

typedef enum
{
    HYPVPP_EVT_RESP_INPUT_BUF_DONE   = 0x80000001,
    HYPVPP_EVT_RESP_OUTPUT_BUF_DONE  = 0x80000002,
} hypvpp_event_type;

typedef struct hypvpp_queue_buf_data
{
    vpp_buffer                buffer_data;
} hypvpp_queue_buf_data_type;

typedef struct hypvpp_set_ctrl_data
{
    hqv_control               ctrl;
} hypvpp_set_ctrl_data_type;

typedef struct hypvpp_set_param_data
{
    enum vpp_port             port;
    struct vpp_port_param     param;
} hypvpp_set_param_data_type;

typedef struct hypvpp_event_data
{
    vpp_event                 event;
} hypvpp_event_data_type;

typedef struct hypvpp_callback_data
{
    hypvpp_event_type         event_type;
    vpp_buffer                buffer_done;
} hypvpp_callback_data_type;

typedef struct hypvpp_msg_data
{
    hypvpp_init_data_type       init_data;
    hypvpp_set_ctrl_data_type   ctrl_data;
    hypvpp_set_param_data_type  param_data;
    hypvpp_return_data_type     return_data;
    hypvpp_event_data_type      event_data;
    hypvpp_callback_data_type   callback_data;
    hypvpp_buf_req_data_type    buf_req_data;
    hypvpp_flush_data_type      flush_data;
    hypvpp_queue_buf_data_type  buf_data;
    hypvpp_reconfig_data_type   reconfig_data;
    hypvpp_vid_prop_data_type   prop_data;
} hypvpp_msg_data_type;

typedef enum
{
    HYPVPP_MSGCMD_INIT          = 0x00000001,
    HYPVPP_MSGCMD_OPEN,
    HYPVPP_MSGCMD_SET_CTRL,
    HYPVPP_MSGCMD_SET_FRC_SEG,
    HYPVPP_MSGCMD_SET_PARAM,
    HYPVPP_MSGCMD_GET_BUF_REQ,
    HYPVPP_MSGCMD_QUEUE_INPUT_BUF,
    HYPVPP_MSGCMD_QUEUE_OUTPUT_BUF,
    HYPVPP_MSGCMD_RECONFIG,
    HYPVPP_MSGCMD_FLUSH,
    HYPVPP_MSGCMD_DRAIN,
    HYPVPP_MSGCMD_SET_VID_PROP,
    HYPVPP_MSGCMD_CLOSE,
    HYPVPP_MSGCMD_TERM,

    HYPVPP_MSGRESP_INIT_RET     = 0x80008001,
    HYPVPP_MSGRESP_OPEN_RET,
    HYPVPP_MSGRESP_SET_CTRL_RET,
    HYPVPP_MSGRESP_SET_PARAM_RET,
    HYPVPP_MSGRESP_GET_BUF_REQ_RET,
    HYPVPP_MSGRESP_QUEUE_INPUT_BUF_RET,
    HYPVPP_MSGRESP_QUEUE_OUTPUT_BUF_RET,
    HYPVPP_MSGRESP_RECONFIG_RET,
    HYPVPP_MSGRESP_FLUSH_RET,
    HYPVPP_MSGRESP_DRAIN_RET,
    HYPVPP_MSGRESP_SET_VID_PROP_RET,
    HYPVPP_MSGRESP_CLOSE_RET,
    HYPVPP_MSGRESP_TERM_RET,
    HYPVPP_MSGRESP_EVENT,
    HYPVPP_MSGRESP_CALLBACK,
    HYPVPP_MSGRESP_MAX,
} hypvpp_msg_id_type;

/***************************************************************************
 *
 * hypvpp_msg_type
 *
 * Define the VPP command message, respond, and event messages
 *
 ***************************************************************************
 */
typedef struct
{
    uint32 version;

    /* the virtual_channle is the abstract handle returned from HAB open */
    uint32 virtual_channel;

    /* identify different message */
    uint32 msg_id;

    /* message number in sequence for debug purpose */
    uint32 message_number;

    /* the send and receive time in ns for debug purpose */
    uint32 time_stamp_ns;

    /* the size of supporting data in unit of bytes */
    uint32 data_size;

    /* pid of the client */
    uint32 pid;

    /* the payload corresponding to the token */
    hypvpp_msg_data_type data;
} hypvpp_msg_type;

#define habmm_msg_desc_t hypvpp_msg_type

#endif /* __HYP_VPP_INF_H__ */

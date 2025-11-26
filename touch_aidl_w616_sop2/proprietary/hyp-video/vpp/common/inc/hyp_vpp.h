/*========================================================================

*//** @file hyp_vpp.h
Hypervisor vpp common interface definitions. The file defines Hypervisor
vpp communication messages and the data structures.

Copyright (c) 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */

#ifndef __HYP_VPP_H__
#define __HYP_VPP_H__

#include <AEEStdDef.h>
#include "hyp_vpp_debug.h"
#include "vpp.h"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  hypvpp_buffer_type
  ----------------------------------------------------------------------------*/
/** This data type lists the supported buffer types. */
typedef enum
{
   /** Input frame buffer */
   HYPVPP_BUFFER_INPUT              = 0x00000001,

   /** Output frame buffer */
   HYPVPP_BUFFER_OUTPUT             = 0x00000002,

   /** Extra data buffer associated with HYPVPP_BUFFER_INPUT */
   HYPVPP_BUFFER_EXTRA_DATA_INPUT   = 0x01000001,

   /** Extra data buffer associated with HYPVPP_BUFFER_OUTPUT */
   HYPVPP_BUFFER_EXTRA_DATA_OUTPUT  = 0x01000002,

   HYPVPP_BUFFER_UNUSED             = 0x10000000
} hypvpp_buffer_type;

/* Hypervisor status type */
typedef enum
{
   HYPVPP_STATUS_FAIL = -1,
   HYPVPP_STATUS_SUCCESS = 0x0,
   HYPVPP_STATUS_PENDING,
   HYPVPP_STATUS_INVALID_STATE,
   HYPVPP_STATUS_INVALID_CONFIG,
   HYPVPP_STATUS_BAD_PARAMETER,
   HYPVPP_STATUS_NO_MEM,
   HYPVPP_STATUS_NO_RESOURCE,
   HYPVPP_STATUS_HW_ERROR,
   HYPVPP_STATUS_FATAL,
   HYPVPP_STATUS_VERSION_MISMATCH,
   HYPVPP_STATUS_MAX = 0x7fffffff
} hypvpp_status_type;


void* hyp_vpp_init(uint32_t flags, vpp_callbacks callback);
hypvpp_status_type hyp_vpp_term(void* context);
hypvpp_status_type hyp_vpp_open(void* context);
hypvpp_status_type hyp_vpp_close(void* context);
hypvpp_status_type hyp_vpp_set_ctrl(void* context, struct hqv_control ctrl);
hypvpp_status_type hyp_vpp_set_param(void* context, enum vpp_port port, struct vpp_port_param param);
hypvpp_status_type hyp_vpp_queue_buf(void* context, enum vpp_port port, struct vpp_buffer *buf);
hypvpp_status_type hyp_vpp_get_buf_requirements(void* context, struct vpp_requirements *req);
hypvpp_status_type hyp_vpp_flush(void* context, enum vpp_port port);
hypvpp_status_type hyp_vpp_drain(void* context);
hypvpp_status_type hyp_vpp_reconfigure(void* context, struct vpp_port_param input_param,
                                 struct vpp_port_param output_param);
hypvpp_status_type hyp_vpp_set_vid_prop(void* context, struct video_property prop);
vpp_error hypvpp_status_to_vpp_error (hypvpp_status_type type);


#endif //__HYP_VPP_H__

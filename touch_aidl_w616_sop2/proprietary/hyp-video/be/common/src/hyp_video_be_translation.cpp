/*===========================================================================

*//** @file hyp_video_be_translation.cpp
This file implements message translation between video BE and habmm

Copyright (c) 2017-2019, 2021 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/

/*===========================================================================
                             Edit History

$Header: $

when(mm/dd/yy)     who         what, where, why
-------------      --------    -------------------------------------------------------
04/01/21           sh          Bringup video on RedBend Hypervisor
02/05/19           rz          Bringup changes for 8155
07/10/18           sm          Add support to use pmem handle for video buffers
01/18/18           sm          Add support for passthrough mode feature
11/29/17           sm          Add support for video input interrupts handling
06/28/17           aw          Unify and update all logs in hyp-video
06/23/17           sm          Streamline hypervisor context structure and definitions
05/08/17           sm          Update for new hyp-video architecture
12/13/16           hl          Support hypervisor encoding and port changes from AGL
08/16/16           rz          Dynamically loading hab library
07/26/16           rz          Handle contiguous memory mapping for QNX OMX
07/08/16           hl          Isolate video data from ioctl
06/22/16           henryl      Add dynamic buffer mode support
06/20/16           henryl      Clean up and add error messages
06/09/16           henryl      Add support linux hypervisor for non-contiguous metadata
06/01/16           hl          Add FE and BE to support Hypervisor interface

============================================================================*/
#include "hyp_vidc_types.h"
#include "hyp_vidc_inf.h"
#include "hyp_vinput_inf.h"
#include "hyp_videopriv.h"
#include "hyp_video.h"
#include "hyp_debug.h"
#include "hyp_video_be_translation.h"
#include "hyp_buffer_manager.h"

/**===========================================================================

FUNCTION translate_habmm_to_hvbe

@brief  Translate message from HAB to hypervisor BE

@param [in] hypv session pointer
@param [in] msg id
@param [in] input buffer pointer
@param [out] output buffer pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type translate_habmm_to_hvbe(hypv_session_t* hypv_session, int msg_id, void* aInBuf, void* aOutBuf)
{
    habmm_msg_desc_t* habmm_msg = (habmm_msg_desc_t*)aInBuf;
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    UNUSED(hypv_session);

    if(HYPVIDEO_MSGRESP_IOCTL_RET == (hypvideo_msg_id_type)msg_id)
    {
        if (NULL == aOutBuf)
        {
            rc = HYPV_STATUS_FAIL;
            HYP_VIDEO_MSG_ERROR("output buffer is NULL");
        }
        else
        {
            switch ( habmm_msg->data.ioctl_data.vidc_ioctl )
            {
                /** Convert 32b struct to vidc_buffer_info_64b_type */
            case VIDC_IOCTL_ALLOCATE_BUFFER:
            case VIDC_IOCTL_SET_BUFFER:
            case VIDC_IOCTL_FREE_BUFFER:
                {
                    vidc_buffer_info_64b_type* pCmdBuffer64b;
                    vidc_buffer_info_type* pCmdBuffer;

                    pCmdBuffer64b = (vidc_buffer_info_64b_type*)habmm_msg->data.ioctl_data.payload;
                    pCmdBuffer = (vidc_buffer_info_type*)aOutBuf;
                    pCmdBuffer->buf_addr = (uint8*)pCmdBuffer64b->buf_addr;
                    pCmdBuffer->buf_size = pCmdBuffer64b->buf_size;
                    pCmdBuffer->buf_type = pCmdBuffer64b->buf_type;
                    pCmdBuffer->contiguous = pCmdBuffer64b->contiguous;
                    pCmdBuffer->extradata_buf_size = pCmdBuffer64b->extradata_buf_size;
                    pCmdBuffer->extradata_buf_addr = (uint8*)pCmdBuffer64b->extradata_buf_addr;
                    break;
                }
            case VIDC_IOCTL_EMPTY_INPUT_BUFFER:
            case VIDC_IOCTL_FILL_OUTPUT_BUFFER:
                {
                    vidc_frame_data_64b_type* pCmdBuffer64b;
                    vidc_frame_data_type* pCmdBuffer;

                    pCmdBuffer64b = (vidc_frame_data_64b_type*)habmm_msg->data.ioctl_data.payload;
                    pCmdBuffer = (vidc_frame_data_type*)aOutBuf;
                    pCmdBuffer->frame_addr = (uint8*)pCmdBuffer64b->frame_addr;
                    pCmdBuffer->metadata_addr = (uint8*)pCmdBuffer64b->metadata_addr;
                    pCmdBuffer->alloc_len = pCmdBuffer64b->alloc_len;
                    pCmdBuffer->data_len = pCmdBuffer64b->data_len;
                    pCmdBuffer->offset = pCmdBuffer64b->offset;
                    pCmdBuffer->timestamp = pCmdBuffer64b->timestamp;
                    pCmdBuffer->flags = pCmdBuffer64b->flags;
                    pCmdBuffer->frm_clnt_data = pCmdBuffer64b->frm_clnt_data;
                    pCmdBuffer->frame_decsp = pCmdBuffer64b->frame_decsp;
                    pCmdBuffer->frame_type = pCmdBuffer64b->frame_type;
                    pCmdBuffer->buf_type = pCmdBuffer64b->buf_type;
                    pCmdBuffer->mark_target = pCmdBuffer64b->mark_target;
                    pCmdBuffer->mark_data = pCmdBuffer64b->mark_data;
                    pCmdBuffer->alloc_metadata_len = pCmdBuffer64b->alloc_metadata_len;
                    pCmdBuffer->non_contiguous_metadata = pCmdBuffer64b->non_contiguous_metadata;
                    break;
                }
            case VIDC_PING_GET_CALLBACK:
                {
                    vidc_drv_msg_info_64b_type *pMsgInfo64;
                    vidc_drv_msg_info_type *pMsgInfo;

                    pMsgInfo64 = (vidc_drv_msg_info_64b_type *)habmm_msg->data.ioctl_data.payload;
                    pMsgInfo = (vidc_drv_msg_info_type *)aOutBuf;

                    pMsgInfo->status = ( pMsgInfo64->status == HYPV_STATUS_SUCCESS ) ? VIDC_ERR_NONE : VIDC_ERR_FAIL;
                    pMsgInfo->event_type = (vidc_event_type) pMsgInfo64->event_type;
                    pMsgInfo->payload.frame_data.data_len = pMsgInfo64->payload.frame_data.data_len;
                    pMsgInfo->payload.frame_data.frm_clnt_data = pMsgInfo64->payload.frame_data.frm_clnt_data;
                    pMsgInfo->payload.frame_data.data_len = pMsgInfo64->payload.frame_data.data_len;
                    pMsgInfo->payload.frame_data.flags = pMsgInfo64->payload.frame_data.flags;
                    pMsgInfo->payload.frame_data.frame_addr = (uint8*)pMsgInfo64->payload.frame_data.frame_addr;
                    pMsgInfo->payload.frame_data.metadata_addr = (uint8*)pMsgInfo64->payload.frame_data.metadata_addr;
                    break;
                }
            default:
                {
                    /** Get property*/
                    HABMM_MEMCPY(aOutBuf, habmm_msg->data.ioctl_data.payload, GET_IOCTL_PAYLOAD_SIZE_FROM_MSG_DATASIZE(habmm_msg->data_size));
                    break;
                }
            }
        }
    }
    else
    {
        rc = HYPV_STATUS_FAIL;
        HYP_VIDEO_MSG_ERROR("invalid msg_id %d",msg_id);
    }
    return rc;
}

/**===========================================================================

FUNCTION translate_hvbe_to_habmm

@brief  Translate message from hypervisor BE to HAB

@param [in] hypv session pointer
@param [in] msg id
@param [in] input buffer pointer
@param [out] output buffer pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type translate_hvbe_to_habmm(hypv_session_t* hypv_session, int msg_id, void* aInBuf, void* aOutBuf)
{
    hypv_lookup_table_t *map_queue = &hypv_session->lookup_queue;
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    if(HYPVIDEO_MSGRESP_EVENT == (hypvideo_msg_id_type)msg_id)
    {
        habmm_msg_desc_t* msg2 = (habmm_msg_desc_t*)aOutBuf;
        hypv_map_entry_t* ret_buff;

        if (VIDEO_SESSION_VINPUT == hypv_session->video_session)
        {
            avin_drv_msg_type *pMsgInfo = ( avin_drv_msg_type*)aInBuf;
            msg2->data.event_data.vidc.status = pMsgInfo->status;
            msg2->data.event_data.vidc.event_type = (uint32)pMsgInfo->event;
            msg2->data.event_data.vidc.payload.event_data_1 = pMsgInfo->info.port_id;
        }
        else
        {
            vidc_drv_msg_info_type* pMsgInfo = ( vidc_drv_msg_info_type*)aInBuf;
            uint8* frame_addr = NULL;
            uint8* metadata_addr = NULL;

            msg2->data.event_data.vidc.status = ( pMsgInfo->status == VIDC_ERR_NONE ) ? HYPV_STATUS_SUCCESS : HYPV_STATUS_FAIL;
            msg2->data.event_data.vidc.event_type = pMsgInfo->event_type;
            msg2->data.event_data.vidc.payload.frame_data.alloc_len = pMsgInfo->payload.frame_data.alloc_len;
            msg2->data.event_data.vidc.payload.frame_data.buf_type = pMsgInfo->payload.frame_data.buf_type;
            msg2->data.event_data.vidc.payload.frame_data.data_len = pMsgInfo->payload.frame_data.data_len;
            msg2->data.event_data.vidc.payload.frame_data.flags = pMsgInfo->payload.frame_data.flags;

            if (NULL != pMsgInfo->payload.frame_data.frame_addr)
            {
                frame_addr = pMsgInfo->payload.frame_data.frame_addr;
                metadata_addr = pMsgInfo->payload.frame_data.metadata_addr;
            }
            else
            {
                frame_addr = pMsgInfo->payload.frame_data.frame_handle;
                metadata_addr = pMsgInfo->payload.frame_data.metadata_handle;
            }
            if (NULL != frame_addr)
            {
                ret_buff = hypv_map_to_lookup(map_queue, (void *)frame_addr);
                if (NULL != ret_buff)
                {
                    msg2->data.event_data.vidc.payload.frame_data.frame_addr = (uint64)ret_buff->bufferid;
                    if (VIDC_EVT_RESP_OUTPUT_DONE == pMsgInfo->event_type)
                    {
                        if ((hypv_session->hypvid_flags & HYPVID_FLAGS_USE_DYNAMIC_OUTPUT_BUFFER_MASK) &&
                            !(pMsgInfo->payload.frame_data.flags & VIDC_FRAME_FLAG_READONLY))
                        {
                            hypv_map_free(&hypv_session->habmm_if, &hypv_session->lookup_queue, frame_addr, ret_buff->bufferid);
                        }
                    }
                    else if (VIDC_EVT_RESP_INPUT_DONE == pMsgInfo->event_type)
                    {
                       if ( hypv_session->hypvid_flags & HYPVID_FLAGS_USE_DYNAMIC_INPUT_BUFFER_MASK )
                       {
                           hypv_map_free(&hypv_session->habmm_if, &hypv_session->lookup_queue, frame_addr, ret_buff->bufferid);
                       }
                    }
                }
                else
                {
                    rc = HYPV_STATUS_FAIL;
                    HYP_VIDEO_MSG_ERROR("failed to lookup entry node for frame addr 0x%p", (void *)pMsgInfo->payload.frame_data.frame_addr);
                }
            }
            else
            {
                msg2->data.event_data.vidc.payload.frame_data.frame_addr = 0;
            }
            msg2->data.event_data.vidc.payload.frame_data.frame_decsp = pMsgInfo->payload.frame_data.frame_decsp;
            msg2->data.event_data.vidc.payload.frame_data.frame_type = pMsgInfo->payload.frame_data.frame_type;
            msg2->data.event_data.vidc.payload.frame_data.frm_clnt_data = pMsgInfo->payload.frame_data.frm_clnt_data;
            msg2->data.event_data.vidc.payload.frame_data.mark_data = pMsgInfo->payload.frame_data.mark_data;
            msg2->data.event_data.vidc.payload.frame_data.mark_target = pMsgInfo->payload.frame_data.mark_target;
            msg2->data.event_data.vidc.payload.frame_data.alloc_metadata_len = pMsgInfo->payload.frame_data.alloc_metadata_len;
            msg2->data.event_data.vidc.payload.frame_data.non_contiguous_metadata = pMsgInfo->payload.frame_data.non_contiguous_metadata;

            if ( pMsgInfo->payload.frame_data.non_contiguous_metadata && NULL != metadata_addr)
            {
                ret_buff = hypv_map_to_lookup(map_queue, (void *)metadata_addr);
                if (NULL != ret_buff)
                {
                    msg2->data.event_data.vidc.payload.frame_data.metadata_addr = (uint64)ret_buff->bufferid;
                    if (VIDC_EVT_RESP_OUTPUT_DONE == pMsgInfo->event_type)
                    {
                        if ((hypv_session->hypvid_flags & HYPVID_FLAGS_USE_DYNAMIC_OUTPUT_BUFFER_MASK) &&
                           !(pMsgInfo->payload.frame_data.flags & VIDC_FRAME_FLAG_READONLY))
                        {
                            hypv_map_free(&hypv_session->habmm_if, &hypv_session->lookup_queue, metadata_addr, ret_buff->bufferid);
                        }
                    }
                }
            }
            else
            {
                msg2->data.event_data.vidc.payload.frame_data.metadata_addr = 0;
            }
            msg2->data.event_data.vidc.payload.frame_data.offset = pMsgInfo->payload.frame_data.offset;
            msg2->data.event_data.vidc.payload.frame_data.timestamp = pMsgInfo->payload.frame_data.timestamp;
        }
    }
    else
    {
        switch ( msg_id )
        {
            /** Convert 32b struct to vidc_buffer_info_64b_type */
        case VIDC_IOCTL_ALLOCATE_BUFFER:
        case VIDC_IOCTL_SET_BUFFER:
        case VIDC_IOCTL_FREE_BUFFER:
            {
                vidc_buffer_info_64b_type* pCmdBuffer64b;
                vidc_buffer_info_type* pCmdBuffer;

                pCmdBuffer = (vidc_buffer_info_type*)aInBuf;
                pCmdBuffer64b = (vidc_buffer_info_64b_type*)aOutBuf;
                pCmdBuffer64b->buf_addr = (uint64)pCmdBuffer->buf_addr;/** Not necessary */
                pCmdBuffer64b->buf_size = pCmdBuffer->buf_size;
                pCmdBuffer64b->buf_type = pCmdBuffer->buf_type;
                pCmdBuffer64b->contiguous = pCmdBuffer->contiguous;
                pCmdBuffer64b->extradata_buf_size = pCmdBuffer->extradata_buf_size;
                pCmdBuffer64b->extradata_buf_addr = (uint64)pCmdBuffer->extradata_buf_addr;/** Not necessary */
                break;
            }

        default:
            {
                break;
            }
        }
    }
    return rc;
}

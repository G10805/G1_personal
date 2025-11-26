/*===========================================================================

*//** @file hyp_vpp_fe_translation.c
This file implements message translation between VPP FE and habmm

Copyright (c) 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/

#include "hyp_vppinf.h"
#include "hyp_vpp_buf_manager.h"
#include "hyp_vpppriv.h"
#include "hyp_vpp.h"
#include "hyp_vpp_fe_translation.h"
#include "hyp_vpp_debug.h"

void increase_buf_req(vpp_requirements* req)
{
    uint32_t old_req = 0;

    if (req)
    {
        for (int i = 0; i < VPP_RESOLUTION_MAX; i++)
        {
            old_req = req->buf_req[i].in_req;
            if ((req->buf_req[i].in_req + EXTRA_BUF_REQ) <= BUF_REQ_LIMIT)
            {
                req->buf_req[i].in_req += EXTRA_BUF_REQ;
            }
            else
            {
                req->buf_req[i].in_req = BUF_REQ_LIMIT;
            }
            HYP_VPP_MSG_LOW("updated buf req[%d].in_req from %u to %u", i, old_req, req->buf_req[i].in_req);

            old_req = req->buf_req[i].out_req;
            if ((req->buf_req[i].out_req + EXTRA_BUF_REQ) <= BUF_REQ_LIMIT)
            {
                req->buf_req[i].out_req += EXTRA_BUF_REQ;
            }
            else
            {
                req->buf_req[i].out_req = BUF_REQ_LIMIT;
            }
            HYP_VPP_MSG_LOW("updated buf req[%d].out_req from %u to %u", i, old_req, req->buf_req[i].out_req);
        }
    }
    else
    {
        HYP_VPP_MSG_ERROR("invalid input parameter");
    }

}

/**===========================================================================

  FUNCTION translate_hvfe_to_habmm

  @brief  Translate message from hypervisor FE to HAB

  @param [in] hyp vpp session pointer
  @param [in] msg id
  @param [in] input buffer pointer
  @param [in] input buffer size
  @param [out] output buffer pointer

  @dependencies
  None

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type translate_hvfe_to_habmm(hypvpp_session_t* hypvpp_session, int msg_id,
        void* pInBuf, int nInSize, void* pOutBuf)
{
    hypvpp_status_type rc = HYPVPP_STATUS_SUCCESS;

    switch (msg_id)
    {
        case HYPVPP_MSGCMD_QUEUE_INPUT_BUF:
        case HYPVPP_MSGCMD_QUEUE_OUTPUT_BUF:
            {
                vpp_buffer* in_buf = (vpp_buffer*)pInBuf;
                vpp_buffer* out_buf = (vpp_buffer*)pOutBuf;
                HABMM_MEMSET(out_buf, 0, sizeof(vpp_buffer));

                uint32 export_id = 0;
                hypvpp_buffer_type buf_type;

                buf_type = msg_id == HYPVPP_MSGCMD_QUEUE_INPUT_BUF? HYPVPP_BUFFER_INPUT : HYPVPP_BUFFER_OUTPUT;
                /* Frame buffer */
                if (nInSize < (int)sizeof(vpp_buffer))
                {
                    rc = HYPVPP_STATUS_FAIL;
                    HYP_VPP_MSG_ERROR("size of msg is too small cur:%d expect:%zu", nInSize, sizeof(vpp_buffer));
                }
                else
                {
                    HABMM_MEMCPY(out_buf, in_buf, sizeof(vpp_buffer));
                }

                if (HYPVPP_STATUS_SUCCESS == rc)
                {
                    export_id = hypvpp_map_add_to_remote(&hypvpp_session->habmm_if,
                            &hypvpp_session->lookup_queue,
                            hypvpp_session->habmm_handle,
                            in_buf->pixel.fd,
                            in_buf->pixel.alloc_len,
                            buf_type);
                    if (0 == export_id)
                    {
                        rc = HYPVPP_STATUS_FAIL;
                        HYP_VPP_MSG_ERROR("failed to export, msg id 0x%x fd %d size %u buf type 0x%x",
                                (unsigned int)msg_id, in_buf->pixel.fd, in_buf->pixel.alloc_len, (unsigned int)buf_type);
                    }
                    else
                    {
                        /* store the share id in member fd */
                        out_buf->pixel.fd = (int)export_id;
                    }
                }
                break;
            }
        default:
            {
                /** Get & Set property - type conversion is not required */
                HABMM_MEMCPY(pOutBuf, pInBuf, nInSize);
                break;
            }
    }

    return rc;
}

/**===========================================================================

  FUNCTION translate_habmm_to_hvfe

  @brief  Translate message from HAB to hypervisor FE

  @param [in] hyp vpp session pointer
  @param [in] msg id
  @param [in] input buffer pointer
  @param [in] input buffer size
  @param [out] output buffer pointer

  @dependencies
  None

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type translate_habmm_to_hvfe(hypvpp_session_t* hypvpp_session, int msg_id,
        void* pInBuf, int nInSize, void* pOutBuf)
{
    UNUSED(nInSize);

    hypvpp_status_type rc = HYPVPP_STATUS_SUCCESS;
    hypvpp_msg_data_type* hypvpp_msg = (hypvpp_msg_data_type*)pInBuf;
    hypvpp_msg_id_type msg_type = (hypvpp_msg_id_type)msg_id;

    if (NULL == pInBuf || NULL == pOutBuf)
    {
        rc = HYPVPP_STATUS_BAD_PARAMETER;
        HYP_VPP_MSG_ERROR("input param is incorrect, pInBuf:%p pOutBuf:%p", pInBuf, pOutBuf);
    }
    else
    {
        if (HYPVPP_MSGRESP_INIT_RET == msg_type)
        {
            hypvpp_init_data_type* value = (hypvpp_init_data_type*)pOutBuf;
            /** device handle */
            value->return_io_handle = hypvpp_msg->init_data.return_io_handle;
            value->return_status= hypvpp_msg->init_data.return_status;
        }
        else if (HYPVPP_MSGRESP_OPEN_RET == msg_type || HYPVPP_MSGRESP_CLOSE_RET == msg_type
                || HYPVPP_MSGRESP_TERM_RET == msg_type || HYPVPP_MSGRESP_SET_CTRL_RET == msg_type
                || HYPVPP_MSGRESP_SET_PARAM_RET == msg_type || HYPVPP_MSGRESP_QUEUE_INPUT_BUF_RET == msg_type
                || HYPVPP_MSGRESP_QUEUE_OUTPUT_BUF_RET == msg_type || HYPVPP_MSGRESP_FLUSH_RET == msg_type
                || HYPVPP_MSGRESP_RECONFIG_RET == msg_type || HYPVPP_MSGRESP_SET_VID_PROP_RET == msg_type
                || HYPVPP_MSGRESP_DRAIN_RET == msg_type)
        {
            hypvpp_return_data_type* value = (hypvpp_return_data_type*)pOutBuf;
            value->return_status= hypvpp_msg->return_data.return_status;
        }
        else if (HYPVPP_MSGRESP_GET_BUF_REQ_RET == msg_type)
        {
            hypvpp_buf_req_data_type* out = (hypvpp_buf_req_data_type*)pOutBuf;
            out->return_status= hypvpp_msg->buf_req_data.return_status;
            HABMM_MEMCPY(&out->buf_req_playload, &hypvpp_msg->buf_req_data.buf_req_playload, sizeof(struct vpp_requirements));
            increase_buf_req(&out->buf_req_playload);

            HYP_VPP_MSG_INFO("get buffer requirement in_req_cnt:SD:%u HD:%u FHD:%u UHD:%u"
                    " out_req_cnt:SD:%u HD:%u FHD:%u UHD:%u",
                    out->buf_req_playload.buf_req[0].in_req, out->buf_req_playload.buf_req[1].in_req,
                    out->buf_req_playload.buf_req[2].in_req, out->buf_req_playload.buf_req[3].in_req,
                    out->buf_req_playload.buf_req[0].out_req, out->buf_req_playload.buf_req[1].out_req,
                    out->buf_req_playload.buf_req[2].out_req, out->buf_req_playload.buf_req[3].out_req);
        }
        else if (HYPVPP_MSGRESP_EVENT == msg_type)
        {
            hypvpp_event_data_type* out = (hypvpp_event_data_type*)pOutBuf;

            HABMM_MEMCPY(&out->event, &hypvpp_msg->event_data.event, sizeof(vpp_event));

            if (VPP_EVENT_FLUSH_DONE == hypvpp_msg->event_data.event.type)
            {
                HYP_VPP_MSG_INFO("flush done port:%s", out->event.flush_done.port == VPP_PORT_INPUT ? "input" : "output");
            }
            else if (VPP_EVENT_RECONFIG_DONE == hypvpp_msg->event_data.event.type)
            {
                HYP_VPP_MSG_INFO("reconfig done, supported color format:0x%lx", (unsigned long)out->event.port_reconfig_done.req.in_color_fmt_mask);
                increase_buf_req(&out->event.port_reconfig_done.req);
                HYP_VPP_MSG_INFO("new buffer requirement in_req_cnt:SD:%u HD:%u FHD:%u UHD:%u",
                        out->event.port_reconfig_done.req.buf_req[0].in_req,
                        out->event.port_reconfig_done.req.buf_req[1].in_req,
                        out->event.port_reconfig_done.req.buf_req[2].in_req,
                        out->event.port_reconfig_done.req.buf_req[3].in_req);
                HYP_VPP_MSG_INFO("new buffer requirement out_req_cnt:SD:%u HD:%u FHD:%u UHD:%u",
                        out->event.port_reconfig_done.req.buf_req[0].out_req,
                        out->event.port_reconfig_done.req.buf_req[1].out_req,
                        out->event.port_reconfig_done.req.buf_req[2].out_req,
                        out->event.port_reconfig_done.req.buf_req[3].out_req);
            }
            else if (VPP_EVENT_DRAIN_DONE == hypvpp_msg->event_data.event.type)
            {
                HYP_VPP_MSG_INFO("drain done");
            }
            else if (VPP_EVENT_ERROR == hypvpp_msg->event_data.event.type)
            {
                HYP_VPP_MSG_ERROR("VPP backend reports an error");
            }
            else
            {
                rc = HYPVPP_STATUS_FAIL;
                HYP_VPP_MSG_ERROR("VPP reports a unsupported event:0x%x", (unsigned int)out->event.type);
            }
        }
        else if (HYPVPP_MSGRESP_CALLBACK == msg_type)
        {
            hypvpp_callback_data_type* in_data = &hypvpp_msg->callback_data;
            hypvpp_callback_data_type* out_data = (hypvpp_callback_data_type*)pOutBuf;
            hypvpp_event_type event_type = in_data->event_type;
            vpp_buffer* in_buf = &in_data->buffer_done;
            vpp_buffer* out_buf = &out_data->buffer_done;
            uint32 share_id = 0;
            hypvpp_map_entry_t *find_entry;

            if (HYPVPP_EVT_RESP_INPUT_BUF_DONE == event_type
                    || HYPVPP_EVT_RESP_OUTPUT_BUF_DONE == event_type)
            {
                HABMM_MEMCPY(out_data, in_data, sizeof(hypvpp_callback_data_type));

                share_id = in_buf->pixel.fd;
                if (share_id)
                {
                    find_entry = hypvpp_map_from_lookup(&hypvpp_session->lookup_queue, share_id);
                    if (NULL == find_entry)
                    {
                        rc = HYPVPP_STATUS_FAIL;
                        HYP_VPP_MSG_ERROR("failed to find an entry node for share id:%u", share_id);
                    }
                    else
                    {
                        /* restore the actual fd from entry */
                        out_buf->pixel.fd = find_entry->fd;
                        hypvpp_map_free(&hypvpp_session->habmm_if, &hypvpp_session->lookup_queue, find_entry->fd, share_id);
                        HYP_VPP_MSG_INFO("erase an entry node for share id:%u fd:%d", share_id, out_buf->pixel.fd);
                    }
                }
            }
            else
            {
                rc = HYPVPP_STATUS_FAIL;
                HYP_VPP_MSG_ERROR("invalid callback event %u", in_data->event_type);
            }
        }
    }

    return rc;
}



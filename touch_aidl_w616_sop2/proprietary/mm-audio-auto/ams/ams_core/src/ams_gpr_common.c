/**
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#define LOG_TAG "ams_gpr_common"
#include <log/log.h>
#include <errno.h>

#include "gpr_api_inline.h"
#include "ams_gpr_common.h"
#include "apm_api.h"
#include "prm_api.h"
#include "dsp_audio_micro_service.h"

#define AMS_GPR_ALIGN_8BYTE(x) (((x) + 7) & (~7))

#define AMS_TIMEOUT_NS(x) ((x) * (1000LL) * (1000LL))

struct ams_gpr_common_ctx_s ams_gpr_common_ctx;

// internal api
static int32_t ams_gpr_common_signal_obj_create(struct ams_gpr_common_signal_obj **sig_obj);
static int32_t ams_gpr_common_signal_obj_release(struct ams_gpr_common_signal_obj *sig_obj);
static int32_t ams_gpr_common_signal_obj_timedwait(struct ams_gpr_common_signal_obj *sig_obj, uint32_t timeout_ms, uint32_t *rsp_flags, uint32_t *rsp_status, void **rsp_gpr_pkt);
static int32_t ams_gpr_common_signal_obj_set(struct ams_gpr_common_signal_obj *sig_obj, uint32_t flags, int32_t status, void *gpr_pkt);
// end of internal api

static uint32_t ams_gpr_common_callback(gpr_packet_t *packet, void *cb_data)
{
    struct ams_gpr_common_ctx_s *ctxt;
    int32_t rc = 0;
    ctxt = (struct ams_gpr_common_ctx_s *)cb_data;
    // ALOGD("enter ams_gpr_commmon_callback");
    switch (packet->opcode)
    {
    case PRM_CMD_RSP_REQUEST_HW_RSC:
    case PRM_CMD_RSP_RELEASE_HW_RSC:
    {
        struct prm_cmd_rsp_hw_rsc_cfg_t *cmd_rsp = GPR_PKT_GET_PAYLOAD(struct prm_cmd_rsp_hw_rsc_cfg_t, packet);
        if (!cmd_rsp->status)
        {
            // differentiate request vs release on packet->opcode if needed
            if (ams_gpr_common_ctx.rsp_buff && ams_gpr_common_ctx.rsp_buff_sz >= GPR_PKT_GET_PAYLOAD_BYTE_SIZE(packet->header))
            {
                memcpy(ams_gpr_common_ctx.rsp_buff, (uint8_t *)cmd_rsp,
                       GPR_PKT_GET_PAYLOAD_BYTE_SIZE(packet->header));
                ams_gpr_common_ctx.rsp_buff_sz = GPR_PKT_GET_PAYLOAD_BYTE_SIZE(packet->header);
            }
        }
        else
        {
            int32_t *pint = (int32_t *)cmd_rsp;
            ALOGE("Response status %d", cmd_rsp->status);
            for (int i = 0; i < GPR_PKT_GET_PAYLOAD_BYTE_SIZE(packet->header) / 4; i++)
            {
                ALOGE("cmd rsp =%08x\n", pint[i]);
            }
        }
        ALOGD("Received response 0x%x, status 0x%x, buf_sz %d, rsp_pld_sz %d\n", packet->opcode,
              cmd_rsp->status, ams_gpr_common_ctx.rsp_buff_sz, GPR_PKT_GET_PAYLOAD_BYTE_SIZE(packet->header));
        rc = ams_gpr_common_signal_obj_set(ctxt->psig_obj, AMS_GPR_COMMON_EVENT_MASK_SPF_RSP,
                                           cmd_rsp->status, NULL);
        rc = __gpr_cmd_free(packet);
        break;
    }
    case DSP_AMS_CMDRSP_GET_PARAM:
    {
        dsp_ams_cmdrsp_get_param_t *cmd_rsp = GPR_PKT_GET_PAYLOAD(dsp_ams_cmdrsp_get_param_t, packet);
        ALOGD("Received GET PARAM response 0x%x, status 0x%x", packet->opcode,
              cmd_rsp->status);
        rc = ams_gpr_common_signal_obj_set(ctxt->psig_obj, AMS_GPR_COMMON_EVENT_MASK_SPF_RSP,
                                           cmd_rsp->status, packet);
        int32_t *pint = (int32_t *)cmd_rsp;
        for (int i = 0; i < GPR_PKT_GET_PAYLOAD_BYTE_SIZE(packet->header) / 4; i++)
        {
            ALOGD("cmd rsp =%08x\n", pint[i]);
        }
        break;
    }
    case GPR_IBASIC_RSP_RESULT:
    {
        struct spf_cmd_basic_rsp *basic_rsp = GPR_PKT_GET_PAYLOAD(struct spf_cmd_basic_rsp, packet);
        ALOGD("Received basic response 0x%x, status 0x%x", packet->opcode,
              basic_rsp->status);
        rc = ams_gpr_common_signal_obj_set(ctxt->psig_obj, AMS_GPR_COMMON_EVENT_MASK_SPF_RSP,
                                           basic_rsp->status, packet);
        break;
    }
    case DSP_AMS_CMDRSP_MEM_MAP:
    {
        dsp_ams_cmdrsp_mem_map_t *cmd_rsp = GPR_PKT_GET_PAYLOAD(dsp_ams_cmdrsp_mem_map_t, packet);
        ALOGD("Received MEM_MAP response 0x%x, status 0x%x, buf_sz %d, rsp_pld_sz %d\n", packet->opcode,
              cmd_rsp->status, ams_gpr_common_ctx.rsp_buff_sz, GPR_PKT_GET_PAYLOAD_BYTE_SIZE(packet->header));
        rc = ams_gpr_common_signal_obj_set(ctxt->psig_obj, AMS_GPR_COMMON_EVENT_MASK_SPF_RSP,
                                           cmd_rsp->status, packet);
        break;
    }
    case DSP_AMS_CMDRSP_GET_VERSION:
    {
        dsp_ams_cmdrsp_get_version_t *cmd_rsp = GPR_PKT_GET_PAYLOAD(dsp_ams_cmdrsp_get_version_t, packet);
        ALOGD("Received GET_VERSION response 0x%x, major_version 0x%x, buf_sz %d, rsp_pld_sz %d\n", packet->opcode,
              cmd_rsp->major_version, ams_gpr_common_ctx.rsp_buff_sz, GPR_PKT_GET_PAYLOAD_BYTE_SIZE(packet->header));
        rc = ams_gpr_common_signal_obj_set(ctxt->psig_obj, AMS_GPR_COMMON_EVENT_MASK_SPF_RSP,
                                           0, packet);
        break;
    }
    case DSP_AMS_CMDRSP_OPEN_GRAPH:
    {
        dsp_ams_cmdrsp_open_graph_t *cmd_rsp = GPR_PKT_GET_PAYLOAD(dsp_ams_cmdrsp_open_graph_t, packet);
        ALOGD("Received OPEN GRAPH response 0x%x, status 0x%x, buf_sz %d, rsp_pld_sz %d\n", packet->opcode,
              cmd_rsp->status, ams_gpr_common_ctx.rsp_buff_sz, GPR_PKT_GET_PAYLOAD_BYTE_SIZE(packet->header));
        rc = ams_gpr_common_signal_obj_set(ctxt->psig_obj, AMS_GPR_COMMON_EVENT_MASK_SPF_RSP,
                                           cmd_rsp->status, packet);
        break;
    }
    default:
        ALOGE("Received unsupported opcode 0x%x", packet->opcode);
        break;
    }
    ALOGD("ams_gpr_common_signal_obj_set ret %d", rc);
    return rc;
}

int32_t ams_gpr_common_ctx_init(void)
{
    int32_t rc = 0;
    rc = ams_osal_mutex_create(&ams_gpr_common_ctx.mutex);
    rc = gpr_init();
    if (rc)
    {
        ALOGE("gpr init failed %d", rc);
        return rc;
    }
    rc = ams_gpr_common_signal_obj_create(&ams_gpr_common_ctx.psig_obj);
    if (rc)
    {
        ALOGE("failed to init sig object!");
        goto exit;
    }
    // TODO: select ports from interval
    rc = __gpr_cmd_register(AMS_GPR_SRC_PORT,
                            ams_gpr_common_callback, &ams_gpr_common_ctx);
    if(rc){
        ams_gpr_common_signal_obj_release(ams_gpr_common_ctx.psig_obj);
    }
    ALOGD("register gpr callback ret [%d]!", rc);
exit:
    if (rc){
        ams_osal_mutex_destroy(ams_gpr_common_ctx.mutex);
    }
    ams_gpr_common_ctx.rsp_buff = NULL;
    ams_gpr_common_ctx.rsp_buff_sz = 0;
    return rc;
}

int32_t ams_gpr_common_ctx_deinit(void)
{
    int32_t rc = 0;
    rc = ams_osal_mutex_destroy(ams_gpr_common_ctx.mutex);
    rc = __gpr_cmd_deregister(AMS_GPR_SRC_PORT);
    if (rc)
        ALOGE("Error [%d] when deregistering gpr!", rc);
    ams_gpr_common_signal_obj_release(ams_gpr_common_ctx.psig_obj);
    return rc;
}

static int32_t ams_gpr_common_signal_obj_create(struct ams_gpr_common_signal_obj **ppsig_obj)
{
    int32_t rc = 0;
    struct ams_gpr_common_signal_obj *psig_obj;
    psig_obj = calloc(1, sizeof(struct ams_gpr_common_signal_obj));

    if (psig_obj == NULL)
    {
        rc = ENOMEM;
        goto exit_on_error;
    }
    memset(psig_obj, 0, sizeof(struct ams_gpr_common_signal_obj));

    if (ar_osal_signal_create(&psig_obj->sig))
    {
        rc = ENOMEM;
        goto free_mem;
    }

    if (ams_osal_mutex_create(&psig_obj->lock))
    {
        rc = ENOMEM;
        goto destroy_sig;
    }
    psig_obj->gpr_packet = NULL;
    *ppsig_obj = psig_obj;
    return rc;
destroy_sig:
    ar_osal_signal_destroy(psig_obj->sig);
free_mem:
    free(psig_obj);
exit_on_error:
    *ppsig_obj = NULL;
    return rc;
}

static int32_t ams_gpr_common_signal_obj_release(struct ams_gpr_common_signal_obj *psig_obj)
{
    int32_t rc = 0;
    if (!psig_obj)
        return EINVAL;
    ams_osal_mutex_destroy(psig_obj->lock);
    ar_osal_signal_destroy(psig_obj->sig);
    free(psig_obj);
    return rc;
}

static int32_t ams_gpr_common_signal_obj_timedwait(struct ams_gpr_common_signal_obj *sig_obj, uint32_t timeout_ms, uint32_t *rsp_flags, uint32_t *rsp_status, void **rsp_gpr_pkt)
{
    int32_t rc = 0;
    int32_t timed_out;
    if (!sig_obj)
        return EINVAL;

    rc = ar_osal_signal_timedwait(sig_obj->sig, AMS_TIMEOUT_NS(timeout_ms));
    if (!rc)
    {
        if (sig_obj->lock)
            ams_osal_mutex_lock(sig_obj->lock);
        ar_osal_signal_clear(sig_obj->sig); // TODO:check if needed
        *rsp_flags = sig_obj->flags;
        if (rsp_status)
            *rsp_status = sig_obj->status;
        if (rsp_gpr_pkt)
            *rsp_gpr_pkt = (gpr_packet_t *)sig_obj->gpr_packet;
        sig_obj->gpr_packet = NULL;
        sig_obj->flags = 0; /* clear */
        sig_obj->status = 0;
        if (sig_obj->lock)
            ams_osal_mutex_unlock(sig_obj->lock);
    }
    return rc;
}

static int32_t ams_gpr_common_signal_obj_set(struct ams_gpr_common_signal_obj *sig_obj, uint32_t flags, int32_t status, void *gpr_pkt)
{
    int32_t rc = 0;
    int32_t timed_out;
    if (!sig_obj)
        return EINVAL;
    if (sig_obj->lock)
        ams_osal_mutex_lock(sig_obj->lock);
    sig_obj->flags |= flags;
    sig_obj->status = status;
    /* if there was a pending gpr packet that never got consumed free it */
    if (sig_obj->gpr_packet)
        __gpr_cmd_free(sig_obj->gpr_packet);
    sig_obj->gpr_packet = gpr_pkt;
    rc = ar_osal_signal_set(sig_obj->sig);
    if (sig_obj->lock)
        ams_osal_mutex_unlock(sig_obj->lock);
    return rc;
}

int32_t ams_gpr_common_allocate_gpr_packet(uint32_t opcode, uint32_t src_domain, uint32_t dst_domain, uint32_t src_port,
                                           uint32_t dst_port, uint32_t payload_size, uint32_t token,
                                           struct gpr_packet_t **alloc_packet)
{
    int32_t rc = 0;
    gpr_cmd_alloc_ext_t alloc_params;

    alloc_params.src_domain_id = src_domain;
    alloc_params.dst_domain_id = dst_domain;
    alloc_params.client_data = 0;

    alloc_params.src_port = src_port;
    alloc_params.dst_port = dst_port;
    alloc_params.token = token;
    alloc_params.opcode = opcode;
    alloc_params.payload_size = payload_size;
    alloc_params.ret_packet = alloc_packet;
    ALOGD("%s:pkt size %d", __func__, payload_size);
    rc = __gpr_cmd_alloc_ext(&alloc_params);

    return rc;
}

int32_t ams_gpr_common_send_spf_cmd_wait_for_rsp(struct gpr_packet_t *packet,
                                                 struct ams_gpr_common_signal_obj *sig_obj, void **rsp_pkt)
{
    int32_t rc = 0;
    uint32_t flags = 0, spf_status = 0;

    uint32_t src_port = packet->src_port, opcode = packet->opcode,
             dst_port = packet->dst_port;
    rc = __gpr_cmd_async_send(packet);
    if (rc)
    {
        ALOGE("Error [%d] when sending gpr pkt!", rc);
        __gpr_cmd_free(packet);
        return rc;
    }

    if (sig_obj != NULL)
    {
        ALOGD("Wait for Rsp opcode[0x%x] src_port[0x%x] dst_port[0x%x]", opcode, src_port, dst_port);
        /*
         * once osal signal is set it remains set till cleared, so if we
         * got a set before wait is called the below wait will immediately
         * return
         */
        rc = ams_gpr_common_signal_obj_timedwait(sig_obj, AMS_GPR_COMMON_SPF_TIMEOUT_MS, &flags,
                                                 &spf_status, rsp_pkt);
        ALOGD("Wait done rc[0x%x] opcd[0x%x] src_prt[0x%x] dst_prt[0x%x]",
              rc, opcode, src_port, dst_port);
        ALOGD("flags[0x%x] spf_status[0x%x]", flags, spf_status);
        if (rc)
            rc = ETIMEDOUT;
        else if (flags & AMS_GPR_COMMON_EVENT_MASK_SSR)
            rc = EAGAIN;
        else if ((flags & AMS_GPR_COMMON_EVENT_MASK_SPF_RSP))
        {
            rc = spf_status;
        }
    }

    return rc;
}

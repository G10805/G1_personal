/**
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#define LOG_TAG "ams_lib"
#include <log/log.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#ifdef AMS_SSR_SUPPORT_EN
#include "ams_osal_ssr.h"
#endif
#include "ams_platform.h"
#include "ams.h"
#include "ams_util.h"

#if (DSP_AMS_CMD_MEM_MAP_NUM_REGIONS != 1)
#error Num regions 1 is supported!
#endif
#define AMS_CSD_DRV_MEM_ALLOC
static ams_status_t ams_platform_init_contig_mem_list(struct platform_drv *pdrv);

static ams_status_t ams_platform_deinit_contig_mem_list(struct platform_drv *pdrv);

static ams_status_t ams_platform_build_tunnel_payload(ams_tunnel_param_t *params, uint32_t opcode, uint32_t pld_sz, uint32_t rsp_sz);

static ams_status_t ams_platform_free_tunnel_payload(ams_tunnel_param_t *params);

static ams_status_t ams_platform_free_tunnel_payload(ams_tunnel_param_t *params)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    if (!params)
    {
        ALOGE("Tunnel command pointer is NULL!");
        return AMS_STATUS_INPUT_ERROR;
    }
    if (params->req)
        free(params->req);
    if (params->rsp)
        free(params->rsp);
    return r;
}

static ams_status_t ams_platform_build_tunnel_payload(ams_tunnel_param_t *params, uint32_t opcode, uint32_t pld_sz, uint32_t rsp_pld_sz)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    if (!params)
    {
        ALOGE("Tunnel command pointer is NULL!");
        return AMS_STATUS_INPUT_ERROR;
    }
    uint32_t req_sz = pld_sz + sizeof(ams_core_tunnel_req_t);
    uint32_t rsp_sz = rsp_pld_sz + sizeof(ams_core_tunnel_rsp_t);
    params->req = calloc(1, req_sz);
    if (!params->req)
    {
        ALOGE("Cannot allocate memory");
        return AMS_STATUS_GENERAL_ERROR;
    }
    params->rsp = calloc(1, rsp_sz);
    if (!params->rsp)
    {
        ALOGE("Cannot allocate memory");
        free(params->req);
        params->req = NULL;
        return AMS_STATUS_GENERAL_ERROR;
    }
    params->req->opcode = opcode;
    params->req->processor_id = 0; // mdsp
    params->req->payload_size = pld_sz;
    params->req->response_size = rsp_sz;
    params->rsp->response_size = rsp_sz;

    return r;
}

static inline ams_status_t ams_platform_alloc_map_smem(struct platform_drv *pdrv, uint32_t size)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    r = ams_platform_drv_set_smem_size(pdrv, size);
    if (r != AMS_STATUS_SUCCESS)
        goto exit;
    r = ams_platform_drv_alloc_smem(pdrv);
    if (r != AMS_STATUS_SUCCESS)
        goto exit;
#ifndef MOCK_QNX_PMEM
    r = ams_platform_map_smem(pdrv);
    if (r != AMS_STATUS_SUCCESS)
        goto exit;
#endif
    r = ams_platform_init_contig_mem_list(pdrv);
exit:
    return r;
}

static inline ams_status_t ams_platform_unmap_free_smem(struct platform_drv *pdrv)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    // unmap
#ifndef MOCK_QNX_PMEM
    r = ams_platform_unmap_smem(pdrv);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Not possible to unmap smem (err=%d)!", r);
        goto exit;
    }
#endif
    // free mem
    r = ams_platform_drv_free_smem(pdrv);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Not possible to free smem (err=%d)!", r);
        goto exit;
    }
    r = ams_platform_deinit_contig_mem_list(pdrv);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Not possible to free smem (err=%d)!", r);
        goto exit;
    }
exit:

    return r;
}

ams_status_t ams_platform_open(
    struct platform_drv *pdrv)
{
    ams_status_t r = AMS_STATUS_SUCCESS;

    r = ams_platform_drv_open(pdrv);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("cannot open ams driver %d", r);
        goto exit;
    }
    // init lock
    if (ams_osal_mutex_create(&(pdrv->smem.rwlock)))
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Unable to init smem rwlock!");
        goto exit;
    }
    // init lock
    if (ams_osal_mutex_create(&(pdrv->ssr.rwlock)))
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Unable to init ssr rwlock!");
        goto exit;
    }
    r = ams_platform_alloc_map_smem(pdrv, AMS_SMEM_BUF_SIZE);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("failed to allocate/map smem (%d)", r);
    }
exit:
    return r;
}

ams_status_t ams_platform_close(
    struct platform_drv *pdrv)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    r = ams_platform_unmap_free_smem(pdrv);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("cannot free/unmap smem (%d)", r);
    }
    // destroy lock
    if (!pdrv->smem.rwlock || ams_osal_mutex_destroy(pdrv->smem.rwlock))
    {
        AMS_LIB_LOGE("Unable to destroy rwlock!");
    }
    if (!pdrv->ssr.rwlock || ams_osal_mutex_destroy(pdrv->ssr.rwlock))
    {
        AMS_LIB_LOGE("Unable to destroy rwlock!");
    }
    r = ams_platform_drv_close(pdrv);

    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("cannot close ams driver %d", r);
    }
    return r;
}

ams_status_t ams_platform_get_version(
    struct platform_drv *pdrv,
    uint32_t *version)
{
    ams_status_t r = AMS_STATUS_SUCCESS;

    uint32_t pld_len = sizeof(uint32_t);
    uint32_t respcode = 0;
    dsp_ams_cmdrsp_get_version_t *respbuf;
    uint32_t respbuf_len = sizeof(*respbuf);
    ams_tunnel_param_t params = {0};
    r = ams_platform_build_tunnel_payload(&params, DSP_AMS_CMD_GET_VERSION, pld_len, respbuf_len);
    respbuf = params.rsp->response;
    r = ams_platform_drv_ioctl(pdrv, AMS_CORE_CMD_TUNNEL, &params, sizeof(params));
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Cannot read fw version %d!", r);
        goto exit;
    }
    if (params.rsp->response_code == DSP_AMS_CMDRSP_GET_VERSION)
    {
        if (params.rsp->response_size == respbuf_len)
        {
            pdrv->version = respbuf->major_version;
            *version = respbuf->major_version;
            AMS_LIB_LOGD("Read fw version: major=%d, minor=%d", respbuf->major_version, respbuf->minor_version);
        }
        else
        {
            r = AMS_STATUS_GENERAL_ERROR;
            AMS_LIB_LOGE("Unexpected response size (%d) when reading fw version!", params.rsp->response_size);
            goto exit;
        }
    }
    else
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Response code %d when reading fw version!", params.rsp->response_code);
        goto exit;
    }
    r = ams_platform_drv_util_check_fw_version(pdrv);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Wrong fw version(%d)!", pdrv->version);
        goto exit;
    }
exit:
    ams_platform_free_tunnel_payload(&params);
    return r;
}

ams_status_t ams_platform_map_smem(
    struct platform_drv *pdrv)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
#ifndef AMS_CSD_DRV_MEM_ALLOC
    dsp_ams_cmd_mem_map_t *pld = NULL;
    dsp_ams_mem_region_t *apx = NULL;
    if (pdrv == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Null platform handle!");
        return r;
    }
    // don't check ssr_st as it is called by cb on ssr complete with ssr_st=SSR_EVENT_RESTART_STAR to prevent other API from running

    pld = calloc(1, sizeof(dsp_ams_cmd_mem_map_t) + DSP_AMS_CMD_MEM_MAP_NUM_REGIONS * sizeof(dsp_ams_mem_region_t));
    if (pld == NULL)
    {
        AMS_LIB_LOGE("Cannot allocate memory!");
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }
    pld->num_regions = DSP_AMS_CMD_MEM_MAP_NUM_REGIONS;
    apx = (dsp_ams_mem_region_t *)((uint8_t *)pld + sizeof(dsp_ams_cmd_mem_map_t));

    uint64_t pa;
    uint8_t *pmem = MAKE_64BIT_ADDR(pdrv->smem.address_lsw, pdrv->smem.address_msw);
    if (ams_platform_drv_prep_dsp_memaddr((void *)pmem, pdrv->smem.len, &pa) != AMS_STATUS_SUCCESS)
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Unable to translate to DSP address!");
        goto exit;
    }
    apx->addr_lsw = (uint32_t)((uint64_t)pa & (0xffffffff));
    apx->addr_msw = (uint32_t)((uint64_t)pa >> 32);
    apx->size_bytes = pdrv->smem.len;

    AMS_LIB_LOGD("apx->addr_lsw %x!", apx->addr_lsw);
    AMS_LIB_LOGD("apx->addr_msw %x!", apx->addr_msw);

    uint32_t pld_len = sizeof(dsp_ams_cmd_mem_map_t) + sizeof(dsp_ams_mem_region_t); // sizeof(pld);
    uint32_t respcode = 0;
    dsp_ams_cmdrsp_mem_map_t respbuf;
    uint32_t respbuf_len = sizeof(respbuf);
    ams_tunnel_param_t params;

    AMS_LIB_LOGD("apx->pld_len %d!", pld_len);
    r = ams_platform_build_tunnel_payload(&params, DSP_AMS_CMD_MEM_MAP, pld_len, respbuf_len);
    memcpy(params.req->payload, &pld, pld_len);

    r = ams_platform_drv_ioctl(pdrv, AMS_CORE_CMD_TUNNEL, &params, sizeof(params));
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Cannot mem map %d!", r);
        goto exit;
    }
    if (params.rsp->response_code == DSP_AMS_CMDRSP_MEM_MAP)
    {
        if (params.rsp->response_size == respbuf_len)
        {
            if (params.rsp->response->status == DSP_AMS_STATUS_OK)
            {
                pdrv->smem.mem_map_handle = params.rsp->response->mem_handle;
            }
            else
            {
                r = AMS_STATUS_GENERAL_ERROR;
                AMS_LIB_LOGE("Mem map failed with err=%d!", params.rsp->response->status);
                goto exit;
            }
        }
        else
        {
            r = AMS_STATUS_GENERAL_ERROR;
            AMS_LIB_LOGE("Unexpected response size (%d) when mem mapping!", params.rsp->response_size);
            goto exit;
        }
    }
    else
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Response code %d when mem mapping!", params.rsp->response_code);
        goto exit;
    }
exit:
    if (pld)
        free(pld);
    ams_platform_free_tunnel_payload(&params);
#else
    AMS_LIB_LOGD("Skip map smem by libams!");
#endif
    return r;
}
ams_status_t ams_platform_unmap_smem(
    struct platform_drv *pdrv)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
#ifndef AMS_CSD_DRV_MEM_ALLOC
    int32_t reslock_ssr_st = 0;
    if (pdrv == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Null platform handle!");
        goto exit;
    }
    LOCK_FOR_READ(reslock_ssr_st, pdrv->ssr.rwlock);
    if ((pdrv->ssr.ssr_adsp_st == SSR_EVENT_RESTART_START) ||
        (pdrv->ssr.ssr_mdsp_st == SSR_EVENT_RESTART_START))
    {
        AMS_LIB_LOGE("SSR is active!");
        r = AMS_STATUS_STATE_ERROR;
        goto exit;
    }
    dsp_ams_cmd_mem_unmap_t pld;

    pld.mem_handle = pdrv->smem.mem_map_handle;
    uint32_t pld_len = sizeof(pld);
    uint32_t respcode = 0;
    struct spf_cmd_basic_rsp respbuf;
    uint32_t respbuf_len = sizeof(respbuf);
    ams_tunnel_param_t params;
    r = ams_platform_build_tunnel_payload(&params, DSP_AMS_CMD_MEM_UNMAP, pld_len, respbuf_len);
    memcpy(params.req->payload, &pld, pld_len);
    r = ams_platform_drv_ioctl(pdrv, AMS_CORE_CMD_TUNNEL, &params, sizeof(params));

    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Cannot mem unmap %d!", r);
        goto exit;
    }
    if (params.rsp->response_code == GPR_IBASIC_RSP_RESULT)
    {
        if (params.rsp->response_size == respbuf_len)
        {
            if (params.rsp->response->status == DSP_AMS_STATUS_OK)
            {
                pdrv->smem.mem_map_handle = 0;
            }
            else
            {
                r = AMS_STATUS_GENERAL_ERROR;
                AMS_LIB_LOGE("Mem unmap failed with err=%d!", params.rsp->response->status);
                goto exit;
            }
        }
        else
        {
            r = AMS_STATUS_GENERAL_ERROR;
            AMS_LIB_LOGE("Unexpected response size (%d) when mem unmapping!", params.rsp->response_size);
            goto exit;
        }
    }
    else
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Response code %d when mem unmapping!", params.rsp->response_code);
        goto exit;
    }
exit:
    ams_platform_free_tunnel_payload(&params);
    UNLOCK_READ_WRITE(reslock_ssr_st, pdrv->ssr.rwlock);
#else
    AMS_LIB_LOGD("Skip unmap smem by libams!");
#endif
    return r;
}

ams_status_t ams_platform_open_graph(
    struct platform_drv *pdrv,
    struct ams_graph *gr)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    void *ppld = NULL;
    int32_t reslock_ssr_st = 0;
    uint32_t respcode = 0;
    dsp_ams_cmdrsp_open_graph_t *respbuf;
    uint32_t respbuf_len = sizeof(*respbuf);
    ams_tunnel_param_t params = {0};
    if (gr == NULL /*|| gr->fw_status!=DSP_AMS_GRAPH_INIT*/)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Null graph handle!");
        return r;
    }

    LOCK_FOR_READ(reslock_ssr_st, pdrv->ssr.rwlock);

    if ((pdrv->ssr.ssr_adsp_st == SSR_EVENT_RESTART_START) ||
        (pdrv->ssr.ssr_mdsp_st == SSR_EVENT_RESTART_START))
    {
        AMS_LIB_LOGE("SSR is active!");
        r = AMS_STATUS_STATE_ERROR;
        goto exit;
    }
    if (gr->fw_status != DSP_AMS_GRAPH_INIT)
    {
        r = AMS_STATUS_STATE_ERROR;
        AMS_LIB_LOGE("wrong fw status %d!", gr->fw_status);
        return r;
    }
    AMS_LIB_LOGD("Open graph fw status %d!", gr->fw_status);
    uint32_t pld_len = sizeof(dsp_ams_cmd_open_graph_t) + gr->modules.el_num * sizeof(ams_module_t) + gr->connections.el_num * sizeof(ams_connection_t) + (gr->sinks.el_num + gr->sources.el_num) * sizeof(ams_endpoint_t);

    r = ams_platform_build_tunnel_payload(&params, DSP_AMS_CMD_OPEN_GRAPH, pld_len, respbuf_len);
    r = ams_util_graph_serialize(gr, params.req->payload, pld_len);
    if (r)
    {
        AMS_LIB_LOGE("Graph serialization failed!");
        goto exit;
    }
    respbuf = params.rsp->response;
    r = ams_platform_drv_ioctl(pdrv, AMS_CORE_CMD_TUNNEL, &params, sizeof(params));
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Open graph failed!");
        goto exit;
    }

    if (params.rsp->response_code == DSP_AMS_CMDRSP_OPEN_GRAPH)
    {
        if (params.rsp->response_size == respbuf_len)
        {
            if (respbuf->status == DSP_AMS_STATUS_OK)
            {
                gr->fw_status = DSP_AMS_GRAPH_OPENED;
                gr->fw_handle = respbuf->handle;
            }
            else
            {
                r = AMS_STATUS_GENERAL_ERROR;
                AMS_LIB_LOGE("Open graph failed with err=%d!", respbuf->status);
                goto exit;
            }
            AMS_LIB_LOGD("Opened gr fw handle=%d, status=%d", respbuf->handle, gr->fw_status);
        }
        else
        {
            r = AMS_STATUS_GENERAL_ERROR;
            AMS_LIB_LOGE("Unexpected response size (%d) when opening graph!", params.rsp->response_size);
            goto exit;
        }
    }
    else
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Response code %d when opening graph!", params.rsp->response_code);
        goto exit;
    }

exit:
    UNLOCK_READ_WRITE(reslock_ssr_st, pdrv->ssr.rwlock);
    ams_platform_free_tunnel_payload(&params);
    return r;
}

ams_status_t ams_platform_set_endpoints_state(
    struct platform_drv *pdrv,
    struct ams_graph *gr,
    uint32_t enable)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_endpoint_el *e_el = gr->sinks.first;
    AMS_LIB_LOGD("enabling sink");
    while (e_el != NULL)
    {
        r = ams_platform_enable_endpoint(pdrv, &e_el->val, gr->base_param.sample_rate, enable);
        if (r != AMS_STATUS_SUCCESS)
        {
            AMS_LIB_LOGE("when enabling sink id=%d,err=%d!", e_el->val.id, r);
            goto exit;
        }
        e_el = e_el->nxt_el;
    }
    e_el = gr->sources.first;
    AMS_LIB_LOGD("enabling source");
    while (e_el != NULL)
    {
        r = ams_platform_enable_endpoint(pdrv, &e_el->val, gr->base_param.sample_rate, enable);
        if (r != AMS_STATUS_SUCCESS)
        {
            AMS_LIB_LOGE("when enabling source id=%d,err=%d!", e_el->val.id, r);
            goto exit;
        }
        e_el = e_el->nxt_el;
    }
exit:
    return r;
}

ams_status_t ams_platform_start_graph(
    struct platform_drv *pdrv,
    struct ams_graph *gr)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    int32_t reslock_ssr_st = 0;
    dsp_ams_cmd_start_graph_t *pld;
    uint32_t pld_len = sizeof(*pld);
    uint32_t respcode = 0;
    struct spf_cmd_basic_rsp *respbuf;
    uint32_t respbuf_len = sizeof(*respbuf);
    ams_tunnel_param_t params = {0};
    if (gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Null graph handle!");
        return r;
    }

    LOCK_FOR_READ(reslock_ssr_st, pdrv->ssr.rwlock);
    if ((pdrv->ssr.ssr_adsp_st == SSR_EVENT_RESTART_START) ||
        (pdrv->ssr.ssr_mdsp_st == SSR_EVENT_RESTART_START))
    {
        AMS_LIB_LOGE("SSR is active!");
        r = AMS_STATUS_STATE_ERROR;
        goto exit;
    }
    UNLOCK_READ_WRITE(reslock_ssr_st, pdrv->ssr.rwlock);
    if (gr->fw_status != DSP_AMS_GRAPH_OPENED)
    {
        r = AMS_STATUS_STATE_ERROR;
        AMS_LIB_LOGE("wrong fw status %d!", gr->fw_status);
        goto exit;
    }
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Cannot enable tdm ports, err=%d!", r);
        // goto exit;
    }

    // enable endpoints
    r = ams_platform_set_endpoints_state(pdrv, gr, 1);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Cannot enable endpoints, err=%d!", r);
        goto unlock;
    }

    r = ams_platform_build_tunnel_payload(&params, DSP_AMS_CMD_START_GRAPH, pld_len, respbuf_len);
    if (r)
    {
        AMS_LIB_LOGE("error %d! when building tunnel payload", r);
        goto exit;
    }
    pld = params.req->payload;
    pld->handle = gr->fw_handle;

    respbuf = params.rsp->response;

    r = ams_platform_drv_ioctl(pdrv, AMS_CORE_CMD_TUNNEL, &params, sizeof(params));
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Cannot start graph!");
        goto exit;
    }
    if (params.rsp->response_code == GPR_IBASIC_RSP_RESULT)
    {
        if (params.rsp->response_size == respbuf_len)
        {
            if (respbuf->status == DSP_AMS_STATUS_OK)
            {
                gr->fw_status = DSP_AMS_GRAPH_STARTED;
            }
            else
            {
                r = AMS_STATUS_GENERAL_ERROR;
                AMS_LIB_LOGE("Start graph failed with err=%d!", respbuf->status);
                goto exit;
            }
            AMS_LIB_LOGD("Graph status=%d", gr->fw_status);
        }
        else
        {
            r = AMS_STATUS_GENERAL_ERROR;
            AMS_LIB_LOGE("Unexpected response size (%d) when starting graph!", params.rsp->response_size);
            goto exit;
        }
    }
    else
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Response code %d when starting graph!", params.rsp->response_code);
        goto exit;
    }

exit:
    ams_platform_free_tunnel_payload(&params);
unlock:
    UNLOCK_READ_WRITE(reslock_ssr_st, pdrv->ssr.rwlock);
    return r;
}

ams_status_t ams_platform_stop_graph(
    struct platform_drv *pdrv,
    struct ams_graph *gr)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    int32_t reslock_ssr_st = 0;
    dsp_ams_cmd_stop_graph_t *pld;
    uint32_t pld_len = sizeof(*pld);
    uint32_t respcode = 0;
    struct spf_cmd_basic_rsp *respbuf;
    uint32_t respbuf_len = sizeof(*respbuf);
    ams_tunnel_param_t params = {0};
    if (gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Null graph handle!");
        goto exit;
    }

    LOCK_FOR_READ(reslock_ssr_st, pdrv->ssr.rwlock);

    if ((pdrv->ssr.ssr_adsp_st == SSR_EVENT_RESTART_START) ||
        (pdrv->ssr.ssr_mdsp_st == SSR_EVENT_RESTART_START))
    {
        AMS_LIB_LOGI("SSR is active!");
        //reset fw graph state
        ams_util_update_graph_fwinfo_on_ssr(gr);
        goto exit;
    }
    UNLOCK_READ_WRITE(reslock_ssr_st, pdrv->ssr.rwlock);
    if (gr->fw_status != DSP_AMS_GRAPH_STARTED)
    {
        r = AMS_STATUS_STATE_ERROR;
        AMS_LIB_LOGE("wrong fw status %d!", gr->fw_status);
        return r;
    }
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Cannot disable tdm ports, err=%d!", r);
        // goto exit;
    }
    r = ams_platform_build_tunnel_payload(&params, DSP_AMS_CMD_STOP_GRAPH, pld_len, respbuf_len);
    if (r)
    {
        AMS_LIB_LOGE("error %d! when building tunnel payload", r);
        goto exit;
    }
    pld = params.req->payload;
    pld->handle = gr->fw_handle;
    respbuf = params.rsp->response;
    r = ams_platform_drv_ioctl(pdrv, AMS_CORE_CMD_TUNNEL, &params, sizeof(params));

    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Cannot stop graph!");
        goto exit;
    }
    if (params.rsp->response_code == GPR_IBASIC_RSP_RESULT)
    {
        if (params.rsp->response_size == respbuf_len)
        {
            if (respbuf->status == DSP_AMS_STATUS_OK)
            {
                gr->fw_status = DSP_AMS_GRAPH_OPENED;
            }
            else
            {
                r = AMS_STATUS_GENERAL_ERROR;
                AMS_LIB_LOGE("Stop graph failed with err=%d!", respbuf->status);
                goto exit;
            }
            AMS_LIB_LOGD("Graph status=%d", gr->fw_status);
        }
        else
        {
            r = AMS_STATUS_GENERAL_ERROR;
            AMS_LIB_LOGE("Unexpected response size (%d) when stoping graph!", params.rsp->response_size);
            goto exit;
        }
    }
    else
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Response code %d when stoping graph!", params.rsp->response_code);
        goto exit;
    }
    // disable all endpoints
    r = ams_platform_set_endpoints_state(pdrv, gr, 0);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Cannot disable endpoints, err=%d!", r);
        goto exit;
    }
exit:
    ams_platform_free_tunnel_payload(&params);
    UNLOCK_READ_WRITE(reslock_ssr_st, pdrv->ssr.rwlock);
    return r;
}

ams_status_t ams_platform_close_graph(
    struct platform_drv *pdrv,
    struct ams_graph *gr)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    int32_t reslock_ssr_st = 0;
    dsp_ams_cmd_close_graph_t *pld;
    uint32_t pld_len = sizeof(*pld);
    uint32_t respcode = 0;
    struct spf_cmd_basic_rsp *respbuf;
    uint32_t respbuf_len = sizeof(*respbuf);
    ams_tunnel_param_t params = {0};
    if (gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Null graph handle!");
        goto exit;
    }

    LOCK_FOR_READ(reslock_ssr_st, pdrv->ssr.rwlock);

    if ((pdrv->ssr.ssr_adsp_st == SSR_EVENT_RESTART_START) ||
        (pdrv->ssr.ssr_mdsp_st == SSR_EVENT_RESTART_START))
    {
        AMS_LIB_LOGI("SSR is active!");
        //reset fw graph state
        gr->fw_status = DSP_AMS_GRAPH_INIT;
        goto exit;
    }
    if (gr->fw_status != DSP_AMS_GRAPH_OPENED)
    {
        r = AMS_STATUS_STATE_ERROR;
        AMS_LIB_LOGE("Wrong graph fw status %d!", gr->fw_status);
        goto exit;
    }

    r = ams_platform_build_tunnel_payload(&params, DSP_AMS_CMD_CLOSE_GRAPH, pld_len, respbuf_len);
    if (r)
    {
        AMS_LIB_LOGE("error %d! when building tunnel payload", r);
        goto exit;
    }
    pld = params.req->payload;
    pld->handle = gr->fw_handle;
    respbuf = params.rsp->response;
    r = ams_platform_drv_ioctl(pdrv, AMS_CORE_CMD_TUNNEL, &params, sizeof(params));
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Cannot close graph!");
        goto exit;
    }
    if (params.rsp->response_code == GPR_IBASIC_RSP_RESULT)
    {
        if (params.rsp->response_size == respbuf_len)
        {
            if (respbuf->status == DSP_AMS_STATUS_OK)
            {
                gr->fw_status = DSP_AMS_GRAPH_INIT;
            }
            else
            {
                r = AMS_STATUS_GENERAL_ERROR;
                AMS_LIB_LOGE("Close graph failed with err=%d!", respbuf->status);
                goto exit;
            }
            AMS_LIB_LOGI("Graph status=%d", respbuf->status);
        }
        else
        {
            r = AMS_STATUS_GENERAL_ERROR;
            AMS_LIB_LOGE("Unexpected response size (%d) when closing graph!", params.rsp->response_size);
            goto exit;
        }
    }
    else
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Response code %d when closing graph!", params.rsp->response_code);
        goto exit;
    }

exit:
    ams_platform_free_tunnel_payload(&params);

    UNLOCK_READ_WRITE(reslock_ssr_st, pdrv->ssr.rwlock);
    return r;
}

static ams_status_t ams_platform_init_contig_mem_list(struct platform_drv *pdrv)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    uint32_t reslock_mem_list = 0;
    uint32_t mem_size_avail = 0;
    uint8_t *mem_blk_addr = NULL;
    struct ams_mem_list *memlist = &pdrv->smem.mem_list;
    // init locks
    if (ams_osal_mutex_create(&memlist->rwlock))
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Unable to init memlist rwlock!");
        goto exit;
    }
    LOCK_FOR_WRITE(reslock_mem_list, memlist->rwlock);
    if (pdrv->smem.mem_list.el_num != 0)
    {
        AMS_LIB_LOGE("Memory list not empty!Deinit first")
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }
    if (pdrv->smem.len == 0)
    {
        AMS_LIB_LOGE("Allocate and map smem first!")
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }
    // allocate  contigious mem blks
    mem_size_avail = pdrv->smem.len;
    mem_blk_addr = (uint8_t *)MAKE_64BIT_ADDR(pdrv->smem.address_lsw, pdrv->smem.address_msw);
    while (mem_size_avail)
    {
        struct ams_mem_block *mem_blk = calloc(1, sizeof(struct ams_mem_block));
        if (mem_blk == NULL)
        {
            r = AMS_STATUS_GENERAL_ERROR;
            AMS_LIB_LOGE("Unable to mem blk!");
            goto exit;
        }
        mem_blk->block_size = mem_size_avail < PMEM_PAGESIZE ? mem_size_avail : PMEM_PAGESIZE;
        mem_blk->mem_block_addr = mem_blk_addr;
        mem_blk_addr += mem_blk->block_size;
        mem_blk->inuse = 0;
        mem_blk->nxt_el = NULL;
        ADD_NEW_LIST_EL(memlist, mem_blk);
        mem_size_avail -= mem_blk->block_size;
    }
exit:
    UNLOCK_READ_WRITE(reslock_mem_list, pdrv->smem.mem_list.rwlock)
    return r;
}

static ams_status_t ams_platform_deinit_contig_mem_list(struct platform_drv *pdrv)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    uint32_t reslock_mem_list = 0;
    struct ams_mem_block *mem_blk = NULL;
    struct ams_mem_list *memlist = &pdrv->smem.mem_list;
    if (pdrv->smem.mem_list.el_num == 0)
    {
        AMS_LIB_LOGE("Memory list empty!")
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }
    LOCK_FOR_WRITE(reslock_mem_list, memlist->rwlock);
    mem_blk = memlist->first;
    REMOVE_LIST_EL_ALL(mem_blk, memlist)
    UNLOCK_READ_WRITE(reslock_mem_list, memlist->rwlock)
exit:
    // deinit locks
    if (!memlist->rwlock || ams_osal_mutex_destroy(memlist->rwlock))
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Unable to destroy memlist rwlock!");
    }
    return r;
}

static ams_status_t ams_platform_get_contig_mem(
    struct platform_drv *pdrv,
    uint32_t *mem_size_req,
    void **pmem_start)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    uint32_t reslock_mem_list = 0;
    uint32_t msize = 0;
    uint32_t calc_new_memsize = 0;
    uint32_t found = 0;
    void *mem_start_addr = NULL;
    struct ams_mem_block *mem_blk_start = NULL;
    struct ams_mem_block *mem_blk_end = NULL;
    LOCK_FOR_WRITE(reslock_mem_list, pdrv->smem.mem_list.rwlock);
    struct ams_mem_list *meml = &pdrv->smem.mem_list;
    struct ams_mem_block *mem_blk = meml->first;
    while (mem_blk != NULL)
    {
        if (mem_blk->inuse)
        {
            // reset state var
            msize = 0;
            calc_new_memsize = 0;
        }
        else if (!mem_blk->inuse && calc_new_memsize == 0)
        {
            // beginining of free mem region
            calc_new_memsize = 1;
            mem_start_addr = mem_blk->mem_block_addr;
            mem_blk_start = mem_blk;
        }
        if (calc_new_memsize)
        {
            msize += mem_blk->block_size;
            if (msize >= *mem_size_req)
            {
                // enough memory found; mark it as used and terminate
                found = 1;
                mem_blk_end = mem_blk;
                mem_blk = mem_blk_start;
                while (mem_blk != mem_blk_end)
                {
                    mem_blk->inuse = 1;
                    mem_blk = mem_blk->nxt_el;
                }
                AMS_LIB_LOGD("Use memblck addr %lx", (uint64_t)mem_blk->mem_block_addr);
                mem_blk->inuse = 1; // mark mem_blk_end
                break;
            }
        }
        mem_blk = mem_blk->nxt_el;
    }
    if (found)
    {
        *pmem_start = mem_start_addr;
        *mem_size_req = msize;
    }
    else
    {
        *pmem_start = NULL;
    }
exit:
    UNLOCK_READ_WRITE(reslock_mem_list, pdrv->smem.mem_list.rwlock)
    return r;
}
static ams_status_t ams_platform_free_contig_mem(
    struct platform_drv *pdrv,
    uint32_t mem_size,
    void *pmem_start)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    uint32_t reslock_mem_list = 0;
    uint32_t msize = 0;
    uint32_t calc_new_memsize = 0;
    LOCK_FOR_WRITE(reslock_mem_list, pdrv->smem.mem_list.rwlock);
    struct ams_mem_list *meml = &pdrv->smem.mem_list;
    struct ams_mem_block *mem_blk = meml->first;
    while (mem_blk != NULL)
    {
        if (mem_blk->mem_block_addr == pmem_start)
        {
            calc_new_memsize = 1;
        }
        if (calc_new_memsize == 1)
        {
            mem_blk->inuse = 0;
            msize += mem_blk->block_size;
            if (msize >= mem_size)
            {
                // stop
                break;
            }
        }
        mem_blk = mem_blk->nxt_el;
    }
exit:
    UNLOCK_READ_WRITE(reslock_mem_list, pdrv->smem.mem_list.rwlock)
    return r;
}
static ams_status_t ams_platform_try_adjust_smem_size(
    struct platform_drv *pdrv,
    uint32_t param_size)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    uint32_t old_smem_size;
    uint32_t new_smem_size;
    if (pdrv == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Null handle!");
        goto exit;
    }
    // new_smem_size = pdrv->smem.len + param_size;
    new_smem_size = param_size;
    old_smem_size = pdrv->smem.len;
    r = ams_platform_unmap_free_smem(pdrv);
    AMS_LIB_LOGD("Unmap free ret %d!", r);
    if (r != AMS_STATUS_SUCCESS)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Cannot free/unmap smem!");
        goto exit;
    }
    // try to allocate new memory
    r = ams_platform_alloc_map_smem(pdrv, new_smem_size);
    AMS_LIB_LOGD("Alloc %d size ret %d!", new_smem_size, r);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Cannot allocatte new smem of %d size!", new_smem_size);
        AMS_LIB_LOGE("Restore smem of %d size!", old_smem_size);
        r = ams_platform_alloc_map_smem(pdrv, old_smem_size);
    }
    if (r != AMS_STATUS_SUCCESS)
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Cannot allocate and map smem!");
        goto exit;
    }

exit:

    return r;
}

ams_status_t ams_platform_set_param(
    struct platform_drv *pdrv,
    struct ams_graph *gr,
    uint32_t module_id,  // [in]
    uint32_t param_id,   // [in]
    uint32_t param_size, // [in]
    void *data)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    uint32_t pld_len = 0;
    dsp_ams_cmd_set_param_t *ppld = NULL;
    uint8_t *pdata = NULL;
    dsp_ams_cmd_set_param_t pld = {0};
    uint32_t mem_size = param_size;
    int32_t reslock_smem = 0;
    int32_t reslock_ssr_st = 0;
    void *psmem = NULL;
    uint32_t respcode = 0;
    struct spf_cmd_basic_rsp *respbuf;
    uint32_t respbuf_len = sizeof(*respbuf);
    ams_tunnel_param_t params = {0};
    if (gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Null graph handle!");
        goto exit;
    }

    LOCK_FOR_READ(reslock_ssr_st, pdrv->ssr.rwlock);

    if ((pdrv->ssr.ssr_adsp_st == SSR_EVENT_RESTART_START) ||
        (pdrv->ssr.ssr_mdsp_st == SSR_EVENT_RESTART_START))
    {
        AMS_LIB_LOGE("SSR is active!");
        r = AMS_STATUS_STATE_ERROR;
        goto exit;
    }
    if (gr->fw_status != DSP_AMS_GRAPH_OPENED && gr->fw_status != DSP_AMS_GRAPH_STARTED)
    {
        r = AMS_STATUS_STATE_ERROR;
        AMS_LIB_LOGE("Wrong fw status %d!", gr->fw_status);
        goto exit;
    }
    AMS_LIB_LOGD("Param size to send=%d", param_size);
    if (param_size > AMS_LIB_APR_PKT_SIZE)
    {
#ifdef MOCK_QNX_PMEM
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("No support for parameter > %d KB when PMEM is mocked!", AMS_LIB_APR_PKT_SIZE);
        goto exit;
#endif
        LOCK_FOR_READ(reslock_smem, pdrv->smem.rwlock)
        r = ams_platform_get_contig_mem(pdrv, &mem_size, &psmem);
        if (psmem == NULL)
        {
            AMS_LIB_LOGE("Contig smem (len=%d) is NULL for param len%d!", mem_size, param_size);
        }
        if (r != AMS_STATUS_SUCCESS || psmem == NULL)
        {
            UNLOCK_READ_WRITE(reslock_smem, pdrv->smem.rwlock)
            // no free contigious memory is available; try to allocate new one
            LOCK_FOR_WRITE(reslock_smem, pdrv->smem.rwlock)
            r = ams_platform_try_adjust_smem_size(pdrv, param_size);
            if (r != AMS_STATUS_SUCCESS)
            {
                AMS_LIB_LOGE("Cannot adjust smem size to %d!", param_size);
                goto exit;
            }
            r = ams_platform_get_contig_mem(pdrv, &mem_size, &psmem);
            if (r != AMS_STATUS_SUCCESS || psmem == NULL)
            {
                AMS_LIB_LOGE("Cannot get contig mem of %d!", param_size);
                goto exit;
            }
        }
#ifndef AMS_CSD_DRV_MEM_ALLOC
        uint64_t pa;
        if (ams_platform_drv_prep_dsp_memaddr(psmem, mem_size, &pa) != AMS_STATUS_SUCCESS)
        {
            r = AMS_STATUS_GENERAL_ERROR;
            AMS_LIB_LOGE("Unable to translate to DSP address!");
            goto exit;
        }
        pld.address_lsw = (uint32_t)((uint64_t)pa & (0xffffffff));
        pld.address_msw = (uint32_t)((uint64_t)pa >> 32);
#else
        pld.address_lsw = pdrv->smem.ion_mem_fd;
#endif
        pld.handle = gr->fw_handle;
        pld.module_id = module_id;
        pld.param_id = param_id;
        pld.param_size = param_size;

        pld.mem_map_handle = pdrv->smem.mem_map_handle;
#ifndef AMS_CSD_DRV_MEM_ALLOC
        AMS_LIB_LOGD("Outband address_lsw =%08x, address_msw=%08x,  handle=%08x", pld.address_lsw, pld.address_msw, pld.mem_map_handle);
#else
        AMS_LIB_LOGD("Outband address_lsw =%08x, handle=%08x", pld.address_lsw, pld.mem_map_handle);
#endif
        // copy data to shared mem
        AMS_LIB_LOGD("Outband copy start addr =%lx, data %d, num=%d bytes", (uint64_t)psmem, *((uint32_t *)data), param_size);
        if (!memcpy(psmem, data, param_size))
        {
            AMS_LIB_LOGE("Failed outband copy start addr =%lx, num=%d bytes", (uint64_t)psmem, param_size);
        }
        AMS_LIB_LOGD("Outband copy done, read back %d", *((uint32_t *)psmem));
        ppld = &pld;
        pld_len = sizeof(pld);
    }
    else
    {
        // inband
        pld_len = sizeof(dsp_ams_cmd_set_param_t) + param_size;
        ppld = calloc(1, pld_len);
        if (ppld == NULL)
        {
            AMS_LIB_LOGE("Cannot allocate memory for payload!");
            r = AMS_STATUS_GENERAL_ERROR;
            goto exit;
        }
        ppld->handle = gr->fw_handle;
        ppld->module_id = module_id;
        ppld->param_id = param_id;
        ppld->param_size = param_size;
        ppld->mem_map_handle = 0;
        ppld->address_lsw = 0;
        ppld->address_msw = 0;
        // copy inband
        pdata = (uint8_t *)ppld + sizeof(dsp_ams_cmd_set_param_t);
        AMS_LIB_LOGD("Inband copy start addr =%lx, num=%d bytes", (uint64_t)pdata, param_size);
        memcpy(pdata, data, param_size);
    }

    r = ams_platform_build_tunnel_payload(&params, DSP_AMS_CMD_SET_PARAM, pld_len, respbuf_len);
    if (r)
    {
        AMS_LIB_LOGE("error %d! when building tunnel payload", r);
        goto exit;
    }
    memcpy(params.req->payload, ppld, pld_len); // FIXME:don't use extra buffer ppld!
    respbuf = params.rsp->response;
    r = ams_platform_drv_ioctl(pdrv, AMS_CORE_CMD_TUNNEL, &params, sizeof(ams_tunnel_param_t));
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Cannot set param %d!", param_id);
        goto exit;
    }
    if (params.rsp->response_code == GPR_IBASIC_RSP_RESULT)
    {
        if (params.rsp->response_size == respbuf_len)
        {
            if (respbuf->status != DSP_AMS_STATUS_OK)
            {
                r = AMS_STATUS_GENERAL_ERROR;
                AMS_LIB_LOGE("Set param failed with err=%d!", respbuf->status);
                goto exit;
            }
            AMS_LIB_LOGD("Set param status=%d", respbuf->status);
        }
        else
        {
            r = AMS_STATUS_GENERAL_ERROR;
            AMS_LIB_LOGE("Unexpected response size (%d) when setting param %d!", params.rsp->response_size, param_id);
            goto exit;
        }
    }
    else
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Response code %d when setting param %d!", params.rsp->response_code, param_id);
        goto exit;
    }

exit:
    if (pdata)
        free(ppld);
    if (psmem != NULL)
        r |= ams_platform_free_contig_mem(pdrv, mem_size, psmem);
    ams_platform_free_tunnel_payload(&params);
    UNLOCK_READ_WRITE(reslock_smem, pdrv->smem.rwlock);
    UNLOCK_READ_WRITE(reslock_ssr_st, pdrv->ssr.rwlock);
    return r;
}

ams_status_t ams_platform_get_param(
    struct platform_drv *pdrv,
    struct ams_graph *gr,
    uint32_t module_id,   // [in]
    uint32_t param_id,    // [in]
    uint32_t *param_size, // [in]
    void *data)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    int32_t reslock_smem = 0;
    uint32_t outband = 0;
    ams_tunnel_param_t params = {0};
    void *psmem = NULL;
    uint32_t mem_size = *param_size;
    int32_t reslock_ssr_st = 0;
    dsp_ams_cmd_get_param_t pld = {0};
    dsp_ams_cmdrsp_get_param_t respbuf = {0};
    uint8_t *prespbuf;
    uint32_t pld_len = sizeof(pld);
    uint32_t respcode = 0;
    uint32_t respbuf_len;
    if (gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Null graph handle!");
        goto exit;
    }

    LOCK_FOR_READ(reslock_ssr_st, pdrv->ssr.rwlock);

    if ((pdrv->ssr.ssr_adsp_st == SSR_EVENT_RESTART_START) ||
        (pdrv->ssr.ssr_mdsp_st == SSR_EVENT_RESTART_START))
    {
        AMS_LIB_LOGE("SSR is active!");
        r = AMS_STATUS_STATE_ERROR;
        goto exit;
    }
    if (gr->fw_status != DSP_AMS_GRAPH_OPENED && gr->fw_status != DSP_AMS_GRAPH_STARTED)
    {
        r = AMS_STATUS_STATE_ERROR;
        AMS_LIB_LOGE("Wrong fw status %d!", gr->fw_status);
        goto exit;
    }

    pld.handle = gr->fw_handle;
    pld.module_id = module_id;
    pld.param_id = param_id;
    pld.param_max_size = *param_size;

    memset(&respbuf, 0, sizeof(respbuf));
    if (*param_size > AMS_LIB_APR_PKT_SIZE)
    {
#ifdef MOCK_QNX_PMEM
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("No support for parameter > %d KB when PMEM is mocked!", AMS_LIB_APR_PKT_SIZE);
        goto exit;
#endif
        respbuf_len = sizeof(respbuf);
        prespbuf = (uint8_t *)&respbuf;
        LOCK_FOR_READ(reslock_smem, pdrv->smem.rwlock)
        r = ams_platform_get_contig_mem(pdrv, &mem_size, &psmem);
        if (r != AMS_STATUS_SUCCESS || psmem == NULL)
        {
            UNLOCK_READ_WRITE(reslock_smem, pdrv->smem.rwlock)
            // no free contigious memory is available; try to allocate new one
            LOCK_FOR_WRITE(reslock_smem, pdrv->smem.rwlock)
            r = ams_platform_try_adjust_smem_size(pdrv, *param_size);
            if (r != AMS_STATUS_SUCCESS)
            {
                AMS_LIB_LOGE("Cannot adjust smem size to %d!", *param_size);
                goto exit;
            }
            r = ams_platform_get_contig_mem(pdrv, &mem_size, &psmem);
            if (r != AMS_STATUS_SUCCESS || psmem == NULL)
            {
                AMS_LIB_LOGE("Cannot get contig mem of %d!", *param_size);
                goto exit;
            }
        }
        outband = 1;
#ifndef AMS_CSD_DRV_MEM_ALLOC
        uint64_t pa;
        if (ams_platform_drv_prep_dsp_memaddr(psmem, mem_size, &pa) != AMS_STATUS_SUCCESS)
        {
            r = AMS_STATUS_GENERAL_ERROR;
            AMS_LIB_LOGE("Unable to translate to DSP address!");
            goto exit;
        }
        pld.address_lsw = (uint32_t)((uint64_t)pa & (0xffffffff));
        pld.address_msw = (uint32_t)((uint64_t)pa >> 32);
#else
        pld.address_lsw = pdrv->smem.ion_mem_fd;
#endif
        pld.mem_map_handle = pdrv->smem.mem_map_handle;
#ifndef AMS_CSD_DRV_MEM_ALLOC
        AMS_LIB_LOGD("Outband address_lsw =%08x, address_msw=%08x,  handle=%08x", pld.address_lsw, pld.address_msw, pld.mem_map_handle);
#else
        AMS_LIB_LOGD("Outband address_lsw =%08x, handle=%08x", pld.address_lsw, pld.mem_map_handle);
#endif
    }
    else
    {
        // in band case
        respbuf_len = sizeof(dsp_ams_cmdrsp_get_param_t) + sizeof(dsp_ams_param_data_t) + *param_size;
        // TODO: check alignment!
        pld.address_lsw = 0;
        pld.address_msw = 0;
        pld.mem_map_handle = 0;
    }
    r = ams_platform_build_tunnel_payload(&params, DSP_AMS_CMD_GET_PARAM, pld_len, respbuf_len);
    if (r)
    {
        AMS_LIB_LOGE("error %d! when building tunnel payload", r);
        goto exit;
    }
    memcpy(params.req->payload, &pld, pld_len);
    prespbuf = params.rsp->response;
    r = ams_platform_drv_ioctl(pdrv, AMS_CORE_CMD_TUNNEL, &params, sizeof(ams_tunnel_param_t));
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Cannot get param %d!", param_id);
        goto exit;
    }
    if (params.rsp->response_code == DSP_AMS_CMDRSP_GET_PARAM)
    {
        if (((dsp_ams_cmdrsp_get_param_t *)prespbuf)->status != DSP_AMS_STATUS_OK)
        {
            AMS_LIB_LOGE("Get param status=%d", ((dsp_ams_cmdrsp_get_param_t *)prespbuf)->status);
            r = AMS_STATUS_GENERAL_ERROR;
        }
        else
        {
            if (outband)
            {
                // copy from shared mem to user provided
                // read first struct dsp_ams_param_data
                dsp_ams_param_data_t param_data = {0, 0, 0};
                memcpy(&param_data, psmem, sizeof(param_data));
                AMS_LIB_LOGD("read from smem modid=%d, param_id=%d, param_size=%d, err=%d", param_data.module_id, param_data.param_id, param_data.param_size, r);
                uint8_t *pdata = psmem + sizeof(param_data);
                memcpy(data, pdata, param_data.param_size);
                AMS_LIB_LOGD("read data from smem param_size=%d, err=%d", param_data.param_size, r);
                *param_size = param_data.param_size;
            }
            else
            {
                // copy from inband to user memory
                prespbuf += sizeof(dsp_ams_cmdrsp_get_param_t);
                dsp_ams_param_data_t *inband_data = (dsp_ams_param_data_t *)prespbuf;
                *param_size = inband_data->param_size;
                inband_data = (dsp_ams_param_data_t *)((uint8_t *)inband_data + sizeof(dsp_ams_param_data_t));
                AMS_LIB_LOGD("Inband copy start addr =%lx, num=%d bytes", (uint64_t)data, *param_size);
                memcpy(data, inband_data, *param_size);
            }
        }
    }
    else
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Response code %d when getting param %d!", params.rsp->response_code, param_id);
        goto exit;
    }

exit:
    if (psmem != NULL)
        r |= ams_platform_free_contig_mem(pdrv, mem_size, psmem);
    r |= ams_platform_free_tunnel_payload(&params);
    UNLOCK_READ_WRITE(reslock_smem, pdrv->smem.rwlock)
    UNLOCK_READ_WRITE(reslock_ssr_st, pdrv->ssr.rwlock);
    return r;
}

ams_status_t ams_platform_enable_endpoint(
    struct platform_drv *pdrv,
    ams_endpoint_t *e,
    uint32_t sr,
    uint32_t enable)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    ams_endpoint_cfg_t params = {0};
    int32_t reslock_ssr_st = 0;
    if (e == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    // form payload
    params.flags = e->flags;
    params.source = (e->type == AMS_ENDPOINT_TYPE_SOURCE) ? true : false;
    params.tdm_params.hw_interface_id = e->tdm_params.hw_interface_id;
    params.tdm_params.num_channels = e->tdm_params.num_channels;
    params.tdm_params.sample_rate = sr;
    params.tdm_params.bit_width = e->tdm_params.bit_width;
    params.tdm_params.data_format = e->tdm_params.data_format;
    params.tdm_params.sync_mode = e->tdm_params.sync_mode;
    params.tdm_params.sync_src = e->tdm_params.sync_src;
    params.tdm_params.nslots_per_frame = e->tdm_params.nslots_per_frame;
    params.tdm_params.ctrl_data_out_enable = e->tdm_params.ctrl_data_out_enable;
    params.tdm_params.ctrl_invert_sync_pulse = e->tdm_params.ctrl_invert_sync_pulse;
    params.tdm_params.ctrl_sync_data_delay = e->tdm_params.ctrl_sync_data_delay;
    params.tdm_params.slot_width = e->tdm_params.slot_width;
    params.tdm_params.slot_mask = e->tdm_params.slot_mask;
    r = ams_platform_drv_ioctl(pdrv, (enable ? AMS_CORE_CMD_ENABLE_ENDPOINT : AMS_CORE_CMD_DISABLE_ENDPOINT), &params, sizeof(ams_endpoint_cfg_t));
exit:

    return r;
}

static ams_status_t ams_platform_apply_prop_exclsv_ep(struct platform_drv *pdrv, struct ams_graph *gr, ams_graph_property_t *p)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    ams_graph_clk_attr_cfg_t param;
    if (!p)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    param.ep_id = p->u.exclv_ep_clk_attr.exclv_ep_id;
    param.attr = p->u.exclv_ep_clk_attr.clk_invert;
    r = ams_platform_drv_ioctl(pdrv, AMS_CORE_CMD_SET_CLK_ATTR, &param, sizeof(param));
exit:
    return r;
}

static ams_status_t ams_platform_apply_prop_ts_msg_link_build(struct platform_drv *pdrv, struct ams_graph *gr, ams_graph_property_t *p)
{
    ams_status_t r = AMS_STATUS_SUCCESS;

    void *ppld = NULL;
    int32_t reslock_ssr_st = 0;
    uint32_t respcode = 0;
    struct spf_cmd_basic_rsp *respbuf;
    uint32_t respbuf_len = sizeof(*respbuf);
    ams_tunnel_param_t params = {0};

    LOCK_FOR_READ(reslock_ssr_st, pdrv->ssr.rwlock);
    if (!p)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }

    if ((pdrv->ssr.ssr_adsp_st == SSR_EVENT_RESTART_START) ||
        (pdrv->ssr.ssr_mdsp_st == SSR_EVENT_RESTART_START))
    {
        AMS_LIB_LOGE("SSR is active!");
        r = AMS_STATUS_STATE_ERROR;
        goto exit;
    }
    if (gr->fw_status != DSP_AMS_GRAPH_OPENED)
    {
        r = AMS_STATUS_STATE_ERROR;
        AMS_LIB_LOGE("wrong fw status %d!", gr->fw_status);
        goto exit;
    }

    uint32_t num_link = p->u.ams_build_msg_link.num_link;

    uint32_t pld_len = sizeof(dsp_ams_cmd_set_prop_t) + sizeof(dsp_ams_build_msg_link_t) + num_link * sizeof(dsp_ams_msg_link_connection_t);
    ppld = calloc(1, pld_len);
    if (ppld == NULL)
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Cannot allocate mem %d", pld_len);
        goto exit;
    }

    dsp_ams_cmd_set_prop_t *set_prop = (dsp_ams_cmd_set_prop_t *)ppld;
    dsp_ams_build_msg_link_t *build_link = (dsp_ams_build_msg_link_t *)(ppld + sizeof(dsp_ams_cmd_set_prop_t));

    set_prop->prop_id = DSP_AMS_PROP_ID_BUILD_TIMESTAMP_MSG_LINK;
    set_prop->prop_size = sizeof(dsp_ams_build_msg_link_t) + num_link * sizeof(dsp_ams_msg_link_connection_t);

    build_link->handle = gr->fw_handle;
    build_link->num_link = num_link;

    uint8_t *pp_temp_src;
    uint8_t *pp_temp_dst;

    pp_temp_src = (uint8_t *)p->u.ams_build_msg_link.msg_link_ptr;

    pp_temp_dst = (uint8_t *)build_link;
    pp_temp_dst = pp_temp_dst + 2 * sizeof(uint32_t);
    uint32_t dest_sz = pld_len - sizeof(dsp_ams_cmd_set_prop_t) - sizeof(dsp_ams_build_msg_link_t);
    if (dest_sz >= num_link * sizeof(dsp_ams_msg_link_connection_t))
    {
        ams_util_memcpy(pp_temp_dst, dest_sz, pp_temp_src, num_link * sizeof(dsp_ams_msg_link_connection_t));
    }
    else
    {
        AMS_LIB_LOGE("Dest size %d is smaller than src sz %ld", dest_sz, (num_link * sizeof(dsp_ams_msg_link_connection_t)));
        goto exit;
    }
    r = ams_platform_build_tunnel_payload(&params, DSP_AMS_CMD_SET_PROP, pld_len, respbuf_len);
    if (r)
    {
        AMS_LIB_LOGE("error %d! when building tunnel payload", r);
        goto exit;
    }
    memcpy(params.req->payload, ppld, pld_len);
    respbuf = params.rsp->response;
    r = ams_platform_drv_ioctl(pdrv, AMS_CORE_CMD_TUNNEL, &params, sizeof(params));
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Cannot set graph property!");
        goto exit;
    }
    if (params.rsp->response_code == GPR_IBASIC_RSP_RESULT)
    {
        if (params.rsp->response_size == respbuf_len)
        {
            if (respbuf->status == DSP_AMS_STATUS_OK)
            {
                r = AMS_STATUS_SUCCESS;
            }
            else
            {
                r = AMS_STATUS_GENERAL_ERROR;
                AMS_LIB_LOGE("set graph property failed with err=%d!", respbuf->status);
                goto exit;
            }
            AMS_LIB_LOGI("Graph status=%d", respbuf->status);
        }
        else
        {
            r = AMS_STATUS_GENERAL_ERROR;
            AMS_LIB_LOGE("Unexpected response size (%d) when set graph property!", params.rsp->response_size);
            goto exit;
        }
    }
    else
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Response code %d when set graph property!", params.rsp->response_code);
        goto exit;
    }

exit:

    UNLOCK_READ_WRITE(reslock_ssr_st, pdrv->ssr.rwlock);
    if (ppld != NULL)
        free(ppld);
    ams_platform_free_tunnel_payload(&params);
    return r;
}

static ams_status_t ams_platform_apply_prop_ts_msg_link_destroy(struct platform_drv *pdrv, struct ams_graph *gr, ams_graph_property_t *p)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    void *ppld = NULL;
    int32_t reslock_ssr_st = 0;
    uint32_t respcode = 0;
    struct spf_cmd_basic_rsp *respbuf;
    uint32_t respbuf_len = sizeof(*respbuf);
    ams_tunnel_param_t params = {0};

    LOCK_FOR_READ(reslock_ssr_st, pdrv->ssr.rwlock);
    if (!p)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }

    if ((pdrv->ssr.ssr_adsp_st == SSR_EVENT_RESTART_START) ||
        (pdrv->ssr.ssr_mdsp_st == SSR_EVENT_RESTART_START))
    {
        AMS_LIB_LOGE("SSR is active!");
        r = AMS_STATUS_STATE_ERROR;
        goto exit;
    }
    if (gr->fw_status != DSP_AMS_GRAPH_OPENED)
    {
        r = AMS_STATUS_STATE_ERROR;
        AMS_LIB_LOGE("wrong fw status %d!", gr->fw_status);
        goto exit;
    }

    uint32_t pld_len = sizeof(dsp_ams_cmd_set_prop_t) + sizeof(dsp_ams_destroy_msg_link_t);
    ppld = calloc(1, pld_len);
    if (ppld == NULL)
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Cannot allocate mem %d", pld_len);
        goto exit;
    }

    dsp_ams_cmd_set_prop_t *set_prop = (dsp_ams_cmd_set_prop_t *)ppld;
    dsp_ams_destroy_msg_link_t *destroy_link = (dsp_ams_destroy_msg_link_t *)(ppld + sizeof(dsp_ams_cmd_set_prop_t));

    set_prop->prop_id = DSP_AMS_PROP_ID_DESTROY_TIMESTAMP_MSG_LINK;
    set_prop->prop_size = sizeof(dsp_ams_destroy_msg_link_t);
    destroy_link->handle = gr->fw_handle;

    r = ams_platform_build_tunnel_payload(&params, DSP_AMS_CMD_SET_PROP, pld_len, respbuf_len);
    if (r)
    {
        AMS_LIB_LOGE("error %d! when building tunnel payload", r);
        goto exit;
    }
    memcpy(params.req->payload, ppld, pld_len);
    respbuf = params.rsp->response;
    r = ams_platform_drv_ioctl(pdrv, AMS_CORE_CMD_TUNNEL, &params, sizeof(params));
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Cannot set graph property!");
        goto exit;
    }
    if (params.rsp->response_code == GPR_IBASIC_RSP_RESULT)
    {
        if (params.rsp->response_size == respbuf_len)
        {
            if (respbuf->status == DSP_AMS_STATUS_OK)
            {
                r = AMS_STATUS_SUCCESS;
            }
            else
            {
                r = AMS_STATUS_GENERAL_ERROR;
                AMS_LIB_LOGE("set graph property failed with err=%d!", respbuf->status);
                goto exit;
            }
            AMS_LIB_LOGI("Graph status=%d", respbuf->status);
        }
        else
        {
            r = AMS_STATUS_GENERAL_ERROR;
            AMS_LIB_LOGE("Unexpected response size (%d) when set graph property!", params.rsp->response_size);
            goto exit;
        }
    }
    else
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Response code %d when set graph property!", params.rsp->response_code);
        goto exit;
    }

exit:
    UNLOCK_READ_WRITE(reslock_ssr_st, pdrv->ssr.rwlock);
    if (ppld != NULL)
        free(ppld);
    ams_platform_free_tunnel_payload(&params);
    return r;
}

static ams_status_t ams_platform_apply_prop_tdm_multilane_cfg(struct platform_drv *pdrv, struct ams_graph *gr, ams_graph_property_t *p)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    void *ppld = NULL;
    uint32_t respcode = 0;
    struct spf_cmd_basic_rsp *respbuf;
    uint32_t respbuf_len = sizeof(*respbuf);
    ams_tunnel_param_t params = {0};

    if (!p)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }

    if (gr->fw_status != DSP_AMS_GRAPH_OPENED)
    {
        r = AMS_STATUS_STATE_ERROR;
        AMS_LIB_LOGE("wrong fw status %d!", gr->fw_status);
        goto exit;
    }

    uint32_t pld_len = sizeof(dsp_ams_cmd_set_prop_t) + sizeof(dsp_ams_tdm_lane_cfg_t);
    ppld = calloc(1, pld_len);
    if (ppld == NULL)
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Cannot allocate mem %d", pld_len);
        goto exit;
    }

    dsp_ams_cmd_set_prop_t *set_prop = (dsp_ams_cmd_set_prop_t *)ppld;
    dsp_ams_tdm_lane_cfg_t *tdm_cfg = (dsp_ams_tdm_lane_cfg_t *)(ppld + sizeof(dsp_ams_cmd_set_prop_t));

    set_prop->prop_id = DSP_AMS_PROP_ID_TDM_LANE_CFG;
    set_prop->prop_size = sizeof(dsp_ams_tdm_lane_cfg_t);

    tdm_cfg->handle = gr->fw_handle;
    tdm_cfg->hw_interface_id = p->u.exclv_ep_tdm_lane_cfg.hw_interface_id;
    tdm_cfg->type = p->u.exclv_ep_tdm_lane_cfg.type;
    tdm_cfg->lane_mask = p->u.exclv_ep_tdm_lane_cfg.lane_mask;

    AMS_LIB_LOGD("handle : 0x%x, hw_interface_id : %u, type : %u and lane_mask : %u ", tdm_cfg->handle,
                 tdm_cfg->hw_interface_id, tdm_cfg->type, tdm_cfg->lane_mask);
    r = ams_platform_build_tunnel_payload(&params, DSP_AMS_CMD_SET_PROP, pld_len, respbuf_len);
    if (r)
    {
        AMS_LIB_LOGE("error %d! when building tunnel payload", r);
        goto exit;
    }
    memcpy(params.req->payload, ppld, pld_len);
    respbuf = params.rsp->response;
    r = ams_platform_drv_ioctl(pdrv, AMS_CORE_CMD_TUNNEL, &params, sizeof(params));
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Cannot set graph property!");
        goto exit;
    }
    if (params.rsp->response_code == GPR_IBASIC_RSP_RESULT)
    {
        if (params.rsp->response_size == respbuf_len)
        {
            if (respbuf->status == DSP_AMS_STATUS_OK)
            {
                r = AMS_STATUS_SUCCESS;
            }
            else
            {
                r = AMS_STATUS_GENERAL_ERROR;
                AMS_LIB_LOGE("set graph property failed with err=%d!", respbuf->status);
                goto exit;
            }
            AMS_LIB_LOGI("Graph status=%d", respbuf->status);
        }
        else
        {
            r = AMS_STATUS_GENERAL_ERROR;
            AMS_LIB_LOGE("Unexpected response size (%d) when set graph property!", params.rsp->response_size);
            goto exit;
        }
    }
    else
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Response code %d when set graph property!", params.rsp->response_code);
        goto exit;
    }

exit:
    if (ppld != NULL)
        free(ppld);
    r |= ams_platform_free_tunnel_payload(&params);
    return r;
}

static ams_status_t (*ams_platfrom_apply_prop_funcs[AMS_GRAPH_PROPERTY_ID_LAST])(struct platform_drv *pdrv, struct ams_graph *gr, ams_graph_property_t *p) = {
    ams_platform_apply_prop_exclsv_ep,
    ams_platform_apply_prop_ts_msg_link_build,
    ams_platform_apply_prop_ts_msg_link_destroy,
    ams_platform_apply_prop_tdm_multilane_cfg,
};

ams_status_t ams_platform_apply_prop(struct platform_drv *pdrv, struct ams_graph *gr, uint32_t after_start)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_graph_property_el *prop_el;
    ams_graph_property_t *prop;
    if (gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Null graph handle!");
        goto exit;
    }
    prop_el = gr->props.first;
    while (prop_el != NULL)
    {
        prop = &prop_el->val;
        if ((prop->prop_id <= AMS_GRAPH_PROPERTY_ID_LAST) && ams_platfrom_apply_prop_funcs[prop->prop_id - AMS_GRAPH_PROPERTY_ID_FIRST] && after_start == prop->appy_after_start)
        {
            r |= ams_platfrom_apply_prop_funcs[prop->prop_id - AMS_GRAPH_PROPERTY_ID_FIRST](pdrv, gr, prop);
        }
        prop_el = prop_el->nxt_el;
    }

exit:
    return r;
}

ams_status_t ams_platform_get_num_graphs_cached(struct platform_drv *pdrv, uint32_t *num_graphs)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    ams_cached_graph_num_t param = {0};
    // readback ioctl
    r = ams_platform_drv_ioctl(pdrv, AMS_CORE_CMD_GET_CACHED_GRAPH_NUM, &param, sizeof(param));
    if (r == AMS_STATUS_SUCCESS)
    {
        *num_graphs = param.num;
    }
    else
    {
        AMS_LIB_LOGE("Error.get num graph cached ret %d!", r);
    }
    return r;
}

ams_status_t ams_platform_restore_graph_cached_info_by_idx(struct ams_session *pses, uint32_t idx, struct ams_graph **ppgr)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    ams_cached_graph_info_t param = {0};
    param.idx = idx;
    ams_cached_graph_descr_t *descr = NULL;
    param.gr_descr_sz = 0;
    uint32_t descr_sz = 0;
    // readback ioctl:get buf size needed
    r = ams_platform_drv_ioctl(&pses->drv, AMS_CORE_CMD_GET_CACHED_GRAPH_INFO, &param, sizeof(param));
    if (r == AMS_STATUS_SUCCESS && param.gr_descr_sz)
    {
        descr_sz = param.gr_descr_sz + sizeof(ams_core_cached_graph_descr_t);
        descr = calloc(1, descr_sz);
        if (descr == NULL)
        {
            AMS_LIB_LOGE("Cannot allocate buffer of size %d!", param.gr_descr_sz);
            goto exit;
        }
        descr->gr_descr_sz = param.gr_descr_sz;
        descr->idx = idx;
        r = ams_platform_drv_ioctl(&pses->drv, AMS_CORE_CMD_GET_CACHED_GRAPH_DESCR, descr, descr_sz);
        AMS_LIB_LOGD("Get graph descr size %d ret [%d] ", descr->gr_descr_sz, r);
        AMS_LIB_LOGD("Graph hdl[%d] state[%d])", param.fw_handle, param.state);
        if (r == AMS_STATUS_SUCCESS)
            r = ams_util_session_restore_graph_info(pses, descr, ppgr); // deserialize pld
        if (ppgr)
        {
            (*ppgr)->fw_handle = param.fw_handle;
            (*ppgr)->fw_status = param.state;
            if (ams_osal_mutex_create(&(*ppgr)->rwlock))
            {
                r = AMS_STATUS_GENERAL_ERROR;
                AMS_LIB_LOGE("Unable to init graph rwlock!");
                ams_util_graph_list_remove_graph(&pses->gr_lst, *ppgr);
                goto exit;
            }
        }
        // TODO: restore property(clk attr)
        AMS_LIB_LOGD("Graph [%d] restored(err=[%d])!", idx, r);
    }
exit:
    if (descr)
        free(descr);
    return r;
}

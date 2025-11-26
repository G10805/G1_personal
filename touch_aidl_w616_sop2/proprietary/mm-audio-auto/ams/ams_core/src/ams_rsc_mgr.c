/**
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#define LOG_TAG "ams_rsc_mgr"
#include <log/log.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "ams_core_ioctl.h"

#include "dsp_audio_micro_service.h"
#include "ams_gpr_common.h"
#include "audio_hw_dma_api.h"
#include "audio_hw_lpm_api.h"
#include "gpr_msg_if.h"
#include "gpr_ids_domains.h"
#include "gpr_api_inline.h"
#include "ams_core.h"
#include "ams_prm.h"
#include "ams_rsc_mgr.h"
#include "ams_osal_shmem.h"
#include "ams_osal_error.h"

#define AMS_NUM_READ_DMA_CHANNELS 8
#define AMS_NUM_WRITE_DMA_CHANNELS 8
#define AMS_NUM_DMA_TYPES 6
#define AMS_LPM_LENGTH (4 * 1024)

#define AMS_DMA_INVALID_REQUEST (0xFFFF)
#define AMS_LPM_INVALID_REQUEST (0xFFFF)
#define DSP_AMS_CMD_MEM_MAP_NUM_REGIONS 1

struct ams_rsc_mgr_dma_conf
{
    uint32_t dma_type;
    uint16_t num_rddma;
    uint16_t num_wrdma;
    uint32_t rddma_idx[AMS_NUM_READ_DMA_CHANNELS];
    uint32_t wrdma_idx[AMS_NUM_WRITE_DMA_CHANNELS];
};

struct ams_rsc_mgr_dma_conf_dsp
{
    uint32_t dma_type;
    uint16_t num_rddma;
    uint16_t num_wrdma;
    // followed by
    // uint32_t rddma_idx[num_rddma];
    // uint32_t wrdma_idx[num_wrdma];
};

struct ams_rsc_mgr_shres
{
    uint32_t lpm_start_addr_lsw;
    /**< Lower 32 bits of LPM start phyical address available for AMS usage. */
    uint32_t lpm_start_addr_msw;
    /**< Higher 32 bits of LPM start phyical address available for AMS
    usage. */
    uint32_t lpm_length;
    /**< LPM length in bytes available for AMS usage. */
    uint32_t dma_types_num;
    struct ams_rsc_mgr_dma_conf dma_conf[AMS_NUM_DMA_TYPES];
};

static struct ams_rsc_mgr_shres ams_rsc_mgr_shres = {
    .lpm_start_addr_lsw = AMS_LPM_INVALID_REQUEST,
    .lpm_start_addr_msw = AMS_LPM_INVALID_REQUEST,
    .lpm_length = AMS_LPM_INVALID_REQUEST,
    .dma_types_num = AMS_DMA_INVALID_REQUEST};

static int32_t ams_rsc_mgr_get_lpass_resources_dma(void);
static int32_t ams_rsc_mgr_get_lpass_resources_lpm(void);

static int32_t ams_rsc_mgr_release_lpass_resources_dma(void);
static int32_t ams_rsc_mgr_release_lpass_resources_lpm(void);

extern struct ams_core_data ams_data;

struct ams_rsc_mgr_shres_init
{
    uint16_t dma_types_num;
    uint16_t dma_types[AMS_NUM_DMA_TYPES];
    uint16_t num_rddma[AMS_NUM_DMA_TYPES];
    uint16_t num_wrdma[AMS_NUM_DMA_TYPES];
};
// TODO:init from cfg file
static struct ams_rsc_mgr_shres_init rm_shres_init_data = {1, {0}, {1}, {1}};

static int32_t ams_rsc_mgr_build_tunnel_pld(ams_core_tunnel_t *params, uint32_t opcode, uint32_t pld_sz, uint32_t rsp_pld_sz)
{
    int32_t r = AMS_EOK;
    if (!params)
    {
        ALOGE("Tunnel command pointer is NULL!");
        return AMS_EBADPARAM;
    }
    uint32_t req_sz = pld_sz + sizeof(ams_core_tunnel_req_t);
    uint32_t rsp_sz = rsp_pld_sz + sizeof(ams_core_tunnel_rsp_t);
    params->req = calloc(1, req_sz);
    if (!params->req)
    {
        ALOGE("Cannot allocate memory");
        return ENOMEM;
    }
    params->rsp = calloc(1, rsp_sz);
    if (!params->rsp)
    {
        ALOGE("Cannot allocate memory");
        free(params->req);
        params->req = NULL;
        return ENOMEM;
    }
    params->req->opcode = opcode;
    params->req->processor_id = 0; // mdsp
    params->req->payload_size = pld_sz;
    params->req->response_size = rsp_sz;
    params->rsp->response_size = rsp_sz;

    return r;
}

static int32_t ams_rsc_mgr_free_tunnel_pld(ams_core_tunnel_t *params)
{
    int32_t r = AMS_EOK;
    if (!params)
    {
        ALOGE("Tunnel command pointer is NULL!");
        return AMS_EBADPARAM;
    }
    if (params->req)
        free(params->req);
    if (params->rsp)
        free(params->rsp);
    return r;
}

static int32_t ams_rsc_mgr_map_smem(struct ams_core_shmem_info *shmem)
{
    int32_t r = AMS_EOK;
    uint32_t len = shmem->shmem_info.buf_size;
    uint32_t *mem_map_handle = &(shmem->spf_mem_handle);
    dsp_ams_cmd_mem_map_t *pld = NULL;
    dsp_ams_mem_region_t *apx = NULL;

    uint32_t pld_len = sizeof(dsp_ams_cmd_mem_map_t) + DSP_AMS_CMD_MEM_MAP_NUM_REGIONS * sizeof(dsp_ams_mem_region_t); // sizeof(pld);
    uint32_t respcode = 0;
    dsp_ams_cmdrsp_mem_map_t *respbuf;
    uint32_t respbuf_len = sizeof(*respbuf);
    ams_core_tunnel_t params = {0};
    r = ams_rsc_mgr_build_tunnel_pld(&params, DSP_AMS_CMD_MEM_MAP, pld_len, respbuf_len);
    if (r != 0)
    {
        ALOGE("ams_rsc_mgr_mem_map: "
              "Cannot allocate memory!");
        r = ENOMEM;
        goto exit;
    }
    pld = params.req->payload;
    respbuf = params.rsp->response;
    pld->num_regions = DSP_AMS_CMD_MEM_MAP_NUM_REGIONS;
    apx = (dsp_ams_mem_region_t *)((uint8_t *)pld + sizeof(dsp_ams_cmd_mem_map_t));
    apx->addr_lsw = ((ams_shmem_handle_data_t *)shmem->shmem_info.metadata)->heap_fd;
    apx->size_bytes = len;

    r = ams_rsc_mgr_send_tunnel_info(&params, DSP_AMS_CMD_MEM_MAP);
    if (r != 0)
    {
        ALOGE("ams_rsc_mgr_map_smem: "
              "Cannot mem map %d!",
              r);
        goto exit;
    }
    if (params.rsp->response_code == DSP_AMS_CMDRSP_MEM_MAP)
    {
        if (params.rsp->response_size == respbuf_len)
        {
            if (respbuf->status == DSP_AMS_STATUS_OK)
            {
                *mem_map_handle = respbuf->mem_handle;
            }
            else
            {
                r = AMS_EBADPARAM;
                ALOGE("ams_rsc_mgr_mem_map: "
                      "resp status error %d",
                      respbuf->status);
                goto exit;
            }
        }
        else
        {
            r = AMS_EBADPARAM;
            ALOGE("ams_rsc_mgr_mem_map: "
                  "resp size wrong %d",
                  params.rsp->response_size);
            goto exit;
        }
    }
    else
    {
        r = AMS_EBADPARAM;
        ALOGE("ams_rsc_mgr_mem_map: "
              "resp code wrong %d",
              params.rsp->response_code);
        goto exit;
    }
exit:
    ams_rsc_mgr_free_tunnel_pld(&params);
    ALOGD("ams_rsc_mgr_mem_map: "
          "ret %d!",
          r);
    return r;
}

static int32_t ams_rsc_mgr_unmap_smem(uint32_t mem_handle)
{
    int32_t r = AMS_EOK;

    dsp_ams_cmd_mem_unmap_t *pld;

    // pld.mem_handle = mem_handle;
    uint32_t pld_len = sizeof(dsp_ams_cmd_mem_unmap_t);
    uint32_t respcode = 0;
    struct spf_cmd_basic_rsp *respbuf;
    uint32_t respbuf_len = sizeof(*respbuf);
    ams_core_tunnel_t params = {0};
    r = ams_rsc_mgr_build_tunnel_pld(&params, DSP_AMS_CMD_MEM_UNMAP, pld_len, respbuf_len);
    if (r != 0)
    {
        ALOGE("ams_rsc_mgr_mem_unmap: "
              "Cannot allocate memory!");
        r = ENOMEM;
        goto exit;
    }
    pld = params.req->payload;
    pld->mem_handle = mem_handle;
    respbuf = params.rsp->response;

    r = ams_rsc_mgr_send_tunnel_info(&params, DSP_AMS_CMD_MEM_UNMAP);

    if (r != AMS_EOK)
    {
        ALOGE("ams_rsc_mgr_unmap_smem: "
              "Cannot unmap smem %d!",
              r);
        goto exit;
    }
    if (params.rsp->response_code == GPR_IBASIC_RSP_RESULT)
    {
        if (params.rsp->response_size == respbuf_len)
        {
            if (respbuf->status == DSP_AMS_STATUS_OK)
            {
                // pdrv->smem.mem_map_handle = 0;
            }
            else
            {
                r = AMS_EBADPARAM;
                ALOGE("ams_rsc_mgr_unmap_smem: "
                      "resp status error %d",
                      respbuf->status);
                goto exit;
            }
        }
        else
        {
            r = AMS_EBADPARAM;
            ALOGE("ams_rsc_mgr_unmap_smem: "
                  "resp size wrong %d",
                  params.rsp->response_size);
            goto exit;
        }
    }
    else
    {
        r = AMS_EBADPARAM;
        ALOGE("ams_rsc_mgr_unmap_smem: "
              "resp code wrong %d",
              params.rsp->response_code);
        goto exit;
    }
exit:
    ams_rsc_mgr_free_tunnel_pld(&params);
    ALOGD("ams_rsc_mgr_unmap_smem: "
          "ret %d!",
          r);
    return r;
}

static int32_t ams_core_add_graph_info(void *payload, uint32_t payload_size, void *response)
{
    int32_t rc = AMS_EOK;
    void *graph_pld = NULL;
    dsp_ams_cmdrsp_open_graph_t *respbuf = NULL;
    struct ams_core_graph_info *new_gr = &ams_data.gr_infoq[ams_data.gr_infoq_pos];

    // cleanup for unclosed graph
    // number of customer graph is not expected to be > 1
    if (new_gr->graph_pld && new_gr->graph_pld_sz > 0)
    {
        free(new_gr->graph_pld);
        new_gr->graph_pld = 0;
        new_gr->graph_pld_sz = 0;
    }
    if (NULL == (graph_pld = calloc(1, payload_size)))
    {
        ALOGE("Unable to allocate memory for graph pld[%d]", payload_size);
        return AMS_ENORESOURCE;
    }
    memcpy((uint8_t *)graph_pld, (uint8_t *)payload, payload_size);
    respbuf = (dsp_ams_cmdrsp_open_graph_t*)response;

    new_gr->handle = respbuf->handle;
    new_gr->state = DSP_AMS_GRAPH_OPENED;
    new_gr->graph_pld = graph_pld;
    new_gr->graph_pld_sz = payload_size;
    if (++ams_data.gr_infoq_el_num > AMS_CORE_USER_GRAPH_MAX)
        ams_data.gr_infoq_el_num = AMS_CORE_USER_GRAPH_MAX;
    if (++ams_data.gr_infoq_pos >= AMS_CORE_USER_GRAPH_MAX)
        ams_data.gr_infoq_pos = 0;//wrap around
    ALOGD("Add graph handle (%08x), size (%d)", new_gr->handle, new_gr->graph_pld_sz);
    return rc;
}

static struct ams_core_graph_info * ams_core_find_graph(uint32_t handle)
{
    int32_t i = 0;
    struct ams_core_graph_info *pgr = NULL;
    for (i = 0; i < AMS_CORE_USER_GRAPH_MAX; i++)
    {
        pgr = &ams_data.gr_infoq[i];
        if (pgr->handle == handle)
            break;
    }
    return pgr;
}

static int32_t ams_core_remove_graph_info(uint32_t handle)
{
    int32_t rc = AMS_EOK;
    struct ams_core_graph_info *pgr = ams_core_find_graph(handle);
    ALOGD("Remove graph handle (%08x), size (%d)", pgr->handle, pgr->graph_pld_sz);
    if (pgr)
    {
        if (pgr->graph_pld)
            free(pgr->graph_pld);
        memset(pgr, 0, sizeof(*pgr));
    }
    if (ams_data.gr_infoq_el_num > 0)
        ams_data.gr_infoq_el_num--;
    return rc;
}

static int32_t ams_core_rsc_mgr_update_graph_info(ams_core_tunnel_t *info_p)
{
    struct ams_core_graph_info *pgr = NULL;
    int32_t graph_idx = 0;
    int32_t rc = AMS_EOK;

    switch(info_p->req->opcode)
    {
        case DSP_AMS_CMD_OPEN_GRAPH:
        {
            dsp_ams_cmdrsp_open_graph_t *respbuf = (dsp_ams_cmdrsp_open_graph_t *)info_p->rsp->response;
            if(respbuf->status == DSP_AMS_STATUS_OK)
            {
                rc = ams_core_add_graph_info((void *)info_p->req->payload, info_p->req->payload_size, (void *)info_p->rsp->response);
            }
            break;
        }
        case DSP_AMS_CMD_START_GRAPH:
        {
            struct spf_cmd_basic_rsp *respbuf = (struct spf_cmd_basic_rsp *)(info_p->rsp->response);
            if(respbuf->status == DSP_AMS_STATUS_OK)
            {
                dsp_ams_cmd_start_graph_t *payload = (dsp_ams_cmd_start_graph_t *)info_p->req->payload;
                if((NULL != (pgr = ams_core_find_graph(payload->handle))))
                {
                    pgr->state = DSP_AMS_GRAPH_STARTED;
                    rc = AMS_EOK;
                }
            }
            break;
        }
        case DSP_AMS_CMD_STOP_GRAPH:
        {
            struct spf_cmd_basic_rsp *respbuf = (struct spf_cmd_basic_rsp *)(info_p->rsp->response);
            if(respbuf->status == DSP_AMS_STATUS_OK)
            {
                dsp_ams_cmd_stop_graph_t *payload = (dsp_ams_cmd_stop_graph_t *)(info_p->req->payload);
                if(NULL != (pgr = ams_core_find_graph(payload->handle)))
                {
                    pgr->state = DSP_AMS_GRAPH_OPENED;
                    rc = AMS_EOK;
                }
            }
            break;
        }
        case DSP_AMS_CMD_CLOSE_GRAPH:
        {
            struct spf_cmd_basic_rsp *respbuf = (struct spf_cmd_basic_rsp *)(info_p->rsp->response);
            if(respbuf->status == DSP_AMS_STATUS_OK)
            {
                dsp_ams_cmd_close_graph_t *payload = (dsp_ams_cmd_close_graph_t *)info_p->req->payload;
                rc = ams_core_remove_graph_info(payload->handle);
            }
            break;
        }

        default:
            rc = AMS_EOK;
            break;
    }
    return rc;
}

int32_t ams_rsc_mgr_shmem_init(void)
{
    int32_t rc =AMS_EOK;
    int32_t i;
    rc = ams_shmem_init();
    if (rc)
    {
        ALOGE("Failed to init shmem, [%d]", rc);
        rc = ENOMEM;
        goto exit;
    }
    for (i = 0; i < AMS_CORE_CLIENTS_NUM; i++)
    {
        ams_data.smem_cli_info[i].shmem_info.num_sys_id = (uint8_t)AMS_CORE_CLIENTS_MEM_IDS;
        ams_data.smem_cli_info[i].shmem_info.sys_id = ams_data.smem_cli_info[i].mem_sys_ids;
        // init all to modem,APPS
        ams_data.smem_cli_info[i].shmem_info.sys_id[0] = (uint8_t)AMS_SS_ID_MODEM_DSP;
        ams_data.smem_cli_info[i].shmem_info.sys_id[1] = (uint8_t)AMS_SS_ID_APSS;
        ams_data.smem_cli_info[i].shmem_info.cache_type = AMS_SHMEM_UNCACHED;
        ams_data.smem_cli_info[i].shmem_info.mem_type = AMS_SHMEM_PHYSICAL_MEMORY;
        ams_data.smem_cli_info[i].shmem_info.flags = AMS_SHMEM_BIT_MASK_HW_ACCELERATOR_FLAG << AMS_SHMEM_SHIFT_HW_ACCELERATOR_FLAG; // 1
    }
exit:
    return rc;
}

int32_t ams_rsc_mgr_mdsp_ssr_cleanup(void)
{
    int32_t rc = AMS_EOK;
    int32_t i;
    for (i = 0; i < AMS_CORE_CLIENTS_NUM; i++)
    {
        ams_data.smem_cli_info[i].spf_mem_handle = 0;
    }
    for (i = 0; i < AMS_CORE_USER_GRAPH_MAX; i++)
    {
        if (ams_data.gr_infoq[i].graph_pld)
            free(ams_data.gr_infoq[i].graph_pld);
    }
    memset(ams_data.gr_infoq, 0, sizeof(ams_data.gr_infoq));
    return rc;
}

int32_t ams_rsc_mgr_shmem_deinit(void)
{
    int32_t rc = AMS_EOK;
    int32_t i;
    rc = ams_shmem_deinit();
    if (rc)
    {
        ALOGE("Failed to deinit shmem, [%d]", rc);
    }
    for (i = 0; i < AMS_CORE_CLIENTS_NUM; i++)
    {
        ams_data.smem_cli_info[i].shmem_info.num_sys_id = (uint8_t)0;
        ams_data.smem_cli_info[i].shmem_info.sys_id = NULL;
        ams_data.smem_cli_info[i].shmem_info.cache_type = 0;
        ams_data.smem_cli_info[i].shmem_info.mem_type = 0;
        ams_data.smem_cli_info[i].shmem_info.flags = 0;
    }
    return rc;
}

int32_t ams_rsc_mgr_alloc_shmem(ams_core_shmem_alloc_t *smem_alloc)
{
    int32_t rc = AMS_EOK;
    if (smem_alloc == NULL){
        ALOGE("%s:NULL shmem alloc!", __func__);
        goto exit;
    }
    ams_core_shmem_alloc_req_t *smem_req = &(smem_alloc->req);
    ams_core_shmem_alloc_rsp_t *smem_resp = &(smem_alloc->resp);

    if (smem_req == NULL || smem_resp == NULL)
    {
        ALOGE("%s:NULL input data!", __func__);
        goto exit;
        rc = AMS_EBADPARAM;
    }
    if (smem_req->client_id < AMS_CORE_CLIENTS_NUM)
    {
        if (ams_data.smem_cli_info[smem_req->client_id].shmem_info.vaddr)
        {
            ALOGD("ams_rsc_mgr_alloc_shmem: "
                  "memory alerady allocated  for client idx %d.",
                  smem_req->client_id);
            if (ams_data.smem_cli_info[smem_req->client_id].spf_mem_handle==0){
                rc = ams_rsc_mgr_map_smem(&ams_data.smem_cli_info[smem_req->client_id]);
            }
            if (rc==AMS_EOK){
                // vaddr returned should be ignored by the client. new mapping with mmap need to be done!
                smem_resp->va_lsw = (uint32_t)((uint64_t)ams_data.smem_cli_info[smem_req->client_id].shmem_info.vaddr & (0xffffffff));
                smem_resp->va_msw = (uint32_t)((uint64_t)ams_data.smem_cli_info[smem_req->client_id].shmem_info.vaddr >> 32);
                smem_resp->mem_map_handle = ams_data.smem_cli_info[smem_req->client_id].spf_mem_handle;
                smem_resp->pa_lsw = ams_data.smem_cli_info[smem_req->client_id].shmem_info.pa_lsw; // check if valid!
                smem_resp->pa_msw = ams_data.smem_cli_info[smem_req->client_id].shmem_info.pa_msw; // check if valid!
#ifdef AMS_USE_DMABUF
               smem_resp->heap_fd = ((ams_shmem_handle_data_t *)ams_data.smem_cli_info[smem_req->client_id].shmem_info.metadata)->heap_fd;
#else
                smem_resp->heap_fd = ((ams_shmem_handle_data_t *)ams_data.smem_cli_info[smem_req->client_id].shmem_info.metadata)->ion_mem_fd;
#endif
            } else {
                ALOGE("ams_rsc_mgr_alloc_shmem: "
                  "memmap failed for client idx %d.",
                  smem_req->client_id);
            }
        }
        else
        {
            ALOGD("ams_rsc_mgr_alloc_shmem: "
                  "allocate memory for client idx %d.",
                  smem_req->client_id);
            ams_data.smem_cli_info[smem_req->client_id].shmem_info.buf_size = smem_req->len; // should be min 4K and 4K aligned!
            rc = ams_shmem_alloc(&(ams_data.smem_cli_info[smem_req->client_id].shmem_info));
#ifdef AMS_CORE_DSP_COMM_EN
            rc = ams_rsc_mgr_map_smem(&ams_data.smem_cli_info[smem_req->client_id]);
#endif
            if (rc == AMS_EOK)
            {
                // get va addr for app
                // vaddr returned should be ignored by the client. new mapping with mmap need to be done!
                smem_resp->va_lsw = (uint32_t)((uint64_t)ams_data.smem_cli_info[smem_req->client_id].shmem_info.vaddr & (0xffffffff));
                smem_resp->va_msw = (uint32_t)((uint64_t)ams_data.smem_cli_info[smem_req->client_id].shmem_info.vaddr >> 32);
                smem_resp->mem_map_handle = ams_data.smem_cli_info[smem_req->client_id].spf_mem_handle;
                smem_resp->pa_lsw = ams_data.smem_cli_info[smem_req->client_id].shmem_info.pa_lsw;
                smem_resp->pa_msw = ams_data.smem_cli_info[smem_req->client_id].shmem_info.pa_msw;
#ifdef AMS_USE_DMABUF
                smem_resp->heap_fd = ((ams_shmem_handle_data_t *)(ams_data.smem_cli_info[smem_req->client_id].shmem_info.metadata))->heap_fd;
                ALOGD("%s:va_lsw=%08x, va_msw=%08x, mem_map_handle=%08x, heap_fd=%08x",
                 __func__, smem_resp->va_lsw, smem_resp->va_msw, smem_resp->mem_map_handle, smem_resp->heap_fd);
#else
                smem_resp->heap_fd = ((ams_shmem_handle_data_t *)ams_data.smem_cli_info[smem_req->client_id].shmem_info.metadata)->ion_mem_fd;
#endif
            }
            else
            {
                ALOGE("ams_rsc_mgr_alloc_shmem: "
                      "failed to allocate for handle %08x, ret %d for client idx %d!",
                      ams_data.smem_cli_info[smem_req->client_id].spf_mem_handle,
                      rc, smem_req->client_id);
            }
        }
    }
    else
    {
        ALOGE("ams_rsc_mgr_alloc_shmem: "
              "wrong client idx %d!",
              smem_req->client_id);
        rc = AMS_EBADPARAM;
    }
exit:
    return rc;
}

int32_t ams_rsc_mgr_free_shmem(ams_core_shmem_free_t *shmem_free)
{
    int32_t rc = AMS_EOK;

    if (shmem_free->client_id < AMS_CORE_CLIENTS_NUM)
    {
#ifdef AMS_CORE_DSP_COMM_EN
        rc = ams_rsc_mgr_unmap_smem(ams_data.smem_cli_info[shmem_free->client_id].spf_mem_handle);
#endif
        ALOGD("ams_rsc_mgr_free_shmem: "
              "unmap handle %08x, ret %d for client idx %d!",
              ams_data.smem_cli_info[shmem_free->client_id].spf_mem_handle,
              rc, shmem_free->client_id);
        if (rc)
        {
            ALOGE("ams_rsc_mgr_free_shmem: "
                  "unable to unmap shared mem for client idx %d!",
                  shmem_free->client_id);
        }
#ifndef AMS_CORE_RSC_FREE_SHMEM
        ams_shmem_free(&(ams_data.smem_cli_info[shmem_free->client_id].shmem_info));
        memset(&(ams_data.smem_cli_info[shmem_free->client_id].shmem_info), 0, sizeof(ams_data.smem_cli_info[shmem_free->client_id].shmem_info));
#endif
        ams_data.smem_cli_info[shmem_free->client_id].spf_mem_handle = 0;
    }
    else
    {
        ALOGE("ams_rsc_mgr_free_shmem: "
              "wrong client idx %d!",
              shmem_free->client_id);
        rc = AMS_EBADPARAM;
    }
    return rc;
}

static int32_t ams_rsc_mgr_resource_config_init(void);

static int32_t ams_rsc_mgr_resource_config_init(void)
{
    int32_t k;
    if (ams_data.ams_core_cfg.init)
    {
        if (ams_data.ams_core_cfg.sh_res_ctrl.dma_types_num <= AMS_NUM_DMA_TYPES)
        {
            rm_shres_init_data.dma_types_num = ams_data.ams_core_cfg.sh_res_ctrl.dma_types_num;
            for (int k = 0; k < rm_shres_init_data.dma_types_num; k++)
            {
                rm_shres_init_data.dma_types[k] = ams_data.ams_core_cfg.sh_res_ctrl.dma_types[k];
                rm_shres_init_data.num_rddma[k] = ams_data.ams_core_cfg.sh_res_ctrl.dma_rd_num[k];
                rm_shres_init_data.num_wrdma[k] = ams_data.ams_core_cfg.sh_res_ctrl.dma_wr_num[k];
            }
        }
    }
    return AMS_EOK;
}

int32_t ams_rsc_mgr_resource_free(void)
{
    int32_t rc = AMS_EOK;
#ifdef AMS_CORE_RSC_FREE_SHMEM
    // default client id 0
    ams_shmem_free(&(ams_data.smem_cli_info[0].shmem_info));
    memset(&(ams_data.smem_cli_info[0].shmem_info), 0, sizeof(ams_data.smem_cli_info[0].shmem_info));
#endif
#ifdef AMS_CORE_DSP_COMM_EN
    rc = ams_rsc_mgr_unshare_resources();
    ALOGD("ams_rsc_mgr_unshare_lpass_resources: ret %d.", rc);
#endif
    rc = ams_rsc_mgr_release_lpass_resources();
    if (rc == AMS_EOK)
    {
        ALOGD("ams_rsc_mgr_release_lpass_resources: all (lpp,dma) resources released OK.");
    }
    else
    {
        ALOGD("ams_rsc_mgr_release_lpass_resources: all (lpp,dma) resources released FAILED.");
    }
    return rc;
}

int32_t ams_rsc_mgr_resource_config(void)
{
    int32_t rc = AMS_EOK;
    // get resources needed from cfg
    rc = ams_rsc_mgr_resource_config_init();

    rc = ams_rsc_mgr_get_lpass_resources();
    ALOGE("ams_rsc_mgr_get_lpass_resources ret %d", rc);
    if (AMS_EOK == rc)
    {
#ifdef AMS_CORE_DSP_COMM_EN
        rc = ams_rsc_mgr_share_resources();
        ALOGE("ams_rsc_mgr_share_resources ret %d", rc);
#endif
        if (AMS_EOK != rc)
        {
            // free lpass resources
            ams_rsc_mgr_release_lpass_resources();
        }
    }
    return rc;
}

int32_t ams_rsc_mgr_mock_rsp(ams_core_tunnel_t *info_p, uint32_t ams_opcode)
{
    int32_t rc = 0;
    ALOGD("%s:req opcode=%08x", __func__, ams_opcode);
    switch (ams_opcode)
    {
    case DSP_AMS_CMD_GET_VERSION:
    {
        dsp_ams_cmdrsp_get_version_t *ver = info_p->rsp->response;
        info_p->rsp->response_code = DSP_AMS_CMDRSP_GET_VERSION;
        if (info_p->req->response_size >= sizeof(dsp_ams_cmdrsp_get_version_t))
        {
            info_p->rsp->response_size = sizeof(dsp_ams_cmdrsp_get_version_t);
            ver->major_version = DSP_AMS_MAJOR_VERSION;
            ver->minor_version = DSP_AMS_MINOR_VERSION;
        }
        else
        {
            ALOGE("%s:rsp buf size %d not enough", __func__, info_p->req->response_size);
            rc = AMS_EBADPARAM;
        }
        break;
    }
    case DSP_AMS_CMD_OPEN_GRAPH:
    {
        dsp_ams_cmd_open_graph_t *op_gr = info_p->req->payload;
        ALOGD("%s:open graph sz = %d", __func__, info_p->req->payload_size);
        info_p->rsp->response_code = DSP_AMS_CMDRSP_OPEN_GRAPH;
        info_p->rsp->response_size = sizeof(dsp_ams_cmdrsp_open_graph_t);
        if (info_p->req->response_size >= sizeof(dsp_ams_cmdrsp_open_graph_t))
        {
            dsp_ams_cmdrsp_open_graph_t *op_gr_rsp = info_p->rsp->response;
            op_gr_rsp->handle = 1;
            op_gr_rsp->status = DSP_AMS_STATUS_OK;
        }
        else
        {
            ALOGE("%s:rsp buf size %d not enough", __func__, info_p->req->response_size);
            rc = AMS_EBADPARAM;
        }
        break;
    }
    case DSP_AMS_CMD_START_GRAPH:
    {
        dsp_ams_cmd_start_graph_t *st_gr = info_p->req->payload;
        ALOGD("%s:start graph fw handle %d", __func__, st_gr->handle);
        struct spf_cmd_basic_rsp *rsp = info_p->rsp->response;
        info_p->rsp->response_code = GPR_IBASIC_RSP_RESULT;
        info_p->rsp->response_size = sizeof(*rsp);
        rsp->opcode = GPR_IBASIC_RSP_RESULT;
        rsp->status = DSP_AMS_STATUS_OK;
        break;
    }
    case DSP_AMS_CMD_STOP_GRAPH:
    {
        dsp_ams_cmd_stop_graph_t *st_gr = info_p->req->payload;
        ALOGD("%s:stop graph fw handle %d", __func__, st_gr->handle);
        struct spf_cmd_basic_rsp *rsp = info_p->rsp->response;
        info_p->rsp->response_code = GPR_IBASIC_RSP_RESULT;
        info_p->rsp->response_size = sizeof(*rsp);
        rsp->opcode = GPR_IBASIC_RSP_RESULT;
        rsp->status = DSP_AMS_STATUS_OK;
        break;
    }
    case DSP_AMS_CMD_CLOSE_GRAPH:
    {
        dsp_ams_cmd_close_graph_t *cl_gr = info_p->req->payload;
        ALOGD("%s:close graph fw handle %d", __func__, cl_gr->handle);
        struct spf_cmd_basic_rsp *rsp = info_p->rsp->response;
        info_p->rsp->response_code = GPR_IBASIC_RSP_RESULT;
        info_p->rsp->response_size = sizeof(*rsp);
        rsp->opcode = GPR_IBASIC_RSP_RESULT;
        rsp->status = DSP_AMS_STATUS_OK;
        break;
    }
    case DSP_AMS_CMD_SET_PARAM:
    {
        dsp_ams_cmd_set_param_t *set_p = info_p->req->payload;
        ALOGD("%s:set param fw handle %d", __func__, set_p->handle);
        struct spf_cmd_basic_rsp *rsp = info_p->rsp->response;
        info_p->rsp->response_code = GPR_IBASIC_RSP_RESULT;
        info_p->rsp->response_size = sizeof(*rsp);
        rsp->opcode = GPR_IBASIC_RSP_RESULT;
        rsp->status = DSP_AMS_STATUS_OK;
        break;
    }
    case DSP_AMS_CMD_GET_PARAM:
    {
        dsp_ams_cmd_get_param_t *get_p = info_p->req->payload;
        ALOGD("%s:get param fw handle %d", __func__, get_p->handle);
        struct dsp_ams_cmdrsp_get_param_t *rsp = info_p->rsp->response;
        info_p->rsp->response_code = DSP_AMS_CMDRSP_GET_PARAM;
        info_p->rsp->response_size = sizeof(*rsp);
        rsp->status = DSP_AMS_STATUS_OK;
        break;
    }
    }

    return rc;
}

int32_t ams_rsc_mgr_send_tunnel_info(ams_core_tunnel_t *info_p, uint32_t ams_opcode)
{
    int32_t rc = AMS_EOK;
#ifdef AMS_CORE_DSP_COMM_EN
    uint32_t send_pkt_len = 0;
    uint32_t pkt_len = 0;
    uint8_t *tunnel_info = NULL;
    struct gpr_packet_t *pkt = NULL, *rcv_pkt = NULL;

    send_pkt_len = info_p->req->payload_size;
    pkt_len = AMS_ALIGN_8BYTE(send_pkt_len);

    rc = ams_gpr_common_allocate_gpr_packet(info_p->req->opcode, GPR_IDS_DOMAIN_ID_APPS_V, GPR_IDS_DOMAIN_ID_MODEM_V,
                                            AMS_GPR_SRC_PORT, AMS_MODULE_INSTANCE_ID, pkt_len, 0, &pkt);
    tunnel_info = GPR_PKT_GET_PAYLOAD(uint8_t, pkt);
    if (info_p->req->opcode == DSP_AMS_CMD_SET_PARAM)
    {
        dsp_ams_cmd_set_param_t *setp_pld = info_p->req->payload;
        uint32_t *pld = (uint32_t *)((uint8_t *)setp_pld + sizeof(*setp_pld));
        ALOGD("ams_send_tunnel_info:Set param dsp payload size %d", sizeof(*setp_pld));
        setp_pld->address_lsw = ((ams_shmem_handle_data_t *)ams_data.smem_cli_info[0].shmem_info.metadata)->heap_fd; // client 0 hardcoded!
        ALOGD("ams_send_tunnel_info:Set param %d, sz %d", pld[0], info_p->req->payload_size);
    }
    else if (info_p->req->opcode == DSP_AMS_CMD_GET_PARAM)
    {
        dsp_ams_cmd_get_param_t *getp_pld = info_p->req->payload;
        getp_pld->address_lsw = ((ams_shmem_handle_data_t *)ams_data.smem_cli_info[0].shmem_info.metadata)->heap_fd; // client 0 hardcoded!
    }
    memcpy(tunnel_info, info_p->req->payload, info_p->req->payload_size);
    // assign memory buffer
    if (info_p->rsp)
    {
        ams_gpr_common_ctx.rsp_buff = info_p->rsp->response;         // move this to ams_gpr_common_send_spf_cmd_wait_for_rsp
        ams_gpr_common_ctx.rsp_buff_sz = info_p->rsp->response_size; // move this to ams_gpr_common_send_spf_cmd_wait_for_rsp
    }
    //
    rc = ams_gpr_common_send_spf_cmd_wait_for_rsp(pkt, ams_gpr_common_ctx.psig_obj, &rcv_pkt);
    if (0 != rc)
    {
        ALOGE("ams_send_tunnel_info: "
              "Can not send command [0x%x] to dsp, err [%d], header [%08x], src_dom[%d], dst_dom[%d], src_port[%d], dst_port[%d], client_data[%d], token[%d]",
              pkt->opcode, rc, pkt->header, pkt->src_domain_id, pkt->dst_domain_id, pkt->src_port, pkt->dst_port, pkt->client_data, pkt->token);
    }
    else
    {
        if (rcv_pkt)
        {
            ALOGD("ams_send_tunnel_info: Opcode received:%d, payload size %d", rcv_pkt->opcode, GPR_PKT_GET_PAYLOAD_BYTE_SIZE(rcv_pkt->header));
            tunnel_info = GPR_PKT_GET_PAYLOAD(uint8_t, rcv_pkt);
            uint32_t *pdata = tunnel_info;
            ALOGD("ams_send_tunnel_info: int data received:%d", pdata[0]);
            if (rcv_pkt->opcode == 0x00013368)
                ALOGD("ams_send_tunnel_info: int data received:%d", pdata[1]);
            if (info_p->rsp)
            {
                (info_p->rsp->response_code) = rcv_pkt->opcode;
                if (((info_p->rsp->response_size)) < GPR_PKT_GET_PAYLOAD_BYTE_SIZE(rcv_pkt->header))
                {
                    ALOGE("ams_send_tunnel_info: Actual response size is more than expected"
                          "Expected response size[%d], received response_size[%d]",
                          (info_p->rsp->response_size), GPR_PKT_GET_PAYLOAD_BYTE_SIZE(rcv_pkt->header));
                }
                memcpy((uint8_t *)(info_p->rsp->response), tunnel_info, GPR_PKT_GET_PAYLOAD_BYTE_SIZE(rcv_pkt->header));
                (info_p->rsp->response_size) = GPR_PKT_GET_PAYLOAD_BYTE_SIZE(rcv_pkt->header);
            }
            if (AMS_EOK != (rc = ams_core_rsc_mgr_update_graph_info(info_p)))
            {
                ALOGE("ams_send_tunnel_info: "
                    "ams_update_graph_info failed command [0x%x]", rcv_pkt->opcode);
            }
        }
        else
        {
            ALOGD("ams_send_tunnel_info: rcvd pkt NULL!");
        }
    }
    /* free mem */
    __gpr_cmd_free(rcv_pkt);
#else
    // provide some fake response from modem
    rc = ams_rsc_mgr_mock_rsp(info_p, ams_opcode);
#endif
    return rc;
}

int32_t ams_rsc_mgr_share_resources(void)
{
    int32_t rc = 0;
    uint32_t *rd_idx = NULL;
    uint32_t *wr_idx = NULL;
    uint32_t send_pkt_len = 0;
    struct apm_cmd_header_t *cmd_header;
    uint32_t pkt_len = 0;
    struct gpr_packet_t *pkt = NULL, *rcv_pkt = NULL;
    struct ams_rsc_mgr_dma_conf_dsp *pdma_conf_dsp = NULL;
    gpr_ibasic_rsp_result_t *rsp = NULL;
    dsp_ams_cmd_shared_resource_config_t *shared_resource_cfg = NULL;

    if (ams_rsc_mgr_shres.dma_types_num == AMS_DMA_INVALID_REQUEST || ams_rsc_mgr_shres.dma_types_num > AMS_NUM_DMA_TYPES ||
        ams_rsc_mgr_shres.lpm_length == AMS_LPM_INVALID_REQUEST)
    {
        ALOGE("ams_rsc_mgr_share_resources: trying to share invalid resources!%d,%d", ams_rsc_mgr_shres.dma_types_num, ams_rsc_mgr_shres.lpm_length);
        return AMS_EBADPARAM;
    }
    send_pkt_len = sizeof(dsp_ams_cmd_shared_resource_config_t);
    for (int i = 0; i < ams_rsc_mgr_shres.dma_types_num; i++)
    {
        send_pkt_len += 2 * sizeof(uint32_t) + ams_rsc_mgr_shres.dma_conf[i].num_rddma * sizeof(uint32_t) + ams_rsc_mgr_shres.dma_conf[i].num_wrdma * sizeof(uint32_t);
    }
    pkt_len = AMS_ALIGN_8BYTE(send_pkt_len);
    rc = ams_gpr_common_allocate_gpr_packet(DSP_AMS_CMD_SHARED_RESOURCE_CONFIG, GPR_IDS_DOMAIN_ID_APPS_V, GPR_IDS_DOMAIN_ID_MODEM_V,
                                            AMS_GPR_SRC_PORT, AMS_MODULE_INSTANCE_ID, pkt_len, 0, &pkt);

    shared_resource_cfg = GPR_PKT_GET_PAYLOAD(dsp_ams_cmd_shared_resource_config_t, pkt);
    shared_resource_cfg->lpm_start_addr_msw = ams_rsc_mgr_shres.lpm_start_addr_msw;
    shared_resource_cfg->lpm_start_addr_lsw = ams_rsc_mgr_shres.lpm_start_addr_lsw;
    shared_resource_cfg->lpm_length = ams_rsc_mgr_shres.lpm_length;
    shared_resource_cfg->num_dma_type = ams_rsc_mgr_shres.dma_types_num;
    ALOGD("ams_rsc_mgr_share_resources: LPM:len %d, msw %08x, lsw %08x", shared_resource_cfg->lpm_length, shared_resource_cfg->lpm_start_addr_msw, shared_resource_cfg->lpm_start_addr_lsw);
    ALOGD("ams_rsc_mgr_share_resources: DMA: num_dma_type %d", shared_resource_cfg->num_dma_type);
    pdma_conf_dsp = (struct ams_rsc_mgr_dma_conf_dsp *)(shared_resource_cfg + 1);

    for (int k = 0; k < ams_rsc_mgr_shres.dma_types_num; k++)
    {
        pdma_conf_dsp->dma_type = ams_rsc_mgr_shres.dma_conf[k].dma_type;
        pdma_conf_dsp->num_rddma = ams_rsc_mgr_shres.dma_conf[k].num_rddma;
        pdma_conf_dsp->num_wrdma = ams_rsc_mgr_shres.dma_conf[k].num_wrdma;
        ALOGD("ams_rsc_mgr_share_resources: DMA: dma_type %d, num_rddma %d, num_wrdma %d",
              ams_rsc_mgr_shres.dma_conf[k].dma_type, ams_rsc_mgr_shres.dma_conf[k].num_rddma, ams_rsc_mgr_shres.dma_conf[k].num_wrdma);
        if (ams_rsc_mgr_shres.dma_conf[k].num_rddma > AMS_NUM_READ_DMA_CHANNELS || ams_rsc_mgr_shres.dma_conf[k].num_wrdma > AMS_NUM_WRITE_DMA_CHANNELS)
        {
            ALOGE("wrong dma number: rd=%d, wr=%d!", ams_rsc_mgr_shres.dma_conf[k].num_rddma, ams_rsc_mgr_shres.dma_conf[k].num_wrdma);
            rc = AMS_EBADPARAM;
            goto memfree;
        }
        rd_idx = (uint32_t *)(&pdma_conf_dsp->num_wrdma + 1);
        for (int i = 0; i < ams_rsc_mgr_shres.dma_conf[k].num_rddma; i++)
        {
            rd_idx[i] = ams_rsc_mgr_shres.dma_conf[k].rddma_idx[i];
            ALOGE("ams_rsc_mgr_share_resources: DMA: rd idx %d", rd_idx[i]);
        }
        wr_idx = &rd_idx[ams_rsc_mgr_shres.dma_conf[k].num_rddma];
        for (int j = 0; j < ams_rsc_mgr_shres.dma_conf[k].num_wrdma; j++)
        {
            wr_idx[j] = ams_rsc_mgr_shres.dma_conf[k].wrdma_idx[j];
            ALOGE("ams_rsc_mgr_share_resources: DMA: wr idx %d", wr_idx[j]);
        }
        pdma_conf_dsp = (struct ams_rsc_mgr_dma_conf_dsp *)&wr_idx[ams_rsc_mgr_shres.dma_conf[k].num_wrdma];
    }
    int retry = 0;
    do
    {
        rc = ams_gpr_common_send_spf_cmd_wait_for_rsp(pkt, ams_gpr_common_ctx.psig_obj, &rcv_pkt);
        if (rcv_pkt) {
            rsp = GPR_PKT_GET_PAYLOAD(gpr_ibasic_rsp_result_t, rcv_pkt);
        }
        if (rc == 0 && rsp)
        {
            if (rcv_pkt->opcode == GPR_IBASIC_RSP_RESULT && rsp->status == 0)
            {
                ALOGE("ams shared res ok ");
                break;
            }
            else
            {
                ALOGE("not expected response %d when share ams resourcses: ", rcv_pkt->opcode);
                rc = AMS_EBADPARAM;
            }
        }
        else
        {
            ALOGE("error [%d] when sharing resources: ", rc);
        }
    } while (rc && ++retry < AMS_RM_RETRY_NUM);
memfree:
    /* free mem */
    if (rcv_pkt)
        __gpr_cmd_free(rcv_pkt);

    return rc;
}

int32_t ams_rsc_mgr_unshare_resources(void)
{
    int32_t rc = AMS_EOK;
    gpr_ibasic_rsp_result_t *rsp = NULL;
    struct gpr_packet_t *pkt = NULL, *rcv_pkt = NULL;
    rc = ams_gpr_common_allocate_gpr_packet(DSP_AMS_CMD_RELEASE_SHARED_RESOURCE, GPR_IDS_DOMAIN_ID_APPS_V, GPR_IDS_DOMAIN_ID_MODEM_V,
                                            AMS_GPR_SRC_PORT, AMS_MODULE_INSTANCE_ID, 0, 0, &pkt);
    rc = ams_gpr_common_send_spf_cmd_wait_for_rsp(pkt, ams_gpr_common_ctx.psig_obj, &rcv_pkt);
    if (rcv_pkt) {
        rsp = GPR_PKT_GET_PAYLOAD(gpr_ibasic_rsp_result_t, rcv_pkt);
    }

    if (rc == AMS_EOK && rsp)
    {
        if (rsp->status == 0)
        {
            ALOGD("ams_rsc_mgr_unshare_resources: unshare resources OK!");
        }
        else
        {
            ALOGD("ams_rsc_mgr_unshare_resources: unshare resources NOK, error from dsp[%d]!", rsp->status);
        }
    }
    else
    {
        ALOGE("ams_rsc_mgr_unshare_resources: unshare resources NOK:, error[%d]!", rc);
    }
    if (rcv_pkt)
        __gpr_cmd_free(rcv_pkt);
    return rc;
}

int32_t ams_rsc_mgr_get_lpass_resources(void)
{
    int32_t rc = AMS_EOK;
    rc = ams_rsc_mgr_get_lpass_resources_dma();
    if (rc == 0)
    {
        rc = ams_rsc_mgr_get_lpass_resources_lpm();
        if (rc != AMS_EOK)
        {
            ams_rsc_mgr_release_lpass_resources_dma();
        }
    }
    return rc;
}

int32_t ams_rsc_mgr_get_lpass_resources_dma(void)
{
    int32_t rc = AMS_EOK;

    struct ams_prm_dma_rsc *lpass_dma_resource_req = NULL;

    uint32_t res_req_len = sizeof(struct ams_prm_dma_rsc) +
                           AMS_NUM_READ_DMA_CHANNELS * sizeof(uint32_t);
    // TODO: check if 4 byte alignment needed for rsp_len
    lpass_dma_resource_req = calloc(1, res_req_len);
    if (lpass_dma_resource_req == NULL)
    {
        ALOGE("ams_rsc_mgr_get_dma_resource: error when allocating memory");
        return ENOMEM;
    }

    memset(lpass_dma_resource_req, 0, res_req_len);
    ams_rsc_mgr_shres.dma_types_num = rm_shres_init_data.dma_types_num;
    for (int k = 0; k < rm_shres_init_data.dma_types_num; k++)
    {
        // request dma of each type
        lpass_dma_resource_req->dma_type = rm_shres_init_data.dma_types[k];

        if (rm_shres_init_data.num_rddma[k] > 0 && rm_shres_init_data.num_rddma[k] <= AMS_NUM_READ_DMA_CHANNELS)
        {
            lpass_dma_resource_req->num_dma_channels = rm_shres_init_data.num_rddma[k];
        }
        else
        {
            ALOGE("ams_rsc_mgr_get_lpass_resource:request:invalid read dma number[%d]!", rm_shres_init_data.num_rddma[k]);
            rc = AMS_EBADPARAM;
            break;
        }
        ALOGD("ams_rsc_mgr_get_lpass_resource:request: rd_dma=%d", lpass_dma_resource_req->num_dma_channels);
        rc = ams_prm_dma_request(RD_DMA, lpass_dma_resource_req);
        if (AMS_EOK != rc)
        {
            ALOGE("ams_rsc_mgr_get_lpass_resource_dma: "
                  "request rd dma, err [%d]",
                  rc);
            break;
        }
        else
        {
            ALOGD("ams_rsc_mgr_get_lpass_resource:resp: rd_dma=%d", lpass_dma_resource_req->num_dma_channels);
            ams_rsc_mgr_shres.dma_conf[k].dma_type = lpass_dma_resource_req->dma_type;
            ams_rsc_mgr_shres.dma_conf[k].num_rddma = lpass_dma_resource_req->num_dma_channels;
            for (int i = 0; i < lpass_dma_resource_req->num_dma_channels; i++)
            {
                ams_rsc_mgr_shres.dma_conf[k].rddma_idx[i] = lpass_dma_resource_req->dma_idx[i];
                ALOGD("ams_rsc_mgr_get_lpass_resource:resp: rd_dma idx=%d", lpass_dma_resource_req->dma_idx[i]);
            }
        }

        if (rm_shres_init_data.num_wrdma[k] > 0 && rm_shres_init_data.num_wrdma[k] <= AMS_NUM_WRITE_DMA_CHANNELS)
        {
            lpass_dma_resource_req->num_dma_channels = rm_shres_init_data.num_wrdma[k];
        }
        else
        {
            ALOGE("ams_rsc_mgr_get_lpass_resource:request:invalid write dma number[%d]!", rm_shres_init_data.num_wrdma[k]);
            rc = AMS_EBADPARAM;
            break;
        }

        ALOGD("ams_rsc_mgr_get_lpass_resource:request: wr_dma=%d", lpass_dma_resource_req->num_dma_channels);
        rc = ams_prm_dma_request(WR_DMA, lpass_dma_resource_req);
        if (AMS_EOK != rc)
        {
            ALOGE("ams_rsc_mgr_get_lpass_resource_dma: "
                  "request wr dma, err [%d]",
                  rc);
            // TODO: free previosly requested channels
            break;
        }
        else
        {
            ALOGD("ams_rsc_mgr_get_lpass_resource:resp: wr_dma=%d", lpass_dma_resource_req->num_dma_channels);
            ams_rsc_mgr_shres.dma_conf[k].dma_type = lpass_dma_resource_req->dma_type;
            ams_rsc_mgr_shres.dma_conf[k].num_wrdma = lpass_dma_resource_req->num_dma_channels;
            for (int i = 0; i < lpass_dma_resource_req->num_dma_channels; i++)
            {
                ams_rsc_mgr_shres.dma_conf[k].wrdma_idx[i] = lpass_dma_resource_req->dma_idx[i];
                ALOGD("ams_rsc_mgr_get_lpass_resource:resp: wr_dma idx=%d", lpass_dma_resource_req->dma_idx[i]);
            }
        }
    }
    if (lpass_dma_resource_req)
        free(lpass_dma_resource_req);

    return rc;
}

static int32_t ams_rsc_mgr_get_lpass_resources_lpm(void)
{
    int32_t rc = AMS_EOK;

    struct ams_prm_lpm_rsc lpass_lpm_resource_req = {0};

    lpass_lpm_resource_req.lpm_type = 0;
    lpass_lpm_resource_req.size_in_bytes = AMS_LPM_LENGTH;

    rc = ams_prm_lpm_request(&lpass_lpm_resource_req);

    if (AMS_EOK != rc)
    {
        ALOGE("ams_rsc_mgr_get_lpass_resource_lpm, err [%d]", rc);
    }
    else
    {
        ams_rsc_mgr_shres.lpm_length = lpass_lpm_resource_req.size_in_bytes;
        ams_rsc_mgr_shres.lpm_start_addr_lsw = lpass_lpm_resource_req.phy_addr_lsw;
        ams_rsc_mgr_shres.lpm_start_addr_msw = lpass_lpm_resource_req.phy_addr_msw;
        ALOGD("ams_rsc_mgr_get_lpm_resource: allocated %d bytes, addr lsw %08x, addr msw =%08x",
              lpass_lpm_resource_req.size_in_bytes, ams_rsc_mgr_shres.lpm_start_addr_lsw, ams_rsc_mgr_shres.lpm_start_addr_msw);
    }

    return rc;
}

int32_t ams_rsc_mgr_release_lpass_resources(void)
{
    int32_t rc = AMS_EOK;
    rc = ams_rsc_mgr_release_lpass_resources_dma();
    rc |= ams_rsc_mgr_release_lpass_resources_lpm();
    return rc;
}

static int32_t ams_rsc_mgr_release_lpass_resources_dma(void)
{
    int32_t rc = AMS_EOK;

    struct ams_prm_dma_rsc *lpass_dma_resource_rel = NULL;

    uint32_t res_rel_len = sizeof(struct ams_prm_dma_rsc) +
                           AMS_NUM_READ_DMA_CHANNELS * sizeof(uint32_t);
    // TODO: check if 4 byte alignment needed for rsp_len
    lpass_dma_resource_rel = calloc(1, res_rel_len);
    if (lpass_dma_resource_rel == NULL)
    {
        ALOGE("ams_rsc_mgr_get_dma_resource: error when allocating memory");
        return ENOMEM;
    }

    for (int k = 0; k < ams_rsc_mgr_shres.dma_types_num; k++)
    {

        lpass_dma_resource_rel->dma_type = ams_rsc_mgr_shres.dma_conf[k].dma_type;
        lpass_dma_resource_rel->num_dma_channels = ams_rsc_mgr_shres.dma_conf[k].num_rddma;

        for (int i = 0; i < lpass_dma_resource_rel->num_dma_channels; i++)
        {
            lpass_dma_resource_rel->dma_idx[i] = ams_rsc_mgr_shres.dma_conf[k].rddma_idx[i];
        }
        rc = ams_prm_dma_release(RD_DMA, lpass_dma_resource_rel);
        if (AMS_EOK != rc)
        {
            ALOGE("ams_rsc_mgr_releasse_lpass_resource_dma: rd dma err [%d]", rc);
        }
        else
        {
            ALOGD("ams_rsc_mgr_release_dma_resource: rd dma resources released OK.");
            // invalidate resources
            memset(ams_rsc_mgr_shres.dma_conf[k].rddma_idx, AMS_DMA_INVALID_REQUEST, ams_rsc_mgr_shres.dma_conf[k].num_rddma * sizeof(ams_rsc_mgr_shres.dma_conf[k].rddma_idx[0]));
            ams_rsc_mgr_shres.dma_conf[k].num_rddma = AMS_DMA_INVALID_REQUEST;
        }
        lpass_dma_resource_rel->num_dma_channels = ams_rsc_mgr_shres.dma_conf[k].num_wrdma;
        for (int j = 0; j < lpass_dma_resource_rel->num_dma_channels; j++)
        {
            lpass_dma_resource_rel->dma_idx[j] = ams_rsc_mgr_shres.dma_conf[k].wrdma_idx[j];
        }
        rc = ams_prm_dma_release(WR_DMA, lpass_dma_resource_rel);
        if (AMS_EOK != rc)
        {
            ALOGE("ams_rsc_mgr_releasse_lpass_resource_dma: wr dma err [%d]", rc);
        }
        else
        {
            ALOGD("ams_rsc_mgr_release_dma_resource: wr dma resources released OK.");
            // invalidate resources
            memset(ams_rsc_mgr_shres.dma_conf[k].wrdma_idx, AMS_DMA_INVALID_REQUEST, ams_rsc_mgr_shres.dma_conf[k].num_wrdma * sizeof(ams_rsc_mgr_shres.dma_conf[k].wrdma_idx[0]));
            ams_rsc_mgr_shres.dma_conf[k].num_wrdma = AMS_DMA_INVALID_REQUEST;
        }
    }
    if (lpass_dma_resource_rel)
        free(lpass_dma_resource_rel);

    return rc;
}

static int32_t ams_rsc_mgr_release_lpass_resources_lpm(void)
{
    int32_t rc = AMS_EOK;

    struct ams_prm_lpm_rsc lpass_lpm_resource_rel = {0};
    if (ams_rsc_mgr_shres.lpm_length == AMS_LPM_INVALID_REQUEST)
    {
        ALOGE("ams_rsc_mgr_release_lpm_resource: trying to release invalid lpm resources!");
        return AMS_EBADPARAM;
    }
    lpass_lpm_resource_rel.lpm_type = 0; // TODO:don't hardcode(use init)
    lpass_lpm_resource_rel.size_in_bytes = ams_rsc_mgr_shres.lpm_length;
    lpass_lpm_resource_rel.phy_addr_lsw = ams_rsc_mgr_shres.lpm_start_addr_lsw;
    lpass_lpm_resource_rel.phy_addr_msw = ams_rsc_mgr_shres.lpm_start_addr_msw;

    rc = ams_prm_lpm_release(&lpass_lpm_resource_rel);

    if (AMS_EOK != rc)
    {
        ALOGE("ams_rsc_mgr_release_lpass_resource_lpm: err [%d]", rc);
    }
    else
    {
        ALOGD("ams_rsc_mgr_release_resource_lpm: lpm resources released OK.");
        // invalidate resources
        ams_rsc_mgr_shres.lpm_length = AMS_LPM_INVALID_REQUEST;
        ams_rsc_mgr_shres.lpm_start_addr_lsw = AMS_LPM_INVALID_REQUEST;
        ams_rsc_mgr_shres.lpm_start_addr_msw = AMS_LPM_INVALID_REQUEST;
    }

    return rc;
}

int32_t ams_rsc_mgr_pcm_logging_enable(uint16_t log_id, uint16_t enable)
{
    int32_t rc = AMS_EOK;
    uint32_t send_pkt_len = 0;

    struct gpr_packet_t *pkt = NULL, *rcv_pkt = NULL;
    dsp_ams_cmd_set_prop_t *ams_set_prop = NULL;
    dsp_ams_ep_pcm_logging_t *ams_pcm_logging = NULL;
    gpr_ibasic_rsp_result_t *rsp = NULL;
    send_pkt_len = sizeof(dsp_ams_cmd_set_prop_t) + sizeof(dsp_ams_ep_pcm_logging_t);

    rc = ams_gpr_common_allocate_gpr_packet(DSP_AMS_CMD_SET_PROP, GPR_IDS_DOMAIN_ID_APPS_V, GPR_IDS_DOMAIN_ID_MODEM_V,
                                            AMS_GPR_SRC_PORT, AMS_MODULE_INSTANCE_ID, send_pkt_len, 0, &pkt);

    ams_set_prop = GPR_PKT_GET_PAYLOAD(dsp_ams_cmd_set_prop_t, pkt);
    ams_set_prop->prop_id = DSP_AMS_PROP_ID_END_POINT_PCM_LOGGING;
    ams_set_prop->prop_size = sizeof(dsp_ams_ep_pcm_logging_t);

    ams_pcm_logging = (dsp_ams_ep_pcm_logging_t *)(ams_set_prop + 1);

    ams_pcm_logging->enable = enable;
    ams_pcm_logging->log_id = log_id;

    rc = ams_gpr_common_send_spf_cmd_wait_for_rsp(pkt, ams_gpr_common_ctx.psig_obj, &rcv_pkt);
    if (rcv_pkt) {
        rsp = GPR_PKT_GET_PAYLOAD(gpr_ibasic_rsp_result_t, rcv_pkt);
    }
    if (rc == 0 && rcv_pkt && rsp)
    {
        if (rcv_pkt->opcode == GPR_IBASIC_RSP_RESULT && rsp->status == 0)
        {
            ALOGD("ams set PCM logging ok ");
        }
        else
        {
            ALOGE("not expected response %d when set PCM logging: ", rcv_pkt->opcode);
            rc = AMS_EBADPARAM;
        }
    }
    else
    {
        ALOGE("error [%d] when set PCM logging: ", rc);
    }

    /* free mem */
    if (rcv_pkt)
        __gpr_cmd_free(rcv_pkt);

    return rc;
}

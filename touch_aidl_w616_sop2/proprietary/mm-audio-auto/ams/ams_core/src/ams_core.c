/**
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#define LOG_TAG "ams_core"
#include <log/log.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "ams_core.h"
#include "ams_core_ioctl.h"
#include "ams_rsc_mgr.h"
#include "ams_endpoint_mgr.h"
#include "ams_gpr_common.h"
#include "ams_prm.h"
#ifdef AMS_SSR_SUPPORT_EN
#include "ams_osal_ssr.h"
#endif
#include "ams_osal_error.h"

#define AMS_SSR_CALLBACK_SLEEP_MS 10
#define AMS_SSR_CALLBACK_RETRY_CNT 10

struct ams_core_data ams_data;

#ifdef AMS_SSR_SUPPORT_EN
static int32_t ams_core_ssr_init(int (*event_handler)(uint32_t ss_id_mask, uint32_t event_id, void *ctx),
    void *ctx);

static int32_t ams_core_ssr_init(int (*event_handler)(uint32_t ss_id_mask, uint32_t event_id, void *ctx),
    void *ctx)
{
    int32_t r = AMS_EOK;
    ams_data.ssr.adsp_st = 0;
    ams_data.ssr.mdsp_st = 0;

    r = ams_osal_mutex_create(&ams_data.ssr.mutex);
    if (0 != r)
    {
        ALOGE("Failed to create mutex!");
        goto exit;
    }

    r = ams_osal_ssr_init("ams_core", event_handler, ctx);
    if (r){
        ALOGE("SSR setup failed!");
        r = AMS_EFAILED;
    }

exit:
    return r;
}

static int ams_ssr_callback_function(
    uint32_t ss_id_mask,
    uint32_t event_id,
    void *ctx)
{
    int r = AMS_EOK;
    struct ams_core_data *pams_data = ctx;//ams_data
    struct ams_graph *gr = NULL;
    struct ams_cb *user_cb = NULL;
    int retry_cnt = 0;
    static int ready = 0;
    if (!(ss_id_mask & SS_ID_LPASS) && !(ss_id_mask & SS_ID_MODEM))
    {
        ALOGE("SSR invalid mask ID %08X", ss_id_mask);
        r = AMS_EBADPARAM;
        goto exit;
    }
    switch (event_id)
    {
    case SSR_EVENT_RESTART_START:
        ams_osal_mutex_lock(ams_data.ssr.mutex);
        if (ss_id_mask & SS_ID_LPASS)
        {
            ams_data.ssr.adsp_st = SSR_EVENT_RESTART_START;
            ALOGE("LPASS SSR_EVENT_RESTART_START");
        }
        else if (ss_id_mask & SS_ID_MODEM)
        {
            ams_data.ssr.mdsp_st = SSR_EVENT_RESTART_START;
            ams_rsc_mgr_mdsp_ssr_cleanup();
            ALOGE("MDSP SSR_EVENT_RESTART_START");
        }
        ams_osal_mutex_unlock(ams_data.ssr.mutex);
        break;
    case SSR_EVENT_RESTART_COMPLETE:
        ams_osal_mutex_lock(ams_data.ssr.mutex);
        if (ss_id_mask & SS_ID_LPASS)
        {
            ams_data.ssr.adsp_st = SSR_EVENT_RESTART_COMPLETE;
            ALOGE("LPASS SSR_EVENT_RESTART_COMPLETE");
            do
            {
                r = ams_rsc_mgr_get_lpass_resources();
                ALOGE("ams_rsc_mgr_get_lpass_resources ret %d", r);
                if (!r){
                    ready = 1;
                    break;
                }
                usleep(AMS_SSR_CALLBACK_SLEEP_MS * 1000);
            }while (++retry_cnt<AMS_SSR_CALLBACK_RETRY_CNT);
        }
        else if (ss_id_mask & SS_ID_MODEM)
        {
            ams_data.ssr.mdsp_st = SSR_EVENT_RESTART_COMPLETE;
            ALOGE("MDSP SSR_EVENT_RESTART_COMPLETE");
        }
        ams_osal_mutex_unlock(ams_data.ssr.mutex);
        if (ams_data.ssr.adsp_st == SSR_EVENT_RESTART_COMPLETE && ams_data.ssr.mdsp_st == SSR_EVENT_RESTART_COMPLETE)
        {
            retry_cnt = 0;
            if (ready){
                do
                {
                    r = ams_rsc_mgr_share_resources();
                    ALOGE("ams_rsc_mgr_share_resources ret %d", r);
                    if (!r){
                        ready = 1;
                        break;
                    }
                }while (++retry_cnt<AMS_SSR_CALLBACK_RETRY_CNT);
            } else {
                // ams core recovery from SSR failed. invalidate handle
                ams_data.g_ams_handle = 0;
            }
            // clean status
            ams_osal_mutex_lock(ams_data.ssr.mutex);
            ams_data.ssr.adsp_st = 0;
            ams_data.ssr.mdsp_st = 0;
            ams_osal_mutex_unlock(ams_data.ssr.mutex);
        }
        break;
    default:
        ALOGE("SSR:Invalid event notification %d", event_id);
        break;
    }
exit:
    ALOGD("SSR:%s exit", __func__);
    return r;
}
#else
static int ams_ssr_callback_function(
    uint32_t ss_id_mask,
    uint32_t event_id,
    void *ctx)
{
    return 0;
}
#endif

int32_t ams_core_init(void)
{
    uint32_t handle; // hopefully not null

    int32_t ret = 0;
    ALOGD("Enter");
    if (ams_data.g_ams_handle)
    {
        ALOGD("AMS Core alreaydy initialized. Skip.");
        goto exit;
    }
    if (ams_data.g_ams_handle == 0)
    {
        ALOGD("Failed to allocate ams handle!Set to 1"); // TODO: add handle generation!
        ams_data.g_ams_handle = 1;
    }

    ret = ams_osal_mutex_create(&ams_data.mutex);
    if (0 != ret)
    {
        ALOGE("Failed to create mutex!");
        goto exit;
    }
    ret = ams_core_cfg_init(&ams_data.ams_core_cfg);
    if (0 != ret)
    {
         ALOGE("Failed to init cfg!");
    }

    ret = ams_gpr_common_ctx_init();
    if (0 != ret)
    {
        ALOGE("Failed in gpr ctx init!");
        goto mutex_destroy;
    }

    ret = ams_prm_init();
    if (0 != ret)
    {
        ALOGE("Failed in prm init!");
        goto gpr_ctx_deinit;
    }
    ret = ams_rsc_mgr_shmem_init();
    if (0 != ret)
    {
        ALOGE("Failed in shmem init!");
        goto prm_deinit;
    }

    ret = ams_rsc_mgr_resource_config();
    if (0 != ret)
    {
        ALOGE("Failed in resource configuration!");
        goto shmem_deinit;
    }
#ifdef AMS_SSR_SUPPORT_EN
    ret = ams_core_ssr_init(ams_ssr_callback_function, (void*) &ams_data);
    if (0 != ret)
    {
        ALOGE("Failed in core ssr init!");
        // goto shmem_deinit;
    }
#endif
    goto exit;
shmem_deinit:
    ams_rsc_mgr_shmem_deinit();
prm_deinit:
    ams_prm_deinit();
gpr_ctx_deinit:
    ams_gpr_common_ctx_deinit();
mutex_destroy:
    ams_osal_mutex_destroy(ams_data.mutex);
    ams_data.g_ams_handle = 0;
exit:
    return ret;
}

int32_t ams_core_deinit(void)
{
    int32_t ret = 0;
    ALOGD("Enter");
    ret = ams_rsc_mgr_resource_free();
    if (ret)
        ALOGE("Error [%d] when releasing resources", ret);
    ret = ams_rsc_mgr_shmem_deinit();
    if (ret)
        ALOGE("Error [%d] when releasing shmem", ret);
    ret = ams_prm_deinit();
    if (ret)
        ALOGE("Error [%d] when deinit prm", ret);
    ret = ams_gpr_common_ctx_deinit();
    if (ret)
        ALOGE("Error [%d] when deinit gpr ctx", ret);
#ifdef AMS_SSR_SUPPORT_EN
    ret = ams_osal_ssr_deinit();
    if (ret)
        ALOGE("Error [%d] when deinit ssr", ret);
#endif
    if (ams_data.g_ams_handle)
        ams_data.g_ams_handle = 0;
    if (ams_data.mutex)
    {
        ams_osal_mutex_destroy(ams_data.mutex);
    }
    memset(&ams_data, 0, sizeof(ams_data));
    return ret;
}

uint32_t ams_core_open(void)
{
    uint32_t handle = 0;
    ALOGD("Enter");
    ams_osal_mutex_lock(ams_data.ssr.mutex);
    if (!ams_data.ssr.adsp_st && !ams_data.ssr.mdsp_st){
        handle = ams_data.g_ams_handle;
    } else {
        ALOGE("SSR is still active!");
    }
    ams_osal_mutex_unlock(ams_data.ssr.mutex);

    return handle;
}

int32_t ams_core_ioctl(uint32_t hndl, uint32_t cmd, void *params, uint32_t size)
{
    int32_t ret = 0;
    ams_osal_mutex_lock(ams_data.mutex);
    ALOGD("ams_core_ioctl: handle=0x%x,ioctl=0x%x,cmd_len=0x%x",
          hndl, cmd, size);
    if ((hndl) && (params))
    {
        switch (cmd)
        {
        case AMS_CORE_CMD_TUNNEL:
        {
            if (size < sizeof(ams_core_tunnel_t))
            {
                ALOGE("ams_core_ioctl: "
                      "Wrong payload size for AMS_CORE_TUNNEL: "
                      "cmd=%x params_len=%d, payload=%lu",
                      cmd,
                      size, sizeof(ams_core_tunnel_t));

                ret = EINVAL;
            }
            else
            {
                ams_core_tunnel_t *tunnel_info = (ams_core_tunnel_t *)(params);
                ret = ams_rsc_mgr_send_tunnel_info(params, tunnel_info->req->opcode);
            }
            break;
        }
        case AMS_CORE_CMD_ENABLE_ENDPOINT:
        {
            ams_core_endpoint_t *enable_endpoint = (ams_core_endpoint_t *)params;
            if (enable_endpoint && !enable_endpoint->flags)
            {
                ret = ams_endpoint_clk_enable(enable_endpoint);
            }
            else
            {
                ALOGD("Skip ep clk enablement(port=%d,flag=%d) ", enable_endpoint->tdm_params.hw_interface_id, enable_endpoint->flags);
            }
            break;
        }
        case AMS_CORE_CMD_DISABLE_ENDPOINT:
        {
            ams_core_endpoint_t *disable_endpoint = (ams_core_endpoint_t *)params;
            if (disable_endpoint && !disable_endpoint->flags)
            {
                ret = ams_endpoint_clk_disable(disable_endpoint);
            }
            else
            {
                ALOGD("Skip ep clk disablement(port=%d,flag=%d) ", disable_endpoint->tdm_params.hw_interface_id, disable_endpoint->flags);
            }
            break;
        }
        case AMS_CORE_CMD_SET_CLK_ATTR:
        {
            ams_core_clk_attr_t *ep_clk_attr = (ams_core_clk_attr_t *)params;
            ret = ams_endpoint_clk_attr_set(ep_clk_attr->ep_id, ep_clk_attr->attr);
            break;
        }
        case AMS_CORE_CMD_ALLOC_SHMEM:
        {
            ams_core_shmem_alloc_t *smem_alloc = (ams_core_shmem_alloc_t *)params;
            ret = ams_rsc_mgr_alloc_shmem(smem_alloc);
            ALOGD("ams_core_ioctl: "
                  "ams_cmd_alloc_shmem ret %d",
                  ret);
            break;
        }
        case AMS_CORE_CMD_FREE_SHMEM:
        {
            ams_core_shmem_free_t *shmem_free = (ams_core_shmem_free_t *)params;
            ret = ams_rsc_mgr_free_shmem(shmem_free);
            ALOGD("ams_core_ioctl: "
                  "ams_cmd_free_shmem ret %d",
                  ret);
            break;
        }
        case AMS_CORE_CMD_GET_CACHED_GRAPH_NUM:
        {
            ams_core_cached_graph_num_t *grap_num = (ams_core_cached_graph_num_t *)params;
            grap_num->num = ams_data.gr_infoq_el_num;
            ALOGD("ams_core_ioctl: "
                    "num graph cached %d",grap_num->num);
            break;
        }
        case AMS_CORE_CMD_GET_CACHED_GRAPH_INFO:
        {
            ams_core_cached_graph_info_t *graph_info_req = (ams_core_cached_graph_info_t *)params;
            struct ams_core_graph_info *graph_info = NULL;
            if(graph_info_req->idx < AMS_CORE_USER_GRAPH_MAX)
                graph_info = &ams_data.gr_infoq[graph_info_req->idx];
            if (graph_info)
            {
                ALOGD("ams_core_ioctl: "
                    "graph_info_sz %d", graph_info->graph_pld_sz);
                graph_info_req->gr_descr_sz = graph_info->graph_pld_sz;
                graph_info_req->state = graph_info->state;
                graph_info_req->fw_handle = graph_info->handle;
            }
            else
            {
                ALOGE("ams_core_ioctl: "
                    "graph_info not found idx %d", graph_info_req->idx);
                ret = AMS_EBADPARAM;
            }
            break;
        }
        case AMS_CORE_CMD_GET_CACHED_GRAPH_DESCR:
        {
            ams_core_cached_graph_descr_t *graph_descr_req = (ams_core_cached_graph_descr_t*)params;
            struct ams_core_graph_info *graph_info = NULL;
            if(graph_descr_req->idx < AMS_CORE_USER_GRAPH_MAX)
                graph_info = &ams_data.gr_infoq[graph_descr_req->idx];
            ALOGD("ams_core_ioctl: "
                    "graph_descr req sz %d", graph_descr_req->gr_descr_sz);
            if ( graph_info )
            {
                if (graph_descr_req->gr_descr_sz >= graph_info->graph_pld_sz)
                {
                    memcpy(graph_descr_req->gr_descr, graph_info->graph_pld, graph_info->graph_pld_sz);
                }
                else
                {
                    ALOGE("ams_core_ioctl: "
                           "graph_descr req sz %d < needed %d", graph_descr_req->gr_descr_sz, graph_info->graph_pld_sz);
                    ret = AMS_EBADPARAM;
                }
            }
            else
            {
                ALOGE("ams_core_ioctl: "
                    "graph_descr not found idx %d", graph_descr_req->idx);
                ret = AMS_EBADPARAM;
            }
            break;
        }
        default:
        {
            ret = EOPNOTSUPP;
            ALOGD("ams_core_ioctl: "
                  "Wrong command passed : cmd=0x%x handle=(%x) ",
                  cmd, hndl);
            break;
        }
        }
    }
    else
    {
        ALOGE("ams_core_ioctl: "
              "Wrong ams handle or.and params is NULL !: "
              "handle=(%x) params=%p",
              hndl, params);
        ret = EINVAL;
    }

    ALOGD("ams_core_ioctl: "
          "ams_core_ioctl end - handle=0x%x,ioctl=0x%x,ret=0x%x",
          hndl, cmd, ret);
    ams_osal_mutex_unlock(ams_data.mutex);
    return ret;
}

int32_t ams_core_close(uint32_t hndl)
{
    int32_t ret = 0;
    ALOGD("Enter");
    if (!hndl)
    {
        ret = EINVAL;
    }
    return ret;
}

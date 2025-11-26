/**
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>

#include "ams_impl.h"
#include "ams_util.h"
#include "ams_platform.h"
#ifdef AMS_SSR_SUPPORT_EN
#include "ams_osal_ssr.h"
#endif
unsigned int g_ams_test_log_enable = 1;
unsigned int g_ams_test_log_dbg_lvl = 2; // 0 - only errors, 1 - info, >1 -all
unsigned int g_ams_test_log_show_tid = 0;

#define AMS_SSR_CALLBACK_SLEEP_MS 10
#define AMS_SSR_CALLBACK_RETRY_CNT 10

#define CHECK_AMS_HANDLE(p_ses)            \
    {                                      \
        if (p_ses == NULL)                 \
        {                                  \
            r = AMS_STATUS_INPUT_ERROR;    \
            AMS_LIB_LOGE("Null session!"); \
            goto exit;                     \
        }                                  \
    }
#define CHECK_GRP_HANDLE(gh)                    \
    {                                           \
        if (gh == NULL)                         \
        {                                       \
            r = AMS_STATUS_INPUT_ERROR;         \
            AMS_LIB_LOGE("Null graph handle!"); \
            goto exit;                          \
        }                                       \
    }
#ifdef AMS_SSR_SUPPORT_EN
static int ams_ssr_callback_function(
    uint32_t ss_id_mask,
    uint32_t event_id,
    void *ctx)
{
    int r = AMS_STATUS_SUCCESS;
    struct ams_session *p_ses = ctx;
    struct ams_graph *gr = NULL;
    struct ams_cb *user_cb = NULL;
    int32_t reslock_cb_lst = 0;
    int32_t reslock_gr_lst = 0;
    int32_t reslock_gr_el = 0;
    int32_t reslock_smem = 0;
    int32_t reslock_ssr_st = 0;
    CHECK_AMS_HANDLE(p_ses);
    struct ams_cb_list *cb_list = &p_ses->cb_lst;

    if (!(ss_id_mask & SS_ID_LPASS) && !(ss_id_mask & SS_ID_MODEM))
    {
        AMS_LIB_LOGE("SSR invalid mask ID %08X", ss_id_mask);
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }
    switch (event_id)
    {
    case SSR_EVENT_RESTART_START:
        if (ss_id_mask & SS_ID_LPASS)
        {
            LOCK_FOR_WRITE(reslock_ssr_st, p_ses->drv.ssr.rwlock);
            p_ses->drv.ssr.ssr_adsp_st = SSR_EVENT_RESTART_START;
            UNLOCK_READ_WRITE(reslock_ssr_st, p_ses->drv.ssr.rwlock);
            AMS_LIB_LOGE("LPASS SSR_EVENT_RESTART_START");
        }
        else if (ss_id_mask & SS_ID_MODEM)
        {
            LOCK_FOR_WRITE(reslock_ssr_st, p_ses->drv.ssr.rwlock);
            p_ses->drv.ssr.ssr_mdsp_st = SSR_EVENT_RESTART_START;
            UNLOCK_READ_WRITE(reslock_ssr_st, p_ses->drv.ssr.rwlock);
            AMS_LIB_LOGE("MDSP SSR_EVENT_RESTART_START");
            LOCK_FOR_READ(reslock_gr_lst, p_ses->gr_lst.rwlock);
            gr = p_ses->gr_lst.first;
            while (gr != NULL)
            {
                LOCK_FOR_WRITE(reslock_gr_el, gr->rwlock);
                if (gr->fw_status == DSP_AMS_GRAPH_STARTED)
                {
                    // disable all endpoints
                    ams_platform_set_endpoints_state(&p_ses->drv, gr, 0);
                }
                gr->fw_status = DSP_AMS_GRAPH_INIT;
                gr->fw_handle = 0;
                UNLOCK_READ_WRITE(reslock_gr_el, gr->rwlock);
                gr = gr->nxt_el;
            }
            UNLOCK_READ_WRITE(reslock_gr_lst, p_ses->gr_lst.rwlock);
        }
        LOCK_FOR_READ(reslock_cb_lst, p_ses->cb_lst.rwlock);
        user_cb = cb_list->first;
        while (user_cb != NULL)
        {
            user_cb->func(AMS_EVENT_SSR_STARTED,
                          user_cb->data);
            user_cb = user_cb->nxt_el;
        }
        UNLOCK_READ_WRITE(reslock_cb_lst, p_ses->cb_lst.rwlock);
        break;
    case SSR_EVENT_RESTART_COMPLETE:
        LOCK_FOR_WRITE(reslock_ssr_st, p_ses->drv.ssr.rwlock);
        if (ss_id_mask & SS_ID_LPASS)
        {
            p_ses->drv.ssr.ssr_adsp_st = SSR_EVENT_RESTART_COMPLETE;
            AMS_LIB_LOGE("LPASS SSR_EVENT_RESTART_COMPLETE");
        }
        else if (ss_id_mask & SS_ID_MODEM)
        {
            p_ses->drv.ssr.ssr_mdsp_st = SSR_EVENT_RESTART_COMPLETE;
            AMS_LIB_LOGE("MDSP SSR_EVENT_RESTART_COMPLETE");
        }
        UNLOCK_READ_WRITE(reslock_ssr_st, p_ses->drv.ssr.rwlock);
        if (p_ses->drv.ssr.ssr_adsp_st == SSR_EVENT_RESTART_COMPLETE && p_ses->drv.ssr.ssr_mdsp_st == SSR_EVENT_RESTART_COMPLETE)
        {
            int retry_cnt = 0;
            int ready = 0;
            do
            {
                if (platform_drv_ready(&p_ses->drv))
                {
                    AMS_LIB_LOGE("SSR active: plarform drv READY");
                    ready = 1;
                    break;
                }
                AMS_LIB_LOGD("SSR active: wait for plarform drv cnt[%d]", retry_cnt);
                usleep(AMS_SSR_CALLBACK_SLEEP_MS * 1000);
            }while (!ready && ++retry_cnt<AMS_SSR_CALLBACK_RETRY_CNT);
            if (!ready)
            {
                AMS_LIB_LOGE("Not able to open platform driver after SSR! User Callbacks on SSR Complete are skipped!");
            }
            else
            {
                // shared mem map
                LOCK_FOR_WRITE(reslock_smem, p_ses->drv.smem.rwlock);
                r = ams_platform_drv_alloc_smem(&p_ses->drv);
                AMS_LIB_LOGD("alloc smem ret (%d)", r);
                UNLOCK_READ_WRITE(reslock_smem, p_ses->drv.smem.rwlock);
                // call user callbacks
                LOCK_FOR_READ(reslock_cb_lst, p_ses->cb_lst.rwlock);
                user_cb = cb_list->first;
                while (user_cb != NULL)
                {
                    user_cb->func(
                        AMS_EVENT_SSR_COMPLETED,
                        user_cb->data);
                    user_cb = user_cb->nxt_el;
                }
                UNLOCK_READ_WRITE(reslock_cb_lst, p_ses->cb_lst.rwlock);
            }
            // clean status
            LOCK_FOR_WRITE(reslock_ssr_st, p_ses->drv.ssr.rwlock);
            p_ses->drv.ssr.ssr_adsp_st = 0;
            p_ses->drv.ssr.ssr_mdsp_st = 0;
            UNLOCK_READ_WRITE(reslock_ssr_st, p_ses->drv.ssr.rwlock);
        }
        break;
    default:
        AMS_LIB_LOGE("SSR:Invalid event notification %d", event_id);
        break;
    }
exit:
    AMS_LIB_LOGD("SSR:%s exit", __func__);
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
ams_status_t ams_init(
    ams_session_t *amss)
{
    struct ams_session *p_ses = NULL;
    ams_status_t r = AMS_STATUS_SUCCESS;
    uint32_t fw_version;

    // create session context
    p_ses = calloc(1, sizeof(struct ams_session));
    if (p_ses == NULL)
    {
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }
    r = ams_platform_open(&p_ses->drv);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Platform open error (%d)!", r);
        goto cleanup;
    }
    AMS_LIB_LOGD("Platform open ret (%d)!", r);
    r = ams_platform_get_version(&p_ses->drv, &fw_version);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Faied to get FW version (%d)!", r);
        goto cleanup;
    }
    // PORTME
    r = ams_platform_drv_ssr_init("ams_lib", &p_ses->drv.ssr, ams_ssr_callback_function, (void *)p_ses);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Faied to init SSR (%d)!", r);
        //goto cleanup;
    }
    // init locks
    if (ams_osal_mutex_create(&(p_ses->gr_lst.rwlock)))
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Unable to init rwlock!");
        goto cleanup;
    }
    // init locks
    if (ams_osal_mutex_create(&(p_ses->cb_lst.rwlock)))
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Unable to init rwlock!");
        goto cleanup;
    }
cleanup:
    if (r != AMS_STATUS_SUCCESS)
    {
        ams_deinit(&p_ses);
    }
exit:
    *amss = (ams_session_t)p_ses;
    return r;
}

ams_status_t ams_deinit(
    ams_session_t *amss)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_session *p_ses = *amss;
    struct ams_graph_list *p_gr_lst = NULL;
    struct ams_graph *p_gr = NULL;
    struct ams_graph *p_nxt_gr = NULL;
    struct ams_cb_list *p_cb_lst = NULL;
    struct ams_cb *p_cb = NULL;
    struct ams_cb *p_nxt_cb = NULL;
    CHECK_AMS_HANDLE(p_ses);
    p_gr_lst = &p_ses->gr_lst;
    if (p_gr_lst != NULL)
    {
        p_gr = p_gr_lst->first;
        AMS_LIB_LOGD("Empty graph list of %d elements", p_gr_lst->el_num);
        while (p_gr != NULL)
        {
            p_nxt_gr = p_gr->nxt_el;
            ams_graph_handle_t gh = (p_gr->handle);
            AMS_LIB_LOGI("Graph %lx was not destroyed properly!Try to destroy", (uint64_t)gh);
            if ((r = ams_destroy_graph(*amss, &gh)) == AMS_STATUS_SUCCESS)
            {
                AMS_LIB_LOGI("Graph destroyed OK");
            }
            else
            {
                AMS_LIB_LOGI("Graph destroyed NOK");
            }
            p_gr = p_nxt_gr;
        }
    }
    p_cb_lst = &p_ses->cb_lst;
    if (p_cb_lst != NULL)
    {
        p_cb = p_cb_lst->first;
        while (p_cb != NULL)
        {
            p_nxt_cb = p_cb->nxt_el;
            r |= ams_deregister_callback(*amss, p_cb->func);
            p_cb = p_nxt_cb;
        }
    }
    r = ams_platform_drv_ssr_deinit(&p_ses->drv.ssr);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Error in SSR deinit");
    }
    r = ams_platform_close(&p_ses->drv);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Error when closing platform");
    }
    if (!p_ses->gr_lst.rwlock || ams_osal_mutex_destroy(p_ses->gr_lst.rwlock))
    {
        AMS_LIB_LOGE("Unable to destroy rwlock!");
    }
    if (!p_ses->cb_lst.rwlock || ams_osal_mutex_destroy(p_ses->cb_lst.rwlock))
    {
        AMS_LIB_LOGE("Unable to destroy rwlock!");
    }

    free(*amss);
    *amss = NULL;
exit:
    return r;
}

ams_status_t ams_create_graph(
    ams_session_t amss,
    ams_graph_basic_params_t *p, // [in]
    ams_graph_handle_t *gh)      // [out]
{
    ams_status_t r = AMS_STATUS_SUCCESS;

    struct ams_session *p_ses = amss;
    struct ams_graph *p_gr = NULL;
    int32_t reslock_gr_lst = 0;
    CHECK_AMS_HANDLE(p_ses);
    if (gh == NULL)
    {
        AMS_LIB_LOGE("Wrong graph handle!");
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    if (ams_util_check_graph_baseparam(p) != AMS_STATUS_SUCCESS)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Wrong graph base param!");
        goto exit;
    }
    LOCK_FOR_WRITE(reslock_gr_lst, p_ses->gr_lst.rwlock);
    r = ams_util_graph_list_add_new_graph(amss, &p_gr);
    if (p_gr != NULL)
    {
        *gh = (p_gr->handle);
        memcpy(&p_gr->base_param, p, sizeof(ams_graph_basic_params_t));
        // init locks
        if (ams_osal_mutex_create(&p_gr->rwlock))
        {
            r = AMS_STATUS_GENERAL_ERROR;
            AMS_LIB_LOGE("Unable to init graph rwlock!");
            ams_util_graph_list_remove_graph(&p_ses->gr_lst, p_gr);
            goto exit;
        }
    }
    else
    {
        *gh = MAKE_EXT_HANDLE(NULL);
    }
exit:
    UNLOCK_READ_WRITE(reslock_gr_lst, p_ses->gr_lst.rwlock);
    return r;
}

ams_status_t ams_graph_add_endpont(
    ams_session_t amss,
    ams_graph_handle_t gh, // [in]
    ams_endpoint_t *e)     // [in]
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_session *p_ses = amss;
    struct ams_graph *p_gr = NULL;
    struct ams_endpoint_el *new_el = NULL;
    struct ams_endpoint_list *dest_e_list = NULL;
    int32_t reslock_gr_lst = 0;
    int32_t reslock_gr_el = 0;
    CHECK_AMS_HANDLE(p_ses);
    CHECK_GRP_HANDLE(gh);
    LOCK_FOR_READ(reslock_gr_lst, p_ses->gr_lst.rwlock);
    p_gr = ams_util_graph_list_find_graph(amss, gh);
    if (p_gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Graph not found by handle %lx!", (int64_t)gh);
        goto exit;
    }
    LOCK_FOR_WRITE(reslock_gr_el, p_gr->rwlock);
    r = ams_util_check_unique_id(p_gr, e->id);
    if (r != AMS_STATUS_SUCCESS)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Endpoint id=%d has not unique id!", e->id);
        goto exit;
    }
    r = ams_util_check_endpoint_param(e);
    if (r != AMS_STATUS_SUCCESS)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Endpoint id=%d has bad param when created!", e->id);
        goto exit;
    }
    r = ams_util_check_graph_tdm_intf(amss, p_gr, e);
    if (r != AMS_STATUS_SUCCESS)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Endpoint id=%d has conflict with TDM params already created!", e->id);
        goto exit;
    }
    new_el = calloc(1, sizeof(struct ams_endpoint_el));
    if (new_el == NULL)
    {
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }

    // sink or source
    if (e->type == AMS_ENDPOINT_TYPE_SINK)
    {
        // sink
        dest_e_list = &p_gr->sinks;
    }
    else
    {
        // source
        dest_e_list = &p_gr->sources;
    }
    ADD_NEW_LIST_EL(dest_e_list, new_el);
    // copy fields
    memcpy(&dest_e_list->last->val, e, sizeof(struct ams_endpoint));
exit:
    UNLOCK_READ_WRITE(reslock_gr_el, p_gr->rwlock);
    UNLOCK_READ_WRITE(reslock_gr_lst, p_ses->gr_lst.rwlock);
    return r;
}

ams_status_t ams_graph_add_module(
    ams_session_t amss,
    ams_graph_handle_t gh, // [in]
    ams_module_t *m)       // [in]
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_session *p_ses = amss;
    struct ams_graph *p_gr = NULL;
    struct ams_module_el *new_el = NULL;
    struct ams_module_list *dest_m_list = NULL;
    int32_t reslock_gr_lst = 0;
    int32_t reslock_gr_el = 0;
    CHECK_AMS_HANDLE(p_ses);
    CHECK_GRP_HANDLE(gh);
    LOCK_FOR_READ(reslock_gr_lst, p_ses->gr_lst.rwlock);
    p_gr = ams_util_graph_list_find_graph(amss, gh);
    if (p_gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Graph not found by handle!");
        goto exit;
    }
    LOCK_FOR_WRITE(reslock_gr_el, p_gr->rwlock);
    r = ams_util_check_module_param(m);
    if (r != AMS_STATUS_SUCCESS)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Module id=%d has bad param when created!", m->id);
        goto exit;
    }
    r = ams_util_check_unique_id(p_gr, m->id);
    if (r != AMS_STATUS_SUCCESS)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Module id=%d has not unique id!", m->id);
        goto exit;
    }
    new_el = calloc(1, sizeof(struct ams_module_el));
    if (new_el == NULL)
    {
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }
    dest_m_list = &p_gr->modules;
    ADD_NEW_LIST_EL(dest_m_list, new_el);
    // copy fields
    memcpy(&dest_m_list->last->val, m, sizeof(struct ams_module));
exit:
    UNLOCK_READ_WRITE(reslock_gr_el, p_gr->rwlock);
    UNLOCK_READ_WRITE(reslock_gr_lst, p_ses->gr_lst.rwlock);
    return r;
}

ams_status_t ams_graph_add_connection(
    ams_session_t amss,
    ams_graph_handle_t gh, // [in]
    ams_connection_t *c)   // [in]
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_session *p_ses = amss;
    struct ams_graph *p_gr = NULL;
    struct ams_connection_el *new_el = NULL;
    struct ams_connection_list *dest_c_list = NULL;
    int32_t reslock_gr_lst = 0;
    int32_t reslock_gr_el = 0;
    CHECK_AMS_HANDLE(p_ses);
    CHECK_GRP_HANDLE(gh);
    LOCK_FOR_READ(reslock_gr_lst, p_ses->gr_lst.rwlock);
    p_gr = ams_util_graph_list_find_graph(amss, gh);
    if (p_gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Graph not found by handle!");
        goto exit;
    }
    LOCK_FOR_WRITE(reslock_gr_el, p_gr->rwlock);
    new_el = calloc(1, sizeof(struct ams_connection_el));
    if (new_el == NULL)
    {
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }
    dest_c_list = &p_gr->connections;
    ADD_NEW_LIST_EL(dest_c_list, new_el);
    // copy fields
    memcpy((void *)&dest_c_list->last->val, c, sizeof(struct ams_connection));

exit:
    UNLOCK_READ_WRITE(reslock_gr_el, p_gr->rwlock);
    UNLOCK_READ_WRITE(reslock_gr_lst, p_ses->gr_lst.rwlock);
    return r;
}

ams_status_t ams_open_graph(
    ams_session_t amss,    // [in]
    ams_graph_handle_t gh) // [out]
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_session *p_ses = amss;
    struct ams_graph *p_gr = NULL;
    int32_t reslock_gr_lst = 0;
    int32_t reslock_gr_el = 0;
    CHECK_AMS_HANDLE(p_ses);
    CHECK_GRP_HANDLE(gh);
    LOCK_FOR_READ(reslock_gr_lst, p_ses->gr_lst.rwlock);
    p_gr = ams_util_graph_list_find_graph(amss, gh);
    if (p_gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Graph not found by handle %lx!", (int64_t)gh);
        goto exit;
    }
    LOCK_FOR_WRITE(reslock_gr_el, p_gr->rwlock);
    r = ams_util_check_graph(p_gr);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Check graph failed %d", r);
        goto exit;
    }
    r = ams_platform_open_graph(&p_ses->drv, p_gr);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Open graph failed %d", r);
        goto exit;
    }

exit:
    UNLOCK_READ_WRITE(reslock_gr_el, p_gr->rwlock);
    UNLOCK_READ_WRITE(reslock_gr_lst, p_ses->gr_lst.rwlock);
    return r;
}

ams_status_t ams_start_graph(
    ams_session_t amss,    // [in]
    ams_graph_handle_t gh) // [in]
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_session *p_ses = amss;
    struct ams_graph *p_gr = NULL;
    int32_t reslock_gr_lst = 0;
    int32_t reslock_gr_el = 0;
    CHECK_AMS_HANDLE(p_ses);
    CHECK_GRP_HANDLE(gh);
    LOCK_FOR_READ(reslock_gr_lst, p_ses->gr_lst.rwlock);
    p_gr = ams_util_graph_list_find_graph(amss, gh);
    if (p_gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Graph not found by handle %lx!", (int64_t)gh);
        goto exit;
    }
    LOCK_FOR_WRITE(reslock_gr_el, p_gr->rwlock);
    r = ams_platform_apply_prop(&p_ses->drv, p_gr, 0);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Apply graph property before start failed %d", r);
    }
    r = ams_platform_start_graph(&p_ses->drv, p_gr);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Start graph failed %d", r);
        goto exit;
    }
    r = ams_platform_apply_prop(&p_ses->drv, p_gr, 1);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Apply graph property after start failed %d", r);
    }
exit:
    UNLOCK_READ_WRITE(reslock_gr_el, p_gr->rwlock);
    UNLOCK_READ_WRITE(reslock_gr_lst, p_ses->gr_lst.rwlock);
    return r;
}

ams_status_t ams_stop_graph(
    ams_session_t amss,    // [in]
    ams_graph_handle_t gh) // [in]
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_session *p_ses = amss;
    struct ams_graph *p_gr = NULL;
    int32_t reslock_gr_lst = 0;
    int32_t reslock_gr_el = 0;
    CHECK_AMS_HANDLE(p_ses);
    CHECK_GRP_HANDLE(gh);
    LOCK_FOR_READ(reslock_gr_lst, p_ses->gr_lst.rwlock);
    p_gr = ams_util_graph_list_find_graph(amss, gh);
    if (p_gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Graph not found by handle %lx!", (int64_t)gh);
        goto exit;
    }
    LOCK_FOR_WRITE(reslock_gr_el, p_gr->rwlock);
    r = ams_platform_apply_prop(&p_ses->drv, p_gr, 2);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Apply graph property before stop failed %d", r);
    }
    r = ams_platform_stop_graph(&p_ses->drv, p_gr);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Stop graph failed %d", r);
        goto exit;
    }
    r = ams_platform_apply_prop(&p_ses->drv, p_gr, 3);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Apply graph property after stop failed %d", r);
    }

exit:
    UNLOCK_READ_WRITE(reslock_gr_el, p_gr->rwlock);
    UNLOCK_READ_WRITE(reslock_gr_lst, p_ses->gr_lst.rwlock);
    return r;
}

ams_status_t ams_close_graph(
    ams_session_t amss,
    ams_graph_handle_t gh) // [in]
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_session *p_ses = amss;
    struct ams_graph *p_gr = NULL;
    int32_t reslock_gr_lst = 0;
    int32_t reslock_gr_el = 0;
    CHECK_AMS_HANDLE(p_ses);
    CHECK_GRP_HANDLE(gh);
    LOCK_FOR_READ(reslock_gr_lst, p_ses->gr_lst.rwlock);
    p_gr = ams_util_graph_list_find_graph(amss, gh);
    if (p_gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Graph not found by handle %lx!", (int64_t)gh);
        goto exit;
    }
    LOCK_FOR_WRITE(reslock_gr_el, p_gr->rwlock);
    r = ams_platform_close_graph(&p_ses->drv, p_gr);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Close graph failed %d", r);
        goto exit;
    }

exit:
    UNLOCK_READ_WRITE(reslock_gr_el, p_gr->rwlock);
    UNLOCK_READ_WRITE(reslock_gr_lst, p_ses->gr_lst.rwlock);
    return r;
}

ams_status_t ams_destroy_graph(
    ams_session_t amss,
    ams_graph_handle_t *gh) // [in]
{
    ams_status_t r = 0;

    struct ams_session *p_ses = amss;
    struct ams_graph *p_gr = NULL;
    int32_t reslock_gr_lst = 0;
    int32_t reslock_gr_el = 0;
    CHECK_AMS_HANDLE(p_ses);
    CHECK_GRP_HANDLE(gh);
    CHECK_GRP_HANDLE(*gh);
    LOCK_FOR_WRITE(reslock_gr_lst, p_ses->gr_lst.rwlock);
    struct ams_graph_list *p_gr_lst = &p_ses->gr_lst;
    p_gr = ams_util_graph_list_find_graph(amss, (*gh));
    if (p_gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Graph not found by handle %lx!", (int64_t)(*gh));
        goto exit;
    }
    LOCK_FOR_WRITE(reslock_gr_el, p_gr->rwlock);
    // stop and close if opened/running
    if (p_gr->fw_status == DSP_AMS_GRAPH_STARTED)
    {
        r = ams_platform_stop_graph(&p_ses->drv, p_gr);
    }
    if (p_gr->fw_status == DSP_AMS_GRAPH_OPENED)
    {
        r = ams_platform_close_graph(&p_ses->drv, p_gr);
    }
    r = ams_util_graph_list_remove_graph_elements(p_gr_lst, p_gr);
    UNLOCK_READ_WRITE(reslock_gr_el, p_gr->rwlock);
    // destroy lock
    if (ams_osal_mutex_destroy(p_gr->rwlock))
    {
        AMS_LIB_LOGE("Unable to destroy rwlock!");
    }
    struct ams_graph *grtmp = NULL;
    REMOVE_LIST2_EL(p_gr_lst, p_gr, grtmp);
exit:
    UNLOCK_READ_WRITE(reslock_gr_lst, p_ses->gr_lst.rwlock);
    if (r == AMS_STATUS_SUCCESS)
        *gh = NULL;
    return r;
}

ams_status_t ams_set_param(
    ams_session_t amss,
    ams_graph_handle_t gh, // [in]
    uint32_t module_id,    // [in]
    uint32_t param_id,     // [in]
    uint32_t param_size,   // [in]
    void *data)            // [in]
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_session *p_ses = amss;
    struct ams_graph *p_gr = NULL;
    int32_t reslock_gr_lst = 0;
    int32_t reslock_gr_el = 0;
    CHECK_AMS_HANDLE(p_ses);
    CHECK_GRP_HANDLE(gh);
    LOCK_FOR_READ(reslock_gr_lst, p_ses->gr_lst.rwlock)
    if (data == NULL || param_size == 0 || module_id == 0 || param_id == 0)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Bad input provided!");
        goto exit;
    }
    p_gr = ams_util_graph_list_find_graph(amss, gh);
    if (p_gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Graph not found by handle %lx!", (int64_t)gh);
        goto exit;
    }
    LOCK_FOR_READ(reslock_gr_el, p_gr->rwlock);
    r = ams_platform_set_param(&p_ses->drv, p_gr, module_id, param_id, param_size, data);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Set param %d failed %d", param_id, r);
        goto exit;
    }

exit:
    UNLOCK_READ_WRITE(reslock_gr_el, p_gr->rwlock);
    UNLOCK_READ_WRITE(reslock_gr_lst, p_ses->gr_lst.rwlock);
    return r;
}

ams_status_t ams_get_param(
    ams_session_t amss,
    ams_graph_handle_t gh, // [in]
    uint32_t module_id,    // [in]
    uint32_t param_id,     // [in]
    uint32_t *param_size,  // [in/out]
    void *data)            // [in/out]
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_session *p_ses = amss;
    struct ams_graph *p_gr = NULL;
    int32_t reslock_gr_lst = 0;
    int32_t reslock_gr_el = 0;
    CHECK_AMS_HANDLE(p_ses);
    CHECK_GRP_HANDLE(gh);
    LOCK_FOR_READ(reslock_gr_lst, p_ses->gr_lst.rwlock);
    if (data == NULL || param_size == NULL || *param_size == 0 || module_id == 0 || param_id == 0)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Bad input provided!");
        goto exit;
    }
    p_gr = ams_util_graph_list_find_graph(amss, gh);
    if (p_gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Graph not found by handle %lx!", (int64_t)gh);
        goto exit;
    }
    LOCK_FOR_READ(reslock_gr_el, p_gr->rwlock);
    r = ams_platform_get_param(&p_ses->drv, p_gr, module_id, param_id, param_size, data);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Get param %d failed %d", param_id, r);
        goto exit;
    }

exit:
    UNLOCK_READ_WRITE(reslock_gr_el, p_gr->rwlock);
    UNLOCK_READ_WRITE(reslock_gr_lst, p_ses->gr_lst.rwlock);
    return r;
}

ams_status_t ams_register_callback(
    ams_session_t amss,
    ams_event_handler_t func,
    void *data)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_session *p_ses = amss;
    int32_t reslock_cb_lst = 0;
    CHECK_AMS_HANDLE(p_ses);
    if (func == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Bad callback handler!");
        goto exit;
    }
    LOCK_FOR_WRITE(reslock_cb_lst, p_ses->cb_lst.rwlock);
    struct ams_cb_list *cb_list = &p_ses->cb_lst;
    struct ams_cb *new_cb = NULL;
    new_cb = calloc(1, sizeof(struct ams_cb));
    if (new_cb == NULL)
    {
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }

    memcpy(&new_cb->func, &func, sizeof(func));
    new_cb->data = data;
    ADD_NEW_LIST2_EL(cb_list, new_cb);

exit:
    UNLOCK_READ_WRITE(reslock_cb_lst, p_ses->cb_lst.rwlock);
    return r;
}

ams_status_t ams_deregister_callback(
    ams_session_t amss,
    ams_event_handler_t h)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_session *p_ses = amss;
    int32_t reslock_cb_lst = 0;
    uint32_t found = 0;
    CHECK_AMS_HANDLE(p_ses);
    if (h == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Bad callback handler!");
        goto exit;
    }
    LOCK_FOR_WRITE(reslock_cb_lst, p_ses->cb_lst.rwlock);
    struct ams_cb_list *cb_list = &p_ses->cb_lst;
    struct ams_cb *cb = cb_list->first;
    while (cb != NULL)
    {
        struct ams_cb *cb_next = cb->nxt_el;
        if (cb->func == h)
        {
            found = 1;
            struct ams_cb *cbtmp;
            REMOVE_LIST2_EL(cb_list, cb, cbtmp);
            break;
        }
        cb = cb_next;
    }
    if (!found)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Callback not found!");
    }
exit:
    UNLOCK_READ_WRITE(reslock_cb_lst, p_ses->cb_lst.rwlock);
    return r;
}

ams_status_t ams_graph_set_property(
    ams_session_t amss,
    ams_graph_handle_t gh,
    ams_graph_property_t *prop)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_session *p_ses = amss;
    struct ams_graph *p_gr = NULL;
    struct ams_graph_property_el *new_el = NULL;
    struct ams_graph_property_list *prop_list = NULL;
    int32_t reslock_gr_lst = 0;
    int32_t reslock_gr_el = 0;
    CHECK_AMS_HANDLE(p_ses);
    CHECK_GRP_HANDLE(gh);
    if (prop == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Property NULL!");
        goto exit;
    }
    LOCK_FOR_READ(reslock_gr_lst, p_ses->gr_lst.rwlock);
    p_gr = ams_util_graph_list_find_graph(amss, gh);
    if (p_gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Graph not found by handle!");
        goto exit;
    }
    r = ams_util_check_graph_property(p_gr, prop);
    if (r)
    {
        AMS_LIB_LOGE("Property cannot be set!");
        goto exit;
    }
    LOCK_FOR_WRITE(reslock_gr_el, p_gr->rwlock);
    new_el = calloc(1, sizeof(struct ams_graph_property_el));
    if (new_el == NULL)
    {
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }
    prop_list = &p_gr->props;
    ADD_NEW_LIST_EL(prop_list, new_el);
    // copy fields
    memcpy((void *)&prop_list->last->val, prop, sizeof(ams_graph_property_t));

exit:
    UNLOCK_READ_WRITE(reslock_gr_el, p_gr->rwlock);
    UNLOCK_READ_WRITE(reslock_gr_lst, p_ses->gr_lst.rwlock);
    return r;
}

// new api
ams_status_t ams_restart(
    ams_session_t *amss,
    ams_session_type_t *session_type)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    uint32_t num_graphs = 0;
    uint32_t i = 0;
    struct ams_session *pses = NULL;
    int32_t reslock_gr_lst = 0;

    r = ams_init(&pses);

    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Cannot init handle!");
        goto exit;
    }

    r = ams_platform_get_num_graphs_cached(&pses->drv, &num_graphs);
    if (num_graphs == 0)
    {
        // new session
        *session_type = AMS_SESSION_NEW;
        goto exit;
    }
    else
    {
        // restore graphs
        struct ams_graph *pgr = NULL;
        LOCK_FOR_WRITE(reslock_gr_lst, pses->gr_lst.rwlock);
        for (i = 0; i < num_graphs; i++)
        {
            r = ams_platform_restore_graph_cached_info_by_idx(pses, i, &pgr);
            AMS_LIB_LOGD("restore_graph_cached_info_by_idx ret  %d", r);
        }
        AMS_LIB_LOGD("Num graphs cached %d", num_graphs);
        *session_type = AMS_SESSION_RESTARTED;
    }
exit:
    UNLOCK_READ_WRITE(reslock_gr_lst, pses->gr_lst.rwlock);
    *amss = pses;
    return r;
}

ams_status_t ams_query_graph_list(
    ams_session_t amss,
    ams_graph_handle_t *gh,
    uint32_t max_num_gh,
    uint32_t *num_gh)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_session *pses = amss;
    ams_graph_handle_t *plist = gh;
    struct ams_graph *pgr = NULL;
    int32_t reslock_gr_lst = 0;
    int32_t i = 0;
    if ( max_num_gh > 1){
        AMS_LIB_LOGE("Only 1 graph is supported!");
        return AMS_STATUS_INPUT_ERROR;
    }
    CHECK_AMS_HANDLE(pses);
    LOCK_FOR_READ(reslock_gr_lst, pses->gr_lst.rwlock);
    if (plist == NULL)
    {
        AMS_LIB_LOGE("Cannot allocate space of graph handle list!");
        goto exit;
    }
    *num_gh = pses->gr_lst.el_num < max_num_gh? pses->gr_lst.el_num: max_num_gh;
    pgr = pses->gr_lst.first;
    while (pgr != NULL && i < *num_gh)
    {
        plist[i++] = pgr->handle;
        pgr = pgr->nxt_el;
    }

exit:
    UNLOCK_READ_WRITE(reslock_gr_lst, pses->gr_lst.rwlock);
    return r;
}

ams_status_t ams_query_graph_info(
    ams_session_t amss,
    ams_graph_handle_t gh,
    ams_graph_info_t *graph_info)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_session *pses = amss;
    struct ams_graph *pgr = NULL;
    ams_graph_info_t *pinfo = graph_info;
    struct ams_module_el *mel = NULL;
    struct ams_endpoint_el *eel = NULL;
    int32_t reslock_gr_lst = 0;
    int32_t reslock_gr_el = 0;
    int32_t i, j;
    CHECK_AMS_HANDLE(pses);
    CHECK_GRP_HANDLE(gh);
    LOCK_FOR_READ(reslock_gr_lst, pses->gr_lst.rwlock);
    pgr = ams_util_graph_list_find_graph(amss, gh);
    if (pgr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Graph not found by handle!");
        goto exit;
    }
    LOCK_FOR_READ(reslock_gr_el, pgr->rwlock);

    if (pinfo == NULL)
    {
        AMS_LIB_LOGE("Cannot allocate memory for graph info!");
        goto exit;
    }
    pinfo->state = pgr->fw_status == DSP_AMS_GRAPH_STARTED? AMS_GRAPH_STARTED: AMS_GRAPH_OPENED;
    pinfo->modules_num = pgr->modules.el_num;
    pinfo->ep_num = pgr->sinks.el_num + pgr->sources.el_num;
    mel = pgr->modules.first;
    for (i = 0; i < pgr->modules.el_num; i++)
    {
        pinfo->modules_ids[i] = mel->val.id;
        mel = mel->nxt_el;
    }
    eel = pgr->sinks.first;
    for (i = 0; i < pgr->sinks.el_num; i++)
    {
        pinfo->ep_ids[i] = eel->val.id;
        eel = eel->nxt_el;
    }
    eel = pgr->sources.first;
    j = i;
    for (i = 0; i < pgr->sources.el_num; i++)
    {
        pinfo->ep_ids[j++] = eel->val.id;
        eel = eel->nxt_el;
    }
exit:
    UNLOCK_READ_WRITE(reslock_gr_el, pgr->rwlock);
    UNLOCK_READ_WRITE(reslock_gr_lst, pses->gr_lst.rwlock);
    return r;
}

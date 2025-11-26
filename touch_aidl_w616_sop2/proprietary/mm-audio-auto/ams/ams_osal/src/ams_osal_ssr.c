/**
 *
 * \file ams_osal_ssr.c
 *
 * \brief
 *      This file has implementation of shared memory allocation for DSP.
 * \copyright
 *  Copyright (c) 2019-2021, 2023-2024 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#define LOG_TAG "ams_osal_ssr"
#include <stddef.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <log/log.h>
#include "ams_osal_servreg.h"
#include "ams_osal_mutex.h"
#include "ams_osal_ssr.h"
#include "ams_osal_error.h"

#define AMS_GET_SPF_SS_MASK(OSAL_SYS_ID) (1 << (OSAL_SYS_ID - 1))
#define AMS_AUDIO_PD_DOMAIN_STR "msm/adsp/audio_pd"
#define AMS_MODEM_PD_DOMAIN_STR "msm/mdsp/audio_pd"
#define AMS_MODEM_PD_DOMAIN_ROOT_STR "msm/modem/root_pd"
#define AMS_AUDIO_PD_SERVICE_STR "avs/audio"


enum ams_ss_state_t
{
    AMS_SPF_SS_STATE_DN = 0,
    AMS_SPF_SS_STATE_UP = 1
};

/*
 * Use int return value as ams_osal_ssr_init considers
 * int type for event_handler
*/
typedef int (*ams_spf_ss_cb_t)(uint32_t ss_id_mask,
                                uint32_t event_id,
                                void *ctx);

struct ams_spf_ss_state
{
    /*
     * bit mask that represents which subsystems are supported
     */
    uint32_t ss_supported_flags;
    /*
     * bit mask representing the state of each subsystem
     */
    uint32_t ss_state_flags;
    /*
     * Used to sychronize accesses to the global subsystem state
     */
    ams_osal_mutex_t lock;
    /*
     * callback invoked whenever there is an SSR up or down event
     */
    ams_spf_ss_cb_t ssr_cb;
    void *ctx;
    ams_osal_servreg_t servreg_handle;
};

#define AMS_MODEM_DSP 1
#define AMS_AUDIO_DSP 2

static struct ams_spf_ss_state ams_cluster_ss_state[2] = {0};

static uint32_t spf_cluster_ref_count;

static uint32_t ams_spf_ss_state_set(uint32_t proc_id, uint32_t ss_mask,
                                     enum ams_ss_state_t state)
{
    uint32_t updated_ss_state = 0;

    ams_osal_mutex_lock(ams_cluster_ss_state[proc_id].lock);
    if ((ss_mask & ams_cluster_ss_state[proc_id].ss_supported_flags) &&
        (state == AMS_SPF_SS_STATE_UP))
        ams_cluster_ss_state[proc_id].ss_state_flags |= ss_mask;
    else if ((ss_mask &
              ams_cluster_ss_state[proc_id].ss_supported_flags) &&
             (state == AMS_SPF_SS_STATE_DN))
    {
        ams_cluster_ss_state[proc_id].ss_state_flags &= ~ss_mask;
    }
    updated_ss_state = ams_cluster_ss_state[proc_id].ss_state_flags;
    ams_osal_mutex_unlock(ams_cluster_ss_state[proc_id].lock);

    return updated_ss_state;
}

static void servreg_callback(ams_osal_servreg_t servreg_handle,
                             ams_osal_servreg_cb_event_type event_id, void *cb_context, void *payload,
                             uint32_t payload_size)
{
    uint32_t ss_mask = 0;
    uint32_t proc_id = 0;

    /*
     * reference unused variables to make compiler happy. Can't change function
     * signature because it needs to match what OSAL expects.
     */
    (void)servreg_handle;
    (void)cb_context;
    (void)payload_size;

    ams_osal_servreg_state_notify_payload_type *serv_ntfy_pld =
        (ams_osal_servreg_state_notify_payload_type *)payload;

    if (event_id == AMS_OSAL_SERVICE_STATE_NOTIFY)
    {
        if (!strcmp(serv_ntfy_pld->domain.name, AMS_AUDIO_PD_DOMAIN_STR))
        {
            ss_mask = SS_ID_LPASS;
            proc_id = AMS_AUDIO_DSP-1;
        } else if (!strcmp(serv_ntfy_pld->domain.name,
            AMS_MODEM_PD_DOMAIN_STR) || !strcmp(serv_ntfy_pld->domain.name,
            AMS_MODEM_PD_DOMAIN_ROOT_STR)) {
            ss_mask = SS_ID_MODEM;
            proc_id = AMS_MODEM_DSP-1;
        } else {
            ALOGE("domain %s not supported",
            serv_ntfy_pld->domain.name);
            return;
        }

    if (serv_ntfy_pld->service_state == AMS_OSAL_SERVICE_STATE_DOWN) {
            /* update the global Spf SS state */
            ams_spf_ss_state_set(proc_id, ss_mask, AMS_SPF_SS_STATE_DN);
            ams_cluster_ss_state[proc_id].ssr_cb(ss_mask, SSR_EVENT_RESTART_START,
                           ams_cluster_ss_state[proc_id].ctx);
    } else if (serv_ntfy_pld->service_state ==
            AMS_OSAL_SERVICE_STATE_UP) {
            /* update the global Spf SS state */
            ams_spf_ss_state_set(proc_id, ss_mask, AMS_SPF_SS_STATE_UP);
            // assume AMS_SPF_SS_STATE_DN=SSR_EVENT_RESTART_COMPLETE
            ams_cluster_ss_state[proc_id].ssr_cb(ss_mask, SSR_EVENT_RESTART_COMPLETE,
                                                 ams_cluster_ss_state[proc_id].ctx);
        }
    }
}

static uint32_t ams_servreg_setup(char *client_name)
{
    int32_t rc = AMS_EOK;
    ams_osal_servreg_entry_type service = {AMS_AUDIO_PD_SERVICE_STR, 0};
    ams_osal_servreg_entry_type *domain_list = NULL;
    uint32_t i = 0, domain_list_size = 0;
    rc = ams_osal_servreg_init();
    if (rc)
    {
        ALOGE("failed to initialize ar_osal state notifier %d", rc);
        goto exit;
    }

    /* first we query for number of entries in domain list */
    rc = ams_osal_servreg_get_domainlist(&service, NULL, &domain_list_size);
    if (rc != AMS_ENOMEMORY)
    {
        ALOGE("failed to get num domains with ar_osal state notifier %d",
              rc);
        goto deinit_state_notify;
    }
    ALOGE("servreg setup: domain_list_size %d", domain_list_size);
    /* next we query for actual list */
    domain_list = calloc(1, domain_list_size *
                                sizeof(ams_osal_servreg_entry_type));
    if (!domain_list)
    {
        rc = AMS_ENOMEMORY;
        goto deinit_state_notify;
    }
    rc = ams_osal_servreg_get_domainlist(&service, domain_list,
                                         &domain_list_size);
    if (rc)
    {
        ALOGE("failed to get domains with ar_osal state notifier %d", rc);
        goto free_domain_list;
    }

    /* scan through domain list and register for notification */
    for (i = 0; i < domain_list_size; ++i)
    {
        ams_cluster_ss_state[i].servreg_handle = ams_osal_servreg_register(client_name,
                                                                           servreg_callback, &ams_cluster_ss_state[i],
                                                                           domain_list + i, &service);
        ALOGE("servreg setup: domain name %s", domain_list[i].name);
        if (!ams_cluster_ss_state[i].servreg_handle)
        {
            ALOGE("failed to register for service %c", rc);
            rc = AMS_EFAILED;
            goto free_domain_list;
        }
    }

    free(domain_list);
    return rc;

free_domain_list:
    free(domain_list);
deinit_state_notify:
    ams_osal_servreg_deinit();
exit:
    return rc;
}


int32_t ams_osal_ssr_init(char *client_name, int (*event_handler)(uint32_t ss_id_mask, uint32_t event_id, void *ctx),
                          void *ctx)
{
    int32_t r;
    ams_cluster_ss_state[AMS_MODEM_DSP - 1].ss_supported_flags = SS_ID_MODEM;
    ams_cluster_ss_state[AMS_MODEM_DSP - 1].ssr_cb = event_handler;
    ams_cluster_ss_state[AMS_MODEM_DSP - 1].ctx = ctx;
    ams_osal_mutex_create(&ams_cluster_ss_state[AMS_MODEM_DSP - 1].lock);
    ams_cluster_ss_state[AMS_AUDIO_DSP - 1].ss_supported_flags = SS_ID_LPASS;
    ams_cluster_ss_state[AMS_AUDIO_DSP - 1].ssr_cb = event_handler;
    ams_cluster_ss_state[AMS_AUDIO_DSP - 1].ctx = ctx;
    ams_osal_mutex_create(&ams_cluster_ss_state[AMS_AUDIO_DSP - 1].lock);
    r = ams_servreg_setup(client_name);
    return r;
}

int32_t ams_osal_ssr_deinit(void)
{
    int32_t r;

    r = ams_osal_servreg_deregister(ams_cluster_ss_state[AMS_MODEM_DSP - 1].servreg_handle);
    r |= ams_osal_servreg_deregister(ams_cluster_ss_state[AMS_AUDIO_DSP - 1].servreg_handle);
    if (r)
    {
        ALOGE("Failed to deregister servreg handles!");
    }
    r = ams_osal_servreg_deinit();
    if (r)
    {
        ALOGE("Failed to deinit servreg!");
    }
    ams_osal_mutex_destroy(ams_cluster_ss_state[AMS_MODEM_DSP - 1].lock);
    ams_osal_mutex_destroy(ams_cluster_ss_state[AMS_AUDIO_DSP - 1].lock);
    memset(ams_cluster_ss_state, 0, sizeof(ams_cluster_ss_state));
    return r;
}
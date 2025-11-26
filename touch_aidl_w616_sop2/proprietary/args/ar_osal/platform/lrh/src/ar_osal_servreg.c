/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


#include "ar_osal_types.h"
#include "ar_osal_servreg.h"
#include "ar_util_list.h"
#include "ar_osal_mutex.h"
#include "ar_osal_error.h"
#include "ar_osal_log.h"
#include "ar_osal_mem_op.h"
#include "ar_osal_sys_id.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "ssr_api.h"
#include "ssr_events.h"

#define AR_OSAL_SERVREG_TAG    "AOSR"

char_t service_name[AR_SUB_SYS_ID_LAST + 1][16] = { "avs/audio","avs/audio","avs/audio","avs/audio","avs/audio","avs/audio","avs/audio","avs/audio","avs/audio","avs/audio","avs/audio","avs/audio"};
char_t domain_name[AR_SUB_SYS_ID_LAST + 1][24] = { "msm/adsp/audio_pd", "msm/mdsp/audio_pd", "msm/adsp/audio_pd","msm/apss/audio_pd","msm/sdsp/audio_pd","msm/cdsp/audio_pd","msm/ccdsp/audio_pd","msm/adsp/audio_pd","msm/adsp/audio_pd", "msm/adsp/audio_pd", "msm/gpdsp0/audio_pd", "msm/gpdsp1/audio_pd"};

int32_t g_init_done = 0;

typedef struct
{
    ar_list_t list_handle;
    uint32_t    num_nodes;
    ar_osal_mutex_t lock;
} servreg_handle_pool_type;

typedef struct
{
    ar_list_node_t             srv_node;
    ar_osal_servreg_t          srv_handle;
    void                      *srv_cb_handle;
    ar_osal_servreg_entry_type service;
    ar_osal_servreg_entry_type domain;
    void                      *cb_context;
    ar_osal_servreg_callback   cb_func;
    ar_osal_client_type        client_type;
    ar_osal_service_state_type srv_state;
    ar_list_t                 *list_handle;
} service_node;

#define get_struc_base(p, type, member) ((type *)( \
    (int8_t *)(p)- \
    (uintptr_t)(&((type *)0)->member)))

servreg_handle_pool_type servreg_handle_pool = { 0 };

PAGED_FUNCTIONS_START

static ar_osal_service_state_type ssr_event_to_ar_osal_pd_state(uint32_t event_id)
{
    ar_osal_service_state_type ret = AR_OSAL_SERVICE_STATE_DOWN;

    AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "arosal state switch, ssr event_id=%d\n", event_id);
    switch (event_id)
    {
        case SSR_EVENT_RESTART_START:
            ret = AR_OSAL_SERVICE_STATE_DOWN;
            break;
        case SSR_EVENT_RESTART_COMPLETE:
            ret = AR_OSAL_SERVICE_STATE_UP;
            break;
        default:
            ret = AR_OSAL_SERVICE_STATE_DOWN;
    }
    return ret;
}

int32_t arosal_ssr_event_cb(uint32_t ss_id_mask,
                         uint32_t event_id,
                         void *ctx)
{
    int32_t rc = AR_EOK;
    ar_osal_servreg_state_notify_payload_type notify_state;
    ar_list_node_t *list_node = NULL;
    service_node *node = NULL;
    char *domain_name;
    char *service_name;

    ar_osal_service_state_type state = ssr_event_to_ar_osal_pd_state(event_id);
    notify_state.service_state = state;
    static bool_t ss_id_lpass_status_is_ssr = FALSE;
    static bool_t ss_id_gpdsp0_status_is_ssr = FALSE;
    static bool_t ss_id_gpdsp1_status_is_ssr = FALSE;

    AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "servreg_handle_pool.num_nodes = %d\n", servreg_handle_pool.num_nodes);
    ar_osal_mutex_lock(servreg_handle_pool.lock);
    switch(event_id)
    {
        case SSR_EVENT_RESTART_START:
            if(ss_id_mask & SS_ID_LPASS)
            {
                ss_id_lpass_status_is_ssr = TRUE;
                AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "ADSP SSR start\n");
            }
            if(ss_id_mask & SS_ID_GPDSP0)
            {
                ss_id_gpdsp0_status_is_ssr = TRUE;
                AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "GPDSP0 SSR start\n");
            }
            if(ss_id_mask & SS_ID_GPDSP1)
            {
                ss_id_gpdsp1_status_is_ssr = TRUE;
                AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "GPDSP1 SSR start\n");
            }
            if(!ss_id_lpass_status_is_ssr || !ss_id_gpdsp0_status_is_ssr || !ss_id_gpdsp1_status_is_ssr)
            {
                AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "Skipping, as ssr restart not received for lpass/gpdsp0/gpdsp1\n");
                goto exit;
            }
            break;
        case SSR_EVENT_RESTART_COMPLETE:
            if(ss_id_mask & SS_ID_LPASS)
            {
                ss_id_lpass_status_is_ssr = FALSE;
                AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "ADSP SSR restart complete\n");
            }
            if(ss_id_mask & SS_ID_GPDSP0)
            {
                ss_id_gpdsp0_status_is_ssr = FALSE;
                AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "GPDSP0 SSR restart complete\n");
            }
            if(ss_id_mask & SS_ID_GPDSP1)
            {
                ss_id_gpdsp1_status_is_ssr = FALSE;
                AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "GPDSP1 SSR restart complete\n");
            }
            if(ss_id_lpass_status_is_ssr || ss_id_gpdsp0_status_is_ssr || ss_id_gpdsp1_status_is_ssr)
            {
                AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "Skipping, as ssr restart complete not received for lpass/gpdsp0/gpdsp1\n");
                goto exit;
            }
            break;
        default :
            AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "Invalid event notification : %d\n", event_id);
            goto exit;
    }

    for (uint32_t i = servreg_handle_pool.num_nodes; i > 0; i--)
    {
        ar_list_for_each_entry(list_node, &servreg_handle_pool.list_handle)
        {
            node = get_struc_base(list_node, service_node, srv_node);
            if (node)
            {
                memcpy((void*)&notify_state.service, (const void*)&node->service, sizeof(ar_osal_servreg_entry_type));
                memcpy((void*)&notify_state.domain, (const void*)&node->domain, sizeof(ar_osal_servreg_entry_type));

                if (node->cb_func) {
                    AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "node->cb_func=%pK\n", node->cb_func);
                    node->cb_func((ar_osal_servreg_t)node,
                        AR_OSAL_SERVICE_STATE_NOTIFY,
                        node->cb_context,
                        (void*)&notify_state,
                        sizeof(notify_state));
                }
                node->srv_state = state;
            }
        }
    }
exit:
    ar_osal_mutex_unlock(servreg_handle_pool.lock);
    return rc;
}

/**
* \brief ar_servreg_validate_service_name
*       internal function to validate supported service name.
* \return
*  0 -- Success
*  Nonzero -- Failure
*/
_IRQL_requires_max_(PASSIVE_LEVEL)
int32_t ar_servreg_validate_service_name(_In_ uint8_t *serv_name)
{
    PAGED_FUNCTION();
    int32_t status = AR_ENOTEXIST;
    if (NULL == service_name)
    {
        status = AR_EBADPARAM;
        goto end;
    }

    for (uint8_t i = AR_SUB_SYS_ID_FIRST; i <= AR_SUB_SYS_ID_LAST; i++)
    {
        if (0 == strcmp(service_name[i], (char *)serv_name))
        {
            status = AR_EOK;
            goto end;
        }
    }

end:
    return status;
}
/**
* \brief ar_osal_servreg_init
*        Initialize servreg interface.
*........Note:This API has to be called before any other API in this interface.
*........Should be called at least once and is expected to be serialized if called
*        multiple times.
* \return
*  0 -- Success
*  Nonzero -- Failure
*/
_IRQL_requires_max_(PASSIVE_LEVEL)
int32_t ar_osal_servreg_init(void)
{
    PAGED_FUNCTION();
    int32_t status = AR_EOK;

    if (g_init_done)
    {
        g_init_done++;
        return status;
    }
    g_init_done = 1;
    memset(&servreg_handle_pool, 0, sizeof(servreg_handle_pool));

    // initialize the list
    status = ar_list_init(&servreg_handle_pool.list_handle, NULL, NULL);
    if (status != AR_EOK)
    {
        AR_LOG_ERR(AR_OSAL_SERVREG_TAG, "ar_list_init error: %d", status);
        goto end;
    }

    // Create a mutex
    status = ar_osal_mutex_create(&servreg_handle_pool.lock);
    if (status != AR_EOK)
    {
        AR_LOG_ERR(AR_OSAL_SERVREG_TAG, "ar_osal_mutex_create error: %d", status);
        goto end;
    }
    AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "ar_osal_servreg_init status(0x%x)", status);
end:
    return status;
}

/**
* \brief ar_osal_servreg_deinit
*        Uninitialize servreg interface.
*........Should be called in pair with ar_osal_servreg_init() and
*........should be a serialized call.
* \return
*  0 -- Success
*  Nonzero -- Failure
*/
_IRQL_requires_max_(PASSIVE_LEVEL)
int32_t ar_osal_servreg_deinit(void)
{
    PAGED_FUNCTION();
    uint32_t status = AR_EOK;
    service_node *node = NULL;
    ar_list_node_t *list_node = NULL;

    g_init_done--;
    if (0 != g_init_done)
    {
        goto end;
    }

    ar_osal_mutex_lock(servreg_handle_pool.lock);

    for (uint32_t i = servreg_handle_pool.num_nodes; i > 0; i--)
    {
        ar_list_for_each_entry(list_node, &servreg_handle_pool.list_handle)
        {
            node = get_struc_base(list_node, service_node, srv_node);
            if (node)
            {
                ar_list_delete(&servreg_handle_pool.list_handle, list_node);
                servreg_handle_pool.num_nodes--;
                AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "ar_list_delete freed node(0x%p)", node);
                free(node);
                break;
            }
        }
        list_node = NULL;
    }
    ar_osal_mutex_unlock(servreg_handle_pool.lock);

    ar_osal_mutex_destroy(servreg_handle_pool.lock);
    memset(&servreg_handle_pool, 0, sizeof(servreg_handle_pool));
    AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "ar_osal_servreg_deinit status(0x%x)", status);
end:
    return status;
}

/**
* \brief ar_osal_servreg_get_domainlist
*        Client to call this API to get a list of domains(msm/domain/subdomain) on which
*        a given service(provider/service) is supported.
*
* \param[in]  service: service(provider/service) for which domain(s) list is required.
* \param[in out] domain_list: service supported in domain(s), client to provide
*.                            payload buffer pointer.
* \param[in out] num_domains: Client to provide the num_domains to get the domain list.
*.                            Input NULL domain_list to get the number of domains 
*                             for the given service if available in num_domains.
*
* \return
*  0 -- Success
*  Nonzero -- Failure
*  AR_ENOMEMORY- Failed due to insufficient memory, client to call the API again
*                  with required size as returned in num_domains.
*.
*/
_IRQL_requires_max_(PASSIVE_LEVEL)
int32_t ar_osal_servreg_get_domainlist(_In_ ar_osal_servreg_entry_type *service,
    _Inout_opt_ ar_osal_servreg_entry_type *domain_list,
    _Inout_ uint32_t *num_domains)
{
    PAGED_FUNCTION();
    int32_t status = AR_EOK;
    if (NULL == service || NULL == num_domains)
    {
        status = AR_EBADPARAM;
        goto end;
    }
    if (NULL == domain_list)
    {
        //return supported domains
        *num_domains = AR_SUB_SYS_ID_LAST + 1;
        status = AR_ENOMEMORY;
        goto end;
    }

    if(AR_EOK == ar_servreg_validate_service_name((uint8_t *)&service->name))
    {
        //copy domain details
        ar_mem_cpy(domain_list[0].name, sizeof(domain_list[0].name), domain_name[AR_SUB_SYS_ID_INVALID], sizeof(domain_name[AR_SUB_SYS_ID_INVALID]));
        ar_mem_cpy(domain_list[1].name, sizeof(domain_list[1].name), domain_name[AR_MODEM_DSP], sizeof(domain_name[AR_MODEM_DSP]));
        ar_mem_cpy(domain_list[2].name, sizeof(domain_list[2].name), domain_name[AR_AUDIO_DSP], sizeof(domain_name[AR_AUDIO_DSP]));
        ar_mem_cpy(domain_list[3].name, sizeof(domain_list[3].name), domain_name[AR_APSS], sizeof(domain_name[AR_APSS]));
        ar_mem_cpy(domain_list[4].name, sizeof(domain_list[4].name), domain_name[AR_SENSOR_DSP], sizeof(domain_name[AR_SENSOR_DSP]));
        ar_mem_cpy(domain_list[5].name, sizeof(domain_list[5].name), domain_name[AR_COMPUTE_DSP], sizeof(domain_name[AR_COMPUTE_DSP]));
        ar_mem_cpy(domain_list[6].name, sizeof(domain_list[6].name), domain_name[AR_CC_DSP], sizeof(domain_name[AR_CC_DSP]));
        ar_mem_cpy(domain_list[7].name, sizeof(domain_list[7].name), domain_name[AR_SUB_SYS_ID_INVALID], sizeof(domain_name[AR_SUB_SYS_ID_INVALID]));
        ar_mem_cpy(domain_list[8].name, sizeof(domain_list[8].name), domain_name[AR_SUB_SYS_ID_INVALID], sizeof(domain_name[AR_SUB_SYS_ID_INVALID]));
        ar_mem_cpy(domain_list[9].name, sizeof(domain_list[9].name), domain_name[AR_SUB_SYS_ID_INVALID], sizeof(domain_name[AR_SUB_SYS_ID_INVALID]));
        ar_mem_cpy(domain_list[10].name, sizeof(domain_list[10].name), domain_name[AR_GP_DSP0], sizeof(domain_name[AR_GP_DSP0]));
        ar_mem_cpy(domain_list[11].name, sizeof(domain_list[11].name), domain_name[AR_GP_DSP1], sizeof(domain_name[AR_GP_DSP1]));

        domain_list[0].instance_id = 74;
        domain_list[1].instance_id = 74;
        domain_list[2].instance_id = 74;
        domain_list[3].instance_id = 74;
        domain_list[4].instance_id = 74;
        domain_list[5].instance_id = 74;
        domain_list[6].instance_id = 74;
        domain_list[7].instance_id = 74;
        domain_list[8].instance_id = 74;
        domain_list[9].instance_id = 74;
        domain_list[10].instance_id = 74;
        domain_list[11].instance_id = 74;
        *num_domains = AR_SUB_SYS_ID_LAST + 1;
        goto end;
    }
    else
    {
        // service not found.
        status = AR_ENOTEXIST;
        goto end;
    }

end:
    AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "service(%s) status(0x%x)", service->name, status);
    return status;
}

/**
* \brief ar_osal_servreg_register
*        Service client(s) to register for the domain service state change notifications.
*
* \param[in opt]  cb_func: callback function pointer to get notifications on.
*.                         This is parameter is optional to Service provider registration.
* \param[in opt]  cb_context: callback function payload/context provided by client.
*.                         This is parameter is optional to Service provider registration.
* \param[in]  domain: domain of the service(msm/domain/subdomain) for which the
*.            state change notifications to be provided.
* \param[in]  service: service(provider/service) for which the
*.            state change notifications to be provided.
* \return
*  servreg_handle on success.
*  null on failure.
*/
_IRQL_requires_max_(PASSIVE_LEVEL)
ar_osal_servreg_t ar_osal_servreg_register(ar_osal_client_type  client_type,
    ar_osal_servreg_callback cb_func,
    void *cb_context,
    ar_osal_servreg_entry_type *domain,
    ar_osal_servreg_entry_type *service)
{
    PAGED_FUNCTION();
    int32_t status = AR_EOK;
    ar_osal_servreg_t handle = NULL;
    service_node *node = NULL;
    service_node *new_node = NULL;
    ar_list_node_t *current_node = NULL;
    uint32_t client_magic = 0;
    uint32_t ssr_event_mask = 0;

    if (NULL == domain || NULL == service)
    {
        status = AR_EBADPARAM;
        goto end;
    }

    AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "service_name:%s domain_name:%s ", service->name, domain->name);

    //check if service is already registered, not required, every new request will allocate a handle.
    ar_list_for_each_entry(current_node, &servreg_handle_pool.list_handle)
    {
        node = get_struc_base(current_node, service_node, srv_node);
        if (strcmp(node->service.name, service->name) == 0) {
            status = AR_EOK;
            handle = node->srv_handle;
            goto end;
        }
    }
    //created a node on each request.
    new_node = malloc(sizeof(service_node));
    if (NULL == new_node)
    {
        status = AR_ENOMEMORY;
        goto end;
    }
    memset(new_node, 0, sizeof(service_node));

    ar_list_init_node(&new_node->srv_node);

    if (cb_func)
    {
        new_node->cb_func = cb_func;
    }
    new_node->cb_context = cb_context;
    new_node->client_type = client_type;
    new_node->srv_state = AR_OSAL_SERVICE_STATE_DOWN;
    ar_mem_cpy(&new_node->service.name, sizeof(new_node->service.name), &service->name, sizeof(service->name));
    ar_mem_cpy(&new_node->domain.name, sizeof(new_node->domain.name), &domain->name, sizeof(domain->name));
    new_node->service.instance_id = service->instance_id;
    new_node->domain.instance_id = domain->instance_id;
    new_node->srv_handle = new_node;
    new_node->srv_cb_handle = 0;

    ar_osal_mutex_lock(servreg_handle_pool.lock);
    //add to the list;
    status = ar_list_add_tail(&servreg_handle_pool.list_handle, &new_node->srv_node);
    servreg_handle_pool.num_nodes++;
    ar_osal_mutex_unlock(servreg_handle_pool.lock);
    if (status != AR_EOK)
    {
        AR_LOG_ERR(AR_OSAL_SERVREG_TAG, "ar_list_add_tail error: %d", status);
        goto end;
    }
    handle = new_node;

    ssr_event_mask = ((SS_ID_LPASS | SS_ID_GPDSP0 | SS_ID_GPDSP1) << SS_ID_SHIFT) |
          (SSR_EVENT_RESTART_START | SSR_EVENT_RESTART_COMPLETE);
    status = ssr_register_callback_events(client_magic, arosal_ssr_event_cb, ssr_event_mask, &new_node->srv_cb_handle, new_node->domain.name, NULL);
    if (status) {
        AR_LOG_ERR(AR_OSAL_SERVREG_TAG, "Failed to reg ssr event: %d", status);
        goto end;
    }

end:
    if (status != AR_EOK)
    {
        if (new_node)
        {
            free(new_node);
        }
    }
    AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "ar_osal_servreg_register servreg_handle(0x%p), status(0x%x)", handle, status);
    return handle;
}

/**
* \brief ar_osal_servreg_deregister
*        Service client(s) to deregister for the service state change notifications.
*
* \param[in]  servreg_handle: interface handle returned by ar_osal_servreg_allocate_handle().
*
* \return
*  0 -- Success
*  Nonzero -- Failure
*/
_IRQL_requires_max_(PASSIVE_LEVEL)
int32_t ar_osal_servreg_deregister(_In_ ar_osal_servreg_t servreg_handle)
{
    PAGED_FUNCTION();
    uint32_t status = AR_EFAILED;
    bool_t node_freed = FALSE;
    service_node *node = NULL;
    ar_list_node_t *list_node = NULL;

    if (NULL == servreg_handle)
    {
        status = AR_EBADPARAM;
        goto end;
    }

    ar_osal_mutex_lock(servreg_handle_pool.lock);

    ar_list_for_each_entry(list_node, &servreg_handle_pool.list_handle)
    {
        node = get_struc_base(list_node, service_node, srv_node);
        if (servreg_handle == node->srv_handle)
        {
            ar_list_delete(&servreg_handle_pool.list_handle, list_node);
            servreg_handle_pool.num_nodes--;
            AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "ar_osal_servreg_deregister freed node(0x%p)", node);
            status = ssr_unregister_callback(node->srv_cb_handle);
            if (status) {
                AR_LOG_ERR(AR_OSAL_SERVREG_TAG, "Failed to dereg ssr event: %d", status);
                break;
            }
            AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "arosal deregister ssr succ, callback context=%s\n", node->cb_context);
            free(node);
            node_freed = TRUE;
            break;
        }
    }
    ar_osal_mutex_unlock(servreg_handle_pool.lock);

end:
    if (TRUE == node_freed)
    {
        status = AR_EOK;
    }
    AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "ar_osal_servreg_deregister servreg_handle(0x%p) status(0x%x)", servreg_handle, status);
    return status;
}

/**
* \brief ar_osal_servreg_set_state
*        Service provider to call this API to register its service states(UP/DOWN).
*        This API to be used only by the service provider(msm/domain/subdomain/provider/service)
*        and not by service client(s).
*
* \param[in]  servreg_handle: interface handle returned by ar_osal_servreg_allocate_handle().
* \param[in]  state: new service state for service registered using ar_osal_servreg_register().
*.
* \return
*  0 -- Success
*  Nonzero -- Failure
*/
_IRQL_requires_max_(PASSIVE_LEVEL)
int32_t ar_osal_servreg_set_state(_In_ ar_osal_servreg_t servreg_handle,
    _In_ ar_osal_service_state_type state)
{
    PAGED_FUNCTION();
    uint32_t status = AR_EOK;
    bool_t state_updated = FALSE;
    service_node *node = NULL;
    service_node *client_node = NULL;
    ar_list_node_t *list_node = NULL;
    ar_osal_servreg_state_notify_payload_type notify_state = { 0 };

    if (NULL == servreg_handle)
    {
        status = AR_EBADPARAM;
        goto end;
    }

    //update service state
    ar_osal_mutex_lock(servreg_handle_pool.lock);
    ar_list_for_each_entry(list_node, &servreg_handle_pool.list_handle)
    {
        node = get_struc_base(list_node, service_node, srv_node);
        if (servreg_handle == node->srv_handle)
        {
            node->srv_state = state;
            state_updated = TRUE;
            AR_LOG_INFO(AR_OSAL_SERVREG_TAG,
                "ar_osal_servreg_set_state service update domain(%s):iid(%d):service(%s):iid(%d) state(%d) handle(0x%p) status(0x%x)",
                node->domain.name, node->domain.instance_id, node->service.name,
                node->service.instance_id, state, servreg_handle, status);
            break;
        }
    }
    ar_osal_mutex_unlock(servreg_handle_pool.lock);

    list_node = NULL;
    //notify clients
    ar_osal_mutex_lock(servreg_handle_pool.lock);
    ar_list_for_each_entry(list_node, &servreg_handle_pool.list_handle)
    {
        client_node = get_struc_base(list_node, service_node, srv_node);
        if ( 0 == strcmp(client_node->service.name, node->service.name) &&
             0 == strcmp(client_node->domain.name, node->domain.name) &&
            client_node->service.instance_id == node->service.instance_id &&
            client_node->domain.instance_id == node->domain.instance_id &&
            AR_OSAL_CLIENT_LISTENER == client_node->client_type)
        {
            client_node->srv_state = state;
            notify_state.service_state = client_node->srv_state;
            ar_mem_cpy(notify_state.service.name, sizeof(notify_state.service.name), client_node->service.name, sizeof(client_node->service.name));
            ar_mem_cpy(notify_state.domain.name, sizeof(notify_state.domain.name), client_node->domain.name, sizeof(client_node->domain.name));
            notify_state.service.instance_id = client_node->service.instance_id;
            notify_state.domain.instance_id = client_node->domain.instance_id;

            //notify client
            if (client_node->cb_func)
            {
                client_node->cb_func(client_node->srv_handle, AR_OSAL_SERVICE_STATE_NOTIFY, client_node->cb_context, &notify_state, sizeof(notify_state));
                AR_LOG_INFO(AR_OSAL_SERVREG_TAG,
                    "ar_osal_servreg_set_state client cb done domain(%s):iid(%d):service(%s):iid(%d) state(%d) handle(0x%p)",
                    client_node->domain.name, client_node->domain.instance_id, client_node->service.name,
                    client_node->service.instance_id, client_node->srv_state, client_node->srv_handle);
            }
        }
    }
    ar_osal_mutex_unlock(servreg_handle_pool.lock);

end:
    if (FALSE == state_updated && status != AR_EBADPARAM)
    {
        // service handle not found and state not updated.
        status = AR_ENOTEXIST;
    }

    AR_LOG_INFO(AR_OSAL_SERVREG_TAG, "ar_osal_servreg_set_state state(%d) handle(0x%p) status(0x%x)",
        state, servreg_handle, status);
    return status;
}

PAGED_FUNCTIONS_END

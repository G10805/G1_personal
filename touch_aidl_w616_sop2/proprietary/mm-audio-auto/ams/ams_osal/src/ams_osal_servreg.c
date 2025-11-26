/**
 * \file ams_osal_serverg.c
 *
 * \brief
 *       This file has implementation of service location, notification, and
 *       state registration.
 *
 * \copyright
 *  Copyright (c) 2019-2021, 2023 by Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define AMS_OSAL_SERVREG_TAG "ams_osal_servreg
#include "ar_osal_types.h"
#include "ams_osal_servreg.h"
#include "ams_osal_mutex.h"
#include "ams_osal_error.h"
#include <stdlib.h>
#include <string.h>
#include <log/log.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <libpdmapper.h>
#include <libpdnotifier.h>
#include <service_registry_notifier_v01.h>

SR_DL_Handle *pd_mapper_handle;


int32_t g_init_done = 0;
struct ams_osal_service_node
{

    ams_osal_servreg_t srv_handle;
    ams_osal_servreg_entry_type service;
    ams_osal_servreg_entry_type domain;
    void *cb_context;
    ams_osal_servreg_callback cb_func;
    ams_osal_service_state_type srv_state;
    PD_Notifier_Handle *pd_handle;
};

typedef struct ams_osal_service_node ams_osal_service_node;
ams_osal_servreg_t serv_reg_handle;

static ams_osal_service_state_type pd_event_to_ams_osal_pd_state(enum pd_event event)
{
    ams_osal_service_state_type ret = AMS_OSAL_SERVICE_STATE_DOWN;
    switch (event)
    {
    case EVENT_PD_DOWN:
        ret = AMS_OSAL_SERVICE_STATE_DOWN;
        break;
    case EVENT_PD_UP:
        ret = AMS_OSAL_SERVICE_STATE_UP;
        break;
    default:
        ret = AMS_OSAL_SERVICE_STATE_DOWN;
        break;
    }
    return ret;
}

static ams_osal_service_state_type pd_state_to_ams_osal_pd_state(pd_state state)
{
    ams_osal_service_state_type ret = AMS_OSAL_SERVICE_STATE_DOWN;

    switch (state)
    {
    case SERVREG_NOTIF_SERVICE_STATE_DOWN_V01:
        ret = AMS_OSAL_SERVICE_STATE_DOWN;
        break;
    case SERVREG_NOTIF_SERVICE_STATE_UP_V01:
        ret = AMS_OSAL_SERVICE_STATE_UP;
        break;
    default:
        ret = AMS_OSAL_SERVICE_STATE_DOWN;
    }
    return ret;
}

static void ams_osal_pd_notifier_cb(void *data, enum pd_event event)
{
    ams_osal_service_node *entry = (ams_osal_service_node *)data;
    ams_osal_service_state_type state = pd_event_to_ams_osal_pd_state(event);
    ams_osal_servreg_state_notify_payload_type notify_state;

    ALOGD("ams_osal_pd_notifier_cb state(%d) service(%s)",
            state, entry->service.name);
    ALOGD("ams_osal_pd_notifier_cb state(%d) domain(%s)",
            state, entry->domain.name);

    notify_state.service_state = state;
    memcpy((void *)&notify_state.service, (const void *)&entry->service, sizeof(ams_osal_servreg_entry_type));
    memcpy((void *)&notify_state.domain, (const void *)&entry->domain, sizeof(ams_osal_servreg_entry_type));

    if (entry)
    {
        if (entry->cb_func)
        {
            entry->cb_func((ams_osal_servreg_t)entry,
                            AMS_OSAL_SERVICE_STATE_NOTIFY,
                            entry->cb_context,
                            (void *)&notify_state,
                            sizeof(notify_state));
        }
        entry->srv_state = state;
    }
}

/**
    * \brief ams_osal_servreg_init
    *        Initialize servreg interface.
    *........Note:This API has to be called before any other API in this interface.
    *........Should be called at least once and is expected to be serialized if called
    *        multiple times.
    * \return
    *  0 -- Success
    *  Nonzero -- Failure
    */

int32_t ams_osal_servreg_init(void)
{
    int32_t status = AMS_EOK;

    if (g_init_done)
    {
        g_init_done++;
        return status;
    }
    g_init_done = 1;

    pd_mapper_handle = servreg_alloc_DLHandle();
    if (!pd_mapper_handle)
    {
        status = AMS_ENOMEMORY;
        ALOGE("servreg allocation error status(%d)", status);
        return status;
    }

    ALOGD("ams_osal_servreg_init success status(%d)", status);

    return status;
}

/**
    * \brief ams_osal_servreg_deinit
    *        Uninitialize servreg interface.
    *        Should be called in pair with ams_osal_servreg_init() and
    *        should be a serialized call.
    * \return
    *  0 -- Success
    *  Nonzero -- Failure
    */

int32_t ams_osal_servreg_deinit(void)
{
    g_init_done--;
    if (0 != g_init_done)
    {
        goto end;
    }
    if (pd_mapper_handle)
    {
        servreg_free_DLHandle(pd_mapper_handle);
        pd_mapper_handle = NULL;
    }
end:
    return AMS_EOK;
}

/**
    * \brief ams_osal_servreg_get_domainlist
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
    *  AMS_STATUS_GENERAL_ERROR- Failed due to insufficient memory, client to call the API again
    *                  with required size as returned in num_domains.
    *.
    */

int32_t ams_osal_servreg_get_domainlist(ams_osal_servreg_entry_type *service,
                                        ams_osal_servreg_entry_type *domain_list,
                                        uint32_t *num_domains)
{
    int32_t status = AMS_EOK;
    if (NULL == service || NULL == num_domains)
    {
        status = AMS_EBADPARAM;
        goto end;
    }
    if (NULL == domain_list)
    {
        enum SR_Result_Enum rc = SR_RESULT_SUCCESS;
        rc = servreg_get_domainlist((char *)service->name, pd_mapper_handle);
        if (rc != SR_RESULT_SUCCESS)
        {
            status = AMS_EFAILED;
            ALOGE("servreg_get_domainlist error status(%d)", rc);
            goto end;
        }

        // return single domain
        *num_domains = servreg_get_numentries(pd_mapper_handle);
        status = AMS_ENOMEMORY;
    }
    else if ((domain_list != NULL) && (num_domains > 0))
    {
        // copy domain details
        enum SR_Result_Enum rc = SR_RESULT_SUCCESS;
        uint32_t i = 0;
        for (i = 0; i < *num_domains; i++)
        {
            char *name;
            int instance = 0;
            int service_data_valid = 0;
            int service_data = 0;
            rc = servreg_get_entry(pd_mapper_handle,
                                    &name,
                                    &instance,
                                    &service_data_valid,
                                    &service_data,
                                    i);
            if (rc != SR_RESULT_SUCCESS)
            {
                status = AMS_EFAILED;
                ALOGE(
                    "servreg_get_domainlist entry(%d) error status(%d) servreg status(%d)",
                    i, status, rc);
                goto end;
            }

            strlcpy((domain_list + i)->name, name, sizeof(domain_list->name));
            (domain_list + i)->instance_id = instance;

            ALOGD("servreg_get_domainlist entry(%d) instance(%d) domain name(%s)",
                    i, instance, (domain_list + i)->name);
        }
    }
    else
    {
        // service not found.
        status = AMS_EBADPARAM;
    }

end:
    ALOGD("ams_osal_servreg_get_domainlist service(%s) status(0x%x)",
            service->name, status);
    return status;
}

/**
    * \brief ams_osal_servreg_register
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

ams_osal_servreg_t ams_osal_servreg_register(char *client_name,
                                                ams_osal_servreg_callback cb_func,
                                                void *cb_context,
                                                ams_osal_servreg_entry_type *domain,
                                                ams_osal_servreg_entry_type *service)
{
    int32_t status = AMS_EOK;
    ams_osal_service_node *srv_reg_handle = NULL;
    enum pd_rcode pd_rc = PD_NOTIFIER_FAIL;
    pd_state state = SERVREG_NOTIF_SERVICE_STATE_DOWN_V01;

    if (NULL == domain || NULL == service || NULL == cb_func)
    {
        status = AMS_EBADPARAM;
        goto end;
    }

    srv_reg_handle = (ams_osal_service_node *)malloc(sizeof(ams_osal_service_node));
    if (!srv_reg_handle)
    {
        status = AMS_ENOMEMORY;
        ALOGE(
            "handle allocation error status(%d)", status);
        goto end;
    }

    memset((void *)srv_reg_handle, 0, sizeof(ams_osal_service_node));
    memcpy(&srv_reg_handle->service, service, sizeof(ams_osal_servreg_entry_type));
    memcpy(&srv_reg_handle->domain, domain, sizeof(ams_osal_servreg_entry_type));

    srv_reg_handle->cb_func = cb_func;
    srv_reg_handle->cb_context = cb_context;

    srv_reg_handle->pd_handle = pd_notifier_alloc(domain->name,
                                                    client_name,
                                                    domain->instance_id,
                                                    ams_osal_pd_notifier_cb,
                                                    (void *)srv_reg_handle);

    if (!srv_reg_handle->pd_handle)
    {
        status = AMS_EFAILED;
        ALOGE("pd_notifier_alloc error status(%d)", status);
        free(srv_reg_handle);
        srv_reg_handle = NULL;
        goto end;
    }

    pd_rc = pd_notifier_register(srv_reg_handle->pd_handle, &state);

    if (pd_rc != PD_NOTIFIER_SUCCESS)
    {
        status = AMS_EFAILED;
        ALOGE("pd_notifier_register error: status(%d)"
                "pd_notifier status(%d)",
                status, pd_rc);
        pd_notifier_free(srv_reg_handle->pd_handle);
        free(srv_reg_handle);
        srv_reg_handle = NULL;
        goto end;
    }
    else
    {
        ALOGD(
            "Successfully registered.  Curr state is %s state (0x%08x)",
            (state == EVENT_PD_UNKNOWN) ? "unknown" : ((state == EVENT_PD_UP) ? "up" : ((state == EVENT_PD_DOWN) ? "down" : "out of range")),
            state);
        srv_reg_handle->srv_state = pd_state_to_ams_osal_pd_state(state);
    }

end:
    ALOGD("Service register status(%d)", status);
    return (ams_osal_servreg_t)srv_reg_handle;
}

/**
    * \brief ams_osal_servreg_deregister
    *        Service client(s) to deregister for the service state change notifications.
    *
    * \param[in]  servreg_handle: interface handle returned by ams_osal_servreg_allocate_handle().
    *
    * \return
    *  0 -- Success
    *  Nonzero -- Failure
    */

int32_t ams_osal_servreg_deregister(ams_osal_servreg_t servreg_handle)
{
    int32_t status = AMS_EOK;
    enum pd_rcode pd_rc = PD_NOTIFIER_FAIL;
    ams_osal_service_node *srv_reg_handle = (ams_osal_service_node *)servreg_handle;
    if (NULL == servreg_handle)
    {
        status = AMS_EBADPARAM;
        return status;
    }

    pd_rc = pd_notifier_deregister(srv_reg_handle->pd_handle);
    if (pd_rc != PD_NOTIFIER_SUCCESS)
    {
        status = AMS_EFAILED;
        ALOGE("pd_notifier_deregister error: status(%d)"
                "pd_notifier status(%d)",
                status, pd_rc);
        pd_notifier_free(srv_reg_handle->pd_handle);
        free(srv_reg_handle);
        srv_reg_handle = NULL;
        return status;
    }
    pd_notifier_free(srv_reg_handle->pd_handle);
    free(srv_reg_handle);
    srv_reg_handle = NULL;

    return status;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
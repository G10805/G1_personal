#ifndef AMS_OSAL_SERVREG_H
#define AMS_OSAL_SERVREG_H

/**
 *
 * \file ams_osal_servreg.h
 * \brief
 *      Defines public APIs for service location,
 *      notification and state registration.
 * \cond
 *  Copyright (c) 2019-2021, 2023 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 * \endcond
 *
 */

#include "ar_osal_types.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/**  Max length of the domain name i.e "soc/domain/subdomain" or
 *   service name i.e "provider/service" is 64 bytes
     e.g. msm/adsp/audio_pd or avs/audio or audio/avs_mdf_sdsp or
     audio/avs_mdf_mdsp */
#define AMS_OSAL_SERVREG_NAME_LENGTH_MAX              (64)

/** ar osal servreg type object.
*/
typedef void *ams_osal_servreg_t;

/**  Struct representing the name of the service or domain and the instance id
 */
typedef struct {
    /**<   Name of the service or domain. */
    char_t name[AMS_OSAL_SERVREG_NAME_LENGTH_MAX + 1];

    /**<   Instance ID. */
    uint32_t instance_id;
}ams_osal_servreg_entry_type;

/** Service state up/down indicator.
 */
typedef enum ams_osal_service_state {
	/* service is in down state */
	AMS_OSAL_SERVICE_STATE_DOWN = 0,
	/* service is in up state */
	AMS_OSAL_SERVICE_STATE_UP = 1,
} ams_osal_service_state_type;

/** Servreg client type: listener or service provider.
 */
typedef enum ams_osal_client {
	/* invalid client */
	AMS_OSAL_CLIENT_INVALID	= 0,
	/* listener client */
	AMS_OSAL_CLIENT_LISTENER	= 1,
	/* service provide client */
	AMS_OSAL_CLIENT_SERVICE_PROVIDER	= 2,
} ams_osal_client_type;

/** Servreg callback notify events.
*/
typedef enum ams_osal_servreg_cb_event
{
    /* service state notify event */
    AMS_OSAL_SERVICE_STATE_NOTIFY = 1,
} ams_osal_servreg_cb_event_type;

/** servreg service state notify callback payload.
 */
typedef struct ams_osal_servreg_state_notify_payload {
    /* Service notification for */
    ams_osal_servreg_entry_type service;
    /* Service domain notification for */
    ams_osal_servreg_entry_type domain;
    /* service state on a domain */
    ams_osal_service_state_type  service_state;
} ams_osal_servreg_state_notify_payload_type;

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
int32_t ams_osal_servreg_init(void);

/**
* \brief ams_osal_servreg_deinit
*        Uninitialize servreg interface.
*........Should be called in pair with ams_osal_servreg_init() and
*........should be a serialized call.
* \return
*  0 -- Success
*  Nonzero -- Failure
*/
int32_t ams_osal_servreg_deinit(void);

/**
* \brief ams_osal_servreg_get_domainlist
*        Client to call this API to get a list of domains(msm/domain/subdomain) on which
*        a given service(provider/service) is supported.
*
* \param[in]  service: service(provider/service) for which domain(s) list is required.
* \param[in out] domain_list: service supported in domain(s), client to provide
*.                            payload buffer pointer.
* \param[in out] num_domains: Client to provide the num_domains to get the domain list.
*.                            If num_domains is zero and domain_list is NULL, API will return
*.                            the number of domains for the given service if available.
*
* \return
*  0 -- Success
*  Nonzero -- Failure
*  AMS_ENOMEMORY- Failed due to insufficient memory, client to call the API again
*                  with required size as returned in num_domains.
*.
*/
int32_t ams_osal_servreg_get_domainlist(ams_osal_servreg_entry_type *service,
    ams_osal_servreg_entry_type *domain_list,
    uint32_t *num_domains);

/**
* \brief ams_osal_servreg_callback
*        Callback function to notify clients for any changes in
*        service state(up/down).
* \param[in] servreg_handle: Servreg handle returned with ams_osal_servreg_register() for the given service.
* \param[in] event_id: callback event id supported by ams_osal_servreg_cb_event_type .
* \param[in] cb_context:  payload/context provided by client in ams_osal_servreg_register() .
*.\param[in] payload: payload provided by the callback.
*.                    service state UP/DOWN payload ams_osal_servreg_state_notify_payload_type.
*.\param[in] payload_size: payload size in bytes.
* \return
*  none
*/
typedef void(*ams_osal_servreg_callback)(ams_osal_servreg_t servreg_handle,
    ams_osal_servreg_cb_event_type event_id, void *cb_context, void *payload,
    uint32_t payload_size);

/**
* \brief ams_osal_servreg_register
*        Service client(s) to register for the domain service state change notifications.
*
* \param[in]  client_type: indicates registering client is a listener or service provider.
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
ams_osal_servreg_t ams_osal_servreg_register(char*  client_name,
    ams_osal_servreg_callback cb_func,
    void *cb_context,
    ams_osal_servreg_entry_type *domain,
    ams_osal_servreg_entry_type *service);

/**
* \brief ams_osal_servreg_deregister
*        Service client(s) to deregister for the service state change notifications.
*
* \param[in]  servreg_handle: interface handle returned by ams_osal_servreg_register().
*
* \return
*  0 -- Success
*  Nonzero -- Failure
*/
int32_t ams_osal_servreg_deregister(ams_osal_servreg_t servreg_handle);


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif //AMS_OSAL_SERVREG_H
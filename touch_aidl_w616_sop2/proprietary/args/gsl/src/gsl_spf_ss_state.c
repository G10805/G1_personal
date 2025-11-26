/**
 * \file gsl_spf_ss_state.c
 *
 * \brief
 * Maintains the state of Spf framework across all subsystems. With MDF
 * (Multi-DSP framework) capabilities there will be a Spf master service that
 * runs in a certain subsystem (typically ADSP) and Spf satelite services
 * run in other subsystems suchs as MDSP or CDSP.
 *
 *  Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include "gsl_spf_ss_state.h"
#include "gsl_common.h"
#include "gsl_intf.h"
#include "ar_osal_types.h"
#include "ar_osal_error.h"
#include "ar_osal_mutex.h"
#include "ar_osal_servreg.h"
#include "gsl_mdf_utils.h"

#define GSL_AUDIO_PD_DOMAIN_STR "msm/adsp/audio_pd"
#define GSL_COMPUTE_PD_DOMAIN_STR "msm/cdsp/audio_pd"
#define GSL_MODEM_PD_DOMAIN_STR "msm/mdsp/audio_pd"
#define GSL_SENSOR_PD_DOMAIN_STR "msm/sdsp/audio_pd"
#define GSL_AUDIO_PD_SERVICE_STR "avs/audio"

struct gsl_spf_ss_state {
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
	ar_osal_mutex_t lock;
	/*
	 * callback invoked whenever there is an SSR up or down event
	 */
	gsl_spf_ss_cb_t ssr_cb;
};

/*
 * A cluster has 1 master and N satellites
 * This array stores state information for all the masters and corresponding
 * satellites.
 */
struct gsl_spf_ss_state *_spf_cluster_ss_state[AR_SUB_SYS_ID_LAST + 1] = {NULL};

static uint32_t spf_cluster_ref_count;

static void servreg_callback(ar_osal_servreg_t servreg_handle,
	ar_osal_servreg_cb_event_type event_id, void *cb_context, void *payload,
	uint32_t payload_size)
{
	uint32_t ss_mask = 0;
	uint32_t master_proc = AR_SUB_SYS_ID_INVALID;

	/*
	 * reference unused variables to make compiler happy. Can't change function
	 * signature because it needs to match what OSAL expects.
	 */
	(void)servreg_handle;
	(void)cb_context;
	(void)payload_size;

	ar_osal_servreg_state_notify_payload_type *serv_ntfy_pld =
		(ar_osal_servreg_state_notify_payload_type *)payload;

	if (event_id == AR_OSAL_SERVICE_STATE_NOTIFY) {
		if (!strcmp(serv_ntfy_pld->domain.name, GSL_AUDIO_PD_DOMAIN_STR)) {
			ss_mask = GSL_GET_SPF_SS_MASK(AR_AUDIO_DSP);
		} else if (!strcmp(serv_ntfy_pld->domain.name,
			GSL_COMPUTE_PD_DOMAIN_STR)) {
			ss_mask = GSL_GET_SPF_SS_MASK(AR_COMPUTE_DSP);
		} else if (!strcmp(serv_ntfy_pld->domain.name,
			GSL_MODEM_PD_DOMAIN_STR)) {
			ss_mask = GSL_GET_SPF_SS_MASK(AR_MODEM_DSP);
		} else if (!strcmp(serv_ntfy_pld->domain.name,
			GSL_SENSOR_PD_DOMAIN_STR)) {
			ss_mask = GSL_GET_SPF_SS_MASK(AR_SENSOR_DSP);
		} else {
			GSL_ERR("could not recognize domain %s",
				serv_ntfy_pld->domain.name);
			return;
		}

		master_proc = gsl_mdf_utils_get_master_proc_id(ss_mask);
		if (master_proc == 0) {
			GSL_ERR("Invalid proc id %d", master_proc);
			return;
		}
		if (serv_ntfy_pld->service_state == AR_OSAL_SERVICE_STATE_DOWN) {
			/* update the global Spf SS state */
			gsl_spf_ss_state_set(master_proc, ss_mask, GSL_SPF_SS_STATE_DN);
			/* notify gsl_main */
			_spf_cluster_ss_state[master_proc]->ssr_cb(GSL_SPF_SS_STATE_DN,
								   ss_mask);
		} else if (serv_ntfy_pld->service_state ==
			AR_OSAL_SERVICE_STATE_UP) {
			/* update the global Spf SS state */
			 gsl_spf_ss_state_set(master_proc, ss_mask, GSL_SPF_SS_STATE_UP);
			/* notify gsl_main */
			_spf_cluster_ss_state[master_proc]->ssr_cb(GSL_SPF_SS_STATE_UP,
								   ss_mask);
		}
	}
}

static uint32_t gsl_servreg_setup(void)
{
	int32_t rc = AR_EOK;
	ar_osal_servreg_entry_type service = { GSL_AUDIO_PD_SERVICE_STR, 0 };
	ar_osal_servreg_entry_type *domain_list = NULL;
	uint32_t i = 0, domain_list_size = 0;
	ar_osal_servreg_t servreg_handle;

	rc = ar_osal_servreg_init();
	if (rc) {
		GSL_ERR("failed to initialize ar_osal state notifier %d", rc);
		goto exit;
	}

	/* first we query for number of entries in domain list */
	rc = ar_osal_servreg_get_domainlist(&service, NULL, &domain_list_size);
	if (rc != AR_ENOMEMORY) {
		GSL_ERR("failed to get num domains with ar_osal state notifier %d",
			rc);
		goto deinit_state_notify;
	}

	/* next we query for actual list */
	domain_list = gsl_mem_zalloc(domain_list_size *
		sizeof(ar_osal_servreg_entry_type));
	if (!domain_list) {
		rc = AR_ENOMEMORY;
		goto deinit_state_notify;
	}
	rc = ar_osal_servreg_get_domainlist(&service, domain_list,
		&domain_list_size);
	if (rc) {
		GSL_ERR("failed to get domains with ar_osal state notifier %d", rc);
		goto free_domain_list;
	}

	/* scan through domain list and register for notification */
	for (i = 0; i < domain_list_size; ++i) {
		servreg_handle = ar_osal_servreg_register(AR_OSAL_CLIENT_LISTENER,
						servreg_callback, &_spf_cluster_ss_state[i],
						domain_list + i, &service);
		if (!servreg_handle) {
			GSL_ERR("failed to register for service %c", rc);
			rc = AR_EFAILED;
			goto free_domain_list;
		}
	}

	gsl_mem_free(domain_list);
	return rc;

free_domain_list:
	gsl_mem_free(domain_list);
deinit_state_notify:
	ar_osal_servreg_deinit();
exit:
	return rc;
}

bool_t gsl_spf_ss_state_is_ss_supported(uint32_t master_proc, uint32_t proc_id)
{
	uint32_t ss_mask = 0;

	if (_spf_cluster_ss_state[master_proc] != NULL) {
		ss_mask = _spf_cluster_ss_state[master_proc]->ss_supported_flags;
		if (GSL_TEST_SPF_SS_BIT(ss_mask, proc_id))
		return TRUE;
	}

		return FALSE;
}

int32_t gsl_spf_ss_state_init(uint32_t master_proc, uint32_t supported_ss_mask,
	gsl_spf_ss_cb_t cb)
{
	int32_t rc = AR_EOK;

	if (master_proc <= AR_SUB_SYS_ID_INVALID ||
	    master_proc > AR_SUB_SYS_ID_LAST)
		return AR_EBADPARAM;
	_spf_cluster_ss_state[master_proc] =
				gsl_mem_zalloc(sizeof(struct gsl_spf_ss_state));

	rc = ar_osal_mutex_create(&_spf_cluster_ss_state[master_proc]->lock);
	if (rc)
		goto exit;

	if (spf_cluster_ref_count == 0) {
		rc = gsl_servreg_setup();
		if (rc)
			goto cleanup;
	}
	_spf_cluster_ss_state[master_proc]->ss_supported_flags = supported_ss_mask;
	_spf_cluster_ss_state[master_proc]->ssr_cb = cb;
	spf_cluster_ref_count++;
	return rc;
cleanup:
	ar_osal_mutex_destroy(_spf_cluster_ss_state[master_proc]->lock);
exit:
	gsl_mem_free(_spf_cluster_ss_state[master_proc]);
	_spf_cluster_ss_state[master_proc] = NULL;
	return rc;
}

void gsl_spf_ss_state_deinit(uint32_t master_proc)
{
	if (master_proc <= AR_SUB_SYS_ID_INVALID ||
	    master_proc > AR_SUB_SYS_ID_LAST)
		return;

	if (_spf_cluster_ss_state[master_proc] != NULL) {
		_spf_cluster_ss_state[master_proc]->ss_supported_flags = 0;
		_spf_cluster_ss_state[master_proc]->ssr_cb = NULL;
		ar_osal_mutex_destroy(_spf_cluster_ss_state[master_proc]->lock);
		gsl_mem_free(_spf_cluster_ss_state[master_proc]);
		_spf_cluster_ss_state[master_proc] = NULL;
		spf_cluster_ref_count--;
	}

	if (spf_cluster_ref_count == 0)
		ar_osal_servreg_deinit();
}

uint32_t gsl_spf_ss_state_set(uint32_t master_proc, uint32_t ss_mask,
	enum spf_ss_state_t state)
{
	uint32_t updated_ss_state = 0;

	if (master_proc <= AR_SUB_SYS_ID_INVALID ||
	    master_proc > AR_SUB_SYS_ID_LAST)
		return AR_EBADPARAM;
	if (_spf_cluster_ss_state[master_proc] == NULL)
		return AR_EBADPARAM;
	GSL_MUTEX_LOCK(_spf_cluster_ss_state[master_proc]->lock);
	if ((ss_mask & _spf_cluster_ss_state[master_proc]->ss_supported_flags) &&
	    (state == GSL_SPF_SS_STATE_UP))
		_spf_cluster_ss_state[master_proc]->ss_state_flags |= ss_mask;
	else if ((ss_mask &
		 _spf_cluster_ss_state[master_proc]->ss_supported_flags) &&
		 (state == GSL_SPF_SS_STATE_DN)) {
		_spf_cluster_ss_state[master_proc]->ss_state_flags &= ~ss_mask;
		/* @TODO free mdf shared mem for proc_ids that went down */
	}
	updated_ss_state = _spf_cluster_ss_state[master_proc]->ss_state_flags;
	GSL_MUTEX_UNLOCK(_spf_cluster_ss_state[master_proc]->lock);

	return updated_ss_state;
}

uint32_t gsl_spf_ss_state_get(uint32_t master_proc)
{
	if (master_proc <= AR_SUB_SYS_ID_INVALID ||
	    master_proc > AR_SUB_SYS_ID_LAST)
		return AR_EBADPARAM;
	if (_spf_cluster_ss_state[master_proc] != NULL)
		return _spf_cluster_ss_state[master_proc]->ss_state_flags;
	return AR_EOK;
}

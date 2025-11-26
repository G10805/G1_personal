/**
 * \file gsl_common.c
 *
 * \brief
 *      Common utilities used by various modules within GSL, this is the
 *      lowest level module within GSL
 *
 *  Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include "gsl_common.h"
#include "ar_osal_error.h"
#include "ar_osal_mutex.h"
#include "gpr_api_inline.h"
#include "apm_api.h"

uint32_t gsl_signal_create(struct gsl_signal *sig_p, ar_osal_mutex_t *lock)
{
	if (!sig_p)
		return AR_EBADPARAM;

	sig_p->flags = 0;
	sig_p->status = 0;
	sig_p->gpr_packet = NULL;
	sig_p->lock = lock;

	return ar_osal_signal_create(&sig_p->sig);
}

uint32_t gsl_signal_destroy(struct gsl_signal *sig_p)
{
	if (!sig_p)
		return AR_EBADPARAM;

	/* if there was a pending gpr packet that never got consumed free it */
	if (sig_p->gpr_packet)
		__gpr_cmd_free(sig_p->gpr_packet);
	sig_p->gpr_packet = NULL;

	return ar_osal_signal_destroy(sig_p->sig);
}

uint32_t gsl_signal_timedwait(struct gsl_signal *sig_p, uint32_t timeout_ms,
	uint32_t *ev_flags, uint32_t *status, gpr_packet_t **gpr_packet)
{
	uint32_t rc = AR_EOK;

	if (!sig_p)
		return AR_EBADPARAM;

	rc = ar_osal_signal_timedwait(sig_p->sig, GSL_TIMEOUT_NS(timeout_ms));
	if (!rc) {
		if (sig_p->lock)
			GSL_MUTEX_LOCK(*sig_p->lock);
		ar_osal_signal_clear(sig_p->sig);
		*ev_flags = sig_p->flags;
		if (status)
			*status = sig_p->status;
		if (gpr_packet)
			*gpr_packet = (gpr_packet_t *) sig_p->gpr_packet;
		sig_p->gpr_packet = NULL;
		sig_p->flags = 0; /* clear */
		sig_p->status = 0;
		if (sig_p->lock)
			GSL_MUTEX_UNLOCK(*sig_p->lock);
	}
	return rc;
}

uint32_t gsl_signal_set(struct gsl_signal *sig_p, uint32_t ev_flags,
	int32_t status, void *gpr_packet)
{
	uint32_t rc = AR_EOK;

	if (!sig_p)
		return AR_EBADPARAM;

	if (sig_p->lock)
		GSL_MUTEX_LOCK(*sig_p->lock);
	sig_p->flags |= ev_flags;
	sig_p->status = status;
	/* if there was a pending gpr packet that never got consumed free it */
	if (sig_p->gpr_packet)
		__gpr_cmd_free(sig_p->gpr_packet);
	sig_p->gpr_packet = gpr_packet;
	rc = ar_osal_signal_set(sig_p->sig);
	if (sig_p->lock)
		GSL_MUTEX_UNLOCK(*sig_p->lock);

	return rc;
}

uint32_t gsl_signal_clear(struct gsl_signal *sig_p, uint32_t ev_flags)
{
	uint32_t rc = AR_EOK;

	if (!sig_p)
		return AR_EBADPARAM;
	if (sig_p->lock)
		GSL_MUTEX_LOCK(*sig_p->lock);
	sig_p->flags &= ~ev_flags;
	rc = ar_osal_signal_clear(sig_p->sig);
	if (sig_p->lock)
		GSL_MUTEX_UNLOCK(*sig_p->lock);

	return rc;
}

int32_t gsl_allocate_gpr_packet(uint32_t opcode, uint32_t src_port,
	uint32_t dst_port, uint32_t payload_size, uint32_t token,
	uint32_t dest_domain, struct gpr_packet_t **alloc_packet)
{
	int32_t rc = AR_EOK;
	gpr_cmd_alloc_ext_t	alloc_params;

	alloc_params.src_domain_id = GSL_GPR_SRC_DOMAIN_ID;
	alloc_params.dst_domain_id = (uint8_t) dest_domain;
	alloc_params.client_data = 0;

	alloc_params.src_port = src_port;
	alloc_params.dst_port = dst_port;
	alloc_params.token = token;
	alloc_params.opcode = opcode;
	alloc_params.payload_size = payload_size;
	alloc_params.ret_packet = alloc_packet;

	rc = __gpr_cmd_alloc_ext(&alloc_params);
	return rc;
}

/* Payload must be 8B aligned for spf */
int32_t gsl_send_spf_cmd(gpr_packet_t **packet, struct gsl_signal *sig_p,
	gpr_packet_t **rsp_pkt)
{
	int32_t rc = AR_EOK;
	uint32_t ev_flags = 0, spf_status = 0;

	#ifdef GSL_DEBUG_ENABLE
	/* Cache debug variables for later
	 * Needed as there is no guarantee of packet surviving beyond async_send
	 */
	uint32_t src_port = (*packet)->src_port, opcode = (*packet)->opcode,
		dst_port = (*packet)->dst_port;
	#endif

	rc = __gpr_cmd_async_send(*packet);
	if (rc) {
		__gpr_cmd_free(*packet);
		goto exit;
	}

	if (sig_p != NULL) {
		GSL_DBG("Wait for Rsp opcode[0x%x] src_port[0x%x] dst_port[0x%x]",
			opcode, src_port, dst_port);
		/*
		 * once osal signal is set it remains set till cleared, so if we
		 * got a set before wait is called the below wait will immediately
		 * return
		 */
		if ((*packet)->opcode == APM_CMD_GRAPH_OPEN) {
			rc = gsl_signal_timedwait(sig_p, GSL_GRAPH_OPEN_TIMEOUT_MS, &ev_flags,
					&spf_status, rsp_pkt);
		} else {
			rc = gsl_signal_timedwait(sig_p, GSL_SPF_TIMEOUT_MS, &ev_flags,
					&spf_status, rsp_pkt);
		}
		GSL_DBG("Wait done rc[0x%x] opcd[0x%x] src_prt[0x%x] dst_prt[0x%x]",
			rc, opcode, src_port, dst_port);
		GSL_DBG("flags[0x%x] spf_status[0x%x]", ev_flags, spf_status);
		if (rc)
			rc = AR_ETIMEOUT;
		else if (ev_flags & GSL_SIG_EVENT_MASK_CLOSE)
			rc = AR_EABORTED;
		else if (ev_flags & GSL_SIG_EVENT_MASK_SSR)
			rc = AR_ESUBSYSRESET;
		else if ((ev_flags & GSL_SIG_EVENT_MASK_SPF_RSP)) {
			rc = spf_status;
			if (rsp_pkt && *rsp_pkt) {
				GSL_LOG_PKT("recv_pkt", ((gpr_packet_t *)(*rsp_pkt))->src_port,
					(gpr_packet_t *)(*rsp_pkt),
					GPR_PKT_GET_HEADER_BYTE_SIZE(
					((gpr_packet_t *)(*rsp_pkt))->header) +
					GPR_PKT_GET_PAYLOAD_BYTE_SIZE(
					((gpr_packet_t *)(*rsp_pkt))->header), NULL, 0);
			}
		}
	}

exit:
	/* mark packet null to indicate it has been sent */
	*packet = NULL;
	return rc;
}

int32_t gsl_send_spf_cmd_wait_for_basic_rsp(gpr_packet_t **packet,
	struct gsl_signal *sig_p)
{
	int32_t rc = AR_EOK;
	gpr_packet_t *rsp = NULL;
	struct spf_cmd_basic_rsp *basic_rsp = NULL;
	uint32_t cached_opcode = (*packet)->opcode;
	/* cache opcode since packet is freed when sent */

	rc = gsl_send_spf_cmd(packet, sig_p, &rsp);
	if (!rc && rsp && rsp->opcode == GPR_IBASIC_RSP_RESULT) {
		basic_rsp = GPR_PKT_GET_PAYLOAD(struct spf_cmd_basic_rsp, rsp);
		if (cached_opcode != basic_rsp->opcode) {
			GSL_ERR("Recieved unexpected rsp opcode %x, expected %x",
				basic_rsp->opcode, cached_opcode);
			rc = AR_EUNEXPECTED;
		}
	}

	if (rsp)
		__gpr_cmd_free(rsp);

	return rc;
}

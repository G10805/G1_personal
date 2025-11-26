/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *               GVM Diag communication support
 *
 *GENERAL DESCRIPTION
 *
 *Implementation of diag GVM specific API's to communicate with PVM diag.
 *
 **====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "diag.h"
#include "diag_cntl.h"
#include "vm.h"

#ifdef USE_GLIB
#include <glib.h>
#define strlcpy g_strlcpy
#endif

/* send feature mask to pvm */
void diag_vm_send_feature_mask(struct peripheral *peripheral)
{
	peripheral->features = 0;
	peripheral->features |= DIAG_FEATURE_FEATURE_MASK_SUPPORT;
	if (peripheral->cmd_fd >= 0)
		peripheral->features |= DIAG_FEATURE_REQ_RSP_SUPPORT;
	if (peripheral->sockets)
		peripheral->features |= DIAG_FEATURE_SOCKETS_ENABLED;
	peripheral->features |= DIAG_FEATURE_DIAG_ID;
	ALOGI("diag: %s: %s Sending feature mask to pvm\n", __func__, peripheral->self_name);
	diag_cntl_send_feature_mask(peripheral, peripheral->features);
}

/* send diag ID query request to pvm */
int diag_vm_query_diag_id(struct peripheral *peripheral, const char* process_name)
{
	struct diag_cntl_cmd_diag_id *ctrl_pkt;
	size_t len;

	if (!process_name || !peripheral)
		return -EINVAL;

	ctrl_pkt = malloc(sizeof(*ctrl_pkt));
	if (!ctrl_pkt)
		return -ENOMEM;

	ALOGI("diag: %s: Requesting diag ID for process %s\n",__func__, process_name);
	ctrl_pkt->hdr.cmd = DIAG_CNTL_CMD_DIAG_ID;
	ctrl_pkt->version = DIAGID_VERSION_1;
	ctrl_pkt->diag_id = 0;
	strlcpy((char *)&ctrl_pkt->process_name, process_name,
		sizeof(ctrl_pkt->process_name));
	ctrl_pkt->hdr.len = sizeof(ctrl_pkt->diag_id) + sizeof(ctrl_pkt->version) +
			strlen(process_name) + 1;
	len = ctrl_pkt->hdr.len+ sizeof(ctrl_pkt->hdr);
	queue_push_cntlq(peripheral, ctrl_pkt, len);
	return 0;
}

/* process msg mask received from pvm */
int diag_vm_process_msg_mask(struct peripheral *peripheral,
			      struct diag_cntl_hdr *hdr, size_t len)
{
	struct diag_cntl_cmd_msg_mask *pkt = to_cmd_msg_mask(hdr);
	int ret = 1;

	if (sizeof(*pkt) + (pkt->msg_mask_len * sizeof(uint32_t)) != len)
		return -EMSGSIZE;

	if (pkt->status != DIAG_CTRL_MASK_INVALID &&
		pkt->stream_id == 1) {
		ret = diag_cmd_set_msg_mask(pkt->range, pkt->range_msg_mask, INVALID_INDEX, 0);
		clients_broadcast_msg_mask();
	}

	return ret;
}

/* process event mask received from pvm */
int diag_vm_process_event_mask(struct peripheral *peripheral,
				struct diag_cntl_hdr *hdr, size_t len)
{
	struct diag_cntl_cmd_event_mask *pkt = to_cmd_event_mask(hdr);
	int ret = 1;

	if (sizeof(*pkt) + pkt->event_mask_len != len)
		return -EMSGSIZE;

	/* handle only valid mask packet with stream_id 1 */
	if(pkt->status != DIAG_CTRL_MASK_INVALID &&
		pkt->stream_id == 1) {
		ret = diag_cmd_update_event_mask((pkt->event_mask_len * 8),
				pkt->event_mask, INVALID_INDEX, 0);
		clients_broadcast_event_mask();
	}

	return ret;
}

/* process log mask received from pvm */
int diag_vm_process_log_mask(struct peripheral *peripheral,
			      struct diag_cntl_hdr *hdr, size_t len)
{
	struct diag_cntl_cmd_log_mask *pkt = to_cmd_log_mask(hdr);
	uint32_t num_items;
	uint32_t mask_size;
	int ret = 1;

	num_items = MIN(pkt->last_item, MAX_ITEMS_ALLOWED);
	mask_size = pkt->log_mask_size;

	if (sizeof(*pkt) + mask_size != len)
		return -EMSGSIZE;

	if (pkt->status != DIAG_CTRL_MASK_INVALID &&
		pkt->stream_id == 1) {
		ret = diag_cmd_set_log_mask(pkt->equip_id, &num_items, pkt->equip_log_mask, &mask_size, 0, INVALID_INDEX);
		clients_broadcast_log_mask();
	}

	return ret;
}

/* send data packet to pvm */
int diag_vm_write(struct diag_client *dm, int proc, int peripheral, unsigned char *buf,
		   int len, struct watch_flow *flow, int cmd_rsp_flag)
{
	struct peripheral *perif = NULL;
	int ret = -1;

	perif = diag_get_periph_info(peripheral);
	if (perif != NULL) {
		queue_push(NULL, &perif->dataq, buf, len);
		ret = 0;
	}

	return ret;
}

/*
 * Copyright (c) 2016-2018, The Linux Foundation. All rights reserved.
 * Copyright (c) 2016, Linaro Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <err.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "diag.h"
#include "dm.h"
#include "hdlc.h"
#include "peripheral.h"
#include "util.h"
#include "diag_hidl_wrapper.h"
#include "diag_mux.h"

struct list_head fallback_cmds = LIST_INIT(fallback_cmds);
struct list_head common_cmds = LIST_INIT(common_cmds);
struct list_head apps_cmds = LIST_INIT(apps_cmds);

int hdlc_enqueue_flow(struct diag_client *dm, struct list_head *queue, const void *msg, size_t msglen, struct watch_flow *flow)
{
	size_t outlen;
	void *outbuf;
	int encode_flag = 0;

	if (dm && dm->hdlc_enc_done) {
		outbuf = (void*)msg;
		outlen = msglen;
	} else {
		outbuf = hdlc_encode(dm, msg, msglen, &outlen);
		if (!outbuf) {
			ALOGE("diag: failed to allocate hdlc destination buffer");
			return -ENOMEM;
		}
		if (dm && (dm->hdlc_read_buf == outbuf))
			encode_flag = 0;
		else
			encode_flag = 1;
	}
	queue_push_flow(dm, queue, outbuf, outlen, flow);
	if (encode_flag) {
		free(outbuf);
		outbuf = NULL;
	}
	return 0;
}
int hdlc_enqueue(struct list_head *queue, const void *msg, size_t msglen)
{
	return hdlc_enqueue_flow(NULL, queue, msg, msglen, NULL);
}
static int diag_cmd_dispatch(struct diag_client *client, uint8_t *ptr,
			     size_t len, int pid)
{
	struct list_head *item;
	struct diag_cmd *dc;
	unsigned int key = 0;
	int handled = 0;
	unsigned char apps_buf[DIAG_MAX_REQ_SIZE];
	int pkt_type = PKT_TYPE;
	int ret = 0;

	memset(apps_buf, 0, sizeof(apps_buf));
	if (ptr[0] == DIAG_CMD_SUBSYS_DISPATCH ||
	    ptr[0] == DIAG_CMD_SUBSYS_DISPATCH_V2) {
		if (len == 1)
			key = ptr[0] << 24;
		else if (len == 2)
			key = ptr[0] << 24 | ptr[1] << 16;
		else if (len == 3)
			key = ptr[0] << 24 | ptr[1] << 16 | ptr[2];
		else
			key = ptr[0] << 24 | ptr[1] << 16 | ptr[3] << 8 | ptr[2];
	} else {
		key = 0xff << 24 | 0xff << 16 | ptr[0];
	}
	if (ptr[0] == MODE_CMD && ptr[1] != MODE_CMD_RESET) {
		goto periph_send;
	}
	list_for_each(item, &common_cmds) {
		dc = container_of(item, struct diag_cmd, node);
		if (key < dc->first || key > dc->last)
			continue;

		return dc->cb(client, ptr, len, pid);
	}

	list_for_each(item, &apps_cmds) {
		dc = container_of(item, struct diag_cmd, node);
		if (key < dc->first || key > dc->last)
			continue;

		if (dc->cb)
			dc->cb(client, ptr, len, pid);
		else {
			if (len + sizeof(pkt_type) >= MAX_PKT_LEN)
				return -EMSGSIZE;
			memcpy(apps_buf, &pkt_type, sizeof(pkt_type));
			memcpy(apps_buf + sizeof(pkt_type), ptr, len);
			if (diag_debug_mask & DIAG_DBG_MASK_CMD) {
				ALOGM(DIAG_DBG_MASK_CMD, "diag: %s: Sending cmd pkt of len: %d to client fd: %d\n",
					__func__, len, dc->fd);
				print_hex_dump(__func__, (void *)ptr, MIN(len, PRINT_HEX_DUMP_LEN));
			}
			ret = write(dc->fd, apps_buf, len + sizeof(pkt_type));
			if (ret < 0)
				return -ENOENT;
		}

		handled++;
		break;
	}
	if (handled)
		return 0;
#ifndef FEATURE_LE_DIAG
	handled = check_for_diag_system_client_commands(key, ptr, len);
	if (handled) {
		if (diag_debug_mask & DIAG_DBG_MASK_CMD)
			print_hex_dump(__func__, ptr, MIN(len, PRINT_HEX_DUMP_LEN));
		return 0;
	}
#endif /* FEATURE_LE_DIAG */
periph_send:
	list_for_each(item, &diag_cmds) {
		dc = container_of(item, struct diag_cmd, node);
		if (key < dc->first || key > dc->last)
			continue;

		if (dc->cb) {
			dc->cb(client, ptr, len, pid);
			return 0;
		} else {
			if (diag_debug_mask & DIAG_DBG_MASK_CMD) {
				ALOGM(DIAG_DBG_MASK_CMD, "diag: %s: Sending cmd pkt of len: %d to perif: %s\n",
					__func__, len, dc->peripheral->name);
				print_hex_dump(__func__, (void *) ptr, MIN(len, PRINT_HEX_DUMP_LEN));
			}
			return peripheral_send(dc->peripheral, ptr, len);
		}
	}

	list_for_each_entry(dc, &fallback_cmds, node) {
		if (key < dc->first || key > dc->last)
			continue;

		if (diag_debug_mask & DIAG_DBG_MASK_CMD) {
			ALOGM(DIAG_DBG_MASK_CMD, "diag: %s: Diag-router handling cmd pkt of len: %d\n",
				__func__, len);
			print_hex_dump(__func__, (void *) ptr, MIN(len, PRINT_HEX_DUMP_LEN));
		}
		return dc->cb(client, ptr, len, pid);
	}

	return -ENOENT;
}

static void diag_rsp_bad_command(struct diag_client *client, uint8_t *msg,
				 size_t len, int error_code, int pid)
{
	uint8_t *buf;
	(void)client;

	buf = malloc(len + 1);
	if (!buf) {
		err(1, "failed to allocate error buffer");
		return;
	}

	buf[0] = error_code;
	memcpy(buf + 1, msg, len);

	diag_apps_rsp_send(pid, buf, len + 1);

	free(buf);
	buf = NULL;
}

int diag_client_handle_command(struct diag_client *client, uint8_t *data, size_t len, int pid)
{
	int ret;

	ret = diag_cmd_dispatch(client, data, len, pid);

	switch (ret) {
	case -ENOENT:
		diag_rsp_bad_command(client, data, len, DIAG_CMD_RSP_BAD_COMMAND, pid);
		break;
	case -EINVAL:
		diag_rsp_bad_command(client, data, len, DIAG_CMD_RSP_BAD_PARAMS, pid);
		break;
	case -EMSGSIZE:
		diag_rsp_bad_command(client, data, len, DIAG_CMD_RSP_BAD_LENGTH, pid);
		break;
	default:
		break;
	}

	return 0;
}

void register_fallback_cmd(unsigned int cmd,
			   int(*cb)(struct diag_client *client,
				    const void *buf, size_t len, int pid))
{
	struct diag_cmd *dc;
	unsigned int key = 0xffff0000 | cmd;

	dc = calloc(1, sizeof(*dc));
	if (!dc) {
		err(1, "failed to allocate diag command\n");
		return;
	}

	dc->first = key;
	dc->last = key;
	dc->cb = cb;

	list_add(&fallback_cmds, &dc->node);
}

void register_fallback_subsys_cmd(unsigned int subsys, unsigned int cmd,
					int(*cb)(struct diag_client *client,
					const void *buf, size_t len, int pid))
{
	struct diag_cmd *dc;
	unsigned int key = DIAG_CMD_SUBSYS_DISPATCH << 24 |
			   (subsys & 0xff) << 16 | cmd;

	dc = calloc(1, sizeof(*dc));
	if (!dc) {
		err(1, "failed to allocate diag command\n");
		return;
	}
	dc->first = key;
	dc->last = key;
	dc->cb = cb;

	list_add(&fallback_cmds, &dc->node);
}
void register_fallback_subsys_cmd_range(unsigned int subsys, unsigned int cmd_code_low,
					unsigned int cmd_code_high,
					int(*cb)(struct diag_client *client,
					const void *buf, size_t len, int pid))
{
	struct diag_cmd *dc;
	unsigned int cmd_code_low_key = DIAG_CMD_SUBSYS_DISPATCH << 24 |
				(subsys & 0xff) << 16 | cmd_code_low;
	unsigned int cmd_code_high_key = DIAG_CMD_SUBSYS_DISPATCH << 24 |
 				(subsys & 0xff) << 16 | cmd_code_high;

	dc = calloc(1, sizeof(*dc));
	if (!dc) {
		err(1, "failed to allocate diag command\n");
		return;
	}
	dc->first = cmd_code_low_key;
	dc->last = cmd_code_high_key;
	dc->cb = cb;

	list_add(&fallback_cmds, &dc->node);
}

void register_fallback_subsys_cmd_v2(unsigned int subsys, unsigned int cmd,
	int(*cb)(struct diag_client *client,
	const void *buf, size_t len, int pid))
{
	struct diag_cmd *dc;
	unsigned int key = DIAG_CMD_SUBSYS_DISPATCH_V2 << 24 |
		(subsys & 0xff) << 16 | cmd;

	dc = calloc(1, sizeof(*dc));
	if (!dc) {
		err(1, "failed to allocate diag command\n");
		return;
	}
	dc->first = key;
	dc->last = key;
	dc->cb = cb;

	list_add(&fallback_cmds, &dc->node);
}

void register_common_cmd(unsigned int cmd, int(*cb)(struct diag_client *client,
						    const void *buf,
						    size_t len , int pid))
{
	struct diag_cmd *dc;
	unsigned int key = 0xffff0000 | cmd;

	dc = calloc(1, sizeof(*dc));
	if (!dc) {
		err(1, "failed to allocate diag command\n");
		return;
	}

	dc->first = key;
	dc->last = key;
	dc->cb = cb;

	list_add(&common_cmds, &dc->node);
}


void register_common_subsys_cmd(unsigned int subsys, unsigned int cmd,
				int(*cb)(struct diag_client *client,
				const void *buf, size_t len, int pid))
{
	struct diag_cmd *dc;
	unsigned int key = DIAG_CMD_SUBSYS_DISPATCH << 24 |
			   (subsys & 0xff) << 16 | cmd;

	dc = calloc(1, sizeof(*dc));
	if (!dc) {
		err(1, "failed to allocate diag command\n");
		return;
	}

	dc->first = key;
	dc->last = key;
	dc->cb = cb;

	list_add(&common_cmds, &dc->node);
}

int diag_add_apps_cmd_registrations(int fd,
			      unsigned char * buf, size_t len)
{
	struct diag_cmd_reg_tbl *pkt = (struct diag_cmd_reg_tbl*)buf;
	struct diag_cmd *dc;
	unsigned int subsys;
	unsigned int cmd;
	unsigned int first;
	unsigned int last;
	int i;
	(void)len;

	for (i = 0; i < pkt->count; i++) {
		cmd = pkt->entries[i].cmd;
		subsys = pkt->entries[i].subsys;
		if (cmd == DIAG_CMD_NO_SUBSYS_DISPATCH && subsys != DIAG_CMD_NO_SUBSYS_DISPATCH)
			cmd = DIAG_CMD_SUBSYS_DISPATCH;
		first = cmd << 24 | subsys << 16 | pkt->entries[i].first;
		last = cmd << 24 | subsys << 16 | pkt->entries[i].last;

		dc = malloc(sizeof(*dc));
		if (!dc) {
			warn("malloc failed");
			return -ENOMEM;
		}
		memset(dc, 0, sizeof(*dc));

		dc->first = first;
		dc->last = last;
		dc->fd = fd;

		list_add(&apps_cmds, &dc->node);
	}

	return 0;
}
int diag_remove_cmd_registrations(int fd)
{
	struct diag_cmd *dc;
	struct list_head *item;
	struct list_head *next;

	list_for_each_safe(item, next, &apps_cmds) {
		dc = container_of(item, struct diag_cmd, node);
		if (dc->fd == fd) {
			list_del(&dc->node);
			free(dc);
			dc = NULL;
		}
	}
	return 0;

}

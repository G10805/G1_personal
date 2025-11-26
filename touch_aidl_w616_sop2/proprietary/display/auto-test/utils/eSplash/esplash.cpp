/*--------------------------------------------------------------------------
 * Copyright (c) 2018-2021, 2023-2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Copyright (c) 2010 - 2013, 2016 - 2017, The Linux Foundation. All rights reserved.
 * Copyright (c) 2011 Benjamin Franzke
 * Copyright (c) 2010 Intel Corporation
 * Copyright 2008 Tungsten Graphics
 *   Jakob Bornecrantz <jakob@tungstengraphics.com>
 * Copyright 2008 Intel Corporation
 *   Jesse Barnes <jesse.barnes@intel.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
--------------------------------------------------------------------------*/
/* Copied from   https://github.com/AlphaPerfect/docs/blob/master/drm-howto/modeset-vsync.c */

/*________________________________________
  * Copyright 2007-8 Advanced Micro Devices, Inc. 
 * Copyright 2008 Red Hat Inc. 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions: 
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software. 
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR 
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
 * OTHER DEALINGS IN THE SOFTWARE. 
________________________________________

 * Not a contribution.

* Copyright 2016 Rockchip Electronics S.LSI Co. LTD 
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0 
 * 
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.*/


#define LOG_TAG "ESPLASH"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <vector>
#include <thread>
#include <utils/Log.h>
#include <drm/msm_drm.h>
#ifdef __MIN_ANDROID_VER_T__
#include <display/drm/sde_drm.h>
#else
#include <drm/sde_drm.h>
#endif
#include <linux/version.h>

#include "xf86drm.h"
#include "xf86drmMode.h"
#include "drm_fourcc.h"
#include "libdrm_macros.h"
#include "esplash.h"

#include <selinux/android.h>

#ifdef SILENT_BOOT_MIN_ANDROID_VER_U
#define SYS_PATH_SILENT_MODE      "/sys/kernel/silent_boot/pm_silentmode_kernel_state"
#else
#define SYS_PATH_SILENT_MODE      "/sys/power/pm_silentmode_kernel_state"
#endif

#define SYS_PATH_DISPLAY_READY    "/sys/devices/platform/soc/ae00000.qcom,mdss_mdp/init_complete"
/* Wait a maximum of 5 seconds for DRM Initialization */
#define MSMDRM_INIT_WAIT_TO_MS    5000
#define MSMDRM_INIT_SUCCESS       1
#define MAX_RETRY_COUNT           10
#define MAX_CARD                  16
#define MAX_DSI_CONNECTOR_WAIT    10
#define MAX_DP_CONNECTOR_WAIT     100

/*
 * global structures
 */
struct fb_obj {
	uint32_t fb_id;
	size_t size;
	size_t pitch;
	unsigned handle;
	void *ptr;
};

struct connector_config {
	uint32_t connector_id;
	uint32_t crtc_id;
	uint32_t crtc_idx;
	uint32_t width;
	uint32_t height;

	uint32_t mode_id;
	drmModeModeInfo mode;

	uint32_t crtc_id_pid;
	uint32_t active_pid;
	uint32_t mode_id_pid;
};

struct plane_config {
	uint32_t plane_id;
	uint32_t crtc_id;
	uint32_t connector_id;

	fb_obj fb[MAX_BUFFER];

	int x_offset;
	int x_increment;
	bool first_run;
	bool handoff_init;
	uint32_t possible_crtcs;

	int x;
	int y;
	int w;
	int h;
	int zpos;
	int format;

	/* optional */
	int handoff;
	bool handoff_set;

	/* properties */
	uint32_t crtc_pid;
	uint32_t fb_pid;
	uint32_t zpos_pid;
	uint32_t crtc_x_pid;
	uint32_t crtc_y_pid;
	uint32_t crtc_w_pid;
	uint32_t crtc_h_pid;
	uint32_t src_x_pid;
	uint32_t src_y_pid;
	uint32_t src_w_pid;
	uint32_t src_h_pid;
	uint32_t handoff_pid;
};

/*
 * global variables
 */
static std::mutex plane_mutex;
static std::vector<plane_config> plane_cfg;
static std::vector<connector_config> connector_cfg;
static int fd = -1;
static int step1[MAX_COUNT] = {0};
static int avail_connector = 0;

static const char *connector_type_names[] = {
	"unknown",
	"VGA",
	"DVI-I",
	"DVI-D",
	"DVI-A",
	"composite",
	"s-video",
	"LVDS",
	"component",
	"9-pin DIN",
	"DP",
	"HDMI-A",
	"HDMI-B",
	"TV",
	"eDP",
	"Virtual",
	"DSI",
};


/*
 * local functions
 */

/****************************************************
 * format_to_bpp()
 * description: converts format to bpp
 ***************************************************/
static int format_to_bpp(uint32_t format)
{
	switch (format) {

	case DRM_FORMAT_UYVY:
		return 16;
	case DRM_FORMAT_NV12:
		return 12;
	case DRM_FORMAT_RGB888:
		return 24;
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_RGBA8888:
	default:
		return 32;
	}
}

/****************************************************
 * place_marker()
 * description: print boot kpi marker
 ***************************************************/
static void place_marker(char const *name)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0)) || defined(USE_BOOT_MARKER)
	int fd = open("/sys/kernel/boot_kpi/kpi_values", O_WRONLY);

	if (fd > 0)
	{
		char earlyapp[128] = {0};
		strlcat(earlyapp, name, sizeof(earlyapp));
		write(fd, earlyapp, strlen(earlyapp));
		close(fd);
	}
#else
	FILE* fd = freopen("/dev/kmsg", "w", stdout);
	if(fd != NULL)
	{
		printf("boot_kpi: %s\n", name);
		fclose(fd);
	}
#endif
}

/****************************************************
 * update_possible_crtcs()
 * description: update the possible crtc for planes
 ***************************************************/
static void update_possible_crtcs(void)
{
	uint32_t j;

	plane_mutex.lock();

	/* update possible_crtcs on errors */
	for (j = 0; j < plane_cfg.size(); j++) {
		drmModePlanePtr plane = drmModeGetPlane(fd,
						plane_cfg[j].plane_id);

		if (plane &&
			plane_cfg[j].possible_crtcs != plane->possible_crtcs) {

			DEBUGT_PRINT_INFO("plane%d possible_crtcs 0x%x => 0x%x\n",
					plane_cfg[j].plane_id,
					plane_cfg[j].possible_crtcs,
					plane->possible_crtcs);

			plane_cfg[j].possible_crtcs = plane->possible_crtcs;
			plane_cfg[j].first_run = true;
		}
		drmModeFreePlane(plane);
	}

	plane_mutex.unlock();
}

/****************************************************
 * parse_display()
 * description: parse all the display resources
 ***************************************************/
static int parse_display(void)
{
	int count_connectors = 0;
	int count_crtcs = 0;
	int count_planes = 0;
	int i = 0;
	int j = 0;
	int conn_wait = 0;
	connector_config cfg[MAX_COUNT] = {};
	plane_config cfg_plane = {};
	drmModePlaneRes *plane_res = NULL;
	drmModeRes *res = NULL;

	/* Get information about connectors */
	res = drmModeGetResources(fd);
	if (!res)
		goto out;

	for (i = 0; i < res->count_connectors; i++) {
		uint32_t crtc_mask = 0;
		uint32_t retry_cnt = 0;
retry:
		drmModeConnectorPtr conn = drmModeGetConnector(fd,
							res->connectors[i]);

		if (!conn || !conn->encoders)
			goto out;

		/* DP connector availability takes more
		 * time compared to DSI connector, both has different
		 * wait time.
		 */
		if (conn->connector_type == DRM_MODE_CONNECTOR_DisplayPort)
			conn_wait = MAX_DP_CONNECTOR_WAIT;
		else
			conn_wait = MAX_DSI_CONNECTOR_WAIT;

		if (conn->connection != DRM_MODE_CONNECTED &&
			retry_cnt++ < MAX_RETRY_COUNT) {
			usleep(conn_wait * 1000);
			goto retry;
		} else if (!(retry_cnt < MAX_RETRY_COUNT)) {
			DEBUGT_PRINT_ERROR("connector%d: status: not connected",
					conn->connector_id);
			continue;
		} else {
			avail_connector++;
		}

		/* dump */
		DEBUGT_PRINT_ERROR("connector%d: %d - %s-%d crtcs:",
				count_connectors, conn->connector_id,
				connector_type_names[conn->connector_type],
				conn->connector_type_id);

		for (j = 0; j < conn->count_encoders; j++) {
			drmModeEncoderPtr enc = drmModeGetEncoder(fd,
							conn->encoders[j]);
			if(!enc)
				goto out;

			crtc_mask |= enc->possible_crtcs;
		}

		for (j = 0; j < res->count_crtcs; j++) {
			if (crtc_mask & (1 << j))
				DEBUGT_PRINT_ERROR(" %d", res->crtcs[j]);
		}

		DEBUGT_PRINT_ERROR("\tmode:");

		for (j = 0; j < conn->count_modes; j++) {
			DEBUGT_PRINT_ERROR(" %s", conn->modes[j].name);
		}

		DEBUGT_PRINT_ERROR("\n");

		cfg[count_connectors].connector_id = conn->connector_id;

		/* store property id */
		drmModeObjectPropertiesPtr props =
					drmModeObjectGetProperties(fd,
					conn->connector_id,
					DRM_MODE_OBJECT_CONNECTOR);

		if (!props)
			goto out;

		for (j = 0; j < (int)props->count_props; j++) {
			drmModePropertyPtr prop = drmModeGetProperty(fd,
							props->props[j]);
			if (!prop)
				goto out;

			if (!strcmp(prop->name, "CRTC_ID"))
				cfg[count_connectors].crtc_id_pid =
							prop->prop_id;
		}

		/* copy mode */
		if (!conn->modes) {
			drmModeFreeConnector(conn);
			goto out;
		}

		memcpy(&cfg[count_connectors].mode,
				&conn->modes[0], sizeof(drmModeModeInfo));

		drmModeFreeConnector(conn);
		count_connectors++;
	}

	/* Get information about crtcs */
	for (i = 0; i < res->count_crtcs; i++) {
		drmModeCrtcPtr crtc = drmModeGetCrtc(fd, res->crtcs[i]);
		if (!crtc || !res->connectors)
			goto out;

		drmModeConnectorPtr conn = drmModeGetConnector(fd,
							res->connectors[i]);
		if (!conn)
			goto out;

		if (conn->connection != DRM_MODE_CONNECTED)
			continue;

		cfg[count_crtcs].crtc_id = crtc->crtc_id;

		/* store property id */
		drmModeObjectPropertiesPtr props =
					drmModeObjectGetProperties(fd,
						crtc->crtc_id,
						DRM_MODE_OBJECT_CRTC);
		if (!props)
			goto out;

		for (j = 0; j < (int)props->count_props; j++) {
			drmModePropertyPtr prop = drmModeGetProperty(fd,
							props->props[j]);
			if (!prop)
				goto out;

			if (!strcmp(prop->name, "ACTIVE"))
				cfg[count_crtcs].active_pid = prop->prop_id;
			else if (!strcmp(prop->name, "MODE_ID"))
				cfg[count_crtcs].mode_id_pid = prop->prop_id;
		}

		/* store crtc index */
		cfg[count_crtcs].crtc_idx = count_crtcs;
		connector_cfg.push_back(cfg[count_crtcs]);

		drmModeFreeCrtc(crtc);
		drmModeFreeConnector(conn);
		count_crtcs++;
	}

	/*Get information about planes*/
	plane_res = drmModeGetPlaneResources(fd);
	if(!plane_res)
		goto out;

	for (i = 0; i < (int)plane_res->count_planes; i++) {
		int handoff;
		drmModePlanePtr plane = drmModeGetPlane(fd,
						plane_res->planes[i]);
		if (!plane || !res->connectors)
			goto out;

		drmModeConnectorPtr conn = drmModeGetConnector(fd,
						res->connectors[i]);
		if (!conn)
			goto out;

		if (conn->connection != DRM_MODE_CONNECTED)
			continue;

		/* dump */
		DEBUGT_PRINT_ERROR("plane%d: %d - crtcs:", i, plane->plane_id);

		for (j = 0; j < res->count_crtcs; j++) {
			if (plane->possible_crtcs & (1 << j) && res->crtcs)
				DEBUGT_PRINT_ERROR(" %d", res->crtcs[j]);
		}
		DEBUGT_PRINT_ERROR("\n");

		cfg_plane.plane_id = plane->plane_id;
		cfg_plane.format = DRM_FORMAT_ARGB8888;	

		/* store property id */
		drmModeObjectPropertiesPtr props =
					drmModeObjectGetProperties(fd,
						plane->plane_id,
						DRM_MODE_OBJECT_PLANE);
		if (!props)
			goto out;

		for (j = 0; j < (int)props->count_props; j++) {
			drmModePropertyPtr prop = drmModeGetProperty(fd,
							props->props[j]);
			if (!prop)
				goto out;

			if (!strcmp(prop->name, "FB_ID"))
				cfg_plane.fb_pid = prop->prop_id;
			else if (!strcmp(prop->name, "CRTC_ID"))
				cfg_plane.crtc_pid = prop->prop_id;
			else if (!strcmp(prop->name, "zpos"))
				cfg_plane.zpos_pid = prop->prop_id;
			else if (!strcmp(prop->name, "CRTC_X"))
				cfg_plane.crtc_x_pid = prop->prop_id;
			else if (!strcmp(prop->name, "CRTC_Y"))
				cfg_plane.crtc_y_pid = prop->prop_id;
			else if (!strcmp(prop->name, "CRTC_W"))
				cfg_plane.crtc_w_pid = prop->prop_id;
			else if (!strcmp(prop->name, "CRTC_H"))
				cfg_plane.crtc_h_pid = prop->prop_id;
			else if (!strcmp(prop->name, "SRC_X"))
				cfg_plane.src_x_pid = prop->prop_id;
			else if (!strcmp(prop->name, "SRC_Y"))
				cfg_plane.src_y_pid = prop->prop_id;
			else if (!strcmp(prop->name, "SRC_W"))
				cfg_plane.src_w_pid = prop->prop_id;
			else if (!strcmp(prop->name, "SRC_H"))
				cfg_plane.src_h_pid = prop->prop_id;
			else if (!strcmp(prop->name, "handoff")) {
				cfg_plane.handoff_pid = prop->prop_id;
				handoff = props->prop_values[j];
			}
		}

		/* update possible_crtcs */
		cfg_plane.possible_crtcs = plane->possible_crtcs;

		if (plane->possible_crtcs & (1 << i) && res->crtcs)
			cfg_plane.crtc_id = res->crtcs[i];

		/* dump handoff status */
		if (cfg_plane.handoff_pid)
			DEBUGT_PRINT_ERROR("\thandoff=%d possible_crtcs=0x%x\n",
					handoff, plane->possible_crtcs);
		cfg_plane.x = plane->x;
		cfg_plane.y = plane->y;
		cfg_plane.h = connector_cfg[count_planes].mode.vdisplay;
		cfg_plane.w = connector_cfg[count_planes].mode.hdisplay;

		plane_cfg.push_back(cfg_plane);

		drmModeFreePlane(plane);
		drmModeFreeConnector(conn);
		count_planes++;
	}
	drmModeFreePlaneResources(plane_res);
	return 0;
out:
	if (res)
		drmModeFreeResources(res);

	if (plane_res)
		drmModeFreePlaneResources(plane_res);

	return ERR_NO;

}

/****************************************************
 * fill_buffer()
 * description: fill the allocated buffers with content
 ***************************************************/
static void fill_buffer(int id, int width, int height, int Bpp)
{
	uint8_t *row_start = (uint8_t*) plane_cfg[0].fb[id].ptr;
	char *ptr = NULL;
	int roi_w = LOGO_WIDTH;
	int roi_h = LOGO_HEIGHT;
	int roi_y = (height - roi_h) / 2;
	int step = LOGO_WIDTH / MAX_BUFFER;
	char *logo_ptr = (char *) logo_map;
	int roi_x = ((width / 2)  -  LOGO_WIDTH) + step * 2 * id;

	memset(row_start, 0x0, (width * height * Bpp));

	ptr = (char *) row_start + (roi_x + roi_y * width) * Bpp;

	for (int i = 0; i < roi_h ; i++) {
		ptr = (char *) row_start + (roi_x + (roi_y + i) * width) * Bpp;
		for (int j = 0; j < roi_w; j++) {
			ptr[0] = *logo_ptr++;
			ptr[1] = *logo_ptr++;
			ptr[2] = *logo_ptr++;
			ptr[3] = *logo_ptr++;
			ptr += Bpp;
		}
	}
	return;
}

/****************************************************
 * create_fb()
 * description: create, map and add the dumb buffer
 ***************************************************/
static int create_fb()
{
	int i, j;
	int ret = 0;
	int width = 0;
	int height = 0;
	int Bpp = 0;

	/* create framebuffer */
	for(j = 0; j < MAX_BUFFER; j++) {
		struct drm_mode_create_dumb create_arg;

		memset(&create_arg, 0, sizeof(create_arg));

		create_arg.bpp = format_to_bpp(plane_cfg[0].format);

		for (i = 0; i < (int)plane_cfg.size(); i++) {
			width = width > plane_cfg[i].w ? width: plane_cfg[i].w;
			height = height > plane_cfg[i].h ? height: plane_cfg[i].h;
		}

		create_arg.width = width;
		create_arg.height = height;

		ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_arg);
		if (ret) {
			DEBUGT_PRINT_ERROR("failed to create dumb buffer: %s\n",
							strerror(errno));
			return ret;
		}

		struct drm_mode_map_dumb map_arg;

		memset(&map_arg, 0, sizeof(map_arg));
		map_arg.handle = create_arg.handle;

		ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map_arg);
		if (ret) {
			DEBUGT_PRINT_ERROR("failed to map dumb buffer: %s\n",
							strerror(errno));
			return ret;
		}

		for (i = 0; i < (int)plane_cfg.size(); i++) {
			plane_cfg[i].fb[j].handle = create_arg.handle;
			plane_cfg[i].fb[j].size = create_arg.size;
			plane_cfg[i].fb[j].pitch = create_arg.pitch;
			plane_cfg[i].fb[j].ptr = drm_mmap(0, create_arg.size,
							PROT_READ | PROT_WRITE,
							MAP_SHARED, fd,
							map_arg.offset);

			if (plane_cfg[i].fb[j].ptr == MAP_FAILED) {
				DEBUGT_PRINT_ERROR("failed to map buffer: %s\n",
							strerror(errno));
				return ret;
			}
		}

		Bpp = create_arg.bpp / 8;

		fill_buffer(j, width, height, Bpp);	

		/* flush cache */
		for (i = 0; i < (int)plane_cfg.size(); i++)
			drmUnmap(plane_cfg[i].fb[j].ptr,
					plane_cfg[i].fb[j].size);

		/* create fbid */
		struct drm_mode_fb_cmd2 cmd2[MAX_BUFFER] {};

		cmd2[j].width = create_arg.width;
		cmd2[j].height = create_arg.height;
		cmd2[j].pixel_format = DRM_FORMAT_ARGB8888;
		cmd2[j].flags = DRM_MODE_FB_MODIFIERS;
		cmd2[j].handles[0] = create_arg.handle;
		cmd2[j].pitches[0] = create_arg.pitch;
		cmd2[j].offsets[0] = 0;
		cmd2[j].modifier[0] = 0;

		ret = drmIoctl(fd, DRM_IOCTL_MODE_ADDFB2, &cmd2[j]);
		if (ret) {
			DEBUGT_PRINT_ERROR("failed to addfb2: %s\n",
							strerror(errno));
			return ret;
		}

		for (i = 0; i < (int)plane_cfg.size(); i++)
			plane_cfg[i].fb[j].fb_id = cmd2[j].fb_id;
	}

	/* first run flag */
	for (i = 0; i < (int)plane_cfg.size(); i++)
		plane_cfg[i].first_run = true;

	return ret;
}

/****************************************************
 * setup_connector()
 * description: setup the connector configuration
 ***************************************************/
static int setup_connector(drmModeAtomicReqPtr req)
{
	int i = 0;
	int ret = 0;

	for (i = 0; i < (int)connector_cfg.size(); i++) {
		/* set mode */
		uint32_t blob_id;

		if (drmModeCreatePropertyBlob(fd,
					(const void *) &connector_cfg[i].mode,
					sizeof(drmModeModeInfo), &blob_id)) {
			DEBUGT_PRINT_ERROR("failed to create mode blob\n");
			return 0;
		}

		drmModeAtomicAddProperty(req, connector_cfg[i].crtc_id,
					connector_cfg[i].mode_id_pid, blob_id);

		/* set active */
		drmModeAtomicAddProperty(req, connector_cfg[i].crtc_id,
					connector_cfg[i].active_pid, 1);

		/* set crtc */
		drmModeAtomicAddProperty(req, connector_cfg[i].connector_id,
					connector_cfg[i].crtc_id_pid,
					connector_cfg[i].crtc_id);

		/* init commit */
		ret = drmModeAtomicCommit(fd, req,
					DRM_MODE_ATOMIC_ALLOW_MODESET, 0);
		if (ret) {
			DEBUGT_PRINT_ERROR("init commit failed: %s\n",
							strerror(errno));
			return ret;
		}

		DEBUGT_PRINT_INFO("init commit\n");

		/* clear previous commit */
		drmModeAtomicSetCursor(req, 0);
	}
	return ret;
}

/****************************************************
 * update_fb()
 * description: update the plane with new framebuffer
 ***************************************************/
static void update_fb(drmModeAtomicReqPtr req, int connector_count)
{
	int j;

	for (j = 0; j < (int)plane_cfg.size(); j++) {
		if (plane_cfg[j].crtc_id ==
			connector_cfg[connector_count].crtc_id) {
			/* update plane in locked context */
			std::lock_guard<std::mutex> guard(plane_mutex);

			/* check possible_crtcs */
			if (!(plane_cfg[j].possible_crtcs &
				(1 << connector_cfg[connector_count].crtc_idx)))
				continue;

			/* first run */
			if (plane_cfg[j].first_run) {
				drmModeAtomicAddProperty(req,
						plane_cfg[j].plane_id,
						plane_cfg[j].zpos_pid,
						plane_cfg[j].zpos);
				drmModeAtomicAddProperty(req,
						plane_cfg[j].plane_id,
						plane_cfg[j].crtc_x_pid,
						plane_cfg[j].x);
				drmModeAtomicAddProperty(req,
						plane_cfg[j].plane_id,
						plane_cfg[j].crtc_y_pid,
						plane_cfg[j].y);
				drmModeAtomicAddProperty(req,
						plane_cfg[j].plane_id,
						plane_cfg[j].crtc_w_pid,
						plane_cfg[j].w);
				drmModeAtomicAddProperty(req,
						plane_cfg[j].plane_id,
						plane_cfg[j].crtc_h_pid,
						plane_cfg[j].h);
				drmModeAtomicAddProperty(req,
						plane_cfg[j].plane_id,
						plane_cfg[j].src_y_pid,
						0 << 16);
				drmModeAtomicAddProperty(req,
						plane_cfg[j].plane_id,
						plane_cfg[j].src_w_pid,
						plane_cfg[j].w << 16);
				drmModeAtomicAddProperty(req,
						plane_cfg[j].plane_id,
						plane_cfg[j].src_h_pid,
						plane_cfg[j].h << 16);

				plane_cfg[j].first_run = false;

				/* only set handoff once */
				if (plane_cfg[j].handoff_set &&
					plane_cfg[j].handoff_pid) {
					drmModeAtomicAddProperty(req,
							plane_cfg[j].plane_id,
							plane_cfg[j].handoff_pid,
							plane_cfg[j].handoff);
					plane_cfg[j].handoff_set = false;
				}
			}

			/*update crtc_id and plane_id*/
			drmModeAtomicAddProperty(req,
					plane_cfg[j].plane_id,
					plane_cfg[j].crtc_pid,
					plane_cfg[j].crtc_id);
			/* update y_offset */
			drmModeAtomicAddProperty(req,
					plane_cfg[j].plane_id,
					plane_cfg[j].fb_pid,
					plane_cfg[j].fb[step1[j]].fb_id);

			/* update x_offset for animation */
			if (step1[j] + plane_cfg[j].x_increment == MAX_BUFFER)
				plane_cfg[j].x_increment = -1;
			else if (step1[j] + plane_cfg[j].x_increment == 0)
				plane_cfg[j].x_increment = 1;

			step1[j] += plane_cfg[j].x_increment;
		}
	}
}

/****************************************************
 * destroy_buff()
 * description: cleanup the allocated dumb buffers
 ***************************************************/
static int destroy_buff()
{
	int k, ret = 0;

	for(k = 0; k < MAX_BUFFER; k++) {
		struct drm_mode_destroy_dumb destroy_arg;

		memset(&destroy_arg, 0, sizeof(destroy_arg));
		destroy_arg.handle = plane_cfg[0].fb[k].handle;

		ret = drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy_arg);
		if (ret) {
			DEBUGT_PRINT_ERROR("failed to destroy dumb buffer %s\n",
							strerror(errno));
			return ret;
		}
	}
	return ret;
}

/****************************************************
 * fetch_silent_mode_status()
 * description: check the silent boot status
 ***************************************************/
static uint8_t fetch_silent_mode_status()
{
	unsigned char buf;
	int silent_mode_fd;
	uint8_t curr_silent_mode = 0;
	int bytes_read;

	silent_mode_fd = open(SYS_PATH_SILENT_MODE, O_RDONLY, 0);
	if (silent_mode_fd < 0) {
		DEBUGT_PRINT_INFO("%s: Failed to open silent mode file: %s", __func__,
			strerror(errno));
		return 0;
	}

	bytes_read = read(silent_mode_fd, (void*)&buf, sizeof(unsigned char));
	if (bytes_read < 0)
		DEBUGT_PRINT_ERROR("%s: Error reading silent mode file: %s", __func__,
			strerror(errno));
	else
		curr_silent_mode = buf - '0';

	close(silent_mode_fd);

	return curr_silent_mode;
}

/***************************************************
 * msm_drv_init_complete()
 * description: read the init_complete value in msm_drm
 ***************************************************/
static int msm_drv_init_complete()
{
	int fd = -1, rc = -1;
	unsigned char buf = 0;

	fd = open(SYS_PATH_DISPLAY_READY, O_RDONLY, 0);
	if (fd < 0) {
		DEBUGT_PRINT_ERROR("%s: Failed to open init_complete status: %s",
			__func__, strerror(errno));
		return fd;
	}

	rc = read(fd, (void*)&buf, sizeof(unsigned char));
	if (rc < 0) {
		DEBUGT_PRINT_ERROR("%s: Error reading init_complete status: %s",
			__func__, strerror(errno));
		return rc;
	} else {
		rc = buf - '0';
	}

	close(fd);
	return rc;
}

/***************************************************
 * is_display_ready()
 * description: check the msm_drm init_complete status
 ***************************************************/
static int is_display_ready()
{
	clock_t tstart = 0;
	clock_t tnow = 0;
	double telapsed = 0;
	int rc = -1;

	/* wait for msm drivers intialization to complete */
	tstart = clock();
	while (rc != MSMDRM_INIT_SUCCESS) {
		rc = msm_drv_init_complete();
		/* Break when time in process exceeds MSMDRM_INIT_WAIT_TO_MS */
		tnow = clock();
		telapsed = (double)(tnow - tstart) / CLOCKS_PER_SEC * 1000;
		if (telapsed > MSMDRM_INIT_WAIT_TO_MS)
			break;

		/* sleep for 50ms, so loop is not tight */
		usleep(10 * 1000);
	}

	if (telapsed > MSMDRM_INIT_WAIT_TO_MS) {
		place_marker("K - Early Dynamic Splash : Not Ready");
		return 0;
	}

	place_marker("K - Early Dynamic Splash : Ready");

	return rc;
}

int main()
{
	/* dri card */
	int i = 0,j = 0;
	int ret = 0;
	char value[PROP_VALUE_MAX] = "";
	char boot_complete[PROP_VALUE_MAX] = "";
	int counter = 200;
	int commit_fail = 0;
	char card_path[256];
	drmModeAtomicReqPtr req;
	drmVersionPtr version;
	int context_initialized = -1;
	int commit_cnt = 3000;
	bool match = false;

	if (fetch_silent_mode_status())
		return 0;

	if (is_display_ready() != MSMDRM_INIT_SUCCESS)
		return 0;

	for (int card = 0; card < MAX_CARD; card++) {
#ifdef __EARLYSERVICES__
		snprintf(card_path, sizeof(card_path), "/early_services/dev/dri/card%d", card);
#else
		snprintf(card_path, sizeof(card_path), "/dev/dri/card%d", card);
#endif
		fd = open(card_path, O_RDWR, 0);
		if (fd < 0)
			continue;

		version = drmGetVersion(fd);
		if (version) {
			match = !strcmp(version->name, "msm_drm3");
			drmFreeVersion(version);
			if (match)
				break;
		}
		close(fd);
	}

	if (fd < 0) {
		DEBUGT_PRINT_ERROR("failed to open %s\n", card_path);
		return 0;
	}

	drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
	drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);

	ret = parse_display();
	if (ret) {
		DEBUGT_PRINT_ERROR("parse_display failed\n");
		return 0;
	}

	/*create framebuffer*/
	ret = create_fb();
	if (ret) {
		DEBUGT_PRINT_ERROR("failed to create framebuffer\n");
		return ret;
	}

	/* init atomic commit */
	req = drmModeAtomicAlloc();
	if (!req) {
		DEBUGT_PRINT_ERROR("failed to init atomic commit\n");
		return 0;
	}

	/* setup crtc / connector */
	ret = setup_connector(req);
	if (ret) {
		DEBUGT_PRINT_ERROR("failed to setup connector\n");
		return ret;
	}

	place_marker("K - Early Dynamic Splash");

	/* animation loop */
	while (1) {
		/* exit the animation loop if
		 * 1. Bootanimation Starts or
		 * 2. Boot is completed or
		 * 3. Atomic commit is failed on connectors
		 * 4. If maximum number of frames are committed
		 */
		if (commit_fail < avail_connector) {
			if (context_initialized == -1) {
				context_initialized =
					selinux_android_setcon(
						"u:r:vendor_esplash:s0");
			}

			__system_property_get("vendor.init.svc.bootanim",
						value);
			__system_property_get("vendor.sys.boot_completed",
						boot_complete);

			if(!(strncmp(value,"running",PROP_VALUE_MAX)) ||
				!(strncmp(boot_complete,"1",PROP_VALUE_MAX)))
				counter--;
		}

		/* update per crtc */
		for (i = 0; i < (int) connector_cfg.size(); i++) {
			uint32_t flags = DRM_MODE_ATOMIC_ALLOW_MODESET;

			/* update fb */
			update_fb(req, i);

			/* asynchronous commit */
			flags |= DRM_MODE_ATOMIC_NONBLOCK;

			ret = drmModeAtomicCommit(fd, req, flags, 0);
			if (ret) {
				DEBUGT_PRINT_INFO("commit failed: %s\n",
							strerror(errno));
				update_possible_crtcs();
				commit_fail++;

				if (commit_fail == avail_connector) {
					counter = 0;
					break;
				}

				counter += 100;
				/* adding number of frames to commit */
				commit_cnt++;
			} else {
				/* reducing number of frames to commit */
				commit_cnt--;
			}

			/* clear previous commit */
			drmModeAtomicSetCursor(req, 0);

		}

		/* sleep for 15ms, simulating 60fps */
		usleep(15 * 1000);

		if (!counter)
			break;

		if (!commit_cnt) {
			DEBUGT_PRINT_ERROR("exit animation loop as max frame commits count exceeded\n");
			break;
		}
	} /* animation loop ends */

	place_marker("K - Early Dynamic Splash - Exit");


        for (i = 0; i < (int)connector_cfg.size(); i++) {
            for (j = 0; j < (int)plane_cfg.size(); j++) {
                if (plane_cfg[j].crtc_id == connector_cfg[i].crtc_id) {
                    /* check possible_crtcs */
                    if (!(plane_cfg[j].possible_crtcs &
                                (1 << connector_cfg[i].crtc_idx)))
                        continue;

                    /* clear previous commit */
                    drmModeAtomicSetCursor(req, 0);
                    drmModeAtomicAddProperty(req,
                            plane_cfg[j].plane_id,
                            plane_cfg[j].crtc_pid,
                            0);

                    drmModeAtomicAddProperty(req,
                            plane_cfg[j].plane_id,
                            plane_cfg[j].fb_pid,
                            0);
                    ret = drmModeAtomicCommit(fd, req,
                            DRM_MODE_ATOMIC_ALLOW_MODESET, 0);
                    if (ret) {
                        DEBUGT_PRINT_ERROR("last commit failed: %s\n",
                                strerror(errno));
                    }
                }
            }

        }
	for (i = 0; i < (int)connector_cfg.size(); i++) {
		/* clear previous commit */
		drmModeAtomicSetCursor(req, 0);

		/* set inactive */
		drmModeAtomicAddProperty(req, connector_cfg[i].crtc_id,
					connector_cfg[i].active_pid,
					0);
		/* last commit */
		ret = drmModeAtomicCommit(fd, req,
					DRM_MODE_ATOMIC_ALLOW_MODESET, 0);
		if (ret)
			DEBUGT_PRINT_ERROR("last commit failed: %s\n",
							strerror(errno));
	}

	/* clear commit */
	drmModeAtomicFree(req);

	/* destroy buffer */
	ret = destroy_buff();
	if (ret)
		return ret;

	/* close fd */
	close(fd);

	return ret;
}

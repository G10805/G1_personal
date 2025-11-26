/*--------------------------------------------------------------------------
 * Copyright (c) 2018-2022 Qualcomm Technologies, Inc.
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
#ifdef __ANDROID_T_AND_ABOVE__
#include <display/drm/sde_drm.h>
#else
#include <drm/msm_drm.h>
#include <drm/sde_drm.h>
#endif

#include "xf86drm.h"
#include "xf86drmMode.h"
#include "drm_fourcc.h"
#include "libdrm_macros.h"
#include "VideoPlatform.h"

struct fb_obj {
   uint32_t fb_id;
   size_t size;
   size_t pitch;
   size_t offset;
   unsigned handle;
   void *ptr;
   void *plane[2];
   int ion_fd;
   int ion_map_fd;
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

int parse_display(void);
int create_fb(uint32_t width, uint32_t height);
int setup_connector(drmModeAtomicReqPtr req);
void update_fb(drmModeAtomicReqPtr req, int connector_count,int buf_idx);
void update_possible_crtcs(void);
int destroy_buff();

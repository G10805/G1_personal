/*--------------------------------------------------------------------------
 * Copyright (c) 2019-2024 Qualcomm Technologies, Inc.
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


#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <vector>
#include <pthread.h>
#include <drm/msm_drm.h>
#include <linux/version.h>
#ifdef __MIN_ANDROID_VER_T__
#include <display/drm/sde_drm.h>
#else
#include <drm/sde_drm.h>
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,15,0)
#ifdef __MIN_ANDROID_VER_T__
#include <display/media/mmm_color_fmt.h>
#else
#include <media/mmm_color_fmt.h>
#endif
#else
#include <media/msm_media_info.h>
#endif

#include "xf86drm.h"
#include "xf86drmMode.h"
#include "drm_fourcc.h"
#include "libdrm_macros.h"
#include "drm-test.h"

#ifdef DEBUG_BUILD
#include "snapdragon.h"
#include "chip_rgb888.h"
#include "image_720p.h"
#include "image_1080p.h"
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,15,0)
#define USE_DMA_HEAP
#else
#define USE_ION
#endif

#ifdef USE_ION
#include <fcntl.h>
#include <unistd.h>
#include <linux/msm_ion.h>
#include <ion/ion.h>
#endif

#ifdef USE_DMA_HEAP
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>

#define DEFAULT_HEAP "/dev/dma_heap/qcom,system"
#endif

#define ROI_MISR_MAX_ROIS_PER_MISR 4

#define ROI_MISR_MAX_MISRS_PER_CRTC 6

#define ROI_MISR_MAX_ROIS_PER_CRTC \
	(ROI_MISR_MAX_MISRS_PER_CRTC * ROI_MISR_MAX_ROIS_PER_MISR)

typedef enum {
	 IMAGE_FLAG_NONE = 0,
	 IMAGE_FLAG_UBWC = 1,
} IMAGE_FLAG;

//#define QUERY_ROI_MISR_IN_THREAD

#define SDE_CSC_MATRIX_COEFF_SIZE 9
#define SDE_CSC_CLAMP_SIZE 6
#define SDE_CSC_BIAS_SIZE 3
struct drm_csc_v1 {
	int64_t ctm_coeff[SDE_CSC_MATRIX_COEFF_SIZE];
	uint32_t pre_bias[SDE_CSC_BIAS_SIZE];
	uint32_t post_bias[SDE_CSC_BIAS_SIZE];
	uint32_t pre_clamp[SDE_CSC_CLAMP_SIZE];
	uint32_t post_clamp[SDE_CSC_CLAMP_SIZE];
};
enum DRMCscType {
	kCscYuv2Rgb601L = 1,
	kCscYuv2Rgb601FR,
	kCscYuv2Rgb709L,
	kCscYuv2Rgb709FR,
	kCscYuv2Rgb2020L,
	kCscYuv2Rgb2020FR,
	kCscTypeMax,
};

static struct drm_csc_v1 csc_10bit_convert[kCscTypeMax] = {
	[kCscYuv2Rgb601L] = {
		{
			0x12A000000,
			0x000000000,
			0x198800000,
			0x12A000000,
			0x7F9B800000,
			0x7F30000000,
			0x12A000000,
			0x204800000,
			0x000000000,
		},
		{
			0xffc0,
			0xfe00,
			0xfe00,
		},
		{
			0x0,
			0x0,
			0x0,
		},
		{
			0x40,
			0x3ac,
			0x40,
			0x3c0,
			0x40,
			0x3c0,
		},
		{
			0x0,
			0x3ff,
			0x0,
			0x3ff,
			0x0,
			0x3ff,
		},
	},
	[kCscYuv2Rgb601FR] = {
		{
			0x100000000,
			0x0,
			0x167000000,
			0x100000000,
			0x7fa8000000,
			0x7f49000000,
			0x100000000,
			0x1c5800000,
			0x0,
		},
		{
			0x0000,
			0xfe00,
			0xfe00,
		},
		{
			0x0,
			0x0,
			0x0,
		},
		{
			0x0,
			0x3ff,
			0x0,
			0x3ff,
			0x0,
			0x3ff,
		},
		{
			0x0,
			0x3ff,
			0x0,
			0x3ff,
			0x0,
			0x3ff,
		},
	},
	[kCscYuv2Rgb709L] = {
		{
			0x12a000000,
			0x0,
			0x1cb000000,
			0x12a000000,
			0x7fc9800000,
			0x7f77800000,
			0x12a000000,
			0x21d000000,
			0x0,
		},
		{
			0xffc0,
			0xfe00,
			0xfe00,
		},
		{
			0x0,
			0x0,
			0x0,
		},
		{
			0x40,
			0x3ac,
			0x40,
			0x3c0,
			0x40,
			0x3c0,
		},
		{
			0x0,
			0x3ff,
			0x0,
			0x3ff,
			0x0,
			0x3ff,
		},
	},
	[kCscYuv2Rgb709FR] = {
		{
			0x100000000,
			0x0,
			0x193000000,
			0x100000000,
			0x7fd0000000,
			0x7f88000000,
			0x100000000,
			0x1db000000,
			0x0,
		},
		{
			0x0,
			0xfe00,
			0xfe00,
		},
		{
			0x0,
			0x0,
			0x0,
		},
		{
			0x0,
			0x3ff,
			0x0,
			0x3ff,
			0x0,
			0x3ff,
		},
		{
			0x0,
			0x3ff,
			0x0,
			0x3ff,
			0x0,
			0x3ff,
		},
	},
	[kCscYuv2Rgb2020L] = {
		{
			0x12b000000,
			0x0,
			0x1af000000,
			0x12b000000,
			0x7fd0000000,
			0x7f59000000,
			0x12b000000,
			0x226000000,
			0x0,
		},
		{
			0xffc0,
			0xfe00,
			0xfe00,
		},
		{
			0x0,
			0x0,
			0x0,
		},
		{
			0x40,
			0x3ac,
			0x40,
			0x3c0,
			0x40,
			0x3c0,
		},
		{
			0x0,
			0x3ff,
			0x0,
			0x3ff,
			0x0,
			0x3ff,
		},
	},
	[kCscYuv2Rgb2020FR] = {
		{
			0x100000000,
			0x0,
			0x179800000,
			0x100000000,
			0x7fd6000000,
			0x7f6d800000,
			0x100000000,
			0x1e1800000,
			0x0,
		},
		{
			0x0000,
			0xfe00,
			0xfe00,
		},
		{
			0x0,
			0x0,
			0x0,
		},
		{
			0x0,
			0x3ff,
			0x0,
			0x3ff,
			0x0,
			0x3ff,
		},
		{
			0x0,
			0x3ff,
			0x0,
			0x3ff,
			0x0,
			0x3ff,
		},
	},
};

struct roi_misr_config {
	int num_rois;
	struct drm_clip_rect roi_rects[ROI_MISR_MAX_ROIS_PER_CRTC];
	uint32_t golden_value[ROI_MISR_MAX_ROIS_PER_CRTC];
	uint32_t crc_value[ROI_MISR_MAX_ROIS_PER_CRTC];
	uint32_t roi_ids[ROI_MISR_MAX_ROIS_PER_CRTC];
	int ids_idx[ROI_MISR_MAX_MISRS_PER_CRTC];
	pthread_t roi_misr_sig_thread;
};

struct fb_obj {
	uint32_t fb_id;
	size_t size;
	size_t offset;
	size_t pitch;
	unsigned handle;
	void *ptr;
	int fd;
	int width;
	int height;
	int aligned_w;
	int aligned_h;
	int bpp;
	uint32_t format;
#ifdef USE_ION
	unsigned ihdl;
	int ion_fd;
	int ion_map_fd;
#endif
#ifdef USE_DMA_HEAP
	int dmabuf_fd;
#endif
	uint64_t modifier[4];
};

struct connector_config {
	uint32_t connector_id;
	uint32_t crtc_id;
	uint32_t crtc_idx;
	uint32_t width;
	uint32_t height;

	uint32_t mode_id;
	drmModeModeInfo mode;
	bool active;
	bool detach;
	roi_misr_config roi_misr;
	struct sde_drm_roi_misr_v1 roi_misr_data;

	uint32_t crtc_id_prop;
	uint32_t active_prop;
	uint32_t mode_id_prop;
	uint32_t roi_misr_id_prop;
};

struct plane_config {
	uint32_t plane_id;
	uint32_t crtc_id;
	uint32_t connector_id;

	int num_fbs;
	fb_obj *fbs;
	int fb_index;
	int last_fb_index;

	int y_offset;
	int y_increment;
	bool first_run;
	bool handoff_init;
	uint32_t possible_crtcs;

	int x;
	int y;
	int w;
	int h;
	int zpos;
	int crop_x;
	int crop_y;
	int crop_w;
	int crop_h;
	float scale;
	bool animation;
	uint32_t rotation;

	/* optional */
	int handoff;
	bool handoff_set;
	bool handoff_on_exit;

	/* properties */
	uint32_t crtc_prop;
	uint32_t fb_prop;
	uint32_t zpos_prop;
	uint32_t crtc_x_prop;
	uint32_t crtc_y_prop;
	uint32_t crtc_w_prop;
	uint32_t crtc_h_prop;
	uint32_t src_x_prop;
	uint32_t src_y_prop;
	uint32_t src_w_prop;
	uint32_t src_h_prop;
	uint32_t handoff_prop;
	uint32_t rotation_prop;
	uint32_t csc;
};

struct image_config {
	char filename[256];
	uint32_t plane_id;
	uint8_t *ptr;
	uint64_t color;
	int w;
	int h;
	uint32_t format;
	int size;
	int bpp;
	int flag;
};

bool quit = false;

static int64_t get_timestamp_ms(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static void usage(char *name)
{
	printf("usage: %s [-csptlLibdfFCm]\n", name);
	printf("\n Query options:\n\n");
	printf("\t-c <card>\n\t\topen DRI card\n");
	printf("\t-s <connector_id>@<crtc_id>:<mode>[@vrefresh][~][~~]\n\t\tsetup a connector\n");
	printf("\t-p <plane_id>@<crtc_id>:<x,y,w,h>#<z-order>[/<crop_x,crop_y,crop_w,crop_h>][*<scale>][!<handoff>][~][^rotation]\n\t\tsetup a plane\n");
	printf("\t-t <n-frame>\n\t\tnumber of frames before terminate\n");
	printf("\t-l \n\t\tlist connectors, crtc and planes\n");
	printf("\t-L \n\t\tlist connectors, crtc and planes, and their properties\n");
#ifdef DEBUG_BUILD
	printf("\t-i <filename/pattern>:<w>,<h>@<format>#flag\n\t\tload image/pattern to the plane in the order, built-in patterns:\n");
	printf("\t\timage_96x96, image_256x256, image_720p, image_1080p, color_xxxxxxxx\n");
#else
	printf("\t-i <filename/color_xxxxxxxx>:<w>,<h>@<format>#flag\n\t\tload image/solid_color\n");
#endif
	printf("\t\t # flag 0(not ubwc image), 1(ubwc image), ...\n");
	printf("\t-b \n\t\tbatch connector configs into one commit\n");
	printf("\t-d <delay_ms>\n\t\tdelay time in ms between two frames\n");
	printf("\t-f\n\t\tprint fps statistics\n");
	printf("\t-F <number of framebuffers>\n\t\tNumber of frame buffers, default is 1\n");
	printf("\t-C\n\t\tdisable framebuffer cache, default is enabled\n");
	printf("\t-m <connector_id>@<x,y,w,h>[#<roi_id>]\n\t\tadd ROI MISR rectangle\n");
	printf("\t-S <color space>\n\t\t1 = 601\n\t\t2 = 601FR\n\t\t3 = 709L\n\t\t4 = 709FR\n");
	exit(0);
}

static bool is_ubwc_format(uint32_t format)
{
	bool is_ubwc = false;

	switch (format) {
	case DRM_FORMAT_BGR565:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_NV12:
		is_ubwc = true;
		break;
	default:
		DEBUGT_PRINT_INFO("Unsupported ubwc format: 0x%x", format);
		break;
	}

	return is_ubwc;
}

static bool is_ubwc_enable(const fb_obj *fb)
{
	if (!fb)
		return false;

	return (fb->modifier[0] == DRM_FORMAT_MOD_QCOM_COMPRESSED)
			&& is_ubwc_format(fb->format);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,15,0)
static size_t get_ubwc_frame_size(fb_obj *fb) {
	size_t frame_size = 0, pitch = 0;

	if (!fb)
		return frame_size;

	switch (fb->format) {
	case DRM_FORMAT_BGR565:
		frame_size = MMM_COLOR_FMT_BUFFER_SIZE(MMM_COLOR_FMT_RGB565_UBWC, fb->width, fb->height);
		pitch = MMM_COLOR_FMT_RGB_STRIDE(MMM_COLOR_FMT_RGB565_UBWC, fb->width);
		fb->aligned_h = (frame_size + pitch - 1) / pitch;
		fb->aligned_w = (pitch * 8 + fb->bpp - 1) / fb->bpp;
		break;
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_XBGR8888:
		frame_size = MMM_COLOR_FMT_BUFFER_SIZE(MMM_COLOR_FMT_RGBA8888_UBWC, fb->width, fb->height);
		pitch = MMM_COLOR_FMT_RGB_STRIDE(MMM_COLOR_FMT_RGBA8888_UBWC, fb->width);
		fb->aligned_h = (frame_size + pitch - 1) / pitch;
		fb->aligned_w = (pitch * 8 + fb->bpp - 1) / fb->bpp;
		break;
	case DRM_FORMAT_ABGR2101010:
		frame_size = MMM_COLOR_FMT_BUFFER_SIZE(MMM_COLOR_FMT_RGBA1010102_UBWC, fb->width, fb->height);
		pitch = MMM_COLOR_FMT_RGB_STRIDE(MMM_COLOR_FMT_RGBA1010102_UBWC, fb->width);
		fb->aligned_h = (frame_size + pitch - 1) / pitch;
		fb->aligned_w = (pitch * 8 + fb->bpp - 1) / fb->bpp;
		break;
	case DRM_FORMAT_NV12:
		frame_size = MMM_COLOR_FMT_BUFFER_SIZE(MMM_COLOR_FMT_NV12_UBWC, fb->width, fb->height);
		break;
	default:
		DEBUGT_PRINT_INFO("Unsupported pixel format: 0x%x", fb->format);
		break;
	}

	DEBUGT_PRINT_INFO("Get ubwc frame size %zu .\n", frame_size);
	return frame_size;
}
#else
static size_t get_ubwc_frame_size(fb_obj *fb) {
	size_t frame_size = 0, pitch = 0;

	if (!fb)
		return frame_size;

	switch (fb->format) {
	case DRM_FORMAT_BGR565:
		frame_size = VENUS_BUFFER_SIZE(COLOR_FMT_RGB565_UBWC, fb->width, fb->height);
		pitch = VENUS_RGB_STRIDE(COLOR_FMT_RGB565_UBWC, fb->width);
		fb->aligned_h = (frame_size + pitch - 1) / pitch;
		fb->aligned_w = (pitch * 8 + fb->bpp - 1) / fb->bpp;
		break;
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_XBGR8888:
		frame_size = VENUS_BUFFER_SIZE(COLOR_FMT_RGBA8888_UBWC, fb->width, fb->height);
		pitch = VENUS_RGB_STRIDE(COLOR_FMT_RGBA8888_UBWC, fb->width);
		fb->aligned_h = (frame_size + pitch - 1) / pitch;
		fb->aligned_w = (pitch * 8 + fb->bpp - 1) / fb->bpp;
		break;
	case DRM_FORMAT_ABGR2101010:
		frame_size = VENUS_BUFFER_SIZE(COLOR_FMT_RGBA1010102_UBWC, fb->width, fb->height);
		pitch = VENUS_RGB_STRIDE(COLOR_FMT_RGBA1010102_UBWC, fb->width);
		fb->aligned_h = (frame_size + pitch - 1) / pitch;
		fb->aligned_w = (pitch * 8 + fb->bpp - 1) / fb->bpp;
		break;
	case DRM_FORMAT_NV12:
		frame_size = VENUS_BUFFER_SIZE(COLOR_FMT_NV12_UBWC, fb->width, fb->height);
		break;
	default:
		DEBUGT_PRINT_INFO("Unsupported pixel format: 0x%x", fb->format);
		break;
	}

	DEBUGT_PRINT_INFO("Get ubwc frame size %zu .\n", frame_size);
	return frame_size;
}
#endif

static size_t calculate_frame_size(fb_obj *fb) {
	size_t frame_size = 0;

	if (!fb)
		return frame_size;

	fb->aligned_h = fb->height;
	fb->aligned_w = fb->width;
	if (is_ubwc_enable(fb)) {
		frame_size = get_ubwc_frame_size(fb);
		fb->pitch = fb->aligned_w * fb->bpp / 8;
		return frame_size;
	}

	if (fb->format == DRM_FORMAT_NV12)
		fb->aligned_h = fb->height * 3 / 2;

	fb->pitch = fb->aligned_w * fb->bpp / 8;
	frame_size = fb->pitch * fb->aligned_h;

	DEBUGT_PRINT_INFO("Get frame size %zu.\n", frame_size);
	return frame_size;
}

static void cpu_access_begin(int fd)
{
#ifdef USE_DMA_HEAP
	struct dma_buf_sync buf_sync;

	buf_sync.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
	if (ioctl(fd, DMA_BUF_IOCTL_SYNC, &buf_sync)) {
		DEBUGT_PRINT_ERROR("Call ioctl failed to start DMA_BUF_IOCTL_SYNC.\n");
	}
#endif
}

static void cpu_access_end(int fd)
{
#ifdef USE_DMA_HEAP
	struct dma_buf_sync buf_sync;

	buf_sync.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW;
	if (ioctl(fd, DMA_BUF_IOCTL_SYNC, &buf_sync)) {
		DEBUGT_PRINT_ERROR("Call ioctl failed to start DMA_BUF_IOCTL_SYNC.\n");
	}
#endif
}

#ifdef USE_ION

static int create_fb_obj(int fd, struct plane_config *plane_cfg, int index)
{
	struct ion_allocation_data ion_alloc_data;
#if TARGET_ION_ABI_VERSION >= 2
	int buf_fd = -1;
#else
	struct ion_fd_data fd_data;
#endif
	int ion_fd, ret;
	size_t page_size, frame_size = 0;
	char *pdata;

#if TARGET_ION_ABI_VERSION >= 2
	ion_fd = ion_open();//"/dev/ion", O_RDWR | O_DSYNC);
#else
	ion_fd = open("/dev/ion", O_RDWR | O_DSYNC);
#endif
	if (ion_fd < 0) {
		fprintf(stderr, "failed to open ion for allocation\n");
		return -EINVAL;
	}

	page_size = sysconf(_SC_PAGESIZE);

	plane_cfg->fbs[index].width = plane_cfg->w;
	if (plane_cfg->animation)
		plane_cfg->fbs[index].height = plane_cfg->h * 3 / 2;
	else
		plane_cfg->fbs[index].height = plane_cfg->h;

	frame_size = calculate_frame_size(&plane_cfg->fbs[index]);

	/* round up to page size */
	plane_cfg->fbs[index].size = (frame_size + page_size - 1) & (~(page_size - 1));
	plane_cfg->fbs[index].offset = 0;
	plane_cfg->fbs[index].fd = fd; /* drm fd */
	plane_cfg->fbs[index].ion_fd = ion_fd; /* ion fd */

	memset(&ion_alloc_data, 0, sizeof(ion_alloc_data));
	ion_alloc_data.len = plane_cfg->fbs[index].size;
#ifndef TARGET_ION_ABI_VERSION
	ion_alloc_data.align = page_size;
#endif
	ion_alloc_data.heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID);
	ion_alloc_data.flags = 0;

	//fprintf(stderr, "len = %lu align=%lu heap_id_msk=0x%x flags=0x%x\n", ion_alloc_data.len,
	//		ion_alloc_data.align, ion_alloc_data.heap_id_mask, ion_alloc_data.flags);

#if TARGET_ION_ABI_VERSION >= 2
	ret = ion_alloc_fd(ion_fd, ion_alloc_data.len, ion_alloc_data.align,
			ion_alloc_data.heap_id_mask, ion_alloc_data.flags, &buf_fd);
#else
	ret = ioctl(ion_fd, ION_IOC_ALLOC, &ion_alloc_data);
#endif
	if (ret) {
		fprintf(stderr, "failed to allocate memory from ion: %s\n",
			strerror(errno));
		goto error;
	}

#if TARGET_ION_ABI_VERSION >= 2
	plane_cfg->fbs[index].ion_map_fd = buf_fd;
#else
	plane_cfg->fbs[index].ihdl = ion_alloc_data.handle;
	fd_data.handle = plane_cfg->fbs[index].ihdl;

	ret = ioctl(ion_fd, ION_IOC_MAP, &fd_data);
	if (ret) {
		fprintf(stderr, "failed to map ion memory: %s\n",
			strerror(errno));
		goto error;
	}

	plane_cfg->fbs[index].ion_map_fd = fd_data.fd;
#endif

	pdata = (char *)mmap(NULL,
				plane_cfg->fbs[index].size,
				PROT_READ|PROT_WRITE, MAP_SHARED, plane_cfg->fbs[index].ion_map_fd, 0);
	if (MAP_FAILED == pdata)
		fprintf(stderr, "mmap FAILED\n");
	else {
		munmap(pdata, plane_cfg->fbs[index].size);
	}

	return 0;

error:
#ifndef TARGET_ION_ABI_VERSION
	if (ion_alloc_data.handle) {
		struct ion_handle_data hdl_data = {
			.handle = ion_alloc_data.handle,
		};
		ioctl(plane_cfg->fbs[index].ion_fd, ION_IOC_FREE, &hdl_data);
	}
#endif
	close(ion_fd);
	return ret;
}

int import_fb_obj(int fd, struct plane_config *plane_cfg, int index)
{
	struct drm_prime_handle prime_req;
	int ret;

	prime_req.fd = plane_cfg->fbs[index].ion_map_fd;

	ret = drmIoctl(fd, DRM_IOCTL_PRIME_FD_TO_HANDLE, &prime_req);
	if (ret) {
		close(plane_cfg->fbs[index].ion_map_fd);
		fprintf(stderr, "failed get prime handle: %s\n",
			strerror(errno));
		return ret;
	}

	plane_cfg->fbs[index].handle = prime_req.handle;

	return 0;
}

int unimport_fb_obj(int fd, struct plane_config *plane_cfg, int index)
{
	struct drm_gem_close gem_close;
	int ret;

	gem_close.handle = plane_cfg->fbs[index].handle;

	ret = drmIoctl(fd, DRM_IOCTL_GEM_CLOSE, &gem_close);
	if (ret) {
		fprintf(stderr, "failed close gem handle: %s\n",
			strerror(errno));
		return ret;
	}

	return 0;
}

static void *map_fb_obj(struct fb_obj *fb)
{
	void *map;

	map = drm_mmap(0, fb->size, PROT_READ | PROT_WRITE, MAP_SHARED,
		       fb->ion_map_fd, fb->offset);
	if (map == MAP_FAILED) {
		DEBUGT_PRINT_ERROR("failed to map buffer: %s\n",
			strerror(errno));
		return NULL;
	}

	fb->ptr = map;

	return map;
}

static void unmap_fb_obj(struct fb_obj *fb)
{
	if (!fb->ptr)
		return;

	drm_munmap(fb->ptr, fb->size);
	fb->ptr = NULL;
}

static int destroy_fb_obj(struct fb_obj *fb)
{
#ifndef TARGET_ION_ABI_VERSION
	struct ion_handle_data hdl_data = {
		.handle = fb->ihdl,
	};
	int ret;
#endif

	unmap_fb_obj(fb);
	close(fb->ion_map_fd);
#ifndef TARGET_ION_ABI_VERSION
	ret = ioctl(fb->ion_fd, ION_IOC_FREE, &hdl_data);
	if (ret) {
		fprintf(stderr, "failed to free ion buffer: %s\n",
			strerror(errno));
		return ret;
	}
#endif
	close(fb->ion_fd);

	return 0;
}

#elif defined(USE_DMA_HEAP)

static int create_fb_obj(int fd, struct plane_config *plane_cfg, int index)
{
	int dma_heap_fd;
	size_t page_size, frame_size = 0;
	struct dma_heap_allocation_data alloc_data;
	unsigned int fd_flags = O_RDWR | O_CLOEXEC;
	unsigned int heap_flags = DMA_HEAP_VALID_HEAP_FLAGS;
	char *pdata;
	int ret;

	dma_heap_fd = open(DEFAULT_HEAP, O_RDONLY | O_CLOEXEC);
	if (dma_heap_fd < 0) {
		fprintf(stderr, "failed to find the dma_buf heap %s\n", DEFAULT_HEAP);
		return -EINVAL;
	}

	page_size = sysconf(_SC_PAGESIZE);

	plane_cfg->fbs[index].width = plane_cfg->w;
	if (plane_cfg->animation)
		plane_cfg->fbs[index].height = plane_cfg->h * 3 / 2;
	else
		plane_cfg->fbs[index].height = plane_cfg->h;

	frame_size = calculate_frame_size(&plane_cfg->fbs[index]);

	/* round up to page size */
	plane_cfg->fbs[index].size = (frame_size + page_size - 1) & (~(page_size - 1));
	plane_cfg->fbs[index].offset = 0;
	plane_cfg->fbs[index].fd = fd; /* drm fd */

	alloc_data.len = plane_cfg->fbs[index].size;
	alloc_data.fd = 0;
	alloc_data.fd_flags = fd_flags;
	alloc_data.heap_flags = heap_flags;

	ret = ioctl(dma_heap_fd, DMA_HEAP_IOCTL_ALLOC, &alloc_data);
	if (ret) {
		fprintf(stderr, "failed to allocate dmabuf from heap %s: %s\n",
			DEFAULT_HEAP, strerror(errno));
		goto error;
	}

	plane_cfg->fbs[index].dmabuf_fd = alloc_data.fd;

	pdata = (char *)mmap(NULL,
				plane_cfg->fbs[index].size,
				PROT_READ|PROT_WRITE, MAP_SHARED, plane_cfg->fbs[index].dmabuf_fd, 0);
	if (MAP_FAILED == pdata)
		fprintf(stderr, "mmap FAILED\n");
	else {
		munmap(pdata, plane_cfg->fbs[index].size);
	}

	close(dma_heap_fd);
	return 0;

error:
	close(dma_heap_fd);
	return ret;
}

int import_fb_obj(int fd, struct plane_config *plane_cfg, int index)
{
	struct drm_prime_handle prime_req;
	int ret;

	prime_req.fd = plane_cfg->fbs[index].dmabuf_fd;

	ret = drmIoctl(fd, DRM_IOCTL_PRIME_FD_TO_HANDLE, &prime_req);
	if (ret) {
		close(plane_cfg->fbs[index].dmabuf_fd);
		fprintf(stderr, "failed get prime handle: %s\n",
			strerror(errno));
		return ret;
	}

	plane_cfg->fbs[index].handle = prime_req.handle;

	return 0;
}

int unimport_fb_obj(int fd, struct plane_config *plane_cfg, int index)
{
	struct drm_gem_close gem_close;
	int ret;

	gem_close.handle = plane_cfg->fbs[index].handle;

	ret = drmIoctl(fd, DRM_IOCTL_GEM_CLOSE, &gem_close);
	if (ret) {
		fprintf(stderr, "failed close gem handle: %s\n",
			strerror(errno));
		return ret;
	}

	return 0;
}

static void *map_fb_obj(struct fb_obj *fb)
{
	void *map;
	struct dma_buf_sync sync_start = {0};

	map = drm_mmap(0, fb->size, PROT_READ | PROT_WRITE, MAP_SHARED,
		       fb->dmabuf_fd, fb->offset);
	if (map == MAP_FAILED) {
		DEBUGT_PRINT_ERROR("failed to map buffer: %s\n",
			strerror(errno));
		return NULL;
	}

	fb->ptr = map;

	sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
	if (ioctl(fb->dmabuf_fd, DMA_BUF_IOCTL_SYNC, &sync_start))
		printf("Failed to sync start dmabuf\n");

	return map;
}

static void unmap_fb_obj(struct fb_obj *fb)
{
	struct dma_buf_sync sync_end = {0};

	if (!fb->ptr)
		return;

	sync_end.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW;
	if (ioctl(fb->dmabuf_fd, DMA_BUF_IOCTL_SYNC, &sync_end))
		printf("Failed to sync end dmabuf\n");

	drm_munmap(fb->ptr, fb->size);
	fb->ptr = NULL;
}

static int destroy_fb_obj(struct fb_obj *fb)
{
	unmap_fb_obj(fb);
	close(fb->dmabuf_fd);

	return 0;
}

#else

static int create_fb_obj(int fd, struct plane_config *plane_cfg, int index)
{
	struct drm_mode_create_dumb create_arg;

	memset(&create_arg, 0, sizeof(create_arg));
	create_arg.bpp = plane_cfg->fb[index].bpp;
	create_arg.width = plane_cfg->w;

	/* create a 1.5x height buffer to show animation */
	if (plane_cfg->animation)
		create_arg.height = plane_cfg->h * 3 / 2;
	else
		create_arg.height = plane_cfg->h;

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

	plane_cfg->fb[index].handle = create_arg.handle;
	plane_cfg->fb[index].size = create_arg.size;
	plane_cfg->fb[index].pitch = create_arg.pitch;
	plane_cfg->fb[index].width = create_arg.width;
	plane_cfg->fb[index].height = create_arg.height;
	plane_cfg->fb[index].fd = fd; /* drm fd */

	return 0;
}

int import_fb_obj(int fd, struct plane_config *plane_cfg, int index)
{
	// No need import
	return 0;
}

int unimport_fb_obj(struct fb_obj *fb)
{
	// No need unimport
	return 0;
}

static void *map_fb_obj(struct fb_obj *fb)
{
	void *map;

	map = drm_mmap(0, fb->size,
		PROT_READ | PROT_WRITE, MAP_SHARED, fb->fd, map_arg.offset);
	if (map == MAP_FAILED) {
		DEBUGT_PRINT_ERROR("failed to map buffer: %s\n",
			strerror(errno));
		return NULL;
	}

	fb->ptr = map;

	return map;
}

static void unmap_fb_obj(struct fb_obj *fb)
{
	if (!fb->ptr)
		return;

	drmUnmap(fb->ptr, fb->size);

	fb->ptr = NULL;
}

static int destroy_fb_obj(struct fb_obj *fb)
{
	struct drm_mode_destroy_dumb destroy_arg;
	int ret;

	memset(&destroy_arg, 0, sizeof(destroy_arg));
	destroy_arg.handle = fb->handle;
	ret = drmIoctl(fb->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy_arg);
	if (ret) {
		DEBUGT_PRINT_ERROR("failed to destroy dumb buffer: %s\n",
			strerror(errno));
		return ret;
	}

	return 0;
}

#endif

static int create_fb_id(int fd, struct plane_config *plane_cfg, int index)
{
	struct drm_mode_fb_cmd2 cmd2 {};
	int ret;

	cmd2.width = plane_cfg->fbs[index].width;
	cmd2.height = plane_cfg->fbs[index].height;
	cmd2.pixel_format = plane_cfg->fbs[index].format;
	cmd2.flags = DRM_MODE_FB_MODIFIERS;
	cmd2.handles[0] = plane_cfg->fbs[index].handle;
	cmd2.pitches[0] = plane_cfg->fbs[index].pitch;
	cmd2.offsets[0] = 0;

	if (is_ubwc_enable(&plane_cfg->fbs[index])) {
		cmd2.modifier[0] = plane_cfg->fbs[index].modifier[0];
	} else {
		cmd2.modifier[0] = 0;
	}

	if (plane_cfg->fbs[index].format == DRM_FORMAT_NV12) {
		cmd2.handles[1] = cmd2.handles[0];
		cmd2.pitches[1] = cmd2.pitches[0];
		cmd2.offsets[1] = cmd2.pitches[0] * cmd2.height;
		cmd2.modifier[1] = cmd2.modifier[0];
	}

	ret = drmIoctl(fd, DRM_IOCTL_MODE_ADDFB2, &cmd2);
	if (ret) {
		DEBUGT_PRINT_ERROR("failed to addfb2: %s\n",
			strerror(errno));
		return ret;
	}
	plane_cfg->fbs[index].fb_id = cmd2.fb_id;

	return 0;
}

static int remove_fb_id(int fd, struct plane_config *plane_cfg, int index)
{
	int ret;

	ret = drmIoctl(fd, DRM_IOCTL_MSM_RMFB2, &plane_cfg->fbs[index].fb_id);
	if (ret) {
		DEBUGT_PRINT_ERROR("failed to rmfb: %s\n",
			strerror(errno));
		return ret;
	}

	return 0;
}

struct plane_config *find_plane_cfg(std::vector<plane_config> &plane_cfg,
		uint32_t plane_id)
{
	int i;

	for (i = 0; i < (int)plane_cfg.size(); i++)
		if (plane_id == plane_cfg[i].plane_id)
			return &plane_cfg[i];

	return NULL;
}

static int parse_plane(struct plane_config *plane, const char *p)
{
	const char *endp;

	if (sscanf(p, "%d@%d:%d,%d,%d,%d#%d", &plane->plane_id, &plane->crtc_id,
		&plane->x, &plane->y, &plane->w, &plane->h, &plane->zpos) != 7) {
		DEBUGT_PRINT_ERROR("failed to parse %s\n", p);
		return -1;
	}

	endp = strchr(p, '/');
	if (endp) {
		if (sscanf(endp, "/%d,%d,%d,%d", &plane->crop_x, &plane->crop_y,
			&plane->crop_w, &plane->crop_h) != 4) {
			DEBUGT_PRINT_ERROR("failed to parse %s\n", endp);
			return -1;
		}
	} else {
		plane->crop_x = 0;
		plane->crop_y = 0;
		plane->crop_w = plane->w;
		plane->crop_h = plane->h;
	}

	endp = strchr(p, '*');
	if (endp) {
		if (sscanf(endp, "*%f", &plane->scale) != 1) {
			DEBUGT_PRINT_ERROR("failed to parse %s\n", endp);
			return -1;
		}
	} else {
		plane->scale = 1.0;
	}

	endp = strchr(p, '!');
	if (endp) {
		if (sscanf(endp, "!%d", &plane->handoff) != 1) {
			DEBUGT_PRINT_ERROR("failed to parse %s\n", endp);
			return -1;
		}
		plane->handoff_set = true;
	}

	endp = strchr(p, '~');
	if (endp) {
		plane->handoff_on_exit = true;
	}

	endp = strchr(p, '^');
	if (endp) {
		if (sscanf(endp, "^%x", &plane->rotation) != 1) {
			DEBUGT_PRINT_ERROR("failed to parse %s\n", endp);
			return -1;
		}
	}else {
		plane->rotation = DRM_MODE_ROTATE_0;
	}

	return 0;
}

struct connector_config *find_connector_cfg(std::vector<connector_config> &connector_cfg,
		uint32_t connector_id)
{
	int i;

	for (i = 0; i < (int)connector_cfg.size(); i++)
		if (connector_id == connector_cfg[i].connector_id)
			return &connector_cfg[i];

	return NULL;
}

static int parse_connector(struct connector_config *connector, const char *p)
{
	const char *endp;
	int ret;

	ret = sscanf(p, "%d@%d:%[^@]@%d", &connector->connector_id,
		&connector->crtc_id, connector->mode.name, &connector->mode.vrefresh);

	if (ret != 3 && ret != 4) {
		DEBUGT_PRINT_ERROR("failed to parse %s\n", p);
		return -1;
	}

	connector->active = true;
	endp = strchr(p, '~');
	if (endp) {
		connector->active = false;
		if (endp[1] == '~')
			connector->detach = true;
	}

	return 0;
}

static int parse_roi_misr(std::vector<connector_config> &connector_cfg, const char *p)
{
	const char *endp;
	uint32_t connector_id, x, y, w, h;
	int id = -1;
	struct connector_config *cfg;
	int ret;

	ret = sscanf(p, "%d@%d,%d,%d,%d", &connector_id, &x, &y, &w, &h);

	if (ret != 5) {
		DEBUGT_PRINT_ERROR("failed to parse %s\n", p);
		return -1;
	}

	endp = strchr(p, '#');
	if (endp) {
		if (sscanf(endp, "#%d", &id) != 1) {
			DEBUGT_PRINT_ERROR("failed to parse roi ID %s\n", endp);
			return -2;
		}
	}

	cfg = find_connector_cfg(connector_cfg, connector_id);
	if (cfg == NULL) {
		DEBUGT_PRINT_ERROR("connector ID %d invalid %s\n", connector_id, p);
		return -3;
	}

	if (cfg->roi_misr.num_rois >= ROI_MISR_MAX_ROIS_PER_CRTC) {
		DEBUGT_PRINT_ERROR("Number of ROI MISR exceed max limit %s\n", p);
		return -4;
	}

	cfg->roi_misr.roi_rects[cfg->roi_misr.num_rois].x1 = x;
	cfg->roi_misr.roi_rects[cfg->roi_misr.num_rois].y1 = y;
	cfg->roi_misr.roi_rects[cfg->roi_misr.num_rois].x2 = x + w - 1;
	cfg->roi_misr.roi_rects[cfg->roi_misr.num_rois].y2 = y + h - 1;
	cfg->roi_misr.roi_ids[cfg->roi_misr.num_rois] = id;
	DEBUGT_PRINT_INFO("Conn%d ROI[%d] id=%d (%d,%d)-(%d,%d)\n", cfg->connector_id,
			cfg->roi_misr.num_rois, cfg->roi_misr.roi_ids[cfg->roi_misr.num_rois],
			cfg->roi_misr.roi_rects[cfg->roi_misr.num_rois].x1,
			cfg->roi_misr.roi_rects[cfg->roi_misr.num_rois].y1,
			cfg->roi_misr.roi_rects[cfg->roi_misr.num_rois].x2,
			cfg->roi_misr.roi_rects[cfg->roi_misr.num_rois].y2);
	cfg->roi_misr.num_rois++;

	return 0;
}

static int parse_image(struct image_config *img_cfg, const char *p)
{
	int ret;
	char format[16];
	int w, h;
	uint64_t color;
	int pattern_index = -1;
	FILE *fp = NULL;
	const char *endp;

	ret = sscanf(p, "%[^:]:%d,%d@%[^#]", img_cfg->filename, &w, &h, format);

	if (ret == 4 && (sscanf(p, "color_%lx:", &color) == 1)) {
		pattern_index = 0;
		img_cfg->color = color;
	} else if (ret == 4) {
		fp = fopen(img_cfg->filename, "rb");
		if (!fp) {
			DEBUGT_PRINT_ERROR("failed to open %s\n", img_cfg->filename);
			return -1;
		}
		endp = strchr(p, '#');
		if (endp) {
			if (sscanf(endp, "#%d", &img_cfg->flag) != 1) {
				DEBUGT_PRINT_INFO("failed to parse image flag %s\n", endp);
			}
		}
		pattern_index = 1;
#ifdef DEBUG_BUILD
	} else if (ret == 1 && !strcmp(img_cfg->filename, "image_96x96")) {
		pattern_index = 2;
		w = image_96x96.width;
		h = image_96x96.height;
		strlcpy(format, image_96x96.format, sizeof(format));
	} else if (ret == 1 && !strcmp(img_cfg->filename, "image_256x256")) {
		pattern_index = 3;
		w = image_chip.width;
		h = image_chip.height;
		strlcpy(format, image_chip.format, sizeof(format));
	} else if (ret == 1 && !strcmp(img_cfg->filename, "image_720p")) {
		pattern_index = 4;
		w = image_720p.width;
		h = image_720p.height;
		strlcpy(format, image_720p.format, sizeof(format));
	} else if (ret == 1 && !strcmp(img_cfg->filename, "image_1080p")) {
		pattern_index = 5;
		w = image_1080p.width;
		h = image_1080p.height;
		strlcpy(format, image_1080p.format, sizeof(format));
#endif
	} else {
		DEBUGT_PRINT_ERROR("failed to parse %s  ret=%d\n", p, ret);
		return -1;
	}

	img_cfg->format = fourcc_code(format[0], format[1], format[2], format[3]);
	img_cfg->w = w;
	img_cfg->h = h;

	switch (img_cfg->format) {
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
		img_cfg->bpp = 16;
		break;
	case DRM_FORMAT_ARGB4444:
	case DRM_FORMAT_XRGB4444:
	case DRM_FORMAT_ABGR4444:
	case DRM_FORMAT_XBGR4444:
	case DRM_FORMAT_RGBA4444:
	case DRM_FORMAT_RGBX4444:
	case DRM_FORMAT_BGRA4444:
	case DRM_FORMAT_BGRX4444:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_BGR565:
	case DRM_FORMAT_ARGB1555:
	case DRM_FORMAT_XRGB1555:
	case DRM_FORMAT_ABGR1555:
	case DRM_FORMAT_XBGR1555:
	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_RGBX5551:
	case DRM_FORMAT_BGRA5551:
	case DRM_FORMAT_BGRX5551:
		img_cfg->bpp = 16;
		break;
	case DRM_FORMAT_BGR888:
	case DRM_FORMAT_RGB888:
		img_cfg->bpp = 24;
		break;
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_BGRA8888:
	case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_XRGB2101010:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_XBGR2101010:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_RGBX1010102:
	case DRM_FORMAT_BGRA1010102:
	case DRM_FORMAT_BGRX1010102:
		img_cfg->bpp = 32;
		break;
	case DRM_FORMAT_XRGB16161616F:
	case DRM_FORMAT_XBGR16161616F:
	case DRM_FORMAT_ARGB16161616F:
	case DRM_FORMAT_ABGR16161616F:
		img_cfg->bpp = 64;
		break;
	case DRM_FORMAT_NV12:
		img_cfg->bpp = 8;
		break;
	default:
		DEBUGT_PRINT_ERROR("unsupported format %c%c%c%c\n",
			format[0], format[1], format[2], format[3]);
		goto error;
	}

	if (pattern_index != 0) {
		if (DRM_FORMAT_NV12 == img_cfg->format) {

			img_cfg->size = (w * h * 3) / 2;
		}
		else
			img_cfg->size = w * h * img_cfg->bpp / 8;

		img_cfg->ptr = (uint8_t*)malloc(img_cfg->size);
		if (!img_cfg->ptr)
			goto error;
	} else {
		img_cfg->size = 0;
		img_cfg->ptr = NULL;
	}

	switch (pattern_index)
	{
	case 0:
		DEBUGT_PRINT_INFO("Load solid color %dx%d color = %lX\n", w, h, img_cfg->color);
		break;
	case 1:
		ret = fread(img_cfg->ptr, 1, img_cfg->size, fp);
		fclose(fp);
		DEBUGT_PRINT_INFO("Load %dx%d image %s\n", w, h, img_cfg->filename);
		break;
#ifdef DEBUG_BUILD
	case 2:
		memcpy(img_cfg->ptr, image_96x96.y, img_cfg->size);
		DEBUGT_PRINT_INFO("Load 96x96 test image, golden CRC = %8.8X\n", image_96x96.crc_value);
		break;
	case 3:
		memcpy(img_cfg->ptr, image_chip.y, img_cfg->size);
		DEBUGT_PRINT_INFO("Load 256x256 test image, golden CRC = %8.8X\n", image_chip.crc_value);
		break;
	case 4:
		memcpy(img_cfg->ptr, image_720p.y, img_cfg->size);
		DEBUGT_PRINT_INFO("Load 1280x720 test image, golden CRC = %8.8X\n", image_720p.crc_value);
		break;
	case 5:
		memcpy(img_cfg->ptr, image_1080p.y, img_cfg->size);
		DEBUGT_PRINT_INFO("Load 1920x1080 test image, golden CRC = %8.8X\n", image_1080p.crc_value);
		break;
#endif
	default:
		break;
	}

	return 0;

error:
	if(fp)
		fclose(fp);
	return -1;
}

static bool check_possible_crtc(int fd, uint32_t crtc_idx, drmModeConnectorPtr conn)
{
	int j;
	bool ret = false;

	for (j = 0; j < conn->count_encoders; j++) {
		drmModeEncoderPtr encoder = drmModeGetEncoder(fd, conn->encoders[j]);
		if (encoder && encoder->possible_crtcs & (1 << crtc_idx))
			ret = true;
		drmModeFreeEncoder(encoder);
		if(ret)
			break;
	}
	return ret;
}

static void build_roi_misr(connector_config &cfg)
{
	int i;

	if (!cfg.roi_misr.num_rois)
		return;

	if (1) {
		//init for DSC cases
		cfg.roi_misr.ids_idx[0] = 0;
		cfg.roi_misr.ids_idx[1] = 4;
		cfg.roi_misr.ids_idx[2] = 8;
		cfg.roi_misr.ids_idx[3] = 12;
		cfg.roi_misr.ids_idx[4] = 16;
		cfg.roi_misr.ids_idx[5] = 20;
	} else {
		//init for 3DMUX cases
		cfg.roi_misr.ids_idx[0] = 0;
		cfg.roi_misr.ids_idx[1] = 8;
		cfg.roi_misr.ids_idx[2] = 16;
	}

	for (i = 0; i < cfg.roi_misr.num_rois; i ++) {
		cfg.roi_misr.crc_value[i] = 0xFFFFFFFF;
		if (cfg.roi_misr.roi_ids[i] == (uint32_t)-1) {
			if (cfg.roi_misr.roi_rects[i].x1 >= 7680) {
				cfg.roi_misr.roi_ids[i] = cfg.roi_misr.ids_idx[2];
				cfg.roi_misr.ids_idx[2]++;
			} else if (cfg.roi_misr.roi_rects[i].x1 >= 5120) {
				cfg.roi_misr.roi_ids[i] = cfg.roi_misr.ids_idx[1];
				cfg.roi_misr.ids_idx[1]++;
			} else {
				cfg.roi_misr.roi_ids[i] = cfg.roi_misr.ids_idx[0];
				cfg.roi_misr.ids_idx[0]++;
			}
		}
	}

	memset(&cfg.roi_misr_data, 0, sizeof(cfg.roi_misr_data));
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,15,0)
	cfg.roi_misr_data.fence_fd_ptr = (__s64 *)malloc(sizeof(__s64));
#else
	cfg.roi_misr_data.fence_fd_ptr = (int64_t *)malloc(sizeof(int64_t));
#endif
	if (cfg.roi_misr_data.fence_fd_ptr)
		*cfg.roi_misr_data.fence_fd_ptr = -1;
	cfg.roi_misr_data.roi_rect_num = cfg.roi_misr.num_rois;
	cfg.roi_misr_data.roi_rects = cfg.roi_misr.roi_rects;
	cfg.roi_misr_data.roi_ids = cfg.roi_misr.roi_ids;

	//DEBUGT_PRINT_INFO("Build conn%d ROIs %d\n", cfg.connector_id, cfg.roi_misr_data.roi_rect_num);
}

static void build_default_connector(int fd, drmModeResPtr res,
		std::vector<connector_config> &connector_cfg)
{
	std::vector<uint32_t> crtc_ids;
	int i, k;

	for (i = 0; i < res->count_crtcs; i++)
		crtc_ids.push_back(res->crtcs[i]);

	for (i = 0; i < res->count_connectors; i++) {
		drmModeConnectorPtr conn = drmModeGetConnector(fd, res->connectors[i]);
		connector_config cfg = {};

		if (!conn || (conn->connection != DRM_MODE_CONNECTED || !conn->count_modes ||
			conn->connector_type == DRM_MODE_CONNECTOR_VIRTUAL ||
			!conn->modes)) {
			drmModeFreeConnector(conn);
			continue;
		}

		cfg.mode = conn->modes[0];

		for (k = 0; k < res->count_crtcs; k++) {
			if (crtc_ids[k]) {
				if (!check_possible_crtc(fd, k, conn))
					continue;
				cfg.connector_id = res->connectors[i];
				cfg.crtc_id = crtc_ids[k];
				cfg.crtc_idx = k;
				connector_cfg.push_back(cfg);
				crtc_ids[k] = 0;
				break;
			}
		}

		DEBUGT_PRINT_INFO("build connector %d crtc %d\n", cfg.connector_id, cfg.crtc_id);

		drmModeFreeConnector(conn);
	}
}

static void build_default_plane(int fd, drmModePlaneResPtr res,
		std::vector<connector_config> &connector_cfg,
		std::vector<plane_config> &plane_cfg)
{
	std::vector<uint32_t> plane_ids;
	int i, j;
	bool found_dup;

	for (i = 0; i < (int)res->count_planes; i++)
		plane_ids.push_back(res->planes[i]);

	for (i = 0; i < (int)connector_cfg.size(); i++) {
		if (!connector_cfg[i].active)
			continue;

		found_dup = false;
		for (j = 0; j < plane_cfg.size(); j++) {
			if (plane_cfg[j].crtc_id == connector_cfg[i].crtc_id) {
				found_dup = true;
				break;
			}
		}
		if (found_dup)
			continue;

		for (j = 0; j < (int)plane_ids.size(); j++) {
			plane_config cfg = {};

			if (!plane_ids[j])
				continue;

			drmModePlanePtr plane = drmModeGetPlane(fd, plane_ids[j]);
			if (plane) {
				cfg.possible_crtcs = plane->possible_crtcs;
				drmModeFreePlane(plane);

				if (cfg.possible_crtcs & (1 << connector_cfg[i].crtc_idx)) {
					cfg.plane_id = plane_ids[j];
					cfg.crtc_id = connector_cfg[i].crtc_id;
					cfg.x = 0;
					cfg.y = 0;
					cfg.w = connector_cfg[i].mode.hdisplay;
					cfg.h = connector_cfg[i].mode.vdisplay;
					cfg.zpos = 0;
					cfg.crop_x = 0;
					cfg.crop_y = 0;
					cfg.crop_w = cfg.w;
					cfg.crop_h = cfg.h;
					cfg.scale = 1.0;
					cfg.rotation = DRM_MODE_ROTATE_0;
					cfg.handoff_on_exit = true;
					plane_cfg.push_back(cfg);
					plane_ids[j] = 0;

					DEBUGT_PRINT_INFO("build plane %d crtc %d (%d,%d,%dx%d)\n",
						cfg.plane_id, cfg.crtc_id,
						cfg.x, cfg.y, cfg.w, cfg.h);
					break;
				}
			}
		}
	}
}

static void build_pattern(void *buffer, int width, int height, int pitch, int off_x, int off_y)
{
	uint8_t *row_start = (uint8_t*)buffer;
	int x, y;

	for (y = off_y; y < height + off_y; ++y) {
		uint32_t *ptr = (uint32_t*)row_start;
		for (x = off_x; x < width + off_x; ++x) {
			div_t d = div(x+y, width);
			uint32_t rgb32 = 0x00130502 * (d.quot >> 6)
				       + 0x000a1120 * (d.rem >> 6);
			uint32_t alpha = ((y < height/2) &&
				       (x < width/2)) ? 127 : 255;
			*(ptr++) = (alpha << 24) | (rgb32 & 0xFFFFFF);
		}
		row_start += pitch;
	}
}

static void fill_color(void *buffer, int width, int height, int pitch, uint64_t color, int bpp)
{
	uint8_t *row_start = (uint8_t*)buffer;
	int x, y;

	for (y = 0; y < height; ++y) {
		switch (bpp) {
		case 8:
			{
				uint8_t *ptr = (uint8_t*)row_start;
				for (x = 0; x < width; ++x) {
					*(ptr++) = (uint8_t)color;
				}
			}
			break;
		case 16:
			{
				uint16_t *ptr = (uint16_t*)row_start;
				for (x = 0; x < width; ++x) {
					*(ptr++) = (uint16_t)color;
				}
			}
			break;
		case 24:
			{
				uint8_t *ptr = (uint8_t*)row_start;
				for (x = 0; x < width; ++x) {
					*(ptr++) = (uint8_t)(color & 0xFF);
					*(ptr++) = (uint8_t)((color >> 8) & 0xFF);
					*(ptr++) = (uint8_t)((color >> 16) & 0xFF);
				}
			}
			break;
		case 32:
			{
				uint32_t *ptr = (uint32_t*)row_start;
				for (x = 0; x < width; ++x) {
					*(ptr++) = (uint32_t)color;
				}
			}
			break;
		case 64:
			{
				uint64_t *ptr = (uint64_t*)row_start;
				for (x = 0; x < width; ++x) {
					*(ptr++) = (uint64_t)color;
				}
			}
			break;
		default:
			/* Not supported */
			DEBUGT_PRINT_ERROR("Color fill for bpp %d isn't supported\n", bpp);
			return;
			break;
		}
		row_start += pitch;
	}
}

static void fill_image_nv12(struct image_config *img_cfg,
		void *buffer, int width, int height, int pitch)
{
	int y;
	int copy_w;
	int src_pitch;
	uint8_t *src, *dst;
	int size = 0;
	src = img_cfg->ptr;
	dst = (uint8_t*)buffer;
	height = height > img_cfg->h ? img_cfg->h : height;
	copy_w = (width > img_cfg->w ? img_cfg->w : width) * img_cfg->bpp / 8;
	src_pitch = img_cfg->w * img_cfg->bpp / 8;

	for (y = 0; y < height; ++y) {
		memcpy(dst, src, copy_w);
		src += src_pitch;
		dst += pitch;
		size += copy_w;
	}

        copy_w = width/2;
	for (y = 0; y < height; y++)
	{
		memcpy(dst, src, copy_w);
		src += copy_w;
		dst += copy_w;
		size += copy_w;
	}
}

static void fill_image(struct image_config *img_cfg,
		void *buffer, int width, int height, int pitch)
{
	int y;
	int copy_w;
	int src_pitch;
	uint8_t *src, *dst;
	int size = 0;
	src = img_cfg->ptr;
	dst = (uint8_t*)buffer;
	height = height > img_cfg->h ? img_cfg->h : height;
	copy_w = (width > img_cfg->w ? img_cfg->w : width) * img_cfg->bpp / 8;
	src_pitch = img_cfg->w * img_cfg->bpp / 8;

	for (y = 0; y < height; ++y) {
		memcpy(dst, src, copy_w);
		src += src_pitch;
		dst += pitch;
		size += copy_w;
	}

}

static void dump_blob(int fd, uint32_t blob_id)
{
	uint32_t i;
	unsigned char *blob_data;
	drmModePropertyBlobPtr blob;
	char temp[32] = {0}, *p;

	blob = drmModeGetPropertyBlob(fd, blob_id);
	if (!blob) {
		printf("\n");
		return;
	}

	blob_data = (unsigned char *)blob->data;
	if (!blob_data || !blob->length) {
		drmModeFreePropertyBlob(blob);
		printf("\n");
		return;
	}

	for (i = 0; i < blob->length; i++) {
		if (i % 16 == 0) {
			p = temp;
			printf("\n\t\t\t");
		}
		printf("%.2hhx", blob_data[i]);
		if (blob_data[i] < 0x20 || blob_data[i] > 0x7F)
			*p++ = '.';
		else
			*p++ = blob_data[i];
		if ((i % 16 == 15) || (i == blob->length - 1)) {
			*p = 0;
			printf("%*s  |%s%*s|", (15 - (i % 16)) * 2, "", temp, 15 - (i % 16), "");
		}
	}
	printf("\n");

	drmModeFreePropertyBlob(blob);
}

static void dump_fourcc(uint32_t fourcc)
{
	printf(" %c%c%c%c",
		fourcc,
		fourcc >> 8,
		fourcc >> 16,
		fourcc >> 24);
}

static void dump_in_formats(int fd, uint32_t blob_id)
{
	uint32_t i;
	drmModePropertyBlobPtr blob;
	struct drm_format_modifier_blob *header;
	uint32_t *formats;
	struct drm_format_modifier *modifiers;

	printf("\t\tin_formats blob decoded:\n");
	blob = drmModeGetPropertyBlob(fd, blob_id);
	if (!blob) {
		printf("\n");
		return;
	}

	header = (struct drm_format_modifier_blob *)blob->data;
	if (!header) {
		printf("no blob data!\n");
		drmModeFreePropertyBlob(blob);
		return;
	}
	formats = (uint32_t *) ((char *) header + header->formats_offset);
	modifiers = (struct drm_format_modifier *)
		((char *) header + header->modifiers_offset);

	for (i = 0; i < header->count_formats; i++) {
		printf("\t\t\t");
		dump_fourcc(formats[i]);
		printf("\n");
	}

	drmModeFreePropertyBlob(blob);
}

static inline int64_t U642I64(uint64_t val)
{
	return (int64_t)*((int64_t *)&val);
}

static void dump_prop(int fd, uint32_t prop_id, uint64_t value)
{
	int i;
	drmModePropertyPtr prop;

	printf("\t%d", prop_id);

	prop = drmModeGetProperty(fd, prop_id);
	if (!prop) {
		printf("\n");
		return;
	}

	printf(" %s:\n", prop->name);

	printf("\t\tflags:");
	if (prop->flags & DRM_MODE_PROP_PENDING)
		printf(" pending");
	if (prop->flags & DRM_MODE_PROP_IMMUTABLE)
		printf(" immutable");
	if (drm_property_type_is(prop, DRM_MODE_PROP_SIGNED_RANGE))
		printf(" signed range");
	if (drm_property_type_is(prop, DRM_MODE_PROP_RANGE))
		printf(" range");
	if (drm_property_type_is(prop, DRM_MODE_PROP_ENUM))
		printf(" enum");
	if (drm_property_type_is(prop, DRM_MODE_PROP_BITMASK))
		printf(" bitmask");
	if (drm_property_type_is(prop, DRM_MODE_PROP_BLOB))
		printf(" blob");
	if (drm_property_type_is(prop, DRM_MODE_PROP_OBJECT))
		printf(" object");
	printf("\n");

	if (drm_property_type_is(prop, DRM_MODE_PROP_SIGNED_RANGE)) {
		printf("\t\tvalues:");
		for (i = 0; i < prop->count_values; i++)
			if (prop->values)
				printf(" %" PRId64, U642I64(prop->values[i]));
		printf("\n");
	}

	if (drm_property_type_is(prop, DRM_MODE_PROP_RANGE)) {
		printf("\t\tvalues:");
		for (i = 0; i < prop->count_values; i++) {
			if (prop->values)
				printf(" %" PRIu64, prop->values[i]);
		}
		printf("\n");
	}

	if (drm_property_type_is(prop, DRM_MODE_PROP_ENUM)) {
		printf("\t\tenums:");
		for (i = 0; i < prop->count_enums; i++) {
			if (prop->enums)
				printf(" %s=%llu", prop->enums[i].name,
				       prop->enums[i].value);
		}
		printf("\n");
	} else if (drm_property_type_is(prop, DRM_MODE_PROP_BITMASK)) {
		printf("\t\tvalues:");
		for (i = 0; i < prop->count_enums; i++) {
			if (prop->enums)
				printf(" %s=0x%llx", prop->enums[i].name,
				       (1LL << prop->enums[i].value));
		}
		printf("\n");
	} else {
		assert(prop->count_enums == 0);
	}

	if (drm_property_type_is(prop, DRM_MODE_PROP_BLOB)) {
		printf("\t\tblobs:\n");
		for (i = 0; i < prop->count_blobs; i++) {
			if (prop->blob_ids)
				dump_blob(fd, prop->blob_ids[i]);
		}
	} else {
		assert(prop->count_blobs == 0);
	}

	printf("\t\tvalue:");
	if (drm_property_type_is(prop, DRM_MODE_PROP_BLOB))
		dump_blob(fd, value);
	else if (drm_property_type_is(prop, DRM_MODE_PROP_SIGNED_RANGE))
		printf(" %" PRId64"\n", value);
	else
		printf(" %" PRIu64"\n", value);

	if (strcmp(prop->name, "IN_FORMATS") == 0)
		dump_in_formats(fd, value);

	drmModeFreeProperty(prop);
}

static void dump_all_props(int fd, uint32_t obj_id, uint32_t type)
{
	int i;
	drmModeObjectPropertiesPtr props = drmModeObjectGetProperties(fd, obj_id, type);

	if (props) {
		printf("\tprops:\n");
		for (i = 0; i < props->count_props; i++)
			dump_prop(fd, props->props[i], props->prop_values[i]);
		drmModeFreeObjectProperties(props);
	}
}

int get_attached_crtc_id(int fd, uint32_t obj_id, uint32_t type)
{
	int i;
	uint32_t crtc_id = 0;
	drmModeObjectPropertiesPtr props = drmModeObjectGetProperties(fd, obj_id, type);

	if (props) {
		for (i = 0; (i < props->count_props) && !crtc_id; i++) {
			drmModePropertyPtr prop;
			prop = drmModeGetProperty(fd, props->props[i]);
			if (prop && !strcmp(prop->name, "CRTC_ID"))
				crtc_id = props->prop_values[i];
			drmModeFreeProperty(prop);
		}
		drmModeFreeObjectProperties(props);
	}

	return crtc_id;
}

static void dump(int fd, drmModeResPtr res, drmModePlaneResPtr plane_res, bool dump_props)
{
	int i, j;
	uint32_t crtc_mask = 0;
	uint32_t crtc_id;

	drmVersionPtr version = drmGetVersion(fd);
	if (version)
		printf("DRI %d-%d: %s - %s -%s\n", version->version_major, version->version_minor,
			version->name, version->date, version->desc);
	else
		printf("DRI (version not found!)\n");
	drmFreeVersion(version);
	printf("--------------\n");

	for (i = 0; i < res->count_connectors; i++) {
		drmModeConnectorPtr conn = drmModeGetConnector(fd, res->connectors[i]);
		if (conn) {
			crtc_id = get_attached_crtc_id(fd,
					conn->connector_id, DRM_MODE_OBJECT_CONNECTOR);

			printf("connector%d: %d connection:%d type:%d size:%dx%dmm\tencoders:", i,
				conn->connector_id, conn->connection, conn->connector_type,
				conn->mmWidth, conn->mmHeight);
			if (conn->encoders) {
				for (j = 0; j < conn->count_encoders; j++) {
					drmModeEncoderPtr enc = drmModeGetEncoder(fd, conn->encoders[j]);
					if (enc) {
						printf(" %d", enc->encoder_id);
						crtc_mask |= enc->possible_crtcs;
						drmModeFreeEncoder(enc);
					}
				}
			}
			printf("\tpossible_crtcs:");
			for (j = 0; j < res->count_crtcs; j++) {
				if (crtc_mask & (1 << j)) {
					if (res->crtcs[j] == crtc_id)
						printf(" <%d>", res->crtcs[j]);
					else
						printf(" %d", res->crtcs[j]);
				}
			}
			printf("\tmode:");
			for (j = 0; j < conn->count_modes; j++) {
				if (conn->modes)
					printf(" %s@%d", conn->modes[j].name, conn->modes[j].vrefresh);
				else
					printf(" mode invalid");
			}
			printf("\n");

			if (dump_props)
				dump_all_props(fd, conn->connector_id, DRM_MODE_OBJECT_CONNECTOR);

			drmModeFreeConnector(conn);
		}
	}
	printf("--------------\n");

	for (i = 0; i < res->count_crtcs; i++) {
		drmModeCrtcPtr crtc = drmModeGetCrtc(fd, res->crtcs[i]);
		if (crtc) {
			printf("crtc%d: %d fb:%d", i, crtc->crtc_id, crtc->buffer_id);
			printf("\tmode:");
			printf(" %s@%d\n", crtc->mode.name, crtc->mode.vrefresh);

			if (dump_props)
				dump_all_props(fd, crtc->crtc_id, DRM_MODE_OBJECT_CRTC);

			drmModeFreeCrtc(crtc);
		}
	}
	printf("--------------\n");

	for (i = 0; i < (int)plane_res->count_planes; i++) {
		drmModePlanePtr plane = drmModeGetPlane(fd, plane_res->planes[i]);
		if (plane) {
			crtc_id = get_attached_crtc_id(fd,
					plane->plane_id, DRM_MODE_OBJECT_PLANE);

			printf("plane%d: %d - possible_crtcs:", i, plane->plane_id);
			for (j = 0; j < res->count_crtcs; j++) {
				if (plane->possible_crtcs & (1 << j)) {
					if (res->crtcs[j] == crtc_id)
						printf(" <%d>", res->crtcs[j]);
					else
						printf(" %d", res->crtcs[j]);
				}
			}

			drmModeObjectPropertiesPtr props =
				drmModeObjectGetProperties(fd, plane->plane_id, DRM_MODE_OBJECT_PLANE);
			if (props) {
				for (j = 0; j < (int)props->count_props; j++) {
					drmModePropertyPtr prop = drmModeGetProperty(fd, props->props[j]);
					if (prop) {
						if (!strcmp(prop->name, "handoff")) {
							printf(" handoff=%d", (int)props->prop_values[j]);
						}
						drmModeFreeProperty(prop);
					}
				}
			}
			drmModeFreeObjectProperties(props);

			printf("\n");

			if (dump_props)
				dump_all_props(fd, plane->plane_id, DRM_MODE_OBJECT_PLANE);

			drmModeFreePlane(plane);
		}
	}
	printf("--------------\n");
}

static void update_possible_crtcs(int fd, std::vector<plane_config> &plane_cfg)
{
	uint32_t j;

	/* update possible_crtcs on errors */
	for (j = 0; j < plane_cfg.size(); j++) {
		drmModePlanePtr plane = drmModeGetPlane(fd, plane_cfg[j].plane_id);
		if (plane) {
			if (plane_cfg[j].possible_crtcs != plane->possible_crtcs) {
				DEBUGT_PRINT_INFO("plane%d possible_crtcs 0x%x => 0x%x\n",
					plane_cfg[j].plane_id,
					plane_cfg[j].possible_crtcs,
					plane->possible_crtcs);
				plane_cfg[j].possible_crtcs = plane->possible_crtcs;
			}

			plane_cfg[j].first_run = true;

			drmModeFreePlane(plane);
		}
	}
}

int get_connectors(int fd, drmModeRes *res, std::vector<connector_config> &connector_cfg)
{
	int i, j;

	if (!connector_cfg.size())
		build_default_connector(fd, res, connector_cfg);

	for (i = 0; i < (int)connector_cfg.size(); i++) {
		drmModeConnectorPtr conn = NULL;

		for (j = 0; j < res->count_connectors; j++) {
			drmModeConnectorPtr tmp = drmModeGetConnector(fd, res->connectors[j]);
			if (tmp && tmp->connector_id == connector_cfg[i].connector_id) {
				conn = tmp;
				break;
			}
			drmModeFreeConnector(tmp);
		}

		if (!conn) {
			printf("failed to find connector %d\n", connector_cfg[i].connector_id);
			return EINVAL;
		}

		/* store property id */
		drmModeObjectPropertiesPtr props =
			drmModeObjectGetProperties(fd, conn->connector_id,
				DRM_MODE_OBJECT_CONNECTOR);
		if (props) {
			for (j = 0; j < (int)props->count_props; j++) {
				drmModePropertyPtr prop = drmModeGetProperty(fd, props->props[j]);
				if (prop) {
					if (!strcmp(prop->name, "CRTC_ID"))
						connector_cfg[i].crtc_id_prop = prop->prop_id;
				}
			}
		}

		/* copy mode */
		for (j = 0; j < conn->count_modes; j++) {
			if (conn->modes && !strcmp(connector_cfg[i].mode.name, conn->modes[j].name)) {
				if (connector_cfg[i].mode.vrefresh)
					if (connector_cfg[i].mode.vrefresh !=
						conn->modes[j].vrefresh)
						continue;
				memcpy(&connector_cfg[i].mode,
					&conn->modes[j], sizeof(drmModeModeInfo));
				break;
			}
		}
		if (j == conn->count_modes) {
			DEBUGT_PRINT_ERROR("failed to find mode %s\n", connector_cfg[i].mode.name);
			drmModeFreeConnector(conn);
			return EINVAL;
		}

		build_roi_misr(connector_cfg[i]);

		drmModeFreeConnector(conn);
	}

	for (i = 0; i < (int)connector_cfg.size(); i++) {
		drmModeCrtcPtr crtc = NULL;

		for (j = 0; j < res->count_crtcs; j++) {
			drmModeCrtcPtr tmp = drmModeGetCrtc(fd, res->crtcs[j]);
			if (tmp) {
				if (tmp->crtc_id == connector_cfg[i].crtc_id) {
					crtc = tmp;
					break;
				}
				drmModeFreeCrtc(tmp);
			}
		}

		if (!crtc) {
			printf("failed to find crtc %d\n", connector_cfg[i].crtc_id);
			return 0;
		}

		/* store crtc index */
		connector_cfg[i].crtc_idx = j;

		/* store property id */
		drmModeObjectPropertiesPtr props =
			drmModeObjectGetProperties(fd, crtc->crtc_id, DRM_MODE_OBJECT_CRTC);
		if (props) {
			for (j = 0; j < (int)props->count_props; j++) {
				drmModePropertyPtr prop = drmModeGetProperty(fd, props->props[j]);
				if (prop) {
					if (!strcmp(prop->name, "ACTIVE"))
						connector_cfg[i].active_prop = prop->prop_id;
					else if (!strcmp(prop->name, "MODE_ID"))
						connector_cfg[i].mode_id_prop = prop->prop_id;
					else if (!strcmp(prop->name, "roi_misr"))
						connector_cfg[i].roi_misr_id_prop = prop->prop_id;
				}
			}
		}

		if (!connector_cfg[i].roi_misr_id_prop && connector_cfg[i].roi_misr.num_rois) {
			DEBUGT_PRINT_ERROR("failed to find roi_misr property for conn%d\n",
					connector_cfg[i].connector_id);
		}

		drmModeFreeCrtc(crtc);
	}

	return 0;
}

int get_planes(int fd, drmModePlaneRes *plane_res,
		std::vector<connector_config> &connector_cfg,
		std::vector<plane_config> &plane_cfg)
{
	int i, j;

	if (!plane_cfg.size())
		build_default_plane(fd, plane_res, connector_cfg, plane_cfg);

	for (i = 0; i < (int)plane_cfg.size(); i++) {
		drmModePlanePtr plane = NULL;

		for (j = 0; j < (int)plane_res->count_planes; j++) {
			drmModePlanePtr tmp = drmModeGetPlane(fd, plane_res->planes[j]);
			if (tmp && tmp->plane_id == plane_cfg[i].plane_id) {
				plane = tmp;
				break;
			}
			drmModeFreePlane(tmp);
		}

		if (!plane) {
			printf("failed to find plane %d\n", plane_cfg[i].plane_id);
			return EINVAL;
		}

		/* store property id */
		drmModeObjectPropertiesPtr props =
			drmModeObjectGetProperties(fd, plane->plane_id, DRM_MODE_OBJECT_PLANE);
		if (props) {
			for (j = 0; j < (int)props->count_props; j++) {
				drmModePropertyPtr prop = drmModeGetProperty(fd, props->props[j]);
				if (!prop) {
					printf("\nWarning: Property is null.\n");
					continue;
				}
				if (!strcmp(prop->name, "FB_ID"))
					plane_cfg[i].fb_prop = prop->prop_id;
				else if (!strcmp(prop->name, "CRTC_ID"))
					plane_cfg[i].crtc_prop = prop->prop_id;
				else if (!strcmp(prop->name, "zpos"))
					plane_cfg[i].zpos_prop = prop->prop_id;
				else if (!strcmp(prop->name, "CRTC_X"))
					plane_cfg[i].crtc_x_prop = prop->prop_id;
				else if (!strcmp(prop->name, "CRTC_Y"))
					plane_cfg[i].crtc_y_prop = prop->prop_id;
				else if (!strcmp(prop->name, "CRTC_W"))
					plane_cfg[i].crtc_w_prop = prop->prop_id;
				else if (!strcmp(prop->name, "CRTC_H"))
					plane_cfg[i].crtc_h_prop = prop->prop_id;
				else if (!strcmp(prop->name, "SRC_X"))
					plane_cfg[i].src_x_prop = prop->prop_id;
				else if (!strcmp(prop->name, "SRC_Y"))
					plane_cfg[i].src_y_prop = prop->prop_id;
				else if (!strcmp(prop->name, "SRC_W"))
					plane_cfg[i].src_w_prop = prop->prop_id;
				else if (!strcmp(prop->name, "SRC_H"))
					plane_cfg[i].src_h_prop = prop->prop_id;
				else if (!strcmp(prop->name, "handoff"))
					plane_cfg[i].handoff_prop = prop->prop_id;
				else if (!strcmp(prop->name, "rotation"))
					plane_cfg[i].rotation_prop = prop->prop_id;
				else if (!strcmp(prop->name, "csc_v1"))
					plane_cfg[i].csc = prop->prop_id;
			}
		}

		/* update possible_crtcs */
		plane_cfg[i].possible_crtcs = plane->possible_crtcs;

		drmModeFreePlane(plane);
	}

	return 0;
}

int create_framebuffers(int fd, std::vector<plane_config> &plane_cfg,
		int num_fbs, std::vector<image_config> &img_cfg, bool import)
{
	int i, j;
	int ret = 0;
	plane_config *plane;
	image_config *img;

	for (i = 0; i < (int)plane_cfg.size(); i++) {
		plane = &plane_cfg[i];

		plane->num_fbs = num_fbs;
		plane->fbs = (struct fb_obj *)calloc(num_fbs, sizeof(struct fb_obj));
		if (plane->fbs == NULL) {
			DEBUGT_PRINT_ERROR("Error: failed to allocate "\
						"memory to frame!\n");
			continue;
		}
		plane->fb_index = 0;
		plane->last_fb_index = -1;

		img = NULL;
		for (j = 0; j < (int)img_cfg.size(); j ++) {
			if (img_cfg[j].plane_id == plane->plane_id) {
				img = &img_cfg[j];
				break;
			}
		}
		if (img == NULL) {
			for (j = 0; j < (int)img_cfg.size(); j ++) {
				if (img_cfg[j].plane_id == -1) {
					img = &img_cfg[j];
					break;
				}
			}
		}

		for (j = 0; j < plane->num_fbs; j ++) {
			if (img) {
				if (img->ptr) {
					plane->fbs[j].bpp = img->bpp;
					plane->fbs[j].format = img->format;
					plane->animation = false;
				} else {
					plane->fbs[j].bpp = img->bpp;
					plane->fbs[j].format = img->format;
					plane->animation = false;
				}
			} else {
				plane->fbs[j].bpp = 32;
				plane->fbs[j].format = DRM_FORMAT_ARGB8888;
				plane->animation = true;
			}

			if (img && (img->flag == IMAGE_FLAG_UBWC)) {
				if (!is_ubwc_format(img->format)) {
					DEBUGT_PRINT_ERROR("The UBWC flag is not matching with format.\n");
					return ret;
				}
				plane->fbs[j].modifier[0] = DRM_FORMAT_MOD_QCOM_COMPRESSED;
			}

			if ((ret = create_fb_obj(fd, &plane_cfg[i], j)))
				return ret;
			if (import) {
				if ((ret = import_fb_obj(fd, &plane_cfg[i], j))) {
					destroy_fb_obj(&plane->fbs[j]);
					return ret;
				}
			}

			if ((ret = map_fb_obj(&plane->fbs[j]) == NULL))
				return ret;

			if (img) {
				if (img->ptr) {
					if (is_ubwc_enable(&plane->fbs[j])) {
						cpu_access_begin(fd);
						FILE *fp = fopen(img->filename, "rb");
						if (fp) {
							size_t bytes_read  = fread(plane->fbs[j].ptr, 1, plane->fbs[j].size, fp);
							if (bytes_read != plane->fbs[j].size)
								DEBUGT_PRINT_INFO("Warning: only %d of %d bytes read from file %s\n",
									(int)bytes_read, (int)plane->fbs[j].size, img->filename);
							fclose(fp);
						} else {
							DEBUGT_PRINT_ERROR("Failed to open file %s\n", img->filename);
						}
						cpu_access_end(fd);
					} else {
						/* copy from image */
						if (plane->fbs[j].format != DRM_FORMAT_NV12)
							fill_image(img, plane->fbs[j].ptr,
								plane->fbs[j].width,
								plane->fbs[j].height,
								plane->fbs[j].pitch);
						else
							fill_image_nv12(img, plane->fbs[j].ptr,
								plane->fbs[j].width,
								plane->fbs[j].height,
								plane->fbs[j].pitch);
					}
				} else {
					/* fill with solid color */
					fill_color(plane->fbs[j].ptr,
							(img->w > plane->fbs[j].width) ? plane->fbs[j].width : img->w,
							(img->h > plane->fbs[j].height) ? plane->fbs[j].height : img->h,
							plane->fbs[j].pitch, img->color, img->bpp);
				}

				if (img->plane_id == -1) {
					// Mark the image has been used
					img->plane_id = 0;
				}
			} else {
				/* fill with tile pattern */
				build_pattern(plane->fbs[j].ptr,
						plane->fbs[j].width, plane->fbs[j].height,
						plane->fbs[j].pitch,
						j * 40, 0);
			}

			/* flush cache */
			unmap_fb_obj(&plane->fbs[j]);

			if (import) {
				/* create fbid */
				if ((ret = create_fb_id(fd, &plane_cfg[i], j)))
					return ret;
			}
		}

		/* first run flag */
		plane->first_run = true;
	}

	return 0;
}

int destroy_framebuffers(int fd, plane_config *plane_cfg)
{
	int i;
	int ret;

	for (i = 0; i < plane_cfg->num_fbs; i ++) {
		if (plane_cfg->fbs[i].fb_id)
			if ((ret = remove_fb_id(fd, plane_cfg, i)))
				return ret;

		if (plane_cfg->fbs[i].handle)
			if ((ret = unimport_fb_obj(fd, plane_cfg, i)))
				return ret;

		if ((ret = destroy_fb_obj(&plane_cfg->fbs[i])))
			return ret;
	}

	return 0;
}

int atomic_commit(int fd, drmModeAtomicReqPtr &req, bool async)
{
	int ret = drmModeAtomicCommit(fd, req,
		DRM_MODE_ATOMIC_ALLOW_MODESET | (async ? DRM_MODE_ATOMIC_NONBLOCK : 0), 0);
	if (ret)
		return ret;

	/* clear previous commit */
	drmModeAtomicSetCursor(req, 0);

	return 0;
}

int setup_connector_roi_misr(drmModeAtomicReqPtr &req, struct connector_config &cfg)
{
	if (!cfg.roi_misr_data.roi_rect_num || !cfg.roi_misr_id_prop)
		return 0;

	//DEBUGT_PRINT_INFO("Setup Conn%d ROI MISR\n", cfg.connector_id);
	return drmModeAtomicAddProperty(req, cfg.crtc_id,
		cfg.roi_misr_id_prop, (uint64_t)&cfg.roi_misr_data);
}

int get_misr_signature(struct connector_config &cfg)
{
	if (!cfg.roi_misr_data.roi_rect_num || !cfg.roi_misr_id_prop)
		return -EINVAL;

	//DEBUGT_PRINT_INFO("roi_misr fence fd: %lld\n", *cfg.roi_misr_data.fence_fd_ptr);
	//DEBUGT_PRINT_INFO("set roi_num = %d\n", cfg.roi_misr_data.roi_rect_num);

	if (*cfg.roi_misr_data.fence_fd_ptr >= 0) {
		struct pollfd poll_fd;
		poll_fd.fd = (int)*cfg.roi_misr_data.fence_fd_ptr;
		poll_fd.events = POLLIN;

		int ret = poll(&poll_fd, 1, -1);
		if (ret < 0) {
			perror("poll error!\n");
			return ret;
		} else if (ret == 0) {
			perror("poll timeout\n");
			return -ETIMEDOUT;
		}

		if (poll_fd.revents == POLLIN) {
			ret = read(*cfg.roi_misr_data.fence_fd_ptr,
					cfg.roi_misr.crc_value, sizeof(cfg.roi_misr.crc_value));
			//DEBUGT_PRINT_INFO("ret = %d\n", ret);
			DEBUGT_PRINT_INFO("Conn%d ROI MISR:", cfg.connector_id);
			for (int j = 0; j < cfg.roi_misr_data.roi_rect_num; j++)
				DEBUGT_PRINT_INFO(" %8.8X", cfg.roi_misr.crc_value[j]);
			DEBUGT_PRINT_INFO("\n");
		}

		close(*cfg.roi_misr_data.fence_fd_ptr);
	}

	return 0;
}

#ifdef QUERY_ROI_MISR_IN_THREAD
void *roi_misr_thread(void *parg)
{
	struct connector_config *cfg = (struct connector_config *)parg;
	char card_path[256];
	int ret;
	drmModeAtomicReqPtr req = drmModeAtomicAlloc();

	DEBUGT_PRINT_INFO("Start thread for conn%d\n", cfg->connector_id);
	while (!quit)
	{
		get_misr_signature(*cfg);
	}

	DEBUGT_PRINT_INFO("Stop thread for conn%d\n", cfg->connector_id);

	return NULL;
}

int start_roi_misr_thread(connector_config &cfg)
{
	int ret;

	// Check if fence fd is available
	if (*cfg.roi_misr_data.fence_fd_ptr == -1)
		return 0;

	ret = pthread_create(&cfg.roi_misr.roi_misr_sig_thread, NULL, roi_misr_thread, &cfg);
	if (ret)
	{
		DEBUGT_PRINT_ERROR("Failed to create thread for conn%d ROI MISR\n", cfg.connector_id);
		return ret;
	}

	DEBUGT_PRINT_INFO("Create thread for conn%d\n", cfg.connector_id);

	return 0;
}
#endif

int setup_connector_crtc(int fd, std::vector<connector_config> &connector_cfg,
		drmModeAtomicReqPtr &req, bool batch)
{
	int i;
	int ret;

	/* setup crtc / connector */
	for (i = 0; i < (int)connector_cfg.size(); i++) {
		/* set mode */
		if (drmModeCreatePropertyBlob(fd, (const void *)&connector_cfg[i].mode,
			sizeof(drmModeModeInfo), &connector_cfg[i].mode_id)) {
			printf("failed to create mode blob\n");
			return 0;
		}
		drmModeAtomicAddProperty(req, connector_cfg[i].crtc_id,
			connector_cfg[i].mode_id_prop, connector_cfg[i].detach ?
				0 : connector_cfg[i].mode_id);

		/* set active */
		drmModeAtomicAddProperty(req, connector_cfg[i].crtc_id,
			connector_cfg[i].active_prop, connector_cfg[i].active);

		/* set crtc */
		drmModeAtomicAddProperty(req, connector_cfg[i].connector_id,
			connector_cfg[i].crtc_id_prop, connector_cfg[i].detach ?
				0 : connector_cfg[i].crtc_id);

		setup_connector_roi_misr(req, connector_cfg[i]);

		if (batch)
			continue;

		/* init commit */
		if ((ret = atomic_commit(fd, req, true))) {
			DEBUGT_PRINT_ERROR("init commit failed: %s\n", strerror(errno));
			return ret;
		}
	}

	if (batch) {
		if ((ret = atomic_commit(fd, req, true))) {
			DEBUGT_PRINT_ERROR("init commit failed: %s\n", strerror(errno));
			return ret;
		}
	}


#ifdef QUERY_ROI_MISR_IN_THREAD
	/* start ROI MISR threads */
	for (i = 0; i < (int)connector_cfg.size(); i++)
		start_roi_misr_thread(connector_cfg[i]);
#endif

	return 0;
}

int populate_plane(drmModeAtomicReqPtr &req, plane_config *plane_cfg)
{
	drmModeAtomicAddProperty(req,
		plane_cfg->plane_id,
		plane_cfg->crtc_prop,
		plane_cfg->crtc_id);
	drmModeAtomicAddProperty(req,
		plane_cfg->plane_id,
		plane_cfg->zpos_prop,
		plane_cfg->zpos);
	drmModeAtomicAddProperty(req,
		plane_cfg->plane_id,
		plane_cfg->crtc_x_prop,
		plane_cfg->x);
	drmModeAtomicAddProperty(req,
		plane_cfg->plane_id,
		plane_cfg->crtc_y_prop,
		plane_cfg->y);
	drmModeAtomicAddProperty(req,
		plane_cfg->plane_id,
		plane_cfg->crtc_w_prop,
		plane_cfg->w * plane_cfg->scale);
	drmModeAtomicAddProperty(req,
		plane_cfg->plane_id,
		plane_cfg->crtc_h_prop,
		plane_cfg->h * plane_cfg->scale);
	drmModeAtomicAddProperty(req,
		plane_cfg->plane_id,
		plane_cfg->src_x_prop,
		plane_cfg->crop_x << 16);
	drmModeAtomicAddProperty(req,
		plane_cfg->plane_id,
		plane_cfg->src_y_prop,
		plane_cfg->crop_y << 16);
	drmModeAtomicAddProperty(req,
		plane_cfg->plane_id,
		plane_cfg->src_w_prop,
		plane_cfg->crop_w << 16);
	drmModeAtomicAddProperty(req,
		plane_cfg->plane_id,
		plane_cfg->src_h_prop,
		plane_cfg->crop_h << 16);
	drmModeAtomicAddProperty(req,
		plane_cfg->plane_id,
		plane_cfg->rotation_prop,
		plane_cfg->rotation);

	/* only set handoff once */
	if (plane_cfg->handoff_set &&
		plane_cfg->handoff_prop) {
		drmModeAtomicAddProperty(req,
			plane_cfg->plane_id,
			plane_cfg->handoff_prop,
			plane_cfg->handoff);
		plane_cfg->handoff_set = false;
	}

	return 0;
}

void set_csc_data(drmModeAtomicReqPtr &req, plane_config *plane_cfg, int32_t csc_type)
{
	int ret = 0;
	if (csc_type < kCscTypeMax)
		ret = drmModeAtomicAddProperty(req, plane_cfg->plane_id,
				plane_cfg->csc,
				(uint64_t)&csc_10bit_convert[csc_type]);
	if (ret < 0)
		DEBUGT_PRINT_ERROR("drmModeAtomicAddProperty for CSC failed\n");
}

int close_connector_crtc(int fd, drmModeAtomicReqPtr &req,
		std::vector<connector_config> &connector_cfg,
		std::vector<plane_config> &plane_cfg,
		bool batch)
{
	int i, j;
	int ret;

	/* close crtc / connector */
	for (i = 0; i < (int)connector_cfg.size(); i++) {
		/* reset rotation_prop first */
		for (j = 0; j < (int)plane_cfg.size(); j++) {
			if (plane_cfg[j].crtc_id == connector_cfg[i].crtc_id) {
				/* check possible_crtcs */
				if (!(plane_cfg[j].possible_crtcs &
						(1 << connector_cfg[i].crtc_idx)))
					continue;

				drmModeAtomicAddProperty(req,
						plane_cfg[j].plane_id,
						plane_cfg[j].rotation_prop,
						DRM_MODE_ROTATE_0);
			}
		}
		if (batch)
			continue;
		if ((ret = atomic_commit(fd, req, false))) {
			DEBUGT_PRINT_ERROR("final commit failed: %s\n", strerror(errno));
			return ret;
		}

		drmModeAtomicAddProperty(req, connector_cfg[i].crtc_id,
			connector_cfg[i].active_prop, 0);

		drmModeAtomicAddProperty(req, connector_cfg[i].crtc_id,
			connector_cfg[i].mode_id_prop, 0);

		drmModeAtomicAddProperty(req, connector_cfg[i].connector_id,
			connector_cfg[i].crtc_id_prop, 0);

		for (j = 0; j < (int)plane_cfg.size(); j++) {
			if (plane_cfg[j].crtc_id == connector_cfg[i].crtc_id) {
				/* check possible_crtcs */
				if (!(plane_cfg[j].possible_crtcs &
						(1 << connector_cfg[i].crtc_idx)))
					continue;

				if (plane_cfg[j].handoff_prop && plane_cfg[j].handoff_on_exit) {
					drmModeAtomicAddProperty(req,
							plane_cfg[j].plane_id,
							plane_cfg[j].handoff_prop,
							1);
				}

				drmModeAtomicAddProperty(req,
						plane_cfg[j].plane_id,
						plane_cfg[j].crtc_prop,
						0);

				drmModeAtomicAddProperty(req,
						plane_cfg[j].plane_id,
						plane_cfg[j].fb_prop,
						0);
			}
		}

		drmModeDestroyPropertyBlob(fd, connector_cfg[i].mode_id);

		if (batch)
			continue;

		if ((ret = atomic_commit(fd, req, false))) {
			DEBUGT_PRINT_ERROR("final commit failed: %s\n", strerror(errno));
			return ret;
		}
	}

	if (batch) {
		if ((ret = atomic_commit(fd, req, false))) {
			DEBUGT_PRINT_ERROR("final commit failed: %s\n", strerror(errno));
			return ret;
		}
	}

	/* close roi misr */
	for (i = 0; i < (int)connector_cfg.size(); i++) {
		if (connector_cfg[i].roi_misr_data.fence_fd_ptr)
			free(connector_cfg[i].roi_misr_data.fence_fd_ptr);
	}

	return 0;
}

static int terminate = 1;

static void sigint_handler(int arg)
{
	arg = 0;
	terminate = 0;
	DEBUGT_PRINT_INFO("terminate\n");
}

int main(int argc, char **argv)
{
	int c;
	int card = 0;
	drmModeRes *res = NULL;
	drmModePlaneRes *plane_res = NULL;
	std::vector<plane_config> plane_cfg;
	std::vector<connector_config> connector_cfg;
	int fd = 0;
	int i, j;
	int ret = 0;
	bool list = false;
	bool list_props = false;
	bool pause = true;
	bool batch = false;
	bool cache_fb = true;
	std::vector<image_config> img_cfg;
	bool check_fps = false;
	int num_fbs = 1;
	int delay_ms = -1;
	int frames, last_frames;
	int64_t start_time, end_time;
	drmModeAtomicReqPtr req = NULL;
	int32_t csc = -1;

	while ((c = getopt(argc, argv, "c:s:p:t:lLi:bd:fF:Cm:S:")) != -1) {
		switch (c) {
		case 'c':
			card = optarg[0] - '0';
			break;
		case 's': {
			connector_config cfg = {};
			if (parse_connector(&cfg, optarg) < 0)
				usage(argv[0]);
			connector_cfg.push_back(cfg);
			break;
		}
		case 'p': {
			plane_config cfg = {};
			if (parse_plane(&cfg, optarg) < 0)
				usage(argv[0]);
			plane_cfg.push_back(cfg);
			break;
		}
		case 't':
			terminate = atoi(optarg);
			pause = false;
			signal(SIGINT, sigint_handler);
			break;
		case 'l':
			list = true;
			break;
		case 'L':
			list = true;
			list_props = true;
			break;
		case 'i':
			image_config cfg;
			cfg.plane_id = -1;
			cfg.flag = 0;
			if (parse_image(&cfg, optarg) < 0)
				usage(argv[0]);
			img_cfg.push_back(cfg);
			break;
		case 'b':
			batch = true;
			break;
		case 'd':
			delay_ms = atol(optarg);
			break;
		case 'f':
			check_fps = true;
			break;
		case 'F':
			num_fbs = atol(optarg);
			break;
		case 'C':
			cache_fb = false;
			break;
		case 'm':
			if (parse_roi_misr(connector_cfg, optarg) < 0)
				usage(argv[0]);
			break;
		case 'S':
			csc = atoi(optarg);
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	char card_path[256];
	snprintf(card_path, sizeof(card_path), "/dev/dri/card%d", card);
	fd = open(card_path, O_RDWR, 0);
	if (fd < 0) {
		DEBUGT_PRINT_ERROR("failed to open %s\n", card_path);
		return 0;
	}

	drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
	drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);

	res = drmModeGetResources(fd);
	if (!res) {
		DEBUGT_PRINT_ERROR("failed to get resources\n");
		goto ret_free_res;
	}

	plane_res = drmModeGetPlaneResources(fd);
	if (!plane_res) {
		DEBUGT_PRINT_ERROR("failed to get plane resources\n");
		goto ret_free_res;
	}

	if (list) {
		dump(fd, res, plane_res, list_props);
		if (!pause) {
			printf("press any key to exit\n");
			getchar();
		}
		goto ret_free_res;
	}

	ret = get_connectors(fd, res, connector_cfg);
	if (ret)
		goto ret_free_res;

	ret = get_planes(fd, plane_res, connector_cfg, plane_cfg);
	if (ret)
		goto ret_free_res;

	drmModeFreePlaneResources(plane_res);
	plane_res = NULL;
	drmModeFreeResources(res);
	res = NULL;
	/* create framebuffer */
	ret = create_framebuffers(fd, plane_cfg, num_fbs, img_cfg, cache_fb);
	if (ret)
		goto ret_free_res;

	/* init atomic commit */
	req = drmModeAtomicAlloc();

	/* setup crtc / connector */
	if ((ret = setup_connector_crtc(fd, connector_cfg, req, batch))) {
		quit = true;
		goto ret_free_res;
	}

	start_time = get_timestamp_ms();
	frames = 0;
	last_frames = 0;
	/* loop */
	while (terminate > 0) {
		/* update per crtc */
		for (i = 0; i < (int)connector_cfg.size(); i++) {
			/* update fb */
			for (j = 0; j < (int)plane_cfg.size(); j++) {
				if (plane_cfg[j].crtc_id == connector_cfg[i].crtc_id) {
					/* check possible_crtcs */
					if (!(plane_cfg[j].possible_crtcs &
							(1 << connector_cfg[i].crtc_idx)))
						continue;

					/* first run */
					if (plane_cfg[j].first_run) {
						populate_plane(req, &plane_cfg[j]);
						plane_cfg[j].first_run = false;
					}

					if (!cache_fb) {
						int index;

						index = plane_cfg[j].last_fb_index;
						if (index >= 0) {
							/* remvoe fbid */
							if ((ret = remove_fb_id(fd, &plane_cfg[j], index)))
								goto ret_free_res;
							else
								plane_cfg[j].fbs[index].fb_id = 0;

							/* umimport */
							if ((ret = unimport_fb_obj(fd, &plane_cfg[j], index)))
								goto ret_free_res;
							else
								plane_cfg[j].fbs[index].handle = 0;
						}

						index = plane_cfg[j].fb_index;
						/* import */
						if ((ret = import_fb_obj(fd, &plane_cfg[j], index)))
							goto ret_free_res;

						/* create fbid */
						if ((ret = create_fb_id(fd, &plane_cfg[j], index)))
							goto ret_free_res;
					}

					/* update fb index */
					drmModeAtomicAddProperty(req,
						plane_cfg[j].plane_id,
						plane_cfg[j].fb_prop,
						plane_cfg[j].fbs[plane_cfg[j].fb_index].fb_id);
					plane_cfg[j].last_fb_index = plane_cfg[j].fb_index;
					plane_cfg[j].fb_index = (plane_cfg[j].fb_index + 1) % plane_cfg[j].num_fbs;

					if (plane_cfg[j].animation) {
						/* update y offset for animation */
						drmModeAtomicAddProperty(req,
							plane_cfg[j].plane_id,
							plane_cfg[j].src_y_prop,
							plane_cfg[j].y_offset << 16);

						if (plane_cfg[j].y_offset == 0)
							plane_cfg[j].y_increment = 1;
						else if (plane_cfg[j].y_offset == (plane_cfg[j].h >> 1))
							plane_cfg[j].y_increment = -1;
						plane_cfg[j].y_offset += plane_cfg[j].y_increment;
					}
				} else if (!plane_cfg[j].crtc_id) {
					if (plane_cfg[j].handoff_prop && plane_cfg[j].handoff_set) {
						drmModeAtomicAddProperty(req,
								plane_cfg[j].plane_id,
								plane_cfg[j].handoff_prop,
								plane_cfg[j].handoff);
					}
				}

				if (csc > 0)
					set_csc_data(req, &plane_cfg[j], csc);
			}

			setup_connector_roi_misr(req, connector_cfg[i]);

			if (batch)
				continue;

			/* asynced commit */
			if ((ret = atomic_commit(fd, req, true))) {
				DEBUGT_PRINT_ERROR("commit failed: %s\n", strerror(errno));
				update_possible_crtcs(fd, plane_cfg);
				goto ret_free_res;
			}
		}

		if (batch) {
			/* asynced commit */
			if ((ret = atomic_commit(fd, req, true))) {
				DEBUGT_PRINT_ERROR("commit failed: %s\n", strerror(errno));
				goto ret_free_res;
			}
		}

#ifndef QUERY_ROI_MISR_IN_THREAD
		for (i = 0; i < (int)connector_cfg.size(); i++)
			get_misr_signature(connector_cfg[i]);
#endif

		if (check_fps) {
			frames ++;
			end_time = get_timestamp_ms();
			if (end_time - start_time >= 1000) {
				printf("Frames %3d\tTime %4ldms\tFPS %6.2f\r", frames - last_frames,
						end_time - start_time,
						(frames - last_frames) * 1000.0 / (end_time - start_time));
				fflush(stdout);
				start_time = end_time;
				last_frames = frames;
			}
		}

		/* sleep for 15ms */
		if (delay_ms > 0)
			usleep(delay_ms * 1000);

		--terminate;
	}

	if (pause) {
		printf("press any key to exit\n");
		getchar();
	}

	quit = true;

	/* close crtc / connector */
	if ((ret = close_connector_crtc(fd, req, connector_cfg, plane_cfg, batch)))
		goto ret_free_res;

	/* clear commit */
	drmModeAtomicFree(req);
	req = NULL;

	/* destroy buffer */
	for (i = 0; i < (int)plane_cfg.size(); i++) {
		for (j = 0; j < plane_cfg[i].num_fbs; j ++) {
			if ((ret = destroy_framebuffers(fd, &plane_cfg[i])))
				goto ret_free_res;
		}
	}

	/* destroy image */
	for (i = 0; i < (int)img_cfg.size(); i++) {
		if (img_cfg[i].ptr)
			free(img_cfg[i].ptr);
	}

	/* close fd */
	close(fd);

	return 0;

	ret_free_res:
		if(plane_res)
			drmModeFreePlaneResources(plane_res);
		if(res)
			drmModeFreeResources(res);
		if(req)
			drmModeAtomicFree(req);
		close(fd);
	return ret;
}

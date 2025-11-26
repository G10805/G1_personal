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
#include "VideoRender.h"

/* Global variables */
extern std::vector<connector_config> connector_cfg;
extern std::vector<plane_config> plane_cfg;
extern int fd;
extern std::mutex plane_mutex;
extern drmModeAtomicReqPtr req;

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
 * update_possible_crtcs()
 * description: update the possible crtc for planes
 ***************************************************/
void update_possible_crtcs(void)
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
int parse_display(void)
{
    int count_connectors = 0;
    int count_crtcs = 0;
    int count_planes = 0;
    int i = 0;
    int j = 0;
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
        drmModeConnectorPtr conn = drmModeGetConnector(fd,
                                       res->connectors[i]);

        if (!conn || !conn->encoders)
            goto out;

        if (conn->connection != DRM_MODE_CONNECTED)
            continue;

        /* dump */
        DEBUGT_PRINT_ERROR("connector%d: %d - %s-%d crtcs:",
                count_connectors, conn->connector_id,
                connector_type_names[conn->connector_type],
                conn->connector_type_id);

        for (j = 0; j < conn->count_encoders; j++) {
            drmModeEncoderPtr enc = drmModeGetEncoder(fd,
                                 conn->encoders[j]);
            if (!enc)
               goto out;

            crtc_mask |= enc->possible_crtcs;
        }

        for (j = 0; j < res->count_crtcs; j++) {
            if (crtc_mask & (1 << j)) {
               DEBUGT_PRINT_ERROR(" %d", res->crtcs[j]);
            }
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
        cfg_plane.format = DRM_FORMAT_NV12;

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

    return -1;
}

/****************************************************
 * create_fb()
 * description: create, map and add the dumb buffer
 ***************************************************/
int create_fb(uint32_t frameWidth, uint32_t frameHeight)
{
    int i, j;
    int ret = 0;

    /* create framebuffer */
    for (j = 0; j < MAX_BUFFER; j++) {
        struct drm_mode_create_dumb create_arg;

        memset(&create_arg, 0, sizeof(create_arg));

        create_arg.bpp = format_to_bpp(plane_cfg[0].format);

        create_arg.width  = frameWidth;
        create_arg.height = frameHeight;

        for (i = 0; i < (int)plane_cfg.size(); i++) {
            plane_cfg[i].x = (plane_cfg[i].w - create_arg.width) / 2;
            plane_cfg[i].y = (plane_cfg[i].h - create_arg.height) / 2;

            plane_cfg[i].w = frameWidth;
            plane_cfg[i].h = frameHeight;
        }

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

        if (DRM_FORMAT_NV12 == plane_cfg[0].format) {
            create_arg.pitch = VENUS_Y_STRIDE(COLOR_FMT_NV12, create_arg.width);
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

        /* create fbid */
        struct drm_mode_fb_cmd2 cmd2[MAX_BUFFER] {};

        cmd2[j].width = create_arg.width;
        cmd2[j].height = create_arg.height;
        cmd2[j].pixel_format = DRM_FORMAT_NV12;
        cmd2[j].flags = DRM_MODE_FB_MODIFIERS;
        cmd2[j].handles[0] = create_arg.handle;
        cmd2[j].pitches[0] = create_arg.pitch;
        cmd2[j].offsets[0] = 0;
        cmd2[j].modifier[0] = 0;

        if (DRM_FORMAT_NV12 == plane_cfg[0].format)
        {
            cmd2[j].handles[1] = create_arg.handle;
            cmd2[j].pitches[1] = create_arg.pitch;
            cmd2[j].offsets[1] = create_arg.pitch * create_arg.height;
            cmd2[j].modifier[1] = 0;
        }

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
int setup_connector(drmModeAtomicReqPtr req)
{
    LOG_FUNC_START;
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
                        DRM_MODE_ATOMIC_ALLOW_MODESET | DRM_MODE_ATOMIC_NONBLOCK, 0);
        if (ret) {
           DEBUGT_PRINT_ERROR("init commit failed: %s\n",
                              strerror(errno));
           return ret;
        }

        DEBUGT_PRINT_INFO("init commit\n");

        /* clear previous commit */
        drmModeAtomicSetCursor(req, 0);
    }
    LOG_FUNC_END;
    return ret;
}

/****************************************************
 * update_fb()
 * description: update the plane with new framebuffer
 ***************************************************/
void update_fb(drmModeAtomicReqPtr req, int connector_count, int buf_idx)
{
    int j;

    for (j = 0; j < (int)plane_cfg.size(); j++) {
        if (plane_cfg[j].crtc_id == connector_cfg[connector_count].crtc_id) {
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
                if (plane_cfg[j].handoff_set && plane_cfg[j].handoff_pid) {
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
                    plane_cfg[j].fb[buf_idx].fb_id);
        }
    }
}

/****************************************************
 * destroy_buff()
 * description: cleanup the allocated dumb buffers
 ***************************************************/
int destroy_buff()
{
    int k, ret = 0;

    for(k = 0; k < MAX_BUFFER; k++) {
        struct drm_mode_destroy_dumb destroy_arg;

        memset(&destroy_arg, 0, sizeof(destroy_arg));
        destroy_arg.handle = plane_cfg[0].fb[k].handle;

        drmUnmap(plane_cfg[0].fb[k].ptr,
                 plane_cfg[0].fb[k].size);

        ret = drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy_arg);
        if (ret) {
           DEBUGT_PRINT_ERROR("failed to destroy dumb buffer %s\n",
                               strerror(errno));
           return ret;
        }
    }
    return ret;
}

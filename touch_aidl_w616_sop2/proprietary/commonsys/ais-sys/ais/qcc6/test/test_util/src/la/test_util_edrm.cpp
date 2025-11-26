/* ===========================================================================
 * Copyright (c) 2017-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * ===========================================================================
 * Not a contribution.
 *
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copied content from   https://github.com/AlphaPerfect/docs/blob/master/drm-howto/modeset-vsync.c
=========================================================================== */

#include "test_util.h"
#include "test_util_edrm.h"
#ifndef C2D_DISABLED
#include "c2d2.h"
#endif
#include <pthread.h>
#include <sys/time.h>
#include <inttypes.h>

#define KGSL_USER_MEM_TYPE_ION 3
#define MAX_EGL_ATTRIBUTES 64
#define EDRM_RETRY_CNTS 200
#define DRM_WINDOW_HEIGHT 1024
#define DRM_WINDOW_WIDTH 1920
#ifdef AIS_EARLYSERVICE
#define ION_NODE "/early_services/dev/ion"
#define DRM_NODE "/early_services/dev/dri/card2"
#else
#define ION_NODE "/dev/ion"
#define DRM_NODE "/dev/dri/card2"
#endif

#ifdef TGA_DUMP_ENABLED
#include "tga.hpp"
static int tgadumped = 0;
#endif

#define MAX_COUNT 10
#define ERR_NO -1

enum {
    HANDOFF_UNSET = 0,
    HANDOFF_SET,
};

//power manager resource
typedef struct
{
    void* event_client_data;
    power_event_callable p_power_event_callback;
}test_util_pm_handle_t;
static test_util_pm_handle_t g_pm_handle;

QCarCamRet_e test_util_create_c2d_surface_target(test_util_ctxt_t *ctxt, unsigned int idx);
QCarCamRet_e test_util_update_c2d_surface(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx);

#ifndef C2D_DISABLED
C2D_STATUS c2d_init();

C2D_DRIVER_SETUP_INFO set_driver_op = {
    .max_surface_template_needed = MAX_BUFFER * 2 }; // Total number of display + camera c2d buffers
#endif

/*Graphics/display utility functions */
static int ALIGN(int x, int y) {
    // y must be a power of 2.
    return (x + y - 1) & ~(y - 1);
}

typedef enum
{
    TEST_UTIL_PATTERN_BLACK = 0
}test_util_pattern_t;

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
        case DRM_FORMAT_ABGR8888:
        default:
            return 32;
    }
}

void place_marker(char const *name)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
    int fd=open("/sys/kernel/boot_kpi/kpi_values", O_WRONLY);
    if (fd > 0)
    {
        char earlyapp[128] = {0};
        strlcat(earlyapp, name, sizeof(earlyapp));
        write(fd, earlyapp, strlen(earlyapp));
        close(fd);
    }
#else
    ALOGE("boot_kpi: %s", name);
#endif
}

static int bo_create_dumb_ion(test_util_ctxt_t *ctxt, int j)
{
    struct ion_allocation_data ion_alloc_data;
#ifndef ION_NEW
    struct ion_fd_data fd_data;
#endif
    int ret;
    struct bo buf_obj = {};
    int width = 0;
    int height = 0;
    int bpp = format_to_bpp(ctxt->pesplash_dev->plane_cfg[0].format);
    int i;
    int ret_value = 0;
    size_t page_size, frame_size;
    struct drm_prime_handle prime_req;
    int ion_fd;

    for (i = 0; i < (int)ctxt->pesplash_dev->plane_cfg.size(); i++) {
        width = width > ctxt->pesplash_dev->plane_cfg[i].w ? width: ctxt->pesplash_dev->plane_cfg[i].w;
        height = height > ctxt->pesplash_dev->plane_cfg[i].h ? height: ctxt->pesplash_dev->plane_cfg[i].h;
    }

    while (access(ION_NODE, F_OK) != 0)
    {
        usleep(100);
    }
    ion_fd = open(ION_NODE, O_RDWR | O_DSYNC);
    if (ion_fd < 0) {
        QCARCAM_ERRORMSG("GL_DRM : failed to open ion_device");
        return -1;
    }
    page_size = sysconf(_SC_PAGESIZE);
    int aligned_width = ALIGN(width, 128);
    buf_obj.pitch = aligned_width * bpp / 8;
    int slice_height = ALIGN(height, 32);
    frame_size = buf_obj.pitch  * slice_height;

    /*(round up to page size(4K))*/
    buf_obj.size = (frame_size + page_size - 1) & (~(page_size - 1));
    buf_obj.offset = 0;
    buf_obj.ion_fd = ion_fd;

    memset(&ion_alloc_data, 0, sizeof(ion_alloc_data));
    ion_alloc_data.len = buf_obj.size;
#ifndef ION_NEW
    ion_alloc_data.align = page_size;
#endif
    ion_alloc_data.heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID);
    ion_alloc_data.flags = 0;

    ret = ioctl(ion_fd, ION_IOC_ALLOC, &ion_alloc_data);
    if (ret) {
        QCARCAM_ERRORMSG("GL_DRM : failed to allocate memory from ion: %d", ret);
        ret_value = -1;
        goto error;
    }

#ifndef ION_NEW
    buf_obj.ihdl = ion_alloc_data.handle;

    fd_data.handle = buf_obj.ihdl;
    ret = ioctl(ion_fd, ION_IOC_MAP, &fd_data);
    if (ret) {
        QCARCAM_ERRORMSG("GL_DRM : failed to map ion memory: %d", ret);
        ret_value = -1;
        goto error;
    }
    buf_obj.ion_map_fd = fd_data.fd;
    prime_req.fd = fd_data.fd;
#else
    buf_obj.ion_map_fd = ion_alloc_data.fd;
    prime_req.fd = ion_alloc_data.fd;
#endif

    ret = drmIoctl(ctxt->pesplash_dev->fd, DRM_IOCTL_PRIME_FD_TO_HANDLE, &prime_req);
    if (ret) {
        close(prime_req.fd);
        QCARCAM_ERRORMSG("GL_DRM : failed get prime handle: %d", ret);
        ret_value = -1;
        goto error;
    }
    buf_obj.handle = prime_req.handle;

    for (i = 0; i < (int)ctxt->pesplash_dev->plane_cfg.size(); i++) {
        ctxt->pesplash_dev->plane_cfg[i].bo[j].size = buf_obj.size;
        ctxt->pesplash_dev->plane_cfg[i].bo[j].offset = buf_obj.offset;
        ctxt->pesplash_dev->plane_cfg[i].bo[j].ion_fd = buf_obj.ion_fd;
        ctxt->pesplash_dev->plane_cfg[i].bo[j].pitch = buf_obj.pitch;
        ctxt->pesplash_dev->plane_cfg[i].bo[j].handle = buf_obj.handle;
        ctxt->pesplash_dev->plane_cfg[i].bo[j].ion_map_fd = buf_obj.ion_map_fd;
        ctxt->pesplash_dev->plane_cfg[i].bo[j].ihdl = buf_obj.ihdl;
    }

error:
#ifndef ION_NEW
    if (ion_alloc_data.handle) {
        struct ion_handle_data hdl_data = {
            .handle = ion_alloc_data.handle,
        };
        ioctl(ion_fd, ION_IOC_FREE, &hdl_data);
    }
#endif
    close(ion_fd);
    return ret_value;
}

static void bo_unmap(struct bo *bo)
{
    if (!bo->ptr)
        return;
    drm_munmap(bo->ptr, bo->size);
    bo->ptr = NULL;
}

static void bo_destroy(struct bo *bo)
{
#ifndef ION_NEW
    struct ion_handle_data hdl_data = {
        .handle = static_cast<ion_user_handle_t>(bo->ihdl),
    };
#endif
    QCARCAM_ERRORMSG("calling bo_destroy");
    bo_unmap(bo);
    close(bo->ion_map_fd);

#ifndef ION_NEW
    ret = ioctl(bo->ion_fd, ION_IOC_FREE, &hdl_data);
    if (ret)
        QCARCAM_ERRORMSG("GL_DRM : failed to free ion buffer: %d", ret);
#endif
    close(bo->ion_fd);
    free(bo);
}

static void update_possible_crtcs(test_util_ctxt_t *ctxt)
{
    uint32_t j;

    ctxt->pesplash_dev->plane_mutex.lock();

    /* update possible_crtcs on errors */
    for (j = 0; j < ctxt->pesplash_dev->plane_cfg.size(); j++) {
            drmModePlanePtr plane = drmModeGetPlane(ctxt->pesplash_dev->fd, ctxt->pesplash_dev->plane_cfg[j].plane_id);

            if (plane && ctxt->pesplash_dev->plane_cfg[j].possible_crtcs != plane->possible_crtcs) {
                    QCARCAM_ERRORMSG("plane%d possible_crtcs 0x%x => 0x%x\n",
                                    ctxt->pesplash_dev->plane_cfg[j].plane_id,
                                    ctxt->pesplash_dev->plane_cfg[j].possible_crtcs,
                                    plane->possible_crtcs);
                    ctxt->pesplash_dev->plane_cfg[j].possible_crtcs = plane->possible_crtcs;
                    ctxt->pesplash_dev->plane_cfg[j].first_run = true;
            }

            drmModeFreePlane(plane);
    }

    ctxt->pesplash_dev->plane_mutex.unlock();
}

int parse_display(test_util_ctxt_t *ctxt)
{
    connector_config cfg[MAX_COUNT] = {};
    plane_config cfg_plane = {};
    drmModePlaneRes *plane_res = NULL;
    drmModeRes *res = NULL;
    int count_connectors = 0;
    int count_crtcs = 0;
    int count_planes = 0;
    int i = 0, j = 0;

    /*Get information about connectors*/
    res = drmModeGetResources(ctxt->pesplash_dev->fd);
    if (!res) {
        QCARCAM_ERRORMSG("GL_DRM %s %d : drmModeGetResources failed \n", __func__, __LINE__);
        goto out;
    }

    for (i = 0; i < res->count_connectors; i++) {
        drmModeConnectorPtr conn = drmModeGetConnector(ctxt->pesplash_dev->fd, res->connectors[i]);
        uint32_t crtc_mask = 0;

        if (!conn || !conn->encoders) {
            QCARCAM_ERRORMSG("GL_DRM %s %d : drmModeGetConnector failed \n", __func__, __LINE__);
            goto out;
        }

        if (conn->connection != DRM_MODE_CONNECTED) {
            QCARCAM_ERRORMSG("GL_DRM %s %d : connector not connected, try other one \n", __func__, __LINE__);
            continue;
        }

        for (j = 0; j < conn->count_encoders; j++) {
            drmModeEncoderPtr enc = drmModeGetEncoder(ctxt->pesplash_dev->fd, conn->encoders[j]);
            if(!enc)
                goto out;
            crtc_mask |= enc->possible_crtcs;
        }
        for (j = 0; j < res->count_crtcs; j++) {
            if (crtc_mask & (1 << j)) {
                QCARCAM_DBGMSG(" %d", res->crtcs[j]);
            }
        }
        QCARCAM_DBGMSG("\tmode:");
        for (j = 0; j < conn->count_modes; j++) {
            QCARCAM_DBGMSG(" %s", conn->modes[j].name);
        }
        QCARCAM_DBGMSG("\n");

        cfg[count_connectors].connector_id = conn->connector_id;

        /* store property id */
        drmModeObjectPropertiesPtr props =
            drmModeObjectGetProperties(ctxt->pesplash_dev->fd, conn->connector_id,
                    DRM_MODE_OBJECT_CONNECTOR);
        if (!props) {
            QCARCAM_ERRORMSG("GL_DRM %s %d : drmModeObjectGetProperties failed \n", __func__, __LINE__);
            goto out;
        }

        for (j = 0; j < (int)props->count_props; j++) {
            drmModePropertyPtr prop = drmModeGetProperty(ctxt->pesplash_dev->fd, props->props[j]);
            if (!prop) {
                QCARCAM_ERRORMSG("GL_DRM %s %d : drmModeGetProperty failed \n", __func__, __LINE__);
                goto out;
            }

            if (!strcmp(prop->name, "CRTC_ID"))
                cfg[count_connectors].crtc_id_pid = prop->prop_id;
        }

        /* copy mode */
        if (!conn->modes) {
            drmModeFreeConnector(conn);
            QCARCAM_ERRORMSG("GL_DRM %s %d : copy mode abort \n", __func__, __LINE__);
            goto out;
        }
        memcpy(&cfg[count_connectors].mode,
                &conn->modes[0], sizeof(drmModeModeInfo));
        drmModeFreeConnector(conn);
        QCARCAM_DBGMSG("GL_DRM %s %d : Found the connector with properties for i=%d. Breaking off \n", __func__, __LINE__, i);
        break;
    }

    /*Get information about crtcs*/
    for (i = 0; i < res->count_crtcs; i++) {
        drmModeCrtcPtr crtc = drmModeGetCrtc(ctxt->pesplash_dev->fd, res->crtcs[i]);
        if (!crtc || !res->connectors) {
            QCARCAM_ERRORMSG("GL_DRM %s %d : drmModeGetCrtc failed \n", __func__, __LINE__);
            goto out;
        }

        drmModeConnectorPtr conn = drmModeGetConnector(ctxt->pesplash_dev->fd, res->connectors[i]);
        if (!conn) {
            QCARCAM_ERRORMSG("GL_DRM %s %d : drmModeGetConnector failed \n", __func__, __LINE__);
            goto out;
        }

        if (conn->connection != DRM_MODE_CONNECTED) {
            QCARCAM_ERRORMSG("GL_DRM %s %d : not connected. Try other \n", __func__, __LINE__);
            continue;
        }

        cfg[count_crtcs].crtc_id = crtc->crtc_id;

        /* store property id */
        drmModeObjectPropertiesPtr props =
            drmModeObjectGetProperties(ctxt->pesplash_dev->fd, crtc->crtc_id, DRM_MODE_OBJECT_CRTC);
        if (!props) {
            QCARCAM_ERRORMSG("GL_DRM %s %d : drmModeObjectGetProperties failed \n", __func__, __LINE__);
            goto out;
        }

        for (j = 0; j < (int)props->count_props; j++) {
            drmModePropertyPtr prop = drmModeGetProperty(ctxt->pesplash_dev->fd, props->props[j]);
            if (!prop) {
                QCARCAM_ERRORMSG("GL_DRM %s %d : drmModeGetProperty failed \n", __func__, __LINE__);
                goto out;
            }

            if (!strcmp(prop->name, "ACTIVE"))
                cfg[count_crtcs].active_pid = prop->prop_id;
            else if (!strcmp(prop->name, "MODE_ID"))
                cfg[count_crtcs].mode_id_pid = prop->prop_id;
        }

        /* store crtc index */
        cfg[count_crtcs].crtc_idx = count_crtcs;
        ctxt->pesplash_dev->connector_cfg.push_back(cfg[count_crtcs]);

        drmModeFreeCrtc(crtc);
        drmModeFreeConnector(conn);
        QCARCAM_DBGMSG("GL_DRM %s %d : Found the config and added to the queue at i=%d \n", __func__, __LINE__, i);
        break;
    }

    /*Get information about planes*/
    plane_res = drmModeGetPlaneResources(ctxt->pesplash_dev->fd);
    if(!plane_res) {
        QCARCAM_ERRORMSG("GL_DRM %s %d : drmModeGetPlaneResources failed \n", __func__, __LINE__);
        goto out;
    }

    for (i = 0; i < (int)plane_res->count_planes; i++) {
        int handoff;
        drmModePlanePtr plane = drmModeGetPlane(ctxt->pesplash_dev->fd, plane_res->planes[i]);
        if (!plane || !res->connectors) {
            QCARCAM_ERRORMSG("GL_DRM %s %d : drmModeGetPlane failed \n", __func__, __LINE__);
            goto out;
        }

        drmModeConnectorPtr conn = drmModeGetConnector(ctxt->pesplash_dev->fd, res->connectors[i]);
        if (!conn) {
            QCARCAM_ERRORMSG("GL_DRM %s %d : drmModeGetConnector failed \n", __func__, __LINE__);
            goto out;
        }

        if (conn->connection != DRM_MODE_CONNECTED) {
            QCARCAM_ERRORMSG("GL_DRM %s %d : DRM_MODE_CONNECTED failed, trying other \n", __func__, __LINE__);
            continue;
        }

        /* dump */
        QCARCAM_DBGMSG("plane%d: %d - crtcs:", i, plane->plane_id);
        for (j = 0; j < res->count_crtcs; j++) {
            if (plane->possible_crtcs & (1 << j) && res->crtcs) {
                QCARCAM_DBGMSG(" %d", res->crtcs[j]);
            }
        }
        QCARCAM_DBGMSG("\n");

        cfg_plane.plane_id = plane->plane_id;
        cfg_plane.format = ctxt->pesplash_dev->format;

        /* store property id */
        drmModeObjectPropertiesPtr props =
            drmModeObjectGetProperties(ctxt->pesplash_dev->fd, plane->plane_id, DRM_MODE_OBJECT_PLANE);
        if (!props) {
            QCARCAM_ERRORMSG("GL_DRM %s %d : drmModeObjectGetProperties failed \n", __func__, __LINE__);
            goto out;
        }

        for (j = 0; j < (int)props->count_props; j++) {
            drmModePropertyPtr prop = drmModeGetProperty(ctxt->pesplash_dev->fd, props->props[j]);
            if (!prop) {
                QCARCAM_ERRORMSG("GL_DRM %s %d : drmModeGetProperty failed \n", __func__, __LINE__);
                goto out;
            }

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
        if (cfg_plane.handoff_pid) {
            QCARCAM_DBGMSG("\thandoff=%d possible_crtcs=0x%x\n",
                    handoff, plane->possible_crtcs);
        }
        cfg_plane.x = plane->x;
        cfg_plane.y = plane->y;
        cfg_plane.h = ctxt->pesplash_dev->connector_cfg[count_planes].mode.vdisplay;
        cfg_plane.w = ctxt->pesplash_dev->connector_cfg[count_planes].mode.hdisplay;

        ctxt->pesplash_dev->plane_cfg.push_back(cfg_plane);

        drmModeFreePlane(plane);
        drmModeFreeConnector(conn);
        QCARCAM_DBGMSG("GL_DRM %s %d : Found the plane, breaking off \n", __func__, __LINE__);
        break;
    }
    drmModeFreePlaneResources(plane_res);

    QCARCAM_DBGMSG("GL_DRM %s %d : Returning successfully \n", __func__, __LINE__);
    return 0;
out:
    if (res)
        drmModeFreeResources(res);

    if (plane_res)
        drmModeFreePlaneResources(plane_res);

    QCARCAM_ERRORMSG("GL_DRM %s %d : Returning unsuccessfully \n", __func__, __LINE__);

    return ERR_NO;
}

static int create_fb(test_util_ctxt_t *ctxt)
{
    int ret;
    int i,j;
    unsigned int handles[2], pitches[2], offsets[2];
    void *map;
    uint32_t fbid;
    int width, height;

    for (j = 0; j < MAX_BUFFER; j++) {
        /* create ion allocated dumb buffer */
        ret = bo_create_dumb_ion(ctxt, j);
        if(0 != ret) {
           QCARCAM_ERRORMSG("GL_DRM : Failed to create dumb buffer");
           return -EINVAL;
        }

        for (i = 0; i < (int)ctxt->pesplash_dev->plane_cfg.size(); i++) {
            map = drm_mmap(0, ctxt->pesplash_dev->plane_cfg[i].bo[j].size, PROT_READ | PROT_WRITE, MAP_SHARED,
                    ctxt->pesplash_dev->plane_cfg[i].bo[j].ion_map_fd, ctxt->pesplash_dev->plane_cfg[i].bo[j].offset);
            if (map == MAP_FAILED) {
                map = NULL;
                QCARCAM_ERRORMSG("GL_DRM : Failed to map");
                return -EINVAL;
            }
            ctxt->pesplash_dev->plane_cfg[i].bo[j].ptr = map;
            ctxt->pesplash_dev->plane_cfg[i].bo[j].plane[0] = (unsigned char *)ctxt->pesplash_dev->plane_cfg[i].bo[j].ptr;
        }

        width = ctxt->pesplash_dev->plane_cfg[0].w;
        height = ctxt->pesplash_dev->plane_cfg[0].h;
        handles[0] = ctxt->pesplash_dev->plane_cfg[0].bo[j].handle;
        pitches[0] = width*format_to_bpp(ctxt->pesplash_dev->plane_cfg[0].format)/8;
        offsets[0] = 0;

        ret = drmModeAddFB2(ctxt->pesplash_dev->fd, width,  height, ctxt->pesplash_dev->plane_cfg[0].format, handles, pitches, offsets, &fbid, 0);
        if (ret) {
           QCARCAM_ERRORMSG("GL_DRM : cannot create framebuffer fd ctxt->pesplash_dev->fd %d width %d height %d format %d ret = %d"
                    , ctxt->pesplash_dev->fd, width, height, ctxt->pesplash_dev->plane_cfg[0].format, ret);
        }

        for (i = 0; i < (int)ctxt->pesplash_dev->plane_cfg.size(); i++) {
            ctxt->pesplash_dev->plane_cfg[i].bo[j].fbid = fbid;
        }

        QCARCAM_DBGMSG("Created FB: %d",ctxt->pesplash_dev->plane_cfg[i].bo[j].fbid);

    }

    return ret;
}

int setup_connector(test_util_ctxt_t *ctxt)
{
    int i = 0;
    int ret = 0;

    for (i = 0; i < (int)ctxt->pesplash_dev->connector_cfg.size(); i++) {
        /* set mode */
        uint32_t blob_id;
        if (drmModeCreatePropertyBlob(ctxt->pesplash_dev->fd, (const void *)&ctxt->pesplash_dev->connector_cfg[i].mode,
                    sizeof(drmModeModeInfo), &blob_id)) {
            QCARCAM_ERRORMSG("failed to create mode blob\n");
            return 0;
        }
        drmModeAtomicAddProperty(ctxt->pesplash_dev->req, ctxt->pesplash_dev->connector_cfg[i].crtc_id,
                ctxt->pesplash_dev->connector_cfg[i].mode_id_pid, blob_id);

        /* set active */
        drmModeAtomicAddProperty(ctxt->pesplash_dev->req, ctxt->pesplash_dev->connector_cfg[i].crtc_id,
                ctxt->pesplash_dev->connector_cfg[i].active_pid, 1);

        /* set crtc */
        drmModeAtomicAddProperty(ctxt->pesplash_dev->req, ctxt->pesplash_dev->connector_cfg[i].connector_id,
                ctxt->pesplash_dev->connector_cfg[i].crtc_id_pid, ctxt->pesplash_dev->connector_cfg[i].crtc_id);

        /* init commit */
        ret = drmModeAtomicCommit(ctxt->pesplash_dev->fd, ctxt->pesplash_dev->req, DRM_MODE_ATOMIC_ALLOW_MODESET, 0);
        if (ret) {
            QCARCAM_ERRORMSG("init commit failed: %s\n",
                    strerror(errno));
            return ret;
        }
        QCARCAM_DBGMSG("init commit\n");
        /* clear previous commit */
        drmModeAtomicSetCursor(ctxt->pesplash_dev->req, 0);
    }
    return ret;
}

void update_fb(test_util_ctxt_t *ctxt, int fb_number)
{
    int j = 0;

    for (j = 0; j < (int)ctxt->pesplash_dev->connector_cfg.size(); j++) {
        /* update plane in locked context */
        std::lock_guard<std::mutex> guard(ctxt->pesplash_dev->plane_mutex);

        /* check possible_crtcs */
        if (!(ctxt->pesplash_dev->plane_cfg[j].possible_crtcs &
                    (1 << ctxt->pesplash_dev->connector_cfg[j].crtc_idx)))
        {
            QCARCAM_ERRORMSG("%s %d : fb_number %d j %d ctxt->pesplash_dev->plane_cfg[j].first_run %d ",__func__, __LINE__,fb_number,j, ctxt->pesplash_dev->plane_cfg[j].first_run);
            continue;
        }

        /* first run */
        if (ctxt->pesplash_dev->plane_cfg[j].first_run) {
            drmModeAtomicAddProperty(ctxt->pesplash_dev->req,
                    ctxt->pesplash_dev->plane_cfg[j].plane_id,
                    ctxt->pesplash_dev->plane_cfg[j].crtc_pid,
                    ctxt->pesplash_dev->plane_cfg[j].crtc_id);
            drmModeAtomicAddProperty(ctxt->pesplash_dev->req,
                    ctxt->pesplash_dev->plane_cfg[j].plane_id,
                    ctxt->pesplash_dev->plane_cfg[j].zpos_pid,
                    ctxt->pesplash_dev->plane_cfg[j].zpos);
            drmModeAtomicAddProperty(ctxt->pesplash_dev->req,
                    ctxt->pesplash_dev->plane_cfg[j].plane_id,
                    ctxt->pesplash_dev->plane_cfg[j].crtc_x_pid,
                    ctxt->pesplash_dev->plane_cfg[j].x);
            drmModeAtomicAddProperty(ctxt->pesplash_dev->req,
                    ctxt->pesplash_dev->plane_cfg[j].plane_id,
                    ctxt->pesplash_dev->plane_cfg[j].crtc_y_pid,
                    ctxt->pesplash_dev->plane_cfg[j].y);
            drmModeAtomicAddProperty(ctxt->pesplash_dev->req,
                    ctxt->pesplash_dev->plane_cfg[j].plane_id,
                    ctxt->pesplash_dev->plane_cfg[j].crtc_w_pid,
                    ctxt->pesplash_dev->plane_cfg[j].w);
            drmModeAtomicAddProperty(ctxt->pesplash_dev->req,
                    ctxt->pesplash_dev->plane_cfg[j].plane_id,
                    ctxt->pesplash_dev->plane_cfg[j].crtc_h_pid,
                    ctxt->pesplash_dev->plane_cfg[j].h);
            drmModeAtomicAddProperty(ctxt->pesplash_dev->req,
                    ctxt->pesplash_dev->plane_cfg[j].plane_id,
                    ctxt->pesplash_dev->plane_cfg[j].src_y_pid,
                    0 << 16);
            drmModeAtomicAddProperty(ctxt->pesplash_dev->req,
                    ctxt->pesplash_dev->plane_cfg[j].plane_id,
                    ctxt->pesplash_dev->plane_cfg[j].src_w_pid,
                    ctxt->pesplash_dev->plane_cfg[j].w << 16);
            drmModeAtomicAddProperty(ctxt->pesplash_dev->req,
                    ctxt->pesplash_dev->plane_cfg[j].plane_id,
                    ctxt->pesplash_dev->plane_cfg[j].src_h_pid,
                    ctxt->pesplash_dev->plane_cfg[j].h << 16);

            if (ctxt->pesplash_dev->plane_cfg[j].handoff_pid) {
                drmModeAtomicAddProperty(ctxt->pesplash_dev->req,
                    ctxt->pesplash_dev->plane_cfg[j].plane_id,
                    ctxt->pesplash_dev->plane_cfg[j].handoff_pid,
                    HANDOFF_UNSET);
            }
            ctxt->pesplash_dev->plane_cfg[j].first_run = false;

            QCARCAM_DBGMSG("%s %d : FB updated, fb_number %d j %d ctxt->pesplash_dev->plane_cfg[j].first_run %d ",__func__, __LINE__,fb_number,j, ctxt->pesplash_dev->plane_cfg[j].first_run);
        }
        /* update y_offset */
        drmModeAtomicAddProperty(ctxt->pesplash_dev->req,
                ctxt->pesplash_dev->plane_cfg[j].plane_id,
                ctxt->pesplash_dev->plane_cfg[j].fb_pid,
                ctxt->pesplash_dev->plane_cfg[j].bo[fb_number].fbid);

        QCARCAM_DBGMSG("%s %d : FB updated, fb_number %d fbid %d j %d",__func__, __LINE__,fb_number,ctxt->pesplash_dev->plane_cfg[j].bo[fb_number].fbid,j);
    }
}

static unsigned int qcarcam_get_stride(QCarCamColorFmt_e fmt, unsigned int width)
{
    unsigned int stride = 0;
    switch (fmt)
    {
    case QCARCAM_FMT_RGB_888:
        stride = width * 3;
        break;
    case QCARCAM_FMT_MIPIRAW_8:
        stride = width;
        break;
    case QCARCAM_FMT_MIPIRAW_10:
        if (0 == (width % 4))
            stride = width * 5 / 4;
        break;
    case QCARCAM_FMT_MIPIRAW_12:
        if (0 == (width % 2))
            stride = width * 3 / 2;
        break;
    case QCARCAM_FMT_UYVY_8:
    case QCARCAM_FMT_PLAIN16_10:
    case QCARCAM_FMT_PLAIN16_12:
    case QCARCAM_FMT_PLAIN16_14:
    case QCARCAM_FMT_PLAIN16_16:
        return width * 2;
    case QCARCAM_FMT_UYVY_10:
        if (0 == (width % 4))
            stride = width * 2 * 5 / 4;
        break;
    case QCARCAM_FMT_UYVY_12:
        if (0 == (width % 2))
            stride = width * 2 * 3 / 2;
        break;
    case QCARCAM_FMT_NV12:
    case QCARCAM_FMT_NV21:
        stride = width * 3/2;
        break;
    default:
        break;
    }

    return stride;
}


#ifndef C2D_DISABLED
/* @Breif: API to init C2d driver.
 * @params: NULL
 * @return: C2D_STATUS */
C2D_STATUS c2d_init()
{
    C2D_STATUS c2d_status = c2dDriverInit(&set_driver_op);

    if(c2d_status != C2D_STATUS_OK)
    {
        QCARCAM_ERRORMSG("GL_DRM \nC2DDriverInit failed \n");
    }

    return c2d_status;
}
#endif

static void esplash_cleanup(struct esplash_dev *iter)
{
    /* clear commit */
    drmModeAtomicFree(iter->req);

    for (int i = 0; i < MAX_BUFFER; i++) {
        /* unmap buffer */
        munmap(iter->plane_cfg[0].bo[i].ptr, iter->plane_cfg[0].bo[i].size);

        /* delete framebuffer */
        drmModeRmFB(iter->fd, iter->plane_cfg[0].bo[i].fbid);

        /* delete dumb buffer */
        bo_destroy(&iter->plane_cfg[0].bo[i]);
    }

    close(iter->fd);

    /* free allocated memory */
    free(iter);
}

bool create_EGLContext(test_util_ctxt_t *ctxt)
{
    EGLint attribList[MAX_EGL_ATTRIBUTES];
    EGLint numConfigs;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLint nAttribCount = 0;
    EGLBoolean ret;
    static const EGLint gl_context_attribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    attribList[nAttribCount++] = EGL_RED_SIZE;
    attribList[nAttribCount++] = 8;
    attribList[nAttribCount++] = EGL_GREEN_SIZE;
    attribList[nAttribCount++] = 8;
    attribList[nAttribCount++] = EGL_BLUE_SIZE;
    attribList[nAttribCount++] = 8;
    attribList[nAttribCount++] = EGL_ALPHA_SIZE;
    attribList[nAttribCount++] = 8;
    attribList[nAttribCount++] = EGL_DEPTH_SIZE;
    attribList[nAttribCount++] = 16;
    attribList[nAttribCount++] = EGL_STENCIL_SIZE;
    attribList[nAttribCount++] = 0;
    attribList[nAttribCount++] = EGL_NONE;
    assert( nAttribCount < MAX_EGL_ATTRIBUTES );

    ctxt->m_egl.dsp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(ctxt->m_egl.dsp != EGL_NO_DISPLAY);
    if(ctxt->m_egl.dsp == EGL_NO_DISPLAY)
        QCARCAM_ERRORMSG("GL_DRM : eglGetDisplay FAIL");
    else
        QCARCAM_DBGMSG("GL_DRM : eglGetDisplay DONE");

    ret = eglInitialize(ctxt->m_egl.dsp, &majorVersion, &minorVersion);
    assert(ret);
    if(0 == ret)
        QCARCAM_ERRORMSG("GL_DRM : eglInitialize FAIL");
    else
       QCARCAM_DBGMSG("GL_DRM : eglInitialize DONE");

    ret = eglGetConfigs(ctxt->m_egl.dsp, NULL, 0, &numConfigs);
    assert(ret);
    if(0 == ret)
        QCARCAM_ERRORMSG("GL_DRM : eglGetConfigs FAIL");
    else
        QCARCAM_DBGMSG("GL_DRM : eglGetConfigs DONE");

    ret = eglChooseConfig(ctxt->m_egl.dsp, attribList, &ctxt->m_egl.cfg, 1, &numConfigs);
    assert(ret);
    if(0 == ret)
        QCARCAM_ERRORMSG("GL_DRM : eglChooseConfig FAIL");
    else
        QCARCAM_DBGMSG("GL_DRM : eglChooseConfig DONE");

    EGLint surfaceAttrib[] =
    {
        EGL_WIDTH,  ctxt->m_egl.width,
        EGL_HEIGHT, ctxt->m_egl.height,
        EGL_NONE
    };

    ctxt->m_egl.surf = eglCreatePbufferSurface(ctxt->m_egl.dsp, ctxt->m_egl.cfg, surfaceAttrib);
    assert(ctxt->m_egl.surf != EGL_NO_SURFACE);
    if(ctxt->m_egl.surf == EGL_NO_SURFACE)
        QCARCAM_ERRORMSG("GL_DRM : eglCreatePbufferSurface FAIL");
    else
        QCARCAM_DBGMSG("GL_DRM : eglCreatePbufferSurface DONE");

    ctxt->m_egl.cxt = eglCreateContext(ctxt->m_egl.dsp, ctxt->m_egl.cfg, EGL_NO_CONTEXT, gl_context_attribs);
    assert(ctxt->m_egl.cxt != EGL_NO_CONTEXT);
    if(ctxt->m_egl.cxt == EGL_NO_CONTEXT)
        QCARCAM_ERRORMSG("GL_DRM : eglCreateContext FAIL");
    else
        QCARCAM_DBGMSG("GL_DRM : eglCreateContext DONE");

    ret = eglMakeCurrent(ctxt->m_egl.dsp, ctxt->m_egl.surf, ctxt->m_egl.surf, ctxt->m_egl.cxt);
    assert(ret);
    if(0 == ret)
        QCARCAM_ERRORMSG("GL_DRM : eglMakeCurrent FAIL");
    else
        QCARCAM_DBGMSG("GL_DRM : eglMakeCurrent DONE");

    EGLDisplay  dsptemp = eglGetCurrentDisplay();
    assert(dsptemp!= EGL_NO_DISPLAY);
    if(dsptemp == EGL_NO_DISPLAY)
        QCARCAM_ERRORMSG("GL_DRM : eglGetCurrentDisplay FAIL");
    else
        QCARCAM_DBGMSG("GL_DRM : eglGetCurrentDisplay DONE");

   return true;
}

QCarCamRet_e graphics_display_init(test_util_ctxt_t *ctxt, int height, int width, int drm_format)
{
    QCarCamRet_e rc = QCARCAM_RET_OK;
    int ret = 0;

    ctxt->m_egl.width = width;
    ctxt->m_egl.height = height;

    /* create a device structure */
    ctxt->pesplash_dev = (struct esplash_dev *)malloc(sizeof(struct esplash_dev));
    if(!ctxt->pesplash_dev) {
        QCARCAM_ERRORMSG("Failed to allocate esplash_dev device %m", errno);
        return QCARCAM_RET_FAILED;
    }
    memset(ctxt->pesplash_dev, 0, sizeof(struct esplash_dev));
    ctxt->pesplash_dev->format = drm_format;

#ifndef C2D_DISABLED
    C2D_STATUS c2d_status = C2D_STATUS_OK;
    c2d_status = c2d_init();
    if (C2D_STATUS_OK != c2d_status)
    {
        QCARCAM_ERRORMSG("GL_DRM c2d_init failed \n");
        return QCARCAM_RET_FAILED;
    }
#else
    create_EGLContext(ctxt);
#endif
    ctxt->pesplash_dev->fd = open(DRM_NODE, O_RDWR, 0);
    if (ctxt->pesplash_dev->fd < 0) {
        QCARCAM_ERRORMSG("failed to open DRM node\n");
        return QCARCAM_RET_FAILED;
    }

    drmSetClientCap(ctxt->pesplash_dev->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
    drmSetClientCap(ctxt->pesplash_dev->fd, DRM_CLIENT_CAP_ATOMIC, 1);

    ret = parse_display(ctxt);
    if (ret) {
        QCARCAM_ERRORMSG("parse_display failed\n");
        return QCARCAM_RET_FAILED;
    }

    /*create framebuffer*/
    ret = create_fb(ctxt);
    if (ret) {
        QCARCAM_ERRORMSG("create_fb failed\n");
        return QCARCAM_RET_FAILED;
    }

    /* init atomic commit */
    ctxt->pesplash_dev->req = drmModeAtomicAlloc();
    if (!ctxt->pesplash_dev->req) {
        QCARCAM_ERRORMSG("drmModeAtomicAlloc failed\n");
        return QCARCAM_RET_FAILED;
    }

    /* setup crtc / connector */
    ret = setup_connector(ctxt);
    if (ret)  {
        QCARCAM_ERRORMSG("drmModeAtomicAlloc failed\n");
        return QCARCAM_RET_FAILED;
    }

    ctxt->pesplash_dev->plane_cfg[0].first_run = true;

#ifndef C2D_DISABLED
    for (int i = 0; i < MAX_BUFFER; i++)
    {
        rc = test_util_create_c2d_surface_target(ctxt, i);
        if (rc != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("GL_DRM : c2d_surface create for target failed for id = %d", i);
            return rc;
        }
    }

    pthread_mutex_init(&ctxt->mutex_c2d, NULL);
#endif
    return rc;
}

void graphics_display_deinit(test_util_ctxt_t *ctxt, test_util_window_t *p_window)
{
    int ret = 0;
#ifndef C2D_DISABLED
    for (int i = 0; i < MAX_BUFFER; i++)
    {
        unsigned int target_id = ctxt->pesplash_dev->plane_cfg[0].bo[i].c2d_surface_id;
        if (p_window->buffers)
        {
            unsigned int source_id = p_window->buffers[i].c2d_surface_id;
            c2dUnMapAddr(p_window->buffers[i].gpuaddr);
            c2dDestroySurface(source_id);
        }
        c2dUnMapAddr(ctxt->pesplash_dev->plane_cfg[0].bo[i].gpuaddr);
        c2dDestroySurface(target_id);
    }
    pthread_mutex_destroy(&ctxt->mutex_c2d);
#else
    /* Graphics changes */
    eglMakeCurrent(ctxt->m_egl.dsp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(ctxt->m_egl.dsp, ctxt->m_egl.cxt);
    eglDestroySurface(ctxt->m_egl.dsp, ctxt->m_egl.surf);
    eglTerminate(ctxt->m_egl.dsp);
    /* Graphics changes end */
#endif
    if (ctxt->pesplash_dev->plane_cfg[0].handoff_pid) {
        drmModeAtomicAddProperty(ctxt->pesplash_dev->req,
                                ctxt->pesplash_dev->plane_cfg[0].plane_id,
                                ctxt->pesplash_dev->plane_cfg[0].handoff_pid,
                                HANDOFF_SET);

        ret = drmModeAtomicCommit(ctxt->pesplash_dev->fd, ctxt->pesplash_dev->req,
                            DRM_MODE_ATOMIC_ALLOW_MODESET | DRM_MODE_ATOMIC_NONBLOCK, 0);
        if (ret) {
            QCARCAM_ERRORMSG("commit failed: %s\n", strerror(errno));
        }
        drmModeAtomicSetCursor(ctxt->pesplash_dev->req, 0);
    }
    esplash_cleanup(ctxt->pesplash_dev);
}

static inline float clamp(float v, float min, float max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

static uint32_t yuvToRgbx(const unsigned char Y, const unsigned char Uin, const unsigned char Vin) {
    float U = Uin - 128.0f;
    float V = Vin - 128.0f;

    float Rf = Y + 1.140f*V;
    float Gf = Y - 0.395f*U - 0.581f*V;
    float Bf = Y + 2.032f*U;
    unsigned char R = (unsigned char)clamp(Rf, 0.0f, 255.0f);
    unsigned char G = (unsigned char)clamp(Gf, 0.0f, 255.0f);
    unsigned char B = (unsigned char)clamp(Bf, 0.0f, 255.0f);

    return (R      ) |
           (G <<  8) |
           (B << 16) |
           0xFF000000;  // Fill the alpha channel with ones
}

void copyUYVYtoRGB32(unsigned width, unsigned height,
                     uint32_t* src, uint32_t* dst)
{
    uint32_t* srcWords = (uint32_t*)src;

    for (unsigned r = 0; r < height; r++) {
        for (unsigned c = 0; c < width/2; c++) {
            // Note:  we're walking two pixels at a time here (even/odd)
            uint32_t srcPixel = *srcWords++;

            uint8_t U = (srcPixel)       & 0xFF;
            uint8_t Y1 = (srcPixel >> 8)  & 0xFF;
            uint8_t V = (srcPixel >> 16) & 0xFF;
            uint8_t Y2 = (srcPixel >> 24) & 0xFF;

            // On the RGB output, we're writing one pixel at a time
            *(dst+0) = yuvToRgbx(Y1, U, V);
            *(dst+1) = yuvToRgbx(Y2, U, V);
            dst += 2;
        }
    }
}

void convert_UYVY_RGBA_buffer(uint32_t* srcBuffer, uint32_t* dstBuffer, uint32_t width, uint32_t height)
{
    unsigned srcStride = width *2;
    unsigned dstStride = width *4;

    QCARCAM_DBGMSG("GL_DRM : creation done srcStride %d dstStride %d width %d, height %d srcsize %d dstsize %d", srcStride, dstStride, width, height, srcStride * height, dstStride * height);
    copyUYVYtoRGB32((unsigned)width, (unsigned)height, srcBuffer, dstBuffer);
}

void graphics_draw_from_buffer(test_util_ctxt_t *ctxt, uint32_t* buffer)
{
    /* The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    Using a single texture for our usecase, for blending 2 textures can be used */
    glGenFramebuffers(1, &(ctxt->pesplash_dev->FramebufferName));
    glBindFramebuffer(GL_FRAMEBUFFER, ctxt->pesplash_dev->FramebufferName);

    QCARCAM_DBGMSG("GL_DRM : Frame buffer created");

    /* Set up the OpenGL texture to contain this image */
    glGenTextures(1, &(ctxt->pesplash_dev->textureId));
    glBindTexture(GL_TEXTURE_2D, ctxt->pesplash_dev->textureId);
    glViewport(0, 0, ctxt->pesplash_dev->width, ctxt->pesplash_dev->height);

    /* Send the image data to GL */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ctxt->pesplash_dev->width, ctxt->pesplash_dev->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    /* Initialize the sampling properties (it seems the sample may not work if this isn't done)
    The user of this texture may very well want to set their own filtering, but we're going
    to pay the (minor) price of setting this up for them to avoid the dreaded "black image" if
    they forget. */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    /* Set "textureId" as our colour attachement #0 */
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ctxt->pesplash_dev->textureId, 0);

    /* Set the list of draw buffers. */
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    glFinish();

    /* Always check that our framebuffer is ok */
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        QCARCAM_ERRORMSG("GL_DRM : Something wrong with the framebuffer");
}

#ifndef C2D_DISABLED
QCarCamRet_e graphics_draw_c2d_uyvy(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx)
{
    QCarCamRet_e rc = QCARCAM_RET_OK;
    C2D_STATUS c2d_status = C2D_STATUS_OK;
    C2D_OBJECT c2dObject;
    c2d_ts_handle c2d_timestamp = NULL;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    test_util_update_c2d_surface(ctxt, p_window, idx);

    memset(&c2dObject, 0x0, sizeof(C2D_OBJECT));
    unsigned int target_id = ctxt->pesplash_dev->plane_cfg[0].bo[idx].c2d_surface_id;
    c2dObject.surface_id = p_window->buffers[idx].c2d_surface_id;

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    QCARCAM_DBGMSG("GL_DRM - TIME_PROFILE : test_util_update_c2d_surface delta_us %" PRIu64"", delta_us);

    pthread_mutex_lock(&ctxt->mutex_c2d);
    c2d_status = c2dDraw(target_id, C2D_TARGET_ROTATE_0, 0x0, 0, 0, &c2dObject, 1);
    if (c2d_status == C2D_STATUS_OK)
    {
        c2d_status = c2dFlush(target_id, &c2d_timestamp);
    }
    else
    {
        QCARCAM_ERRORMSG("GL_DRM : c2dDraw Failed\n");
        rc = QCARCAM_RET_FAILED;
    }
    pthread_mutex_unlock(&ctxt->mutex_c2d);

    if (c2d_status == C2D_STATUS_OK)
    {
        c2d_status = c2dWaitTimestamp(c2d_timestamp);
    }

    if (c2d_status != C2D_STATUS_OK)
    {
        QCARCAM_ERRORMSG("GL_DRM : c2d conversion failed with error %d", c2d_status);
        rc = QCARCAM_RET_FAILED;
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    QCARCAM_DBGMSG("GL_DRM - TIME_PROFILE : graphics_draw_c2d_uyvy total delta_us %" PRIu64"", delta_us);

    /* Display next fd to show is updated */
    ctxt->pesplash_dev->active_fbid = idx;

    return rc;
}
#endif

void fill_display_buffer(test_util_ctxt_t *ctxt, int buf_seqno)
{
    int width = ctxt->pesplash_dev->width;
    int height = ctxt->pesplash_dev->height;
    char *buff = (char *) ctxt->pesplash_dev->plane_cfg[0].bo[buf_seqno].plane[0];
    int Bpp = (format_to_bpp(ctxt->pesplash_dev->format)/8);
    ctxt->pesplash_dev->active_fbid = buf_seqno;

    memset(buff,0x0, width*height*Bpp);

    QCARCAM_DBGMSG("Testutil context used for grahic m_egl rendering %p",ctxt);
    QCARCAM_DBGMSG("GL_DRM : Rendering m_egl %p",ctxt->m_egl);

    // read render target
    glReadPixels(0, 0, ctxt->m_egl.width, ctxt->m_egl.height, GL_RGBA, GL_UNSIGNED_BYTE, (GLuint *)buff);

#ifdef TGA_DUMP_ENABLED
    // write tga file
    if (0 == tgadumped)
    {
#ifndef AIS_EARLYSERVICE
        writeImageTGA("/data/vendor/camera/camera_pbuffer.tga", (void *)buff, GL_RGBA, ctxt->m_egl.width, ctxt->m_egl.height);
#else
        writeImageTGA("/early_services/camera_pbuffer.tga", (void *)buff, GL_RGBA, ctxt->m_egl.width, ctxt->m_egl.height);
        QCARCAM_ERRORMSG("GL_DRM : writeImageTGA dump done /early_services/camera_pbuffer.tga");
#endif
        tgadumped++;
    }
#endif //TGA_DUMP_ENABLED

    /* Cleanup */
    glDeleteFramebuffers(1, &(ctxt->pesplash_dev->FramebufferName));
    glDeleteTextures(1, &(ctxt->pesplash_dev->textureId));
}

int esplash_draw(test_util_ctxt_t *ctxt)
{
    int ret = 0;

    if(ctxt->pesplash_dev == NULL) {
        QCARCAM_ERRORMSG("Exit rendering - no edrm-display node");
        return -1;
    }

    /* update fb */
    update_fb(ctxt, ctxt->pesplash_dev->active_fbid);

    /* asynced commit */
    ret = drmModeAtomicCommit(ctxt->pesplash_dev->fd, ctxt->pesplash_dev->req,
            DRM_MODE_ATOMIC_ALLOW_MODESET | DRM_MODE_ATOMIC_NONBLOCK, 0);
    if (ret) {
        QCARCAM_ERRORMSG("commit failed: %s\n", strerror(errno));
        update_possible_crtcs(ctxt);
    }

    /* clear previous commit */
    drmModeAtomicSetCursor(ctxt->pesplash_dev->req, 0);

    return ret;
}


/* Test util functions */
static void test_util_fill_planes(QCarCamBuffer_t* pBuffer, QCarCamColorFmt_e fmt)
{
    switch (fmt)
    {
    case QCARCAM_FMT_RGB_888:
        pBuffer->planes[0].stride = pBuffer->planes[0].width * 3;
        pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        break;
    case QCARCAM_FMT_MIPIRAW_8:
        pBuffer->planes[0].stride = pBuffer->planes[0].width;
        pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        break;
    case QCARCAM_FMT_MIPIRAW_10:
        if (0 == (pBuffer->planes[0].width % 4))
        {
            pBuffer->planes[0].stride = pBuffer->planes[0].width * 5 / 4;
            pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        }
        break;
    case QCARCAM_FMT_MIPIRAW_12:
        if (0 == (pBuffer->planes[0].width % 2))
        {
            pBuffer->planes[0].stride = pBuffer->planes[0].width * 3 / 2;
            pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        }
        break;
    case QCARCAM_FMT_UYVY_8:
        pBuffer->n_planes = 1;
        pBuffer->planes[0].stride = pBuffer->planes[0].width * 2;
        pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        break;
    case QCARCAM_FMT_UYVY_10:
        pBuffer->n_planes = 1;
        pBuffer->planes[0].stride = pBuffer->planes[0].width * 5 / 4;
        pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        break;
    case QCARCAM_FMT_UYVY_12:
        if (0 == (pBuffer->planes[0].width % 2))
        {
            pBuffer->planes[0].stride = pBuffer->planes[0].width * 2 * 3 / 2;
        }
        pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        break;
    case QCARCAM_FMT_NV12:
    case QCARCAM_FMT_NV21:
    {
#ifdef USE_NV12_HEIGHT_ALIGNED
        uint32 width_align = 64;
        uint32 height_align = 64;
#else
        uint32 width_align = 64;
        uint32 height_align = 32;
#endif
        pBuffer->n_planes = 2;

        pBuffer->planes[0].stride = TESTUTIL_ALIGN(pBuffer->planes[0].width, width_align);

        //plane 2
        pBuffer->planes[1].width = pBuffer->planes[0].width;
        pBuffer->planes[1].height = pBuffer->planes[0].height / 2;
        pBuffer->planes[1].stride = pBuffer->planes[0].stride;

        pBuffer->planes[0].size = pBuffer->planes[0].stride * TESTUTIL_ALIGN(pBuffer->planes[0].height, height_align);
        pBuffer->planes[1].size = pBuffer->planes[1].stride * pBuffer->planes[1].height;

#ifndef USE_NV12_HEIGHT_ALIGNED
        //plane 0 is 4k aligned
        pBuffer->planes[0].size = TESTUTIL_ALIGN(pBuffer->planes[0].size, 4096);
#endif
        break;
    }
    default:
        QCARCAM_ERRORMSG("Unknown format 0x%x", fmt);
        pBuffer->planes[0].size = 0;
        break;
    }
}

static void test_util_fill_buffer(test_util_buffer_t* buffer, test_util_pattern_t pattern, test_util_color_fmt_t format)
{
    (void)pattern;
    if (format == TESTUTIL_FMT_UYVY_8)
    {
        //grey
        memset(buffer->ptr[0], 0x80, buffer->size[0]);
    }
    else if (format == TESTUTIL_FMT_NV12)
    {
        //black
#if 0
        memset(buffer->ptr[0], 0x0, buffer->size[0]);
        memset(buffer->ptr[1], 0x80, buffer->size[1]);
#else
        memset(buffer->ptr[0], 0x80, buffer->size[0]);
        memset(buffer->ptr[1], 0xFF, buffer->size[1]);
#endif
    }
    else
    {
        memset(buffer->ptr[0], 0x0, buffer->size[0]);
    }
}

static QCarCamRet_e alloc_ion_buffers(test_util_window_t *p_window, QCarCamBufferList_t *buffers)
{
    QCarCamRet_e rc = QCARCAM_RET_OK;
    int i;
    int ret;
    size_t page_size;

    p_window->ion_fd = open(ION_NODE, O_RDONLY | O_CLOEXEC);
    if (p_window->ion_fd < 0) {
        QCARCAM_ERRORMSG("failed to open ion_device");
        return QCARCAM_RET_FAILED;
    }
    if(p_window->buffers == NULL) {
        p_window->buffers = (test_util_buffer_t*)calloc(p_window->n_buffers, sizeof(*p_window->buffers));
        if (p_window->buffers == NULL) {
            QCARCAM_ERRORMSG("Failed to allocate buffer list");
            close(p_window->ion_fd);
            return QCARCAM_RET_NOMEM;
        }
    }
    else {
        QCARCAM_DBGMSG("Already to allocate buffer list");
    }
    for (i = 0; i < p_window->n_buffers; i++)
    {
        uint32 allocSize;
        test_util_fill_planes(&buffers->buffers[i], buffers->color_fmt);

        p_window->buffers[i].stride[0] = buffers->buffers[i].planes[0].stride;
        p_window->buffers[i].stride[1] = buffers->buffers[i].planes[1].stride;
        p_window->buffers[i].size[0] = buffers->buffers[i].planes[0].size;
        p_window->buffers[i].size[1] = buffers->buffers[i].planes[1].size;
        allocSize = buffers->buffers[i].planes[0].size + buffers->buffers[i].planes[1].size;

        page_size = sysconf(_SC_PAGESIZE);
        memset(&p_window->buffers[i].ion_alloc_data, 0, sizeof(p_window->buffers[i].ion_alloc_data));

        p_window->buffers[i].ion_alloc_data.len = allocSize;
        p_window->buffers[i].ion_alloc_data.heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID);
        p_window->buffers[i].ion_alloc_data.flags = 0;
        ret = ioctl(p_window->ion_fd, ION_IOC_ALLOC, &p_window->buffers[i].ion_alloc_data);
        if (ret) {
            QCARCAM_ERRORMSG("failed to allocate memory from ion: %d", ret);
            rc =  QCARCAM_RET_NOMEM;
            break;
        }

        QCARCAM_DBGMSG("Success: map ion_device");
        buffers->buffers[i].planes[0].p_buf = (void*)(uintptr_t)p_window->buffers[i].ion_alloc_data.fd;
        p_window->buffers[i].ion_map_fd = p_window->buffers[i].ion_alloc_data.fd;

        p_window->buffers[i].is_dequeud = 1;
        p_window->buffers[i].ptr[0] =
                mmap(NULL, p_window->buffers[i].ion_alloc_data.len,
                        PROT_READ | PROT_WRITE, MAP_SHARED,
                        p_window->buffers[i].ion_alloc_data.fd, 0);
        if (p_window->buffers[i].ptr[0] == NULL)
        {
            QCARCAM_ERRORMSG("failed to map memory from ion: %d", ret);
            rc = QCARCAM_RET_NOMEM;
            break;
        }
        QCARCAM_DBGMSG("Success:[%d] map ion_device. ptr=%x, fd=%d", i, p_window->buffers[i].ptr[0], p_window->buffers[i].ion_alloc_data.fd);
    }

    if (rc != QCARCAM_RET_OK) {
        for (int i = 0; i < p_window->n_buffers; i++) {
            if (p_window->buffers[i].ptr[0])
            {
                if (munmap(p_window->buffers[i].ptr[0],
                            p_window->buffers[i].ion_alloc_data.len) == -1)
                {
                    QCARCAM_ERRORMSG("QCDEBUG: munmap FAILED\n");
                }
                close(p_window->buffers[i].ion_map_fd);
                QCARCAM_DBGMSG("Close Ion [%d] unmap ion_device. ptr=%x, fd=%d", i, p_window->buffers[i].ptr[0], p_window->buffers[i].ion_alloc_data.fd);
                p_window->buffers[i].ptr[0] = NULL;
            }
        }
        close(p_window->ion_fd);
        free(p_window->buffers);
        p_window->buffers = NULL;
        return rc;
    }
    // Prefill all buffers as black
    for (int i = 0; i < p_window->n_buffers; i++)
    {
        test_util_fill_buffer(&p_window->buffers[i], TEST_UTIL_PATTERN_BLACK, p_window->format);
    }

    return rc;
}


///////////////////////////////////////////////////////////////////////////////
/// test_util_init
///
/// @brief Initialize context that is to be used to display content on the screen.
///
/// @param ctxt   Pointer to context to be initialized
/// @param params Parameters to init ctxt
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_init(test_util_ctxt_t **ctxt, test_util_ctxt_params_t *params)
{
    QCarCamRet_e rc = QCARCAM_RET_OK;
    test_util_ctxt_t* pCtxt;

    *ctxt = NULL;
    if (!params)
    {
        return QCARCAM_RET_BADPARAM;
    }
#ifdef AIS_EARLYSERVICE
    // Run app only if RVC file created in target
    if(access("/early_services/rvc", F_OK))
    {
        QCARCAM_ERRORMSG("rvc file not available, exiting rvc app");
        place_marker("rvc file not found on target, exiting");
        return QCARCAM_RET_FAILED; // return FAILED to exit test app
    }
#endif
    pCtxt = (test_util_ctxt_t*)calloc(1, sizeof(struct test_util_ctxt_t));
    if (!pCtxt)
    {
        return QCARCAM_RET_NOMEM;
    }

    pCtxt->params = *params;

    /* Init graphics and drm display */
    if (!params->disable_display)
    {
        /* Init graphics and drm display */
        rc = graphics_display_init(pCtxt, DRM_WINDOW_HEIGHT, DRM_WINDOW_WIDTH, DRM_FORMAT_ARGB8888);
    }

    *ctxt = pCtxt;

    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_init_window
///
/// @brief Initialize window for display
///
/// @param ctxt             Pointer to test_util_ctxt
/// @param pp_window        Pointer to window pointer
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_init_window(test_util_ctxt_t *ctxt, test_util_window_t **pp_window)
{
    (void)ctxt;
    *pp_window = (test_util_window_t*)calloc(1, sizeof(struct test_util_window_t));
    if (!*pp_window)
    {
        return QCARCAM_RET_NOMEM;
    }

    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_init_window_buffers
///
/// @brief Initialize buffers for display
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
/// @param buffers          Pointer to qcarcam buffers
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_init_window_buffers(test_util_ctxt_t *ctxt, test_util_window_t *p_window, QCarCamBufferList_t *buffers)
{
    QCarCamRet_e rc;

    p_window->n_buffers = buffers->n_buffers;
    p_window->buffers = (test_util_buffer_t*)calloc(p_window->n_buffers, sizeof(*p_window->buffers));
    if(NULL == p_window->buffers)
    {
        QCARCAM_ERRORMSG("Failed to allocate buffer list");
        return QCARCAM_RET_NOMEM;
    }

    p_window->width = buffers->buffers[0].planes[0].width;
    p_window->height = buffers->buffers[0].planes[0].height;
#ifdef C2D_DISABLED
    // TODO::Remove this RGB buffer once GFX YUV pipeline fixed
    p_window->rgbBuffer = (uint32_t*)malloc((size_t)(p_window->width*4*p_window->height));
#endif

    QCARCAM_DBGMSG("Buffers %d width = %d height = %d",
            p_window->n_buffers, p_window->width, p_window->height);

    rc = alloc_ion_buffers(p_window, buffers);
    if (rc != QCARCAM_RET_OK)
    {
        free(p_window->buffers);
        p_window->buffers = NULL;

        if (!ctxt->params.disable_display)
        {
            graphics_display_deinit(ctxt, p_window);
        }
    }

#ifndef C2D_DISABLED
    if (!ctxt->params.disable_display)
    {
        for (int i = 0; i < MAX_BUFFER; i++)
        {
            rc = test_util_create_c2d_surface(ctxt, p_window, i);
            if (rc != QCARCAM_RET_OK)
            {
                QCARCAM_ERRORMSG("GL_DRM : c2d_surface create for source failed for id = %d", i);
                return rc;
            }
            else
            {
                QCARCAM_DBGMSG("GL_DRM : c2d_surface create for source passed for id = %d surface_id %u", i, ctxt->pesplash_dev->plane_cfg[0].bo[i].c2d_surface_id);
            }
        }
    }
#endif

    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_post_window_buffer
///
/// @brief Send frame to display
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
/// @param idx              Frame ID number
/// @param p_rel_buf_idx    List to fill with buffers ready to release
/// @param field_type     Field type in current frame buffer if interlaced
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_post_window_buffer(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx,
        std::list<uint32>* p_rel_buf_idx, QCarCamInterlaceField_e field_type)
{
    QCarCamRet_e rc = QCARCAM_RET_OK;
    int ret = 0;
    static struct timespec previous;
    struct timespec now;

    (void)field_type;

    clock_gettime(CLOCK_MONOTONIC_RAW, &now);

    uint64_t delta_us = (now.tv_sec - previous.tv_sec) * 1000000 + (now.tv_nsec - previous.tv_nsec) / 1000;
    QCARCAM_DBGMSG("GL_DRM - TIME_PROFILE : from consecutive frames delta_us %" PRIu64"", delta_us);

    previous.tv_sec = now.tv_sec;
    previous.tv_nsec = now.tv_nsec;
    if (!ctxt->params.disable_display)
    {
        if (ctxt->pesplash_dev)
        {
#ifdef C2D_DISABLED
            test_util_buffer_t* buffer = &p_window->buffers[idx];
            /* Software Conversion */
            /* Convert buffer into RGB buffer */
            convert_UYVY_RGBA_buffer((uint32_t*)buffer->ptr[0], p_window->rgbBuffer, p_window->width, p_window->height);

            /* Draw the image using gl APIs */
            graphics_draw_from_buffer(ctxt, p_window->rgbBuffer);

            /* Send graphics output to display */
            fill_display_buffer(ctxt, idx);
#else
            /* C2d Conversion */
            rc = graphics_draw_c2d_uyvy(ctxt, p_window, idx);
#endif
            /* Draw it on the display */
            struct timespec start, end;
            clock_gettime(CLOCK_MONOTONIC_RAW, &start);

            ret = esplash_draw(ctxt);
            if (0 !=  ret)
            {
                QCARCAM_ERRORMSG("esplash_draw failed with ret %d", ret);
                rc = QCARCAM_RET_FAILED;
            }
            clock_gettime(CLOCK_MONOTONIC_RAW, &end);
            delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
            QCARCAM_DBGMSG("GL_DRM - TIME_PROFILE : esplash_draw delta_us %" PRIu64"", delta_us);
        }
        else
            QCARCAM_ERRORMSG("ctxt->pesplash_dev is null");
    }

    if (p_rel_buf_idx)
    {
        // buffer is fully consumed and can be released
        p_rel_buf_idx->push_back(idx);
    }

    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_dump_window_buffer
///
/// @brief Dump frame to a file
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
/// @param idx              Frame ID number
/// @param filename         Char pointer to file name to be dumped
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_dump_window_buffer(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx, const char *filename)
{
    (void)ctxt;
    FILE *fp;
    size_t numBytesWritten = 0;
    size_t numByteToWrite = 0;

    if (!p_window->buffers[idx].ptr[0])
    {
        QCARCAM_ERRORMSG("buffer is not mapped");
        return QCARCAM_RET_FAILED;
    }

    fp = fopen(filename, "w+");

    QCARCAM_ERRORMSG("dumping qcarcam frame %s", filename);

    if (0 != fp)
    {
        test_util_buffer_t* buffer = &p_window->buffers[idx];

        numByteToWrite = buffer->size[0];
        numBytesWritten = fwrite(buffer->ptr[0], 1, buffer->size[0], fp);

        if (p_window->format == TESTUTIL_FMT_NV12)
        {
            numByteToWrite += buffer->size[1];
            numBytesWritten += fwrite(buffer->ptr[1], 1, buffer->size[1], fp);
        }

        if (numBytesWritten != numByteToWrite)
        {
            QCARCAM_ERRORMSG("error no data written to file");
        }

        fclose(fp);
    }
    else
    {
        QCARCAM_ERRORMSG("failed to open file");
        return QCARCAM_RET_FAILED;
    }
    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_get_buf_ptr
///
/// @brief Get buffer virtual address
///
/// @param p_window       window
/// @param p_buf          pointer to buffer structure to be filled
///
/// @return Void
///////////////////////////////////////////////////////////////////////////////
void test_util_get_buf_ptr(test_util_window_t *p_window, test_util_buf_ptr_t *p_buf)
{
    int idx = p_buf->buf_idx % p_window->n_buffers;

    p_buf->p_va[0] = (unsigned char *)p_window->buffers[idx].ptr[0];
    p_buf->stride[0] = p_window->buffers[idx].stride[0];
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_deinit
///
/// @brief Destroy context and free memory.
///
/// @param ctxt   Pointer to context to be destroyed
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_deinit(test_util_ctxt_t *ctxt)
{
    (void)ctxt;
    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_create_c2d_surface
///
/// @brief Create a C2D surface for source/camera
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
/// @param idx              Frame ID number
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_create_c2d_surface(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx)
{
    (void)ctxt;

#ifndef C2D_DISABLED
    int stride = qcarcam_get_stride(QCARCAM_FMT_UYVY_8, p_window->width);
    void *gpuaddr = NULL;
    int length = stride * p_window->height;
    length = (length + 4096 - 1) & ~(4096 - 1);

    C2D_STATUS c2d_status;
    C2D_YUV_SURFACE_DEF c2d_yuv_surface_def = {};
    C2D_SURFACE_TYPE surface_type = (C2D_SURFACE_TYPE)(C2D_SURFACE_YUV_HOST | C2D_SURFACE_WITH_PHYS);
    c2d_yuv_surface_def.format = C2D_COLOR_FORMAT_422_UYVY;
    c2d_yuv_surface_def.width = p_window->width;
    c2d_yuv_surface_def.height = p_window->height;
    c2d_yuv_surface_def.stride0 = stride;
    c2d_yuv_surface_def.stride1 = stride;
    c2d_yuv_surface_def.plane0 = p_window->buffers[idx].ptr[0];
    c2d_yuv_surface_def.plane1 = (void *) ((unsigned char *)c2d_yuv_surface_def.plane0 + (p_window->width * p_window->height));

    c2d_status = c2dMapAddr(p_window->buffers[idx].ion_map_fd, p_window->buffers[idx].ptr[0], length, 0, KGSL_USER_MEM_TYPE_ION, &gpuaddr);
    if (c2d_status != C2D_STATUS_OK)
    {
        QCARCAM_ERRORMSG("c2dMapAddr failed for id %d status %d", idx, c2d_status);
        return QCARCAM_RET_FAILED;
    }
    c2d_yuv_surface_def.phys0 = gpuaddr;
    p_window->buffers[idx].gpuaddr = gpuaddr;

    c2d_status = c2dCreateSurface(&p_window->buffers[idx].c2d_surface_id,
            C2D_SOURCE,
            surface_type,
            &c2d_yuv_surface_def);

    if (c2d_status != C2D_STATUS_OK)
    {
        QCARCAM_ERRORMSG("c2dCreateSurface failed for id %d status %d", idx, c2d_status);
        return QCARCAM_RET_FAILED;
    }
#endif
    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_update_c2d_surface
///
/// @brief Update already created C2D surface
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
/// @param idx              Frame ID number
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_update_c2d_surface(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx)
{
    (void)ctxt;

#ifndef C2D_DISABLED
    int stride = qcarcam_get_stride(QCARCAM_FMT_UYVY_8, p_window->width);

    C2D_STATUS c2d_status;
    C2D_YUV_SURFACE_DEF c2d_yuv_surface_def = {};
    C2D_SURFACE_TYPE surface_type = (C2D_SURFACE_TYPE)(C2D_SURFACE_YUV_HOST | C2D_SURFACE_WITH_PHYS);
    c2d_yuv_surface_def.format = C2D_COLOR_FORMAT_422_UYVY;
    c2d_yuv_surface_def.width = p_window->width;
    c2d_yuv_surface_def.height = p_window->height;
    c2d_yuv_surface_def.stride0 = stride;
    c2d_yuv_surface_def.stride1 = stride;
    c2d_yuv_surface_def.plane0 = p_window->buffers[idx].ptr[0];
    c2d_yuv_surface_def.plane1 = (void *) ((unsigned char *)c2d_yuv_surface_def.plane0 + (p_window->width * p_window->height));
    c2d_yuv_surface_def.phys0 = p_window->buffers[idx].gpuaddr;
    c2d_yuv_surface_def.format = C2D_COLOR_FORMAT_422_UYVY;

    c2d_status = c2dUpdateSurface(p_window->buffers[idx].c2d_surface_id,
            C2D_SOURCE,
            surface_type,
            &c2d_yuv_surface_def);

    if (c2d_status != C2D_STATUS_OK)
    {
        QCARCAM_ERRORMSG("c2dUpdateSurface failed for id %d status %d", idx, c2d_status);
        return QCARCAM_RET_FAILED;
    }
#endif
    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_create_c2d_surface
///
/// @brief Create a C2D surface for target/display
///
/// @param ctxt             Test util context
/// @param idx              Frame ID number
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_create_c2d_surface_target(test_util_ctxt_t *ctxt, unsigned int idx)
{
#ifndef C2D_DISABLED
    int stride = ctxt->pesplash_dev->plane_cfg[0].w * 4;
    void *gpuaddr = NULL;
    int length = stride * ctxt->pesplash_dev->plane_cfg[0].h;
    length = (length + 4096 - 1) & ~(4096 - 1);

    C2D_STATUS c2d_status;
    C2D_YUV_SURFACE_DEF c2d_yuv_surface_def = {};
    C2D_SURFACE_TYPE surface_type = (C2D_SURFACE_TYPE)(C2D_SURFACE_RGB_HOST | C2D_SURFACE_WITH_PHYS);
    c2d_yuv_surface_def.format = C2D_COLOR_FORMAT_8888_ARGB;
    c2d_yuv_surface_def.width = ctxt->pesplash_dev->plane_cfg[0].w;
    c2d_yuv_surface_def.height = ctxt->pesplash_dev->plane_cfg[0].h;
    c2d_yuv_surface_def.stride0 = stride;
    c2d_yuv_surface_def.stride1 = stride;
    c2d_yuv_surface_def.plane0 = ctxt->pesplash_dev->plane_cfg[0].bo[idx].plane[0];
    c2d_yuv_surface_def.plane1 = (void *) ((unsigned char *)c2d_yuv_surface_def.plane0 + (ctxt->pesplash_dev->width * ctxt->pesplash_dev->height));

    c2d_status = c2dMapAddr(ctxt->pesplash_dev->plane_cfg[0].bo[idx].ion_map_fd, ctxt->pesplash_dev->plane_cfg[0].bo[idx].plane[0], length, 0, KGSL_USER_MEM_TYPE_ION, &gpuaddr);
    if (c2d_status != C2D_STATUS_OK)
    {
        QCARCAM_ERRORMSG("c2dMapAddr failed for id %d status %d", idx, c2d_status);
        return QCARCAM_RET_FAILED;
    }
    c2d_yuv_surface_def.phys0 = gpuaddr;
    ctxt->pesplash_dev->plane_cfg[0].bo[idx].gpuaddr = gpuaddr;

    c2d_status = c2dCreateSurface(&ctxt->pesplash_dev->plane_cfg[0].bo[idx].c2d_surface_id,
            C2D_SOURCE | C2D_TARGET,
            surface_type,
            &c2d_yuv_surface_def);

    if (c2d_status != C2D_STATUS_OK)
    {
        QCARCAM_ERRORMSG("c2dCreateSurface %d buf %d failed %d", idx, c2d_status);
        return QCARCAM_RET_FAILED;
    }
#endif
    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_get_c2d_surface_id
///
/// @brief Get the ID from a C2D surface
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
/// @param idx              Frame ID number
/// @param surface_id       Pointer to C2D sruface ID number
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_get_c2d_surface_id(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx, unsigned int *surface_id)
{
    (void)ctxt;

    if (!surface_id)
        return QCARCAM_RET_BADPARAM;

    *surface_id = p_window->buffers[idx].c2d_surface_id;

    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_deinit_window
///
/// @brief Destroy window
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_deinit_window(test_util_ctxt_t *ctxt, test_util_window_t *p_window)
{
    (void)ctxt;

    // Deinit window/surface if any
    free(p_window);

    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_deinit_window_buffer
///
/// @brief Destroy window buffers
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_deinit_window_buffer(test_util_ctxt_t *ctxt, test_util_window_t *p_window)
{
    int i = 0;

    if (!ctxt->params.disable_display)
    {
        graphics_display_deinit(ctxt, p_window);
    }

#ifdef C2D_DISABLED
    // TODO::Remove this RGB buffer once GFX YUV pipeline fixed
    if (p_window->rgbBuffer)
        free(p_window->rgbBuffer);
#endif

    if (p_window->buffers)
    {
        for (i=0; i<p_window->n_buffers; i++)
        {
            if (p_window->buffers[i].ptr[0])
            {
                if (munmap(p_window->buffers[i].ptr[0],
                        p_window->buffers[i].ion_alloc_data.len) == -1)
                {
                    QCARCAM_ERRORMSG("QCDEBUG: munmap FAILED\n");
                }
                close(p_window->buffers[i].ion_map_fd);
                p_window->buffers[i].ptr[0] = NULL;
            }
        }
        close(p_window->ion_fd);
        free(p_window->buffers);
        p_window->buffers = NULL;
    }

    memset(p_window, 0x0, sizeof(*p_window));

    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_set_window_param
///
/// @brief Send window parameters to display
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
/// @param window_params    Pointer to structure with window properties
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_set_window_param(test_util_ctxt_t *ctxt, test_util_window_t *p_window, test_util_window_param_t *window_params)
{
    (void)ctxt;
    (void)p_window;
    (void)window_params;
    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_set_diag
///
/// @brief set the diagnostic structure to test_util_window_t
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
/// @param diag             diagnostic structure
///
/// @return Void
///////////////////////////////////////////////////////////////////////////////
void test_util_set_diag(test_util_ctxt_t *ctxt, test_util_window_t *p_window, test_util_diag_t* diag)
{
    (void)ctxt;
    (void)diag;
    if(p_window)
    {
        p_window->diag = diag;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_gpio_interrupt_config
///
/// @brief enable IO privileges, configure the gpio and set it up for interrupts
///
/// @param intr             Pointer for the IRQ to be stored
/// @param gpio_number      Specific gpio that is being utilized
/// @param trigger          Instance of the signal which shall causes the interrupt
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_gpio_interrupt_config(uint32_t *intr, int gpio_number, test_util_trigger_type_t trigger)
{
    (void)intr;
    (void)gpio_number;
    (void)trigger;
    return QCARCAM_RET_UNSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_interrupt_attach
///
/// @brief create a thread to handle the interrupt
///
/// @param arguments    arguments to pass to the newly created thread
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_interrupt_attach(test_util_intr_thrd_args_t *arguments)
{
    (void)arguments;
    return QCARCAM_RET_UNSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_interrupt_wait_and_unmask
///
/// @brief wait for a GPIO interrupt and then unmask it
///
/// @param irq              IRQ to unmask
/// @param interrupt_id     interrupt id to unmask
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_interrupt_wait_and_unmask(uint32_t irq, int interrupt_id)
{
    (void)irq;
    (void)interrupt_id;
    return QCARCAM_RET_UNSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_get_param
///
/// @brief get the value of the window parameter of the window
///
/// @param p_window         Pointer to window
/// @param param            window parameter you are trying to access
/// @param value            value of parameter will be stored here
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_get_param(test_util_window_t *p_window, test_util_params_t param, int *value)
{
    (void)p_window;
    (void)param;
    (void)value;
    return QCARCAM_RET_UNSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_set_param
///
/// @brief set the value of the window parameter
///
/// @param p_window         Pointer to window
/// @param param            window parameter you want to change
/// @param value            value you want to set the param to
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_set_param(test_util_window_t *p_window, test_util_params_t param, int value)
{
    (void)p_window;
    (void)param;
    (void)value;
    return QCARCAM_RET_UNSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_set_power_callback
///
/// @brief set power event callback
///
/// @return NULL
///////////////////////////////////////////////////////////////////////////////
void test_util_set_power_callback(power_event_callable pm_event_callback, void* p_usr_ctxt)
{
    g_pm_handle.p_power_event_callback = pm_event_callback;
    g_pm_handle.event_client_data      = p_usr_ctxt;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_allocate_input_buffers
///
/// @brief Allocate buffers for injection as input to qcarcam
///
/// @param p_window         Pointer to window
/// @param p_buffer_list    Pointer to buffer list
/// @param size             size to allocate
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_allocate_input_buffers(test_util_window_t*                  p_window,
                                                           qcarcam_bufferlist_t*    p_buffer_list,
                                                           unsigned int             size)
{
    return QCARCAM_RET_UNSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_free_input_buffers
///
/// @brief Free buffers allocated for injection as input to qcarcam
///
/// @param p_window         Pointer to window
/// @param p_buffer_list    Pointer to buffer list
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_free_input_buffers(test_util_window_t*              p_window,
                                                     qcarcam_bufferlist_t*  p_buffer_list)
{
    return QCARCAM_RET_UNSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_read_input_data
///
/// @brief Read input data into buffer list
///
/// @param p_window         Pointer to window
/// @param nframes          Number of frames stored in the file
/// @param filename         Path to data file to be read
/// @param size             Size of each frame
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_read_input_data(test_util_window_t*                 p_window,
                                                 const char*                filename,
                                                 int                        nframes,
                                                 size_t                     size)
{
    return QCARCAM_RET_UNSUPPORTED;
}

/*--------------------------------------------------------------------------
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
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

#define LOG_TAG "OMX-VDEC-RENDER"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include <drm.h>
#include <ion/ion.h>
#include <linux/msm_ion.h>
#include <libdrm_macros.h>
#include "omx_vdec_render_android.h"

#include <binder/ProcessState.h>

using namespace android;

unsigned int edrm_fd;
unsigned static int fb_id;

struct edrmmodeset_dev;
static int edrmmodeset_find_crtc(int fd, drmModeRes *res, drmModeConnector *conn,
                                struct edrmmodeset_dev *dev);
static int edrmmodeset_create_fb(int fb, int fd, struct edrmmodeset_dev *dev, int format);
static int edrmmodeset_setup_dev(int fd, drmModeRes *res, drmModeConnector *conn,
                                struct edrmmodeset_dev *dev);
static int edrmmodeset_open(int *out, const char *node);
static int edrmmodeset_prepare(int fd);
static int edrmmodeset_draw(int fd, struct OMX_BUFFERHEADERTYPE *p, unsigned int width, unsigned int height);
static void edrmmodeset_cleanup(int fd);

static int ALIGN(int x, int y) {
    // y must be a power of 2.
    return (x + y - 1) & ~(y - 1);
}

#ifdef NATIVEWINDOW_DISPLAY
sp<SurfaceComposerClient>   m_pSurfaceComposer;
sp<Surface>                 m_pSurface;
sp<SurfaceControl>          m_pControl;

int nativeWindow_buf_render(struct OMX_BUFFERHEADERTYPE *pBufHdr)
{
    sp<ANativeWindow> m_pNativeWindow = m_pSurface;
    int err;

    int mCropWidth = width;
    int mCropHeight = height;

    int halFormat = QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m;//colorspace
    int bufWidth = (mCropWidth + 1) & ~1;
    int bufHeight = (mCropHeight + 1) & ~1;

    DEBUGT_PRINT("showImageInNativeWindow width: %d, heitht: %d\n", width, height);

    native_window_api_connect(m_pNativeWindow.get(), NATIVE_WINDOW_API_MEDIA);

    CHECK_EQ(0,
            native_window_set_usage(
            m_pNativeWindow.get(),
            GRALLOC_USAGE_SW_READ_NEVER | GRALLOC_USAGE_SW_WRITE_OFTEN
            | GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_EXTERNAL_DISP));

    CHECK_EQ(0,
            native_window_set_scaling_mode(
            m_pNativeWindow.get(),
            NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW));

    CHECK_EQ(0, native_window_set_buffers_geometry(
                m_pNativeWindow.get(),
                bufWidth,
                bufHeight,
                halFormat));

    ANativeWindowBuffer *buf;
    if ((err = native_window_dequeue_buffer_and_wait(m_pNativeWindow.get(),
            &buf)) != 0) {
        printf("Surface::dequeueBuffer returned error %d", err);
        return -1;
    }

    GraphicBufferMapper &mapper = GraphicBufferMapper::get();
    Rect bounds(mCropWidth, mCropHeight);
    void *dst;
    CHECK_EQ(0, mapper.lock(
                buf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, bounds, &dst));

    const uint8_t *src_y = (const uint8_t *)(pBufHdr->pBuffer);
    if (true){
        size_t dst_y_size = buf->stride * ALIGN(buf->height, 32);
        size_t dst_c_stride = ALIGN(buf->stride / 2, 16);
        size_t dst_c_size = dst_c_stride * ALIGN(buf->height / 2, 16);

        memcpy(dst, src_y, dst_y_size + dst_c_size*2);
    }

    CHECK_EQ(0, mapper.unlock(buf->handle));

    if ((err = m_pNativeWindow->queueBuffer(m_pNativeWindow.get(), buf, -1)) != 0) {
        DEBUGT_PRINT_ERROR("native window queueBuffer returned error %d", err);
        buf = NULL;
        return -1;
    }
    buf = NULL;

    DEBUGT_PRINT("show the image in native window");

    DEBUGT_PRINT("render_fb complete!\n");

    return 0;
}
#endif

struct bo
{
    int fd;
    void *ptr;
    size_t size;
    size_t offset;
    size_t pitch;
    unsigned handle;
    void *plane[2];
#ifdef USE_ION
    unsigned ihdl;
    int ion_fd;
    int ion_map_fd;
#endif
};


struct edrmmodeset_dev {
    struct edrmmodeset_dev *next;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t conn;
    uint32_t crtc;
    struct bo *bo[2];
    uint32_t fb[2];
    drmModeModeInfo mode;
    drmModeRes *resources;
    drmModeEncoder *encoder;
    drmModeCrtc *saved_crtc;
};

static int edrmmodeset_open(int *out, const char *node)
{
    int fd=-1;
    int ret;
    uint64_t has_dumb;
    int retries = 0;
    DEBUGT_PRINT("Enter drmmodeset_open\n");

    while ((fd < 0) && (retries < EDRM_RETRY_CNTS)){
        fd = open(node, O_RDWR | O_CLOEXEC);
        if (fd < 0) {
            usleep(10000); // Sleep for 10 ms
            retries++;
        }
    }
    if (fd < 0) {
        ret = -errno;
        DEBUGT_PRINT_ERROR("cannot open '%s': %m\n", node);
        return ret;
    }

    if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 || !has_dumb) {
        DEBUGT_PRINT_ERROR("drm device '%s' does not support dumb buffers\n", node);
        close(fd);
        return -EOPNOTSUPP;
    }
    edrm_fd = fd;
    *out = fd;
    return 0;
}

static int format_to_bpp(uint32_t format)
{
    switch (format) {
        case DRM_FORMAT_NV12:
            return 12;
        case DRM_FORMAT_ARGB8888:
            case DRM_FORMAT_XRGB8888:
        case DRM_FORMAT_RGBA8888:
        default:
            return 32;
    }
}
static struct edrmmodeset_dev *edrmmodeset_list = NULL;

static int edrmmodeset_prepare(int fd)
{
    drmModeRes *res;
    drmModeConnector *conn;
    struct edrmmodeset_dev *dev;
    int ret;

    /* retrieve resources */
    res = drmModeGetResources(fd);
    if (!res) {
        DEBUGT_PRINT_ERROR("cannot retrieve DRM resources (%d): %m\n", errno);
        return -errno;
    }
    if(!res->connectors) {
        DEBUGT_PRINT_ERROR(" No connector found for fd (%d): %m\n", fd, errno);
        return -errno;
    }

    /* get information of connector */
    conn = drmModeGetConnector(fd, res->connectors[0]);
    if (!conn) {
        DEBUGT_PRINT_ERROR("cannot retrieve DRM connector (%d): %m\n",
                res->connectors[0], errno);
        return -errno;
    }

    /* create a device structure */
    dev = (struct edrmmodeset_dev *)malloc(sizeof(*dev));
    if(!dev) {
        DEBUGT_PRINT_ERROR("Failed to allocate edrmmodeset_dev device %m\n", errno);
        return -errno;
    }
    memset(dev, 0, sizeof(*dev));
    dev->conn = conn->connector_id;
    dev->resources = res;

    /* call helper function to prepare this connector */
    ret = edrmmodeset_setup_dev(fd, res, conn, dev);
    if (ret) {
        if (ret != -ENOENT) {
            errno = -ret;
            DEBUGT_PRINT_ERROR("cannot setup device for connector (%d): %m\n",
                    res->connectors[0], errno);
        }
        free(dev);
        drmModeFreeConnector(conn);
        return -errno;
    }
    /* link device into global list */
    dev->next = edrmmodeset_list;
    edrmmodeset_list = dev;

    return 0;
}

static int edrmmodeset_setup_dev(int fd, drmModeRes *res,
        drmModeConnector *conn, struct edrmmodeset_dev *dev)
{
    int ret, i;

    /* check if a monitor is connected */
    if (conn->connection != DRM_MODE_CONNECTED) {
        DEBUGT_PRINT_ERROR("ignoring unused connector %u\n", conn->connector_id);
        return -ENOENT;
    }

    /* check if there is at least one valid mode */
    if (conn->count_modes == 0) {
        DEBUGT_PRINT_ERROR("no valid mode for connector %u\n", conn->connector_id);
        return -EFAULT;
    }

    /* copy the mode information into our device structure */
    memcpy(&dev->mode, &conn->modes[0], sizeof(dev->mode));
    dev->width = conn->modes[0].hdisplay;
    dev->height = conn->modes[0].vdisplay;
    DEBUGT_PRINT("mode for connector %u is %ux%u\n",
            conn->connector_id, dev->width, dev->height);

    /* find a crtc for this connector */
    ret = edrmmodeset_find_crtc(fd, res, conn, dev);
    if (ret) {
        DEBUGT_PRINT_ERROR("no valid crtc for connector %u\n", conn->connector_id);
        return ret;
    }

    /* create a 2 framebuffer for this CRTC */
    for(i = 0; i < 2; ++i) {
        ret = edrmmodeset_create_fb(i,fd, dev, DRM_FORMAT_NV12);
        if (ret) {
            DEBUGT_PRINT_ERROR("cannot create framebuffer for connector %u\n",
                conn->connector_id);
            return ret;
        }
    }

    return 0;
}

static int edrmmodeset_find_crtc(int fd, drmModeRes *res, drmModeConnector *conn,
                                struct edrmmodeset_dev *dev)
{
    drmModeEncoder *enc;
    int i,j;
    uint32_t crtc;
    struct edrmmodeset_dev *iter;


    /* first try the currently conected encoder+crtc */
    if (conn->encoder_id)
        enc = drmModeGetEncoder(fd, conn->encoder_id);
    else
        enc = NULL;

    dev->encoder = enc;
    if (enc) {
        if (enc->crtc_id) {
            crtc = enc->crtc_id;
            for (iter = edrmmodeset_list; iter; iter = iter->next) {
                if (iter->crtc == crtc) {
                    crtc = -1;
                    break;
                }
            }
            if (crtc >= 0) {
                dev->crtc = crtc;
                return 0;
            }
        }
    }

    /* If the connector is not currently bound to an encoder or if the
     * encoder+crtc is already used by another connector (actually unlikely
     * but lets be safe), iterate all other available encoders to find a
     * matching CRTC. */
    for (i = 0; i < conn->count_encoders; ++i) {
        enc = drmModeGetEncoder(fd, conn->encoders[i]);
        dev->encoder = enc;
        if (!enc) {
            DEBUGT_PRINT_ERROR("cannot retrieve encoder %u:%u (%d): %m\n",
                    i, conn->encoders[i], errno);
            continue;
        }

        /* iterate all global CRTCs */
        for (j = 0; j < res->count_crtcs; ++j) {
            /* check whether this CRTC works with the encoder */
            if (!(enc->possible_crtcs & (1 << j))) {
                continue;
            }

            /* check that no other device already uses this CRTC */
            crtc = res->crtcs[j];
            for (iter = edrmmodeset_list; iter; iter = iter->next) {
                if (iter->crtc == crtc) {
                    crtc = -1;
                    break;
                }
            }

            /* we have found a CRTC, so save it and return */
            if (crtc >= 0) {
                dev->crtc = crtc;                        // --> actual CRTC we are going to use //
                return 0;
            }
        }
    }
    DEBUGT_PRINT("cannot find suitable CRTC for connector %u\n", conn->connector_id);
    return -ENOENT;
}


static struct bo *
bo_create_dumb_ion(int fd, unsigned int width, unsigned int height, unsigned int bpp)
{
    int ret;
    struct bo *bo;
    size_t page_size, frame_size;
    struct drm_prime_handle prime_req;
    int ion_fd = -1;
    while (access("/dev/ion", F_OK) != 0)
    {
        usleep(100);
    }

    bo->ion_map_fd = -1;
    ion_fd = ion_open();
    if (ion_fd < 0) {
        DEBUGT_PRINT_ERROR("failed to open ion_device\n");
        return NULL;
    }

    bo = (struct bo *)malloc(sizeof(*bo));
    if (bo == NULL) {
        close(ion_fd);
        return NULL;
    }

    page_size = sysconf(_SC_PAGESIZE);

    int aligned_width = ALIGN(width, 128);
    bo->pitch = aligned_width * bpp / 8;
    int slice_height = ALIGN(height, 32);
    frame_size = bo->pitch  * slice_height;

    /*(round up to page size(4K))*/
    bo->size = (frame_size + page_size - 1) & (~(page_size - 1));
    bo->offset = 0;
    bo->fd = fd;
    bo->ion_fd = ion_fd;

    if (0 > ion_alloc_fd(ion_fd, bo->size, page_size,
                ION_HEAP(ION_SYSTEM_HEAP_ID), 0, &bo->ion_map_fd))
    {
        DEBUGT_PRINT_ERROR("ion_alloc_fd failed\n");
        goto error;
    }

    prime_req.fd = bo->ion_map_fd;
    ret = drmIoctl(fd, DRM_IOCTL_PRIME_FD_TO_HANDLE, &prime_req);
    if (ret) {
        ion_close(bo->ion_map_fd);
        DEBUGT_PRINT_ERROR("failed get prime handle: %d\n", ret);
        goto error;
    }

    bo->handle = prime_req.handle;

    return bo;

error:
    if (bo->ion_map_fd >= 0)
        ion_close(bo->ion_map_fd);

    if (ion_fd >= 0)
        ion_close(ion_fd);

    free(bo);
    return NULL;
}

static int bo_ion_map(struct bo *bo)
{
    void *map;

    map = drm_mmap(0, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED,
            bo->ion_map_fd, bo->offset);
    if (map == MAP_FAILED) {
        map = NULL;
        DEBUGT_PRINT_ERROR("Failed to map\n");
        return -EINVAL;
    }
    bo->ptr = map;

    return 0;
}

static void bo_unmap(struct bo *bo)
{
    if (!bo->ptr)
        return;

    drm_munmap(bo->ptr, bo->size);
    bo->ptr = NULL;
}

void bo_destroy(struct bo *bo)
{
    int ret;
    DEBUGT_PRINT_ERROR("calling bo_destroy\n");
    bo_unmap(bo);
    close(bo->ion_map_fd);
    close(bo->ion_fd);
    free(bo);
}


static int edrmmodeset_create_fb(int fb,int fd, struct edrmmodeset_dev *dev, int format)
{
    int ret;
    unsigned int handles[2], pitches[2], offsets[2];
    unsigned int bpp = format_to_bpp(format);

    /* create ion allocated dumb buffer */
    dev->bo[fb] = bo_create_dumb_ion(fd, dev->width, dev->height, bpp);
    if(!dev->bo[fb])
        DEBUGT_PRINT_ERROR("Failed to create dumb buffer\n");

    /* Map dumb buffer */
    if(bo_ion_map(dev->bo[fb]))
        DEBUGT_PRINT_ERROR("Failed to map dumb buffer\n");

    handles[0] = dev->bo[fb]->handle;
    pitches[0] = dev->width;
    offsets[0] = 0;
    handles[1] = dev->bo[fb]->handle;
    pitches[1] = pitches[0];
    offsets[1] = pitches[0] * dev->height;
    dev->bo[fb]->plane[0] = (unsigned char *)dev->bo[fb]->ptr;
    memset(dev->bo[fb]->plane[0], 0, dev->width * dev->height);
    dev->bo[fb]->plane[1] = (unsigned char *)dev->bo[fb]->ptr + dev->width * dev->height;
    memset(dev->bo[fb]->plane[1], 128, (dev->width * dev->height) >> 1);

    ret = drmModeAddFB2(fd, dev->width, dev->height, DRM_FORMAT_NV12, handles, pitches, offsets, &dev->fb[fb], 0);
    if (ret) {
        DEBUGT_PRINT_ERROR("cannot create framebuffer\n");
    }

    dev->stride = dev->width;

    return ret;
}

void init(){

}

void deInit(){

}

void init_queue(){
    queue_buffer_list = alloc_queue();
}

void clear_queue(){
    free_queue_and_qelement(queue_buffer_list);
}

void clear_all_queue(){
    free_queue_and_qelement(queue_buffer_list);
}

bool is_empty_queue(){
    if ( queue_buffer_list->current_size > 0){
        return false;
    } else {
        return true;
    }
}

struct queue_buffer* pop_queue(){
    return (struct queue_buffer *)pop(queue_buffer_list);
}

void push_queue(struct queue_buffer *queue_buffer){
    push(queue_buffer_list, queue_buffer);
}

int queue_length(){
    return queue_buffer_list->current_size;
}

#ifdef NATIVEWINDOW_DISPLAY
int open_display()
{
    // create a client to surfaceflinger
    ProcessState::initWithDriver("/dev/binder");
    m_pSurfaceComposer = new SurfaceComposerClient();
    sp<IBinder> dtoken = SurfaceComposerClient::getInternalDisplayToken();
    if (dtoken == nullptr) {
        DEBUGT_PRINT_ERROR("No internal display\n");
        return -1;
    }
    DisplayInfo dinfo;

    status_t status = SurfaceComposerClient::getDisplayInfo(dtoken, &dinfo);
    if (status) {
        DEBUGT_PRINT_ERROR("Unable to get display characterstics\n");
        return -1;
    }
    printf("w=%d,h=%d,xdpi=%f,ydpi=%f,fps=%f,ds=%f\n",
        dinfo.w, dinfo.h, dinfo.xdpi, dinfo.ydpi, dinfo.fps, dinfo.density);
    //create surface
    m_pControl = m_pSurfaceComposer->createSurface(String8("videoearly"),
            dinfo.w, dinfo.h, PIXEL_FORMAT_RGBA_8888, 0);

    printf("width=%d,height=%d\n", width, height);
/*********************config surface*******************************************************************/
    SurfaceComposerClient::Transaction mTransaction;
    mTransaction.setLayer(m_pControl, 100000);
    mTransaction.setPosition(m_pControl, 0, 0);
    mTransaction.setSize(m_pControl, dinfo.w, dinfo.h);
    mTransaction.apply();
    m_pSurface = m_pControl->getSurface();
    printf("[%s][%d]\n",__FILE__,__LINE__);

    return 0;
}
#endif

int open_edrmdisplay()
{
    int ret = 0;
    int fd;
    const char *card;
    struct edrmmodeset_dev *iter;

    /* open eDRM device */
    card = "/dev/dri/card1";

    ret = edrmmodeset_open(&fd, card);
    if (ret) {
        DEBUGT_PRINT_ERROR("closing edrm display fd\n");
        return -1;
    }
    place_marker(" === edrmmodeset_open success");

    /* prepare connectors and CRTCs
     * for edrm, only one connector is supported DSI-0/HDMI */
    ret = edrmmodeset_prepare(fd);
    if (ret) {
        DEBUGT_PRINT_ERROR("edrm_mode_set failed for connector \n");
        goto out_close;
    }

    /* perform actual modesetting on each found connector+CRTC */
    iter = edrmmodeset_list;
    iter->saved_crtc = drmModeGetCrtc(fd, iter->crtc);
    ret = drmModeSetCrtc(fd, iter->crtc, iter->fb[1], 0, 0, &iter->conn, 1, &iter->mode);
    if(ret) {
        DEBUGT_PRINT_ERROR("cannot set CRTC for connector %u (%d): %m\n",
                iter->conn, errno);
        goto out_close;
    }
    fb_id = 1;

    return ret;

out_close:
    edrmmodeset_cleanup(fd);
    return -1;
}

static int edrmmodeset_draw(int fd, struct OMX_BUFFERHEADERTYPE *p, unsigned int FrameWidth, unsigned int FrameHeight)
{
    unsigned int h, i, ret;
    unsigned int h_offset, w_offset, luma_offset,chroma_offset;
    unsigned int ysub = 2;
    unsigned int uv = 2;    /* UV_stride */
    unsigned char *tempY = NULL;
    unsigned char *tempUV = NULL;
    unsigned char *luma, *chroma;
    unsigned int scanlines;
    struct edrmmodeset_dev *iter = edrmmodeset_list;
    if(iter == NULL) {
        DEBUGT_PRINT_ERROR("Exit rendering - no edrm-display node\n");
        return -1;
    }

    h_offset = (iter->height - FrameHeight) / 2;
    w_offset = (iter->width - FrameWidth) / 2;
    luma_offset = iter->stride * h_offset + w_offset;
    chroma_offset = (iter->stride / uv) * h_offset + w_offset;

    scanlines = VENUS_Y_SCANLINES(COLOR_FMT_NV12, FrameHeight);
    tempY = (unsigned char *) p->pBuffer;
    tempUV = tempY + FrameWidth * scanlines;

    fb_id++;
    if(fb_id > 1)
        fb_id = 0;

    ret = drmModeSetCrtc(fd, iter->crtc, iter->fb[fb_id], 0, 0, &iter->conn, 1, &iter->mode);
    if(ret) {
        DEBUGT_PRINT_ERROR("cannot set CRTC for connector %u (%d): %m\n", iter->conn, errno);
        return -1;
    }

    luma = ( unsigned char*)iter->bo[fb_id]->plane[0];
    chroma = ( unsigned char*)iter->bo[fb_id]->plane[1];

    for(h = 0; h < FrameHeight; ++h){
        memcpy(&luma[luma_offset], tempY, FrameWidth);
        luma += iter->stride;
        tempY += FrameWidth;
    }

    for(i = 0; i < FrameHeight / ysub; ++i){
        memcpy(&chroma[chroma_offset], tempUV, FrameWidth);
        chroma += iter->stride;
        tempUV += FrameWidth;
    }

    return 0;
}

static void edrmmodeset_cleanup(int fd)
{
    struct edrmmodeset_dev *iter;

    while (edrmmodeset_list) {
        /* remove from global list */
        iter = edrmmodeset_list;
        edrmmodeset_list = iter->next;

        drmModeFreeEncoder(iter->encoder);
        drmModeFreeResources(iter->resources);

        /* restore saved CRTC configuration */
        drmModeSetCrtc(fd,
                iter->saved_crtc->crtc_id,
                iter->saved_crtc->buffer_id,
            iter->saved_crtc->x,
                iter->saved_crtc->y,
                &iter->conn,
                1,
                &iter->saved_crtc->mode);
        drmModeFreeCrtc(iter->saved_crtc);

        /* unmap buffer */
        munmap(iter->bo[0]->ptr, iter->bo[0]->size);
        munmap(iter->bo[1]->ptr, iter->bo[1]->size);

        /* delete framebuffer */
        drmModeRmFB(fd, iter->fb[0]);
        drmModeRmFB(fd, iter->fb[1]);

        /* delete dumb buffer */
        bo_destroy(iter->bo[0]);
        bo_destroy(iter->bo[1]);

        /* free allocated memory */
        free(iter);
    }
    close(fd);
}

void close_display()
{
    return;
}

void *open_edrmdisplay_thread(void* pArg) {
    if (open_edrmdisplay() != 0) {
        DEBUGT_PRINT_ERROR("Failed to open edrm display node\n");
        return NULL;
    } else {
        opendisplay = 0;
    }
    return NULL;
}

#ifdef NATIVEWINDOW_DISPLAY
void* open_display_thread(void* pArg) {
   if (open_display() != 0) {
       DEBUGT_PRINT_ERROR(" Error opening display! Video won't be displayed...");
       return NULL;
    } else {
       isdisplayopened = true;
    }
    return NULL;
}
#endif

void display_marker() {
    return;
}

int display_render(struct OMX_BUFFERHEADERTYPE *pBuffer, int frameWidth, int frameHeight, uint32_t end_playback) {
    int ret = 0;
#ifdef NATIVEWINDOW_DISPLAY
    if (displayYuv && opendisplay) {
        if (open_display() != 0) {
            printf("\n Error opening display! Video won't be displayed...");
            displayYuv = 0;
        }else{
            opendisplay = 0;
        }
    }
    if(opendisplay==0){
        ret = nativeWindow_buf_render(pBuffer);
    }
#else
#ifdef EARLY_BOOTVIDEO
    if(opendisplay==0){
        ret = edrmmodeset_draw(edrm_fd, pBuffer, frameWidth, frameHeight);
    }
    if(end_playback){
        DEBUGT_PRINT_ERROR("closing edrm display node\n");
        edrmmodeset_cleanup(edrm_fd);
    }
#endif
#endif

    return ret;
}

void free_ion(int index){
    (void) index;
    return;
}
void free_outport_ion(){
    return;
}

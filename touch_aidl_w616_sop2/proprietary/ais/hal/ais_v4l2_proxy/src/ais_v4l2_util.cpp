/* ===========================================================================
 * Copyright (c) 2019, 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 =========================================================================== */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <linux/msm_ion.h>
#include <linux/ion.h>

#include <sys/mman.h>

#include "ais_v4l2_proxy_util.h"

#define KGSL_USER_MEM_TYPE_ION 3

#define ION_NODE "/dev/ion"

struct ais_util_buffer {
    size_t size;
    void* data;
    uint32_t flags;
};

static unsigned int qcarcam_get_stride(qcarcam_color_fmt_t fmt, unsigned int width)
{
    unsigned int stride = 0;
    switch(fmt)
    {
        case QCARCAM_FMT_RGB_888:
            stride = width*3;
            break;
        case QCARCAM_FMT_MIPIRAW_8:
            stride = width;
            break;
        case QCARCAM_FMT_MIPIRAW_10:
            if (0 == (width % 4))
                stride = width*5/4;
            break;
        case QCARCAM_FMT_MIPIRAW_12:
            if (0 == (width % 2))
                stride = width*3/2;
            break;
        case QCARCAM_FMT_UYVY_8:
            return width*2;
        case QCARCAM_FMT_UYVY_10:
            if (0 == (width % 4))
                stride = width*2*5/4;
            break;
        case QCARCAM_FMT_UYVY_12:
            if (0 == (width % 2))
                stride = width*2*3/2;
            break;
        case QCARCAM_FMT_NV12:
            if (0 == (width % 2))
                stride = width;
            break;
        default:
            break;
    }

    return stride;
}

static void test_util_fill_planes(qcarcam_buffer_t* pBuffer, qcarcam_color_fmt_t fmt)
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
        pBuffer->planes[0].stride = pBuffer->planes[0].width * 2;
        pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        break;
    case QCARCAM_FMT_UYVY_10:
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
static unsigned int v4l2_complement_height_to_size(unsigned int fmt, unsigned int width)
{
    unsigned int complement;
    switch(fmt)
    {
        case V4L2_PIX_FMT_UYVY:
            complement = width * 2;
            break;
        case V4L2_PIX_FMT_NV12:
            complement = width * 3 / 2;
            break;
        case V4L2_PIX_FMT_YUV420:
            complement = width;
            break;
        case V4L2_PIX_FMT_RGB24:
            complement = width * 3;
            break;
        default:
            complement = width;
            break;
    }

    return complement;
}

qcarcam_color_fmt_t get_qcarcam_format_from_v4l2(unsigned int fmt)
{
    qcarcam_color_fmt_t q_fmt = QCARCAM_FMT_UYVY_8;
    switch(fmt)
    {
        case V4L2_PIX_FMT_UYVY:
            q_fmt = QCARCAM_FMT_UYVY_8;
            break;
        case V4L2_PIX_FMT_NV12:
            q_fmt = QCARCAM_FMT_NV12;
            break;
        case V4L2_PIX_FMT_YUV420:
            q_fmt = QCARCAM_FMT_UYVY_12;
            break;
        case V4L2_PIX_FMT_RGB24:
            q_fmt = QCARCAM_FMT_RGB_888;
            break;
        default:
            break;
    }

    return q_fmt;
}

static int qcarcam_to_native_color_format(qcarcam_color_fmt_t fmt)
{
    switch(fmt)
    {
        case QCARCAM_FMT_MIPIRAW_8:
        case QCARCAM_FMT_MIPIRAW_10:
        case QCARCAM_FMT_MIPIRAW_12:
        case QCARCAM_FMT_RGB_888:
            return HAL_PIXEL_FORMAT_BGR_888;
        case QCARCAM_FMT_UYVY_8:
        case QCARCAM_FMT_UYVY_10:
        case QCARCAM_FMT_UYVY_12:
        case QCARCAM_FMT_NV12:
            return HAL_PIXEL_FORMAT_NV12_ENCODEABLE;
        case QCARCAM_FMT_YUYV_8:
        case QCARCAM_FMT_YUYV_10:
        case QCARCAM_FMT_YUYV_12:
        default:
            return HAL_PIXEL_FORMAT_CbYCrY_422_I;
            break;
    }
    return 0;
}

static void alloc_ion_buffers(qcarcam_frame_buffer_t *user_ctxt, qcarcam_buffers_t *buffers)
{

    int i;
    struct ion_allocation_data ion_alloc_data[buffers->n_buffers];
    int ret;
    size_t page_size;

    user_ctxt->format = buffers->color_fmt;
    user_ctxt->n_buffers = buffers->n_buffers;
    user_ctxt->buffer_size[0] = buffers->buffers[0].planes[0].width;
    user_ctxt->buffer_size[1] = buffers->buffers[0].planes[0].height;
    user_ctxt->stride = qcarcam_get_stride(buffers->color_fmt, buffers->buffers[0].planes[0].width);

    QCARCAM_DBGMSG("Buffers %d width = %d height = %d \n",
                user_ctxt->n_buffers, user_ctxt->buffer_size[0], user_ctxt->buffer_size[1]);


    user_ctxt->buffers = (ais_buffer_t*)calloc(user_ctxt->n_buffers, sizeof(*user_ctxt->buffers));
    user_ctxt->pbuf = (void**)calloc(user_ctxt->n_buffers, sizeof(void*));
    if(NULL == user_ctxt->buffers)
    {
        QCARCAM_ERRORMSG("Value of user_ctxt->buffers is NULL");
        return;
    }

    if(NULL == user_ctxt->pbuf)
    {
        QCARCAM_ERRORMSG("Value of user_ctxt->pbuf is NULL");
        return;
    }

    for (i = 0; i < user_ctxt->n_buffers; i++)
    {
        uint32 allocSize;
        test_util_fill_planes(&buffers->buffers[i], buffers->color_fmt);

        if (buffers->buffers[i].n_planes == 2)
        {
            user_ctxt->buffers[i].size = buffers->buffers[i].planes[0].size + buffers->buffers[i].planes[1].size;
        }
        else
        {
            user_ctxt->buffers[i].size = buffers->buffers[i].planes[0].size;
        }

        allocSize = user_ctxt->buffers[i].size;

        page_size = sysconf(_SC_PAGESIZE);
        memset(&ion_alloc_data[i], 0, sizeof(ion_alloc_data[i]));

        ion_alloc_data[i].len = allocSize;
        ion_alloc_data[i].heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID);
        ion_alloc_data[i].flags = 0;
        ret = ioctl(user_ctxt->ion_dev, ION_IOC_ALLOC, &ion_alloc_data[i]);
        if (ret) {
            QCARCAM_ERRORMSG("failed to allocate memory from ion: %d", ret);
            return;
        }

        QCARCAM_DBGMSG("Success: map ion_device %d", ion_alloc_data[i].fd);

        buffers->buffers[i].planes[0].p_buf = (void*)(uintptr_t)ion_alloc_data[i].fd;

        if (user_ctxt->pbuf)
        {
            user_ctxt->pbuf[i] = buffers->buffers[i].planes[0].p_buf;
        }

        if (user_ctxt->buffers)
        {
            user_ctxt->buffers[i].fd = ion_alloc_data[i].fd;
        }

        user_ctxt->buffers[i].is_dequeud = 1;

        user_ctxt->buffers[i].ptr =
            mmap(NULL, user_ctxt->buffers[i].size,
                    PROT_READ | PROT_WRITE, MAP_SHARED,
                    user_ctxt->buffers[i].fd, 0);
    }
    return;
}


///////////////////////////////////////////////////////////////////////////////
/// qcarcam_init_ion_buffers
///
/// @brief Initialize window for display
///
/// @param ctxt             Pointer to display context
/// @param user_ctxt        Pointer to structure containing window parameters
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
qcarcam_ret_t qcarcam_init_ion_buffers(qcarcam_frame_buffer_t **user_ctxt, qcarcam_buffers_t *buffers)
{
    *user_ctxt = (qcarcam_frame_buffer_t*)calloc(1, sizeof(struct qcarcam_frame_buffer_t));
    if (!*user_ctxt)
    {
        return QCARCAM_RET_NOMEM;
    }

    (*user_ctxt)->ion_dev = open(ION_NODE, O_RDONLY | O_CLOEXEC);
    if ((*user_ctxt)->ion_dev < 0) {
        QCARCAM_ERRORMSG("failed to open ion_device");
        return QCARCAM_RET_FAILED;
    }

    alloc_ion_buffers(*user_ctxt, buffers);

    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// free_ion_buffer
///
/// @brief frees ion buffer
///
/// @param buffer           Pointer to buffer
///
///////////////////////////////////////////////////////////////////////////////
static void  free_ion_buffer(ais_buffer_t* buffer)
{
    int result = 0;
    result = close(buffer->fd);
    if (result == -1)
    {
        QCARCAM_ERRORMSG("ION descriptor(%d) close failed: %s", buffer->fd,
        strerror(errno));
    }
    buffer->fd = 0;
}

void ConvertUyvyToYuv420P1(uint8_t* destFrame, uint8_t* srcFrame, int width, int height)
{

    uint8_t* pyFrame = destFrame;
    uint8_t* pvFrame = pyFrame + width*height;
    uint8_t* puFrame = pvFrame+1;
    int uvOffset = width * 2 * sizeof(uint8_t);
    int i,j;

    for(i=0; i<height; i++)
    {
        for(j=0;j<width;j+=2)
        {
            uint16_t calc;
            if ((i&1) == 0)
            {
                calc = *srcFrame;
                calc += *(srcFrame + uvOffset);
                calc /= 2;
                *puFrame = (uint8_t) calc;
                puFrame+=2;
            }
            srcFrame++;
            *pyFrame++ = *srcFrame++;

            if ((i&1) == 0)
            {
                calc = *srcFrame;
                calc += *(srcFrame + uvOffset);
                calc /= 2;
                *pvFrame = (uint8_t) calc;
                pvFrame+=2;

            }
            srcFrame++;
            *pyFrame++ = *srcFrame++;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// post_ion_buffer_to_v4l2node
///
/// @brief Send frame to v4l2node
///
/// @param v4l2sink         v4l2node where data will be dumped
/// @param user_ctxt        Pointer to structure containing window parameters
/// @param idx              Frame ID number
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
qcarcam_ret_t post_ion_buffer_to_v4l2node(unsigned int v4l2sink, qcarcam_frame_buffer_t *user_ctxt, unsigned int idx, qcarcam_v4l2_param_t *ctxt)
{
    size_t numBytesWritten = 0;
    size_t numByteToWrite = 0;

    QCARCAM_DBGMSG("Posting %d buffer to v4l2node, width=%d, height=%d, format=%d", idx, ctxt->width, ctxt->height, ctxt->pixformat);

    if (!user_ctxt->buffers[idx].ptr)
    {
        QCARCAM_ERRORMSG("buffer is not mapped");
        return QCARCAM_RET_FAILED;
    }

    struct ais_util_buffer buffer = {
        .size = static_cast<size_t>(user_ctxt->buffers[idx].size),
        .data = user_ctxt->buffers[idx].ptr,
        .flags = ctxt->buffer_status
    };
    numByteToWrite = sizeof(buffer);

    numBytesWritten = write(v4l2sink, reinterpret_cast<uint8_t*>(&buffer), numByteToWrite);
    if (numBytesWritten != buffer.size)
    {
        QCARCAM_ERRORMSG("Error: Written only %zu bytes; %zu bytes should have been written instead",
            numBytesWritten,
            buffer.size);
    }

    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// qcarcam_dump_frame_buffer
///
/// @brief Dump frame to a file
///
/// @param ctxt             Pointer to display context
/// @param user_ctxt        Pointer to structure containing window parameters
/// @param idx              Frame ID number
/// @param filename         Char pointer to file name to be dumped
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
qcarcam_ret_t qcarcam_dump_frame_buffer(qcarcam_frame_buffer_t *user_ctxt, unsigned int idx, const char *filename)
{
    FILE *fp;
    size_t numBytesWritten = 0;
    size_t numByteToWrite = 0;
    unsigned char *pBuf = 0;

    if (!user_ctxt->buffers[idx].ptr)
    {
        QCARCAM_ERRORMSG("buffer is not mapped");
        return QCARCAM_RET_FAILED;
    }

    fp = fopen(filename, "w+");
    QCARCAM_ERRORMSG("dumping qcarcam frame %s numbytes %d", filename,
            user_ctxt->buffers[idx].size);

    if (0 != fp)
    {
        pBuf = (unsigned char *)user_ctxt->buffers[idx].ptr;
        numByteToWrite = (size_t)user_ctxt->buffers[idx].size;
        numBytesWritten = fwrite(pBuf, 1, numByteToWrite, fp);

        if (numBytesWritten != numByteToWrite)
        {
            QCARCAM_ERRORMSG("Error: Written only %zu bytes to file; %zu bytes should have been written instead",
                numBytesWritten,
                numByteToWrite);
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
/// qcarcam_deinit_ion_buffers
///
/// @brief Destroy window
///
/// @param ctxt             Pointer to display context
/// @param user_ctxt        Pointer to structure containing window parameters
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
qcarcam_ret_t qcarcam_deinit_ion_buffers(qcarcam_frame_buffer_t *user_ctxt)
{

    if (user_ctxt == NULL)
    {
        QCARCAM_DBGMSG("user_ctxt is NULL");
        return QCARCAM_RET_OK;
    }

    int i = 0;

    for (i=0; i<user_ctxt->n_buffers; i++)
    {
        if (user_ctxt->buffers[i].ptr)
        {
            free_ion_buffer(&(user_ctxt->buffers[i]));
            munmap(user_ctxt->buffers[i].ptr,
                    user_ctxt->buffers[i].size);
        }
    }

    if (user_ctxt->buffers)
    {
        free(user_ctxt->buffers);
    }

    if (user_ctxt->pbuf)
    {
        free(user_ctxt->pbuf);
    }

    if (user_ctxt->ion_dev)
    {
        close(user_ctxt->ion_dev);
        user_ctxt->ion_dev = 0;
    }

    // Deinit window/surface if any
    free(user_ctxt);

    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// create_c2d_surface
///
/// @brief Create a C2D surface
///
/// @param ctxt             Pointer to display context
/// @param user_ctxt        Pointer to structure containing window parameters
/// @param idx              Frame ID number
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
qcarcam_ret_t create_c2d_surface(qcarcam_frame_buffer_t *user_ctxt, unsigned int idx)
{
#ifndef C2D_DISABLED
    void *gpuaddr = NULL;
    int stride = qcarcam_get_stride((qcarcam_color_fmt_t)user_ctxt->format, user_ctxt->buffer_size[0]);
    int length = stride * user_ctxt->buffer_size[1];
    length = (length + 4096 - 1) & ~(4096 - 1);
    if (QCARCAM_FMT_UYVY_8 == user_ctxt->format)
    {

        C2D_STATUS c2d_status;
        C2D_YUV_SURFACE_DEF c2d_yuv_surface_def;
        c2d_yuv_surface_def.format = C2D_COLOR_FORMAT_422_UYVY;
        c2d_yuv_surface_def.width = user_ctxt->buffer_size[0];
        c2d_yuv_surface_def.height = user_ctxt->buffer_size[1];
        c2d_yuv_surface_def.stride0 = stride;
        c2d_yuv_surface_def.plane0 = user_ctxt->buffers[idx].ptr;
        c2d_status = c2dMapAddr((int)(uintptr_t)user_ctxt->pbuf[idx], user_ctxt->buffers[idx].ptr, length, 0, KGSL_USER_MEM_TYPE_ION, &gpuaddr);
        c2d_yuv_surface_def.phys0 = gpuaddr;
        C2D_SURFACE_TYPE surface_type = (C2D_SURFACE_TYPE)(C2D_SURFACE_YUV_HOST | C2D_SURFACE_WITH_PHYS);
        c2d_status = c2dCreateSurface(&user_ctxt->buffers[idx].c2d_surface_id,
                                      C2D_SOURCE | C2D_TARGET,
                                      surface_type,
                                      &c2d_yuv_surface_def);
        if (c2d_status != C2D_STATUS_OK)
        {
            QCARCAM_ERRORMSG("c2dCreateSurface %d buf %d failed %d", 0, idx, c2d_status);
            return QCARCAM_RET_FAILED;
        }
    }
    else
    {
        C2D_STATUS c2d_status;
        C2D_RGB_SURFACE_DEF c2d_rgb_surface_def;
        c2d_rgb_surface_def.format = C2D_COLOR_FORMAT_565_RGB;
        c2d_rgb_surface_def.width = user_ctxt->buffer_size[0];
        c2d_rgb_surface_def.height = user_ctxt->buffer_size[1];
        c2d_rgb_surface_def.stride = user_ctxt->stride;
        c2d_rgb_surface_def.buffer = user_ctxt->buffers[idx].ptr;
        c2d_status = c2dMapAddr((int)(uintptr_t)user_ctxt->pbuf[idx], user_ctxt->buffers[idx].ptr, length, 0, KGSL_USER_MEM_TYPE_ION, &gpuaddr);
        c2d_rgb_surface_def.phys = gpuaddr;
        C2D_SURFACE_TYPE surface_type = (C2D_SURFACE_TYPE)(C2D_SURFACE_RGB_HOST | C2D_SURFACE_WITH_PHYS);
        c2d_status = c2dCreateSurface(&user_ctxt->buffers[idx].c2d_surface_id,
                                      C2D_SOURCE | C2D_TARGET,
                                      surface_type,
                                      &c2d_rgb_surface_def);

        if (c2d_status != C2D_STATUS_OK)
        {
            QCARCAM_ERRORMSG("c2dCreateSurface %d buf %d failed %d", 0, idx, c2d_status);
            return QCARCAM_RET_FAILED;
        }
    }
#endif
    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// get_c2d_surface_id
///
/// @brief Get the ID from a C2D surface
///
/// @param ctxt             Pointer to display context
/// @param user_ctxt        Pointer to structure containing window parameters
/// @param idx              Frame ID number
/// @param surface_id       Pointer to C2D sruface ID number
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
qcarcam_ret_t get_c2d_surface_id(qcarcam_frame_buffer_t *user_ctxt, unsigned int idx, unsigned int *surface_id)
{
    if (!surface_id)
        return QCARCAM_RET_BADPARAM;

    *surface_id = user_ctxt->buffers[idx].c2d_surface_id;

    return QCARCAM_RET_OK;
}

qcarcam_ret_t qcarcam_init_v4l2device(qcarcam_v4l2_param_t *v4l2_params)
{
    struct v4l2_format v;
    int t;
    int vidsendsiz;
    unsigned int height_complement;
    struct v4l2_crop crop;

    if (!v4l2_params)
    {
        QCARCAM_ERRORMSG("Invalid argument");
        return QCARCAM_RET_FAILED;
    }

    QCARCAM_DBGMSG("v4l2_params->v4l2_node - %s ", v4l2_params->v4l2_node);
    QCARCAM_DBGMSG("v4l2_params->pixformat - %d V4L2_PIX_FMT_UYVY = %d ", v4l2_params->pixformat, V4L2_PIX_FMT_UYVY);
    QCARCAM_DBGMSG("v4l2_params->width - %u ", v4l2_params->width);
    QCARCAM_DBGMSG("v4l2_params->height - %u\n", v4l2_params->height);

    v4l2_params->v4l2sink = open(v4l2_params->v4l2_node, O_WRONLY);
    if (v4l2_params->v4l2sink < 0) {
        QCARCAM_ERRORMSG("Failed to open v4l2sink device. (%s)\n", strerror(errno));
        return QCARCAM_RET_NOMEM;
    }
    // setup video for proper format
    v.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    t = ioctl(v4l2_params->v4l2sink, VIDIOC_G_FMT, &v);
    if( t < 0 ) {
        QCARCAM_ERRORMSG("Error in VIDIOC_G_FMT");
        return QCARCAM_RET_NOMEM;
    }

    height_complement = v4l2_complement_height_to_size(v4l2_params->pixformat, v4l2_params->aligned_width);
    v.fmt.pix.width  = v4l2_params->aligned_width;
    v.fmt.pix.height = v4l2_params->aligned_height;
    v.fmt.pix.pixelformat = v4l2_params->pixformat;
    v.fmt.pix.sizeimage = v.fmt.pix.height * height_complement;
    t = ioctl(v4l2_params->v4l2sink, VIDIOC_S_FMT, &v);
    if( t < 0 ) {
        QCARCAM_ERRORMSG("VIDIOC_S_FMT failed for format %d", v.fmt.pix.pixelformat);
        return QCARCAM_RET_NOMEM;
    }

    crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    crop.c = (struct v4l2_rect)
        {
        (int) v4l2_params->crop_left,
        (int) v4l2_params->crop_top,
        v4l2_params->width - v4l2_params->crop_left - v4l2_params->crop_right,
        v4l2_params->height - v4l2_params->crop_top - v4l2_params->crop_bottom
        };
    t = ioctl(v4l2_params->v4l2sink, VIDIOC_S_CROP, &crop);
    if( t < 0 ) {
        QCARCAM_ERRORMSG("VIDIOC_S_CROP failed for crop type %d", crop.type);
        return QCARCAM_RET_NOMEM;
    }

    return QCARCAM_RET_OK;
}

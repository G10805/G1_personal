/* ===========================================================================
 * Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
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

#include <sys/mman.h>
#include <string>
#include <media/ais_v4l2loopback.h>
#include <media/cam_defs.h>
#include "ais_v4l2_proxy_util.h"
#include "ais_dmabuffer.h"

using std::string;

/*
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
        case V4L2_PIX_FMT_BGR24:
            q_fmt = QCARCAM_FMT_RGB_888;
            break;
        default:
            break;
    }

    return q_fmt;
}
*/


unsigned int get_v4l2_format_from_qcarcam(QCarCamColorFmt_e fmt)
{
    unsigned int v_fmt = V4L2_PIX_FMT_UYVY;
    switch(fmt)
    {
        case QCARCAM_FMT_UYVY_8:
        case QCARCAM_FMT_PLAIN16_12:
            v_fmt = V4L2_PIX_FMT_UYVY;
            break;
        case QCARCAM_FMT_NV12:
        case QCARCAM_FMT_MIPIRAW_12:
            v_fmt = V4L2_PIX_FMT_NV12;
            VPROXY_INFO("-> V4L2_PIX_FMT_NV12");
            break;
        case QCARCAM_FMT_RGB_888:
            v_fmt = V4L2_PIX_FMT_BGR24; // qcarcam RGB888 is relative to V4L2 BGR888
            VPROXY_INFO("-> V4L2_PIX_FMT_BGR24");
            break;
        default:
            VPROXY_INFO("unsupported");
            break;
    }

    return v_fmt;
}

///////////////////////////////////////////////////////////////////////////////
/// post_gfx_buffer_to_v4l2node
///
/// @brief Send frame to v4l2node
///
/// @param v4l2sink         v4l2node where data will be dumped
/// @param user_ctxt        Pointer to structure containing window parameters
/// @param idx              Frame ID number
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
int post_gfx_buffer_to_v4l2node(unsigned int v4l2sink, const QCarCamFrameInfo_t& frame_info, uint32 batch_frames)
{
    // todo: qbuf to kernel
    qbuf(v4l2sink, frame_info, batch_frames);

    return 0;
}

int qcarcam_init_v4l2device(qcarcam_v4l2_param_t *v4l2_params)
{
    if (!v4l2_params)
    {
        VPROXY_ERROR("Invalid argument");
        return 1;
    }

    VPROXY_INFO("v4l2_params->v4l2_node - %s ", v4l2_params->v4l2_node);

    v4l2_params->v4l2sink = open(v4l2_params->v4l2_node, O_WRONLY|O_NONBLOCK);
    if (v4l2_params->v4l2sink < 0) {
        VPROXY_ERROR("Failed to open v4l2sink device. (%s)\n", strerror(errno));
        return 1;
    }

    //set_v4l2_format(v4l2_params, hndl);

    return 0;
}

int set_v4l2_format(qcarcam_v4l2_param_t *v4l2_params, frame_buffer_handle_t hndl)
{
    struct v4l2_format v;
    struct v4l2_crop crop;
    int t;
    int vidsendsiz;
    unsigned int stride;
    buf_info_t info;

    if (!v4l2_params)
    {
        VPROXY_ERROR("Invalid argument");
        return 1;
    }

    // setup video for proper format
    v.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    t = ioctl(v4l2_params->v4l2sink, VIDIOC_G_FMT, &v);
    if( t < 0 )
    {
      VPROXY_ERROR("Error in VIDIOC_G_FMT");
      return 1;
    }

    t = v4l2_get_buf_info(hndl, info);
    if (t)
    {
      VPROXY_ERROR("fail to get buf info");
      return 1;
    }

    v.fmt.pix.width = info.w;
    v.fmt.pix.height = info.h;
    v.fmt.pix.pixelformat = v4l2_params->v4l2_format;
    v.fmt.pix.bytesperline = info.bytesperline;
    v.fmt.pix.sizeimage = info.size;

    v4l2_params->size = info.size;

    VPROXY_INFO("w %u h %u bpl %u sizeimage - %d\n",
        v.fmt.pix.width, v.fmt.pix.height, v.fmt.pix.bytesperline,
        v.fmt.pix.sizeimage);

    t = ioctl(v4l2_params->v4l2sink, VIDIOC_S_FMT, &v);
    if( t < 0 )
    {
      VPROXY_ERROR("VIDIOC_S_FMT failed for format %x", v.fmt.pix.pixelformat);
      return 1;
    }

    return 0;
}


int send_ioctrl_cmd(int fd, uint8 op_code, uint32 param_type, unsigned int size, uint8* payload)
{
    int rc = 0;
    struct ais_v4l2_control_t ctrl;
    ctrl.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ctrl.payload = (uint64)payload;
    ctrl.cmd = op_code;
    ctrl.param_type = param_type;
    ctrl.size = size;
    rc = ioctl(fd,
        VIDIOC_CAM_CONTROL,
        &ctrl);
    if (rc < 0)
    {
        VPROXY_ERROR("send_ioctrl_cmd rc %d %u failed", rc, op_code, param_type);
        return 1;
    }

    return 0;
}

int send_ioctrl_ret(int fd, uint8 op_code, uint32 ret)
{
    int rc = 0;
    struct ais_v4l2_control_t ctrl;
    ctrl.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ctrl.payload = (uint64)NULL;
    ctrl.cmd = op_code;
    ctrl.ctrl_ret = ret;
    ctrl.size = 0;
    rc = ioctl(fd,
        VIDIOC_CAM_CONTROL,
        &ctrl);
    if (rc < 0)
    {
        VPROXY_ERROR("send_ioctrl_ret rc %d %u failed", rc, op_code, ret);
        return 1;
    }

    return 0;
}

int send_ioctrl_bufs(int fd, uint8 op_code, uint32 ret, unsigned int size, uint8* payload)
{
    int rc = 0;
    struct ais_v4l2_control_t ctrl;
    ctrl.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ctrl.payload = (uint64)payload;
    ctrl.cmd = op_code;
    ctrl.param_type = ret;
    ctrl.size = size;
    rc = ioctl(fd,
        VIDIOC_CAM_CONTROL,
        &ctrl);
    if (rc < 0)
    {
        VPROXY_ERROR("send_ioctrl_cmd rc %d %u failed", rc, op_code, ret);
        return -1;
    }

    return 0;
}


int qbuf(int fd, const QCarCamFrameInfo_t& frame_info, uint32 batch_frames)
{
    v4l2_buffer device_buffer;
    uint64* p_reserved = (uint64*)&device_buffer.reserved2;
    struct ais_v4l2_buffer_ext_t ext;
    memset(&device_buffer, 0, sizeof(device_buffer));
    device_buffer.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    device_buffer.memory = V4L2_MEMORY_MMAP;
    device_buffer.index = frame_info.bufferIndex;
    device_buffer.sequence = frame_info.seqNo;
    device_buffer.timestamp.tv_sec = frame_info.timestamp/1000000000;
    device_buffer.timestamp.tv_usec = (unsigned long long)(frame_info.timestamp * 0.001)%1000000;

    ext.batch_num = batch_frames;
    for (uint32 i = 0; i < ext.batch_num ; ++i)
    {
        ext.timestamp[i] = frame_info.sofTimestamp.timestamp;

    }
    if (ext.batch_num)
    {
        *p_reserved = (uint64)&ext;
    }
    else
    {
        *p_reserved = 0;
    }

    VPROXY_DBG1("QBUF idx %u seq %u timestamp %u %u, batch_frame %u", device_buffer.index,frame_info.seqNo,
        device_buffer.timestamp.tv_sec, device_buffer.timestamp.tv_usec, batch_frames);

    if(ioctl(fd, VIDIOC_QBUF, &device_buffer) < 0)
    {
        VPROXY_ERROR( "VIDIOC_QBUF returned Error err= %s\n", strerror(-errno));
        return 1;
    }

    return 0;
}

int dqbuf(int fd, int& idx)
{
    struct v4l2_buffer capture_buf;
    int rc;

    memset(&capture_buf, 0, sizeof(capture_buf));
    capture_buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    capture_buf.memory = V4L2_MEMORY_MMAP;

    rc = ioctl(fd, VIDIOC_DQBUF, &capture_buf);
    if (-EAGAIN == rc)
    {
        CAM_MSG(ERROR, "VIDIOC_DQBUF again Error err= %s", strerror(-errno));
        return 1;
    }
    else if (rc < 0)
    {
        CAM_MSG(ERROR, "VIDIOC_DQBUF returned Error err= %s rc %d", strerror(-errno), rc);
        return 1;
    }

    idx = capture_buf.index;

    return 0;

}

int subscribeEvent(int fd, uint32 type, uint32 id)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    struct v4l2_event_subscription sub = {};
    int rc = -1;

    sub.type = type;
    sub.id   = id;

    rc = ioctl(fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    if (0 != rc)
    {
        VPROXY_ERROR( "Subscribe event %d %d failed %d", type, id, rc);
        ret = QCARCAM_RET_FAILED;
    }

    return ret;
}



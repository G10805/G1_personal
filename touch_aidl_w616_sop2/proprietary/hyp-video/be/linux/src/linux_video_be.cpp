/*===========================================================================

*//** @file linux_video_be.c
This file implements linux video backend services that communicates with
frontend applications via habmm and video driver in backend.

Copyright (c) 2018, 2021-2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/


/*===========================================================================
                             Edit History

$Header:$

when(mm/dd/yy)     who         what, where, why
--------------   --------    -------------------------------------------------
08/30/23           nb          Fix compilation errors due to additon of new compiler flags
04/07/23           xg          Add initialization of device_name
11/02/22           sk          Use flag and change header path for kernel 5.15
06/24/21           xg          Add initialization of msg2
05/27/21           sh          Guard passthrough structure with Linux specific macro
04/01/21           sh          Bringup video on RedBend Hypervisor
10/22/18           sm          Fix buffer overflow
06/05/18           sm          Initial version of hypervisor linux video BE

=============================================================================*/
#include <unistd.h>
#include <dlfcn.h>
#include <linux/videodev2.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/eventfd.h>
#include "habmm.h"
#include "MMThread.h"
#include "MMCriticalSection.h"
#include "MMDebugMsg.h"
#include "MMTimer.h"
#include "hyp_queue_utility.h"
#include "hyp_video_be_translation.h"
#include "hyp_buffer_manager.h"
#include "hyp_vidc_inf.h"
#include "hyp_vidc_types.h"
#include "hyp_video.h"
#include "hyp_debug.h"
#include "hyp_video_be.h"
#ifndef _LINUX_
#include <bits/epoll_event.h>
#endif
#if defined(_ANDROID_) || defined(_LINUX_)
#ifdef SUPPORT_DMABUF
#include <vidc/media/msm_vidc_utils.h>
#else
#include <media/msm_vidc_utils.h>
#endif
#endif

#define BE_VDEC_DEVICE   "/dev/video32"
#define BE_VENC_DEVICE   "/dev/video33"
#define BE_VINPUT_DEVICE "/dev/video35"

#define MAX_EPOLL_EVENTS  100
#define MAX_TIMEOUT 0x7fffffff
#define PLANES_RESERVED_SIZE 44

int debug_level = 0;

/**===========================================================================
FUNCTION process_io_buffer_mapping

@brief  It handles the hypervisor share memory mapping

@param [in/out] buf
@param [in] pHab_if
@param [in] map_queue
@param [in] habmm_handle

@dependencies
  None

@return
  Returns hypv_status_type
===========================================================================*/
static hypv_status_type process_io_buffer_mapping
(
   struct v4l2_buffer_64b  *imp_buf,
   struct v4l2_buffer  *buf,
   habIf*              pHab_if,
   hypv_lookup_table_ex_t *map_queue,
   uint32              habmm_handle
)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    uint32 frame_import_id = 0;
    uint32 extradata_import_id = 0;
    uint32 extradata_size = 0;
    hypv_map_entry_ex_t* entry = NULL;

    buf->index = imp_buf->index;
    buf->type = imp_buf->type;
    buf->bytesused = imp_buf->bytesused;
    buf->flags = imp_buf->flags;
    buf->field = imp_buf->field;
    buf->timestamp.tv_sec = imp_buf->tv_sec;
    buf->timestamp.tv_usec = imp_buf->tv_usec;
    HABMM_MEMCPY(&buf->timecode, &imp_buf->timecode, sizeof(struct v4l2_timecode));
    buf->sequence = imp_buf->sequence;
    buf->memory = V4L2_MEMORY_DMABUF;
    for (uint64 i = 0; i < imp_buf->length; i++)
    {
        buf->m.planes[i].bytesused = imp_buf->m.planes[i].bytesused;
        buf->m.planes[i].length = imp_buf->m.planes[i].length;
        buf->m.planes[i].m.userptr = imp_buf->m.planes[i].m.userptr;
        buf->m.planes[i].data_offset = imp_buf->m.planes[i].data_offset;
        HABMM_MEMCPY(&buf->m.planes[i].reserved[0], &imp_buf->m.planes[i].reserved[0], PLANES_RESERVED_SIZE);
    }
    buf->length = imp_buf->length;
    buf->reserved2 = imp_buf->reserved2;
    buf->reserved = imp_buf->reserved;

    frame_import_id = imp_buf->m.planes[0].reserved[0];
    if (1 < buf->length)
    {
        extradata_import_id = imp_buf->m.planes[1].reserved[0];
        extradata_size = imp_buf->m.planes[1].length;
    }

    entry = hypv_map_add_from_remote_ext(pHab_if,map_queue,
                        habmm_handle, imp_buf->index,
                        frame_import_id, imp_buf->m.planes[0].length,
                        extradata_import_id, extradata_size, imp_buf->type);

    if (NULL != entry)
    {
        buf->m.planes[0].reserved[0] = buf->m.planes[0].m.fd = (unsigned long)entry->frame_addr;
        if (1 < buf->length)
        {
            buf->m.planes[1].reserved[0] = buf->m.planes[1].m.fd = (unsigned long)entry->extradata_addr;
        }
    }
    else
    {
        HYP_VIDEO_MSG_ERROR("Failed to create an entry for buffer index %lld", (long long)imp_buf->index);
    }

    return rc;
}

/**===========================================================================
FUNCTION vinput_handler

@brief  Platform specific hypervisor BE to handle video input events

@param [in] void pointer
@param [in] events
@param [in] void pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type vinput_handler(void *data, uint32 poll_events, void *cd)
{
    UNUSED(cd);
    habmm_msg_desc_t msg;
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    hypv_session_t* hypv_session = (hypv_session_t *)data;
    struct v4l2_event event;

    HABMM_MEMSET((void *)&event, 0, sizeof(event));
    if (poll_events & EPOLLERR)
    {
        HYP_VIDEO_MSG_ERROR("avinput ba port event error");
        rc = HYPV_STATUS_FAIL;
    }
    else if (poll_events & EPOLLPRI)
    {
        int sts = 0;

        sts = ioctl(hypv_session->handle_64b, VIDIOC_DQEVENT, &event);

        if (0 > sts)
        {
            HYP_VIDEO_MSG_ERROR("dq ba event error\n");
            rc = HYPV_STATUS_FAIL;
        }
    }
    if (TRUE == hypv_session->passthrough_mode)
    {
#ifdef LINUX_PASSTHROUGH
        HABMM_MEMCPY((void *)(&msg.data.event_data.v4l2_passthru.payload.event_data),
                     (void *)&event, sizeof(struct v4l2_event));
        msg.data.event_data.v4l2_passthru.event_type = event.type;
        msg.data.event_data.v4l2_passthru.status = rc;
#endif
    }
    else
    {
        msg.data.event_data.vidc.payload.event_data_1 = event.u.data[0];
        msg.data.event_data.vidc.event_type = event.type;
        msg.data.event_data.vidc.status = rc;
    }

    hvbe_callback_handler((uint8 *) &msg.data.event_data,
                          sizeof(hypvideo_event_data_type),
                          (void *)hypv_session);

    return rc;
}

/**===========================================================================
FUNCTION update_lookup_queue

@brief  This function removes stale entries & updates RO flag

@param [in] void pointer
@param [in] v4l2_buffer pointer
@param [in] v4l2_plane pointer

@dependencies
  None

@return
  Returns void
===========================================================================*/
#ifdef LINUX_PASSTHROUGH
static void update_lookup_queue(void *data, struct v4l2_buffer *v4l2_buf, struct v4l2_plane *plane)
{
    hypv_session_t* hypv_session = (hypv_session_t *)data;
    hypv_map_entry_ex_t* entry = NULL;

    entry = hypv_map_get_entry(&hypv_session->lookup_queue_ex, v4l2_buf->index, TRUE, v4l2_buf->type);
    if (NULL != entry)
    {
        hypv_map_free_ext(&hypv_session->habmm_if, &hypv_session->lookup_queue_ex, v4l2_buf->index, entry->frame_addr, entry->frame_bufferid);
        HYP_VIDEO_MSG_INFO("Clearing stale buffer index %d buf type %x", v4l2_buf->index, v4l2_buf->type);
    }
    else
    {
        HYP_VIDEO_MSG_INFO("No stale entry found for buffer index %d", v4l2_buf->index);
    }

    entry = hypv_map_get_entry(&hypv_session->lookup_queue_ex, v4l2_buf->index, FALSE, v4l2_buf->type);
    if (NULL != entry)
    {
        plane[0].m.fd = plane[0].reserved[0] = entry->frame_bufferid;
        if ((0 != plane[1].m.fd) && (1 < v4l2_buf->length))
        {
            plane[1].m.fd = plane[1].reserved[0] = entry->extradata_bufferid;
        }
        if (v4l2_buf->flags & V4L2_BUF_FLAG_READONLY)
        {
            hypv_map_update_readonly(entry, TRUE);
        }
    }
}
#endif
/**===========================================================================
FUNCTION copy_v4l2_buffer

@brief  This function copies v4l2 buffer content from 32 bit to 64 bit

@param [in] v4l2_buffer_64b pointer
@param [in] v4l2_buffer pointer

@dependencies
  None

@return
  Returns void
===========================================================================*/
#ifdef LINUX_PASSTHROUGH
static void copy_v4l2_buffer(struct v4l2_buffer_64b *v4l2_buf_exp, struct v4l2_buffer *v4l2_buf)
{
    v4l2_buf_exp->index = v4l2_buf->index;
    v4l2_buf_exp->type = v4l2_buf->type;
    v4l2_buf_exp->bytesused = v4l2_buf->bytesused;
    v4l2_buf_exp->flags = v4l2_buf->flags;
    v4l2_buf_exp->field = v4l2_buf->field;
    v4l2_buf_exp->tv_sec = v4l2_buf->timestamp.tv_sec;
    v4l2_buf_exp->tv_usec = v4l2_buf->timestamp.tv_usec;
    HABMM_MEMCPY(&v4l2_buf_exp->timecode, &v4l2_buf->timecode, sizeof(struct v4l2_timecode));
    v4l2_buf_exp->sequence = v4l2_buf->sequence;
    v4l2_buf_exp->memory = V4L2_MEMORY_USERPTR;
    for (uint32 i = 0; i < v4l2_buf->length; i++)
    {
        v4l2_buf_exp->m.planes[i].bytesused = v4l2_buf->m.planes[i].bytesused;
        v4l2_buf_exp->m.planes[i].length = v4l2_buf->m.planes[i].length;
        v4l2_buf_exp->m.planes[i].m.userptr = v4l2_buf->m.planes[i].m.userptr;
        v4l2_buf_exp->m.planes[i].data_offset = v4l2_buf->m.planes[i].data_offset;
        HABMM_MEMCPY(&v4l2_buf_exp->m.planes[i].reserved[0], &v4l2_buf->m.planes[i].reserved[0], PLANES_RESERVED_SIZE);
    }
    v4l2_buf_exp->length = v4l2_buf->length;
    v4l2_buf_exp->reserved2 = v4l2_buf->reserved2;
    v4l2_buf_exp->reserved = v4l2_buf->reserved;
}
#endif
/**===========================================================================
FUNCTION vdec_enc_handler

@brief  Platform specific hypervisor BE to handle video decode and encode events

@param [in] void pointer
@param [in] events
@param [in] void pointer

@dependencies
  None

@return
  Returns hypv_status_type
===========================================================================*/
static hypv_status_type vdec_enc_handler(void *data, uint32 poll_event, void *cd)
{
    UNUSED(cd);
    habmm_msg_desc_t msg;
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    hypv_session_t* hypv_session = (hypv_session_t *)data;
    struct v4l2_buffer v4l2_buf;
    struct v4l2_buffer_64b v4l2_buf_exp;
    struct v4l2_event event_msg;
    struct v4l2_plane plane[VIDEO_MAX_PLANES];

    HABMM_MEMSET(&v4l2_buf, 0, sizeof(v4l2_buf));
    HABMM_MEMSET(&v4l2_buf_exp, 0, sizeof(v4l2_buf_exp));
    HABMM_MEMSET(plane, 0, sizeof(plane));
    HABMM_MEMSET((void *)&event_msg, 0, sizeof(event_msg));
    HABMM_MEMSET(&msg, 0, sizeof(msg));

    if (poll_event & POLLERR)
    {
        HYP_VIDEO_MSG_ERROR("video decode event error");
        rc = HYPV_STATUS_FAIL;
    }
    if ((poll_event & POLLIN || poll_event & POLLRDNORM))
    {
        v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        v4l2_buf.memory = V4L2_MEMORY_DMABUF;
        v4l2_buf.length = hypv_session->num_output_planes;
        v4l2_buf.m.planes = plane;
        while (!ioctl(hypv_session->handle_64b, VIDIOC_DQBUF, &v4l2_buf))
        {
#ifdef LINUX_PASSTHROUGH
            msg.data.event_data.v4l2_passthru.event_type = VIDC_EVT_RESP_OUTPUT_DONE;
            msg.data.event_data.v4l2_passthru.status = HYPV_STATUS_SUCCESS;

            update_lookup_queue(data, &v4l2_buf, plane);
            copy_v4l2_buffer(&v4l2_buf_exp, &v4l2_buf);

            HABMM_MEMCPY((void *)(&msg.data.event_data.v4l2_passthru.payload.buffer),
                         (void *)&v4l2_buf_exp, sizeof(struct v4l2_buffer_64b));
#endif
            hvbe_callback_handler((uint8 *) &msg.data.event_data,
                                  sizeof(hypvideo_event_data_type),
                                  (void *)hypv_session);
        }
    }
    if (((poll_event & POLLOUT) || (poll_event & POLLWRNORM)))
    {
        v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        v4l2_buf.memory = V4L2_MEMORY_DMABUF;
        v4l2_buf.length = hypv_session->num_input_planes;
        v4l2_buf.m.planes = plane;
        while (!ioctl(hypv_session->handle_64b, VIDIOC_DQBUF, &v4l2_buf))
        {
#ifdef LINUX_PASSTHROUGH
            msg.data.event_data.v4l2_passthru.event_type = VIDC_EVT_RESP_INPUT_DONE;
            msg.data.event_data.v4l2_passthru.status = HYPV_STATUS_SUCCESS;

            update_lookup_queue(data, &v4l2_buf, plane);
            copy_v4l2_buffer(&v4l2_buf_exp, &v4l2_buf);

            HABMM_MEMCPY((void *)(&msg.data.event_data.v4l2_passthru.payload.buffer),
                         (void *)&v4l2_buf_exp, sizeof(struct v4l2_buffer_64b));
#endif
            hvbe_callback_handler((uint8 *) &msg.data.event_data,
                                  sizeof(hypvideo_event_data_type),
                                  (void *)hypv_session);
        }
    }
    if (poll_event & POLLPRI)
    {
        rc = (hypv_status_type)ioctl(hypv_session->handle_64b, VIDIOC_DQEVENT, &event_msg);
#ifdef LINUX_PASSTHROUGH
        msg.data.event_data.v4l2_passthru.event_type = event_msg.type;
        msg.data.event_data.v4l2_passthru.status = HYPV_STATUS_SUCCESS;
#endif
        switch (event_msg.type)
        {
            case V4L2_EVENT_MSM_VIDC_PORT_SETTINGS_CHANGED_INSUFFICIENT:
            {
                HYP_VIDEO_MSG_ERROR("Received V4L2_EVENT_MSM_VIDC_PORT_SETTINGS_CHANGED_INSUFFICIENT event");
                break;
            }
            case V4L2_EVENT_MSM_VIDC_PORT_SETTINGS_CHANGED_SUFFICIENT:
            {
                HYP_VIDEO_MSG_ERROR("Received V4L2_EVENT_MSM_VIDC_PORT_SETTINGS_CHANGED_SUFFICIENT event");
                break;
            }
            case V4L2_EVENT_MSM_VIDC_FLUSH_DONE:
            {
                HYP_VIDEO_MSG_INFO("Received V4L2_EVENT_MSM_VIDC_FLUSH_DONE event");
                break;
            }
            case V4L2_EVENT_MSM_VIDC_HW_OVERLOAD:
            {
                HYP_VIDEO_MSG_ERROR("Received V4L2_EVENT_MSM_VIDC_HW_OVERLOAD event");
                break;
            }
            case V4L2_EVENT_MSM_VIDC_HW_UNSUPPORTED:
            {
                HYP_VIDEO_MSG_ERROR("Received V4L2_EVENT_MSM_VIDC_HW_UNSUPPORTED event");
                break;
            }
            case V4L2_EVENT_MSM_VIDC_SYS_ERROR:
            {
                HYP_VIDEO_MSG_ERROR("Received V4L2_EVENT_MSM_VIDC_SYS_ERROR event");
                break;
            }
            case V4L2_EVENT_MSM_VIDC_RELEASE_BUFFER_REFERENCE:
            {
                uint8 *ptr = event_msg.u.data;
                HYP_VIDEO_MSG_INFO("Received V4L2_EVENT_MSM_VIDC_RELEASE_BUFFER_REFERENCE event id %d", *ptr);
                if (0 != ptr[0])
                {
                    hypv_map_entry_ex_t* entry = hypv_map_to_lookup_ext(&hypv_session->lookup_queue_ex,
                                                        (void *)(uintptr_t)ptr[0], TRUE);
                    if (NULL != entry)
                    {
                        ptr[0] = entry->frame_bufferid;
                        hypv_map_update_readonly(entry, FALSE);
                    }
                }
                break;
            }
            case V4L2_EVENT_MSM_VIDC_RELEASE_UNQUEUED_BUFFER:
            {
                HYP_VIDEO_MSG_INFO("Received V4L2_EVENT_MSM_VIDC_RELEASE_UNQUEUED_BUFFER event");
                break;
            }
            default:
            {
                HYP_VIDEO_MSG_ERROR( "Unknown Event 0x%x rc %d", event_msg.type, rc);
                break;
            }

        }
#ifdef LINUX_PASSTHROUGH
        HABMM_MEMCPY((void *)(&msg.data.event_data.v4l2_passthru.payload.event_data),
                         (void *)&event_msg, sizeof(struct v4l2_event));
#endif
        hvbe_callback_handler((uint8 *) &msg.data.event_data,
                              sizeof(hypvideo_event_data_type),
                              (void *)hypv_session);
    }

    return rc;
}

/**===========================================================================
FUNCTION video_events_thread

@brief  Platform specific hypervisor BE thread to listen to video events

@param [in] msg pointer

@dependencies
  None

@return
  Returns void
===========================================================================*/
static int video_events_thread(void* session)
{
    hypv_session_t* hypv_session = (hypv_session_t *)session;
    int32 rc = 0;
    hypv_session->be_thread_stop = FALSE;
    struct pollfd pfds[2];
    pfds[0].events = POLLIN | POLLRDNORM | POLLOUT | POLLWRNORM | POLLRDBAND | POLLPRI;
    pfds[0].fd = hypv_session->handle_64b;
    pfds[1].events = POLLIN | POLLERR;
    pfds[1].fd = hypv_session->epollfd;

    HYP_VIDEO_MSG_HIGH("video_events_thread");
    while (TRUE != hypv_session->be_thread_stop)
    {
        rc = poll(pfds, 2, MAX_TIMEOUT);
        if (!rc)
        {
            HYP_VIDEO_MSG_ERROR("Poll timedout");
            break;
        }
        else if (rc < 0 && errno != EINTR && errno != EAGAIN)
        {
            HYP_VIDEO_MSG_ERROR("Error while polling: %d, errno %d", rc, errno);
        }

        if ((pfds[1].revents & POLLIN) || (pfds[1].revents & POLLERR))
        {
            HYP_VIDEO_MSG_ERROR("Poll thread interrupted to be exited");
            break;
        }

        if (hypv_session->hvbe_session_cb.handler)
        {
            hypv_session->hvbe_session_cb.handler((uint8 *)hypv_session, pfds[0].revents, NULL);
        }
    }

    HYP_VIDEO_MSG_HIGH("video_events_thread exiting rc = %d", rc);

    return 0;
}

/**===========================================================================
FUNCTION video_passthrough

@brief  Platform specific hypervisor BE to support passthrough mode

@param [in] hypv_session pointer
@param [in] hypvideo_ioctl_data_type pointer

@dependencies
  None

@return
  Returns hypv_status_type
===========================================================================*/
static hypv_status_type video_passthrough(hypv_session_t* hypv_session, hypvideo_ioctl_data_type *ioctl_data)
{
    int ret = 0;

    switch (ioctl_data->vidc_ioctl)
    {
    case VIDIOC_S_FMT:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (struct v4l2_format *) ioctl_data->payload);
            break;
        }
    case VIDIOC_G_FMT:
        {
            struct v4l2_format *format = (struct v4l2_format *) ioctl_data->payload;
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (struct v4l2_format *) format);
            if ((0 == ret) &&
                (VIDEO_SESSION_DECODE == hypv_session->video_session ||
                 VIDEO_SESSION_ENCODE == hypv_session->video_session))
            {
                if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == format->type)
                {
                    hypv_session->num_output_planes = format->fmt.pix_mp.num_planes;
                }
                else
                {
                    hypv_session->num_input_planes = format->fmt.pix_mp.num_planes;
                }
            }
            break;
        }
    case VIDIOC_S_EXT_CTRLS:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (struct v4l2_ext_controls *) ioctl_data->payload);
            break;
        }
    case VIDIOC_REQBUFS:
        {
            struct v4l2_requestbuffers buf_req;
            struct v4l2_requestbuffers* temp_buf_req = (struct v4l2_requestbuffers *) ioctl_data->payload;
            HABMM_MEMSET(&buf_req, 0, sizeof(struct v4l2_requestbuffers));
            HABMM_MEMCPY(&buf_req, ioctl_data->payload, sizeof(struct v4l2_requestbuffers));

            buf_req.memory = V4L2_MEMORY_DMABUF;
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl, &buf_req);
            HABMM_MEMCPY(temp_buf_req, &buf_req, sizeof(struct v4l2_requestbuffers));
            temp_buf_req->memory = V4L2_MEMORY_USERPTR;
            if (0 == temp_buf_req->count)
            {
                HYP_VIDEO_MSG_LOW("Removing %s buffer map entries", (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == temp_buf_req->type)?"input":"output");
                hypv_map_cleanup_buf_type_ext(&hypv_session->habmm_if,
                             &hypv_session->lookup_queue_ex,
                             temp_buf_req->type);
                if (hypv_session->num_output_planes > 1)
                {
                    HYP_VIDEO_MSG_LOW("Removing %s extradata buffer map entries", (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == temp_buf_req->type)?"input":"output");
                    hypv_map_cleanup_buf_type_ext(&hypv_session->habmm_if,
                                 &hypv_session->lookup_queue_ex,
                                 temp_buf_req->type);
                }
                if (hypv_session->num_input_planes > 1)
                {
                    HYP_VIDEO_MSG_LOW("Removing %s extradata buffer map entries", (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == temp_buf_req->type)?"input":"output");
                    hypv_map_cleanup_buf_type_ext(&hypv_session->habmm_if,
                                 &hypv_session->lookup_queue_ex,
                                 temp_buf_req->type);
                }
            }
            break;
        }
    case VIDIOC_S_CTRL:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (struct v4l2_control *) ioctl_data->payload);
            break;
        }
    case VIDIOC_G_CTRL:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (struct v4l2_control *) ioctl_data->payload);
            break;
        }
    case VIDIOC_PREPARE_BUF:
        {
            struct v4l2_buffer_64b *imp_buf = (struct v4l2_buffer_64b *) ioctl_data->payload;
            struct v4l2_buffer buf;
            struct v4l2_plane plane[2];

            HABMM_MEMSET(&buf, 0, sizeof(struct v4l2_buffer));
            HABMM_MEMSET(&plane[0], 0, 2 * sizeof(struct v4l2_plane));
            buf.m.planes = plane;

            if (HYPV_STATUS_SUCCESS == process_io_buffer_mapping(imp_buf, &buf, &hypv_session->habmm_if,
                                      &hypv_session->lookup_queue_ex, hypv_session->habmm_handle))
            {
                ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl, &buf);
            }
            else
            {
                ret = -1;
            }
            break;
        }
    case VIDIOC_STREAMON:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (enum v4l2_buf_type *) (ioctl_data->payload));
            break;
        }
    case VIDIOC_QBUF:
        {
            struct v4l2_buffer_64b *imp_buf = (struct v4l2_buffer_64b *) ioctl_data->payload;
            struct v4l2_buffer buf;
            struct v4l2_plane plane[2];

            HABMM_MEMSET(&buf, 0, sizeof(struct v4l2_buffer));
            HABMM_MEMSET(&plane[0], 0, 2 * sizeof(struct v4l2_plane));
            buf.m.planes = plane;

            if (HYPV_STATUS_SUCCESS == process_io_buffer_mapping(imp_buf, &buf, &hypv_session->habmm_if,
                                       &hypv_session->lookup_queue_ex, hypv_session->habmm_handle))
            {
                ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl, &buf);
            }
            else
            {
                ret = -1;
            }

            break;
        }
    case VIDIOC_SUBSCRIBE_EVENT:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (struct v4l2_event_subscription *) ioctl_data->payload);
            break;
        }
    case VIDIOC_UNSUBSCRIBE_EVENT:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (struct v4l2_control *) ioctl_data->payload);
            break;
        }
    case VIDIOC_QUERYCAP:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (struct v4l2_capability *) ioctl_data->payload);
            break;
        }
    case VIDIOC_QUERYCTRL:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (struct v4l2_queryctrl *) ioctl_data->payload);
            break;
        }
    case VIDIOC_QUERYMENU:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (struct v4l2_querymenu *) ioctl_data->payload);
            break;
        }
    case VIDIOC_ENUM_FMT:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (struct v4l2_fmtdesc *) ioctl_data->payload);
            break;
        }
    case VIDIOC_DECODER_CMD:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (struct v4l2_decoder_cmd *) ioctl_data->payload);
            break;
        }
    case VIDIOC_ENCODER_CMD:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (struct v4l2_encoder_cmd *) ioctl_data->payload);
            break;
        }
    case VIDIOC_STREAMOFF:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (enum v4l2_buf_type *) (ioctl_data->payload));
            enum v4l2_buf_type *buf_type = (enum v4l2_buf_type*)ioctl_data->payload;
            if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == *buf_type)
            {
                HYP_VIDEO_MSG_LOW("VIDIOC_STREAMOFF:Removing output buffer map entries");
                hypv_map_cleanup_buf_type_ext(&hypv_session->habmm_if,
                             &hypv_session->lookup_queue_ex,
                             V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
                if (hypv_session->num_output_planes > 1)
                {
                    HYP_VIDEO_MSG_LOW("VIDIOC_STREAMOFF:Removing output extradata buffer map entries");
                    hypv_map_cleanup_buf_type_ext(&hypv_session->habmm_if,
                                 &hypv_session->lookup_queue_ex,
                                 V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
                }
            }
            break;
        }
    case VIDIOC_S_PARM:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (struct v4l2_streamparm *) ioctl_data->payload);
            break;
        }
    case VIDIOC_ENUM_FRAMESIZES:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (struct v4l2_frmsizeenum *) ioctl_data->payload);
            break;
        }
    case VIDIOC_S_INPUT:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (int *) ioctl_data->payload);
            break;
        }
    case VIDIOC_G_INPUT:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (struct v4l2_control *) ioctl_data->payload);
            break;
        }
    case VIDIOC_ENUMINPUT:
        {
            ret = ioctl(ioctl_data->io_handle, ioctl_data->vidc_ioctl,
                       (struct v4l2_input *) ioctl_data->payload);
            break;
        }
    default:
        {
            HYP_VIDEO_MSG_ERROR("unsupported cmd 0x%x", ioctl_data->vidc_ioctl);
            ret = -1;
            break;
        }
    }

    return (ret == 0) ? HYPV_STATUS_SUCCESS:HYPV_STATUS_FAIL;
}

/**===========================================================================
FUNCTION plt_hvbe_open

@brief  Platform specific hypervisor BE open

@param [in] habmm_msg_desc_t pointer

@dependencies
  None

@return
  Returns void
===========================================================================*/
void plt_hvbe_open(hypv_session_t* hypv_session, habmm_msg_desc_t* pMsgBufferNode)
{
    int handle;
    int32 rc = 0;
    habmm_msg_desc_t msg2;
    char device_name[MAX_DEVICE_NAME_LEN];

    HABMM_MEMSET(device_name, 0, sizeof(device_name));
    HABMM_MEMSET(&msg2, 0, sizeof(msg2));
    hypv_session->video_session = pMsgBufferNode->data.open_data.video_session;

    if (VIDEO_SESSION_DECODE == hypv_session->video_session)
    {
        hypv_session->hvbe_session_cb.handler = (callback_handler_t)vdec_enc_handler;
        snprintf(device_name, MAX_DEVICE_NAME_LEN, "%s", BE_VDEC_DEVICE);
    }
    else if (VIDEO_SESSION_ENCODE == hypv_session->video_session)
    {
        hypv_session->hvbe_session_cb.handler = (callback_handler_t)vdec_enc_handler;
        snprintf(device_name, MAX_DEVICE_NAME_LEN, "%s", BE_VENC_DEVICE);
    }
    else if (VIDEO_SESSION_VINPUT == hypv_session->video_session)
    {
        hypv_session->hvbe_session_cb.handler = (callback_handler_t)vinput_handler;
        snprintf(device_name, MAX_DEVICE_NAME_LEN, "%s", BE_VINPUT_DEVICE);
    }

    handle = open( device_name, O_RDWR );
    hypv_session->handle_64b = (int64)(handle);
    msg2.msg_id = HYPVIDEO_MSGRESP_OPEN_RET;

    /**return_io_handle should be device handle */
    if (NULL == hypv_session->hvbe_callback_thread)
    {
        msg2.data.open_data.return_io_handle = (int64)(handle);
        msg2.data.open_data.return_status = HYPV_STATUS_SUCCESS;
        HYP_VIDEO_MSG_HIGH("Opened device handle %lld", msg2.data.open_data.return_io_handle);
        msg2.data.open_data.passthrough_mode = TRUE;
        hypv_session->passthrough_mode = TRUE;
        hypv_session->hyp_platform = pMsgBufferNode->data.open_data.hyp_plt_id;
        msg2.data.open_data.hyp_plt_id = pMsgBufferNode->data.open_data.hyp_plt_id;
        hypv_session->epollfd = eventfd(0, 0);
        rc = MM_Thread_CreateEx(MM_Thread_DefaultPriority, 0, video_events_thread,
                               (void *)hypv_session, HYPV_THREAD_STACK_SIZE,
                               "hvbe_kernel_event_cb_mgr", &hypv_session->hvbe_callback_thread);
        if (0 != rc)
        {
             HYP_VIDEO_MSG_ERROR("failed to create hvbe kernel event callback thread");
        }
    }
    else
    {
        rc = -1;
    }

    if (0 != rc)
    {
        if (-1 < handle)
        {
            close(handle);
        }
        msg2.data.open_data.return_status = HYPV_STATUS_FAIL;
    }

    msg2.data_size = pMsgBufferNode->data_size;
    msg2.pid = (uint32) getpid();
    HYP_VIDEO_MSG_HIGH("passthrough %d, platform %d", msg2.data.open_data.passthrough_mode, msg2.data.open_data.hyp_plt_id);
    MM_CriticalSection_Enter(hypv_session->sendCritSection);
    rc = hypv_session->habmm_if.pfSend(hypv_session->habmm_handle, &msg2, sizeof(habmm_msg_desc_t), 0);
    MM_CriticalSection_Leave(hypv_session->sendCritSection);
    if (0 != rc)
    {
        HYP_VIDEO_MSG_ERROR("habmm socket send fail rc=%d", rc);
    }
}

/**===========================================================================
FUNCTION plt_hvbe_ioctl

@brief  Platform specific hypervisor BE ioctl

@param [in] habmm_msg_desc_t pointer

@dependencies
  None

@return
  Returns void
===========================================================================*/
void plt_hvbe_ioctl(hypv_session_t* hypv_session, habmm_msg_desc_t* pMsgBufferNode)
{

    int32 rc = 0;
    hypv_status_type hypv_status = HYPV_STATUS_SUCCESS;
    habmm_msg_desc_t msg2;

    msg2.pid = (uint32) getpid();

    /* initialize return ioctl payload */
    HABMM_MEMCPY(msg2.data.ioctl_data.payload, pMsgBufferNode->data.ioctl_data.payload,
                 GET_IOCTL_PAYLOAD_SIZE_FROM_MSG_DATASIZE(pMsgBufferNode->data_size));

    if (TRUE == hypv_session->passthrough_mode)
    {
        hypv_status = video_passthrough(hypv_session, &pMsgBufferNode->data.ioctl_data);
    }
    else
    {
        /* only passthrogh supported */
        HYP_VIDEO_MSG_ERROR("Only passthrough mode supported");
        hypv_status = HYPV_STATUS_FAIL;
    }
    HABMM_MEMCPY(msg2.data.ioctl_data.payload, pMsgBufferNode->data.ioctl_data.payload,
                 GET_IOCTL_PAYLOAD_SIZE_FROM_MSG_DATASIZE(pMsgBufferNode->data_size));

    msg2.data.ioctl_data.return_value = hypv_status;
    msg2.data.ioctl_data.vidc_ioctl = pMsgBufferNode->data.ioctl_data.vidc_ioctl;
    msg2.msg_id = HYPVIDEO_MSGRESP_IOCTL_RET;
    msg2.data_size = pMsgBufferNode->data_size;

    MM_CriticalSection_Enter(hypv_session->sendCritSection);
    /** Wake up IO message handling thread */
    rc = hypv_session->habmm_if.pfSend(hypv_session->habmm_handle, &msg2, sizeof(habmm_msg_desc_t), 0);
    MM_CriticalSection_Leave(hypv_session->sendCritSection);
    if (0 != rc)
    {
        HYP_VIDEO_MSG_ERROR("habmm socket send failed rc=%d", rc);
    }
}

/**===========================================================================
FUNCTION plt_hvbe_close

@brief  Platform specific hypervisor BE close

@param [in] habmm_msg_desc_t pointer

@dependencies
  None

@return
  Returns void
===========================================================================*/
void plt_hvbe_close(hypv_session_t* hypv_session, habmm_msg_desc_t* pMsgBufferNode)
{

    int32 rc = 0;
    int32 status = 0;
    habmm_msg_desc_t msg2;

    msg2.msg_id = HYPVIDEO_MSGRESP_CLOSE_RET;
    msg2.data.close_data.return_status = HYPV_STATUS_SUCCESS;
    msg2.data_size = pMsgBufferNode->data_size;

    hypv_map_cleanup(&hypv_session->habmm_if, &hypv_session->lookup_queue);
    hypv_map_cleanup_ext(&hypv_session->habmm_if, &hypv_session->lookup_queue_ex);
    status = close((int)pMsgBufferNode->data.close_data.io_handle );
    if (0 != status)
    {
        HYP_VIDEO_MSG_ERROR("failed to close device fd %d", (int)pMsgBufferNode->data.close_data.io_handle);
        msg2.data.close_data.return_status = (hypv_status_type)status;
    }
    else
    {
       HYP_VIDEO_MSG_HIGH("device close done");
    }

    eventfd_write(hypv_session->epollfd, 1);
    hypv_session->be_thread_stop = TRUE;
    close(hypv_session->epollfd);
    MM_Thread_Join((MM_HANDLE)hypv_session->hvbe_callback_thread, &rc);

    MM_CriticalSection_Enter(hypv_session->sendCritSection);
    rc = hypv_session->habmm_if.pfSend(hypv_session->habmm_handle, &msg2, sizeof(habmm_msg_desc_t), 0);
    MM_CriticalSection_Leave(hypv_session->sendCritSection);
    if (0 != rc)
    {
        HYP_VIDEO_MSG_ERROR("habmm socket send failed rc=%d", rc);
    }
    HYP_VIDEO_MSG_INFO("Close ACK sent to FE");
}


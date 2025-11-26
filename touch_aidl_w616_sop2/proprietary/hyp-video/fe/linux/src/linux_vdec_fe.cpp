/*========================================================================

*//** @file linux_vdec_fe.cpp

@par DESCRIPTION:
Linux video decoder hypervisor front-end implementation

@par FILE SERVICES:

@par EXTERNALIZED FUNCTIONS:
See below.

Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/
/*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---     -------------------------------------------------------
03/04/25    su     Fix state machine of FE in reconfigure
02/19/25    su     Report ENOMEM error code when setting alloc_mode is failed
07/15/24    su     Update reconfig event for multi-stream usecases
06/11/24    cg     Resolve KW for uninitialized buffer_type
02/25/24    pc     Add support for tier for HEVC
03/22/24    fz     Fix null dereference on fe_ioss->out_dynamic_info after stream off
10/26/23    mm     Support frame rate setting for decoder
08/30/23    nb     Fix compilation errors due to additon of new compiler flags
06/22/23    mm     Add flush lock to avoid race condition issue
11/15/22    sj     Pass min output buffer count to C2 in insufficient event payload
11/07/22    su     Add support to SYNCFRAMEDECODE in thumbnail mode
11/02/22    sk     Use flag and change header path for kernel 5.15
10/18/22    mm     Report output crop info during reconfig and refine magic number
09/26/22    mm     Remove dependency OMX
09/03/22    mz     Free internally allocated meta buffer for lemans backward compatibility
07/12/22    ls     Handle VP9 linear output decoding
04/12/22    ls     Remove resolution restriction on buffer count update
02/14/22    sj     Synchronize lock handling
01/28/22    sj     Handle unsupported ctrl & reduce loglevel
01/14/22    nb     Add secure property setting after set format
09/15/21    sh     Set actual count to MAX when C2 HAL is used
06/17/21    sj     Do not align width/height in reconfig
06/16/21    sh     Include input_tag2 as its needed for dropping inputs in codec2
06/15/21    mm     Override actual count of VIDC_BUFFER_OUTPUT for some cases
06/09/21    sh     Generate HW Unsupported event for unsupported resolution
06/08/21    sh     Update frame buffer & meta buffer in reserved fields
05/25/21    sj     Fix compilation errors on linux due to C99 standard
05/08/21    sj     Update output buffer flags to handle multiple outputs for single input
05/06/21    sj     Add support for querying profile/level
04/01/21    sh     Bringup video on RedBend Hypervisor
03/28/21    sj     Added a print if dynamic buffer mode setting failed
03/26/21    rq     Remove redundant signal set for VIDC_EVT_RESP_RESUME callback
03/18/21    rq     Call back only once for the flush command with FLASH_ALL type
02/01/21    rq     Delay setting secure mode until right before streamon
10/19/20    sh     Enable multistream for linear color formats
10/04/20    yc     Add HEVC HDR10/HDR10plus playback support
09/23/20    mm     Handle nonsupport of FRAME_RATE setting
09/22/20    mm     Fix compiling issue on LV
09/15/20    sh     Restrict max buffer count as sent from the firmware
08/27/20    sh     Bringup video decode using codec2
08/12/20    sh     Update Linux FE with kernel 5.4 macros
07/28/20    sh     Enable sequence display extradata for MPEG-2
07/28/20    sh     Update input buffer size in REQBUFS IOCTL
07/23/20    sh     Add STOP & RELEASE_RESOURCE IOCTLs to maintain proper driver & FW states
01/28/20    sh     Add query control to enumerate supported profiles
11/26/19    sh     Fix seek failure for multi resolution clips
11/04/19    sm     Enable secure video usecases
10/16/19    sm     Fix EOS buffer leakage
08/23/19    sm     Update P010 color format type
07/09/19    sm     Add P010 color format
06/27/19    sm     Differentiate input and output metadata buffer length
06/17/19    sm     Fix linear playbacks involved when multi-stream is enabled
06/03/19    sm     Handle decode frames with frame drop flag
05/30/19    sm     Add support for hab export using ion fd
05/13/19    sh     Avoid calling FTB when flush in progress
04/24/19    sm     Multi stream buffer handling during reconfig
04/11/19    sm     Align hypervisor event handling with native drivers
04/09/19    sm     Handle encoder buffer reference in dynamic mode
03/18/19    sm     Synchronize FW reference frames between FE and BE
03/18/19    rz     Return max of the min/actual count at g_ctrl buffer count
03/15/19    rz     Fix release_reference buffer check
03/14/19    sm     Generalize enc and dec command handling
03/06/19    rz     Handle decoder output dynamic map/unmap/export/unexport buffers
03/04/19    sm     Add functions to set/get buf alloc mode
03/04/19    sm     Make sure logging is properly enabled
02/15/19    rz     Use internal buffer to handle EOS
02/14/19    hl     Support full range V4L2_MPEG_VIDC_EXTRADATA_VUI_DISPLAY
02/05/19    rz     Add decoder dynamic input buffer mode
11/06/18    hl     Support interlace playback fall back to NV12 linear
11/01/18    sm     Make use of a common macro to retrive output size
08/21/18    aw     Fix Klockwork P1, compilation and MISRA warning
06/29/18    sm     Initialize session codec type in context structure
06/12/18    sm     Use a common macro for memset
05/08/18    sm     Add support for 10 bit playback
04/11/18    sm     Fix timestamp calculation
04/04/18    sm     Add support output crop metadata
03/23/18    sm     Update the actual resolution instead of crop resolution
03/14/18    sm     Propagate EOS buffer with decoder stop command
03/07/18    sm     Update width and height in the correct field
01/18/18    sm     Add support for passthrough mode feature
12/12/17    sm     Add support for drop frames flags from venus
11/14/17    sm     Add support for UBWC
10/12/17    sm     Simplify buffer management logics for dynamic alloc mode
08/23/17    sm     Add support for single flush request for input and output port
07/25/17    sm     Fix issues with dynamic allocation mode
06/28/17    aw     Unify and update all logs in hyp-video
05/08/17    sm     Update for new hyp-video architecture
04/03/17    sm     Add support for Video input FE-BE
02/02/17    hl     Support video encode
09/29/16    hl     Fix to work with hypervisor LA target test
07/15/16    hl     Add code to support LA target
06/22/16    hl     Add dynamic buffer mode support
========================================================================== */

/* =======================================================================

INCLUDE FILES FOR MODULE

========================================================================== */
#include <poll.h>
#include <errno.h>
#include "hyp_videopriv_fe.h"
#include "hyp_videopriv.h"
#include "hyp_vidc_types.h"
#include "hyp_vidc_inf.h"
#include "linux_video_fe.h"
#include "linux_vdec_fe.h"
#include "linux_vdec_venc_common.h"
#include "MMCriticalSection.h"
#ifdef WIN32
#include "types.h"
#include "msm_vidc_dec.h"
#include "VideoPlatform.h"
#include "VideoComDef.h"
#include "msm_media_info.h"
#else

#include <ion/ion.h>
#include <linux/videodev2.h>
#ifdef SUPPORT_DMABUF
#include <vidc/media/msm_media_info.h>
#else
#include <media/msm_media_info.h>
#endif
#endif
#include "hyp_debug.h"

#define DEFAULT_METADATA_SIZE 4096

v4l2fe_vidc_format vdec_formats[] = {
    {
#ifdef _LINUX_
        {.name = "YCbCr Semiplanar 4:2:0"},
        {.description = "Y/CbCr 4:2:0"},
#else
        .name = "YCbCr Semiplanar 4:2:0",
        .description = "Y/CbCr 4:2:0",
#endif
        .fourcc = V4L2_PIX_FMT_NV12,
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
        .get_frame_size = NULL,
        .defer_outputs = false,
        .input_min_count = 0,
        .output_min_count = 0,
    },
    {
#ifdef _LINUX_
        {.name = "YCbCr Semiplanar 4:2:0 128 aligned"},
        {.description = "Y/CbCr 4:2:0 128 aligned"},
#else
        .name = "YCbCr Semiplanar 4:2:0 128 aligned",
        .description = "Y/CbCr 4:2:0 128 aligned",
#endif
        .fourcc = V4L2_PIX_FMT_NV12_128,
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
        .get_frame_size = NULL,
        .defer_outputs = false,
        .input_min_count = 0,
        .output_min_count = 0,
    },
    {
#ifdef _LINUX_
        {.name = "YCbCr Semiplanar 4:2:0 10bit"},
        {.description = "Y/CbCr 4:2:0 10bit"},
#else
        .name = "YCbCr Semiplanar 4:2:0 10bit",
        .description = "Y/CbCr 4:2:0 10bit",
#endif
        .fourcc = V4L2_PIX_FMT_SDE_Y_CBCR_H2V2_P010_VENUS,
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
        .get_frame_size = NULL,
        .defer_outputs = false,
        .input_min_count = 0,
        .output_min_count = 0,
    },
    {
#ifdef _LINUX_
        {.name = "UBWC YCbCr Semiplanar 4:2:0"},
        {.description = "UBWC Y/CbCr 4:2:0"},
#else
        .name = "UBWC YCbCr Semiplanar 4:2:0",
        .description = "UBWC Y/CbCr 4:2:0",
#endif
        .fourcc = V4L2_PIX_FMT_NV12_UBWC,
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
        .get_frame_size = NULL,
        .defer_outputs = false,
        .input_min_count = 0,
        .output_min_count = 0,
    },
    {
#ifdef _LINUX_
        {.name = "UBWC YCbCr Semiplanar 4:2:0 10bit"},
        {.description = "UBWC Y/CbCr 4:2:0 10bit"},
#else
        .name = "UBWC YCbCr Semiplanar 4:2:0 10bit",
        .description = "UBWC Y/CbCr 4:2:0 10bit",
#endif
        .fourcc = V4L2_PIX_FMT_NV12_TP10_UBWC,
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
        .get_frame_size = NULL,
        .defer_outputs = false,
        .input_min_count = 0,
        .output_min_count = 0,
    },
    {
#ifdef _LINUX_
        {.name = "Mpeg2"},
        {.description = "Mpeg2 compressed format"},
#else
        .name = "Mpeg2",
        .description = "Mpeg2 compressed format",
#endif
        .fourcc = V4L2_PIX_FMT_MPEG2,
        .type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
        .get_frame_size = NULL,
        .defer_outputs = false,
        .input_min_count = 4,
        .output_min_count = 6,
    },
    {
#ifdef _LINUX_
        {.name = "H264"},
        {.description = "H264 compressed format"},
#else
        .name = "H264",
        .description = "H264 compressed format",
#endif
        .fourcc = V4L2_PIX_FMT_H264,
        .type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
        .get_frame_size = NULL,
        .defer_outputs = false,
        .input_min_count = 4,
        .output_min_count = 8,
    },
    {
#ifdef _LINUX_
        {.name = "HEVC"},
        {.description = "HEVC compressed format"},
#else
        .name = "HEVC",
        .description = "HEVC compressed format",
#endif
        .fourcc = V4L2_PIX_FMT_HEVC,
        .type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
        .get_frame_size = NULL,
        .defer_outputs = false,
        .input_min_count = 4,
        .output_min_count = 8,
    },
    {
#ifdef _LINUX_
        {.name = "VP8"},
        {.description = "VP8 compressed format"},
#else
        .name = "VP8",
        .description = "VP8 compressed format",
#endif
        .fourcc = V4L2_PIX_FMT_VP8,
        .type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
        .get_frame_size = NULL,
        .defer_outputs = false,
        .input_min_count = 4,
        .output_min_count = 6,
    },
    {
#ifdef _LINUX_
        {.name = "VP9"},
        {.description = "VP9 compressed format"},
#else
        .name = "VP9",
        .description = "VP9 compressed format",
#endif
        .fourcc = V4L2_PIX_FMT_VP9,
        .type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
        .get_frame_size = NULL,
        .defer_outputs = true,
        .input_min_count = 4,
        .output_min_count = 11,
    },
};

v4l2fe_vidc_ctrl vdec_ctrls[] = {
    {
        .id = V4L2_CID_MPEG_VIDEO_H264_PROFILE,
#ifdef _LINUX_
        {.name = "H264 Profile"},
#else
        .name = "H264 Profile",
#endif
        .default_value = V4L2_MPEG_VIDEO_H264_PROFILE_HIGH,
        .minimum = 0,
        .maximum = 0,
        .level = 0,
        .menu_skip_mask = 0,
        .qmenu = NULL,
    },

    {
        .id = V4L2_CID_MPEG_VIDEO_HEVC_PROFILE,
#ifdef _LINUX_
        {.name = "HEVC Profile"},
#else
        .name = "HEVC Profile",
#endif
        .default_value = V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN,
        .minimum = 0,
        .maximum = 0,
        .level = 0,
        .menu_skip_mask = 0,
        .qmenu = NULL,
    },

    {
        .id = V4L2_CID_MPEG_VIDEO_VP8_PROFILE,
#ifdef _LINUX_
        {.name = "VP8 Profile"},
#else
        .name = "VP8 Profile",
#endif
        .default_value = V4L2_MPEG_VIDEO_VP8_PROFILE_0,
        .minimum = 0,
        .maximum = 0,
        .level = 0,
        .menu_skip_mask = 0,
        .qmenu = NULL,
    },

    {
        .id = V4L2_CID_MPEG_VIDEO_VP9_PROFILE,
#ifdef _LINUX_
        {.name = "VP9 Profile"},
#else
        .name = "VP9 Profile",
#endif
        .default_value = V4L2_MPEG_VIDEO_VP9_PROFILE_0,
        .minimum = 0,
        .maximum = 0,
        .level = 0,
        .menu_skip_mask = 0,
        .qmenu = NULL,
    },

    {
        .id = V4L2_CID_MPEG_VIDC_VIDEO_MPEG2_PROFILE,
#ifdef _LINUX_
        {.name = "MPEG2 Profile"},
#else
        .name = "MPEG2 Profile",
#endif
        .default_value = V4L2_MPEG_VIDC_VIDEO_MPEG2_PROFILE_MAIN,
        .minimum = 0,
        .maximum = 0,
        .level = 0,
        .menu_skip_mask = 0,
        .qmenu = NULL,
    },

    {
        .id = V4L2_CID_MPEG_VIDEO_HEVC_TIER,
#ifdef _LINUX_
        {.name = "HEVC Tier"},
#else
        .name = "HEVC Tier",
#endif
        .default_value = V4L2_MPEG_VIDEO_HEVC_TIER_HIGH,
        .minimum = 0,
        .maximum = 0,
        .level = 0,
        .menu_skip_mask = 0,
        .qmenu = NULL,
    },

    {
        .id = V4L2_CID_MPEG_VIDEO_H264_LEVEL,
#ifdef _LINUX_
        {.name = "H264 Level"},
#else
        .name = "H264 Level",
#endif
        .default_value = V4L2_MPEG_VIDEO_H264_LEVEL_5_0,
        .minimum = 0,
        .maximum = 0,
        .level = 0,
        .menu_skip_mask = 0,
        .qmenu = NULL,
    },

    {
        .id = V4L2_CID_MPEG_VIDEO_HEVC_LEVEL,
#ifdef _LINUX_
        {.name = "HEVC Level"},
#else
        .name = "HEVC Level",
#endif
        .default_value = V4L2_MPEG_VIDEO_HEVC_LEVEL_5,
        .minimum = 0,
        .maximum = 0,
        .level = 0,
        .menu_skip_mask = 0,
        .qmenu = NULL,
    },

    {
        .id = V4L2_CID_MPEG_VIDC_VIDEO_VP8_PROFILE_LEVEL,
#ifdef _LINUX_
        {.name = "VP8 Profile Level"},
#else
        .name = "VP8 Profile Level",
#endif
        .default_value = V4L2_MPEG_VIDC_VIDEO_VP8_VERSION_3,
        .minimum = 0,
        .maximum = 0,
        .level = 0,
        .menu_skip_mask = 0,
        .qmenu = NULL,
    },

    {
        .id = V4L2_CID_MPEG_VIDC_VIDEO_VP9_LEVEL,
#ifdef _LINUX_
        {.name = "VP9 Level"},
#else
        .name = "VP9 Level",
#endif
        .default_value = V4L2_MPEG_VIDC_VIDEO_VP9_LEVEL_61,
        .minimum = 0,
        .maximum = 0,
        .level = 0,
        .menu_skip_mask = 0,
        .qmenu = NULL,
    },

    {
        .id = V4L2_CID_MPEG_VIDC_VIDEO_MPEG2_LEVEL,
#ifdef _LINUX_
        {.name = "MPEG2 Level"},
#else
        .name = "MPEG2 Level",
#endif
        .default_value = V4L2_MPEG_VIDC_VIDEO_MPEG2_LEVEL_2,
        .minimum = 0,
        .maximum = 0,
        .level = 0,
        .menu_skip_mask = 0,
        .qmenu = NULL,
    },
};

/**===========================================================================

FUNCTION device_callback_dec_passthrough

@brief  Hypervisor decode FE callback function

@param [in] msg pointer
@param [in] lenght
@param [in] void pointer for private data

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type device_callback_dec_passthrough(uint8 *msg, uint32 length, void *cd)
{
    fe_io_session_t *fe_ioss = (fe_io_session_t *)cd;
    fe_linux_plt_data_t *plt_data = (fe_linux_plt_data_t *)fe_ioss->fe_plt_data;
    uint32 flags = 0;
    struct v4l2_event event;
    event_buf_type event_buf;

    UNUSED(length);

    HABMM_MEMSET(&event, 0, sizeof(struct v4l2_event));
    HABMM_MEMSET(&event_buf, 0, sizeof(event_buf_type));

    v4l2_passthrough_event_data_type *event_data = (v4l2_passthrough_event_data_type *)msg;
    v4l2_buffer_64b *buffer = (v4l2_buffer_64b *)&event_data->payload.buffer;
    v4l2_event_64b *v4l2_event_data = (v4l2_event_64b *)&event_data->payload.event_data;
    switch (event_data->event_type)
    {
        case VIDC_EVT_RESP_OUTPUT_DONE:
        {
            event_buf.buf.index = buffer->index;
            event_buf.buf.type = buffer->type;
            event_buf.buf.bytesused = buffer->bytesused;
            event_buf.buf.flags = buffer->flags;
            event_buf.buf.field = buffer->field;
            event_buf.buf.timestamp.tv_sec = buffer->tv_sec;
            event_buf.buf.timestamp.tv_usec = buffer->tv_usec;
            HABMM_MEMCPY(&event_buf.buf.timecode, &buffer->timecode, sizeof(struct v4l2_timecode));
            event_buf.buf.sequence = buffer->sequence;
            event_buf.buf.memory = V4L2_MEMORY_USERPTR;
            for (uint64 i = 0; i < buffer->length; i++)
            {
                event_buf.buf_planes[i].bytesused = buffer->m.planes[i].bytesused;
                event_buf.buf_planes[i].length = buffer->m.planes[i].length;
                event_buf.buf_planes[i].m.userptr = buffer->m.planes[i].m.userptr;
                event_buf.buf_planes[i].data_offset = buffer->m.planes[i].data_offset;
                HABMM_MEMCPY(&event_buf.buf_planes[i].reserved[0], &buffer->m.planes[i].reserved[0], 44);
            }
            event_buf.buf.length = buffer->length;
            event_buf.buf.reserved2 = buffer->reserved2;
            event_buf.buf.reserved = buffer->reserved;
            HYP_VIDEO_MSG_INFO("FBD: index = %u frame fd = %u flag = 0x%x tv_sec = %ld tv_usec = %ld"
                " inputTag = %u metadata = fd %u",
                event_buf.buf.index, event_buf.buf_planes[0].reserved[MSM_VIDC_BUFFER_FD],
                event_buf.buf.flags, event_buf.buf.timestamp.tv_sec,
                event_buf.buf.timestamp.tv_usec,
                event_buf.buf_planes[0].reserved[MSM_VIDC_INPUT_TAG_1], event_buf.buf_planes[1].reserved[MSM_VIDC_BUFFER_FD]);

            hyp_enqueue(&plt_data->evt_output_buf_queue, (void *)&event_buf);
            flags = POLLIN;
            fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &flags);
            break;
        }
        case VIDC_EVT_RESP_INPUT_DONE:
        {
            event_buf.buf.index = buffer->index;
            event_buf.buf.type = buffer->type;
            event_buf.buf.bytesused = buffer->bytesused;
            event_buf.buf.flags = buffer->flags;
            event_buf.buf.field = buffer->field;
            event_buf.buf.timestamp.tv_sec = buffer->tv_sec;
            event_buf.buf.timestamp.tv_usec = buffer->tv_usec;
            HABMM_MEMCPY(&event_buf.buf.timecode, &buffer->timecode, sizeof(struct v4l2_timecode));
            event_buf.buf.sequence = buffer->sequence;
            event_buf.buf.memory = V4L2_MEMORY_USERPTR;
            for (uint64 i = 0; i < buffer->length; i++)
            {
                event_buf.buf_planes[i].bytesused = buffer->m.planes[i].bytesused;
                event_buf.buf_planes[i].length = buffer->m.planes[i].length;
                event_buf.buf_planes[i].m.userptr = buffer->m.planes[i].m.userptr;
                event_buf.buf_planes[i].data_offset = buffer->m.planes[i].data_offset;
                HABMM_MEMCPY(&event_buf.buf_planes[i].reserved[0], &buffer->m.planes[i].reserved[0], 44);
            }
            event_buf.buf.length = buffer->length;
            event_buf.buf.reserved2 = buffer->reserved2;
            event_buf.buf.reserved = buffer->reserved;
            HYP_VIDEO_MSG_INFO("EBD: index = %u frame fd = %u flag = 0x%x tv_sec = %ld tv_usec = %ld"
                " inputTag = %u metadata = fd %u buffer->fd %d",
                event_buf.buf.index, event_buf.buf_planes[0].reserved[MSM_VIDC_BUFFER_FD],
                event_buf.buf.flags, event_buf.buf.timestamp.tv_sec,
                event_buf.buf.timestamp.tv_usec,
                event_buf.buf_planes[0].reserved[MSM_VIDC_INPUT_TAG_1], event_buf.buf_planes[1].reserved[MSM_VIDC_BUFFER_FD], (int)buffer->m.planes[0].reserved[0]);

            hyp_enqueue(&plt_data->evt_input_buf_queue, (void *)&event_buf);
            flags = POLLOUT;
            fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &flags);
            break;
        }
        default:
        {
            event.type = v4l2_event_data->type;
            HABMM_MEMCPY(event.u.data, v4l2_event_data->u.data, 64);
            event.pending = v4l2_event_data->pending;
            event.sequence = v4l2_event_data->sequence;
            event.timestamp.tv_sec = v4l2_event_data->tv_sec;
            event.timestamp.tv_nsec = v4l2_event_data->tv_nsec;
            event.id = v4l2_event_data->id;
            for (int i = 0; i < 8; i++)
            {
                event.reserved[i] = v4l2_event_data->reserved[i];
            }
            flags = POLLPRI;
            HYP_VIDEO_MSG_INFO("v4l2_event_data->type %llu", v4l2_event_data->type);
            hyp_enqueue(&plt_data->evt_queue, (void *)&event);
            fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &flags);
            break;
        }
    }

    return HYPV_STATUS_SUCCESS;
}

/**===========================================================================

FUNCTION device_callback_dec

@brief  Hypervisor decode FE callback function

@param [in] msg pointer
@param [in] lenght
@param [in] void pointer for private data

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type device_callback_dec(uint8 *msg, uint32 length, void *cd)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    uint32 i = 0;
    fe_io_session_t *fe_ioss = (fe_io_session_t *)cd;
    vidc_frame_data_type* frame_data;
    long sec, usec;
    vidc_drv_msg_info_type *pEvent = (vidc_drv_msg_info_type *)msg;
    fe_linux_plt_data_t *plt_data = (fe_linux_plt_data_t *)fe_ioss->fe_plt_data;
    uint32 flags = 0;
    struct v4l2_event event;
    event_buf_type event_buf;

    UNUSED(length);

    HABMM_MEMSET(&event, 0, sizeof(struct v4l2_event));
    HABMM_MEMSET(&event_buf, 0, sizeof(event_buf_type));

    if (TRUE == fe_ioss->io_handle->passthrough_mode)
    {
        return device_callback_dec_passthrough(msg, length, cd);
    }

    HYP_VIDEO_MSG_INFO("event type 0x%x event status %u",
                        (unsigned int)pEvent->event_type, pEvent->status);
    switch (pEvent->event_type)
    {
    case VIDC_EVT_RESP_FLUSH_INPUT_DONE:
    case VIDC_EVT_RESP_FLUSH_OUTPUT_DONE:
        {
            hypv_session_t* hypv_session = fe_ioss->io_handle;
            /* clearing the bit indicates the receiving of corresponding event */
            MM_CriticalSection_Enter(hypv_session->flush_lock);
            if (VIDC_EVT_RESP_FLUSH_OUTPUT_DONE == pEvent->event_type)
            {
                hypv_session->flush_clr ^= V4L2_CMD_FLUSH_CAPTURE;
            }
            else
            {
                hypv_session->flush_clr ^= V4L2_CMD_FLUSH_OUTPUT;
            }
            MM_CriticalSection_Leave(hypv_session->flush_lock);

            /* callback when cmd=FLUSH_OUTPUT or cmd=FLUSH_CAPTURE or when
             * cmd=FLUSH_ALL but one device_callback_dec has been recevied previously */
            if (0 == hypv_session->flush_clr)
            {
                event.type = V4L2_EVENT_MSM_VIDC_FLUSH_DONE;
                *(uint32_t *)event.u.data = hypv_session->flush_req;
                hyp_enqueue(&plt_data->evt_queue, (void *)&event);
                flags = POLLPRI;
                fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &flags);
                MM_CriticalSection_Enter(hypv_session->flush_lock);
                hypv_session->flush_req = 0;
                MM_CriticalSection_Leave(hypv_session->flush_lock);
            }
            else
            {
                HYP_VIDEO_MSG_INFO("wait one more flush_done event for type 0x%x",
                        hypv_session->flush_clr);
            }
            break;
        }
    case VIDC_EVT_INPUT_RECONFIG:
        {
            if (VIDC_ERR_UNSUPPORTED_STREAM == pEvent->status)
            {
                event.type = V4L2_EVENT_MSM_VIDC_HW_UNSUPPORTED;
            }
            else
            {
                event.type = V4L2_EVENT_MSM_VIDC_SYS_ERROR;
            }
            hyp_enqueue(&plt_data->evt_queue, (void *)&event);
            flags = POLLPRI;
            fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &flags);
            break;
        }
    case VIDC_EVT_RESP_INPUT_DONE:
        {
            if (V4L2FE_STATE_PAUSE == plt_data->state)
            {
               plt_data->state = V4L2FE_STATE_EXECUTING;
            }
            frame_data = &pEvent->payload.frame_data;
            hypv_session_t* hypv_session = fe_ioss->io_handle;

            if (HYP_TARGET_LEMANS == hypv_session->target_variant)
            {
                dec_enc_free_meta_buffer((int)(uintptr_t)frame_data->metadata_addr);
                HYP_VIDEO_MSG_INFO("Free meta buffer fd %lu",
                        (unsigned long)frame_data->frame_addr);

            }

            if (fe_ioss->eos_buffer.data_fd == (int32)(uintptr_t)frame_data->frame_addr)
            {
               /* This is EOS buffer allocated internally by hyp-video.
                  Skip returning to client */
               dec_enc_free_eos_buffer(fe_ioss);
               HYP_VIDEO_MSG_HIGH("Free EOS buffer fd %lu",
                                   (unsigned long)frame_data->frame_addr );
            }
            else if (VIDC_ERR_UNSUPPORTED_STREAM == pEvent->status)
            {
                HYP_VIDEO_MSG_ERROR("unspported stream");
                event.type = V4L2_EVENT_MSM_VIDC_HW_UNSUPPORTED;
                hyp_enqueue(&plt_data->evt_queue, (void *)&event);
                flags = POLLPRI;
                fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &flags);
            }
            else if (VIDC_ERR_HW_FATAL == pEvent->status)
            {
                HYP_VIDEO_MSG_ERROR("failed h/w fatal error- pHeader 0x%llx pBuffer %p",
                                     pEvent->payload.frame_data.frm_clnt_data,
                                     pEvent->payload.frame_data.frame_addr );
                event.type = V4L2_EVENT_MSM_VIDC_SYS_ERROR;
                hyp_enqueue(&plt_data->evt_queue, (void *)&event);
                flags = POLLPRI;
                fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &flags);
            }
            else
            {
                event_buf.buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
                event_buf.buf.memory = V4L2_MEMORY_USERPTR;
                event_buf.buf.length = NUM_OUTPUT_BUFFER_PLANE_DEC;
                event_buf.buf_planes[0].reserved[MSM_VIDC_BUFFER_FD] = (unsigned long)frame_data->frame_addr;
                event_buf.buf_planes[0].m.userptr = (unsigned long)frame_data->frame_addr;
                event_buf.buf_planes[0].length = frame_data->alloc_len;
                event_buf.buf_planes[0].bytesused = frame_data->data_len;
                event_buf.buf_planes[0].reserved[MSM_VIDC_DATA_OFFSET] = frame_data->offset;

                MM_CriticalSection_Enter(fe_ioss->lock_buffer);
                if (NULL != plt_data->v4l2fe_etb_flag_info)
                {
                    for (i = 0; i< fe_ioss->input_buffer_count; i++)
                    {
                        if (0 != plt_data->v4l2fe_etb_flag_info[i].is_etb)
                        {
                            if (plt_data->v4l2fe_etb_flag_info[i].address == frame_data->frame_addr)
                            {
                                // found the flag information
                                event_buf.buf.flags |= plt_data->v4l2fe_etb_flag_info[i].flags;
                                plt_data->v4l2fe_etb_flag_info[i].is_etb = 0;
                            }
                        }
                    }
                }
                MM_CriticalSection_Leave(fe_ioss->lock_buffer);

                if (frame_data->flags & VIDC_FRAME_FLAG_EOS)
                {
                    event_buf.buf.flags |= V4L2_BUF_FLAG_EOS;
                }
                if (frame_data->flags & VIDC_FRAME_FLAG_CODECCONFIG)
                {
                    event_buf.buf.flags |= V4L2_BUF_FLAG_CODECCONFIG;
                }
                if (frame_data->flags & VIDC_FRAME_FLAG_SYNCFRAME)
                {
                    event_buf.buf.flags |= V4L2_BUF_FLAG_KEYFRAME;
                }
                sec = (long) V4L2FE_CONVERT_USEC_TO_SEC(frame_data->timestamp);
                usec =(long) (frame_data->timestamp - (int64) V4L2FE_CONVERT_SEC_TO_USEC(sec));
                event_buf.buf.timestamp.tv_sec = sec;
                event_buf.buf.timestamp.tv_usec = usec;
                std::map<uint64, uint64>::iterator itr;
                MM_CriticalSection_Enter(fe_ioss->lock_buffer);
                itr = fe_ioss->input_tag_entry->find(frame_data->frm_clnt_data);
                if (itr != fe_ioss->input_tag_entry->end())
                {
                    event_buf.buf.index = (unsigned int)itr->second;
                    fe_ioss->input_tag_entry->erase(itr);
                }
                MM_CriticalSection_Leave(fe_ioss->lock_buffer);
                HYP_VIDEO_MSG_HIGH("EBD: flag = 0x%x tv_sec = %ld tv_usec = %ld"
                                   " index = %u userptr = 0x%lx input tag %lu",
                                    event_buf.buf.flags, event_buf.buf.timestamp.tv_sec,
                                    event_buf.buf.timestamp.tv_usec, event_buf.buf.index,
                                    event_buf.buf_planes[0].m.userptr, (unsigned long)frame_data->input_tag);

                hyp_enqueue(&plt_data->evt_input_buf_queue, (void *)&event_buf);
                flags = POLLOUT;
                fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &flags);
            }
            break;
        }
    case VIDC_EVT_RESP_OUTPUT_DONE:
        {
            if (V4L2FE_STATE_PAUSE == plt_data->state)
            {
               plt_data->state = V4L2FE_STATE_EXECUTING;
            }
            event_buf.buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
            event_buf.buf.memory = V4L2_MEMORY_USERPTR;
            event_buf.buf.length = NUM_CAPTURE_BUFFER_PLANE_DEC;
            frame_data = &pEvent->payload.frame_data;
            if (frame_data->flags & VIDC_FRAME_FLAG_EOS)
            {
                event_buf.buf.flags |= V4L2_BUF_FLAG_EOS;
            }
            if (frame_data->flags & VIDC_FRAME_FLAG_CODECCONFIG)
            {
                event_buf.buf.flags |= V4L2_BUF_FLAG_CODECCONFIG;
            }
            if (frame_data->flags & VIDC_FRAME_FLAG_SYNCFRAME)
            {
                event_buf.buf.flags |= V4L2_BUF_FLAG_KEYFRAME;
            }
            if (frame_data->flags & VIDC_FRAME_FLAG_DATACORRUPT)
            {
                event_buf.buf.flags |= V4L2_BUF_FLAG_DATA_CORRUPT;
            }
            if (frame_data->flags & VIDC_FRAME_FLAG_ENDOFSUBFRAME)
            {
                event_buf.buf.flags |= V4L2_BUF_FLAG_END_OF_SUBFRAME;
            }
            if (frame_data->flags & VIDC_FRAME_FLAG_READONLY)
            {
                event_buf.buf.flags |= V4L2_BUF_FLAG_READONLY;
            }
            if (frame_data->flags & VIDC_FRAME_FLAG_DROP_FRAME)
            {
                frame_data->data_len = 0;
            }
            event_buf.buf_planes[0].m.userptr = (unsigned long)frame_data->frame_addr;
            event_buf.buf_planes[1].m.userptr = (unsigned long)frame_data->metadata_addr;
            event_buf.buf_planes[0].bytesused = frame_data->data_len;
            event_buf.buf_planes[1].bytesused = frame_data->alloc_metadata_len;
            event_buf.buf_planes[0].reserved[MSM_VIDC_BUFFER_FD] = (unsigned long)frame_data->frame_addr;
            event_buf.buf_planes[1].reserved[MSM_VIDC_BUFFER_FD] = (unsigned long)frame_data->metadata_addr;
            event_buf.buf_planes[0].reserved[MSM_VIDC_DATA_OFFSET] = frame_data->offset;
            event_buf.buf_planes[0].reserved[MSM_VIDC_COMP_RATIO] = 0;
            event_buf.buf_planes[0].reserved[MSM_VIDC_INPUT_TAG_1] = frame_data->input_tag;
            event_buf.buf_planes[0].reserved[MSM_VIDC_INPUT_TAG_2] = frame_data->input_tag2;
            event_buf.buf_planes[0].reserved[MSM_VIDC_FRAMERATE] = 0;
            event_buf.buf_planes[0].reserved[6] = 0;
            event_buf.buf_planes[0].reserved[7] = 0;
            event_buf.buf.index = (unsigned int)frame_data->frm_clnt_data;
            sec = (long) V4L2FE_CONVERT_USEC_TO_SEC(frame_data->timestamp);
            usec = (long) (frame_data->timestamp - (int64) V4L2FE_CONVERT_SEC_TO_USEC(sec));
            event_buf.buf.timestamp.tv_sec = sec;
            event_buf.buf.timestamp.tv_usec = usec;
            HYP_VIDEO_MSG_HIGH("FBD: index = %u frame_addr = %lu flag = 0x%x tv_sec = %ld tv_usec = %ld"
                               " inputTag %lu metadata fd %lu",
                                event_buf.buf.index, event_buf.buf_planes[0].m.userptr,
                                event_buf.buf.flags, event_buf.buf.timestamp.tv_sec,
                                event_buf.buf.timestamp.tv_usec,
                                frame_data->input_tag, event_buf.buf_planes[1].m.userptr);

            MM_CriticalSection_Enter( fe_ioss->lock_buffer );
            if (!(frame_data->flags & VIDC_FRAME_FLAG_READONLY))
            {
                dec_enc_rm_dyn_buf_ref(fe_ioss, frame_data);
            }
            MM_CriticalSection_Leave( fe_ioss->lock_buffer );

            hyp_enqueue(&plt_data->evt_output_buf_queue, (void *)&event_buf);
            flags = POLLIN;
            fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &flags);
            break;
        }
    case VIDC_EVT_OUTPUT_RECONFIG:
        {
            vidc_frame_size_type frame_size;
            vidc_bit_depth_type bit_depth;
            vidc_dec_output_crop_type crop;
            uint32 isProgressive = 1;
            vidc_buffer_type buf_type;

            event.type = V4L2_EVENT_MSM_VIDC_PORT_SETTINGS_CHANGED_INSUFFICIENT;
            unsigned int *ptr = (unsigned int *)(void *)event.u.data;

            HABMM_MEMSET(&bit_depth, 0, sizeof(vidc_bit_depth_type));
            buf_type = VIDC_BUFFER_OUTPUT;

            bit_depth.buffer_type = buf_type;
            MM_CriticalSection_Enter(fe_ioss->lock_buffer);

            rc = dec_enc_get_drv_property(fe_ioss,
                                          VIDC_I_BIT_DEPTH,
                                          sizeof(vidc_bit_depth_type),
                                          (uint8 *)&bit_depth );

            /* update output pixel color format based on the bit depth */
            if (HYPV_STATUS_SUCCESS != rc)
            {
                ptr[MSM_VIDC_BIT_DEPTH] = MSM_VIDC_BIT_DEPTH_8;
                HYP_VIDEO_MSG_ERROR("Query bit depth failed. Default to 8 bit");
            }
            else
            {
                if (VIDC_BITDEPTTH_8BIT == bit_depth.bit_depth)
                {
                    ptr[MSM_VIDC_BIT_DEPTH] = MSM_VIDC_BIT_DEPTH_8;
                }
                else
                {
                    ptr[MSM_VIDC_BIT_DEPTH] = MSM_VIDC_BIT_DEPTH_10;
                }
                HYP_VIDEO_MSG_INFO("Query bit depth %u", bit_depth.bit_depth);
            }
            fe_ioss->io_handle->bit_depth = ptr[MSM_VIDC_BIT_DEPTH];

            HABMM_MEMSET(&frame_size, 0, sizeof(vidc_frame_size_type));
            frame_size.buf_type = buf_type;

            rc = dec_enc_get_drv_property(fe_ioss,
                                          VIDC_I_FRAME_SIZE,
                                          sizeof(vidc_frame_size_type),
                                          (uint8 *)&frame_size);
            if (HYPV_STATUS_SUCCESS == rc)
            {
                HYP_VIDEO_MSG_INFO("frame size, height=%u width=%u",
                                    frame_size.height, frame_size.width);

                fe_ioss->frame_size.height = frame_size.height;
                fe_ioss->frame_size.width = frame_size.width;

                if (MIN_SUPPORTED_HEIGHT > V4L2FE_ALIGN(frame_size.height, 16) ||
                    MIN_SUPPORTED_WIDTH > V4L2FE_ALIGN(frame_size.width, 16))
                {
                    HYP_VIDEO_MSG_ERROR("Unsupported WxH (%u)x(%u), min supported is (%d)x(%d)",
                                         V4L2FE_ALIGN(frame_size.width, 16),
                                         V4L2FE_ALIGN(frame_size.height, 16),
                                         MIN_SUPPORTED_WIDTH, MIN_SUPPORTED_HEIGHT);
                    event.type = V4L2_EVENT_MSM_VIDC_HW_UNSUPPORTED;
                    rc = HYPV_STATUS_FAIL;
                }
            }
            else
            {
                HYP_VIDEO_MSG_INFO("Failed to get frame size");
            }

            if (HYPV_STATUS_SUCCESS == rc)
            {
                ptr[MSM_VIDC_HEIGHT] = fe_ioss->frame_size.height;
                ptr[MSM_VIDC_WIDTH] = fe_ioss->frame_size.width;

                rc = dec_enc_get_drv_property(fe_ioss,
                                              VIDC_I_DEC_PROGRESSIVE_ONLY,
                                              sizeof(uint32),
                                              (uint8 *)&isProgressive);
                if (HYPV_STATUS_SUCCESS != rc)
                {
                    HYP_VIDEO_MSG_HIGH("failed to query progressive, default to progressive");
                    isProgressive = 1;
                }
                ptr[MSM_VIDC_PIC_STRUCT] = isProgressive;
            }

            /* If scan type is interlaced then query the minimum output
            ** buffers set by firmware
            */
            if (HYPV_STATUS_SUCCESS == rc && 0 == isProgressive)
            {
               vidc_buffer_reqmnts_type buffer_req_data;

               HABMM_MEMSET(&buffer_req_data, 0, sizeof(vidc_buffer_reqmnts_type));
               buffer_req_data.buf_type = VIDC_BUFFER_OUTPUT;

               rc = dec_enc_get_drv_property(fe_ioss,
                                             VIDC_I_BUFFER_REQUIREMENTS,
                                             sizeof(buffer_req_data),
                                             (uint8 *)&buffer_req_data);

               if (HYPV_STATUS_SUCCESS == rc)
               {
                  ptr[MSM_VIDC_FW_MIN_COUNT] = buffer_req_data.min_count;
               }
               else
               {
                  HYP_VIDEO_MSG_ERROR("failed to query buffer requirements for buffer type %d", buffer_req_data.buf_type);
               }
            }

            HABMM_MEMSET(&crop, 0, sizeof(vidc_dec_output_crop_type));

            if (HYPV_STATUS_SUCCESS == rc)
            {
                rc = dec_enc_get_drv_property(fe_ioss,
                                              VIDC_I_DEC_OUTPUT_CROP,
                                              sizeof(vidc_dec_output_crop_type),
                                              (uint8 *)&crop);
                if (HYPV_STATUS_SUCCESS != rc)
                {
                    HYP_VIDEO_MSG_HIGH("failed to query crop info. default to zero");
                    HABMM_MEMSET(&crop, 0, sizeof(vidc_dec_output_crop_type));
                }
                ptr[MSM_VIDC_CROP_LEFT] = crop.crop_left;
                ptr[MSM_VIDC_CROP_TOP] = crop.crop_top;
                ptr[MSM_VIDC_CROP_WIDTH] = crop.crop_width;
                ptr[MSM_VIDC_CROP_HEIGHT] = crop.crop_height;
            }

            fe_ioss->io_handle->in_output_reconfig = TRUE;

            MM_CriticalSection_Leave(fe_ioss->lock_buffer);

            hyp_enqueue(&plt_data->evt_queue, (void *)&event);
            flags = POLLPRI;
            fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &flags);
            break;
        }
    case VIDC_EVT_ERR_HWFATAL:
    case VIDC_EVT_ERR_CLIENTFATAL:
        {
            event.type = V4L2_EVENT_MSM_VIDC_SYS_ERROR;
            hyp_enqueue(&plt_data->evt_queue, (void *)&event);
            flags = POLLPRI;
            fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &flags);
            break;
        }
    case VIDC_EVT_RESP_START:
        {
            if (V4L2FE_STATE_EXECUTING != plt_data->state)
            {
                plt_data->state = V4L2FE_STATE_EXECUTING;
                MM_Signal_Set(fe_ioss->state_synch_obj);
            }
            break;
        }
    case VIDC_EVT_RESP_STOP:
        {
            if (V4L2FE_STATE_IDLE != plt_data->state)
            {
                plt_data->state = V4L2FE_STATE_IDLE;
                MM_Signal_Set(fe_ioss->state_synch_obj);
            }
            break;
        }
    case VIDC_EVT_RESP_PAUSE:
        {
            plt_data->state = V4L2FE_STATE_PAUSE;
            break;
        }
    case VIDC_EVT_RESP_RESUME:
        {
            if (V4L2FE_STATE_EXECUTING != plt_data->state)
            {
                plt_data->state = V4L2FE_STATE_EXECUTING;
            }
            break;
        }
    case VIDC_EVT_RESP_LOAD_RESOURCES:
        {
            plt_data->state = V4L2FE_STATE_IDLE;
            break;
        }
    case VIDC_EVT_RESP_RELEASE_RESOURCES:
        {
            if (V4L2FE_STATE_LOADED != plt_data->state)
            {
                plt_data->state = V4L2FE_STATE_LOADED;
                MM_Signal_Set(fe_ioss->state_synch_obj);
            }
            break;
        }
    case VIDC_EVT_RELEASE_BUFFER_REFERENCE:
        {
            hypv_session_t* hypv_session = fe_ioss->io_handle;
            frame_data = &pEvent->payload.frame_data;

            MM_CriticalSection_Enter(fe_ioss->lock_buffer);
            if (NULL == fe_ioss->out_dynamic_info)
            {
                HYP_VIDEO_MSG_ERROR("null out_dynamic_info");
                MM_CriticalSection_Leave(fe_ioss->lock_buffer);
                break;
            }

            for (i = 0; i < fe_ioss->dynmaic_buf_entry_count; i++)
            {
                if ((int32)(uintptr_t)frame_data->frame_addr == fe_ioss->out_dynamic_info[i].fd)
                {
                    uint32 *ptr = (uint32 *)(void *)event.u.data;

                    ptr[0] = fe_ioss->out_dynamic_info[i].fd;
                    frame_data->metadata_addr = (uint8 *)(uintptr_t)fe_ioss->out_dynamic_info[i].metadata_fd;
                    frame_data->alloc_metadata_len = fe_ioss->out_dynamic_info[i].metadata_alloc_len;
                    frame_data->frm_clnt_data = fe_ioss->out_dynamic_info[i].index;
                    break;
                }
            }

            dec_enc_rm_dyn_buf_ref(fe_ioss, frame_data);
            if (1 == fe_ioss->out_dynamic_info[i].ref_count)
            {
                if (!(hypv_session->flush_req & V4L2_CMD_FLUSH_CAPTURE))
                {
                    /* queue pending FTB */
                    if (NULL != frame_data->metadata_addr)
                    {
                        frame_data->non_contiguous_metadata = true;
                    }
                    HYP_VIDEO_MSG_HIGH("Queue pending fbd. index %lu buffer %d metadata_fd %d alloc_metadata_len %u",
                                        (unsigned long)frame_data->frm_clnt_data,
                                        (int32)(uintptr_t)frame_data->frame_addr,
                                        (int32)(uintptr_t)frame_data->metadata_addr,
                                        frame_data->alloc_metadata_len);
                    if (HYPV_STATUS_SUCCESS != hyp_device_ioctl(
                                                    fe_ioss->io_handle,
                                                    VIDC_IOCTL_FILL_OUTPUT_BUFFER,
                                                    (uint8 *)frame_data,
                                                    sizeof(vidc_frame_data_64b_type),
                                                    NULL, 0))
                    {
                        HYP_VIDEO_MSG_ERROR("failed to push pending FTB");
                    }
                }
                else
                {
                    pEvent->event_type = VIDC_EVT_RESP_OUTPUT_DONE;
                    MM_CriticalSection_Leave(fe_ioss->lock_buffer);
                    device_callback_dec(msg, length, cd);
                    MM_CriticalSection_Enter(fe_ioss->lock_buffer);
                }
            }

            MM_CriticalSection_Leave( fe_ioss->lock_buffer );

            event.type = V4L2_EVENT_MSM_VIDC_RELEASE_BUFFER_REFERENCE;
            hyp_enqueue(&plt_data->evt_queue, (void *)&event);
            flags = POLLPRI;
            fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &flags);

            break;
        }
    default:
        {
            HYP_VIDEO_MSG_ERROR( "Unknown Event 0x%x", (unsigned int)pEvent->event_type);
            break;
        }
    }

    return HYPV_STATUS_SUCCESS;
}

/**===========================================================================

FUNCTION dec_s_frame_rate

@brief  Decoder FE set frame rate

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type dec_s_frame_rate(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_frame_rate_type frame_rate;

    HABMM_MEMSET(&frame_rate, 0, sizeof(vidc_frame_rate_type));

    // round off the fps
    frame_rate.fps_numerator = (data->value >> 16) + ((data->value & 0xFFFF) >= 0x8000 ? 1 : 0);
    frame_rate.fps_denominator = 1;
    frame_rate.buf_type = VIDC_BUFFER_INPUT;
    rc = dec_enc_set_drv_property(fe_ioss,
                                  VIDC_I_FRAME_RATE,
                                  sizeof(vidc_frame_rate_type),
                                  ( uint8* )&frame_rate);
    if (HYPV_STATUS_SUCCESS != rc)
    {
        rc = HYPV_STATUS_FAIL;
        HYP_VIDEO_MSG_ERROR("failed to set dec frame rate %u fps", frame_rate.fps_numerator);
    }
    else
    {
        HYP_VIDEO_MSG_INFO("Set dec framerate to %u fps", frame_rate.fps_numerator);
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_dec_s_fmt

@brief  Decoder FE set format

@param [in] fe_ioss pointer
@param [in] v4l2_format pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_dec_s_fmt(fe_io_session_t* fe_ioss, struct v4l2_format* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_frame_size_type frame_size;
    vidc_buffer_type buf_type = VIDC_BUFFER_UNUSED;

    HYP_VIDEO_MSG_INFO("buf type %u, pixel format 0x%x", data->type,
                        data->fmt.pix_mp.pixelformat);
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == data->type)
    {
        vidc_color_format_config_type   color_format_config;
        HYP_VIDEO_MSG_INFO("width %u height %u", data->fmt.pix_mp.width,
                            data->fmt.pix_mp.height);
        if ((TRUE == fe_ioss->io_handle->multi_stream_enable) ||
            (V4L2_PIX_FMT_SDE_Y_CBCR_H2V2_P010_VENUS == data->fmt.pix_mp.pixelformat) ||
            (V4L2_PIX_FMT_NV12 == data->fmt.pix_mp.pixelformat) ||
            (V4L2_PIX_FMT_NV12_128 == data->fmt.pix_mp.pixelformat))
        {
            v4l2fe_decoder_multistream_config(fe_ioss);
            buf_type = VIDC_BUFFER_OUTPUT2;
        }
        else
        {
            buf_type = VIDC_BUFFER_OUTPUT;
        }
        frame_size.buf_type = color_format_config.buf_type = buf_type;

        /* set dynamic buffer mode as default */
        HYP_VIDEO_MSG_HIGH("Set dynamic buffer mode for buf type %d", buf_type);
        rc = dec_enc_set_buf_alloc_mode(fe_ioss, VIDC_BUFFER_MODE_DYNAMIC, buf_type);
        if (HYPV_STATUS_SUCCESS != rc)
        {
             HYP_VIDEO_MSG_ERROR("set dynamic alloc mode failed for buf type %d", buf_type);
        }

        // program the color format
        if ((V4L2_PIX_FMT_NV12 == data->fmt.pix_mp.pixelformat) ||
            (V4L2_PIX_FMT_NV12_128 == data->fmt.pix_mp.pixelformat))
        {
            color_format_config.color_format = VIDC_COLOR_FORMAT_NV12;
        }
        else if (V4L2_PIX_FMT_NV12_UBWC == data->fmt.pix_mp.pixelformat)
        {
            color_format_config.color_format = VIDC_COLOR_FORMAT_NV12_UBWC;
        }
        else if (V4L2_PIX_FMT_NV12_TP10_UBWC == data->fmt.pix_mp.pixelformat)
        {
            color_format_config.color_format = VIDC_COLOR_FORMAT_YUV420_TP10_UBWC;
        }
        else if (V4L2_PIX_FMT_SDE_Y_CBCR_H2V2_P010_VENUS == data->fmt.pix_mp.pixelformat)
        {
            color_format_config.color_format = VIDC_COLOR_FORMAT_NV12_P010;
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("Color format 0x%x not supported!", data->fmt.pix_mp.pixelformat);
            rc = HYPV_STATUS_FAIL;
        }
        if (HYPV_STATUS_SUCCESS == rc)
        {
            rc = dec_enc_set_drv_property(fe_ioss,
                    VIDC_I_COLOR_FORMAT,
                    sizeof(vidc_color_format_config_type),
                    ( uint8* )&color_format_config);
            if (HYPV_STATUS_SUCCESS == rc)
            {
                fe_ioss->color_format_config = color_format_config;
                fe_ioss->output_fourCC = data->fmt.pix_mp.pixelformat;
            }
        }
    }
    else if (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == data->type)
    {
        frame_size.buf_type = VIDC_BUFFER_INPUT;

        // program the codec
        if (0 == fe_ioss->session_codec.codec)
        {
            vidc_session_codec_type session_codec;

            session_codec.session = VIDC_SESSION_DECODE;
            session_codec.codec = codecV4l2ToVidc(data->fmt.pix_mp.pixelformat);
            if (VIDC_CODEC_UNUSED != session_codec.codec)
            {
                // set the session_codec
                rc = dec_enc_set_drv_property(fe_ioss,
                    VIDC_I_SESSION_CODEC,
                    sizeof(vidc_session_codec_type),
                    ( uint8* )&session_codec );
                if (HYPV_STATUS_SUCCESS == rc)
                {

                    fe_ioss->input_fourCC = data->fmt.pix_mp.pixelformat;
                    fe_ioss->session_codec = session_codec;
                    fe_ioss->io_handle->codec = session_codec.codec;
                }
                else
                {
                    HYP_VIDEO_MSG_ERROR("failed to set drv property session codec");
                }
            }
            else
            {
                HYP_VIDEO_MSG_ERROR("unsupported codec");
                rc = HYPV_STATUS_FAIL;
            }


        }
        /* set dynamic buffer mode as default */
        HYP_VIDEO_MSG_HIGH("Set dynamic buffer mode for buf type %d", VIDC_BUFFER_INPUT);
        rc = dec_enc_set_buf_alloc_mode(fe_ioss, VIDC_BUFFER_MODE_DYNAMIC, VIDC_BUFFER_INPUT);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            if (HYPV_STATUS_ALLOC_FAIL == rc)
            {
               errno = ENOMEM;
               HYP_VIDEO_MSG_ERROR("NO_MEMORY error reported in alloc mode rc 0x%x", rc);
            }
            else
            {
                HYP_VIDEO_MSG_ERROR("set dynamic alloc mode failed for buf type %d", VIDC_BUFFER_INPUT);
            }
        }
    }
    else
    {
        HYP_VIDEO_MSG_ERROR("failed to set dec format. bad buf type %u", data->type);
        rc = HYPV_STATUS_FAIL;
    }

    if (HYPV_STATUS_SUCCESS == rc)
    {

        frame_size.height = data->fmt.pix_mp.height;
        frame_size.width = data->fmt.pix_mp.width;

        rc = dec_enc_set_drv_property(fe_ioss,
            VIDC_I_FRAME_SIZE,
            sizeof(vidc_frame_size_type),
            ( uint8* )&frame_size );

        if (HYPV_STATUS_SUCCESS == rc)
        {
            fe_ioss->frame_size.height = frame_size.height;
            fe_ioss->frame_size.width = frame_size.width;
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("failed to set frame size property. buf type %d",
                                 frame_size.buf_type);
            rc = HYPV_STATUS_FAIL;
        }
    }
    if (HYPV_STATUS_SUCCESS == rc)
    {
        if ((FALSE == fe_ioss->io_handle->set_secure_property) &&
            (TRUE == fe_ioss->io_handle->secure))
        {
            fe_ioss->io_handle->set_secure_property = TRUE;
            rc = dec_enc_s_secure_video(fe_ioss, TRUE);
            if (HYPV_STATUS_SUCCESS != rc)
            {
                if (HYPV_STATUS_ERR_MAX_CLIENT == rc)
                {
                    HYP_VIDEO_MSG_ERROR("secure instances count reached max value");
                    errno = ENOMEM;
                }
                else
                {
                    HYP_VIDEO_MSG_ERROR("failed to set secure video property");
                }
            }
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_dec_g_fmt

@brief  Decoder FE get format

@param [in] fe_ioss pointer
@param [in] v4l2_format pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_dec_g_fmt(fe_io_session_t* fe_ioss, struct v4l2_format* data)
{
    vidc_buffer_type buffer_type;
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    uint32 pixel_format = 0;
    uint32 num_planes = 0;

    HYP_VIDEO_MSG_INFO("buf type is %u", data->type);
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == data->type)
    {
        if (TRUE == fe_ioss->io_handle->multi_stream_enable)
        {
            buffer_type = VIDC_BUFFER_OUTPUT2;
        }
        else
        {
            buffer_type = VIDC_BUFFER_OUTPUT;
        }
        pixel_format = fe_ioss->output_fourCC ? fe_ioss->output_fourCC : V4L2_PIX_FMT_NV12_UBWC;
        num_planes = NUM_CAPTURE_BUFFER_PLANE_DEC;
    }
    else if (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == data->type)
    {
        buffer_type = VIDC_BUFFER_INPUT;
        pixel_format = fe_ioss->input_fourCC;
        num_planes = NUM_OUTPUT_BUFFER_PLANE_DEC;
    }
    else
    {
        HYP_VIDEO_MSG_ERROR("unsupported type");
        rc = HYPV_STATUS_FAIL;
    }
    if (HYPV_STATUS_SUCCESS == rc)
    {
        vidc_frame_size_type frame_size;

        HABMM_MEMSET(&frame_size, 0, sizeof(vidc_frame_size_type));
        frame_size.buf_type = buffer_type;
        rc = dec_enc_get_drv_property(fe_ioss,
            VIDC_I_FRAME_SIZE,
            sizeof(vidc_frame_size_type),
            ( uint8* )&frame_size );
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to get drv property - VIDC_I_FRAME_SIZE vidc buf type %d",frame_size.buf_type);
        }
        else
        {
            uint32 stride = frame_size.width;
            uint32 scanlines = frame_size.height;

            vidc_buffer_reqmnts_type buffer_req_data;

            HABMM_MEMSET(&buffer_req_data, 0, sizeof(vidc_buffer_reqmnts_type));
            buffer_req_data.buf_type = buffer_type;


            if (VIDC_BUFFER_OUTPUT == buffer_type || VIDC_BUFFER_OUTPUT2 == buffer_type)
            {
                vidc_plane_def_type     planeDef;

                HABMM_MEMSET(&planeDef, 0, sizeof(vidc_plane_def_type));
                planeDef.buf_type       = buffer_type;
                planeDef.plane_index    = 1;    // Y plane
                rc = dec_enc_get_drv_property( fe_ioss,
                    VIDC_I_PLANE_DEF,
                    sizeof(vidc_plane_def_type),
                    ( uint8* )&planeDef );
                if (0 == rc)
                {
                    stride = planeDef.actual_stride;
                    scanlines = planeDef.actual_plane_buf_height;
                }
                else
                {
                    HYP_VIDEO_MSG_ERROR("failed to get drv property - VIDC_I_PLANE_DEF buffer type %d", planeDef.buf_type);
                }
            }
            rc = dec_enc_get_drv_property(fe_ioss,
                VIDC_I_BUFFER_REQUIREMENTS,
                sizeof(buffer_req_data),
                (uint8*)&buffer_req_data);

            // update the fmt information
            if (HYPV_STATUS_SUCCESS != rc)
            {
                HYP_VIDEO_MSG_ERROR("failed to get drv property - VIDC_I_BUFFER_REQUIREMENTS");
            }
            else
            {
                data->fmt.pix_mp.pixelformat = pixel_format;
                data->fmt.pix_mp.num_planes = (unsigned char)num_planes;
                data->fmt.pix_mp.height = frame_size.height;
                data->fmt.pix_mp.width = frame_size.width;
                data->fmt.pix_mp.plane_fmt[0].bytesperline = (unsigned short)stride;
                data->fmt.pix_mp.plane_fmt[0].reserved[0] = (unsigned short)scanlines;
                if (VIDC_BUFFER_INPUT == buffer_type)
                {
                    data->fmt.pix_mp.plane_fmt[0].sizeimage = V4L2FE_ALIGN(MAX_INPUT_BUFFER_SIZE, 4096);
                }
                else
                {
                    enum color_fmts color_format = mapV4l2colorfmt(pixel_format);

                    data->fmt.pix_mp.plane_fmt[0].sizeimage = VENUS_BUFFER_SIZE(color_format, frame_size.width, frame_size.height);
                }
                data->fmt.pix_mp.plane_fmt[1].bytesperline = (unsigned short)0;
                data->fmt.pix_mp.plane_fmt[1].reserved[0] = (unsigned short)0;
                data->fmt.pix_mp.plane_fmt[1].sizeimage = (unsigned int)0;
                if (VIDC_BUFFER_OUTPUT == buffer_type || VIDC_BUFFER_OUTPUT2 == buffer_type)
                {
                    // get the extra data information
                    HABMM_MEMSET(&buffer_req_data, 0, sizeof(vidc_buffer_reqmnts_type));
                    if (TRUE == fe_ioss->io_handle->multi_stream_enable)
                    {
                        buffer_req_data.buf_type = VIDC_BUFFER_METADATA_OUTPUT2;
                    }
                    else
                    {
                        buffer_req_data.buf_type = VIDC_BUFFER_METADATA_OUTPUT;
                    }
                    rc = dec_enc_get_drv_property( fe_ioss,
                        VIDC_I_BUFFER_REQUIREMENTS,
                        sizeof( buffer_req_data ),
                        ( uint8* )&buffer_req_data );

                    if (HYPV_STATUS_SUCCESS == rc)
                    {
                        data->fmt.pix_mp.plane_fmt[1].sizeimage = V4L2FE_ALIGN(buffer_req_data.size,buffer_req_data.align);
                        if (buffer_req_data.size == 0)
                        {
                            //Set Metadata size to default value. This is needed for codec2 unit test.
                            buffer_req_data.size = data->fmt.pix_mp.plane_fmt[1].sizeimage = DEFAULT_METADATA_SIZE;
                        }
                        fe_ioss->output_metabuf_length = buffer_req_data.size;
                    }
                    else
                    {
                        HYP_VIDEO_MSG_ERROR("failed to get drv property - VIDC_I_BUFFER_REQUIREMENTS buf_type=%d", buffer_req_data.buf_type);
                        rc = HYPV_STATUS_SUCCESS;
                    }
                }
            }
            if ((fe_ioss->frame_size.buf_type == 0) ||
                (frame_size.buf_type == VIDC_BUFFER_OUTPUT) ||
                (frame_size.buf_type == VIDC_BUFFER_OUTPUT2))
            {
                fe_ioss->frame_size = frame_size;
            }
        }
    }
    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_dec_reqbufs

@brief  Decoder FE request buffer

@param [in] fe_ioss pointer
@param [in] v4l2_requestbuffers pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_dec_reqbufs(fe_io_session_t* fe_ioss, struct v4l2_requestbuffers* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    vidc_buffer_reqmnts_type buffer_req_data;

    HYP_VIDEO_MSG_INFO("request dec buffer for buf type %u", data->type);
    if (data->count == 0)
    {
        HYP_VIDEO_MSG_HIGH("buffer type %u count is 0", data->type);
        return rc;
    }

    HABMM_MEMSET(&buffer_req_data, 0, sizeof(vidc_buffer_reqmnts_type));
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == data->type)
    {
        if (TRUE == fe_ioss->io_handle->multi_stream_enable)
        {
            buffer_req_data.buf_type = VIDC_BUFFER_OUTPUT2;
        }
        else
        {
            buffer_req_data.buf_type = VIDC_BUFFER_OUTPUT;
        }
    }
    else
    {
        buffer_req_data.buf_type = VIDC_BUFFER_INPUT;
    }

    if (HYPV_STATUS_SUCCESS == rc)
    {
        rc = dec_enc_get_drv_property(fe_ioss,
            VIDC_I_BUFFER_REQUIREMENTS,
            sizeof(buffer_req_data),
            (uint8 *)&buffer_req_data);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to get buf req drv property");
            buffer_req_data.actual_count = data->count;
        }
        else
        {
            HYP_VIDEO_MSG_HIGH("get buf req. buf type %u req count %u min count %u actual count %u max count %u",
                                data->type, data->count, buffer_req_data.min_count,
                                buffer_req_data.actual_count, buffer_req_data.max_count);
            if (VIDC_BUFFER_INPUT ==  buffer_req_data.buf_type)
            {
                if (MIN_INPUT_BUFFER_COUNT_DEC > buffer_req_data.actual_count)
                {
                    buffer_req_data.actual_count = MIN_INPUT_BUFFER_COUNT_DEC;
                }
                if (MIN_INPUT_BUFFER_COUNT_DEC > buffer_req_data.max_count)
                {
                    buffer_req_data.max_count = MIN_INPUT_BUFFER_COUNT_DEC;
                }
                if (MIN_INPUT_BUFFER_COUNT_DEC > data->count)
                {
                    data->count = MIN_INPUT_BUFFER_COUNT_DEC;
                }
                else if ((buffer_req_data.actual_count <= data->count) &&
                         (buffer_req_data.max_count >= data->count))
                {
                    buffer_req_data.actual_count = data->count;
                }
                else
                {
                    data->count = buffer_req_data.actual_count;
                }
                if (fe_ioss->enable_thumbnail_mode)
                {
                   data->count = THUMBNAIL_BUFFER_COUNT;
                   buffer_req_data.actual_count = THUMBNAIL_BUFFER_COUNT;
                }
            }
            else if (VIDC_BUFFER_OUTPUT2 ==  buffer_req_data.buf_type)
            {
                if (MIN_OUTPUT_BUFFER_COUNT_DEC > buffer_req_data.actual_count)
                {
                    buffer_req_data.actual_count = MIN_OUTPUT_BUFFER_COUNT_DEC;
                }
                if (MIN_OUTPUT_BUFFER_COUNT_DEC > buffer_req_data.max_count)
                {
                    buffer_req_data.max_count = MIN_OUTPUT_BUFFER_COUNT_DEC;
                }
                if (MIN_OUTPUT_BUFFER_COUNT_DEC > data->count)
                {
                    data->count = MIN_OUTPUT_BUFFER_COUNT_DEC;
                }
                else if ((buffer_req_data.actual_count <= data->count) &&
                         (buffer_req_data.max_count >= data->count))
                {
                    buffer_req_data.actual_count = data->count;
                }
                else
                {
                    data->count = buffer_req_data.actual_count;
                }
            }
            else
            {
                if (MIN_OUTPUT_BUFFER_COUNT_DEC > buffer_req_data.min_count)
                {
                    buffer_req_data.actual_count = MIN_OUTPUT_BUFFER_COUNT_DEC;
                }
                else
                {
                    buffer_req_data.actual_count = buffer_req_data.min_count;
                }
                if (buffer_req_data.actual_count <= data->count
                    && MAX_OUTPUT_BUFFER_COUNT_DEC >= data->count)
                {
                    buffer_req_data.actual_count = data->count;
                }
                else
                {
                    data->count = buffer_req_data.actual_count;
                }
                if (fe_ioss->enable_thumbnail_mode)
                {
                   data->count = THUMBNAIL_BUFFER_COUNT;
                   buffer_req_data.min_count = THUMBNAIL_BUFFER_COUNT;
                   buffer_req_data.actual_count = THUMBNAIL_BUFFER_COUNT;
                }
            }
        }
        rc = dec_enc_set_drv_property(fe_ioss,
            VIDC_I_BUFFER_REQUIREMENTS,
            sizeof(buffer_req_data),
            (uint8 *)&buffer_req_data);

        if (HYPV_STATUS_SUCCESS == rc)
        {
            HYP_VIDEO_MSG_HIGH("set buffer count to %u for buf type %u requested count %u",
                                buffer_req_data.actual_count, data->type, data->count);

            if (VIDC_BUFFER_INPUT ==  buffer_req_data.buf_type)
            {
                // Assigning buffer count to 64 as Codec2 can send more buffers
                // than the actual count.
                // ToDo: Use actual buffer count once Codec2 uses actaul count instead
                // of MAX_COUNT
                fe_ioss->input_buffer_count = VIDEO_MAX_FRAME;
                fe_ioss->vidc_input_buffer_size = buffer_req_data.size;
            }
            else
            {
                // Assigning buffer count to 64 as Codec2 can send more buffers
                // than the actual count.
                // ToDo: Use actual buffer count once Codec2 uses actaul count instead
                // of MAX_COUNT
                fe_ioss->output_buffer_count = VIDEO_MAX_FRAME;
                fe_ioss->dynmaic_buf_entry_count = VIDEO_MAX_FRAME;
            }
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("failed to set buf req drv property");
        }
    }
    return rc;
}

//
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_OUTPUT_ORDER)
//          => VIDC_IOCTL_SET_PROPERTY (VIDC_I_DEC_OUTPUT_ORDER)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_ALLOC_MODE_OUTPUT,V4L2_MPEG_VIDC_VIDEO_DYNAMIC)
//          => VIDC_IOCTL_SET_PROPERTY (VIDC_I_BUFFER_ALLOC_MODE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_STREAM_FORMAT)
//          => VIDC_IOCTL_SET_PROPERTY(VIDC_I_NAL_STREAM_FORMAT)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_EXTRADATA,MSM_VIDC_EXTRADATA_INTERLACE_VIDEO)
//          => VIDC_IOCTL_SET_PROPERTY(VIDC_I_METADATA_HEADER,VIDC_METADATA_SCAN_FORMAT)
//
// TODO: the following list
// ------------------------
// VIDIOC_S_CTRL(id = V4L2_CID_MPEG_VIDEO_DIVX_FORMAT; value=...4/5/6)
// VIDIOC_S_CTRL (V4L2_CID_MPEG_VIDEO_CONCEAL_COLOR)
// VIDIOC_S_CTRL(id = V4L2_CID_MPEG_VIDEO_MVC_BUFFER_LAYOUT; value=…TOP_BOTTOM)
// VIDIOC_S_CTRL(id = V4L2_CID_MPEG_VIDEO_CONTINUE_DATA_TRANSFER)
// VIDIOC_S_CTRL(id = V4L2_CID_MPEG_SET_PERF_LEVEL, value = ..._TURBO)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_NON_SECURE_OUTPUT2)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_STREAM_OUTPUT_MODE,V4L2_CID_MPEG_VIDEO_STREAM_OUTPUT_SECONDARY)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_KEEP_ASPECT_RATIO)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_BUFFER_SIZE_LIMIT, buffer_size)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_SET_PERF_LEVEL, OMX_QCOM_PerfLevelNominal/_TURBO)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_PRIORITY,V4L2_MPEG_VIDC_VIDEO_PRIORITY_REALTIME_ENABLE/_DISABLE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_OPERATING_RATE, #)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_EXTRADATA,MSM_VIDC_EXTRADATA_FRAME_RATE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_EXTRADATA,MSM_VIDC_EXTRADATA_RECOVERY_POINT_SEI)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_EXTRADATA,MSM_VIDC_EXTRADATA_PANSCAN_WINDOW)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_EXTRADATA,MSM_VIDC_EXTRADATA_ASPECT_RATIO)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_EXTRADATA,MSM_VIDC_EXTRADATA_MPEG2_SEQDISP)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_EXTRADATA,MSM_VIDC_EXTRADATA_TIMESTAMP)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_EXTRADATA,MSM_VIDC_EXTRADATA_S3D_FRAME_PACKING)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_EXTRADATA,MSM_VIDC_EXTRADATA_FRAME_QP)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_EXTRADATA,MSM_VIDC_EXTRADATA_FRAME_BITS_INFO)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_EXTRADATA,MSM_VIDC_EXTRADATA_STREAM_USERDATA)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_EXTRADATA,MSM_VIDC_EXTRADATA_VQZIP_SEI)
//
/**===========================================================================

FUNCTION v4l2fe_dec_s_ctrl

@brief  Decoder FE set control

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_dec_s_ctrl(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    HYP_VIDEO_MSG_INFO("data id %u data value %d", data->id, data->value);
    switch (data->id)
    {
    case V4L2_CID_MPEG_VIDC_VIDEO_DECODE_ORDER:
        {
            vidc_output_order_type vidc_op_order;
            if (V4L2_MPEG_MSM_VIDC_DISABLE == data->value)
            {
                vidc_op_order.output_order = VIDC_DEC_ORDER_DISPLAY;
            }
            else
            {
                vidc_op_order.output_order = VIDC_DEC_ORDER_DECODE;
            }
            rc = dec_enc_set_drv_property(fe_ioss,
                VIDC_I_DEC_OUTPUT_ORDER,
                sizeof(vidc_output_order_type),
                ( uint8* )&vidc_op_order);
            if (HYPV_STATUS_SUCCESS != rc)
            {
                HYP_VIDEO_MSG_ERROR("failed to set drv output order property. order %d",
                                     vidc_op_order.output_order);
            }
        }
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_STREAM_FORMAT:
        {
            vidc_stream_format_type vidc_stream_fmt;
            vidc_stream_format_type nal_cap;

            if (V4L2_MPEG_VIDC_VIDEO_NAL_FORMAT_STARTCODES == data->value)
            {
                vidc_stream_fmt.nal_format = (uint32) VIDC_NAL_FORMAT_STARTCODES;
            }
            else if (V4L2_MPEG_VIDC_VIDEO_NAL_FORMAT_FOUR_BYTE_LENGTH == data->value)
            {
                vidc_stream_fmt.nal_format = (uint32) VIDC_NAL_FORMAT_FOUR_BYTE_LENGTH;
            }
            else
            {
                // default
                vidc_stream_fmt.nal_format = (uint32) VIDC_NAL_FORMAT_STARTCODES;
            }

            // get the supported NAL_STREAM_FORMAT
            rc = dec_enc_get_drv_property(fe_ioss, VIDC_I_CAPABILITY_NAL_STREAM_FORMAT,
                                          sizeof(vidc_stream_format_type), ( uint8* )&nal_cap);
            if (HYPV_STATUS_SUCCESS != rc)
            {
                HYP_VIDEO_MSG_ERROR("failed to get nal stream property for data id %u", data->id);
            }
            else
            {
                HYP_VIDEO_MSG_INFO("supported nal stream format 0x%x", nal_cap.nal_format);
                if (nal_cap.nal_format & vidc_stream_fmt.nal_format)
                {
                    rc = dec_enc_set_drv_property(fe_ioss,
                        VIDC_I_NAL_STREAM_FORMAT,
                        sizeof(vidc_stream_format_type),
                        ( uint8* )&vidc_stream_fmt);
                    if (HYPV_STATUS_SUCCESS != rc)
                    {
                        HYP_VIDEO_MSG_ERROR("failed to set drv property. data id %u nal format %u",
                                             data->id, vidc_stream_fmt.nal_format);
                    }
                }
                else
                {
                    HYP_VIDEO_MSG_HIGH("vidc not supported, this assume pass");
                }
            }
        }
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_EXTRADATA:
        {
            vidc_metadata_header_type meta_header;
            meta_header.enable = TRUE;
            meta_header.port_index = V4L2FE_METADATA_DEFAULT_PORTINDEX_OUTPUT;
            meta_header.version = V4L2FE_METADATA_DEFAULT_VERSION;

            //Set EXTRADATA for EXTRADATA_DEFAULT
            switch (fe_ioss->input_fourCC)
            {
                case V4L2_PIX_FMT_H264:
                case V4L2_PIX_FMT_HEVC:
                {
                     meta_header.client_type = VIDC_METADATA_VUI_DISPLAY_INFO;
                     meta_header.metadata_type = VIDC_METADATA_VUI_DISPLAY_INFO;
                     break;
                }
                case V4L2_PIX_FMT_VP9:
                {
                    //Add macros on FE & BE
                    meta_header.enable = FALSE;
                    break;
                }
                case V4L2_PIX_FMT_MPEG2:
                {
                    meta_header.client_type = VIDC_METADATA_MPEG2_SEQDISP;
                    meta_header.metadata_type = VIDC_METADATA_MPEG2_SEQDISP;
                    break;
                }
                default:
                {
                    meta_header.enable = FALSE;
                    break;
                }
            }

            if (TRUE == meta_header.enable)
            {
                rc = dec_enc_set_drv_property(fe_ioss,
                    VIDC_I_METADATA_HEADER,
                    sizeof(vidc_metadata_header_type),
                    ( uint8* )&meta_header);
                if (HYPV_STATUS_SUCCESS != rc)
                {
                    HYP_VIDEO_MSG_ERROR("failed to set drv property metadata type %u", meta_header.metadata_type);
                }
            }

            meta_header.enable = TRUE;
            meta_header.client_type = VIDC_METADATA_PROPERTY_INDEX_OUTPUT_CROP;
            meta_header.metadata_type = VIDC_METADATA_PROPERTY_INDEX_OUTPUT_CROP;
            rc = dec_enc_set_drv_property(fe_ioss,
                VIDC_I_METADATA_HEADER,
                sizeof(vidc_metadata_header_type),
                ( uint8* )&meta_header);
            if (HYPV_STATUS_SUCCESS != rc)
            {
                HYP_VIDEO_MSG_ERROR("failed to set drv property for metadata type %u", meta_header.metadata_type);
            }

            meta_header.client_type = VIDC_METADATA_SCAN_FORMAT;
            meta_header.metadata_type = VIDC_METADATA_SCAN_FORMAT;
            rc = dec_enc_set_drv_property(fe_ioss,
                VIDC_I_METADATA_HEADER,
                sizeof(vidc_metadata_header_type),
                ( uint8* )&meta_header);
            if (HYPV_STATUS_SUCCESS != rc)
            {
                HYP_VIDEO_MSG_ERROR("failed to set drv property metadata type %u", meta_header.metadata_type);
            }

            meta_header.client_type = VIDC_METADATA_CONCEALMB;
            meta_header.metadata_type = VIDC_METADATA_CONCEALMB;
            rc = dec_enc_set_drv_property(fe_ioss,
                VIDC_I_METADATA_HEADER,
                sizeof(vidc_metadata_header_type),
                ( uint8* )&meta_header);
            if (HYPV_STATUS_SUCCESS != rc)
            {
                HYP_VIDEO_MSG_ERROR("failed to set drv property metadata type %u", meta_header.metadata_type);
            }

            if (V4L2_PIX_FMT_HEVC == fe_ioss->input_fourCC) {
                meta_header.enable = TRUE;
                meta_header.client_type = VIDC_EXTRADATA_MASTERING_DISPLAY_COLOUR_SEI;
                meta_header.metadata_type = VIDC_EXTRADATA_MASTERING_DISPLAY_COLOUR_SEI;
                rc = dec_enc_set_drv_property(fe_ioss,
                    VIDC_I_METADATA_HEADER,
                    sizeof(vidc_metadata_header_type),
                    (uint8* )&meta_header);
                if (HYPV_STATUS_SUCCESS != rc)
                {
                    HYP_VIDEO_MSG_ERROR("failed to set drv property for data id %u metadata type %u",
                                         data->id, meta_header.metadata_type);
                }

                meta_header.client_type = VIDC_EXTRADATA_CONTENT_LIGHT_LEVEL_SEI;
                meta_header.metadata_type = VIDC_EXTRADATA_CONTENT_LIGHT_LEVEL_SEI;
                rc = dec_enc_set_drv_property(fe_ioss,
                    VIDC_I_METADATA_HEADER,
                    sizeof(vidc_metadata_header_type),
                    (uint8* )&meta_header);
                if (HYPV_STATUS_SUCCESS != rc)
                {
                    HYP_VIDEO_MSG_ERROR("failed to set drv property for data id %u metadata type %u",
                                         data->id, meta_header.metadata_type);
                }

                meta_header.client_type = VIDC_METADATA_STREAM_USERDATA;
                meta_header.metadata_type = VIDC_METADATA_STREAM_USERDATA;
                rc = dec_enc_set_drv_property(fe_ioss,
                    VIDC_I_METADATA_HEADER,
                    sizeof(vidc_metadata_header_type),
                    (uint8* )&meta_header);
                if (HYPV_STATUS_SUCCESS != rc)
                {
                    HYP_VIDEO_MSG_ERROR("failed to set drv property for data id %u metadata type %u",
                                         data->id, meta_header.metadata_type);
                }
            }
            //ToDo EXTRADATA_ADVANCED
        }
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_SECURE:
        {   // The real deal is delayed until dec_enc_apply_properties()
            fe_ioss->io_handle->secure = (data->value) ? TRUE : FALSE;
        }
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_LOWLATENCY_MODE:
        {
            rc = dec_enc_s_low_latency_mode(fe_ioss, data);
        }
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_FRAME_RATE:
        {
            rc = dec_s_frame_rate(fe_ioss, data);
        }
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_OPERATING_RATE:
    case V4L2_CID_MPEG_VIDC_VIDEO_PRIORITY:
    case V4L2_CID_MPEG_VIDC_VIDEO_DISABLE_TIMESTAMP_REORDER:
    case V4L2_CID_MPEG_VIDC_VIDEO_BUFFER_SIZE_LIMIT:
        {
            HYP_VIDEO_MSG_INFO("//TODO id %u value %d", data->id, data->value);
            rc = HYPV_STATUS_SUCCESS;
        }
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_SYNC_FRAME_DECODE:
        {
            rc = dec_enc_s_thumbnail_mode(fe_ioss, data);
        }
        break;
    default:
        {
            HYP_VIDEO_MSG_ERROR("unsupported id %u", data->id);
            rc = HYPV_STATUS_FAIL;
        }

    }
    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_dec_s_parm

@brief  Decoder FE set parameter

@param [in] fe_ioss pointer
@param [in] v4l2_streamparm pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_dec_s_parm(fe_io_session_t* fe_ioss, struct v4l2_streamparm* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_frame_rate_type frame_rate;

    HABMM_MEMSET(&frame_rate, 0, sizeof(vidc_frame_rate_type));

    if (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == data->type)
    {
        frame_rate.buf_type = VIDC_BUFFER_INPUT;

        // from spf to fps
        frame_rate.fps_denominator = data->parm.output.timeperframe.numerator;
        frame_rate.fps_numerator = data->parm.output.timeperframe.denominator;
        rc = dec_enc_set_drv_property(fe_ioss,
            VIDC_I_FRAME_RATE,
            sizeof(vidc_frame_rate_type),
            ( uint8* )&frame_rate);
        if (rc == HYPV_STATUS_SUCCESS)
        {
            HYP_VIDEO_MSG_INFO("set frame rate successfully. numerator=%u denominator=%u",
                                frame_rate.fps_numerator, frame_rate.fps_denominator);
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("failed to set frame rate. numerator=%u denominator=%u",
                                 frame_rate.fps_numerator, frame_rate.fps_denominator);
        }
    }
    else
    {
        HYP_VIDEO_MSG_INFO("unsupported v4l2 buf type %u", data->type);
    }
    return rc;
}

// TO do after demo: (not need for demo)
// VIDIOC_G_CTRL(profile_control.id = V4L2_CID_MPEG_VIDEO_H264_PROFILE)
//          => VIDC_I_PROFILE
// VIDIOC_G_CTRL(profile_control.id = V4L2_CID_MPEG_VIDEO_H264_LEVEL)
//          => VIDC_I_LEVEL
// VIDIOC_G_CTRL(V4L2_CID_MPEG_VIDEO_SECURE_SCALING_THRESHOLD)
//          => ?
// VIDIOC_G_CTRL(V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE)
//          => ?
//
/**===========================================================================

FUNCTION v4l2fe_dec_g_ctrl

@brief  Decoder FE get control

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_dec_g_ctrl(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
   vidc_buffer_type buffer_type = VIDC_BUFFER_UNUSED;
   hypv_status_type rc = HYPV_STATUS_SUCCESS;

   HYP_VIDEO_MSG_INFO("get ctrl buf id is %u", data->id);
   if (V4L2_CID_MIN_BUFFERS_FOR_CAPTURE == data->id)
   {
       if (TRUE == fe_ioss->io_handle->multi_stream_enable)
       {
           buffer_type = VIDC_BUFFER_OUTPUT2;
       }
       else
       {
           buffer_type = VIDC_BUFFER_OUTPUT;
       }
   }
   else if (V4L2_CID_MIN_BUFFERS_FOR_OUTPUT == data->id)
   {
       buffer_type = VIDC_BUFFER_INPUT;
   }
   else if (V4L2_CID_MPEG_VIDEO_HEVC_TIER == data->id)
   {
       vidc_tier_type vidc_tier;
       HABMM_MEMSET(&vidc_tier, 0, sizeof(vidc_tier_type));

       rc = dec_enc_get_drv_property(fe_ioss,
                                     VIDC_I_TIER,
                                     sizeof(vidc_tier_type),
                                     (uint8 *)&vidc_tier);
       if (HYPV_STATUS_SUCCESS == rc)
       {
           data->value = dec_enc_convert_vidc_to_v4l2(data->id, vidc_tier.tier);
       }
       else
       {
           rc = HYPV_STATUS_FAIL;
           HYP_VIDEO_MSG_ERROR("failed to get tier %d", rc);
       }
       return rc;
   }
   else
   {
       HYP_VIDEO_MSG_ERROR("unsupported type");
       rc = HYPV_STATUS_FAIL;
   }

   if (HYPV_STATUS_SUCCESS == rc)
   {
      vidc_buffer_reqmnts_type buffer_req_data;

      HABMM_MEMSET(&buffer_req_data, 0, sizeof(vidc_buffer_reqmnts_type));
      buffer_req_data.buf_type = buffer_type;


      rc = dec_enc_get_drv_property(fe_ioss,
          VIDC_I_BUFFER_REQUIREMENTS,
          sizeof(buffer_req_data),
          (uint8*)&buffer_req_data);

      // update the fmt information
      if (HYPV_STATUS_SUCCESS != rc)
      {
          HYP_VIDEO_MSG_ERROR("failed to get buffer requirement for buf type %d", buffer_type);
      }
      else
      {
         if (VIDC_BUFFER_OUTPUT == buffer_type)
         {
            if (fe_ioss->enable_thumbnail_mode)
            {
               data->value = THUMBNAIL_BUFFER_COUNT;
            }
            else
            {
               data->value = ( buffer_req_data.min_count > MIN_OUTPUT_BUFFER_COUNT_DEC )
                              ? buffer_req_data.min_count : MIN_OUTPUT_BUFFER_COUNT_DEC;
            }
         }
         else
         {
            if (fe_ioss->enable_thumbnail_mode)
            {
               data->value = THUMBNAIL_BUFFER_COUNT;
            }
            else
            {
               data->value = ( buffer_req_data.actual_count > buffer_req_data.min_count )
                              ? buffer_req_data.actual_count : buffer_req_data.min_count;
            }
         }
         HYP_VIDEO_MSG_HIGH("get ctrl buffer_type %d count %d",
                            buffer_type, data->value);
      }
   }
   return rc;

}

/**===========================================================================

FUNCTION v4l2fe_decoder_multistream_config

@brief  Configure multistream decode

@param [in] fe_ioss pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_decoder_multistream_config(fe_io_session_t *fe_ioss)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    if (FALSE == fe_ioss->io_handle->multi_stream_enable)
    {
        /* enable multi stream */
        vidc_multi_stream_type  vidc_multi_stream;
        vidc_color_format_config_type vidc_color_format;

        HYP_VIDEO_MSG_HIGH("Enable multi stream");
        vidc_multi_stream.buf_type = VIDC_BUFFER_OUTPUT2;
        vidc_multi_stream.enable = TRUE;
        rc = dec_enc_set_drv_property(fe_ioss, VIDC_I_DEC_MULTI_STREAM,
                                      sizeof(vidc_multi_stream_type),
                                      ( uint8* )&vidc_multi_stream);
        vidc_multi_stream.buf_type = VIDC_BUFFER_OUTPUT;
        vidc_multi_stream.enable = FALSE;
        rc = dec_enc_set_drv_property(fe_ioss, VIDC_I_DEC_MULTI_STREAM,
                                      sizeof(vidc_multi_stream_type),
                                      ( uint8* )&vidc_multi_stream);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("error enabling multi stream %d", rc);
        }
        else
        {
            /* set property VIDC_I_COLOR_FORMAT on output */
            fe_ioss->io_handle->multi_stream_enable = TRUE;

            vidc_color_format.buf_type = VIDC_BUFFER_OUTPUT;
            if (MSM_VIDC_BIT_DEPTH_8 == fe_ioss->io_handle->bit_depth)
            {
                vidc_color_format.color_format = VIDC_COLOR_FORMAT_NV12_UBWC;
            }
            else if (MSM_VIDC_BIT_DEPTH_10 == fe_ioss->io_handle->bit_depth)
            {
                vidc_color_format.color_format = VIDC_COLOR_FORMAT_YUV420_TP10_UBWC;
            }
            else
            {
                vidc_color_format.color_format = VIDC_COLOR_FORMAT_NV12_UBWC;
            }

            HYP_VIDEO_MSG_HIGH("set dpb color format to %d",
                                vidc_color_format.color_format);
            rc = dec_enc_set_drv_property(fe_ioss, VIDC_I_COLOR_FORMAT,
                                          sizeof(vidc_color_format_config_type),
                                          (uint8 *)&vidc_color_format);
            if (HYPV_STATUS_SUCCESS != rc)
            {
                HYP_VIDEO_MSG_ERROR("error setting dpb color format to %d",
                                     vidc_color_format.color_format);
            }
            else
            {
                vidc_frame_size_type frame_size;
                frame_size.buf_type = VIDC_BUFFER_OUTPUT2;
                frame_size.height = fe_ioss->frame_size.height;
                frame_size.width =  fe_ioss->frame_size.width;
                rc = dec_enc_set_drv_property(fe_ioss,
                                              VIDC_I_FRAME_SIZE,
                                              sizeof(vidc_frame_size_type),
                                              (uint8 *)&frame_size);
                if (HYPV_STATUS_SUCCESS != rc)
                {
                    HYP_VIDEO_MSG_ERROR("error setting opb frame size");
                }
            }
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_dec_querymenu

@brief  Decoder FE query menu

@param [in] v4l2_querymenu pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_dec_querymenu(struct v4l2_querymenu* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    unsigned long idx = 0;

    switch (data->id)
    {
        case V4L2_CID_MPEG_VIDEO_H264_PROFILE:
        case V4L2_CID_MPEG_VIDEO_HEVC_PROFILE:
        case V4L2_CID_MPEG_VIDEO_MPEG2_PROFILE:
        case V4L2_CID_MPEG_VIDEO_VP9_PROFILE:
        case V4L2_CID_MPEG_VIDEO_VP8_PROFILE:
        case V4L2_CID_MPEG_VIDC_VIDEO_MPEG2_PROFILE:
        case V4L2_CID_MPEG_VIDEO_HEVC_TIER:
        {
            rc = dec_enc_query_menu(vdec_ctrls, ARRAY_SIZE(vdec_ctrls), data);
            break;
        }
        case V4L2_CID_MPEG_VIDEO_H264_LEVEL:
        case V4L2_CID_MPEG_VIDEO_HEVC_LEVEL:
        case V4L2_CID_MPEG_VIDEO_MPEG2_LEVEL:
        case V4L2_CID_MPEG_VIDC_VIDEO_VP9_LEVEL:
        case V4L2_CID_MPEG_VIDC_VIDEO_VP8_PROFILE_LEVEL:
        case V4L2_CID_MPEG_VIDC_VIDEO_MPEG2_LEVEL:
        case V4L2_CID_MPEG_VIDEO_MPEG4_LEVEL:
        {
             for (idx = 0; idx < ARRAY_SIZE(vdec_ctrls); idx++)
             {
                 if (vdec_ctrls[idx].id == data->id)
                 {
                    if ((uint)vdec_ctrls[idx].maximum < data->index)
                    {
                       rc = HYPV_STATUS_FAIL;
                    }
                    break;
                 }
             }
             break;
        }
        default:
            HYP_VIDEO_MSG_ERROR("unsupported id %u", data->id);
            rc = HYPV_STATUS_FAIL;
            break;
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_dec_queryctrl

@brief  Decoder FE query control

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_dec_queryctrl(fe_io_session_t *fe_ioss, struct v4l2_queryctrl* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    switch (data->id)
    {
        case V4L2_CID_MPEG_VIDEO_H264_PROFILE:
        case V4L2_CID_MPEG_VIDEO_HEVC_PROFILE:
        case V4L2_CID_MPEG_VIDEO_MPEG2_PROFILE:
        case V4L2_CID_MPEG_VIDEO_VP9_PROFILE:
        case V4L2_CID_MPEG_VIDEO_VP8_PROFILE:
        case V4L2_CID_MPEG_VIDC_VIDEO_MPEG2_PROFILE:
        case V4L2_CID_MPEG_VIDEO_HEVC_TIER:
        {
            rc = dec_enc_query_profile(fe_ioss, vdec_ctrls, ARRAY_SIZE(vdec_ctrls), data);
            break;
        }
        case V4L2_CID_MPEG_VIDEO_H264_LEVEL:
        case V4L2_CID_MPEG_VIDEO_HEVC_LEVEL:
        case V4L2_CID_MPEG_VIDEO_MPEG2_LEVEL:
        case V4L2_CID_MPEG_VIDC_VIDEO_VP9_LEVEL:
        case V4L2_CID_MPEG_VIDC_VIDEO_VP8_PROFILE_LEVEL:
        case V4L2_CID_MPEG_VIDC_VIDEO_MPEG2_LEVEL:
        case V4L2_CID_MPEG_VIDEO_MPEG4_LEVEL:
        {
            rc = dec_enc_query_level(fe_ioss, vdec_ctrls, ARRAY_SIZE(vdec_ctrls), data);
            break;
        }
        default:
            HYP_VIDEO_MSG_ERROR("unsupported id %u", data->id);
            rc = HYPV_STATUS_FAIL;
            break;
    }

    return rc;
}

hypv_status_type v4l2fe_dec_enum_fmt(fe_io_session_t* fe_ioss, struct v4l2_fmtdesc* data)
{
    (void)fe_ioss;
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    const v4l2fe_vidc_format *fmt = NULL;

    if (NULL == data)
    {
        HYP_VIDEO_MSG_ERROR("Invalid fmt descriptor");
        rc = HYPV_STATUS_FAIL;
    }

    if (HYPV_STATUS_SUCCESS == rc)
    {
        if (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == data->type)
        {
            data->flags = V4L2_FMT_FLAG_COMPRESSED;
        }

        fmt = dec_enc_get_pixel_fmt_index(vdec_formats,
              ARRAY_SIZE(vdec_formats), data->index, data->type);
        if (fmt)
        {
            HABMM_MEMCPY(data->description, fmt->description, sizeof(data->description));
            data->pixelformat = fmt->fourcc;
        }
        else
        {
            HYP_VIDEO_MSG_INFO("No more formats found");
            rc = HYPV_STATUS_FAIL;
        }
    }

    return rc;
}

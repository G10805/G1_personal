/*========================================================================

*//** @file linux_venc_fe.cpp

@par DESCRIPTION:
Linux video encoder hypervisor front-end implementation

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
04/29/25    su     Report ENOMEM error code when setting alloc_mode is failed
04/03/25    su     Fix state machine of FE in reconfigure
10/23/24    ms     Add support for sending HDR10plus dynamic info
20/05/24    vz     Enable intra refresh cyclic mode
02/25/24    pc     Add support for tier for HEVC
08/30/23    nb     Fix compilation errors due to additon of new compiler flags
06/22/23    mm     Add flush lock to avoid race condition issue
05/24/23    pc     Add HDR10 static metadata support for HEVC encode
03/16/23    sd     Support encoder output metadata to get average frame QP
11/02/22    sk     Use flag and change header path for kernel 5.15
09/26/22    mm     Remove dependency OMX
08/30/22    nb     Initialize variables to fix MISRA issues
08/22/22    nb     Add support to query Max supported B frames
08/03/22    nb     Set H264 entropy coding property during streamon
07/30/22    nb     Refine configuration of Intra refresh property
07/30/22    nb     Update P-frames setting during streamon & remove B-frames setting
07/30/22    nb     Refine configuration of layer encode properties
07/30/22    nb     Refine setting frame QP range property
07/30/22    nb     Set frame QP during streamon
07/30/22    nb     Set rate_control proeperty during streamon
05/26/22    ll     Enable bitrate saving mode
05/13/22    sd     Add support to handle HEVC tier setting
05/04/22    nd     Fix KW issues
03/28/22    sh     Enable Layer Encode for GVM
03/30/22    sd     Add support for blur filter
03/25/22    sh     Enable LTR support with Codec2
02/14/22    sj     Synchronize lock handling
01/28/22    sj     Handle unsupported ctrl & reduce loglevel
01/14/22    nb     Add secure property setting after set format
10/20/21    mm     Fix rotation 90/270 issue for OMX case
10/05/21    rq     postpone setting slice mode until streamon and fix MISRA error
09/24/21    nb     Make first frame in every GOP as IDR frame for HEVC & AVC Encode
07/29/21    mm     Postpone setting of intra refresh and clean up
07/21/21    mm     Align the usage of intra refresh random mode across SIs
06/22/21    sj     Handle setting frame size for 90/270 deg rotation case in set fmt
06/21/21    sj     Update plane constraints for NV12_512 fmt to plane for HEIC
05/25/21    sj     Fix compilation errors on linux due to C99 standard
05/06/21    sj     Add support to query supported profiles/levels
04/01/21    sh     Bringup video on RedBend Hypervisor
03/20/21    sj     Set dynamic buffer mode in v4l2fe_s_fmt
03/20/21    sj     Handle odd input frame width and height for encoder
03/18/21    rq     Call back only once for the flush command with FLASH_ALL type
03/16/21    jz     Add support for disabling rate control
02/08/21    mm     Support HEIC profile and remove useless code
02/01/21    rq     Delay setting secure mode until right before streamon
01/22/21    mm     Handle nonsupport of RC_ENABLE/HEVC_SIZE_OF_LENGTH_FIELD setting
11/03/20    hl     Add Long Term Reference Support
10/29/20    sh     Enable dynamic frame rate configuration on output buffer
10/19/20    sh     Use VIDEO_MAX_FRAME instead of actual count to store buffer entries
10/16/20    xg     Fix flip both for newer call sequence of flip in codec2
10/12/20    sh     Enable CSC for H264/HEVC encoder
10/10/20    sh     Reorder color formats in order to set NV12_UBWC as first format in Codec2
10/09/20    jz     Flip changes for newer version of V4L2 API
09/24/20    mm     Enable video encoder since omx upgrade
09/24/20    jz     Add support for cyclic intra refresh
09/22/20    mm     Fix compiling issue on LV
09/22/20    sh     Update input crop metadata macro
08/27/20    sh     Bringup video decode using codec2
08/12/20    sh     Update Linux FE with kernel 5.4 macros
07/28/20    sh     Update input buffer size in REQBUFS IOCTL
07/23/20    sh     Add STOP & RELEASE_RESOURCE IOCTLs to maintain proper driver & FW states
05/27/20    sh     Add CS section to synchronize get_property call on driver
04/21/20    sj     Add support for HEIC/HEIF encoding
03/17/20    sm     Add multislice query controls
01/09/20    sm     Support for secure playback
11/18/19    sm     Extend rate control for encode usecase
11/04/19    sm     Enable secure video usecases
10/16/19    sm     Fix EOS buffer leakage
10/15/19    sm     Support flip and rotation simultaneously
08/23/19    sm     Enable extradata flag only when an extradata is set
08/23/19    sm     Add support for ROI QP
07/24/19    sm     Add conditional compilation for crop
07/18/19    sm     Alloc mode not invoked in execute state
07/09/19    sm     Add additional supported color format
06/27/19    sm     Add support for input crop
06/24/19    sm     Add support for flip, rotation and cleanups
06/17/19    sm     Enable intra refresh mode
04/11/19    sm     Align hypervisor event handling with native drivers
04/09/19    sm     Handle encoder buffer reference in dynamic mode
03/14/19    sm     Handle stop cmd for encoder
03/04/19    sm     Add dynamic buffer mode for encode usecase
03/04/19    sm     Add support for new v4l2 API
02/15/19    rz     Rename buffer req to buffer count
02/05/19    rz     Bringup changes for 8155
11/06/18    sh     Populate data_offset field in v4l2_plane
11/01/18    sm     Make use of a common function to map color formats
09/14/18    sh     Extract buffer address when index is passed during EOS
08/21/18    aw     Fix Klockwork P1, compilation and MISRA warning
06/29/18    sm     Initialize session codec type in context structure
06/12/18    sm     Add sequence header retrieval in FE
05/24/18    sm     Add support for more input color formats for encoder
05/17/18    sm     Bypass AU delimiter setting
04/12/18    aw     Fix color format issue for video encoder
04/11/18    sm     Fix timestamp calculation
04/02/18    sm     Make use of a common function to convert v4l2 to vidc codec
10/12/17    sm     Simplify buffer management logics for dynamic alloc mode
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
#include "linux_venc_fe.h"
#include "linux_vdec_venc_common.h"
#include "MMCriticalSection.h"
#ifdef WIN32
#include "types.h"
#include "msm_vidc_dec.h"
#include "VideoPlatform.h"
#include "VideoComDef.h"
#include "msm_vidc_enc.h"
#include "msm_media_info.h"
#else
#include <linux/videodev2.h>
#ifdef SUPPORT_DMABUF
#include <vidc/media/msm_media_info.h>
#else
#include <media/msm_media_info.h>
#endif
#endif
#include "hyp_debug.h"

#define MIN_HIER_LAYER_COUNT (2)
#define NUM_OF_MB(width,height)  \
  ((((width) + 15) >> 4) * (((height) + 15) >> 4))

/*
 * Custom conversion coefficients for resolution: 176x144 negative
 * coeffs are converted to s4.9 format
 * (e.g. -22 converted to ((1 << 13) - 22)
 * 3x3 transformation matrix coefficients in s4.9 fixed point format
 */
uint32 vpe_csc_custom_matrix_coeff[VIDC_MAX_MATRIX_COEFFS] = {
440, 8140, 8098, 0, 460, 52, 0, 34, 463
};

/* offset coefficients in s9 fixed point format */
uint32 vpe_csc_custom_bias_coeff[VIDC_MAX_BIAS_COEFFS] = {
53, 0, 4
};

/* clamping value for Y/U/V([min,max] for Y/U/V) */
uint32 vpe_csc_custom_limit_coeff[VIDC_MAX_LIMIT_COEFFS] = {
16, 235, 16, 240, 16, 240
};

v4l2fe_vidc_format venc_formats[] = {
    {
#ifdef _LINUX_
        {.name = "YCbCr Semiplanar 4:2:0"},
        {.description = "Y/CbCr 4:2:0"},
#else
        .name = "YCbCr Semiplanar 4:2:0",
        .description = "Y/CbCr 4:2:0",
#endif
        .fourcc = V4L2_PIX_FMT_NV12,
        .type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
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
        .type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
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
        .type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
        .get_frame_size = NULL,
        .defer_outputs = false,
        .input_min_count = 0,
        .output_min_count = 0,
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
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
        .get_frame_size = NULL,
        .defer_outputs = false,
        .input_min_count = 4,
        .output_min_count = 4,
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
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
        .get_frame_size = NULL,
        .defer_outputs = false,
        .input_min_count = 4,
        .output_min_count = 4,
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
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
        .get_frame_size = NULL,
        .defer_outputs = false,
        .input_min_count = 4,
        .output_min_count = 4,
    },
    {
#ifdef _LINUX_
        {.name = "YCrCb Semiplanar 4:2:0"},
        {.description = "Y/CrCb 4:2:0"},
#else
        .name = "YCrCb Semiplanar 4:2:0",
        .description = "Y/CrCb 4:2:0",
#endif
        .fourcc = V4L2_PIX_FMT_NV21,
        .type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
        .get_frame_size = NULL,
        .defer_outputs = false,
        .input_min_count = 0,
        .output_min_count = 0,
    },
    {
#ifdef _LINUX_
        {.name = "TP10 UBWC 4:2:0"},
        {.description = "TP10 UBWC 4:2:0"},
#else
        .name = "TP10 UBWC 4:2:0",
        .description = "TP10 UBWC 4:2:0",
#endif
        .fourcc = V4L2_PIX_FMT_NV12_TP10_UBWC,
        .type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
        .get_frame_size = NULL,
        .defer_outputs = false,
        .input_min_count = 0,
        .output_min_count = 0,
    },
    /*{
        .name = "TME",
        .description = "TME MBI format",
        .fourcc = V4L2_PIX_FMT_TME,
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
        .input_min_count = 4,
        .output_min_count = 4,
    },*/
    {
#ifdef _LINUX_
        {.name = "YCbCr Semiplanar 4:2:0 10bit"},
        {.description = "Y/CbCr 4:2:0 10bit"},
#else
        .name = "YCbCr Semiplanar 4:2:0 10bit",
        .description = "Y/CbCr 4:2:0 10bit",
#endif
        .fourcc = V4L2_PIX_FMT_SDE_Y_CBCR_H2V2_P010_VENUS,
        .type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
        .get_frame_size = NULL,
        .defer_outputs = false,
        .input_min_count = 0,
        .output_min_count = 0,
    },
    {
#ifdef _LINUX_
        {.name = "YCbCr Semiplanar 4:2:0 512 aligned"},
        {.description = "Y/CbCr 4:2:0 512 aligned"},
#else
        .name = "YCbCr Semiplanar 4:2:0 512 aligned",
        .description = "Y/CbCr 4:2:0 512 aligned",
#endif
        .fourcc = V4L2_PIX_FMT_NV12_512,
        .type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
        .get_frame_size = NULL,
        .defer_outputs = false,
        .input_min_count = 0,
        .output_min_count = 0,
    },
};

v4l2fe_vidc_ctrl venc_ctrls[] = {
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

    {
        .id = V4L2_CID_MPEG_VIDEO_VIDC_INTRA_REFRESH_TYPE,
#ifdef _LINUX_
        {.name = "Intra Refresh Type"},
#else
        .name = "Intra Refresh Type",
#endif
        .default_value = V4L2_MPEG_VIDEO_VIDC_INTRA_REFRESH_RANDOM,
        .minimum = V4L2_MPEG_VIDEO_VIDC_INTRA_REFRESH_RANDOM,
        .maximum = V4L2_MPEG_VIDEO_VIDC_INTRA_REFRESH_CYCLIC,
        .level = 0,
        .menu_skip_mask = 0,
        .qmenu = NULL,
    },

};

/**===========================================================================

FUNCTION device_callback_enc_passthrough

@brief  Hypervisor encode FE callback function

@param [in] msg pointer
@param [in] lenght
@param [in] void pointer for private data

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type device_callback_enc_passthrough(uint8 *msg, uint32 length, void *cd)
{
    fe_io_session_t *fe_ioss = (fe_io_session_t *)cd;
    fe_linux_plt_data_t *plt_data = (fe_linux_plt_data_t *)(fe_ioss->fe_plt_data);
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
            HYP_VIDEO_MSG_ERROR("FBD: index = %u frame fd = %u flag = 0x%x tv_sec = %ld tv_usec = %ld"
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
            HYP_VIDEO_MSG_ERROR("EBD: index = %u frame fd = %u flag = 0x%x tv_sec = %ld tv_usec = %ld"
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
            HYP_VIDEO_MSG_ERROR("v4l2_event_data->type %llu", v4l2_event_data->type);
            hyp_enqueue(&plt_data->evt_queue, (void *)&event);
            fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &flags);
            break;
        }
    }

    return HYPV_STATUS_SUCCESS;
}
/**===========================================================================

FUNCTION device_callback_enc

@brief  Hypervisor encode FE callback function

@param [in] msg pointer
@param [in] lenght
@param [in] void pointer for private data

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type device_callback_enc(uint8 *msg, uint32 length, void *cd)
{
    vidc_drv_msg_info_type *pEvent = (vidc_drv_msg_info_type *)msg;
    fe_io_session_t *fe_ioss = (fe_io_session_t *)cd;
    fe_linux_plt_data_t *plt_data = (fe_linux_plt_data_t *)(fe_ioss->fe_plt_data);
    uint32 i = 0;
    vidc_frame_data_type* frame_data;
    long sec, usec;
    hypv_session_t* hypv_session = fe_ioss->io_handle;
    uint32 flags = 0;
    struct v4l2_event event;
    event_buf_type event_buf;

    UNUSED(length);

    HABMM_MEMSET(&event, 0, sizeof(struct v4l2_event));
    HABMM_MEMSET(&event_buf, 0, sizeof(event_buf_type));

    if (TRUE == fe_ioss->io_handle->passthrough_mode)
    {
        return device_callback_enc_passthrough(msg, length, cd);
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
    case VIDC_EVT_RESP_INPUT_DONE:
        {
            if (V4L2FE_STATE_PAUSE == plt_data->state)
            {
               plt_data->state = V4L2FE_STATE_EXECUTING;
            }
            frame_data = &pEvent->payload.frame_data;
            if (VIDC_ERR_HW_FATAL == pEvent->status)
            {
                HYP_VIDEO_MSG_ERROR("h/w fatal error. pHeader 0x%llx pBuffer %p",
                                     pEvent->payload.frame_data.frm_clnt_data,
                                     pEvent->payload.frame_data.frame_addr );
            }
            else if (VIDC_ERR_SESSION_PICTURE_DROPPED == pEvent->status)
            {
                HYP_VIDEO_MSG_ERROR("picture dropped" );
            }
            else if (fe_ioss->eos_buffer.data_fd == (int32)(uintptr_t)frame_data->frame_addr)
            {
                /* This is EOS buffer allocated internally by FE
                 * Skip returning to client
                 */
                dec_enc_free_eos_buffer(fe_ioss);
                HYP_VIDEO_MSG_INFO("Free EOS buffer fd %lu",
                                    (unsigned long)frame_data->frame_addr);
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
                    for (i = 0; i < fe_ioss->input_buffer_count; i++)
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

            MM_CriticalSection_Enter(fe_ioss->lock_buffer);

            frame_data = &pEvent->payload.frame_data;
            dec_enc_rm_dyn_buf_ref(fe_ioss, frame_data);

            hypv_session->fbd_count++;

            //ToDo: Remove this condition if encode works on 6155.
            //With fbd_count = 1, two copies of CSD buffer is sent to Codec2.
            //Due to this QuTest is skipping first IDR frame while writing to file
            if ((0 == hypv_session->fbd_count) &&
                (VIDC_CODEC_VP8 != hypv_session->codec) &&
                (VIDC_CODEC_H263 != hypv_session->codec))
            {
                uint8 payload[MAX_SEQ_HEADER_LEN + sizeof(vidc_seq_hdr_64b_type)];
                vidc_seq_hdr_64b_type *pHeader = (vidc_seq_hdr_64b_type* )payload;

                HABMM_MEMSET(payload, 0, sizeof(payload));

                pHeader->seq_hdr = ((unsigned long) payload + sizeof(vidc_seq_hdr_64b_type));
                pHeader->seq_hdr_len = MAX_SEQ_HEADER_LEN;
                hypv_status_type rc = HYPV_STATUS_SUCCESS;

                rc = dec_enc_get_drv_property(fe_ioss, VIDC_I_SEQUENCE_HEADER,
                                               sizeof(vidc_seq_hdr_64b_type) + MAX_SEQ_HEADER_LEN,
                                               payload);
                if (HYPV_STATUS_SUCCESS == rc)
                {
                    HYP_VIDEO_MSG_INFO("sequence header len = %u", pHeader->seq_hdr_len);
                    pHeader->seq_hdr = ((unsigned long)payload + sizeof(vidc_seq_hdr_64b_type));

                    uint8 *frame_addr = (uint8*)mmap(NULL, hypv_session->first_frame_info.alloc_len,
                                 PROT_READ|PROT_WRITE, MAP_SHARED,
                                 (int32)(uintptr_t) hypv_session->first_frame_info.frame_addr, 0);
                    if (MAP_FAILED == frame_data->frame_addr)
                    {
                        HYP_VIDEO_MSG_ERROR("mmap failed. errno: %d fd: %d size %u",
                                             errno, (int32)(uintptr_t) hypv_session->first_frame_info.frame_addr,
                                             hypv_session->first_frame_info.alloc_len);
                    }
                    else
                    {
                        /* copy sequence header to the first buffer */
                        HABMM_MEMCPY(frame_addr, (void *)pHeader->seq_hdr, pHeader->seq_hdr_len);
                        if (-1 == munmap(frame_addr, hypv_session->first_frame_info.alloc_len))
                        {
                            HYP_VIDEO_MSG_ERROR("munmap %p failed len %u",
                                                 frame_addr,
                                                 hypv_session->first_frame_info.alloc_len);
                        }
                    }
                    dec_enc_rm_dyn_buf_ref(fe_ioss, &hypv_session->first_frame_info);
                    event_buf.buf.flags = V4L2_BUF_FLAG_CODECCONFIG;
                    event_buf.buf_planes[0].m.userptr = (unsigned long)hypv_session->first_frame_info.frame_addr;
                    event_buf.buf_planes[1].m.userptr = 0;
                    event_buf.buf_planes[0].length = frame_data->alloc_len;
                    event_buf.buf_planes[1].length = 0;
                    event_buf.buf_planes[0].bytesused = pHeader->seq_hdr_len;
                    event_buf.buf_planes[0].reserved[MSM_VIDC_DATA_OFFSET] = 0;
                    event_buf.buf_planes[0].reserved[MSM_VIDC_INPUT_TAG_1] = frame_data->input_tag;
                    event_buf.buf.timestamp.tv_sec = 0;
                    event_buf.buf.timestamp.tv_usec = 0;
                    event_buf.buf.length = NUM_CAPTURE_BUFFER_PLANE_ENC;
                    event_buf.buf.index = (unsigned int)hypv_session->first_frame_info.frm_clnt_data;
                    HYP_VIDEO_MSG_HIGH("FBD: First fbd. flag = 0x%x index = %u frame_addr = 0x%lx",
                                        event_buf.buf.flags, event_buf.buf.index,
                                        event_buf.buf_planes[0].m.userptr);

                    hyp_enqueue(&plt_data->evt_output_buf_queue, (void *)&event_buf);
                    HABMM_MEMSET(&event_buf, 0, sizeof(event_buf_type));
                    event_buf.buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
                    event_buf.buf.memory = V4L2_MEMORY_USERPTR;
                    flags = POLLIN;
                    fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &flags);
                }
                else
                {
                    HYP_VIDEO_MSG_ERROR("failed to get sequence header");
                    event.type = V4L2_EVENT_MSM_VIDC_SYS_ERROR;
                    hyp_enqueue(&plt_data->evt_queue, (void *)&event);
                    flags = POLLPRI;
                    fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &flags);
                }
            }

            event_buf.buf.flags = 0;
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
            if (frame_data->flags & VIDC_FRAME_FLAG_ENDOFSUBFRAME)
            {
                event_buf.buf.flags |= V4L2_BUF_FLAG_END_OF_SUBFRAME;
            }
            if (frame_data->flags & VIDC_FRAME_FLAG_DATACORRUPT)
            {
                event_buf.buf.flags |= V4L2_BUF_FLAG_DATA_CORRUPT;
            }
            if (frame_data->flags & VIDC_FRAME_FLAG_READONLY)
            {
                event_buf.buf.flags |= V4L2_BUF_FLAG_READONLY;
            }
            event_buf.buf_planes[0].m.userptr = (unsigned long)frame_data->frame_addr;
            event_buf.buf_planes[1].m.userptr = (unsigned long)frame_data->metadata_addr;
            event_buf.buf_planes[0].length = frame_data->alloc_len + fe_ioss->output_metabuf_length;
            event_buf.buf_planes[1].length = fe_ioss->output_metabuf_length;
            event_buf.buf_planes[0].bytesused = frame_data->data_len;
            event_buf.buf_planes[1].bytesused = frame_data->alloc_metadata_len;
            event_buf.buf_planes[0].reserved[MSM_VIDC_DATA_OFFSET] = frame_data->offset;
            event_buf.buf_planes[0].data_offset = frame_data->offset;
            event_buf.buf_planes[0].reserved[MSM_VIDC_INPUT_TAG_1] = frame_data->input_tag;
            sec = (long) V4L2FE_CONVERT_USEC_TO_SEC(frame_data->timestamp);
            usec = (long) (frame_data->timestamp - (int64) V4L2FE_CONVERT_SEC_TO_USEC(sec));
            event_buf.buf.timestamp.tv_sec = sec;
            event_buf.buf.timestamp.tv_usec = usec;
            event_buf.buf.length = NUM_CAPTURE_BUFFER_PLANE_ENC;
            event_buf.buf.index = (unsigned int)frame_data->frm_clnt_data;
            HYP_VIDEO_MSG_HIGH("FBD: flag = 0x%x tv_sec = %ld tv_usec = %ld"
                               " index = %u frame_addr = 0x%lx inputTag %lu",
                                event_buf.buf.flags, event_buf.buf.timestamp.tv_sec,
                                event_buf.buf.timestamp.tv_usec, event_buf.buf.index,
                                event_buf.buf_planes[0].m.userptr, frame_data->input_tag);

            MM_CriticalSection_Leave(fe_ioss->lock_buffer);

            hyp_enqueue(&plt_data->evt_output_buf_queue, (void *)&event_buf);
            flags = POLLIN;
            fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &flags);
            break;
        }
    case VIDC_EVT_INFO_OUTPUT_RECONFIG:
        {
            // smooth streaming case - just info resolution change
            // the crop information should be reflected in the output buffer done
            HYP_VIDEO_MSG_ERROR("smooth streaming resolution change");
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
                MM_Signal_Set(fe_ioss->state_synch_obj);
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
    default:
        {
            HYP_VIDEO_MSG_ERROR( "Unknown Event 0x%x", (unsigned int)pEvent->event_type);
            break;
        }
    }
    return HYPV_STATUS_SUCCESS;
}

/**===========================================================================

FUNCTION enc_map_input_colorfmt

@brief  Convert color format type from v4l2 to hypervisor format type

@param [in] v4l2colorfmt

@dependencies
  None

@return
  Returns vidc_color_format_type

===========================================================================*/
static vidc_color_format_type enc_map_input_colorfmt(uint32 v4l2colorfmt)
{
    vidc_color_format_type vidc_colorfmt = VIDC_COLOR_FORMAT_UNUSED;

    switch(v4l2colorfmt)
    {
    case V4L2_PIX_FMT_NV12:
    case V4L2_PIX_FMT_NV12_128:
    case V4L2_PIX_FMT_NV12_512:
        vidc_colorfmt = VIDC_COLOR_FORMAT_NV12;
        break;
    case V4L2_PIX_FMT_NV12_UBWC:
        vidc_colorfmt = VIDC_COLOR_FORMAT_NV12_UBWC;
        break;
    case V4L2_PIX_FMT_NV21:
        vidc_colorfmt = VIDC_COLOR_FORMAT_NV21;
        break;
    case V4L2_PIX_FMT_RGB32:
        vidc_colorfmt = VIDC_COLOR_FORMAT_RGBA8888;
        break;
    case V4L2_PIX_FMT_RGBA8888_UBWC:
        vidc_colorfmt = VIDC_COLOR_FORMAT_RGBA8888_UBWC;
        break;
    case V4L2_PIX_FMT_SDE_Y_CBCR_H2V2_P010_VENUS:
        vidc_colorfmt = VIDC_COLOR_FORMAT_NV12_P010;
        break;
    case V4L2_PIX_FMT_NV12_TP10_UBWC:
        vidc_colorfmt = VIDC_COLOR_FORMAT_YUV420_TP10_UBWC;
        break;
    default:
        vidc_colorfmt = VIDC_COLOR_FORMAT_UNUSED;
    }

    return vidc_colorfmt;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_fmt

@brief  Encoder FE set format

@param [in] fe_ioss pointer
@param [in] v4l2_format pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_enc_s_fmt(fe_io_session_t* fe_ioss, struct v4l2_format* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_frame_size_type frame_size;
    fe_linux_plt_data_t *linux_plt_data = (fe_linux_plt_data_t *)(fe_ioss->fe_plt_data);
    enum color_fmts color_format;

    frame_size.height = data->fmt.pix_mp.height;
    frame_size.width = data->fmt.pix_mp.width;

    HYP_VIDEO_MSG_INFO("buf type 0x%x", data->type);
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == data->type)
    {
        HYP_VIDEO_MSG_HIGH("width %u height %u color format 0x%x",
                            data->fmt.pix_mp.width, data->fmt.pix_mp.height,
                            data->fmt.pix_mp.pixelformat);
        frame_size.buf_type = VIDC_BUFFER_OUTPUT;
        if (0 == fe_ioss->output_fourCC)
        {
            vidc_session_codec_type session_codec;
            session_codec.session = fe_ioss->session_codec.session;
            session_codec.codec = codecV4l2ToVidc(data->fmt.pix_mp.pixelformat);
            if (VIDC_CODEC_UNUSED ==  session_codec.codec)
            {
                HYP_VIDEO_MSG_ERROR("unsupported codec 0x%x",
                                     data->fmt.pix_mp.pixelformat);
                rc = HYPV_STATUS_FAIL;
            }
            else
            {
                // set the session_codec
                rc = dec_enc_set_drv_property(fe_ioss,
                                              VIDC_I_SESSION_CODEC,
                                              sizeof(vidc_session_codec_type),
                                              ( uint8* )&session_codec );
                if (0 == rc)
                {
                    fe_ioss->output_fourCC = data->fmt.pix_mp.pixelformat;
                    fe_ioss->session_codec = session_codec;
                    fe_ioss->io_handle->codec = session_codec.codec;
                }
                else
                {
                    HYP_VIDEO_MSG_ERROR("failed to set drv session codec property."
                                        " session 0x%x codec 0x%x",
                                         (unsigned int)session_codec.session, session_codec.codec);
                }
            }
        }
        /* set dynamic buffer mode as default */
        HYP_VIDEO_MSG_HIGH("Set dynamic buffer mode for buf type %d", VIDC_BUFFER_OUTPUT);
        rc = dec_enc_set_buf_alloc_mode(fe_ioss, VIDC_BUFFER_MODE_DYNAMIC, VIDC_BUFFER_OUTPUT);
        if (HYPV_STATUS_SUCCESS != rc)
        {
             if (HYPV_STATUS_ALLOC_FAIL == rc)
             {
                errno = ENOMEM;
                HYP_VIDEO_MSG_ERROR("NO_MEMORY error reported in alloc mode rc 0x%x", rc);
             }
             else
             {
                HYP_VIDEO_MSG_ERROR("set dynamic alloc mode failed for buf type %d", VIDC_BUFFER_OUTPUT);
             }
        }
    }
    else if (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == data->type)
    {
        HYP_VIDEO_MSG_INFO("pixel format 0x%x", data->fmt.pix_mp.pixelformat);
        frame_size.buf_type = VIDC_BUFFER_INPUT;
        fe_ioss->input_fourCC = data->fmt.pix_mp.pixelformat ? data->fmt.pix_mp.pixelformat : V4L2_PIX_FMT_NV12_UBWC;

        if (0 != fe_ioss->session_codec.codec)
        {
            vidc_color_format_config_type color_format_config;
            color_format_config.buf_type = VIDC_BUFFER_INPUT;
            color_format = mapV4l2colorfmt(fe_ioss->input_fourCC);
            color_format_config.color_format = enc_map_input_colorfmt(fe_ioss->input_fourCC);
            if (VIDC_COLOR_FORMAT_UNUSED == color_format_config.color_format)
            {
                HYP_VIDEO_MSG_ERROR("unsupported color format 0x%x",
                                     data->fmt.pix_mp.pixelformat);
                rc = HYPV_STATUS_FAIL;
            }
            else
            {
                rc = dec_enc_set_drv_property(fe_ioss,
                                              VIDC_I_COLOR_FORMAT,
                                              sizeof(vidc_color_format_config_type),
                                              ( uint8* )&color_format_config);
                if (HYPV_STATUS_SUCCESS == rc)
                {
                    fe_ioss->color_format_config = color_format_config;
                    fe_ioss->input_fourCC = data->fmt.pix_mp.pixelformat;

                    /* Always use COLOR_FMT_NV12_512 for HEIC encode */
                    /* Update the plane constraints for NV12_512 color
                     * format to firmware
                    */
                    if (COLOR_FMT_NV12_512 == color_format)
                    {
                       vidc_plane_def_type plane_def;

                       HABMM_MEMSET(&plane_def, 0, sizeof(vidc_plane_def_type));

                       plane_def.buf_type = VIDC_BUFFER_INPUT;
                       plane_def.plane_index = 1;    // Y plane
                       rc = dec_enc_get_drv_property(fe_ioss,
                                                     VIDC_I_PLANE_DEF,
                                                     sizeof(vidc_plane_def_type),
                                                     ( uint8* )&plane_def );

                       plane_def.stride_multiples = 512;
                       plane_def.min_plane_buf_height_multiple = 512;
                       plane_def.buf_alignment = 512;
                       plane_def.actual_stride = VENUS_Y_STRIDE(COLOR_FMT_NV12_512, frame_size.width);
                       plane_def.actual_plane_buf_height = VENUS_Y_SCANLINES(COLOR_FMT_NV12_512, frame_size.height);
                       plane_def.min_stride = plane_def.actual_stride;
                       plane_def.min_plane_buf_height = plane_def.actual_plane_buf_height;

                       rc = dec_enc_set_drv_property(fe_ioss,
                                                     VIDC_I_PLANE_DEF,
                                                     sizeof(vidc_plane_def_type),
                                                     ( uint8* )&plane_def );

                       if (HYPV_STATUS_SUCCESS == rc)
                       {
                          HYP_VIDEO_MSG_ERROR("[HEIC] Buffer alignment for Y plane set");

                          plane_def.buf_type = VIDC_BUFFER_INPUT;
                          plane_def.plane_index = 2;    // UV plane

                          plane_def.stride_multiples = 512;
                          plane_def.min_plane_buf_height_multiple = 256;
                          plane_def.buf_alignment = 256;
                          plane_def.actual_stride = VENUS_UV_STRIDE(COLOR_FMT_NV12_512, frame_size.width);
                          plane_def.actual_plane_buf_height = VENUS_UV_SCANLINES(COLOR_FMT_NV12_512, frame_size.height);
                          plane_def.min_stride = plane_def.actual_stride;
                          plane_def.min_plane_buf_height = plane_def.actual_plane_buf_height;

                          rc = dec_enc_set_drv_property(fe_ioss,
                                                        VIDC_I_PLANE_DEF,
                                                        sizeof(vidc_plane_def_type),
                                                        ( uint8* )&plane_def );

                          if (HYPV_STATUS_SUCCESS == rc)
                          {
                             HYP_VIDEO_MSG_ERROR("[HEIC] Buffer alignment for UV plane set");
                          }
                          else
                          {
                             HYP_VIDEO_MSG_ERROR("[HEIC] Failed to set buffer alignment for UV plane");
                          }
                       }
                       else
                       {
                          HYP_VIDEO_MSG_ERROR("[HEIC] Failed to set buffer alignment for Y plane");
                       }
                    }
                }
                else
                {
                    HYP_VIDEO_MSG_ERROR("failed to set drv color format property");
                }
            }
        }
        /* set dynamic buffer mode as default */
        HYP_VIDEO_MSG_HIGH("Set dynamic buffer mode for buf type %d", VIDC_BUFFER_INPUT);
        rc = dec_enc_set_buf_alloc_mode(fe_ioss, VIDC_BUFFER_MODE_DYNAMIC, VIDC_BUFFER_INPUT);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("set dynamic alloc mode failed for buf type %d", VIDC_BUFFER_INPUT);
        }
    }
    else
    {
        HYP_VIDEO_MSG_ERROR("invalid mplane type %u", data->type);
        rc = HYPV_STATUS_FAIL;
    }

    if (HYPV_STATUS_SUCCESS == rc)
    {
        if (linux_plt_data->state == V4L2FE_STATE_EXECUTING)
        {
            HYP_VIDEO_MSG_ERROR("error calling set frame size in executing state");
        }
        else
        {
            if (!(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == data->type) ||
                 !(VIDC_ROTATE_90 == fe_ioss->io_handle->rotate ||
                   VIDC_ROTATE_270 == fe_ioss->io_handle->rotate))
            {
                if (!(0x1 & frame_size.width) && !(0x1 & frame_size.height))
                {
                      rc = dec_enc_set_drv_property(fe_ioss,
                                          VIDC_I_FRAME_SIZE,
                                          sizeof(vidc_frame_size_type),
                                          ( uint8* )&frame_size );
                      if (HYPV_STATUS_SUCCESS != rc)
                      {
                          HYP_VIDEO_MSG_ERROR("failed to set drv frame size property."
                                    " buf type %d width %u height %u rotate %d",
                                    frame_size.buf_type, frame_size.width,
                                    frame_size.height, fe_ioss->io_handle->rotate);
                      }
                      else
                      {
                          HYP_VIDEO_MSG_HIGH("set drv frame size property."
                                    " buf type %d width %u height %u rotate %d",
                                    frame_size.buf_type, frame_size.width,
                                    frame_size.height, fe_ioss->io_handle->rotate);
                          fe_ioss->frame_size = frame_size;
                      }
                }
                else
                {
                     HYP_VIDEO_MSG_ERROR("Odd width x height %ux%u passed for pixel format 0x%x",
                                          frame_size.width,
                                          frame_size.height,
                                          data->fmt.pix_mp.pixelformat);
                     rc = HYPV_STATUS_FAIL;
                }
            }
            else if ((V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == data->type) &&
                     (VIDC_ROTATE_90 == fe_ioss->io_handle->rotate ||
                      VIDC_ROTATE_270 == fe_ioss->io_handle->rotate))
            {
               vidc_frame_size_type flipped_frame_size;

               flipped_frame_size.buf_type = VIDC_BUFFER_OUTPUT;
               flipped_frame_size.width = fe_ioss->frame_size.height;
               flipped_frame_size.height = fe_ioss->frame_size.width;

               rc = dec_enc_set_drv_property(fe_ioss,
                                             VIDC_I_FRAME_SIZE,
                                             sizeof(vidc_frame_size_type),
                                             ( uint8* )&flipped_frame_size );
               if (HYPV_STATUS_SUCCESS != rc)
               {
                   HYP_VIDEO_MSG_ERROR("failed to flip resolution buf type %d width %u height %u",
                                        flipped_frame_size.buf_type, flipped_frame_size.width,
                                        flipped_frame_size.height);
               }
               else
               {
                   HYP_VIDEO_MSG_INFO("set drv frame size property buf type %d width %u height %u",
                                       flipped_frame_size.buf_type, flipped_frame_size.width,
                                       flipped_frame_size.height);
               }
            }
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

FUNCTION get_frame_size_compressed

@brief  Get compressed frame size

@param [in] width
@param [in] height

@dependencies
  None

@return
  Returns size of frame(unsigned int)

===========================================================================*/
static unsigned int get_frame_size_compressed(unsigned int width, unsigned int height)
{
    int sz = V4L2FE_ALIGN(height, 32) * V4L2FE_ALIGN(width, 32) * 3 / 2;
    return V4L2FE_ALIGN(sz, 4096);
}

/**===========================================================================

FUNCTION v4l2fe_enc_g_fmt

@brief  Encoder FE get format

@param [in] fe_ioss pointer
@param [in] v4l2_format pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_enc_g_fmt(fe_io_session_t* fe_ioss, struct v4l2_format* data)
{
    vidc_buffer_type buffer_type;
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    uint32 pixel_format = 0;
    uint32 num_planes = 0;
    uint32 binary_format = 0;

    HYP_VIDEO_MSG_INFO("buf type %u", data->type);
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == data->type)
    {
        buffer_type = VIDC_BUFFER_OUTPUT;
        pixel_format = fe_ioss->output_fourCC;
        num_planes = NUM_CAPTURE_BUFFER_PLANE_ENC;
        binary_format = 1;
    }
    else if (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == data->type)
    {
        buffer_type = VIDC_BUFFER_INPUT;
        pixel_format = fe_ioss->input_fourCC;
        num_planes = NUM_OUTPUT_BUFFER_PLANE_ENC;
        binary_format = 0;
    }
    else
    {
        HYP_VIDEO_MSG_ERROR("unsupported buf type %u", data->type);
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
            HYP_VIDEO_MSG_ERROR("failed to get drv frame size property. buf type %d",
                                 frame_size.buf_type);
        }
        else
        {
            uint32 stride = frame_size.width;
            uint32 scanlines = frame_size.height;

            if ( 0 == binary_format)
            {
                vidc_plane_def_type plane_def;

                HABMM_MEMSET(&plane_def, 0, sizeof(vidc_plane_def_type));
                plane_def.buf_type = buffer_type;
                plane_def.plane_index = 1;    // Y plane
                rc = dec_enc_get_drv_property(fe_ioss,
                                              VIDC_I_PLANE_DEF,
                                              sizeof(vidc_plane_def_type),
                                              ( uint8* )&plane_def );
                if (HYPV_STATUS_SUCCESS == rc)
                {
                    stride = plane_def.actual_stride;
                    scanlines = plane_def.actual_plane_buf_height;
                }
                else
                {
                    HYP_VIDEO_MSG_ERROR("failed to get drv plane def property. buf type %d",
                                         plane_def.buf_type);
                }
            }

            vidc_buffer_reqmnts_type buffer_req_data;

            HABMM_MEMSET(&buffer_req_data, 0, sizeof(vidc_buffer_reqmnts_type));
            buffer_req_data.buf_type = buffer_type;

            rc = dec_enc_get_drv_property(fe_ioss,
                                          VIDC_I_BUFFER_REQUIREMENTS,
                                          sizeof( buffer_req_data ),
                                          (uint8*)&buffer_req_data );

            // update the fmt information
            if (HYPV_STATUS_SUCCESS != rc)
            {
                HYP_VIDEO_MSG_ERROR("failed to get drv buf requirement property. buf type %d",
                                     buffer_req_data.buf_type);
            }
            else
            {
                unsigned int linux_bufsize = 0;
                unsigned int vidc_bufsize = buffer_req_data.size;

                if (VIDC_BUFFER_OUTPUT == buffer_type)
                {
                    linux_bufsize = get_frame_size_compressed(frame_size.width, frame_size.height);
                }
                else
                {
                    enum color_fmts color_format = mapV4l2colorfmt(pixel_format);

                    linux_bufsize = VENUS_BUFFER_SIZE(color_format, frame_size.width, frame_size.height);
                }
                buffer_req_data.size = MAX(vidc_bufsize, linux_bufsize);
                HYP_VIDEO_MSG_INFO("buf type %d size %u linux buffsize = %u",
                                    buffer_type, vidc_bufsize, linux_bufsize);
                data->fmt.pix_mp.pixelformat = pixel_format;
                data->fmt.pix_mp.num_planes = (unsigned char)num_planes;
                data->fmt.pix_mp.height = frame_size.height;
                data->fmt.pix_mp.width = frame_size.width;
                data->fmt.pix_mp.plane_fmt[0].bytesperline = (unsigned short)stride;
                data->fmt.pix_mp.plane_fmt[0].reserved[0] = (unsigned short)scanlines;
                data->fmt.pix_mp.plane_fmt[0].sizeimage = V4L2FE_ALIGN((unsigned int)buffer_req_data.size, 4096);
                data->fmt.pix_mp.plane_fmt[1].bytesperline = (unsigned short)0;
                data->fmt.pix_mp.plane_fmt[1].reserved[0] = (unsigned short)0;
                data->fmt.pix_mp.plane_fmt[1].sizeimage = (unsigned int)0;

                /* get the extra data information */
                HABMM_MEMSET(&buffer_req_data, 0, sizeof(vidc_buffer_reqmnts_type));
                if (VIDC_BUFFER_INPUT == buffer_type)
                {
                    buffer_req_data.buf_type = VIDC_BUFFER_METADATA_INPUT;
                }
                else if (VIDC_BUFFER_OUTPUT == buffer_type)
                {
                    buffer_req_data.buf_type = VIDC_BUFFER_METADATA_OUTPUT;
                }

                rc = dec_enc_get_drv_property(fe_ioss,
                                              VIDC_I_BUFFER_REQUIREMENTS,
                                              sizeof( buffer_req_data ),
                                              ( uint8* )&buffer_req_data );

                if (HYPV_STATUS_SUCCESS == rc)
                {
                    data->fmt.pix_mp.plane_fmt[1].sizeimage = V4L2FE_ALIGN(buffer_req_data.size,
                                                                           buffer_req_data.align);
                    if (!data->fmt.pix_mp.plane_fmt[1].sizeimage)
                    {
                        data->fmt.pix_mp.plane_fmt[1].sizeimage = MAX_EXTRADATA_SIZE;
                    }
                    if (VIDC_BUFFER_OUTPUT == buffer_type)
                    {
                        fe_ioss->output_metabuf_length = data->fmt.pix_mp.plane_fmt[1].sizeimage;
                    }
                    HYP_VIDEO_MSG_INFO("buf type %d extradata bufsize %u", buffer_type,
                                         data->fmt.pix_mp.plane_fmt[1].sizeimage);
                }
                else
                {
                    HYP_VIDEO_MSG_ERROR("failed to get drv extradata buf requirement property. buf type %d",
                                         buffer_req_data.buf_type);
                    rc = HYPV_STATUS_SUCCESS;
                }
            }
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_reqbufs

@brief  Encoder FE request buffer

@param [in] fe_ioss pointer
@param [in] v4l2_requestbuffers pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_enc_reqbufs(fe_io_session_t* fe_ioss, struct v4l2_requestbuffers* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    uint32 existing_actual_count = 0;
    vidc_buffer_reqmnts_type buffer_req_data;
    vidc_buffer_type buf_type = VIDC_BUFFER_INPUT;

    if (data->count == 0)
    {
        HYP_VIDEO_MSG_INFO("reqbufs, buffer type %u count is 0", data->type);
        return rc;
    }
    HABMM_MEMSET(&buffer_req_data, 0, sizeof(vidc_buffer_reqmnts_type));

    HYP_VIDEO_MSG_INFO("reqbufs for type %u", data->type);
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == data->type)
    {
        buf_type = VIDC_BUFFER_OUTPUT;
    }
    else
    {
        buf_type = VIDC_BUFFER_INPUT;
    }
    buffer_req_data.buf_type = buf_type;
    rc = dec_enc_get_drv_property(fe_ioss,
                                  VIDC_I_BUFFER_REQUIREMENTS,
                                  sizeof( buffer_req_data ),
                                  ( uint8* )&buffer_req_data );
    existing_actual_count = buffer_req_data.actual_count;
    if (0 != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to get drv buf requirement property. buf type %d",
                             buffer_req_data.buf_type);
        buffer_req_data.actual_count = data->count;
    }
    else
    {
        if (buffer_req_data.min_count <= data->count)
        {
            buffer_req_data.actual_count = data->count;
        }
        else
        {
            data->count = buffer_req_data.min_count;
            buffer_req_data.actual_count = data->count;
        }
    }
    if (MIN_BUFFER_COUNT_ENC > buffer_req_data.actual_count)
    {
        buffer_req_data.actual_count = MIN_BUFFER_COUNT_ENC;
        data->count = MIN_BUFFER_COUNT_ENC;
    }
    if (existing_actual_count != buffer_req_data.actual_count)
    {
        rc = dec_enc_set_drv_property(fe_ioss,
                                      VIDC_I_BUFFER_REQUIREMENTS,
                                      sizeof( buffer_req_data ),
                                      ( uint8* )&buffer_req_data );
    }
    if (HYPV_STATUS_SUCCESS == rc)
    {
        HYP_VIDEO_MSG_INFO("set reqbufs buf type %d count = %u",
                            buffer_req_data.buf_type, buffer_req_data.actual_count);
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
        HYP_VIDEO_MSG_ERROR("reqbufs buf type %d failed, req %u, actual %u",
                             buffer_req_data.buf_type, data->count,
                             buffer_req_data.actual_count);
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_bitrate

@brief  Encoder FE set bitrate

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_bitrate(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_target_bitrate_type payload;

    HYP_VIDEO_MSG_INFO("bitrate %d", data->value);
    if (data->value > 0)
    {
        payload.target_bitrate = data->value;
        rc = dec_enc_set_drv_property(fe_ioss,
                                      VIDC_I_TARGET_BITRATE,
                                      sizeof(vidc_target_bitrate_type),
                                      ( uint8* )&payload);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to set drv target bitrate property %u",
                                 payload.target_bitrate);
        }
    }
    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_rate_control

@brief  Encoder FE set rate control

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_rate_control(fe_io_session_t *fe_ioss,
                                                  struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    fe_ioss->rate_control_type.rate_control = VIDC_RATE_CONTROL_UNUSED;

    HYP_VIDEO_MSG_INFO("rate control id: %u, value: %d", data->id, data->value);

    if (V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE == data->id)
    {
        //only effective when frontend wants to disable rate control
        if (!data->value)
        {
            fe_ioss->rate_control_type.rate_control = VIDC_RATE_CONTROL_OFF;
        }
    }
    else if (V4L2_CID_MPEG_VIDEO_BITRATE_MODE == data->id)
    {
        switch(data->value)
        {
            case V4L2_MPEG_VIDEO_BITRATE_MODE_VBR:
                fe_ioss->rate_control_type.rate_control = VIDC_RATE_CONTROL_VBR_CFR;
                break;
            case V4L2_MPEG_VIDEO_BITRATE_MODE_CBR_VFR:
                fe_ioss->rate_control_type.rate_control = VIDC_RATE_CONTROL_CBR_VFR;
                break;
            case V4L2_MPEG_VIDEO_BITRATE_MODE_CBR:
                fe_ioss->rate_control_type.rate_control = VIDC_RATE_CONTROL_CBR_CFR;
                break;
            case V4L2_MPEG_VIDEO_BITRATE_MODE_MBR:
                fe_ioss->rate_control_type.rate_control = VIDC_RATE_CONTROL_MBR_CFR;
                break;
            case V4L2_MPEG_VIDEO_BITRATE_MODE_MBR_VFR:
                fe_ioss->rate_control_type.rate_control = VIDC_RATE_CONTROL_MBR_VFR;
                break;
            case V4L2_MPEG_VIDEO_BITRATE_MODE_CQ:
                fe_ioss->rate_control_type.rate_control = VIDC_RATE_CONTROL_CQ;
                break;
            default:
                rc = HYPV_STATUS_FAIL;
                HYP_VIDEO_MSG_ERROR("unsupported rate control %d", data->value);
        }
    }
    else
    {
        rc = HYPV_STATUS_FAIL;
        HYP_VIDEO_MSG_ERROR("unsupported rate control id %u", data->id);
    }

    return rc;
}

/**===========================================================================

FUNCTION enc_s_rate_control

@brief  Encoder FE set rate control

@param [in] fe_ioss pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type enc_s_rate_control(fe_io_session_t *fe_ioss)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_rate_control_type payload = fe_ioss->rate_control_type;

    if ((TRUE == fe_ioss->enable_rate_control) &&
        (VIDC_RATE_CONTROL_UNUSED != payload.rate_control))
    {
        rc = dec_enc_set_drv_property(fe_ioss,
                                        VIDC_I_ENC_RATE_CONTROL,
                                        sizeof(vidc_rate_control_type),
                                        ( uint8* )&payload);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to set drv rate control property %d",
                                    payload.rate_control);
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION enc_s_num_p_frames

@brief  Encoder FE set number of p frames

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type enc_s_num_p_frames(fe_io_session_t *fe_ioss)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_iperiod_type payload;

    HYP_VIDEO_MSG_INFO("set P- %u B- %u frames", fe_ioss->enc_iperiod.p_frames,
                                        fe_ioss->enc_iperiod.b_frames);

    payload.p_frames = fe_ioss->enc_iperiod.p_frames;
    payload.b_frames = fe_ioss->enc_iperiod.b_frames;

    rc = dec_enc_set_drv_property(fe_ioss,
                                  VIDC_I_ENC_INTRA_PERIOD,
                                  sizeof(vidc_iperiod_type),
                                 ( uint8* )&payload);
    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to set drv intra period property p_frames %u b_frames %u",
                             payload.p_frames, payload.b_frames);
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_request_seq_header

@brief  Encoder FE set sequence header

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_request_seq_header(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_enable_type payload;
    boolean b_submit = TRUE;

    payload.enable = FALSE;

    if (V4L2_CID_MPEG_VIDEO_PREPEND_SPSPPS_TO_IDR == data->id)
    {
        HYP_VIDEO_MSG_INFO("video header mode is %d", data->value);
        if (V4L2_MPEG_MSM_VIDC_DISABLE == data->value)
        {
            payload.enable = FALSE;
        }
        else if (V4L2_MPEG_MSM_VIDC_ENABLE == data->value)
        {
            payload.enable = TRUE;
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("unsupported header mode of %d", data->value);
            b_submit = FALSE;
        }
    }

    if (b_submit)
    {
        rc = dec_enc_set_drv_property(fe_ioss,
                                      VIDC_I_ENC_SYNC_FRAME_SEQ_HDR,
                                      sizeof(vidc_enable_type),
                                      ( uint8* )&payload);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to set drv sync frame seq hdr property enable %d",
                                 payload.enable);
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_frame_qp

@brief  Encoder FE set frame QP

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_frame_qp(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    HYP_VIDEO_MSG_INFO("data id %u data value %d", data->id, data->value);
    switch (data->id)
    {
        case V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP:
        case V4L2_CID_MPEG_VIDEO_MPEG4_I_FRAME_QP:
        case V4L2_CID_MPEG_VIDEO_VPX_I_FRAME_QP:
        case V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_QP:
        {
            fe_ioss->enc_qp.i_frame_qp = data->value;
            break;
        }

        case V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP:
        case V4L2_CID_MPEG_VIDEO_MPEG4_P_FRAME_QP:
        case V4L2_CID_MPEG_VIDEO_VPX_P_FRAME_QP:
        case V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_QP:
        {
            fe_ioss->enc_qp.p_frame_qp = data->value;
            break;
        }

        case V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP:
        case V4L2_CID_MPEG_VIDEO_MPEG4_B_FRAME_QP:
        case V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_QP:
        {
            fe_ioss->enc_qp.b_frame_qp = data->value;
            break;
        }

        default:
        {
            HYP_VIDEO_MSG_ERROR("Unsupported QP: value %d id %u", data->value, data->id);
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION enc_s_frame_qp

@brief  Encoder FE set frame QP for I,P & B frames

@param [in] fe_ioss pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type enc_s_frame_qp(fe_io_session_t *fe_ioss)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_session_qp_type payload = fe_ioss->enc_qp;

    if (TRUE == fe_ioss->enable_frame_qp)
    {
        rc = dec_enc_set_drv_property(fe_ioss,
                                    VIDC_I_ENC_SESSION_QP,
                                    sizeof(vidc_session_qp_type),
                                    ( uint8* )&payload);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to set drv qp property iframe_qp %u pframe_qp %u bframe_qp %u",
                                payload.i_frame_qp, payload.p_frame_qp, payload.b_frame_qp);
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_qp_range

@brief  Encoder FE set max QP & min QP for I,P & B frames

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_qp_range(fe_io_session_t *fe_ioss)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_session_qp_range_type payload = fe_ioss->enc_qp_range;

    HYP_VIDEO_MSG_INFO("MAX_QP - %u & MIN_QP - %u range", payload.qp_max_packed, payload.qp_min_packed);
    /* QP range applicable for all layer frames */
    payload.layerID = VIDC_ALL_LAYER_ID;

    rc = dec_enc_set_drv_property(fe_ioss,
                                  VIDC_I_ENC_SESSION_QP_RANGE,
                                  sizeof(vidc_session_qp_range_type),
                                  ( uint8* )&payload);
    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to set drv qp range property %x-%x",
                             payload.qp_min_packed, payload.qp_max_packed);
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_profile

@brief  Encoder FE set profile

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_profile(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_profile_type payload;
    int32 profile = 0;

    HYP_VIDEO_MSG_INFO("set profile to %d", data->value);
    payload.profile = VIDC_PROFILE_H264_UNUSED;
    profile = dec_enc_convert_v4l2_to_vidc(data->id, data->value);
    if (profile >= 0)
    {
        payload.profile = profile;
        rc = dec_enc_set_drv_property(fe_ioss,
                                      VIDC_I_PROFILE,
                                      sizeof(vidc_profile_type),
                                      ( uint8* )&payload);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to set drv profile property 0x%x",
                                 payload.profile);
        }
    }
    else
    {
        rc = HYPV_STATUS_FAIL;
        HYP_VIDEO_MSG_ERROR("unsupported id %u profile 0x%x",
                             data->id, (unsigned int)data->value);
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_level

@brief  Encoder FE set level

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_level(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_level_type payload;
    int32 level = 0;

    HYP_VIDEO_MSG_INFO("set level to %d", data->value);
    payload.level = VIDC_LEVEL_H264_UNUSED;
    level = dec_enc_convert_v4l2_to_vidc(data->id, data->value);
    if (level >= 0)
    {
        payload.level = level;
        rc = dec_enc_set_drv_property(fe_ioss,
                                      VIDC_I_LEVEL,
                                      sizeof(vidc_level_type),
                                      ( uint8* )&payload);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to set drv level  property. level 0x%x",
                                 payload.level);
        }
    }
    else
    {
        rc = HYPV_STATUS_FAIL;
        HYP_VIDEO_MSG_ERROR("unsupported id %u level 0x%x",
                             data->id, (unsigned int)data->value);
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_tier

@brief  Encoder FE set tier

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_tier(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_tier_type payload;
    int32 tier = -1;

    HYP_VIDEO_MSG_INFO("set tier to %d", data->value);
    tier = dec_enc_convert_v4l2_to_vidc(data->id,data->value);

    if (0 <= tier)
    {
        HABMM_MEMSET(&payload, 0, sizeof(vidc_tier_type));

        payload.tier = tier;
        rc = dec_enc_set_drv_property(fe_ioss,
                                      VIDC_I_TIER,
                                      sizeof(vidc_tier_type),
                                      (uint8 *)&payload);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to set drv tier property. tier 0x%x", payload.tier);
        }
    }
    else
    {
        rc = HYPV_STATUS_FAIL;
        HYP_VIDEO_MSG_ERROR("unsupported id %u tier 0x%x",
                                data->id, (unsigned int)data->value);
    }

    return rc;
}

/**===========================================================================

FUNCTION enc_s_h264_entropy_mode

@brief  Encoder FE set H264 entropy mode

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type enc_s_h264_entropy_mode(fe_io_session_t *fe_ioss)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_entropy_control_type payload;

    payload.entropy_mode = VIDC_ENTROPY_UNUSED;

    if (TRUE == fe_ioss->enable_entropy_mode)
    {
        HYP_VIDEO_MSG_INFO("entropy mode %d", fe_ioss->enc_entropy_info.value);

        if (V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC == fe_ioss->enc_entropy_info.value)
        {
            payload.entropy_mode = VIDC_ENTROPY_MODE_CAVLC;
            payload.cabac_model = VIDC_CABAC_MODEL_UNUSED;
        }
        else if (V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CABAC == fe_ioss->enc_entropy_info.value)
        {
            /* If entroy_mode is CABAC, supported model is VIDC_CABAC_MODEL_NUMBER_2*/
            payload.entropy_mode = VIDC_ENTROPY_MODE_CABAC;
            payload.cabac_model = VIDC_CABAC_MODEL_NUMBER_2;
        }

        if (VIDC_ENTROPY_UNUSED != payload.entropy_mode)
        {
            rc = dec_enc_set_drv_property(fe_ioss,
                                        VIDC_I_ENC_H264_ENTROPY_CTRL,
                                        sizeof(vidc_entropy_control_type),
                                        ( uint8* )&payload);
            if (HYPV_STATUS_SUCCESS != rc)
            {
                HYP_VIDEO_MSG_ERROR("failed to set drv entropy mode property."
                                    "  entropy_mode 0x%x cabac_model 0x%x",
                                    (unsigned int)payload.entropy_mode, (unsigned int)payload.cabac_model);
            }
            else
            {
                fe_ioss->enc_entropy = payload;
            }
        }
        else
        {
            rc = HYPV_STATUS_FAIL;
            HYP_VIDEO_MSG_ERROR("unsupported entropy mode %d", fe_ioss->enc_entropy_info.value);
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_multi_slice

@brief  Encoder FE saves multi slice parameters

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_multi_slice(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_multi_slice_type payload;

    payload.slice_mode = VIDC_MULTI_SLICE_UNUSED;
    payload.slice_size = 0;

    HYP_VIDEO_MSG_INFO("data id %u data value 0x%x", data->id, (unsigned int)data->value);
    if (V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE == data->id)
    {
        if (V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_SINGLE == data->value)
            payload.slice_mode = VIDC_MULTI_SLICE_OFF;
        else if (V4L2_MPEG_VIDEO_MULTI_SICE_MODE_MAX_MB == data->value)
            payload.slice_mode = VIDC_MULTI_SLICE_BY_MB_COUNT;
        else if (V4L2_MPEG_VIDEO_MULTI_SICE_MODE_MAX_BYTES == data->value)
            payload.slice_mode = VIDC_MULTI_SLICE_BY_BYTE_COUNT;
    }
    else if (V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB == data->id)
    {
        payload.slice_mode = VIDC_MULTI_SLICE_BY_MB_COUNT;
        payload.slice_size = data->value;
    }
    else if (V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_BYTES == data->id)
    {
        payload.slice_mode = VIDC_MULTI_SLICE_BY_BYTE_COUNT;
        payload.slice_size = data->value;
    }

    fe_ioss->enc_multi_slice_type.slice_mode = payload.slice_mode;
    fe_ioss->enc_multi_slice_type.slice_size = payload.slice_size;

    if (VIDC_MULTI_SLICE_UNUSED == payload.slice_mode)
    {
        rc = HYPV_STATUS_FAIL;
        HYP_VIDEO_MSG_ERROR("unsupported slice mode %d", payload.slice_mode);
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_loop_filter

@brief  Encoder FE set loop filter

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_loop_filter(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_db_control_type payload;
    boolean b_submit = FALSE;

    payload = fe_ioss->enc_db_control;
    if (0 == payload.db_mode)
    {
        payload.db_mode = VIDC_DB_UNUSED;
    }

    HYP_VIDEO_MSG_INFO("data id %u data value 0x%x", data->id, (unsigned int)data->value);
    if (V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE == data->id)
    {
        if (V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_DISABLED == data->value)
        {
            payload.db_mode = VIDC_DB_DISABLE;
            b_submit = TRUE;
        }
        else if (V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_DISABLED_AT_SLICE_BOUNDARY == data->value)
        {
            payload.db_mode = VIDC_DB_SKIP_SLICE_BOUNDARY;
            b_submit = TRUE;
        }
        else if (V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_ENABLED  == data->value)
        {
            payload.db_mode = VIDC_DB_ALL_BLOCKING_BOUNDARY;
            b_submit = TRUE;
        }
    }
    else if (V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_ALPHA == data->id)
    {
        payload.slice_alpha_offset = data->value;
        b_submit = TRUE;
    }
    else if (V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_BETA == data->id)
    {
        payload.slice_beta_offset = data->value;
        b_submit = TRUE;
    }
    if (VIDC_DB_UNUSED != payload.db_mode)
    {
        if (b_submit)
        {
            rc = dec_enc_set_drv_property(fe_ioss,
                                          VIDC_I_ENC_H264_DEBLOCKING,
                                          sizeof(vidc_db_control_type),
                                          ( uint8* )&payload);
            if (HYPV_STATUS_SUCCESS != rc)
            {
                HYP_VIDEO_MSG_ERROR("failed to set drv deblocking  property."
                                    " mode 0x%x alpha %d beta %d",
                                    (unsigned int)payload.db_mode, payload.slice_alpha_offset,
                                    payload.slice_beta_offset);
            }
        }
        if (HYPV_STATUS_SUCCESS == rc)
        {
            fe_ioss->enc_db_control = payload;
        }
    }
    else
    {
        rc = HYPV_STATUS_FAIL;
        HYP_VIDEO_MSG_ERROR("id 0x%x, value %d unsupported", data->id, data->value);
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_request_iframe

@brief  Encoder FE request iframe

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_request_iframe(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_enable_type payload;

    HYP_VIDEO_MSG_INFO("request iframe %d", data->value);
    if (0 == data->value)
        payload.enable = FALSE;
    else
        payload.enable = TRUE;
    rc = dec_enc_set_drv_property(fe_ioss,
                                  VIDC_I_ENC_REQUEST_SYNC_FRAME,
                                  sizeof(vidc_enable_type),
                                  ( uint8* )&payload);
    if (HYPV_STATUS_SUCCESS != rc)
    {
        rc = HYPV_STATUS_FAIL;
        HYP_VIDEO_MSG_ERROR("failed to set drv sync frame property. enable %d",
                             payload.enable);
    }
    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_flip

@brief  Encoder FE set flip

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_flip(fe_io_session_t *fe_ioss,
                                          struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_spatial_transform_type payload;

    HYP_VIDEO_MSG_INFO("set flip %u", data->id);
    HABMM_MEMSET(&payload, 0, sizeof(vidc_spatial_transform_type));
    payload.rotate = fe_ioss->io_handle->rotate ?
                     fe_ioss->io_handle->rotate : VIDC_ROTATE_NONE;

    if (data->value == V4L2_MPEG_MSM_VIDC_ENABLE)
    {
        switch(data->id)
        {
            case V4L2_CID_HFLIP:
                payload.flip = VIDC_FLIP_HORIZ;
                break;
            case V4L2_CID_VFLIP:
                payload.flip = VIDC_FLIP_VERT;
                break;
            default:
                HYP_VIDEO_MSG_ERROR("unsupported flip value %d", data->value);
                rc = HYPV_STATUS_FAIL;
        }

       if (HYPV_STATUS_SUCCESS == rc)
       {
            /**
            *This is for VIDC_FLIP_BOTH case. Client may not send VIDC_FLIP_BOTH directly.
            *Instead it sends VIDC_FLIP_HORIZ/VIDC_FLIP_VERT separately. In this case frontend
            *shall perform an OR operation with stored flip value to get VIDC_FLIP_BOTH.
            **/
            payload.flip = (vidc_flip_type)(fe_ioss->io_handle->flip | payload.flip);

            rc = dec_enc_set_drv_property(fe_ioss,
                    VIDC_I_VPE_SPATIAL_TRANSFORM,
                    sizeof(vidc_spatial_transform_type),
                    ( uint8* )&payload);
            if (HYPV_STATUS_SUCCESS != rc)
            {
                rc = HYPV_STATUS_FAIL;
                HYP_VIDEO_MSG_ERROR("failed to set drv flip property 0x%x",
                   (unsigned int)payload.flip);
            }
            else
            {
                fe_ioss->io_handle->flip = payload.flip;
            }
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_rotation

@brief  Encoder FE set rotation

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_rotation(fe_io_session_t *fe_ioss,
                                              struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_spatial_transform_type payload;

    HYP_VIDEO_MSG_ERROR("set rotation %d", data->value);
    HABMM_MEMSET(&payload, 0, sizeof(vidc_spatial_transform_type));
    payload.flip = fe_ioss->io_handle->flip ?
                   fe_ioss->io_handle->flip : VIDC_FLIP_NONE;

    switch(data->value)
    {
        case 0:
            payload.rotate = VIDC_ROTATE_NONE;
            break;
        case 90:
            payload.rotate = VIDC_ROTATE_90;
            break;
        case 180:
            payload.rotate = VIDC_ROTATE_180;
            break;
        case 270:
            payload.rotate = VIDC_ROTATE_270;
            break;
        default:
            HYP_VIDEO_MSG_ERROR("unsupported rotation value %d", data->value);
            rc = HYPV_STATUS_FAIL;
    }

    if (HYPV_STATUS_SUCCESS == rc)
    {
        rc = dec_enc_set_drv_property(fe_ioss,
                                      VIDC_I_VPE_SPATIAL_TRANSFORM,
                                      sizeof(vidc_spatial_transform_type),
                                      ( uint8* )&payload);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            rc = HYPV_STATUS_FAIL;
            HYP_VIDEO_MSG_ERROR("failed to set drv rotation property 0x%x",
                                 (unsigned int)payload.rotate);
        }
        else
        {
            fe_ioss->io_handle->rotate = payload.rotate;

            /* flip resolution incase of 90/270 rotation */
            if (VIDC_ROTATE_90 ==  payload.rotate ||
                VIDC_ROTATE_270 == payload.rotate)
            {
                vidc_frame_size_type frame_size;

                frame_size.buf_type = VIDC_BUFFER_OUTPUT;
                frame_size.width = fe_ioss->frame_size.height;
                frame_size.height = fe_ioss->frame_size.width;

                rc = dec_enc_set_drv_property(fe_ioss,
                                              VIDC_I_FRAME_SIZE,
                                              sizeof(vidc_frame_size_type),
                                              ( uint8* )&frame_size );
                if (HYPV_STATUS_SUCCESS != rc)
                {
                    HYP_VIDEO_MSG_ERROR("failed to flip resolution buf type %d width %u height %u",
                                         frame_size.buf_type, frame_size.width, frame_size.height);
                }
                else
                {
                    HYP_VIDEO_MSG_INFO("set drv frame size property buf type %d width %u height %u",
                                        frame_size.buf_type, frame_size.width, frame_size.height);
                }
            }
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_frame_quality

@brief  Encoder FE set frame quality

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_frame_quality(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_frame_quality_type payload;

    HYP_VIDEO_MSG_INFO("set frame quality %d", data->value);
    if (0 < data->value)
    {
        payload.frame_quality = data->value;
        rc = dec_enc_set_drv_property(fe_ioss,
                                  VIDC_I_ENC_FRAME_QUALITY,
                                  sizeof(vidc_frame_quality_type),
                                  ( uint8* )&payload);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            rc = HYPV_STATUS_FAIL;
            HYP_VIDEO_MSG_ERROR("failed to set frame quality %u",
                                payload.frame_quality);
        }
    }

    return rc;
}

/**===========================================================================
FUNCTION v4l2fe_enc_s_bitrate_peak

@brief  Encoder FE set bitrate peak

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_bitrate_peak(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_bitrate_type payload;

    HYP_VIDEO_MSG_INFO("set bitrate peak to %d", data->value);
    payload.layer_id = 0;
    payload.bitrate = data->value;
    rc = dec_enc_set_drv_property(fe_ioss,
                                  VIDC_I_ENC_MAX_BITRATE,
                                  sizeof(vidc_bitrate_type),
                                  ( uint8* )&payload);
    if (HYPV_STATUS_SUCCESS != rc)
    {
        rc = HYPV_STATUS_FAIL;
        HYP_VIDEO_MSG_ERROR("failed to set drv max bitrate property. layer_id %u bitrate %u",
                             payload.layer_id, payload.bitrate);
    }

    return rc;
}

/**===========================================================================
FUNCTION enc_s_extradata

@brief  Encoder FE set extradata

@param [in] fe_ioss pointer
@param [in] vidc_metadata_header_type pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type enc_s_extradata(fe_io_session_t *fe_ioss,
                                        vidc_metadata_header_type *meta_header)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    rc = dec_enc_set_drv_property(fe_ioss,
                                  VIDC_I_METADATA_HEADER,
                                  sizeof(vidc_metadata_header_type),
                                  ( uint8* ) meta_header);
    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to set metadata header property type %u, rc=%d",
                            meta_header->metadata_type, rc);
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_extradata

@brief  Encoder FE set extradata

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_extradata(fe_io_session_t *fe_ioss,
                                               struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_metadata_header_type meta_header;

    HYP_VIDEO_MSG_INFO("set extradata %d", data->value);

    meta_header.enable = TRUE;
    meta_header.version = V4L2FE_METADATA_DEFAULT_VERSION;
    if (data->value & EXTRADATA_ENC_INPUT_CROP)
    {
        meta_header.client_type = VIDC_METADATA_PROPERTY_INDEX_INPUT_CROP;
        meta_header.metadata_type = VIDC_METADATA_PROPERTY_INDEX_INPUT_CROP;
        meta_header.port_index = V4L2FE_METADATA_DEFAULT_PORTINDEX_INPUT;
        fe_ioss->io_handle->input_extradata_enable = TRUE;
        if (HYPV_STATUS_SUCCESS != enc_s_extradata(fe_ioss, &meta_header))
        {
            rc = HYPV_STATUS_FAIL;
        }
    }
    if (data->value & EXTRADATA_ENC_INPUT_ROI)
    {
        meta_header.client_type = VIDC_METADATA_ROI_QP;
        meta_header.metadata_type = VIDC_METADATA_ROI_QP;
        meta_header.port_index = V4L2FE_METADATA_DEFAULT_PORTINDEX_INPUT;
        fe_ioss->io_handle->input_extradata_enable = TRUE;
        if (HYPV_STATUS_SUCCESS != enc_s_extradata(fe_ioss, &meta_header))
        {
            rc = HYPV_STATUS_FAIL;
        }
    }
    if (data->value & EXTRADATA_ENC_FRAME_QP)
    {
        meta_header.client_type = VIDC_METADATA_FRAME_QP;
        meta_header.metadata_type = VIDC_METADATA_FRAME_QP;
        meta_header.port_index = V4L2FE_METADATA_DEFAULT_PORTINDEX_OUTPUT;
        if (HYPV_STATUS_SUCCESS != enc_s_extradata(fe_ioss, &meta_header))
        {
            rc = HYPV_STATUS_FAIL;
        }
    }
    if (data->value & EXTRADATA_ENC_INPUT_HDR10PLUS)
    {
        meta_header.client_type = VIDC_METADATA_HDR10_PLUS;
        meta_header.metadata_type = VIDC_METADATA_HDR10_PLUS;
        meta_header.port_index = V4L2FE_METADATA_DEFAULT_PORTINDEX_OUTPUT;
        fe_ioss->io_handle->input_extradata_enable = TRUE;
        if (HYPV_STATUS_SUCCESS != enc_s_extradata(fe_ioss, &meta_header))
        {
            rc = HYPV_STATUS_FAIL;
        }
    }


    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_operating_rate

@brief  Encoder FE set operating rate

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_operating_rate(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{

    UNUSED(fe_ioss);
    UNUSED(data);

    HYP_VIDEO_MSG_INFO("unsupported");

    return HYPV_STATUS_FAIL;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_frame_rate

@brief  Encoder FE set frame rate

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_frame_rate(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_frame_rate_type frame_rate;

    frame_rate.fps_numerator = data->value >> 16;
    frame_rate.fps_denominator = 1;
    frame_rate.buf_type = VIDC_BUFFER_OUTPUT;
    rc = dec_enc_set_drv_property(fe_ioss,
                                  VIDC_I_FRAME_RATE,
                                  sizeof(vidc_frame_rate_type),
                                  ( uint8* )&frame_rate);
    if (HYPV_STATUS_SUCCESS != rc)
    {
        rc = HYPV_STATUS_FAIL;
        HYP_VIDEO_MSG_ERROR("failed to set frame rate %u", frame_rate.fps_numerator);
    }
    else
    {
        HYP_VIDEO_MSG_INFO("Set framerate to %u", frame_rate.fps_numerator);
    }

    return rc;
}

/**===========================================================================


FUNCTION v4l2fe_enc_s_mark_ltr_frame

@brief  Encoder FE set mark ltr frame

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_mark_ltr_frame(fe_io_session_t* fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_BAD_PARAMETER;

    uint32 ltrMarkFrame = (uint32) data->value;

    HYP_VIDEO_MSG_INFO("set ltr mark frame %u", ltrMarkFrame);

    rc = dec_enc_set_drv_property(fe_ioss,
                               VIDC_I_ENC_MARKLTRFRAME,
                               sizeof(ltrMarkFrame),
                               ( uint8* )&ltrMarkFrame);
    if (HYPV_STATUS_SUCCESS != rc)
    {
        rc = HYPV_STATUS_FAIL;
        HYP_VIDEO_MSG_ERROR("failed to set ltr mark frame %u", ltrMarkFrame);
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_use_ltr_frame

@brief  Encoder FE set use LTR frame
@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_s_use_ltr_frame(fe_io_session_t* fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_BAD_PARAMETER;
    vidc_ltr_use_type ltr_use_frame;

    HABMM_MEMSET(&ltr_use_frame, 0, sizeof(vidc_ltr_use_type));
    ltr_use_frame.ref_ltr = (uint32) data->value;
    ltr_use_frame.use_constraint = FALSE;

    HYP_VIDEO_MSG_INFO("set ltr use frame %d", data->value);

    rc = dec_enc_set_drv_property(fe_ioss,
                               VIDC_I_ENC_USELTRFRAME,
                               sizeof(ltr_use_frame),
                               ( uint8* )&ltr_use_frame);
    if (HYPV_STATUS_SUCCESS != rc)
    {
        rc = HYPV_STATUS_FAIL;
        HYP_VIDEO_MSG_ERROR("failed to set ltr use frame %d", data->value);
    }

    return rc;
}

//
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_BITRATE)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_TARGET_BITRATE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_RATE_CONTROL)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_GOP_SIZE)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_INTRA_PERIOD)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_B_FRAMES)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_INTRA_PERIOD)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_REQUEST_SEQ_HEADER)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_SYNC_FRAME_SEQ_HDR)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_SESSION_QP)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_SESSION_QP)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_SESSION_QP)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_H264_MIN_QP)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_SESSION_QP_RANGE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_H264_MAX_QP)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_SESSION_QP_RANGE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_VP8_MIN_QP)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_VP8_MAX_QP)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_H264_PROFILE)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_PROFILE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_HEVC_PROFILE)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_PROFILE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_H264_LEVEL)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_LEVEL)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_HEVC_LEVEL)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_LEVEL)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_IDR_PERIOD)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_IDR_PERIOD)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_H264_ENTROPY_CTRL)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_H264_CABAC_MODEL)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_H264_ENTROPY_CTRL)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_MULTI_SLICE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_MULTI_SLICE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_BYTES)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_MULTI_SLICE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_MULTI_SLICE_GOB)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_MULTI_SLICE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_MODE)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_INTRA_REFRESH)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_CIR_MBS)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_INTRA_REFRESH)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_AIR_MBS)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_INTRA_REFRESH)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_AIR_REF)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_INTRA_REFRESH)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_H264_DEBLOCKING)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_ALPHA)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_H264_DEBLOCKING)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_BETA)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_H264_DEBLOCKING)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_FORCE_KEY_FRAME)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_REQUEST_SYNC_FRAME)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_ROTATION)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_VPE_SPATIAL_TRANSFORM)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_BITRATE_PEAK)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_ENC_MAX_BITRATE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_OPERATING_RATE)=>VIDC_IOCTL_SET_PROPERTY(VIDC_I_FRAME_RATE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_USELTRFRAME)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_MARKLTRFRAME)
// To do:
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_EXTRADATA.V4L2_MPEG_VIDC_EXTRADATA_MULTISLICE_INFO)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_EXTRADATA.V4L2_MPEG_VIDC_EXTRADATA_METADATA_MBI)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_EXTRADATA.V4L2_MPEG_VIDC_EXTRADATA_INPUT_CROP)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_EXTRADATA.V4L2_MPEG_VIDC_EXTRADATA_PQ_INFO)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_VP8_PROFILE_LEVEL)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_H264_PIC_ORDER_CNT)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_H264_VUI_TIMING_INFO)
// To do: QNX_NOT_SUPPORTED_YET
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_H264_AU_DELIMITER)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_MBI_STATISTICS_MODE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_VQZIP_SEI)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_EXTRADATA:V4L2_MPEG_VIDC_EXTRADATA_YUV_STATS)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_MAX_HIERP_LAYERS)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_HIER_P_NUM_LAYERS)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_H264_NAL_SVC)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_HIER_B_NUM_LAYERS)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDEO_MULTI_SLICE_DELIVERY_MODE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_DEINTERLACE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_PERF_MODE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_CONFIG_QP)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_BASELAYER_ID)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_VPX_ERROR_RESILIENCE)
// VIDIOC_S_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_PRIORITY)
// VIDIOC_S_CTRL(V4L2_CID_VIDC_QBUF_MODE)
/**===========================================================================

FUNCTION v4l2fe_enc_s_ctrl

@brief  Encoder FE set control

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_enc_s_ctrl(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    fe_linux_plt_data_t *plt_data = (fe_linux_plt_data_t *)(fe_ioss->fe_plt_data);

    HYP_VIDEO_MSG_INFO("encode set control id %u value %d",
                        data->id, data->value);
    switch (data->id)
    {
    case V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE:
    case V4L2_CID_MPEG_VIDEO_BITRATE_MODE:
        fe_ioss->enable_rate_control = TRUE;
        rc = v4l2fe_enc_s_rate_control(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDEO_BITRATE:
        rc = v4l2fe_enc_s_bitrate(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDEO_PREPEND_SPSPPS_TO_IDR:
        rc = v4l2fe_enc_s_request_seq_header(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDEO_H264_PROFILE:
        rc = v4l2fe_enc_s_profile(fe_ioss, data);

        if (HYPV_STATUS_SUCCESS == rc &&
           (V4L2_MPEG_VIDEO_H264_PROFILE_MAIN == data->value ||
            V4L2_MPEG_VIDEO_H264_PROFILE_HIGH == data->value))
        {
            fe_ioss->enable_entropy_mode = TRUE;
            fe_ioss->enc_entropy_info.value = V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CABAC;
            fe_ioss->enc_entropy.entropy_mode = VIDC_ENTROPY_MODE_CABAC;
            rc = enc_s_h264_entropy_mode(fe_ioss);
        }
        break;
    case V4L2_CID_MPEG_VIDEO_HEVC_PROFILE:
    case V4L2_CID_MPEG_VIDEO_VP8_PROFILE:
        rc = v4l2fe_enc_s_profile(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDEO_H264_LEVEL:
    case V4L2_CID_MPEG_VIDEO_HEVC_LEVEL:
        rc = v4l2fe_enc_s_level(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDEO_HEVC_TIER:
        rc = v4l2fe_enc_s_tier(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDEO_GOP_SIZE:
        fe_ioss->enc_iperiod.p_frames = data->value;
        if (V4L2FE_STATE_EXECUTING == plt_data->state)
        {
            /* Dynamic setting of P-frames */
            rc = enc_s_num_p_frames(fe_ioss);
        }
        break;
    case V4L2_CID_MPEG_VIDEO_B_FRAMES:
        if (V4L2FE_STATE_EXECUTING == plt_data->state)
        {
            HYP_VIDEO_MSG_ERROR("Dynamic B-frames is not supported id %u value %d",
                                data->id, data->value);
            rc = HYPV_STATUS_FAIL;
        }
        else
        {
            fe_ioss->enc_iperiod.b_frames = data->value;
        }
        break;
    case V4L2_CID_MPEG_VIDEO_HEVC_MIN_QP:
    case V4L2_CID_MPEG_VIDEO_H264_MIN_QP:
    case V4L2_CID_MPEG_VIDEO_VPX_MIN_QP:
        fe_ioss->enc_qp_range.qp_min_packed = data->value;
        rc = HYPV_STATUS_SUCCESS;
        break;
    case V4L2_CID_MPEG_VIDEO_HEVC_MAX_QP:
    case V4L2_CID_MPEG_VIDEO_H264_MAX_QP:
    case V4L2_CID_MPEG_VIDEO_VPX_MAX_QP:
        fe_ioss->enc_qp_range.qp_max_packed = data->value;
        rc = v4l2fe_enc_s_qp_range(fe_ioss);
        break;
    case V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP:
    case V4L2_CID_MPEG_VIDEO_MPEG4_I_FRAME_QP:
    case V4L2_CID_MPEG_VIDEO_VPX_I_FRAME_QP:
    case V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_QP:
        fe_ioss->enable_frame_qp = TRUE;
        if (V4L2FE_STATE_EXECUTING == plt_data->state)
        {
            /* Dynamic QP setting */
            fe_ioss->enc_qp.i_frame_qp = data->value;
            rc = enc_s_frame_qp(fe_ioss);
        }
        else
        {
            rc = v4l2fe_enc_s_frame_qp(fe_ioss, data);
        }
        break;
    case V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP:
    case V4L2_CID_MPEG_VIDEO_MPEG4_P_FRAME_QP:
    case V4L2_CID_MPEG_VIDEO_VPX_P_FRAME_QP:
    case V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_QP:
    case V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP:
    case V4L2_CID_MPEG_VIDEO_MPEG4_B_FRAME_QP:
    case V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_QP:
        fe_ioss->enable_frame_qp = TRUE;
        rc = v4l2fe_enc_s_frame_qp(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE:
        fe_ioss->enable_entropy_mode = TRUE;
        fe_ioss->enc_entropy_info.value = data->value;
        break;
    case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE:
    case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB:
    case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_BYTES:
        rc = v4l2fe_enc_s_multi_slice(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_RANDOM:
    case V4L2_CID_MPEG_VIDEO_CYCLIC_INTRA_REFRESH_MB:
        HYP_VIDEO_MSG_ERROR("Intra refresh type setting (RANDOM/CYCLIC) is deprecated,"
                           "please use V4L2_CID_MPEG_VIDEO_VIDC_INTRA_REFRESH_TYPE.");
        rc = HYPV_STATUS_SUCCESS;
        break;
    case V4L2_CID_MPEG_VIDEO_VIDC_INTRA_REFRESH_TYPE:
        fe_ioss->enable_intra_refresh = TRUE;
        fe_ioss->intra_refresh_info = *data;
        rc = HYPV_STATUS_SUCCESS;
        break;
    case V4L2_CID_MPEG_VIDC_INTRA_REFRESH_PERIOD:
        fe_ioss->intra_refresh_period = data->value;
        rc = HYPV_STATUS_SUCCESS;
        break;
    case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE:
    case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_ALPHA:
    case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_BETA:
        rc = v4l2fe_enc_s_loop_filter(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDEO_FORCE_KEY_FRAME:
        rc = v4l2fe_enc_s_request_iframe(fe_ioss, data);
        break;
    case V4L2_CID_ROTATE:
        rc = v4l2fe_enc_s_rotation(fe_ioss, data);
        break;
    case V4L2_CID_HFLIP:
    case V4L2_CID_VFLIP:
        rc = v4l2fe_enc_s_flip(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDEO_BITRATE_PEAK:
        rc = v4l2fe_enc_s_bitrate_peak(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_OPERATING_RATE:
        rc = v4l2fe_enc_s_operating_rate(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_EXTRADATA:
        rc = v4l2fe_enc_s_extradata(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_SECURE:
        // The real one is delayed until dec_enc_apply_properities()
        fe_ioss->io_handle->secure = (data->value) ? TRUE : FALSE;
        break;
    case V4L2_CID_MPEG_VIDC_IMG_GRID_SIZE:
        fe_ioss->enc_grid_mode = data->value;
        break;
    case V4L2_CID_MPEG_VIDC_COMPRESSION_QUALITY:
        rc = v4l2fe_enc_s_frame_quality(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_LOWLATENCY_MODE:
        rc = dec_enc_s_low_latency_mode(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_USELTRFRAME:
        rc = v4l2fe_enc_s_use_ltr_frame(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_MARKLTRFRAME:
        rc = v4l2fe_enc_s_mark_ltr_frame(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_LTRCOUNT:
        rc = v4l2fe_enc_s_ltr_count(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_VP8_PROFILE_LEVEL:
    case V4L2_CID_MPEG_VIDC_VIDEO_VUI_TIMING_INFO:
        HYP_VIDEO_MSG_ERROR("ToDo id %u value %d", data->id, data->value);
        rc = HYPV_STATUS_SUCCESS;
        break;
    case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_TYPE:
         fe_ioss->hier_layer_type = data->value;
         break;
    case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_LAYER:
         fe_ioss->hier_layer_count = data->value;
         if (V4L2FE_STATE_EXECUTING == plt_data->state)
         {
            /* Dynamic setting of Hier layers count */
            rc = enc_s_hp_layer(fe_ioss);
         }
         break;
    case V4L2_CID_MPEG_VIDC_VIDEO_HEVC_MAX_HIER_CODING_LAYER:
         fe_ioss->max_hier_layer_count = data->value;
         break;
    case V4L2_CID_MPEG_VIDC_VIDEO_BASELAYER_ID:
    case V4L2_CID_MPEG_VIDC_VIDEO_VPX_ERROR_RESILIENCE:
    case V4L2_CID_MPEG_VIDC_VIDEO_PRIORITY:
    case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L0_BR:
    case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L1_BR:
    case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L2_BR:
    case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L3_BR:
    case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L4_BR:
    case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L5_BR:
    case V4L2_CID_MPEG_VIDC_VIDEO_BUFFER_SIZE_LIMIT:
    case V4L2_CID_MPEG_VIDC_VENC_BITRATE_BOOST:
        HYP_VIDEO_MSG_INFO("//TODO not supported yet id %u value %d",
                             data->id, data->value);
        rc = HYPV_STATUS_SUCCESS;
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_AU_DELIMITER:
        HYP_VIDEO_MSG_ERROR("ToDo id %u value %d", data->id, data->value);
        rc = HYPV_STATUS_SUCCESS;
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_STREAM_FORMAT:
        HYP_VIDEO_MSG_ERROR("ToDo id %u value %d", data->id, data->value);
        rc = HYPV_STATUS_SUCCESS;
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_FRAME_RATE:
        rc = v4l2fe_enc_s_frame_rate(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDEO_H264_8X8_TRANSFORM:
    case V4L2_CID_MPEG_VIDEO_HEVC_SIZE_OF_LENGTH_FIELD:
        HYP_VIDEO_MSG_ERROR("ToDo id %u value %d", data->id, data->value);
        rc = HYPV_STATUS_SUCCESS;
        break;
    case V4L2_CID_MPEG_VIDC_VENC_NATIVE_RECORDER:
        HYP_VIDEO_MSG_ERROR("ToDo id %u value %d", data->id, data->value);
        rc = HYPV_STATUS_SUCCESS;
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_COLOR_SPACE:
        fe_ioss->vui_video_signal_info.colour_primaries = data->value;
        rc = HYPV_STATUS_SUCCESS;
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_FULL_RANGE:
        fe_ioss->vui_video_signal_info.video_full_Range_flag = data->value;
        rc = HYPV_STATUS_SUCCESS;
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_TRANSFER_CHARS:
        fe_ioss->vui_video_signal_info.transfer_characteristics = data->value;
        rc = HYPV_STATUS_SUCCESS;
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_MATRIX_COEFFS:
        fe_ioss->vui_video_signal_info.matrix_coeffs = data->value;
        rc = HYPV_STATUS_SUCCESS;
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_VPE_CSC:
        fe_ioss->video_vpe_csc = data->value;
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_VPE_CSC_CUSTOM_MATRIX:
        fe_ioss->video_vpe_csc_custom_matrix = data->value;
        break;
    /*case V4L2_CID_MPEG_VIDC_VIDEO_QP_MASK:
        HYP_VIDEO_MSG_ERROR("ToDo id %d value %d", data->id, data->value);
        rc = HYPV_STATUS_SUCCESS;
        break;*/
    case V4L2_CID_MPEG_VIDC_VIDEO_BLUR_DIMENSIONS:
    {
        if (V4L2FE_STATE_EXECUTING == plt_data->state)
        {
            rc = enc_s_blur_dimensions(fe_ioss, data);
        }
        else
        {
            fe_ioss->enable_blur_filter = TRUE;
            fe_ioss->blur_filter_info = *data;
            rc = HYPV_STATUS_SUCCESS;
        }
        break;
    }
    case V4L2_CID_MPEG_VIDC_VENC_BITRATE_SAVINGS:
    {
        fe_ioss->enable_bitrate_saving_mode = TRUE;
        fe_ioss->bitrate_saving_mode = *data;
        break;
    }
    case V4L2_CID_MPEG_VIDC_VENC_HDR_INFO:
    {
        uint32 info_type = ((uint32)data->value >> INFO_TYPE_OFFSET) & INFO_TYPE_MASK;
        uint32 value = (data->value & INFO_VALUE_MASK);
        fe_ioss->hdr10_sei_enabled = true;
        switch (info_type)
        {
            case MSM_VIDC_RGB_PRIMARY_00:
                fe_ioss->hdr_static_info.mastering_disp_colour_sei.display_primaries_x_0 = value;
                break;
            case MSM_VIDC_RGB_PRIMARY_01:
                fe_ioss->hdr_static_info.mastering_disp_colour_sei.display_primaries_y_0 = value;
                break;
            case MSM_VIDC_RGB_PRIMARY_10:
                fe_ioss->hdr_static_info.mastering_disp_colour_sei.display_primaries_x_1 = value;
                break;
            case MSM_VIDC_RGB_PRIMARY_11:
                fe_ioss->hdr_static_info.mastering_disp_colour_sei.display_primaries_y_1 = value;
                break;
            case MSM_VIDC_RGB_PRIMARY_20:
                fe_ioss->hdr_static_info.mastering_disp_colour_sei.display_primaries_x_2 = value;
                break;
            case MSM_VIDC_RGB_PRIMARY_21:
                fe_ioss->hdr_static_info.mastering_disp_colour_sei.display_primaries_y_2 = value;
                break;
            case MSM_VIDC_WHITEPOINT_X:
                fe_ioss->hdr_static_info.mastering_disp_colour_sei.white_point_x = value;
                break;
            case MSM_VIDC_WHITEPOINT_Y:
                fe_ioss->hdr_static_info.mastering_disp_colour_sei.white_point_y = value;
                break;
            case MSM_VIDC_MAX_DISP_LUM:
                fe_ioss->hdr_static_info.mastering_disp_colour_sei.max_display_mastering_luminance = value;
                break;
            case MSM_VIDC_MIN_DISP_LUM:
                fe_ioss->hdr_static_info.mastering_disp_colour_sei.min_display_mastering_luminance = value;
                break;
            case MSM_VIDC_RGB_MAX_CLL:
                fe_ioss->hdr_static_info.content_light_level_sei.max_pic_average_light_level = value;
                break;
            case MSM_VIDC_RGB_MAX_FLL:
                fe_ioss->hdr_static_info.content_light_level_sei.max_content_light_level = value;
                break;
            default:
                HYP_VIDEO_MSG_ERROR("Unknown Ctrl:%u, not part of HDR Info with value %u",info_type, value);
                rc = HYPV_STATUS_FAIL;
        }
        break;
    }
    default:
        HYP_VIDEO_MSG_ERROR("unknown and not supported id %u value %d",
                             data->id, data->value);
        rc = HYPV_STATUS_FAIL;
        break;
    }

    return rc;
}


/**===========================================================================

FUNCTION v4l2fe_enc_g_profile

@brief  Encoder FE get profile

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_g_profile(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_profile_type payload;

    HABMM_MEMSET(&payload, 0, sizeof(vidc_profile_type));

    HYP_VIDEO_MSG_INFO("get profile for id %u", data->id);
    rc = dec_enc_get_drv_property(fe_ioss,
        VIDC_I_PROFILE,
        sizeof(vidc_profile_type),
        ( uint8* )&payload);
    if (0 != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to get drv property - VIDC_I_PROFILE");
    }
    else
    {
        data->value = dec_enc_convert_vidc_to_v4l2(data->id, payload.profile);
        if (data->value < 0)
        {
            rc = HYPV_STATUS_FAIL;
            HYP_VIDEO_MSG_ERROR("profile %u unsupported",data->id);
        }
    }
    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_g_level

@brief  Encoder FE get level

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_g_level(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_level_type payload;

    HABMM_MEMSET(&payload, 0, sizeof(vidc_level_type));

    HYP_VIDEO_MSG_INFO("get level for id %u", data->id);
    rc = dec_enc_get_drv_property(fe_ioss,
                                  VIDC_I_LEVEL,
                                  sizeof(vidc_level_type),
                                  ( uint8* )&payload);
    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to get drv level property");
    }
    else
    {
        data->value = dec_enc_convert_vidc_to_v4l2(data->id, payload.level);
        if (data->value < 0)
        {
            rc = HYPV_STATUS_FAIL;
            HYP_VIDEO_MSG_ERROR("unsupported level %u", data->id);
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_g_frame_qp

@brief  Encoder FE get frame QP

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_g_frame_qp(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_session_qp_type payload;

    HABMM_MEMSET(&payload, 0, sizeof(vidc_session_qp_type));

    HYP_VIDEO_MSG_INFO("data id %u data value %d", data->id, data->value);
    rc = dec_enc_get_drv_property(fe_ioss,
        VIDC_I_ENC_SESSION_QP,
        sizeof(vidc_session_qp_type),
        ( uint8* )&payload);

    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to get drv qp property");
    }
    else
    {
        if (V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP == data->id)
        {
            data->value = payload.i_frame_qp;
        }
        else if (V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP == data->id)
        {
            data->value = payload.p_frame_qp;
        }
        else if (V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP == data->id)
        {
            data->value = payload.b_frame_qp;
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_g_rate_control

@brief  Encoder FE get rate control

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_g_rate_control(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_rate_control_type payload;

    HABMM_MEMSET(&payload, 0, sizeof(vidc_rate_control_type));

    rc = dec_enc_get_drv_property(fe_ioss,
                                  VIDC_I_ENC_RATE_CONTROL,
                                  sizeof(vidc_rate_control_type),
                                  ( uint8* )&payload);
    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to get drv rate control property");
    }
    else
    {
        switch (payload.rate_control)
        {
            case VIDC_RATE_CONTROL_VBR_VFR:
            case VIDC_RATE_CONTROL_VBR_CFR:
                data->value = V4L2_MPEG_VIDEO_BITRATE_MODE_VBR;
                break;
            case VIDC_RATE_CONTROL_CBR_VFR:
                data->value = V4L2_MPEG_VIDEO_BITRATE_MODE_CBR_VFR;
                break;
            case VIDC_RATE_CONTROL_CBR_CFR:
                data->value = V4L2_MPEG_VIDEO_BITRATE_MODE_CBR;
                break;
            case VIDC_RATE_CONTROL_MBR_CFR:
                data->value = V4L2_MPEG_VIDEO_BITRATE_MODE_MBR;
                break;
            case VIDC_RATE_CONTROL_MBR_VFR:
                data->value = V4L2_MPEG_VIDEO_BITRATE_MODE_MBR_VFR;
                break;
            case VIDC_RATE_CONTROL_CQ:
                data->value = V4L2_MPEG_VIDEO_BITRATE_MODE_CQ;
                break;
            default:
                HYP_VIDEO_MSG_ERROR("vidc rate control 0x%x not supported",
                                     (unsigned int)payload.rate_control);
                rc = HYPV_STATUS_FAIL;
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_g_bitrate

@brief  Encoder FE get bitrate

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_g_bitrate(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_target_bitrate_type payload;

    HABMM_MEMSET(&payload, 0, sizeof(vidc_target_bitrate_type));

    rc = dec_enc_get_drv_property(fe_ioss,
                                  VIDC_I_TARGET_BITRATE,
                                  sizeof(vidc_target_bitrate_type),
                                  ( uint8* )&payload);

    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to get drv target bitrate property");
    }
    else
    {
        data->value = payload.target_bitrate;
        HYP_VIDEO_MSG_INFO("target bitrate %d", data->value);
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_g_bitrate_peak

@brief  Encoder FE get peak bitrate

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_g_bitrate_peak(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_bitrate_type payload;

    HABMM_MEMSET(&payload, 0, sizeof(vidc_bitrate_type));

    rc = dec_enc_get_drv_property(fe_ioss,
                                  VIDC_I_ENC_MAX_BITRATE,
                                  sizeof(vidc_bitrate_type),
                                  ( uint8* )&payload);

    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to get drv max bitrate property");
    }
    else
    {
        data->value = payload.bitrate;
        HYP_VIDEO_MSG_INFO("bitrate peak is %d", data->value);
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_g_num_p_frames

@brief  Encoder FE get number of P frames

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_g_num_p_frames(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_iperiod_type payload;

    HABMM_MEMSET(&payload, 0, sizeof(vidc_iperiod_type));

    rc = dec_enc_get_drv_property(fe_ioss,
                                  VIDC_I_ENC_INTRA_PERIOD,
                                  sizeof(vidc_iperiod_type),
                                  ( uint8* )&payload);

    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to get drv intra period property");
    }
    else
    {
        data->value = payload.p_frames;
        HYP_VIDEO_MSG_INFO("num of p frames is %d", data->value);
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_g_num_b_frames

@brief  Encoder FE get number of B frames

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_g_num_b_frames(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_iperiod_type payload;

    HABMM_MEMSET(&payload, 0, sizeof(vidc_iperiod_type));

    rc = dec_enc_get_drv_property(fe_ioss,
                                  VIDC_I_ENC_INTRA_PERIOD,
                                  sizeof(vidc_iperiod_type),
                                  ( uint8* )&payload);

    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to get drv intra period property");
    }
    else
    {
        data->value = payload.b_frames;
        HYP_VIDEO_MSG_INFO("num of b frames is %d", data->value);
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_g_h264_entropy_mode

@brief  Encoder FE get H264 entropy mode

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_g_h264_entropy_mode(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_entropy_control_type payload;

    HABMM_MEMSET(&payload, 0, sizeof(vidc_entropy_control_type));

    rc = dec_enc_get_drv_property(fe_ioss,
                                  VIDC_I_ENC_H264_ENTROPY_CTRL,
                                  sizeof(vidc_entropy_control_type),
                                  ( uint8* )&payload);

    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to get drv h264 entropy ctrl  property");
    }
    else
    {
        HYP_VIDEO_MSG_INFO("entropy mode is %d", payload.entropy_mode);
        if (VIDC_ENTROPY_MODE_CAVLC == payload.entropy_mode)
        {
            data->value = V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC;
        }
        else if (VIDC_ENTROPY_MODE_CABAC == payload.entropy_mode)
        {
            data->value = V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CABAC;
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("vidc mode 0x%x not supported", (unsigned int)payload.entropy_mode);
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_g_roi_type

@brief  Encoder FE get ROI type

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type v4l2fe_enc_g_roi_type(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_roi_mode_type payload;

    HABMM_MEMSET(&payload, 0, sizeof(vidc_roi_mode_type));

    rc = dec_enc_get_drv_property(fe_ioss,
                                  VIDC_I_ENC_ROI_MODE_TYPE,
                                  sizeof(vidc_roi_mode_type),
                                  ( uint8* )&payload);

    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to get drv ROI type property");
    }
    else
    {
        switch (payload.roi_mode)
        {
        case VIDC_ROI_MODE_2BIT:
            data->value = V4L2_CID_MPEG_VIDC_VIDEO_ROI_TYPE_2BIT;
            break;
        case VIDC_ROI_MODE_2BYTE:
            data->value = V4L2_CID_MPEG_VIDC_VIDEO_ROI_TYPE_2BYTE;
            break;
        default:
            data->value = V4L2_CID_MPEG_VIDC_VIDEO_ROI_TYPE_NONE;
        }

        HYP_VIDEO_MSG_INFO("ROI type %d", data->value);
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_g_ctrl

@brief  Encoder FE get control

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_enc_g_ctrl(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    switch (data->id)
    {
    case V4L2_CID_MPEG_VIDEO_H264_PROFILE:
    case V4L2_CID_MPEG_VIDEO_HEVC_PROFILE:
        rc = v4l2fe_enc_g_profile(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDEO_H264_LEVEL:
    case V4L2_CID_MPEG_VIDEO_HEVC_LEVEL:
        rc = v4l2fe_enc_g_level(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP:
    case V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP:
    case V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP:
        rc = v4l2fe_enc_g_frame_qp(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDEO_BITRATE_MODE:
        rc = v4l2fe_enc_g_rate_control(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDEO_BITRATE:
        rc = v4l2fe_enc_g_bitrate(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDEO_BITRATE_PEAK:
        rc = v4l2fe_enc_g_bitrate_peak(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDEO_GOP_SIZE:
        rc = v4l2fe_enc_g_num_p_frames(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDEO_B_FRAMES:
        rc = v4l2fe_enc_g_num_b_frames(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE:
        rc = v4l2fe_enc_g_h264_entropy_mode(fe_ioss, data);
        break;
    case V4L2_CID_MIN_BUFFERS_FOR_CAPTURE:
        {
            vidc_buffer_type buffer_type;
            vidc_buffer_reqmnts_type buffer_req_data;

            buffer_type = VIDC_BUFFER_OUTPUT;
            HABMM_MEMSET(&buffer_req_data, 0, sizeof(vidc_buffer_reqmnts_type));
            buffer_req_data.buf_type = buffer_type;
            rc = dec_enc_get_drv_property(fe_ioss,
                                          VIDC_I_BUFFER_REQUIREMENTS,
                                          sizeof(buffer_req_data),
                                          (uint8*)&buffer_req_data);

            if (HYPV_STATUS_SUCCESS != rc)
            {
                HYP_VIDEO_MSG_ERROR("failed to get drv buf requirement property");
            }
            else
            {
                data->value = buffer_req_data.min_count;
                if (MIN_BUFFER_COUNT_ENC > data->value)
                {
                    data->value = MIN_BUFFER_COUNT_ENC;
                }
                HYP_VIDEO_MSG_INFO("min output buf count %d", data->value);
            }
            break;
        }
    case V4L2_CID_MIN_BUFFERS_FOR_OUTPUT:
        {
            vidc_buffer_type buffer_type;
            vidc_buffer_reqmnts_type buffer_req_data;

            buffer_type = VIDC_BUFFER_INPUT;
            HABMM_MEMSET(&buffer_req_data, 0, sizeof(vidc_buffer_reqmnts_type));
            buffer_req_data.buf_type = buffer_type;
            rc = dec_enc_get_drv_property(fe_ioss,
                                          VIDC_I_BUFFER_REQUIREMENTS,
                                          sizeof(buffer_req_data),
                                          (uint8*)&buffer_req_data);

            if (HYPV_STATUS_SUCCESS != rc)
            {
                HYP_VIDEO_MSG_ERROR("failed to get drv buf requirement  property");
            }
            else
            {
                data->value = buffer_req_data.min_count;
                if (MIN_BUFFER_COUNT_ENC > data->value)
                {
                    data->value = MIN_BUFFER_COUNT_ENC;
                }
                HYP_VIDEO_MSG_INFO("min input count %d", data->value);
            }
            break;
        }
    case V4L2_CID_MPEG_VIDC_VIDEO_STREAM_FORMAT:
        data->value = 1 << V4L2_MPEG_VIDC_VIDEO_NAL_FORMAT_STARTCODES;
        HYP_VIDEO_MSG_ERROR("ToDo id %u value %d", data->id, data->value);
        rc = HYPV_STATUS_SUCCESS;
        break;
    /*case V4L2_CID_MPEG_VIDC_VIDEO_HYBRID_HIERP_MODE:
        data->value = 0;
        HYP_VIDEO_MSG_ERROR("ToDo id %d value %d", data->id, data->value);
        rc = HYPV_STATUS_SUCCESS;
        break;*/
    case V4L2_CID_MPEG_VIDC_VIDEO_ROI_TYPE:
        rc = v4l2fe_enc_g_roi_type(fe_ioss, data);
        break;
    case V4L2_CID_MPEG_VIDC_IMG_GRID_SIZE:
        {
            hypv_status_type rc = HYPV_STATUS_SUCCESS;
            vidc_enable_type payload;
            HABMM_MEMSET(&payload, 0, sizeof(vidc_enable_type));
            rc = dec_enc_get_drv_property(fe_ioss,
                                        VIDC_I_ENC_ENABLE_GRID,
                                        sizeof(vidc_enable_type),
                                        ( uint8* )&payload);
            if (HYPV_STATUS_SUCCESS != rc)
            {
                HYP_VIDEO_MSG_ERROR("Failed to get grid enable property");
            }
            else
            {
                data->value = payload.enable;
                if (!data->value)
                {
                    HYP_VIDEO_MSG_INFO("Grid not enabled");
                }
                else
                {
                    HYP_VIDEO_MSG_INFO("Grid enabled");
                }
            }
            break;
        }
    case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_TYPE:
        data->value = fe_ioss->hier_layer_type;
        rc = HYPV_STATUS_SUCCESS;
        break;
    case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_LAYER:
        data->value = fe_ioss->hier_layer_count;
        rc = HYPV_STATUS_SUCCESS;
        break;
    case V4L2_CID_MPEG_VIDC_VIDEO_HEVC_MAX_HIER_CODING_LAYER:
        data->value = fe_ioss->max_hier_layer_count;
        rc = HYPV_STATUS_SUCCESS;
        break;
    default:
        HYP_VIDEO_MSG_ERROR("unsupported id %u value %d", data->id, data->value);
        rc = HYPV_STATUS_FAIL;
        break;
    }
    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_querymenu

@brief  Encoder FE query menu

@param [in] v4l2_querymenu pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_enc_querymenu(struct v4l2_querymenu* data)
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
            rc = dec_enc_query_menu(venc_ctrls, ARRAY_SIZE(venc_ctrls), data);
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
             for (idx = 0; idx < ARRAY_SIZE(venc_ctrls); idx++)
             {
                 if (venc_ctrls[idx].id == data->id)
                 {
                    if ((uint)venc_ctrls[idx].maximum < data->index)
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

FUNCTION v4l2fe_enc_g_ctrl

@brief  Encoder FE get control

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_enc_queryctrl(fe_io_session_t *fe_ioss, struct v4l2_queryctrl* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    switch (data->id)
    {
    case V4L2_CID_MPEG_VIDEO_B_FRAMES:
        {
            vidc_range_type num_B_frm_sup;

            HABMM_MEMSET(&num_B_frm_sup, 0, sizeof(vidc_range_type));

            rc = dec_enc_get_drv_property(fe_ioss,
                                          VIDC_I_CAPABILITY_MAX_NUM_BFRAMES,
                                          sizeof(vidc_range_type),
                                          ( uint8* )&num_B_frm_sup );
            if (HYPV_STATUS_SUCCESS == rc)
            {
                data->minimum = num_B_frm_sup.min;
                data->maximum = num_B_frm_sup.max;
            }
            break;
        }
    case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB:
        {
            data->minimum = MIN_ENC_SLICE_MB_SIZE;
            data->maximum = MAX_ENC_SLICE_MB_SIZE;
            break;
        }
    case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_BYTES:
        {
            data->minimum = MIN_ENC_SLICE_BYTE_SIZE;
            data->maximum = MAX_ENC_SLICE_BYTE_SIZE;
            break;
        }
    case V4L2_CID_MPEG_VIDEO_H264_PROFILE:
    case V4L2_CID_MPEG_VIDEO_HEVC_PROFILE:
    case V4L2_CID_MPEG_VIDEO_MPEG2_PROFILE:
    case V4L2_CID_MPEG_VIDEO_VP9_PROFILE:
    case V4L2_CID_MPEG_VIDEO_VP8_PROFILE:
    case V4L2_CID_MPEG_VIDC_VIDEO_MPEG2_PROFILE:
    case V4L2_CID_MPEG_VIDEO_HEVC_TIER:
    {
            rc = dec_enc_query_profile(fe_ioss, venc_ctrls, ARRAY_SIZE(venc_ctrls), data);
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
            rc = dec_enc_query_level(fe_ioss, venc_ctrls, ARRAY_SIZE(venc_ctrls), data);
            break;
    }
    default:
        HYP_VIDEO_MSG_ERROR("unsupported query ctrl id %u", data->id);
        rc = HYPV_STATUS_FAIL;
        break;
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_s_parm

@brief  Encoder FE set parameter

@param [in] fe_ioss pointer
@param [in] v4l2_streamparm pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_enc_s_parm(fe_io_session_t* fe_ioss, struct v4l2_streamparm* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_frame_rate_type frame_rate;

    HABMM_MEMSET(&frame_rate, 0, sizeof(vidc_frame_rate_type));

    frame_rate.buf_type = VIDC_BUFFER_OUTPUT;

    // from spf to fps
    frame_rate.fps_denominator = data->parm.output.timeperframe.numerator;
    frame_rate.fps_numerator = data->parm.output.timeperframe.denominator;
    rc = dec_enc_set_drv_property(fe_ioss,
                                  VIDC_I_FRAME_RATE,
                                  sizeof(vidc_frame_rate_type),
                                  ( uint8* )&frame_rate);
    if (rc == HYPV_STATUS_SUCCESS)
    {
        HYP_VIDEO_MSG_INFO("set frame rate property numerator %u denominator %u",
                            frame_rate.fps_numerator, frame_rate.fps_denominator);
    }
    else
    {
        HYP_VIDEO_MSG_ERROR("failed to set frame rate property. numerator %u denominator %u",
                             frame_rate.fps_numerator, frame_rate.fps_denominator);
    }

    return rc;
}

hypv_status_type v4l2fe_enc_enum_fmt(fe_io_session_t* fe_ioss, struct v4l2_fmtdesc* data)
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

        fmt = dec_enc_get_pixel_fmt_index(venc_formats,
              ARRAY_SIZE(venc_formats), data->index, data->type);
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

/**===========================================================================

FUNCTION v4l2fe_enc_s_ltr_count

@brief  Encoder FE set LTR count
@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_enc_s_ltr_count(fe_io_session_t* fe_ioss,
                                        struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_BAD_PARAMETER;

    uint32 ltrCount = (uint32) data->value;

    vidc_ltr_mode ltr_mode = VIDC_LTR_MODE_MANUAL;

    HYP_VIDEO_MSG_INFO("set ltr count %u", ltrCount);

    rc = dec_enc_set_drv_property(fe_ioss,
                               VIDC_I_ENC_LTR_MODE,
                               sizeof(vidc_ltr_mode),
                               ( uint8* )&ltr_mode);
    if (HYPV_STATUS_SUCCESS != rc)
    {
        rc = HYPV_STATUS_FAIL;
        HYP_VIDEO_MSG_ERROR("failed to set ltr manual mode %d", ltr_mode);
    }
    else
    {
        rc = dec_enc_set_drv_property(fe_ioss,
                                   VIDC_I_ENC_LTR_COUNT,
                                   sizeof(ltrCount),
                                   ( uint8* )&ltrCount);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            rc = HYPV_STATUS_FAIL;
            HYP_VIDEO_MSG_ERROR("failed to set ltr count %u", ltrCount);
        }
        else
        {
            fe_ioss->ltr_count = ltrCount;
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION enc_s_intra_refresh

@brief  Encoder FE set intra refresh

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type enc_s_intra_refresh(fe_io_session_t *fe_ioss, struct v4l2_control* data) {
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_intra_refresh_type payload;
    uint32 num_mbs_per_frame = 0;

    HABMM_MEMSET(&payload, 0, sizeof(vidc_intra_refresh_type));

    if (V4L2_CID_MPEG_VIDEO_VIDC_INTRA_REFRESH_TYPE == data->id)
    {
        /* There are two types of intra refresh, data->value is the type of intra refresh.
           For period, is set by data->value of V4L2_CID_MPEG_VIDC_INTRA_REFRESH_PERIOD.
        */

        if (V4L2_MPEG_VIDEO_VIDC_INTRA_REFRESH_RANDOM == data->value)
        {
            if (fe_ioss->intra_refresh_period)
            {
                payload.ir_mode = VIDC_INTRA_REFRESH_RANDOM;
                num_mbs_per_frame = NUM_OF_MB(fe_ioss->frame_size.width, fe_ioss->frame_size.height);
                payload.air_mb_count = num_mbs_per_frame / fe_ioss->intra_refresh_period;
                if (num_mbs_per_frame % fe_ioss->intra_refresh_period)
                {
                    payload.air_mb_count++;
                }
                HYP_VIDEO_MSG_INFO("intra refresh random mode, mbs:%u", payload.air_mb_count);
            }
            else
            {
                payload.ir_mode = VIDC_INTRA_REFRESH_NONE;
            }
        }
        else if (V4L2_MPEG_VIDEO_VIDC_INTRA_REFRESH_CYCLIC == data->value)
        {
            if (fe_ioss->intra_refresh_period)
            {
                payload.ir_mode = VIDC_INTRA_REFRESH_CYCLIC;
                num_mbs_per_frame = NUM_OF_MB(fe_ioss->frame_size.width, fe_ioss->frame_size.height);
                payload.cir_mb_count = num_mbs_per_frame / fe_ioss->intra_refresh_period;
                if (num_mbs_per_frame % fe_ioss->intra_refresh_period)
                {
                    payload.cir_mb_count++;
                }
                HYP_VIDEO_MSG_INFO("intra refresh cyclic mode, mbs:%u", payload.cir_mb_count);
            }
            else
            {
                payload.ir_mode = VIDC_INTRA_REFRESH_NONE;
            }
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("unknown intra refresh mode %d", data->value);
            rc = HYPV_STATUS_FAIL;
        }
    }

    if (HYPV_STATUS_SUCCESS == rc)
    {
        rc = dec_enc_set_drv_property(fe_ioss,
                                    VIDC_I_ENC_INTRA_REFRESH,
                                    sizeof(vidc_intra_refresh_type),
                                    ( uint8* )&payload);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to set intra refresh property. id 0x%x valie %d",
                                data->id, data->value);
            rc = HYPV_STATUS_FAIL;
        }
        else
        {
            fe_ioss->enc_intra_refresh = payload;
        }
    }

    return rc;
}

hypv_status_type enc_s_grid_mode(fe_io_session_t* fe_ioss)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_enable_type payload;
    vidc_frame_size_type frame_size;

    if ((VIDC_CODEC_HEVC == fe_ioss->session_codec.codec) &&
        (0 != fe_ioss->enc_grid_mode))
    {
        HYP_VIDEO_MSG_INFO("enable grid");
        payload.enable = TRUE;
        rc = dec_enc_set_drv_property(fe_ioss,
                                      VIDC_I_ENC_ENABLE_GRID,
                                      sizeof(vidc_enable_type),
                                      ( uint8* )&payload);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            rc = HYPV_STATUS_FAIL;
            HYP_VIDEO_MSG_ERROR("failed to enable grid %d",
                                payload.enable);
        }
        else
        {
            frame_size.buf_type = VIDC_BUFFER_OUTPUT;
            frame_size.height = DEFAULT_TILE_DIMENSION;
            frame_size.width = DEFAULT_TILE_DIMENSION;

            rc = dec_enc_set_drv_property(fe_ioss,
                                          VIDC_I_FRAME_SIZE,
                                          sizeof(vidc_frame_size_type),
                                          ( uint8* )&frame_size );

            if (HYPV_STATUS_SUCCESS == rc)
            {
                fe_ioss->frame_size.height = frame_size.height;
                fe_ioss->frame_size.width = frame_size.width;
                HYP_VIDEO_MSG_INFO("set HEIC output width:%u height:%u in grid mode",
                                   fe_ioss->frame_size.width, fe_ioss->frame_size.height);
            }
            else
            {
                HYP_VIDEO_MSG_ERROR("failed to set frame size property. buf type %d",
                                    frame_size.buf_type);
                rc = HYPV_STATUS_FAIL;
            }
        }
    }

    return rc;

}

hypv_status_type enc_s_idr_period(fe_io_session_t *fe_ioss)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_idr_period_type payload;

    if ((VIDC_CODEC_HEVC == fe_ioss->session_codec.codec) ||
        (VIDC_CODEC_H264 == fe_ioss->session_codec.codec))
    {
       payload.idr_period = 1;
       HYP_VIDEO_MSG_INFO("Set IDR Period %u", payload.idr_period);
       rc = dec_enc_set_drv_property(fe_ioss,
                                    VIDC_I_ENC_IDR_PERIOD,
                                    sizeof(vidc_idr_period_type),
                                    ( uint8* )&payload);
       if (HYPV_STATUS_SUCCESS != rc)
       {
          HYP_VIDEO_MSG_ERROR("failed to set drv IDR period property %u",
                               payload.idr_period);
       }
    }

    return rc;

}

hypv_status_type enc_s_vpe_csc(fe_io_session_t* fe_ioss) {
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    if (((VIDC_CODEC_H264 == fe_ioss->session_codec.codec) ||
        (VIDC_CODEC_HEVC == fe_ioss->session_codec.codec)) &&
        (0 != fe_ioss->video_vpe_csc))
    {
        vidc_vpe_csc_type vpe_csc;

        HABMM_MEMSET(&vpe_csc, 0, sizeof(vidc_vpe_csc_type));

        vpe_csc.colour_primaries = fe_ioss->vui_video_signal_info.colour_primaries;
        vpe_csc.custom_matrix_enabled = fe_ioss->video_vpe_csc_custom_matrix;

        if (vpe_csc.custom_matrix_enabled)
        {
            uint32 count = 0;
            while (count < VIDC_MAX_MATRIX_COEFFS)
            {
                if (count < VIDC_MAX_BIAS_COEFFS)
                {
                    vpe_csc.csc_bias[count] = vpe_csc_custom_bias_coeff[count];
                    if (count < VIDC_MAX_LIMIT_COEFFS)
                    {
                        vpe_csc.csc_limit[count] = vpe_csc_custom_limit_coeff[count];
                    }
                    vpe_csc.csc_matrix[count] = vpe_csc_custom_matrix_coeff[count];
                }
                count = count + 1;
            }
        }
        rc = dec_enc_set_drv_property(fe_ioss, VIDC_I_VPE_CSC,
                         sizeof(vidc_vpe_csc_type),
                         ( uint8* )&vpe_csc);
        if (HYPV_STATUS_SUCCESS == rc)
        {
            HYP_VIDEO_MSG_INFO("Applied VIDC_I_VPE_CSC: colour_primaries %u"
                    " custom_matrix_enabled %u",
                    vpe_csc.colour_primaries, vpe_csc.custom_matrix_enabled);
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("Failed to apply VIDC_I_VPE_CSC");
            rc = HYPV_STATUS_FAIL;
        }
    }

    return rc;
}

hypv_status_type enc_s_video_signal_info(fe_io_session_t* fe_ioss)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    if ((VIDC_CODEC_H264 == fe_ioss->session_codec.codec) ||
        (VIDC_CODEC_HEVC == fe_ioss->session_codec.codec))
    {
        if ((int)fe_ioss->vui_video_signal_info.video_full_Range_flag == COLOR_RANGE_UNSPECIFIED &&
            fe_ioss->vui_video_signal_info.colour_primaries == MSM_VIDC_RESERVED_1)
        {
            fe_ioss->vui_video_signal_info.enable = false;
        }
        else
        {
            fe_ioss->vui_video_signal_info.enable = true;
            fe_ioss->vui_video_signal_info.video_format = VIDC_VIDEO_FORMAT_NTSC;
            fe_ioss->vui_video_signal_info.color_description_flag = true;
        }

        rc = dec_enc_set_drv_property(fe_ioss, VIDC_I_ENC_VUI_VIDEO_SIGNAL_INFO,
                         sizeof(vidc_vui_video_signal_info_type),
                         ( uint8* )&fe_ioss->vui_video_signal_info);
        if (HYPV_STATUS_SUCCESS == rc)
        {
            HYP_VIDEO_MSG_INFO("Applied VIDC_I_ENC_VUI_VIDEO_SIGNAL_INFO: enable %d"
                    " video_format %d color_description_flag %d colour_primaries %u"
                    " transfer_characteristics %u matrix_coeffs %u"
                    " fullrange %u",
                    fe_ioss->vui_video_signal_info.enable, fe_ioss->vui_video_signal_info.video_format,
                    fe_ioss->vui_video_signal_info.color_description_flag, fe_ioss->vui_video_signal_info.colour_primaries,
                    fe_ioss->vui_video_signal_info.transfer_characteristics, fe_ioss->vui_video_signal_info.matrix_coeffs,
                    fe_ioss->vui_video_signal_info.video_full_Range_flag);
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("Failed to apply VIDC_I_ENC_VUI_VIDEO_SIGNAL_INFO");
            rc = HYPV_STATUS_FAIL;
        }
    }

    return rc;
}

hypv_status_type enc_s_multi_slice(fe_io_session_t *fe_ioss)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_multi_slice_type *payload = &fe_ioss->enc_multi_slice_type;

    HYP_VIDEO_MSG_INFO("Set Multi Slice, mode %d size =%u",
            payload->slice_mode, payload->slice_size);

    if ((VIDC_MULTI_SLICE_OFF == payload->slice_mode) || (0 != payload->slice_size))
    {
        rc = dec_enc_set_drv_property(fe_ioss,
                                      VIDC_I_ENC_MULTI_SLICE,
                                      sizeof(vidc_multi_slice_type),
                                      ( uint8* )payload);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to set drv multi slice property."
                                " mode = 0x%x size = %u",
                                  (unsigned int)payload->slice_mode, payload->slice_size);
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION enc_s_blur_dimensions

@brief  Encoder FE set blur dimensions
@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type enc_s_blur_dimensions(fe_io_session_t* fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_blur_filter_type payload = {0};

    // translate blur value to the VIDC API
    switch(data->value)
    {
        case 0:
            payload.blur_value = VIDC_BLUR_ADAPTIVE;
            break;
        case 1:
            payload.blur_value = VIDC_BLUR_EXTERNAL;
            break;
        case 2:
            payload.blur_value = VIDC_BLUR_NONE;
            break;
        default:
            payload.blur_value = data->value;
            uint32 blur_width = (data->value >> 16) & 0xFFFF;
            uint32 blur_height = data->value & 0xFFFF;
            HYP_VIDEO_MSG_INFO("Blur resolution is %ux%u", blur_width, blur_height);
            break;
    }

    HYP_VIDEO_MSG_INFO("Set blur filter info with blur_value=0x%x", payload.blur_value);

    rc = dec_enc_set_drv_property(fe_ioss,
                                  VIDC_I_VPE_BLUR_FILTER,
                                  sizeof(vidc_blur_filter_type),
                                  ( uint8* )&payload);
    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("Failed to set blur filter info with blur_value=0x%x, "
                            "rc=%d", payload.blur_value, rc);
        rc = HYPV_STATUS_FAIL;
    }

    return rc;
}

/**===========================================================================

FUNCTION enc_s_hb_max_layer

@brief  Encoder FE set Max Hier-B Layer Count
@param [in] fe_ioss pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type enc_s_hb_max_layer(fe_io_session_t *fe_ioss)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    uint32 payload = fe_ioss->max_hier_layer_count;

    HYP_VIDEO_MSG_INFO("Requested Hier-B max layer: %u", payload);

    if ((MIN_HIER_LAYER_COUNT <= payload) &&
        (V4L2_MPEG_VIDEO_HEVC_HIERARCHICAL_CODING_B == fe_ioss->hier_layer_type))
    {
        rc = dec_enc_set_drv_property(fe_ioss,
                                      VIDC_I_ENC_HIER_B_MAX_NUM_ENH_LAYER,
                                      sizeof(uint32),
                                      ( uint8* )&payload);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to set drv property."
                                " Hier-B max layer: %u",
                                  payload);
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION enc_s_hp_max_layer

@brief  Encoder FE set Max Hier-P Layer Count
@param [in] fe_ioss pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type enc_s_hp_max_layer(fe_io_session_t *fe_ioss)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    uint32 payload = fe_ioss->max_hier_layer_count;

    HYP_VIDEO_MSG_INFO("Requested Hier-P max layer: %u", payload);

    if ((MIN_HIER_LAYER_COUNT <= payload) &&
        (V4L2_MPEG_VIDEO_HEVC_HIERARCHICAL_CODING_P == fe_ioss->hier_layer_type))
    {
        rc = dec_enc_set_drv_property(fe_ioss,
                                      VIDC_I_ENC_HIER_P_MAX_NUM_ENH_LAYER,
                                      sizeof(uint32),
                                      ( uint8* )&payload);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to set drv property."
                                " Hier-P max layer: %u",
                                  payload);
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION enc_s_hp_layer

@brief  Encoder FE set Hier-P Layer Count
@param [in] fe_ioss pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type enc_s_hp_layer(fe_io_session_t *fe_ioss)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    uint32 payload = fe_ioss->hier_layer_count;

    HYP_VIDEO_MSG_INFO("Requested Hier-P layer: %u", payload);

    if ((fe_ioss->max_hier_layer_count >= payload) &&
        (V4L2_MPEG_VIDEO_HEVC_HIERARCHICAL_CODING_P == fe_ioss->hier_layer_type))
    {
        rc = dec_enc_set_drv_property(fe_ioss,
                                        VIDC_I_ENC_HIER_P_ENH_LAYER,
                                        sizeof(uint32),
                                        ( uint8* )&payload);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to set drv property."
                                " Hier-P layer: %u",
                                  payload);
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION enc_s_bitrate_saving_mode

@brief  Encoder FE set bitrate saving mode

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/

hypv_status_type enc_s_bitrate_saving_mode(fe_io_session_t* fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_enable_type bitrate_saving;

    HABMM_MEMSET(&bitrate_saving, 0, sizeof(vidc_enable_type));

    HYP_VIDEO_MSG_INFO("enable bitrate saving mode %d, color format %d", data->value, fe_ioss->color_format_config.color_format);
    switch (data->value)
    {
        case V4L2_MPEG_VIDC_VIDEO_BRS_DISABLE:
          bitrate_saving.enable = FALSE;
          break;
        case V4L2_MPEG_VIDC_VIDEO_BRS_ENABLE_ALL:
          bitrate_saving.enable = TRUE;
          break;
        case V4L2_MPEG_VIDC_VIDEO_BRS_ENABLE_8BIT:
          if (VIDC_COLOR_FORMAT_NV12_P010 != fe_ioss->color_format_config.color_format &&
              VIDC_COLOR_FORMAT_YUV420_TP10_UBWC != fe_ioss->color_format_config.color_format)
          {
              bitrate_saving.enable = TRUE;
          }
          else
          {
              bitrate_saving.enable = FALSE;
              HYP_VIDEO_MSG_ERROR("color is format %d, can not enable bitrate saving 8bit mode", fe_ioss->color_format_config.color_format);
              rc = HYPV_STATUS_FAIL;
          }
          break;
        case V4L2_MPEG_VIDC_VIDEO_BRS_ENABLE_10BIT:
          if (VIDC_COLOR_FORMAT_NV12_P010 == fe_ioss->color_format_config.color_format ||
              VIDC_COLOR_FORMAT_YUV420_TP10_UBWC == fe_ioss->color_format_config.color_format)
          {
              bitrate_saving.enable = TRUE;
          }
          else
          {
              bitrate_saving.enable = FALSE;
              HYP_VIDEO_MSG_ERROR("color format is %d, can not enable bitrate saving 10bit mode", fe_ioss->color_format_config.color_format);
              rc = HYPV_STATUS_FAIL;
          }
          break;
        default:
          HYP_VIDEO_MSG_ERROR("not support bitrate saving mode %d", data->value);
          rc = HYPV_STATUS_FAIL;
    }
    if (HYPV_STATUS_SUCCESS == rc)
    {
        HYP_VIDEO_MSG_INFO("bitrate_saving.enable bitrate saving mode %d", bitrate_saving.enable);
        rc = dec_enc_set_drv_property(fe_ioss,
                                  VIDC_I_ENC_CONTENT_ADAPTIVE_CODING,
                                  sizeof(vidc_enable_type),
                                  ( uint8* )&bitrate_saving);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            rc = HYPV_STATUS_FAIL;
            HYP_VIDEO_MSG_ERROR("failed to enable bitrate saving %d",
                            bitrate_saving.enable);
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_enc_set_hdr_info

@brief  Encoder FE set HDR info

@param [in] fe_ioss pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/

hypv_status_type v4l2fe_enc_set_hdr_info(fe_io_session_t *fe_ioss)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_metadata_hdr_static_info payload;

    HABMM_MEMSET(&payload, 0, sizeof(vidc_metadata_hdr_static_info));

    payload.mastering_disp_colour_sei.display_primaries_x_0 = fe_ioss->hdr_static_info.mastering_disp_colour_sei.display_primaries_x_0;
    payload.mastering_disp_colour_sei.display_primaries_y_0 = fe_ioss->hdr_static_info.mastering_disp_colour_sei.display_primaries_y_0;
    payload.mastering_disp_colour_sei.display_primaries_x_1 = fe_ioss->hdr_static_info.mastering_disp_colour_sei.display_primaries_x_1;
    payload.mastering_disp_colour_sei.display_primaries_y_1 = fe_ioss->hdr_static_info.mastering_disp_colour_sei.display_primaries_y_1;
    payload.mastering_disp_colour_sei.display_primaries_x_2 = fe_ioss->hdr_static_info.mastering_disp_colour_sei.display_primaries_x_2;
    payload.mastering_disp_colour_sei.display_primaries_y_2 = fe_ioss->hdr_static_info.mastering_disp_colour_sei.display_primaries_y_2;

    payload.mastering_disp_colour_sei.white_point_x = fe_ioss->hdr_static_info.mastering_disp_colour_sei.white_point_x;
    payload.mastering_disp_colour_sei.white_point_y = fe_ioss->hdr_static_info.mastering_disp_colour_sei.white_point_y;

    payload.mastering_disp_colour_sei.max_display_mastering_luminance = fe_ioss->hdr_static_info.mastering_disp_colour_sei.max_display_mastering_luminance;
    payload.mastering_disp_colour_sei.min_display_mastering_luminance = fe_ioss->hdr_static_info.mastering_disp_colour_sei.min_display_mastering_luminance;

    payload.content_light_level_sei.max_pic_average_light_level = fe_ioss->hdr_static_info.content_light_level_sei.max_pic_average_light_level;
    payload.content_light_level_sei.max_content_light_level = fe_ioss->hdr_static_info.content_light_level_sei.max_content_light_level;

    if (TRUE == fe_ioss->hdr10_sei_enabled)
    {
        rc = dec_enc_set_drv_property(fe_ioss,
                                        VIDC_I_ENC_HDR_INFO,
                                        sizeof(vidc_metadata_hdr_static_info),
                                        ( uint8* )&payload);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to set HDR info");
        }
    }

    return rc;
}

/*========================================================================

*/
/** @file linux_video_fe.cpp

@par DESCRIPTION:
V4L2 Device driver hypervisor front-end implementation

@par FILE SERVICES:

@par EXTERNALIZED FUNCTIONS:
See below.

Copyright (c) 2016-2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*/
/*========================================================================*/
/*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---     -------------------------------------------------------
12/22/23    bh     Fix alloc-free-mismatch caused memory leakage issue
11/02/22    sk     Use flag and change header path for kernel 5.15
09/26/22    mm     Remove dependency OMX
02/14/22    sj     Synchronize lock handling
05/06/21    sj     Handle VIDIOC_QUERYMENU ioctl
10/12/20    sh     Enable CSC for H264/HEVC encoder
08/27/20    sh     Bringup video decode using codec2
05/26/20    sj     Process dequeue commands outside of critical region
01/28/20    sh     Add query control to enumerate supported profiles
04/24/19    sm     Multi stream buffer handling during reconfig
04/11/19    sm     Add V4l2 dequeue event and buffer API handling
03/14/19    sm     Generalize enc and dec command handling
03/04/19    sm     Add support for new v4l2 API
02/05/19    rz     Bringup changes for 8155
08/21/18    aw     Fix Klockwork P1, compilation and MISRA warning
05/08/18    sm     Add support for 10 bit playback
02/20/18    sm     Synchronize lock handling
01/18/18    sm     Add support for passthrough mode feature
11/29/17    sm     Add support for video input interrupts handling
10/12/17    sm     Simplify buffer management logics for dynamic alloc mode
06/28/17    aw     Unify and update all logs in hyp-video
06/23/17    sm     Streamline hypervisor context structure and definitions
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
#include <string.h>
#include "hyp_videopriv_fe.h"
#include "hyp_videopriv.h"
#include "hyp_vidc_types.h"
#include "hyp_vidc_inf.h"
#include "linux_video_fe.h"
#include "linux_vinput_fe.h"
#include "linux_venc_fe.h"
#include "linux_vdec_fe.h"
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
#include <linux/msm_ion.h>
#include <fcntl.h>
#ifdef SUPPORT_DMABUF
#include <vidc/media/msm_media_info.h>
#else
#include <media/msm_media_info.h>
#endif
#endif
#include "hyp_debug.h"

// ----------------------------------------------------------------------------

/**===========================================================================

FUNCTION ensure_free_buffer

@brief  Calls stream OFF to free all pending buffers

@param [in] fe_ioss pointer

@dependencies
  None

@return
  Returns void

===========================================================================*/
static void ensure_free_buffer(fe_io_session_t *fe_ioss)
{
    // In case buffer is not freed and is registered, need to free it by calling the streamoff
    if (NULL != fe_ioss->vidc_input_buffer_info)
    {
        dec_enc_streamoff(fe_ioss, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
    }
    if (NULL != fe_ioss->vidc_output_buffer_info)
    {
        dec_enc_streamoff(fe_ioss, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
    }
}

/**===========================================================================

FUNCTION plt_fe_open

@brief  Linux platform specific FE open

@param [in] name of the component
@param [in] flag
@param [out] fe_ioss pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type plt_fe_open(const char *str, int flag, fe_io_session_t *fe_ioss)
{

    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    hypv_callback_t hypv_cb;
    char device_name[MAX_DEVICE_NAME_LEN];
    fe_linux_plt_data_t *linux_plt_data;
    UNUSED(flag);

    /* Allocate platform specific data */
    linux_plt_data = (fe_linux_plt_data_t *)HABMM_MALLOC(sizeof(fe_linux_plt_data_t));
    if (linux_plt_data == NULL)
    {
        HYP_VIDEO_MSG_ERROR("failed to malloc linux platform data");
        return HYPV_STATUS_FAIL;
    }
    HABMM_MEMSET(linux_plt_data, 0, sizeof(fe_linux_plt_data_t));

    rc = hyp_queue_init(&linux_plt_data->evt_queue, sizeof(struct v4l2_event),
                        MAX_QUEUE_EVENTS);
    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to init event queue");
    }
    else
    {
        rc = hyp_queue_init(&linux_plt_data->evt_output_buf_queue,
                            sizeof(event_buf_type), MAX_QUEUE_EVENTS);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to init output buf queue");
            hyp_queue_deinit(&linux_plt_data->evt_queue);
        }
        else
        {
            rc = hyp_queue_init(&linux_plt_data->evt_input_buf_queue,
                                sizeof(event_buf_type), MAX_QUEUE_EVENTS);
            if (HYPV_STATUS_SUCCESS != rc)
            {
                HYP_VIDEO_MSG_ERROR("failed to init input buf queue");
                hyp_queue_deinit(&linux_plt_data->evt_queue);
                hyp_queue_deinit(&linux_plt_data->evt_output_buf_queue);
            }
        }
    }

    if (HYPV_STATUS_SUCCESS != rc)
    {
        free(linux_plt_data);
        return rc;
    }

    fe_ioss->fe_plt_data = (void *)linux_plt_data;
    fe_ioss->hyp_plt_id = HYP_PLATFORM_LA;

    if (0 == strcmp(str, "/dev/video32"))
    {
        fe_ioss->session_codec.session = VIDC_SESSION_DECODE;
        fe_ioss->video_session = VIDEO_SESSION_DECODE;
        snprintf(device_name, MAX_DEVICE_NAME_LEN, "%s", VDEC_DEVICE);
        hypv_cb.handler = (callback_handler_t)device_callback_dec;
        hypv_cb.data = fe_ioss;
    }
    else if (0 == strcmp(str, "/dev/video33"))
    {
        fe_ioss->session_codec.session = VIDC_SESSION_ENCODE;
        fe_ioss->video_session = VIDEO_SESSION_ENCODE;
        fe_ioss->vui_video_signal_info.video_full_Range_flag = COLOR_RANGE_UNSPECIFIED;
        fe_ioss->vui_video_signal_info.colour_primaries = MSM_VIDC_RESERVED_1;
        snprintf(device_name, MAX_DEVICE_NAME_LEN, "%s", VENC_DEVICE);
        hypv_cb.handler = (callback_handler_t)device_callback_enc;
        hypv_cb.data = fe_ioss;
    }
    else if (0 == strcmp(str, "/dev/video35"))
    {
        fe_ioss->video_session = VIDEO_SESSION_VINPUT;
        snprintf(device_name, MAX_DEVICE_NAME_LEN, "%s", VINPUT_DEVICE);
        hypv_cb.handler = (callback_handler_t)device_callback_vinput;
        hypv_cb.data = fe_ioss;
    }
    else
    {
        HYP_VIDEO_MSG_ERROR("failed: unknown device %s", str);
        rc = HYPV_STATUS_FAIL;
    }
    if (HYPV_STATUS_FAIL != rc)
    {
        fe_ioss->io_handle = hyp_device_open(device_name, &hypv_cb);
        if (fe_ioss->io_handle == NULL)
        {
            HYP_VIDEO_MSG_ERROR("device open fail io_handle is NULL!!!");
            rc = HYPV_STATUS_FAIL;
        }
        else
        {
            linux_plt_data->state = V4L2FE_STATE_LOADED;
        }
    }

    if (HYPV_STATUS_FAIL == rc)
    {
        hyp_queue_deinit(&linux_plt_data->evt_queue);
        hyp_queue_deinit(&linux_plt_data->evt_output_buf_queue);
        hyp_queue_deinit(&linux_plt_data->evt_input_buf_queue);
        HABMM_FREE(linux_plt_data);
        linux_plt_data = NULL;
    }

    return rc;
}

/**===========================================================================

FUNCTION plt_fe_ioctl

@brief  Linux platform specific FE ioctl

@param [in] handle
@param [in] cmd
@param [out] void data pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type plt_fe_ioctl(HVFE_HANDLE handle, uint32 cmd, void *data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    fe_io_session_t *fe_ioss = (fe_io_session_t *)handle;

    if (0 == fe_ioss)
    {
        HYP_VIDEO_MSG_ERROR("failed, handle is 0");
        return HYPV_STATUS_FAIL;
    }

    HYP_VIDEO_MSG_INFO("cmd is 0x%x", cmd);

    if (TRUE == fe_ioss->io_handle->passthrough_mode)
    {
        MM_CriticalSection_Enter(fe_ioss->lock_buffer);
        if ((VIDEO_SESSION_ENCODE == fe_ioss->video_session) ||
            (VIDEO_SESSION_DECODE == fe_ioss->video_session))
        {
            rc = v4l2fe_vdec_venc_ioctl_passthrough(fe_ioss, cmd, data);
        }
        else if (VIDEO_SESSION_VINPUT == fe_ioss->video_session)
        {
            rc = v4l2fe_vinput_ioctl_passthrough(fe_ioss, cmd, data);
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("failed, unknown session %d", fe_ioss->video_session);
            rc = HYPV_STATUS_FAIL;
        }
        MM_CriticalSection_Leave(fe_ioss->lock_buffer);

        return rc;
    }

    if (VIDIOC_DQEVENT == cmd)
    {
        if ((VIDEO_SESSION_ENCODE == fe_ioss->video_session) ||
            (VIDEO_SESSION_DECODE == fe_ioss->video_session))
        {
            rc = dec_enc_dqevent(fe_ioss, (struct v4l2_event *)data);
        }
        if (VIDEO_SESSION_VINPUT == fe_ioss->video_session)
        {
            rc = v4l2fe_vinput_dqevent(fe_ioss, (struct v4l2_event *)data);
        }
    }
    else if (VIDIOC_DQBUF == cmd)
    {
        if ((VIDEO_SESSION_ENCODE == fe_ioss->video_session) ||
            (VIDEO_SESSION_DECODE == fe_ioss->video_session))
        {
            rc = dec_enc_dqbuf(fe_ioss, (struct v4l2_buffer *)data);
        }
    }
    /* Keeping the complete function under lock increases the
    ** critical section eventually leading to inefficient
    ** thread usage. Bring dec_enc_qbuf call out of lock and
    ** introduce locks inside the function at appropriate
    ** places.
    */
    else if (VIDIOC_QBUF == cmd)
    {
        if ((VIDEO_SESSION_ENCODE == fe_ioss->video_session) ||
            (VIDEO_SESSION_DECODE == fe_ioss->video_session))
        {
            rc = dec_enc_qbuf(fe_ioss, (struct v4l2_buffer *)data);
        }
    }
    else
    {
        MM_CriticalSection_Enter(fe_ioss->lock_buffer);
        switch (cmd)
        {
            case VIDIOC_S_FMT:
            {
                if (VIDEO_SESSION_ENCODE == fe_ioss->video_session)
                {
                    rc = v4l2fe_enc_s_fmt(fe_ioss, (struct v4l2_format *)data);
                }
                else if (VIDEO_SESSION_DECODE == fe_ioss->video_session)
                {
                    rc = v4l2fe_dec_s_fmt(fe_ioss, (struct v4l2_format *)data);
                }
                break;
            }
            case VIDIOC_G_FMT:
            {
                if (VIDEO_SESSION_ENCODE == fe_ioss->video_session)
                {
                    rc = v4l2fe_enc_g_fmt(fe_ioss, (struct v4l2_format *)data);
                }
                else if (VIDEO_SESSION_DECODE == fe_ioss->video_session)
                {
                    rc = v4l2fe_dec_g_fmt(fe_ioss, (struct v4l2_format *)data);
                }
                else if (VIDEO_SESSION_VINPUT == fe_ioss->video_session)
                {
                    rc = v4l2fe_vinput_g_fmt(fe_ioss, (struct v4l2_format *)data);
                }
                break;
            }
            case VIDIOC_S_EXT_CTRLS:
            {
                if ((VIDEO_SESSION_ENCODE == fe_ioss->video_session) ||
                    (VIDEO_SESSION_DECODE == fe_ioss->video_session))
                {
                    rc = dec_enc_s_ext_ctrl(fe_ioss, (struct v4l2_ext_controls *)data);
                }
                break;
            }
            case VIDIOC_REQBUFS:
            {
                if (VIDEO_SESSION_ENCODE == fe_ioss->video_session)
                {
                    rc = v4l2fe_enc_reqbufs(fe_ioss, (struct v4l2_requestbuffers *)data);
                }
                else if (VIDEO_SESSION_DECODE == fe_ioss->video_session)
                {
                    rc = v4l2fe_dec_reqbufs(fe_ioss, (struct v4l2_requestbuffers *)data);
                }
                break;
            }
            case VIDIOC_S_CTRL:
            {
                if (VIDEO_SESSION_ENCODE == fe_ioss->video_session)
                {
                    rc = v4l2fe_enc_s_ctrl(fe_ioss, (struct v4l2_control *)data);
                }
                else if (VIDEO_SESSION_DECODE == fe_ioss->video_session)
                {
                    rc = v4l2fe_dec_s_ctrl(fe_ioss, (struct v4l2_control *)data);
                }
                break;
            }
            case VIDIOC_G_CTRL:
            {
                if (VIDEO_SESSION_ENCODE == fe_ioss->video_session)
                {
                    rc = v4l2fe_enc_g_ctrl(fe_ioss, (struct v4l2_control *)data);
                }
                else if (VIDEO_SESSION_DECODE == fe_ioss->video_session)
                {
                    rc = v4l2fe_dec_g_ctrl(fe_ioss, (struct v4l2_control *)data);
                }
                else if (VIDEO_SESSION_VINPUT == fe_ioss->video_session)
                {
                    rc = v4l2fe_vinput_g_ctrl(fe_ioss, (struct v4l2_control *)data);
                }
                break;
            }
            case VIDIOC_PREPARE_BUF:
            {
                if ((VIDEO_SESSION_ENCODE == fe_ioss->video_session) ||
                    (VIDEO_SESSION_DECODE == fe_ioss->video_session))
                {
                    rc = dec_enc_prepare_buf(fe_ioss, (struct v4l2_buffer *)data);
                }
                break;
            }
            case VIDIOC_STREAMON:
            {
                if ((VIDEO_SESSION_ENCODE == fe_ioss->video_session) ||
                    (VIDEO_SESSION_DECODE == fe_ioss->video_session))
                {
                    rc = dec_enc_streamon(fe_ioss, *((enum v4l2_buf_type *)data));
                }
                else if (VIDEO_SESSION_VINPUT == fe_ioss->video_session)
                {
                    rc = v4l2fe_vinput_streamon(fe_ioss, *((enum v4l2_buf_type *)data));
                }
                break;
            }
            case VIDIOC_SUBSCRIBE_EVENT:
            {
                if ((VIDEO_SESSION_ENCODE == fe_ioss->video_session) ||
                    (VIDEO_SESSION_DECODE == fe_ioss->video_session))
                {
                    rc = dec_enc_subscribe_event(fe_ioss, (struct v4l2_event_subscription *)data);
                }
                else if (VIDEO_SESSION_VINPUT == fe_ioss->video_session)
                {
                    rc = v4l2fe_vinput_subscribe_event(fe_ioss, (struct v4l2_event_subscription *)data);
                }
                break;
            }
            case VIDIOC_UNSUBSCRIBE_EVENT:
            {
                if ((VIDEO_SESSION_ENCODE == fe_ioss->video_session) ||
                    (VIDEO_SESSION_DECODE == fe_ioss->video_session))
                {
                    rc = dec_enc_unsubscribe_event(fe_ioss, (struct v4l2_event_subscription *)data);
                }
                else if (VIDEO_SESSION_VINPUT == fe_ioss->video_session)
                {
                    rc = v4l2fe_vinput_unsubscribe_event(fe_ioss, (struct v4l2_event_subscription *)data);
                }
                break;
            }
            case VIDIOC_QUERYCAP:
            {
                if ((VIDEO_SESSION_ENCODE == fe_ioss->video_session) ||
                    (VIDEO_SESSION_DECODE == fe_ioss->video_session))
                {
                    rc = dec_enc_querycap(fe_ioss, (struct v4l2_capability *)data);
                }
                break;
            }
            case VIDIOC_QUERYCTRL:
            {
                if (VIDEO_SESSION_ENCODE == fe_ioss->video_session)
                {
                    rc = v4l2fe_enc_queryctrl(fe_ioss, (struct v4l2_queryctrl *)data);
                }
                else if (VIDEO_SESSION_DECODE == fe_ioss->video_session)
                {
                    rc = v4l2fe_dec_queryctrl(fe_ioss, (struct v4l2_queryctrl *)data);
                }
                break;
            }
            case VIDIOC_QUERYMENU:
            {
                if (VIDEO_SESSION_DECODE == fe_ioss->video_session)
                {
                   rc = v4l2fe_dec_querymenu((struct v4l2_querymenu *)data);
                }
                else if (VIDEO_SESSION_ENCODE == fe_ioss->video_session)
                {
                   rc = v4l2fe_enc_querymenu((struct v4l2_querymenu *)data);
                }
                break;
            }
            case VIDIOC_ENUM_FMT:
            {
                if (VIDEO_SESSION_ENCODE == fe_ioss->video_session)
                {
                    rc = v4l2fe_enc_enum_fmt(fe_ioss, (struct v4l2_fmtdesc *)data);
                }
                else if (VIDEO_SESSION_DECODE == fe_ioss->video_session)
                {
                    rc = v4l2fe_dec_enum_fmt(fe_ioss, (struct v4l2_fmtdesc *)data);
                }
                break;
            }
            case VIDIOC_DECODER_CMD:
            case VIDIOC_ENCODER_CMD:
            {
                if ((VIDEO_SESSION_ENCODE == fe_ioss->video_session) ||
                    (VIDEO_SESSION_DECODE == fe_ioss->video_session))
                {
                    rc = dec_enc_cmd(fe_ioss, (struct v4l2_decoder_cmd *)data);
                }
                break;
            }
            case VIDIOC_STREAMOFF:
            {
                if ((VIDEO_SESSION_ENCODE == fe_ioss->video_session) ||
                    (VIDEO_SESSION_DECODE == fe_ioss->video_session))
                {
                    rc = dec_enc_streamoff(fe_ioss, *(enum v4l2_buf_type *)data);
                }
                else if (VIDEO_SESSION_VINPUT == fe_ioss->video_session)
                {
                    rc = v4l2fe_vinput_streamoff(fe_ioss, *((enum v4l2_buf_type *)data));
                }
                break;
            }
            case VIDIOC_S_PARM:
            {
                if (VIDEO_SESSION_ENCODE == fe_ioss->video_session)
                {
                    rc = v4l2fe_enc_s_parm(fe_ioss, (struct v4l2_streamparm *)data);
                }
                else if (VIDEO_SESSION_DECODE == fe_ioss->video_session)
                {
                    rc = v4l2fe_dec_s_parm(fe_ioss, (struct v4l2_streamparm *)data);
                }
                break;
            }
            case VIDIOC_ENUM_FRAMESIZES:
            {
                if ((VIDEO_SESSION_ENCODE == fe_ioss->video_session) ||
                    (VIDEO_SESSION_DECODE == fe_ioss->video_session))
                {
                    rc = dec_enc_enum_framesizes(fe_ioss, (struct v4l2_frmsizeenum *)data);
                }
                break;
            }
            case VIDIOC_S_INPUT:
            {
                if (VIDEO_SESSION_VINPUT == fe_ioss->video_session)
                {
                    rc = v4l2fe_vinput_s_input(fe_ioss, (int *)data);
                }
                break;
            }
            case VIDIOC_G_INPUT:
            {
                if (VIDEO_SESSION_VINPUT == fe_ioss->video_session)
                {
                    rc = v4l2fe_vinput_g_input(fe_ioss, (int *)data);
                }
                break;
            }
            case VIDIOC_ENUMINPUT:
            {
                if (VIDEO_SESSION_VINPUT == fe_ioss->video_session)
                {
                    rc = v4l2fe_vinput_enuminput(fe_ioss, (struct v4l2_input *)data);
                }
                break;
            }
            default:
            {
                HYP_VIDEO_MSG_ERROR("unsupported cmd 0x%x", cmd);
                rc = HYPV_STATUS_FAIL;
                break;
            }
        }
        MM_CriticalSection_Leave(fe_ioss->lock_buffer);
    }

    return rc;
}

/**===========================================================================

FUNCTION plt_fe_close

@brief  Linux platform specific FE close

@param [in] handle

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type plt_fe_close(HVFE_HANDLE handle)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    fe_io_session_t *fe_ioss = (fe_io_session_t *)handle;
    fe_linux_plt_data_t *plt_data = (fe_linux_plt_data_t *)fe_ioss->fe_plt_data;

    if ((VIDEO_SESSION_ENCODE == fe_ioss->video_session) ||
        (VIDEO_SESSION_DECODE == fe_ioss->video_session))
    {
        ensure_free_buffer(fe_ioss);
    }
    rc = hyp_device_close(fe_ioss->io_handle);

    hyp_queue_deinit(&plt_data->evt_queue);
    hyp_queue_deinit(&plt_data->evt_input_buf_queue);
    hyp_queue_deinit(&plt_data->evt_output_buf_queue);

    HABMM_FREE(fe_ioss->fe_plt_data);
    fe_ioss->fe_plt_data = NULL;

    return rc;
}

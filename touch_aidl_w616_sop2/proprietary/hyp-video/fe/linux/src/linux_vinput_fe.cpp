/*========================================================================

*//** @file linux_vinput_fe.cpp

@par DESCRIPTION:
Linux video input device driver hypervisor front-end implementation

@par FILE SERVICES:

@par EXTERNALIZED FUNCTIONS:
See below.

Copyright (c) 2017-2019, 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/
/*========================================================================
                             Edit History

$Header: //deploy/qcom/qct/platform/qnp/qnx/auto/components/rel/vm_video.qxa_qa/2.0/fe/linux/src/linux_vinput_fe.c#1 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
08/30/23   nb      Fix compilation errors due to additon of new compiler flags
02/05/19   rz      Bringup changes for 8155
06/12/18   sm      Use a common macro for memset
01/18/18   sm      Add support for passthrough mode feature
11/29/17   sm      Add support for video input interrupts handling
08/11/17   sm      Move to safer APIs
06/28/17   aw      Unify and update all logs in hyp-video
05/08/17   sm      Update for new hyp-video architecture
04/03/17   sm      Add support for Video input FE-BE
========================================================================== */

/* =======================================================================

INCLUDE FILES FOR MODULE

========================================================================== */
#include "hyp_vidc_inf.h"
#include "hyp_vinput_inf.h"
#include "hyp_vinput_types.h"
#include "hyp_videopriv_fe.h"
#include "hyp_videopriv.h"
#include "hyp_debug.h"
#include "linux_video_fe.h"
#include "linux_vinput_fe.h"

/**===========================================================================

FUNCTION device_callback_vinput

@brief  Hypervisor vinput FE callback function

@param [in] msg pointer
@param [in] lenght
@param [in] void pointer for private data

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type device_callback_vinput(uint8 *msg, uint32 length, void *cd)
{
    vidc_drv_msg_info_type *pEvent = (vidc_drv_msg_info_type *)msg;
    fe_io_session_t *fe_ioss = (fe_io_session_t *)cd;
    struct v4l2_event event;
    boolean valid_event = TRUE;
    int port = pEvent->payload.event_data_1;
    UNUSED(length);

    HABMM_MEMSET(&event, 0, sizeof(struct v4l2_event));

    if (TRUE == fe_ioss->io_handle->passthrough_mode)
    {
        fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &port);
        return HYPV_STATUS_SUCCESS;
    }

    HYP_VIDEO_MSG_INFO("event 0x%x status 0x%x port %d", (unsigned int)pEvent->event_type, pEvent->status, port);
    switch ((uint32)pEvent->event_type)
    {
    case AVIN_EVT_PORT_CONNECTED:
        {
#ifdef VINPUT_SUPPORT
            if (0 == port)
            {
                event.type = V4L2_EVENT_MSM_BA_SIGNAL_IN_LOCK;
            }
            else if (1 == port)
            {
                event.u.data[1] = 1;
                event.type = V4L2_EVENT_MSM_BA_PORT_SETTINGS_CHANGED;
            }
            else
            {
                valid_event = FALSE;
                HYP_VIDEO_MSG_INFO("unknown port %d", port);
            }
#else
            event.type = 0;
            HYP_VIDEO_MSG_ERROR("NOT supported");
#endif
            break;
        }
    case AVIN_EVT_PORT_DISCONNECTED:
        {
#ifdef VINPUT_SUPPORT
            if (0 == port)
            {
                event.type = V4L2_EVENT_MSM_BA_SIGNAL_LOST_LOCK;
            }
            else if (1 == port)
            {
                event.u.data[1] = 0;
                event.type = V4L2_EVENT_MSM_BA_CABLE_DETECT;
            }
            else
            {
                valid_event = FALSE;
                HYP_VIDEO_MSG_INFO("unknown port id %d", port);
            }
#else
            event.type = 0;
            HYP_VIDEO_MSG_ERROR("NOT supported");
#endif
            break;
        }
    case AVIN_EVT_PORT_SETTINGS_CHANGED:
        {
#ifdef VINPUT_SUPPORT
            event.type = V4L2_EVENT_MSM_BA_PORT_SETTINGS_CHANGED;
#endif
            break;
        }
    case AVIN_EVT_CABLE_DETECT:
        {
#ifdef VINPUT_SUPPORT
           event.type = V4L2_EVENT_MSM_BA_CABLE_DETECT;
#endif
           break;
        }

    default:
        {
            HYP_VIDEO_MSG_ERROR("unknown event 0x%x", (unsigned int)pEvent->event_type);
            valid_event = FALSE;
            break;
        }
    }

    if (valid_event)
    {
        event.u.data[0] = port;
        event.pending = 1;
        HABMM_MEMCPY((void *)fe_ioss->event_buffer, (const void *)&event, sizeof(struct v4l2_event));
        fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &port);
    }

    return HYPV_STATUS_SUCCESS;
}

/**===========================================================================

FUNCTION vinput_ioctl_passthrough

@brief  Video input ioctl passthrough

@param [in] fe_ioss
@param [in] cmd
@param [in] data point

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_vinput_ioctl_passthrough(fe_io_session_t *fe_ioss, uint32 cmd, void* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    uint32 size = 0;

    switch (cmd)
    {
    case VIDIOC_G_FMT:
        {
            size = sizeof(struct v4l2_format);
            break;
        }
    case VIDIOC_G_CTRL:
        {
            size = sizeof(struct v4l2_control);
            break;
        }
    case VIDIOC_STREAMON:
        {
            size = sizeof(enum v4l2_buf_type);
            break;
        }
    case VIDIOC_SUBSCRIBE_EVENT:
        {
            size = sizeof(struct v4l2_event_subscription);
            break;
        }
    case VIDIOC_UNSUBSCRIBE_EVENT:
        {
            size = sizeof(struct v4l2_event_subscription);
            break;
        }
    case VIDIOC_STREAMOFF:
        {
            size = sizeof(enum v4l2_buf_type);
            break;
        }
    case VIDIOC_S_INPUT:
        {
            size = sizeof(int);
            break;
        }
    case VIDIOC_G_INPUT:
        {
            size = sizeof(int);
            break;
        }
    case VIDIOC_ENUMINPUT:
        {
            size = sizeof(struct v4l2_input);
            break;
        }
    case VIDIOC_DQEVENT:
        {
            size = sizeof(struct v4l2_event);
            break;
        }
     default:
            HYP_VIDEO_MSG_ERROR("vinput cmd not found %u", cmd);
            break;

    }

    if (size)
    {
        rc = hyp_device_ioctl(fe_ioss->io_handle, cmd, (uint8 *)data, size, (uint8 *)data, size);
    }

    return rc;
}

/**===========================================================================

FUNCTION set_vinput_drv_property

@brief  Set vinput driver property

@param [in] fe_ioss pointer
@param [in] property Id
@param [in] packet size
@param [in] packet pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
#ifdef VINPUT_SUPPORT
static hypv_status_type set_vinput_drv_property
(
 fe_io_session_t*        fe_ioss,
 avin_property_id_type   propId,
 uint32                  nPktSize,
 uint8*                  pPkt
 )
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    avin_property_hdr_type* pProp = (avin_property_hdr_type* )fe_ioss->dev_cmd_buffer;
    int32 nMsgSize = sizeof(avin_property_hdr_type) + nPktSize;
    UNUSED(pPkt);

    pProp->size    = nPktSize;
    pProp->prop_id = propId;

    if (HYPV_STATUS_SUCCESS != (rc = hyp_device_ioctl(fe_ioss->io_handle,
        AVIN_IOCTL_SET_PROPERTY,
        fe_ioss->dev_cmd_buffer,
        nMsgSize,
        NULL,
        0)))
    {
        HYP_VIDEO_MSG_ERROR("id %d failed rc %d", propId, rc);
        rc = HYPV_STATUS_FAIL;
    }

    return rc;
}
#endif
/**===========================================================================

FUNCTION get_vinput_drv_property

@brief  Get vinput driver property

@param [in] fe_ioss pointer
@param [in] property Id
@param [in] port
@param [in] packet size
@param [in] packet pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type get_vinput_drv_property
(
 fe_io_session_t*        fe_ioss,
 avin_property_id_type   propId,
 uint32                  port,
 uint32                  nPktSize,
 uint8*                  pPkt
 )
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    avin_property_hdr_type* pProp = (avin_property_hdr_type* )fe_ioss->dev_cmd_buffer;
    int32 nMsgSize = sizeof(avin_property_hdr_type) + nPktSize;

    pProp->size    = nPktSize;
    pProp->prop_id = propId;
    pProp->port = port;

    if (HYPV_STATUS_SUCCESS != (rc = hyp_device_ioctl(fe_ioss->io_handle,
        AVIN_IOCTL_GET_PROPERTY,
        fe_ioss->dev_cmd_buffer,
        nMsgSize,
        pPkt,
        nPktSize)))
    {
        HYP_VIDEO_MSG_ERROR("id %d failed rc %d", propId, rc);
        rc = HYPV_STATUS_FAIL;
    }

    return rc;
}

/**===========================================================================

FUNCTION submit_command

@brief  Submit an vinput command from video FE

@param [in] fe_ioss pointer
@param [in] cmd
@param [in] data pointer
@param [in] data size

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type submit_command(fe_io_session_t* fe_ioss, uint32 vidc_cmd, uint8* data, uint32 size)
{
    return hyp_device_ioctl(fe_ioss->io_handle, vidc_cmd, data, size, NULL, 0);
}

/**===========================================================================

FUNCTION v4l2fe_vinput_g_fmt

@brief  Vinput FE get format

@param [in] fe_ioss pointer
@param [in] v4l2_format pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_vinput_g_fmt(fe_io_session_t* fe_ioss, struct v4l2_format* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == data->type)
    {
        avin_property_port_resolution_type resolution;
        HABMM_MEMSET(&resolution, 0, sizeof(avin_property_port_resolution_type));
        rc = get_vinput_drv_property(fe_ioss,
            AVIN_PROP_PORT_RESOLUTION,
            0,
            sizeof(avin_property_port_resolution_type),
            ( uint8* )&resolution);
        if (HYPV_STATUS_SUCCESS != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to get drv property - AVIN_PROP_PORT_RESOLUTION");
        }
        else
        {
            avin_property_port_clrfmt_type colorformat;
            HABMM_MEMSET(&colorformat, 0, sizeof(avin_property_port_clrfmt_type));
            rc = get_vinput_drv_property(fe_ioss,
                AVIN_PROP_PORT_COLORFMT,
                0,
                sizeof(avin_property_port_clrfmt_type),
                ( uint8* )&colorformat);
            if (HYPV_STATUS_SUCCESS != rc)
            {
                HYP_VIDEO_MSG_ERROR("failed to get drv property - AVIN_PROP_PORT_COLORFMT");
            }
            else
            {
                data->fmt.pix.pixelformat = colorformat.clr_fmt;
                data->fmt.pix.width = resolution.res.width;
                data->fmt.pix.height = resolution.res.height;
            }
        }
    }
    else
    {
        HYP_VIDEO_MSG_ERROR("unsupported type");
        rc = HYPV_STATUS_FAIL;
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_vinput_g_ctrl

@brief  Vinput FE get control

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_vinput_g_ctrl(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    UNUSED(fe_ioss);

    HYP_VIDEO_MSG_INFO("enter");
    data->value = 0;

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_vinput_streamon

@brief  Vinput stream ON

@param [in] fe_ioss pointer
@param [in] v4l2_buf_type pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_vinput_streamon(fe_io_session_t* fe_ioss, enum v4l2_buf_type data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    UNUSED(data);

    HYP_VIDEO_MSG_INFO("enter");
    rc = submit_command(fe_ioss, AVIN_IOCTL_START, NULL, 0);

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_vinput_streamoff

@brief  Vinput stream OFF function

@param [in] fe_ioss pointer
@param [in] v4l2_buf_type

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_vinput_streamoff(fe_io_session_t* fe_ioss, enum v4l2_buf_type data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    UNUSED(data);

    HYP_VIDEO_MSG_INFO("enter");
    rc = submit_command(fe_ioss, AVIN_IOCTL_STOP, NULL, 0);

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_vinput_subscribe_event

@brief  Vinput subscribe for an event

@param [in] fe_ioss pointer
@param [in] v4l2_event_subscription pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_vinput_subscribe_event(fe_io_session_t* fe_ioss, struct v4l2_event_subscription* data)
{

    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    UNUSED(fe_ioss);

    HYP_VIDEO_MSG_INFO("event type %u id %u",data->type, data->id);
    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_vinput_unsubscribe_event

@brief  Vinput unsubscribe an event

@param [in] fe_ioss pointer
@param [in] v4l2_event_subscription pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_vinput_unsubscribe_event(fe_io_session_t* fe_ioss, struct v4l2_event_subscription* data)
{

    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    UNUSED(fe_ioss);

    HYP_VIDEO_MSG_INFO("event type %u id %u",data->type, data->id);
    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_vinput_s_input

@brief  Vinput set input

@param [in] fe_ioss pointer
@param [in] input data pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_vinput_s_input(fe_io_session_t* fe_ioss, int* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    HYP_VIDEO_MSG_INFO("input %d", *data);
    rc = submit_command(fe_ioss, AVIN_IOCTL_CONNECT, (uint8 *)data, sizeof(uint32));

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_vinput_g_input

@brief  Vinput get input

@param [in] fe_ioss pointer
@param [out] return data pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_vinput_g_input(fe_io_session_t* fe_ioss, int* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    UNUSED(fe_ioss);

    *data = 0;

    return rc;

}

/**===========================================================================

FUNCTION v4l2fe_vinput_enuminput

@brief  Vinput enumerate an input

@param [in] fe_ioss pointer
@param [out] v4l2_input pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_vinput_enuminput(fe_io_session_t* fe_ioss, struct v4l2_input* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    UNUSED(fe_ioss);

    HYP_VIDEO_MSG_INFO("enter");
    avin_property_port_connect_type connect;
    HABMM_MEMSET(&connect, 0, sizeof(avin_property_port_connect_type));
    rc = get_vinput_drv_property(fe_ioss,
            AVIN_PROP_PORT_CONNECT_TYPE,
            data->index,
            sizeof(avin_property_port_connect_type),
            (uint8 *)&connect);
    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to get drv property - AVIN_PROP_PORT_CONNECT_TYPE");
    }
    else
    {
        switch(connect.connect_type)
        {
        case AVIN_PORT_TYPE_ANALOG:
            HABMM_MEMCPY((char *)data->name, "CVBS", 5);
            break;
        case AVIN_PORT_TYPE_HDMI:
            HABMM_MEMCPY((char *)data->name, "HDMI", 5);
            break;
        default:
            rc = HYPV_STATUS_FAIL;
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_vinput_dqevent

@brief  Vinput dqueue event

@param [in] fe_ioss pointer
@param [in] v4l2_event pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_vinput_dqevent(fe_io_session_t* fe_ioss, struct v4l2_event* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    struct v4l2_event *event = (struct v4l2_event *)fe_ioss->event_buffer;

    if (event->pending)
    {
        event->pending = 0;
        HABMM_MEMCPY((void *)data, (const void *)event, sizeof(struct v4l2_event));
        HYP_VIDEO_MSG_INFO("event type %u", event->type);
    }
    else
    {
        rc = HYPV_STATUS_FAIL;
        HYP_VIDEO_MSG_INFO("No pending events");
    }

    return rc;
}

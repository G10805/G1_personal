/*========================================================================

*//** @file hyp_vinput_test.c

@par DESCRIPTION:
Video input hypervisor unit test

@par FILE SERVICES:

@par EXTERNALIZED FUNCTIONS:
See below.

Copyright (c) 2017, 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/
/*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---     -------------------------------------------------------
08/30/23   nb      Fix compilation errors due to additon of new compiler flags
07/25/17   sm      Fix LA logging
06/28/17   aw      Unify and update all logs in hyp-video
05/08/17   sm      Update for new hyp-video architecture
04/19/17   sm      Add unit test for video input
========================================================================== */

#include <stdio.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include "hyp_debug.h"
#include "hyp_vinput_types.h"
#include "hyp_video_fe.h"
#include "hyp_video_test.h"

#define DEVICE "/dev/video35"

/**===========================================================================

FUNCTION vinput_callback

@brief  vinput test callback function

@param [in] void pointer (context)
@param [in] void pointer (message)

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static int vinput_callback(void *context, void *message)
{
    (void)context;
    (void)message;

    HYP_VIDEO_MSG_INFO("enter");

    return 0;
}

/**===========================================================================

FUNCTION vinput_open

@brief  vinput test open function

@dependencies
  None

@return
  Returns HVFE_HANDLE

===========================================================================*/
static HVFE_HANDLE vinput_open(void)
{
    int ret = 0;
    HVFE_HANDLE v4l2_handle;
    hvfe_callback_t v4l2cb;

    v4l2cb.handler = vinput_callback;

    v4l2_handle = video_fe_open(DEVICE, O_RDWR, &v4l2cb);
    if (!v4l2_handle)
    {
        HYP_VIDEO_MSG_ERROR("open failed for device %s", DEVICE);
        ret = 1;
    }

    HYP_VIDEO_TEST_RESULT("open", ret);

    return v4l2_handle;
}

/**===========================================================================

FUNCTION vinput_ioctl

@brief  vinput test ioctl function

@param [in] HVFE_HANDLE
@dependencies
  None

@return
  Returns void

===========================================================================*/
static void vinput_ioctl(HVFE_HANDLE v4l2_handle)
{
    int ret = 0;
    struct v4l2_format format;
    struct v4l2_input input;
    enum v4l2_buf_type buf_type;
    struct v4l2_control control;
    struct v4l2_event_subscription event;
    int input_id = 0;
    int max_input = 0, index = 0;

    for (index = 0; !ret; index++)
    {
        input.index = index;
        ret = video_fe_ioctl(v4l2_handle, VIDIOC_ENUMINPUT, &input);
        if (ret)
            HYP_VIDEO_MSG_ERROR("VIDIOC_ENUMINPUT for device %s failed with error %d", DEVICE, ret);
        else
            HYP_VIDEO_MSG_INFO("VIDIOC_ENUMINPUT: index %u, name %s", input.index, input.name);
    }
    if (input.index)
    {
        max_input = input.index;
        ret = 0;
    }
    HYP_VIDEO_TEST_RESULT("VIDIOC_ENUMINPUT", ret);

    ret = 0;
    ret = video_fe_ioctl(v4l2_handle, VIDIOC_G_FMT, &format);
    if (ret)
    {
        HYP_VIDEO_MSG_ERROR("VIDIOC_G_FMT for device %s failed with error %d", DEVICE, ret);
    }
    else
    {
        HYP_VIDEO_MSG_INFO("VIDIOC_G_FMT: pixelformat %u, width %u height %u", format.fmt.pix.pixelformat,
            format.fmt.pix.width, format.fmt.pix.height);
    }
    HYP_VIDEO_TEST_RESULT("VIDIOC_G_FMT", ret);

    ret = 0;
    memset(&event, 0, sizeof(struct v4l2_event_subscription));
    ret = video_fe_ioctl(v4l2_handle, VIDIOC_SUBSCRIBE_EVENT, &event);
    if (ret)
    {
        HYP_VIDEO_MSG_ERROR("VIDIOC_SUBSCRIBE_EVENT for device %s failed with error %d", DEVICE, ret);
    }
    HYP_VIDEO_TEST_RESULT("VIDIOC_SUBSCRIBE_EVENT", ret);

    ret = 0;
    memset(&event, 0, sizeof(struct v4l2_event_subscription));
    ret = video_fe_ioctl(v4l2_handle, VIDIOC_UNSUBSCRIBE_EVENT, &event);
    if (ret)
    {
        HYP_VIDEO_MSG_ERROR("VIDIOC_UNSUBSCRIBE_EVENT for device %s failed with error %d", DEVICE, ret);
    }
    HYP_VIDEO_TEST_RESULT("VIDIOC_UNSUBSCRIBE_EVENT", ret);

    ret = 0;
    ret = video_fe_ioctl(v4l2_handle, VIDIOC_G_CTRL, &control);
    if (ret)
    {
        HYP_VIDEO_MSG_ERROR("VIDIOC_G_CTRL for device %s failed with error %d", DEVICE, ret);
    }
    else
    {
        HYP_VIDEO_MSG_INFO("VIDIOC_G_CTRL: index %d", control.value);
    }
    HYP_VIDEO_TEST_RESULT("VIDIOC_G_CTRL", ret);

    ret = 0;
    ret = video_fe_ioctl(v4l2_handle, VIDIOC_G_INPUT, &input_id);
    if (ret)
    {
        HYP_VIDEO_MSG_ERROR("VIDIOC_G_INPUT for device %s failed with error %d", DEVICE, ret);
    }
    else
    {
        HYP_VIDEO_MSG_INFO("VIDIOC_G_INPUT: input id %d", input_id);
    }
    HYP_VIDEO_TEST_RESULT("VIDIOC_G_INPUT", ret);

    ret = 0;
    if (max_input == 0)
    {
        ret = -1;
    }
    else
    {
        char test_name[MAX_TEST_NAME_LEN];
        for (index = 0; index < max_input; index++)
        {
            ret |= video_fe_ioctl(v4l2_handle, VIDIOC_S_INPUT, &index);
            if (ret)
            {
                HYP_VIDEO_MSG_ERROR("VIDIOC_S_INPUT for device %s failed with error %d", DEVICE, ret);
            }
            else
            {
                HYP_VIDEO_MSG_INFO("VIDIOC_S_INPUT: input %d", index);
            }
            snprintf(test_name, MAX_TEST_NAME_LEN, "VIDIOC_S_INPUT_%d", index);
            HYP_VIDEO_TEST_RESULT("VIDIOC_S_INPUT", ret);

            if (ret == 0)
            {
                ret = video_fe_ioctl(v4l2_handle, VIDIOC_STREAMON, &buf_type);
                if (ret)
                {
                    HYP_VIDEO_MSG_ERROR("VIDIOC_STREAMON for device %s failed with error %d", DEVICE, ret);
                }
            }
            snprintf(test_name, MAX_TEST_NAME_LEN, "VIDIOC_STREAMON_%d", index);
            HYP_VIDEO_TEST_RESULT(test_name, ret);

            if (ret == 0)
            {
                ret = video_fe_ioctl(v4l2_handle, VIDIOC_STREAMOFF, &buf_type);
                if (ret)
                {
                    HYP_VIDEO_MSG_ERROR("VIDIOC_STREAMOFF for device %s failed with error %d", DEVICE, ret);
                }
            }
            snprintf(test_name, MAX_TEST_NAME_LEN, "VIDIOC_STREAMOFF_%d", index);
            HYP_VIDEO_TEST_RESULT(test_name, ret);
        }
    }

}

/**===========================================================================

FUNCTION vinput_close

@brief  vinput test close function

@param [in] HVFE_HANDLE
@dependencies
  None

@return
  Returns void

===========================================================================*/
static void vinput_close(HVFE_HANDLE v4l2_handle)
{
    int ret = 0;

    ret = video_fe_close(v4l2_handle);
    if (ret)
    {
        HYP_VIDEO_MSG_ERROR("close failed for device %s with error %d", DEVICE, ret);
    }

    HYP_VIDEO_TEST_RESULT("close", ret);

    return;
}

int vinput_main(void)
{
    HVFE_HANDLE v4l2_handle = NULL;

    v4l2_handle = vinput_open();
    if (v4l2_handle)
    {
        vinput_ioctl(v4l2_handle);
        vinput_close(v4l2_handle);
    }

    return 0;
}

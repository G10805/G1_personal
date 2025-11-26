/*========================================================================

*//** @file linux_vinput_fe.h

@par FILE SERVICES:
      Linux video input driver hypervisor front-end implementation header


@par EXTERNALIZED FUNCTIONS:
      See below.

Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---     -------------------------------------------------------
01/18/18    sm     Add support for passthrough mode feature
11/29/17    sm     Add support for video input interrupts handling
05/08/17    sm     Update for new hyp-video architecture
04/03/17    sm     Add support for Video input FE-BE
========================================================================== */

/* =======================================================================

INCLUDE FILES FOR MODULE

========================================================================== */

#ifndef __LINUX_VINPUT_FE_H__
#define __LINUX_VINPUT_FE_H__

#include "hyp_videopriv_fe.h"
#include <linux/videodev2.h>

hypv_status_type device_callback_vinput(uint8 *msg, uint32 length, void *cd);
hypv_status_type v4l2fe_vinput_g_fmt(fe_io_session_t* fe_ioss, struct v4l2_format* data);
hypv_status_type v4l2fe_vinput_g_ctrl(fe_io_session_t *fe_ioss, struct v4l2_control* data);
hypv_status_type v4l2fe_vinput_streamon(fe_io_session_t* fe_ioss, enum v4l2_buf_type data);
hypv_status_type v4l2fe_vinput_streamoff(fe_io_session_t* fe_ioss, enum v4l2_buf_type data);
hypv_status_type v4l2fe_vinput_subscribe_event(fe_io_session_t* fe_ioss, struct v4l2_event_subscription* data);
hypv_status_type v4l2fe_vinput_unsubscribe_event(fe_io_session_t* fe_ioss, struct v4l2_event_subscription* data);
hypv_status_type v4l2fe_vinput_s_input(fe_io_session_t* fe_ioss, int* data);
hypv_status_type v4l2fe_vinput_g_input(fe_io_session_t* fe_ioss, int* data);
hypv_status_type v4l2fe_vinput_enuminput(fe_io_session_t* fe_ioss, struct v4l2_input* data);
hypv_status_type v4l2fe_vinput_dqevent(fe_io_session_t* fe_ioss, struct v4l2_event* data);
hypv_status_type v4l2fe_vinput_ioctl_passthrough(fe_io_session_t* fe_ioss, uint32 cmd, void* data);

#endif

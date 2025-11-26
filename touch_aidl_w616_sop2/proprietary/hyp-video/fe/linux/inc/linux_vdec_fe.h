/*========================================================================

*//** @file linux_vdec_fe.h

@par FILE SERVICES:
      Linux video decoder hypervisor front-end implementation header


@par EXTERNALIZED FUNCTIONS:
      See below.

Copyright (c) 2017-2021 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---     -------------------------------------------------------
05/06/21    sj     Add query menu to enumerate supported profiles
08/27/20    sh     Bringup video decode using codec2
01/28/20    sh     Add query control to enumerate supported profiles
04/24/19    sm     Multi stream buffer handling during reconfig
03/14/19    sm     Generalize enc and dec command handling
02/15/19    rz     Use internal buffer to handle EOS
05/08/18    sm     Add support for 10 bit playback
05/08/17    sm     Update for new hyp-video architecture
========================================================================== */

/* =======================================================================

INCLUDE FILES FOR MODULE

========================================================================== */

#ifndef __LINUX_VDEC_FE_H__
#define __LINUX_VDEC_FE_H__

#include "hyp_videopriv_fe.h"
#include <linux/videodev2.h>

hypv_status_type device_callback_dec(uint8 *msg, uint32 length, void *cd);
hypv_status_type v4l2fe_dec_s_fmt(fe_io_session_t* fe_ioss, struct v4l2_format* data);
hypv_status_type v4l2fe_dec_g_fmt(fe_io_session_t* fe_ioss, struct v4l2_format* data);
hypv_status_type v4l2fe_dec_reqbufs(fe_io_session_t* fe_ioss, struct v4l2_requestbuffers* data);
hypv_status_type v4l2fe_dec_s_ctrl(fe_io_session_t *fe_ioss, struct v4l2_control* data);
hypv_status_type v4l2fe_dec_g_ctrl(fe_io_session_t *fe_ioss, struct v4l2_control* data);
hypv_status_type v4l2fe_decoder_cmd(fe_io_session_t* fe_ioss, struct v4l2_decoder_cmd* data);
hypv_status_type v4l2fe_dec_s_parm(fe_io_session_t* fe_ioss, struct v4l2_streamparm* data);
hypv_status_type v4l2fe_decoder_multistream_config(fe_io_session_t *fe_ioss);
hypv_status_type v4l2fe_dec_queryctrl(fe_io_session_t *fe_ioss, struct v4l2_queryctrl* data);
hypv_status_type v4l2fe_dec_querymenu(struct v4l2_querymenu* data);
hypv_status_type v4l2fe_dec_enum_fmt(fe_io_session_t* fe_ioss, struct v4l2_fmtdesc* data);

#endif

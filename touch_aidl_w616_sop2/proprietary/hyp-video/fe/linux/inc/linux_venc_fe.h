/*========================================================================

*//** @file linux_venc_fe.h

@par FILE SERVICES:
      Linux video enoder hypervisor front-end implementation header


@par EXTERNALIZED FUNCTIONS:
      See below.

Copyright (c) 2017,2019-2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---     -------------------------------------------------------
08/30/23    nb     Fix compilation errors due to additon of new compiler flags
05/24/23    pc     Add HDR10 static metadata support for HEVC encode
09/26/22    mm     Remove dependency OMX
08/03/22    nb     Set H264 entropy coding property during streamon
07/30/22    nb     Update P-frames setting during streamon & remove B-frames setting
07/30/22    nb     Set frame QP during streamon
07/30/22    nb     Set rate_control property during streamon
05/26/22    ll     Enable bitrate saving mode
03/28/22    sh     Enable Layer Encode for GVM
03/30/22    sd     Add support for blur filter
03/25/22    sh     Enable LTR support with Codec2
10/05/21    rq     Postpone setting multi slice until streamon
07/29/21    mm     Postpone setting of intra refresh and clean up
05/10/21    sj     Add support to query supported profiles/levels
10/05/20    hl     Add Long Term Reference Support
08/27/20    sh     Bringup video decode using codec2
03/04/19    sm     Add query control API
05/08/17    sm     Update for new hyp-video architecture
========================================================================== */

/* =======================================================================

INCLUDE FILES FOR MODULE

========================================================================== */

#ifndef __LINUX_VENC_FE_H__
#define __LINUX_VENC_FE_H__

#include "hyp_videopriv_fe.h"
#include <sys/mman.h>
#include <linux/videodev2.h>

hypv_status_type device_callback_enc(uint8 *msg, uint32 length, void *cd);
hypv_status_type v4l2fe_enc_s_fmt(fe_io_session_t* fe_ioss, struct v4l2_format* data);
hypv_status_type v4l2fe_enc_g_fmt(fe_io_session_t* fe_ioss, struct v4l2_format* data);
hypv_status_type v4l2fe_enc_reqbufs(fe_io_session_t* fe_ioss, struct v4l2_requestbuffers* data);
hypv_status_type v4l2fe_enc_s_ctrl(fe_io_session_t *fe_ioss, struct v4l2_control* data);
hypv_status_type v4l2fe_enc_g_ctrl(fe_io_session_t *fe_ioss, struct v4l2_control* data);
hypv_status_type v4l2fe_encoder_cmd(fe_io_session_t* fe_ioss, struct v4l2_encoder_cmd* data);
hypv_status_type v4l2fe_enc_s_parm(fe_io_session_t* fe_ioss, struct v4l2_streamparm* data);
hypv_status_type v4l2fe_enc_queryctrl(fe_io_session_t *fe_ioss, struct v4l2_queryctrl* data);
hypv_status_type v4l2fe_enc_querymenu(struct v4l2_querymenu* data);
hypv_status_type v4l2fe_enc_enum_fmt(fe_io_session_t* fe_ioss, struct v4l2_fmtdesc* data);
hypv_status_type v4l2fe_enc_s_ltr_count(fe_io_session_t* fe_ioss, struct v4l2_control* data);
hypv_status_type enc_s_grid_mode(fe_io_session_t* fe_ioss);
hypv_status_type enc_s_idr_period(fe_io_session_t *fe_ioss);
hypv_status_type enc_s_vpe_csc(fe_io_session_t* fe_ioss);
hypv_status_type enc_s_video_signal_info(fe_io_session_t* fe_ioss);
hypv_status_type enc_s_intra_refresh(fe_io_session_t *fe_ioss, struct v4l2_control* data);
hypv_status_type enc_s_multi_slice(fe_io_session_t *fe_ioss);
hypv_status_type enc_s_blur_dimensions(fe_io_session_t* fe_ioss, struct v4l2_control* data);
hypv_status_type enc_s_hb_max_layer(fe_io_session_t *fe_ioss);
hypv_status_type enc_s_hp_max_layer(fe_io_session_t *fe_ioss);
hypv_status_type enc_s_hp_layer(fe_io_session_t *fe_ioss);
hypv_status_type enc_s_bitrate_saving_mode(fe_io_session_t* fe_ioss, struct v4l2_control* data);
hypv_status_type enc_s_rate_control(fe_io_session_t *fe_ioss);
hypv_status_type enc_s_frame_qp(fe_io_session_t *fe_ioss);
hypv_status_type enc_s_num_p_frames(fe_io_session_t *fe_ioss);
hypv_status_type enc_s_h264_entropy_mode(fe_io_session_t *fe_ioss);
hypv_status_type v4l2fe_enc_set_hdr_info(fe_io_session_t *fe_ioss);
#endif

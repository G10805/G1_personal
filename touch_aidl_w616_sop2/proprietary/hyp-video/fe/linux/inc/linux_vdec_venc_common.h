/*========================================================================

*//** @file linux_vdec_venc_common.h

@par FILE SERVICES:
      Linux video decoder and encoder shared  hypervisor front-end
      implementation header


@par EXTERNALIZED FUNCTIONS:
      See below.

Copyright (c) 2017-2022 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---     -------------------------------------------------------
11/07/22    su     Add support to SYNCFRAMEDECODE in thumbnail mode
11/02/22    sk     Use flag and change header path for kernel 5.15
09/03/22    mz     Add function to free internally allocated meta buffer for lemans
05/10/21    sj     Add support to query supported profiles/levels
02/01/20    rq     Delay setting secure mode until right before steamon
08/27/20    sh     Bringup video decode using codec2
01/09/20    sm     Support for secure playback
04/11/19    sm     Add V4l2 dequeue event and buffer API handling
04/09/19    sm     Handle encoder buffer reference in dynamic mode
03/14/19    sm     Generalize enc and dec command handling
03/04/19    sm     Add dynamic buffer mode for encode usecase
02/05/19    rz     Bringup changes for 8155
11/01/18    sm     Add a common function to map color formats
04/02/18    sm     Make use of a common function to convert v4l2 to vidc codec
01/18/18    sm     Add support for passthrough mode feature
10/12/17    sm     Simplify buffer management logics for dynamic alloc mode
05/08/17    sm     Update for new hyp-video architecture
========================================================================== */

/* =======================================================================

INCLUDE FILES FOR MODULE

========================================================================== */


#ifndef __LINUX_VDEC_VENC_COMMON_H__
#define __LINUX_VDEC_VENC_COMMON_H__

#include "hyp_videopriv_fe.h"
#include <linux/videodev2.h>
#ifdef SUPPORT_DMABUF
#include <vidc/media/msm_media_info.h>
#else
#include <media/msm_media_info.h>
#endif

hypv_status_type dec_enc_streamoff(fe_io_session_t* fe_ioss, enum v4l2_buf_type data);
hypv_status_type dec_enc_streamon(fe_io_session_t* fe_ioss, enum v4l2_buf_type data);
hypv_status_type dec_enc_qbuf(fe_io_session_t* fe_ioss, struct v4l2_buffer* data);
hypv_status_type dec_enc_enum_framesizes(fe_io_session_t* fe_ioss, struct v4l2_frmsizeenum* data);
hypv_status_type dec_enc_subscribe_event(fe_io_session_t* fe_ioss, struct v4l2_event_subscription* data);
hypv_status_type dec_enc_unsubscribe_event(fe_io_session_t* fe_ioss, struct v4l2_event_subscription* data);
hypv_status_type dec_enc_querycap(fe_io_session_t* fe_ioss, struct v4l2_capability* data);
hypv_status_type dec_enc_enum_fmt(fe_io_session_t* fe_ioss, struct v4l2_fmtdesc* data);
hypv_status_type dec_enc_s_ext_ctrl(fe_io_session_t* fe_ioss, struct v4l2_ext_controls* data);
hypv_status_type cleanup_registered_binfo(fe_io_session_t* fe_ioss);
hypv_status_type dec_enc_submit_command(fe_io_session_t* fe_ioss, uint32 vidc_cmd);
hypv_status_type dec_enc_set_drv_property(fe_io_session_t *fe_ioss, vidc_property_id_type propId,
                               uint32 nPktSize, uint8 *pPkt);
hypv_status_type dec_enc_get_drv_property(fe_io_session_t *fe_ioss, vidc_property_id_type propId,
                               uint32 nPktSize, uint8 *pPkt);
hypv_status_type dec_enc_prepare_buf(fe_io_session_t* fe_ioss, struct v4l2_buffer* data);
hypv_status_type dec_enc_set_buf_alloc_mode(fe_io_session_t* fe_ioss, vidc_buffer_mode_type buf_mode,
                               vidc_buffer_type buf_type);
hypv_status_type v4l2fe_vdec_venc_ioctl_passthrough(fe_io_session_t* fe_ioss, uint32 cmd, void* data);
hypv_status_type dec_enc_cmd(fe_io_session_t* fe_ioss, struct v4l2_decoder_cmd* data);
hypv_status_type dec_enc_free_eos_buffer(fe_io_session_t *fe_ioss);
hypv_status_type dec_enc_free_meta_buffer(int data_fd);
hypv_status_type dec_enc_rm_dyn_buf_ref(fe_io_session_t *fe_ioss, vidc_frame_data_type* frame_data);
hypv_status_type dec_enc_dqevent(fe_io_session_t* fe_ioss, struct v4l2_event* data);
hypv_status_type dec_enc_dqbuf(fe_io_session_t* fe_ioss, struct v4l2_buffer* data);
hypv_status_type dec_enc_s_secure_video(fe_io_session_t *fe_ioss, boolean enable);
hypv_status_type dec_enc_s_low_latency_mode(fe_io_session_t *fe_ioss, struct v4l2_control* data);
hypv_status_type dec_enc_query_level(fe_io_session_t *fe_ioss, v4l2fe_vidc_ctrl *ctrl,
                                     int size, struct v4l2_queryctrl* data);
hypv_status_type dec_enc_query_profile(fe_io_session_t *fe_ioss, v4l2fe_vidc_ctrl *ctrl,
                                       int size, struct v4l2_queryctrl* data);
hypv_status_type dec_enc_query_menu(v4l2fe_vidc_ctrl *ctrls, int size, struct v4l2_querymenu* data);
hypv_status_type dec_enc_s_thumbnail_mode(fe_io_session_t *fe_ioss, struct v4l2_control* data);
vidc_codec_type codecV4l2ToVidc(uint32 v4l2Codec);
enum color_fmts mapV4l2colorfmt(uint32 v4l2colorfmt);
const v4l2fe_vidc_format *dec_enc_get_pixel_fmt_index(const v4l2fe_vidc_format fmt[], int size, int index, int fmt_type);
int dec_enc_convert_v4l2_to_vidc(int id, int value);
int dec_enc_convert_vidc_to_v4l2(int id, int value);
#endif

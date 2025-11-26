/*========================================================================

*//** @file hyp_videopriv_fe.h

@par FILE SERVICES:
      Hypervisor video private hypervisor front-end header


@par EXTERNALIZED FUNCTIONS:
      See below.

Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---     -------------------------------------------------------
10/18/24    su     Update reconfig event for multi-stream
05/24/24    vz     Enable intra refresh cyclic mode
05/24/23    pc     Add HDR10 static metadata support for HEVC encode
10/07/22    su     Add support to SYNCFRAMEDECODE in thumbnail mode
08/03/22    nb     Set H264 entropy coding property during streamon
07/30/22    nb     Refine configuration of layer encode properties
07/30/22    nb     Set frame QP during streamon
07/30/22    nb     Set rate_control property during streamon
07/12/22    ls     Handle VP9 linear output decoding
05/26/22    ll     Enable bitrate saving mode
03/28/22    sh     Enable Layer Encode for GVM
01/05/22    sd     Add support for blur filter
10/05/21    rq     Postpone setting multi slice until streamon
07/29/21    mm     Postpone setting of intra refresh and clean up
02/08/21    mm     Enalbe grid mode for HEIC
10/12/20    sh     Enable CSC for H264/HEVC encoder
08/27/20    sh     Bringup video decode using codec2
06/27/19    sm     Differentiate output and input metadata buffer length
05/30/19    sm     Add support for hab export using ion fd
03/14/19    sm     Handle stop cmd for encoder
03/06/19    rz     Handle decoder output dynamic map/unmap/export/unexport buffers
03/04/19    sm     Add dynamic buffer mode for encode usecase
02/15/19    rz     Use internal buffer to handle EOS
01/18/18    sm     Add support for passthrough mode feature
11/29/17    sm     Add support for video input interrupts handling
10/12/17    sm     Simplify buffer management logics for dynamic alloc mode
06/23/17    sm     Streamline hypervisor context structure and definitions
05/08/17    sm     Update for new hyp-video architecture
06/22/16    hl     Initial implementation of video FE-BE
========================================================================== */

/* =======================================================================

INCLUDE FILES FOR MODULE

========================================================================== */


#ifndef __HYP_VIDEOPRIV_FE_H__
#define __HYP_VIDEOPRIV_FE_H__

#include "hyp_videopriv.h"
#include "hyp_video_fe.h"
#include <map>

typedef struct
{
    int     dev_fd;
    int     data_fd;
    uint8*  frame_addr;
} hyp_internal_buffer;

typedef struct
{
   int32   fd;
   int32   dup_fd;
   uint8   ref_count;
   uint32  alloc_len;
   int32   metadata_fd;
   uint32  metadata_alloc_len;
   uint64  index;
} dynamic_buf_info;

typedef struct
{
    hypv_session_t*                 io_handle;
    hvfe_callback_t                 fe_cb;
    hypv_callback_t                 device_cb;
    MM_HANDLE                       lock_buffer;
    uint8                           dev_cmd_buffer[MAX_DEV_CMD_BUFFER_SIZE];
    uint32                          input_fourCC;
    uint32                          output_fourCC;
    vidc_session_codec_type         session_codec;
    vidc_color_format_config_type   color_format_config;
    vidc_frame_size_type            frame_size;
    uint32                          output_metabuf_length;
    MM_HANDLE                       state_synch_obj_q;
    MM_HANDLE                       state_synch_obj;
    uint32                          input_buffer_count;
    uint32                          output_buffer_count;
    uint32                          vidc_input_buffer_size;
    vidc_buffer_info_type*          vidc_input_buffer_info;
    uint32                          vidc_output_buffer_size;
    vidc_buffer_info_type*          vidc_output_buffer_info;
    vidc_iperiod_type               enc_iperiod;
    vidc_session_qp_type            enc_qp;
    vidc_session_qp_range_type      enc_qp_range;
    vidc_entropy_control_type       enc_entropy;
    vidc_intra_refresh_type         enc_intra_refresh;
    vidc_db_control_type            enc_db_control;
    video_session_type              video_session;
    hyp_platform_type               hyp_plt_id;
    void                            *fe_plt_data;
    uint8                           event_buffer[MAX_DEV_CMD_BUFFER_SIZE];
    hyp_internal_buffer             eos_buffer;
    uint32                          dynmaic_buf_entry_count;
    dynamic_buf_info                *out_dynamic_info;
    std::map<uint64, uint64>        *input_tag_entry;
    vidc_vui_video_signal_info_type vui_video_signal_info;
    uint8                           video_vpe_csc;
    uint8                           video_vpe_csc_custom_matrix;
    boolean                         enc_grid_mode;
    boolean                         enable_intra_refresh;
    struct v4l2_control             intra_refresh_info;
    uint32                          intra_refresh_period;
    vidc_multi_slice_type           enc_multi_slice_type;
    boolean                         enable_blur_filter;
    struct v4l2_control             blur_filter_info;
    uint32                          hier_layer_type;
    uint32                          max_hier_layer_count;
    uint32                          hier_layer_count;
    uint32                          ltr_count;
    boolean                         enable_bitrate_saving_mode;
    struct v4l2_control             bitrate_saving_mode;
    boolean                         enable_rate_control;
    vidc_rate_control_type          rate_control_type;
    boolean                         enable_frame_qp;
    boolean                         enable_entropy_mode;
    struct v4l2_control             enc_entropy_info;
    boolean                         enable_thumbnail_mode;
    boolean                         hdr10_sei_enabled;
    vidc_metadata_hdr_static_info   hdr_static_info;
}fe_io_session_t;

hypv_session_t* hyp_device_open(char* name, hypv_callback_t* cb);
hypv_status_type hyp_device_close(hypv_session_t* hypv_session);

// return the number of bytes actually read
hypv_status_type hyp_device_read(hypv_session_t* hypv_session, uint8* aBuf, uint32 aBufSize);

// return the number of bytes actually write
hypv_status_type hyp_device_write(hypv_session_t* hypv_session, uint8* aBuf, uint32 aBufSize);

hypv_status_type hyp_device_ioctl(hypv_session_t* hypv_session, uint32 aCommand, uint8 *aInBuf,
                     uint32 aInBufSize, uint8* aOutBuf, uint32 aOutBufSize);

/* ping the device with cmd and input data. Once receviing the ping, the device is
*  expected to return the response in order to unblock client followed by
*  processing input data
*/
hypv_status_type hyp_device_ping(hypv_session_t* hypv_session, uint32 aCommand,
                    uint8 *aBuf, uint32 aBufSize);

hypv_status_type plt_fe_open(const char* str, int flag, fe_io_session_t* cb );
hypv_status_type plt_fe_ioctl(HVFE_HANDLE handle, uint32 cmd, void* data);
hypv_status_type plt_fe_close(HVFE_HANDLE handle);

#endif

/*========================================================================

*//** @file linux_video_fe.h

@par FILE SERVICES:
      Linux video hypervisor front-end implementation header


@par EXTERNALIZED FUNCTIONS:
      See below.

Copyright (c) 2016-2017, 2019-2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: //deploy/qcom/qct/platform/qnp/qnx/auto/components/rel/hyp_video.qxa_qa/4.0/fe/linux/inc/linux_video_fe.h#2 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
08/30/23    nb     Fix compilation errors due to additon of new compiler flags
03/16/23    sd     Support encoder output metadata to get average frame QP
11/07/22    su     Add support to SYNCFRAMEDECODE in thumbnail mode
11/02/22    sk     Use flag and change header path for kernel 5.15
09/26/22    mm     Remove dependency OMX
04/12/22    ls     Remove resolution restriction on buffer count update
06/15/21    mm     Restrict the max count of output buffer
06/09/21    sh     Add macros for minimum supported width & height
05/25/21    sj     Fix compilation errors on linux due to C99 standard
05/06/21    sj     Add support to query supported profiles/levels
10/12/20    sh     Enable CSC for H264/HEVC encoder
09/15/20    sh     Change minimum input & output buffer count to 6 & 10 respectively
08/27/20    sh     Bringup video decode using codec2
03/17/20    sm     Add multislice query controls
09/09/19    sm     Fix timestamp overflow
09/04/19    sm     Calculate the actual extradata buffer size
06/27/19    sm     Add support for input crop
04/11/19    sm     Add V4l2 dequeue event and buffer API handling
10/12/17    sm     Simplify buffer management logics for dynamic alloc mode
09/20/17    sm     Add import/export 4k alignment to meet HAB requirement
05/08/17    sm     Update for new hyp-video architecture
========================================================================== */

/* =======================================================================

INCLUDE FILES FOR MODULE

========================================================================== */

#ifndef __LINUX_VIDEO_FE_H__
#define __LINUX_VIDEO_FE_H__

#include <linux/msm_ion.h>
#include <ion/ion.h>
#include <poll.h>
#include <linux/videodev2.h>
#ifdef SUPPORT_DMABUF
#include <vidc/media/msm_vidc_utils.h>
#else
#include <media/msm_vidc_utils.h>
#endif

#define V4L2FE_CONVERT_USEC_TO_SEC(us) (us/1000000)
#define V4L2FE_CONVERT_SEC_TO_USEC(s)  ((uint64)s*1000000)

#define ALIGN(x, to_align) ((((unsigned) x) + (to_align - 1)) & ~(to_align - 1))

#define V4L2FE_METADATA_DEFAULT_PORTINDEX_INPUT      0
#define V4L2FE_METADATA_DEFAULT_PORTINDEX_OUTPUT     1
#define V4L2FE_METADATA_DEFAULT_VERSION              0x01010200

#define FE_VIDC_BUFFER_INPUT           0
#define FE_VIDC_BUFFER_OUTPUT          1
#define FE_VIDC_BUFFER_OUTPUT2         2
#define NUM_CAPTURE_BUFFER_PLANE_DEC   2
#define NUM_OUTPUT_BUFFER_PLANE_DEC    1
#define MIN_INPUT_BUFFER_COUNT_DEC     6
#define MIN_OUTPUT_BUFFER_COUNT_DEC    10
#define MAX_OUTPUT_BUFFER_COUNT_DEC    64
#define THUMBNAIL_BUFFER_COUNT         1
#define NUM_CAPTURE_BUFFER_PLANE_ENC   2
#define NUM_OUTPUT_BUFFER_PLANE_ENC    2
#define MIN_BUFFER_COUNT_ENC           4
#define MAX_WIDTH                      4096
#define MAX_HEIGHT                     2304
#define MAX_INPUT_BUFFER_SIZE          (((MAX_WIDTH) * (MAX_HEIGHT) * (3) / (2)) / (2))
#define MAX_EXTRADATA_SIZE             (16 * 1024)
#define WAIT_STATE_TIMEOUT             100000
#define MAX_QUEUE_EVENTS               50
#define MAX_NUM_BUFFER_PLANES          2
#define MAX_EXTRADATA_BUFS             64
#define MAX_ENC_SLICE_MB_SIZE          (((MAX_WIDTH) * (MAX_HEIGHT) >> 8))
#define MIN_ENC_SLICE_MB_SIZE          1
#define MAX_ENC_BIT_RATE               300000000
#define MAX_ENC_SLICE_BYTE_SIZE        ((MAX_ENC_BIT_RATE) >> 3)
#define MIN_ENC_SLICE_BYTE_SIZE        512
#define MIN_SUPPORTED_WIDTH            96
#define MIN_SUPPORTED_HEIGHT           96
#define DEFAULT_TILE_DIMENSION         512

#define MAX_NAME_LENGTH 64
#define MAX_DESCRIPTION_LENGTH 32
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define COLOR_RANGE_UNSPECIFIED (-1)

// DINIT ( open-> / <- close) LOADED
// LOADED ( load_resources -> / <- release_resources) IDLE
// IDLE ( start -> / <- stop) EXECUTING
// EXECUTING (pause -> / <- resume) PAUSE
// PAUSE ( stop -> / <- pause) IDLE

typedef enum
{
   V4L2FE_STATE_DEINIT   = 0,
   V4L2FE_STATE_LOADED,
   V4L2FE_STATE_IDLE,
   V4L2FE_STATE_EXECUTING,
   V4L2FE_STATE_PAUSE,
   V4L2FE_STATE_UNUSED = 0xf0000000
}v4l2fe_state_t;

typedef struct
{
    uint32  is_etb;
    uint8*  address;
    uint32  flags;
}v4l2fe_etb_flag_info_type;

typedef struct {
    struct v4l2_plane  buf_planes[MAX_NUM_BUFFER_PLANES];
    struct v4l2_buffer buf;
}event_buf_type;

typedef struct
{
    v4l2fe_etb_flag_info_type* v4l2fe_etb_flag_info;
    v4l2fe_state_t             state;
    hyp_queue_type             evt_queue;
    hyp_queue_type             evt_output_buf_queue;
    hyp_queue_type             evt_input_buf_queue;
}fe_linux_plt_data_t;

typedef struct
{
    char name[MAX_NAME_LENGTH];
    uint8 description[MAX_DESCRIPTION_LENGTH];
    uint32 fourcc;
    int32 type;
    uint32 (*get_frame_size)(int plane, uint32 height, uint32 width);
    bool defer_outputs;
    uint32 input_min_count;
    uint32 output_min_count;
}v4l2fe_vidc_format;

typedef struct
{
    uint32 id;
    char name[MAX_NAME_LENGTH];
    int32 default_value;
    int32 minimum;
    int32 maximum;
    int32 level;
    uint32 menu_skip_mask;
    const char * const *qmenu;
}v4l2fe_vidc_ctrl;

typedef struct
{
    uint32 v4l2_name;
    uint32 vidc_name;
}v4l2_vidc_table_t;

typedef enum
{
    V4L2_CONVERT_TO_VIDC = 0,
    V4L2_CONVERT_FROM_VIDC
}v4l2fe_conversion_mode;


#endif

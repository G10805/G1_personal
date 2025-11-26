/*========================================================================

*//** @file hyp_dvr.h

@par FILE SERVICES:
      Hypervisor dvr interface definitions. The file defines Hypervisor
      dvr communication messages and the data structures.

@par EXTERNALIZED FUNCTIONS:
      See below.

Copyright (c) 2018, 2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */
/*===========================================================================
                             Edit History

$Header: $

when(mm/dd/yy)     who         what, where, why
-------------      --------    ------------------------------------------------
03/24/20           sh          Add P010 format
10/12/18           sm          Remove restriction on number of buffers received
09/28/18           sm          Increase the number of buffer intake for muxing
08/22/18           sm          Initial version of hypervisor DVR
===============================================================================*/
#ifndef __HYP_DVR_H__
#define __HYP_DVR_H__

#include <AEEStdDef.h>

#define IOCTL_HDVR_MUX_VIDEO_META_INFO  0
#define IOCTL_HDVR_MUX_AUDIO_META_INFO  1
#define IOCTL_HDVR_MUX_BUF_INFO         2
#define IOCTL_HDVR_DISP_BUF_INFO        3

typedef enum
{
    HDVR_STREAM_TYPE_AUDIO = 0,
    HDVR_STREAM_TYPE_VIDEO = 1
} hdvr_stream_type;

typedef enum
{
    HDVR_VIDEO_CODEC_TYPE_H264 = 0,
    HDVR_VIDEO_CODEC_TYPE_H265 = 1
} hdvr_video_codec_type;

typedef enum
{
    HDVR_PIXEL_FORMAT_TYPE_NV12   = 0,
    HDVR_PIXEL_FORMAT_TYPE_RGB888 = 1,
    HDVR_PIXEL_FORMAT_TYPE_P010 = 2
} hdvr_video_pixel_type;

typedef enum
{
    HDVR_COLOR_SPACE_TYPE_EXCEPT_BT2020 = 0,
    HDVR_COLOR_SPACE_TYPE_ITU_R_709     = 1,
} hdvr_video_color_space_type;

typedef enum
{
    HDVR_AUDIO_CODEC_TYPE_AAC    = 0,
    HDVR_AUDIO_CODEC_TYPE_AMR_NB = 1,
    HDVR_AUDIO_CODEC_TYPE_AMR_WB = 2,
} hdvr_audio_codec_type;

typedef enum
{
    HDVR_CONTAINER_TYPE_MP4 = 0,
    HDVR_CONTAINER_TYPE_3GP = 1
} hdvr_container_type;

typedef enum
{
    HDVR_BUF_FLAG_TYPE_NOSYNCFRAME = 0,
    HDVR_BUF_FLAG_TYPE_SYNCFRAME   = 1,
    HDVR_BUF_FLAG_TYPE_CODECCONFIG = 2,
    HDVR_BUF_FLAG_TYPE_EOS         = 4
} hdvr_buf_flag_type;

#define HDVR_FE_INVALID_HANDLE       -1

typedef void *HDVR_FE_HANDLE;
typedef int (*hdvr_fe_callback_handler_t)(void* context, void* message);

/* Hypervisor status type */
typedef enum
{
   HDVR_STATUS_FAIL    = -1,
   HDVR_STATUS_SUCCESS = 0x0,
   HDVR_STATUS_BAD_PARAMETER,
   HDVR_STATUS_ALLOC_FAIL,
   HDVR_STATUS_VERSION_MISMATCH,
   HDVR_STATUS_MAX     = 0x7fffffff
} hdvr_status_type;

typedef struct
{
    uint32                height;    /* video height */
    uint32                width;     /* video width */
    uint32                bitrate;   /* video bitrate */
    uint32                framerate; /* video framerate */
    hdvr_video_codec_type codec;     /* video codec */
    hdvr_container_type   container; /* container format to mux */
} hdvr_mux_video_meta_info;

typedef struct
{
    uint32                bitrate;       /* audio bitrate */
    uint32                samplerate;    /* audio samplerate */
    uint32                channel_count; /* audio channel count */
    hdvr_audio_codec_type codec;         /* audio codec */
} hdvr_mux_audio_meta_info;

typedef struct
{
    uint32             buf_size;    /* buffer size */
    uint32             buf_count;   /* number of buffers */
    uint32             *fill_len;   /* pointer to array of fill length */
    uint64             *pts;        /* pointer to array of presentation time stamp in microsec */
    hdvr_buf_flag_type *flags;      /* pointer to array of flags */
    hdvr_stream_type   stream;      /* stream type audio or video */
    void               *buf_handle; /* buffer handle */
} hdvr_mux_buf_info;

typedef struct
{
    uint32                      buf_size;    /* buffer size */
    uint32                      fill_len;    /* fill length */
    uint32                      height;      /* frame height */
    uint32                      width;       /* frame width */
    hdvr_video_pixel_type       format;      /* frame format */
    hdvr_video_color_space_type colorspace;  /* colorspace */
    uint32                      field;       /* field order of the frame */
    uint32                      sequence;    /* sequence count of the frame */
    void                        *buf_handle; /* frame buffer handle */
} hdvr_disp_buf_info;

typedef struct
{
    hdvr_fe_callback_handler_t handler;
    void*                     context;
} hdvr_fe_callback_t;

HDVR_FE_HANDLE hdvr_fe_open(const char* str, int flag, hdvr_fe_callback_t* cb);
hdvr_status_type hdvr_fe_ioctl(HDVR_FE_HANDLE handle, int cmd, void* data);
hdvr_status_type hdvr_fe_close(HDVR_FE_HANDLE handle);

#endif //__HYP_DVR_H__

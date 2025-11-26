/*===========================================================================

*//** @file hyp_dvr_mux_be.cpp
This file implements hypervisor dvr be services

Copyright (c) 2018-2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/

/*===========================================================================
                             Edit History

$Header:  $

when(mm/dd/yy)     who         what, where, why
-------------      --------    -------------------------------------------------------
08/30/23           br          Change boot kpi markers
11/02/22           sk          Use flag and change header path for kernel 5.15
06/18/21           su          Initialize varibles to avoid K/W errors
07/12/20           sa          Bringup DVR on LA GVM without muxer
04/23/20           sh          Fix DVR BE crash when muxing is done in FE
01/15/20           sh          Bringup DVR on LA GVM on Hana
09/23/19           sh          Allocate local cacheable buffer to reduce CPU load
04/10/19           sh          Disable import/unimport of buffers during muxing
03/30/19           sh          Impement HIDL interface for DVR
03/28/19           sh          Add support for HEVC
03/08/19           sh          Force close the mux instance on reaching max filesize limit
02/28/19           sh          Skip writing ADTS header into QTI muxer
02/14/19           sh          Fix incorrect fps calculation due to wrong sample_delta duration
12/04/18           sm          Update DVR storage and control file locations
12/03/18           sm          Add KPI log when the app is ready
11/27/18           sm          Invoke certain functionality only if that feature is enabled
11/22/18           sm          DVR input pipe is created again if not created early in boot
11/22/18           sm          Add support for circular mux file storage
11/20/18           sm          Queue preview buffers only when valid frames are received
11/16/18           sm          Add support for runtime preview enable/disable
11/13/18           sm          Fix audio playback from container format
11/12/18           sm          Add support for runtime debug level configuration
11/02/18           sm          Fix display buffer size to avoid corrupted preview
10/25/18           sm          Update debug log level to log only error
10/12/18           sm          Free mapping to avoid stale va
09/28/18           sm          Add support for QTI muxer
08/22/18           sm          Initial version of hypervisor DVR
============================================================================*/

#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <errno.h>
#include <linux/version.h>
#include "MMThread.h"
#include "MMSignal.h"
#include "MMCriticalSection.h"
#include "hyp_dvr.h"
#include "hyp_debug.h"
#include "MMDebugMsg.h"
#include "MMTimer.h"
#include "AEEStdDef.h"
#include <MediaMuxer.h>
#include <foundation/AMessage.h>
#include <foundation/ABuffer.h>
#include "hyp_dvrpriv.h"
#include "hyp_buffer_manager.h"
#include <sys/mman.h>
#ifdef ENABLE_DVR_MEDIA_MUXER
#include <MediaCodec.h>
#include <gralloc_priv.h>
#include <filemux.h>
#include <filemuxtypes.h>
#endif
#ifdef SUPPORT_DMABUF
#include <vidc/media/msm_media_info.h>
#else
#include <media/msm_media_info.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <OMX_Audio.h>
#include <poll.h>
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#define QTI_MUXER            1
#define MUX_MM_SUBID         1

#define RECORD_FILE_NAME     "/mnt/vendor/dvr_mux/hdvr_mux_output.mp4"
#define RECORD_FILE_NAME_ROT "/mnt/vendor/dvr_mux/hdvr_mux_output"
#define DISPLAY_FILE_NAME    "/mnt/vendor/dvr_mux/hdvr_disp_output.yuv"
#define BOOTKPI_DEVICE       "/sys/kernel/debug/bootkpi/kpi_values"
#define V4L2_LOOPBACK_DEVICE "/dev/video0"

#define AUDIO_CS_LEN            7
#define VIDEO_SPS_LEN           16
#define MAX_BUFFERS             6
#define MAX_RESPONSE_TIMEOUT_MS 1000

#define MAX_NUM_STREAMS                      2
#define VIDEO_TRACK_ID                       0
#define AUDIO_TRACK_ID                       1
#define MOVIE_TIMESCALE                      1000
#define DEFAULT_H264_PROFILE                 66
#define DEFAULT_H264_LEVEL                   30
#define DEFAULT_HEVC_PROFILE                 66
#define DEFAULT_HEVC_LEVEL                   30
#define VIDEO_MUX_INTERLACE_RATE             3000
#define VIDEO_MUX_SAMPLE_DELTA               30
#define VIDEO_MUX_DEFAULT_CHUNK_SIZE         200000
#define VIDEO_MUX_BUFFER_SIZE                400000
#define VIDEO_MUX_MAX_HEADER                 1024
#define VIDEO_MUX_MAX_FOOTER                 1024
#define VIDEO_MUX_MAX_CHUNKS                 200000
#define VIDEO_MUX_MAX_TABLE_STORES           60
#define VIDEO_MUX_MAX_SAMPLES                160000
#define MUX_IMMINENT_THRESHOLD_WARNING_LIMIT 12
#define MUX_NEAR_THRESHOLD_WARNING_LIMIT     40
#define AUDIO_MUX_MAX_SAMPLE_RATE            50
#define AUDIO_MUX_SAMPLE_DELTA               1024
#define AUDIO_MUX_MAX_CHUNKS                 6000
#define AUDIO_MUX_MAX_TABLE_STORES           60
#define AUDIO_MUX_MAX_SAMPLES                16000
#define AUDIO_MUX_MAX_BUFFER_SIZE            240000
#define AUDIO_MUX_MAX_HEADER                 162
#define AUDIO_MUX_FRAME_PER_SAMPLE           1
#define ADTS_HEADER_SIZE_WITH_CRC            9
#define ADTS_HEADER_SIZE_WITHOUT_CRC         7
#define CRC_BIT_IN_ADTS_HEADER               0x80

#define WAIT_FOR_RESPONSE()                                                   \
{                                                                             \
    MM_HANDLE msg_set_handle = 0;                                             \
    hdvr_session->time_out = 0;                                               \
    MM_SignalQ_TimedWait(hdvr_session->signal_handle_q,                       \
                         MAX_RESPONSE_TIMEOUT_MS,                             \
                         (void **)&msg_set_handle, &hdvr_session->time_out ); \
}

#define MAX_MSG_LEN     80

#define MAX_MUX_FILE_SIZE   (10 * 1024 * 1024)
#define MUX_FILE_SIZE_WARNING_LIMIT ((MAX_MUX_FILE_SIZE) + (MAX_MUX_FILE_SIZE) * 40 / 100)

using namespace android;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int debug_level = 0x1;
static AMessage *video_format;
static AMessage *audio_format;
static const char *MP4_MAJOR_BRAND = "isom";
static const char *MP4_COMPAT_BRANDS [] = {"mp41", "isom"};

#ifdef ENABLE_DVR_MEDIA_MUXER
MUX_sample_info_type mux_sample_info;
MUX_create_params_type filemux_params;
MUX_stream_create_params_type streams[MAX_NUM_STREAMS];
#endif

/**===========================================================================
FUNCTION add_kpi_marker

@brief  Add boot KPI marker

@param [in] marker string

@dependencies
  None

@return
  Returns void
===========================================================================*/
static void add_kpi_marker(const char *marker)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
    if (0 == access(BOOTKPI_DEVICE, F_OK))
    {
        int fd = open(BOOTKPI_DEVICE, O_RDWR);

        if (-1 < fd)
        {
            write(fd, marker, strlen(marker));
            close(fd);
        }
    }
#else
    /* debug info, hence not adding boot_kpi string.
     */
    HYP_VIDEO_MSG_ERROR("%s", marker);
#endif
}

/**===========================================================================
FUNCTION calc_buffer_size

@brief  Calculate buffer size based on hardware requirement

@param [in] color_format
@param [in] width
@param [in] height

@dependencies
  None

@return
  Returns uint32 buffer size
===========================================================================*/
#ifdef ENABLE_DVR_MEDIA_MUXER
static uint32 calc_buffer_size(uint32 color_format, uint32 width, uint32 height)
{
    enum color_fmts color;

    switch (color_format)
    {
    case HAL_PIXEL_FORMAT_YCbCr_420_SP_VENUS:
        color = COLOR_FMT_NV21;
        break;
    case HAL_PIXEL_FORMAT_RGB_888:
        color = COLOR_FMT_RGBA8888;
        break;
    default:
        color = COLOR_FMT_NV21;
    }

    return (VENUS_BUFFER_SIZE(color, width, height));
}
#endif

/**===========================================================================
FUNCTION filemuxcb

@brief  Callback function for filemux notifications

@param [in] status
@param [in] pclient data
@param [in] pdata
@param [in] pbuffer

@dependencies
  None

@return
  Returns void
===========================================================================*/
#ifdef ENABLE_DVR_MEDIA_MUXER
void filemuxcb(int status, void *pclient_data, void *pdata, void *pbuffer)
{
    hdvr_session_t* hdvr_session = (hdvr_session_t*)pclient_data;
    (void) pdata;
    (void) pbuffer;

    HYP_VIDEO_MSG_HIGH("callback from filemux. status = %d", status);
    MM_Signal_Set(hdvr_session->signal_handle);

    return;
}
#endif

/**===========================================================================
FUNCTION filemux_init

@brief  Initialize filemux

@param [in] hdvr_session

@dependencies
  None

@return
  Returns hdvr_status_type
===========================================================================*/
#if ENABLE_DVR_MEDIA_MUXER
static hdvr_status_type filemux_init(hdvr_session_t* hdvr_session, uint32 index)
{
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;

    HABMM_MEMSET(&filemux_params, 0, sizeof(MUX_create_params_type));
    HABMM_MEMSET(streams, 0, sizeof(MUX_stream_create_params_type) *
                 MAX_NUM_STREAMS);
    filemux_params.streams = streams;

    /*--------------------------------------------------------------------------
      Initialize common params
     ----------------------------------------------------------------------------
    */
    filemux_params.drm_distribution_rights = 0;
    filemux_params.enable_fixupdata = FALSE;
    filemux_params.force_stsz_table = FALSE;
    filemux_params.include_drm = FALSE;
    filemux_params.movie_size_warning_imminent_threshold =
                   MUX_IMMINENT_THRESHOLD_WARNING_LIMIT;
    filemux_params.movie_size_warning_near_threshold =
                   MUX_NEAR_THRESHOLD_WARNING_LIMIT;
    filemux_params.num_streams = MAX_NUM_STREAMS;
    filemux_params.encrypt_param.streamEncrypted = FALSE;
    filemux_params.encrypt_param.type = MUX_ENCRYPT_TYPE_INVALID;
    filemux_params.movie_timescale = MOVIE_TIMESCALE;
    filemux_params.major_brand = MP4_MAJOR_BRAND;
    filemux_params.num_compat_brands = sizeof (MP4_COMPAT_BRANDS) /
                                       sizeof (MP4_COMPAT_BRANDS [0]);
    filemux_params.compat_brands = &MP4_COMPAT_BRANDS [0];
    filemux_params.stream_bitrate = hdvr_session->video_mux_meta_info.bitrate;
    filemux_params.num_channels = hdvr_session->audio_mux_meta_info.channel_count;
    filemux_params.sampling_rate = hdvr_session->audio_mux_meta_info.samplerate;
    filemux_params.version_minor = 0;
    filemux_params.version_major = 0;

    /*--------------------------------------------------------------------------
     Initialize basic audio stream params
    ----------------------------------------------------------------------------
   */
    filemux_params.streams[AUDIO_TRACK_ID].type = MUX_STREAM_AUDIO;
    filemux_params.streams[AUDIO_TRACK_ID].handler = "soun";
    filemux_params.streams[AUDIO_TRACK_ID].interlace = AUDIO_TRACK_ID;
    filemux_params.streams[AUDIO_TRACK_ID].media_timescale =
                   hdvr_session->audio_mux_meta_info.samplerate;
    filemux_params.streams[AUDIO_TRACK_ID].max_samples_rate = AUDIO_MUX_MAX_SAMPLE_RATE;
    filemux_params.streams[AUDIO_TRACK_ID].sample_delta = AUDIO_MUX_SAMPLE_DELTA;
    filemux_params.streams[AUDIO_TRACK_ID].max_chunks = AUDIO_MUX_MAX_CHUNKS;
    filemux_params.streams[AUDIO_TRACK_ID].max_table_stores = AUDIO_MUX_MAX_TABLE_STORES;
    filemux_params.streams[AUDIO_TRACK_ID].max_samples = AUDIO_MUX_MAX_SAMPLES;
    filemux_params.streams[AUDIO_TRACK_ID].buffer_size = AUDIO_MUX_MAX_BUFFER_SIZE;
    filemux_params.streams[AUDIO_TRACK_ID].max_header = AUDIO_MUX_MAX_HEADER;
    filemux_params.streams[AUDIO_TRACK_ID].inter_frames = TRUE;
    filemux_params.streams[AUDIO_TRACK_ID].chunk_size = AUDIO_MUX_MAX_CHUNKS;
    filemux_params.streams[AUDIO_TRACK_ID].frames_per_sample = AUDIO_MUX_FRAME_PER_SAMPLE;
    filemux_params.streams[AUDIO_TRACK_ID].subinfo.audio.sampling_frequency =
                   hdvr_session->audio_mux_meta_info.samplerate;

    if (HDVR_AUDIO_CODEC_TYPE_AAC == hdvr_session->audio_mux_meta_info.codec)
    {
        filemux_params.streams[AUDIO_TRACK_ID].subinfo.audio.format =
                       MUX_STREAM_AUDIO_MPEG4_AAC;
    }
    else if (HDVR_AUDIO_CODEC_TYPE_AMR_NB == hdvr_session->audio_mux_meta_info.codec)
    {
        filemux_params.streams[AUDIO_TRACK_ID].subinfo.audio.format =
                       MUX_STREAM_AUDIO_AMR;
    }
    else if (HDVR_AUDIO_CODEC_TYPE_AMR_WB == hdvr_session->audio_mux_meta_info.codec)
    {
        filemux_params.streams[AUDIO_TRACK_ID].subinfo.audio.format =
                       MUX_STREAM_AUDIO_AMR_WB;
    }
    else
    {
        filemux_params.streams[AUDIO_TRACK_ID].subinfo.audio.format =
                       MUX_STREAM_AUDIO_MPEG4_AAC;
    }

    /*--------------------------------------------------------------------------
      Initialize basic video stream params
     ----------------------------------------------------------------------------
    */
    filemux_params.streams[VIDEO_TRACK_ID].type = MUX_STREAM_VIDEO;
    filemux_params.streams[VIDEO_TRACK_ID].handler = "vide";
    filemux_params.streams[VIDEO_TRACK_ID].media_timescale = MOVIE_TIMESCALE;
    filemux_params.streams[VIDEO_TRACK_ID].interlace = VIDEO_TRACK_ID;
    filemux_params.streams[VIDEO_TRACK_ID].interlace_rate = VIDEO_MUX_INTERLACE_RATE;
    filemux_params.streams[VIDEO_TRACK_ID].inter_frames = TRUE;
    filemux_params.streams[VIDEO_TRACK_ID].max_samples_rate =
                   hdvr_session->video_mux_meta_info.framerate;
    filemux_params.streams[VIDEO_TRACK_ID].width =
                   hdvr_session->video_mux_meta_info.width << 16;
    filemux_params.streams[VIDEO_TRACK_ID].height =
                   hdvr_session->video_mux_meta_info.height << 16;
    filemux_params.streams[VIDEO_TRACK_ID].chunk_size = VIDEO_MUX_DEFAULT_CHUNK_SIZE;
    filemux_params.streams[VIDEO_TRACK_ID].buffer_size = VIDEO_MUX_BUFFER_SIZE;
    filemux_params.streams[VIDEO_TRACK_ID].max_header = VIDEO_MUX_MAX_HEADER;
    filemux_params.streams[VIDEO_TRACK_ID].max_footer = VIDEO_MUX_MAX_FOOTER;
    filemux_params.streams[VIDEO_TRACK_ID].max_chunks = VIDEO_MUX_MAX_CHUNKS;
    filemux_params.streams[VIDEO_TRACK_ID].max_table_stores = VIDEO_MUX_MAX_TABLE_STORES;
    filemux_params.streams[VIDEO_TRACK_ID].max_samples = VIDEO_MUX_MAX_SAMPLES;
    filemux_params.streams[VIDEO_TRACK_ID].subinfo.video.frame_rate =
                   hdvr_session->video_mux_meta_info.framerate;
    filemux_params.streams[VIDEO_TRACK_ID].sample_delta = ((MOVIE_TIMESCALE) / hdvr_session->video_mux_meta_info.framerate);
    filemux_params.streams[VIDEO_TRACK_ID].subinfo.video.width =
                   hdvr_session->video_mux_meta_info.width;
    filemux_params.streams[VIDEO_TRACK_ID].subinfo.video.height =
                   hdvr_session->video_mux_meta_info.height;
    if (HDVR_VIDEO_CODEC_TYPE_H264 == hdvr_session->video_mux_meta_info.codec)
    {
        filemux_params.streams[VIDEO_TRACK_ID].subinfo.video.format = MUX_STREAM_VIDEO_H264;
        filemux_params.streams[VIDEO_TRACK_ID].subinfo.video.profile = DEFAULT_H264_PROFILE;
        filemux_params.streams[VIDEO_TRACK_ID].subinfo.video.level = DEFAULT_H264_LEVEL;
    }
    else if (HDVR_VIDEO_CODEC_TYPE_H265 == hdvr_session->video_mux_meta_info.codec)
    {
        filemux_params.streams[VIDEO_TRACK_ID].subinfo.video.format = MUX_STREAM_VIDEO_HEVC;
        filemux_params.streams[VIDEO_TRACK_ID].subinfo.video.profile = DEFAULT_HEVC_PROFILE;
        filemux_params.streams[VIDEO_TRACK_ID].subinfo.video.level = DEFAULT_HEVC_LEVEL;
    }
    else
    {
        filemux_params.streams[VIDEO_TRACK_ID].subinfo.video.format = MUX_STREAM_VIDEO_H264;
        filemux_params.streams[VIDEO_TRACK_ID].subinfo.video.profile = DEFAULT_H264_PROFILE;
        filemux_params.streams[VIDEO_TRACK_ID].subinfo.video.level = DEFAULT_H264_LEVEL;
    }

    MUX_handle_type sFileHandle;

    HABMM_MEMSET(&sFileHandle, 0, sizeof(MUX_handle_type));
    sFileHandle.method = MUX_METHOD_EFS;
    sFileHandle.efs.method = MUX_METHOD_EFS;
    snprintf(sFileHandle.efs.filename, FS_FILENAME_MAX_LENGTH_P,"%s_%d.mp4",
             RECORD_FILE_NAME_ROT, index);
    hdvr_session->muxer[index] = new FileMux(&filemux_params, MUX_FMT_MP4, MUX_BRAND_MP4,
                                             &sFileHandle, false, &filemuxcb,
                                             (void *)hdvr_session);

    FileMux *muxer = (FileMux *)hdvr_session->muxer[index];
    if (!muxer || (muxer->MUX_get_Status() != MUX_SUCCESS))
    {
        HYP_VIDEO_MSG_ERROR("Create filemux failed");
        hdvr_status = HDVR_STATUS_FAIL;
    }
    else
    {
        HYP_VIDEO_MSG_HIGH("Created filemux muxer = %p filename = %s",
                            hdvr_session->muxer[index], sFileHandle.efs.filename);
    }

    return hdvr_status;
}
#endif

/**===========================================================================
FUNCTION filemux_deinit

@brief  DeInitialize filemux

@param [in] hdvr_session
@param [in] index

@dependencies
  None

@return
  Returns hdvr_status_type
===========================================================================*/
#ifdef ENABLE_DVR_MEDIA_MUXER
static hdvr_status_type filemux_deinit(hdvr_session_t* hdvr_session, uint32 index)
{
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;
    FileMux *muxer = (FileMux *)hdvr_session->muxer[index];

    muxer->MUX_end_Processing((void *)hdvr_session);
    WAIT_FOR_RESPONSE();
    delete muxer;
    hdvr_session->muxer[index] = NULL;

    return hdvr_status;
}
#endif

/**===========================================================================
FUNCTION hdvr_be_open

@brief  Filemux hypervisor BE open

@param [in] hypdvr_msg_type pointer

@dependencies
  None

@return
  Returns void
===========================================================================*/
static void hdvr_be_open(hdvr_session_t* hdvr_session,
                         hypdvr_msg_type* pMsgBufferNode)
{
    int handle = -1;
    int32 rc = 0;
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;
    hypdvr_msg_type msg2;

    pthread_mutex_lock(&mutex);

#ifdef ENABLE_DVR_MEDIA_MUXER
    hdvr_session->mux_file_index = -1;
    rc = MM_SignalQ_Create(&hdvr_session->signal_handle_q);
    if (rc)
    {
        HYP_VIDEO_MSG_ERROR("hyp dvr SignalQ Create failed");
        hdvr_status = HDVR_STATUS_FAIL;
    }
    else
    {
        rc = MM_Signal_Create(hdvr_session->signal_handle_q, NULL,
                              NULL, &hdvr_session->signal_handle);
        if (rc)
        {
            HYP_VIDEO_MSG_ERROR("hyp dvr Signal Create failed");
            MM_SignalQ_Release(hdvr_session->signal_handle_q);
            hdvr_status = HDVR_STATUS_FAIL;
        }
    }

    hdvr_session->qti_muxer = !!QTI_MUXER;

    if (HDVR_STATUS_SUCCESS == hdvr_status && !hdvr_session->qti_muxer)
    {
        hdvr_session->record_fd = open(RECORD_FILE_NAME,
             O_CREAT | O_LARGEFILE | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
        if (-1 != hdvr_session->record_fd)
        {
            HYP_VIDEO_MSG_ERROR("fail to open file %s for muxed data", RECORD_FILE_NAME);
            hdvr_status = HDVR_STATUS_FAIL;
        }
        else
        {
            hdvr_session->muxer[0] = new MediaMuxer(hdvr_session->record_fd,
                                  MediaMuxer::OUTPUT_FORMAT_MPEG_4);
            close(hdvr_session->record_fd);
            if (NULL == hdvr_session->muxer[0])
            {
                HYP_VIDEO_MSG_ERROR("media muxer open failed");
                hdvr_status = HDVR_STATUS_FAIL;
            }
        }
    }
#endif

    if (HDVR_STATUS_SUCCESS == hdvr_status)
    {
        handle = 0x1;
        hdvr_session->handle_64b = (int64)(handle);
    }
    msg2.msg_id = HYPDVR_MSGRESP_OPEN_RET;
    msg2.data.open_data.return_io_handle = (int64)(handle);
    msg2.data.open_data.return_status = hdvr_status;
    msg2.data.open_data.drv_session = (hypdvr_session_type)1;
    msg2.data_size = pMsgBufferNode->data_size;
    msg2.pid = (uint32) getpid();

    MM_CriticalSection_Enter(hdvr_session->send_crit_section);
    rc = hdvr_session->habmm_if.pfSend(hdvr_session->habmm_handle, &msg2,
                                       sizeof(hypdvr_msg_type), 0);
    MM_CriticalSection_Leave(hdvr_session->send_crit_section);
    if (0 != rc)
    {
        HYP_VIDEO_MSG_ERROR("habmm socket send fail rc = %d", rc);
    }
    pthread_mutex_unlock(&mutex);
}

/**===========================================================================
FUNCTION filemux_write_header

@brief  Filemux write header for a stream

@param [in] hdvr_session
@param [in] track_id
@param [in] len
@param [in] header pointer

@dependencies
  None

@return
  Returns hdvr_status_type
===========================================================================*/
#ifdef ENABLE_DVR_MEDIA_MUXER
static hdvr_status_type filemux_write_header(hdvr_session_t* hdvr_session,
                        size_t track_id, uint32 len, uint8 *header)
{
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;
    MUX_STATUS ret = MUX_SUCCESS;
    FileMux *muxer = (FileMux *)hdvr_session->muxer[hdvr_session->mux_file_index];

    ret = muxer->MUX_write_header(track_id, len, (const uint8 *)header);
    if (ret != MUX_SUCCESS)
    {
        HYP_VIDEO_MSG_ERROR("filemux write header failed. muxer = %p"
                            " mux_file_index = %d track_id = %d, len = %d header = %p",
                            (void *)muxer, hdvr_session->mux_file_index,
                            (int)track_id, len, header);
        hdvr_status = HDVR_STATUS_FAIL;
    }
    else
    {
        HYP_VIDEO_MSG_HIGH("filemux write header success. muxer = %p"
                           " mux_file_index = %d track_id = %d, len = %d header = %p",
                            (void *)muxer, hdvr_session->mux_file_index,
                            (int)track_id, len, header);
    }

    return hdvr_status;
}
#endif

/**===========================================================================
FUNCTION rotate_mux_file

@brief  Rotate mux file

@param [in] hdvr_session

@dependencies
  None

@return
  Returns hdvr_status_type
===========================================================================*/
#ifdef ENABLE_DVR_MEDIA_MUXER
static hdvr_status_type rotate_mux_file(hdvr_session_t* hdvr_session)
{
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;

    filemux_deinit(hdvr_session, hdvr_session->mux_file_index);
    hdvr_session->mux_file_index++;
    if (MAX_NUM_MUX_FILE <= hdvr_session->mux_file_index)
    {
        hdvr_session->mux_file_index = 0;
    }
    hdvr_status = filemux_init(hdvr_session, hdvr_session->mux_file_index);
    filemux_write_header(hdvr_session, VIDEO_TRACK_ID,
                         hdvr_session->video_header_len,
                         hdvr_session->video_header);
    filemux_write_header(hdvr_session, AUDIO_TRACK_ID, 2,
                         hdvr_session->audio_header);
    hdvr_session->audio_time_update = TRUE;
    hdvr_session->video_time_update = TRUE;

    return hdvr_status;
}
#endif

/**===========================================================================
FUNCTION write_to_qti_muxer

@brief  Write the audio/video buf to qti muxer

@param [in] hdvr_session
@param [in] buf_info
@param [in] va

@dependencies
  None

@return
  Returns hdvr_status_type
===========================================================================*/
#ifdef ENABLE_DVR_MEDIA_MUXER
static hdvr_status_type write_to_qti_muxer(hdvr_session_t* hdvr_session,
                                hdvr_mux_buf64_info* buf_info, char* va)
{
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;
    size_t track_id = 0;
    FileMux *muxer = NULL;

    if (-1 == hdvr_session->mux_file_index)
    {
        hdvr_session->mux_file_index = 0;
        hdvr_status = filemux_init(hdvr_session, hdvr_session->mux_file_index);
    }

    if (HDVR_STATUS_SUCCESS == hdvr_status)
    {
        MUX_STATUS ret = MUX_SUCCESS;
        muxer = (FileMux *)hdvr_session->muxer[hdvr_session->mux_file_index];

        for (uint32 i = 0; i < buf_info->buf_count && i < MAX_DVR_BUFFER_COUNT; i++)
        {
            HABMM_MEMSET(&mux_sample_info, 0, sizeof(MUX_sample_info_type));
            HYP_VIDEO_MSG_LOW("mux buf info: stream_id %d va = %p count = %d "
                              "offset = %d pts = %llu flag = 0x%x buf_size = %d "
                              "fill_len = %d", buf_info->stream, (void *)va,
                               buf_info->buf_count, buf_info->offset[i],
                               buf_info->pts[i], buf_info->flags[i],
                               buf_info->buf_size, buf_info->fill_len[i]);

            if ((MUX_FILE_SIZE_WARNING_LIMIT <= hdvr_session->mux_file_size) ||
                (MAX_MUX_FILE_SIZE <= hdvr_session->mux_file_size &&
                HDVR_STREAM_TYPE_VIDEO == buf_info->stream &&
                (HDVR_BUF_FLAG_TYPE_SYNCFRAME & buf_info->flags[i])))
            {
                if (MUX_FILE_SIZE_WARNING_LIMIT <= hdvr_session->mux_file_size)
                {
                    HYP_VIDEO_MSG_ERROR("Closing %d mux file due to %u size", hdvr_session->mux_file_index, hdvr_session->mux_file_size);
                }
                rotate_mux_file(hdvr_session);
                hdvr_session->mux_file_size = 0;
                muxer = (FileMux *)hdvr_session->muxer[hdvr_session->mux_file_index];
            }

            if (HDVR_STREAM_TYPE_AUDIO == buf_info->stream)
            {
                track_id = AUDIO_TRACK_ID;
                if (FALSE == hdvr_session->recv_audio_mux_buf)
                {
                    hdvr_session->recv_audio_mux_buf = TRUE;
                    add_kpi_marker("M - DVR:First enc audio buf");
                    hdvr_session->start_audio_time = buf_info->pts[i];
                    filemux_write_header(hdvr_session, track_id, 2,
                                         hdvr_session->audio_header);
                }

                if (TRUE == hdvr_session->audio_time_update)
                {
                    hdvr_session->audio_time_update = FALSE;
                    hdvr_session->start_audio_time = buf_info->pts[i];
                }
                mux_sample_info.time = (buf_info->pts[i] -
                    hdvr_session->start_audio_time) / MOVIE_TIMESCALE;
            }
            else if (HDVR_STREAM_TYPE_VIDEO == buf_info->stream)
            {
                track_id = VIDEO_TRACK_ID;
                if (HDVR_BUF_FLAG_TYPE_CODECCONFIG & buf_info->flags[i])
                {
                    hdvr_session->recv_video_codedconfig = TRUE;
                    add_kpi_marker("M - DVR:First enc video buf");
                    HABMM_MEMCPY(hdvr_session->video_header, va, buf_info->fill_len[i]);
                    hdvr_session->video_header_len = buf_info->fill_len[i];

                    filemux_write_header(hdvr_session, track_id,
                                         hdvr_session->video_header_len,
                                         hdvr_session->video_header);
                }
                else if (FALSE == hdvr_session->recv_video_mux_buf)
                {
                    hdvr_session->recv_video_mux_buf = TRUE;
                    hdvr_session->start_video_time = buf_info->pts[i];
                }

                mux_sample_info.time = (buf_info->pts[i] -
                    hdvr_session->start_video_time) / MOVIE_TIMESCALE;
            }

            if ((HDVR_STATUS_SUCCESS == hdvr_status)          &&
                ((TRUE == hdvr_session->recv_video_mux_buf    &&
                  HDVR_STREAM_TYPE_VIDEO == buf_info->stream) ||
                (HDVR_STREAM_TYPE_AUDIO == buf_info->stream)))
            {
                size_t adts_header_size = 0;
                uint8 *mux_buffer = NULL;
                if (HDVR_BUF_FLAG_TYPE_SYNCFRAME & buf_info->flags[i])
                {
                    mux_sample_info.sync = TRUE;
                }
                else
                {
                    mux_sample_info.sync = FALSE;
                }

                if ((HDVR_STREAM_TYPE_AUDIO == buf_info->stream) &&
                    (HDVR_AUDIO_CODEC_TYPE_AAC == hdvr_session->audio_mux_meta_info.codec))
                {
                    adts_header_size = ADTS_HEADER_SIZE_WITH_CRC;
                    if (*(va + buf_info->offset[i] + 1) & CRC_BIT_IN_ADTS_HEADER)
                    {
                        adts_header_size = ADTS_HEADER_SIZE_WITHOUT_CRC;
                    }
                    buf_info->fill_len[i] -= adts_header_size;
                    buf_info->offset[i] += adts_header_size;
                }

                mux_sample_info.size = buf_info->fill_len[i];
                mux_buffer = (uint8 *)(va + buf_info->offset[i]);
                HYP_VIDEO_MSG_INFO("MUX_Process_Sample track_id=%zu time=%d",
                                    track_id, mux_sample_info.time);
                if (hdvr_session->cache_buffer)
                {
                    HABMM_MEMCPY(hdvr_session->cache_buffer, (const uint8 *)(va + buf_info->offset[i]),
                                 buf_info->fill_len[i]);
                    mux_buffer = (uint8 *)(hdvr_session->cache_buffer);
                }

                ret = muxer->MUX_Process_Sample(track_id, 1,
                       (const MUX_sample_info_type *)&mux_sample_info,
                       (const uint8 *)(mux_buffer), NULL);

                hdvr_session->mux_file_size += buf_info->fill_len[i];

                if (!hdvr_session->muxer_write)
                {
                    hdvr_session->muxer_write = TRUE;
                    add_kpi_marker("M - DVR:First muxed frame");
                }
                if (ret != MUX_SUCCESS)
                {
                    HYP_VIDEO_MSG_ERROR("MUX_Process_Sample failed");
                    hdvr_status = HDVR_STATUS_FAIL;
                }
                else
                {
                    WAIT_FOR_RESPONSE()
                    if (0 != hdvr_session->time_out)
                    {
                        HYP_VIDEO_MSG_ERROR("timed out. failed to get response from muxer");
                    }
                }
            }
        }
    }
    else
    {
        HYP_VIDEO_MSG_ERROR("failed to init filemux");
        hdvr_status = HDVR_STATUS_FAIL;
    }

    return hdvr_status;
}
#endif

/**===========================================================================
FUNCTION write_to_media_muxer

@brief  Write the audio/video buf to media muxer

@param [in] hdvr_session
@param [in] buf_info
@param [in] va

@dependencies
  None

@return
  Returns hdvr_status_type
===========================================================================*/
#ifdef ENABLE_MEDIA_MUXER
static hdvr_status_type write_to_media_muxer(hdvr_session_t* hdvr_session,
                                             hdvr_mux_buf64_info* buf_info,
                                             char* va)
{
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;
    size_t track_id = 0;
    MediaMuxer *muxer = (MediaMuxer*)hdvr_session->muxer[0];

    for (uint32 i = 0; i < buf_info->buf_count && i < MAX_DVR_BUFFER_COUNT; i++)
    {
        HYP_VIDEO_MSG_LOW("mux buf info: stream_id %d va = %p count = %d "
                          "offset = %d pts = %llu flag = 0x%x buf_size = %d "
                          "fill_len = %d", buf_info->stream, (void *)va,
                           buf_info->buf_count, buf_info->offset[i],
                           buf_info->pts[i], buf_info->flags[i],
                           buf_info->buf_size, buf_info->fill_len[i]);

        if (HDVR_STREAM_TYPE_VIDEO == buf_info->stream)
        {
            if (HDVR_BUF_FLAG_TYPE_CODECCONFIG & buf_info->flags[i])
            {
                ABuffer *buf = new ABuffer(va, VIDEO_SPS_LEN);
                ABuffer *buf2 = new ABuffer(va + VIDEO_SPS_LEN,
                                    buf_info->fill_len[0] - VIDEO_SPS_LEN);
                hdvr_session->recv_video_codedconfig = TRUE;
                video_format->setBuffer("csd-0", buf);
                video_format->setBuffer("csd-1", buf2);
                hdvr_session->video_track = muxer->addTrack(video_format);
                add_kpi_marker("M - DVR:First enc video buf");
                HYP_VIDEO_MSG_INFO("Created video track %lu",
                                    hdvr_session->video_track);
            }
            else if (FALSE == hdvr_session->recv_video_mux_buf)
            {
                hdvr_session->recv_video_mux_buf = TRUE;
            }
            track_id = hdvr_session->video_track;
        }
        else if (HDVR_STREAM_TYPE_AUDIO == buf_info->stream)
        {
            if (FALSE == hdvr_session->recv_audio_mux_buf)
            {
                ABuffer *buf = new ABuffer(va, AUDIO_CS_LEN);
                audio_format->setBuffer("csd-0", buf);
                hdvr_session->audio_track = muxer->addTrack(audio_format);
                hdvr_session->recv_audio_mux_buf = TRUE;
                add_kpi_marker("M - DVR:First enc audio buf");
                HYP_VIDEO_MSG_INFO("Created audio track %lu",
                                    hdvr_session->audio_track);
            }
            track_id = hdvr_session->audio_track;
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("Error in stream id in buffer info ioctl. %d",
                                 buf_info->stream);
            hdvr_status = HDVR_STATUS_FAIL;
        }

        if (!hdvr_session->muxer_start &&
             hdvr_session->recv_audio_mux_buf &&
             hdvr_session->recv_video_codedconfig)
        {
            hdvr_session->muxer_start = TRUE;
            muxer->start();
            HYP_VIDEO_MSG_LOW("Muxer started");
        }

        if (HDVR_STATUS_SUCCESS == hdvr_status           &&
           ((TRUE == hdvr_session->recv_video_mux_buf    &&
             HDVR_STREAM_TYPE_VIDEO == buf_info->stream) ||
            (HDVR_STREAM_TYPE_AUDIO == buf_info->stream)))
        {
            buf_info->flags[i] |= MediaCodec::BUFFER_FLAG_SYNCFRAME;

            ABuffer *buf = new ABuffer((void* )(va + buf_info->offset[i] *
                               buf_info->buf_size), buf_info->fill_len[i]);
            muxer->writeSampleData(buf, track_id, buf_info->pts[i],
                                   buf_info->flags[i]);
            if (!hdvr_session->muxer_write)
            {
                hdvr_session->muxer_write = TRUE;
                add_kpi_marker("M - DVR:First muxed frame");
            }
        }
    }

    return hdvr_status;
}
#endif

/**===========================================================================
FUNCTION get_aac_sample_index

@brief  Mapping from samplerate to index for AAC

@param [in] samplerate

@dependencies
  None

@return
  Returns indxex
===========================================================================*/

static uint32 get_aac_sample_index(uint32 samplerate)
{
    uint32 index = 0;

    switch (samplerate)
    {
    case 96000:
        index = 0;
        break;
    case 88200:
        index = 1;
        break;
    case 64000:
        index = 2 ;
        break;
    case 48000:
        index = 3;
        break;
    case 44100:
        index = 4;
        break;
    case 32000:
        index = 5;
        break;
    case 24000:
        index = 6;
        break;
    case 22050:
        index = 7;
        break;
    case 16000:
        index = 8;
        break;
    case 12000:
        index = 9;
        break;
    case 11025:
        index = 10;
        break;
    case 8000:
        index = 11;
        break;
    default:
        index = 3;
    }

    return index;
}

/**===========================================================================
FUNCTION hdvr_be_ioctl

@brief  Filemux hypervisor BE ioctl

@param [in] hypdvr_msg_type pointer

@dependencies
  None

@return
  Returns void
===========================================================================*/
static void hdvr_be_ioctl(hdvr_session_t* hdvr_session, hypdvr_msg_type* pMsgBufferNode)
{
    int32 rc = 0;
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;
    hypdvr_msg_type msg2;
    char *va = NULL;

    pthread_mutex_lock(&mutex);
    msg2.pid = (uint32) getpid();

    HABMM_MEMSET(msg2.data.ioctl_data.payload, 0,
                 GET_IOCTL_PAYLOAD_SIZE_FROM_MSG_DATASIZE(pMsgBufferNode->data_size));
    /* initialize return ioctl payload */
    HABMM_MEMCPY(msg2.data.ioctl_data.payload, pMsgBufferNode->data.ioctl_data.payload,
                 GET_IOCTL_PAYLOAD_SIZE_FROM_MSG_DATASIZE(pMsgBufferNode->data_size));

    switch (pMsgBufferNode->data.ioctl_data.dvr_ioctl)
    {
        case IOCTL_HDVR_MUX_VIDEO_META_INFO:
        {
            HABMM_MEMCPY((void *)&hdvr_session->video_mux_meta_info,
                         (void *)msg2.data.ioctl_data.payload,
                          sizeof(hdvr_mux_video_meta_info));

            video_format = new AMessage;
            video_format->setInt32("width", hdvr_session->video_mux_meta_info.width);
            video_format->setInt32("height", hdvr_session->video_mux_meta_info.height);
            video_format->setInt32("bitrate", hdvr_session->video_mux_meta_info.bitrate);
            video_format->setInt32("frame-rate", hdvr_session->video_mux_meta_info.framerate);
            if (HDVR_VIDEO_CODEC_TYPE_H264 == hdvr_session->video_mux_meta_info.codec)
            {
                video_format->setString("mime", "video/avc");
            }
            else if (HDVR_VIDEO_CODEC_TYPE_H265 == hdvr_session->video_mux_meta_info.codec)
            {
                video_format->setString("mime", "video/hevc");
            }
            else
            {
                video_format->setString("mime", "video/avc");
            }

            HYP_VIDEO_MSG_LOW("video mux meta info width=%d height=%d "
                              "bitrate=%d framerate=%d codec=%d",
                               hdvr_session->video_mux_meta_info.width,
                               hdvr_session->video_mux_meta_info.height,
                               hdvr_session->video_mux_meta_info.bitrate,
                               hdvr_session->video_mux_meta_info.framerate,
                               hdvr_session->video_mux_meta_info.codec);

            hdvr_session->cache_buffer =
                  (void *)malloc(hdvr_session->video_mux_meta_info.width * hdvr_session->video_mux_meta_info.height * 3 * 2);
            if (NULL == hdvr_session->cache_buffer)
            {
                HYP_VIDEO_MSG_ERROR("Failed to allocate local buffer");
            }

            break;
        }
        case IOCTL_HDVR_MUX_AUDIO_META_INFO:
        {
            HABMM_MEMCPY((void *)&hdvr_session->audio_mux_meta_info,
                         (void *)msg2.data.ioctl_data.payload,
                         sizeof(hdvr_mux_audio_meta_info));

            audio_format = new AMessage;
            audio_format->setInt32("bitrate", hdvr_session->audio_mux_meta_info.bitrate);
            audio_format->setInt32("sample-rate", hdvr_session->audio_mux_meta_info.samplerate);
            audio_format->setInt32("channel-count", hdvr_session->audio_mux_meta_info.channel_count);
            audio_format->setInt32("profile", 2);
            audio_format->setInt32("max-input-size", 8192);
            if (HDVR_AUDIO_CODEC_TYPE_AAC == hdvr_session->audio_mux_meta_info.codec)
            {
                audio_format->setString("mime", "audio/mp4a-latm");
                hdvr_session->audio_header[0] = (OMX_AUDIO_AACObjectLC << 3) |
                   ((get_aac_sample_index(hdvr_session->audio_mux_meta_info.samplerate) & 0x0E) >> 1);
                hdvr_session->audio_header[1] =
                   ((get_aac_sample_index(hdvr_session->audio_mux_meta_info.samplerate) & 0x01) << 7) |
                   (hdvr_session->audio_mux_meta_info.channel_count << 3);
            }
            else if (HDVR_AUDIO_CODEC_TYPE_AMR_NB == hdvr_session->audio_mux_meta_info.codec)
            {
                audio_format->setString("mime", "audio/3gpp");
            }
            else if (HDVR_AUDIO_CODEC_TYPE_AMR_WB == hdvr_session->audio_mux_meta_info.codec)
            {
                audio_format->setString("mime", "audio/amr-wb");
            }
            else
            {
                audio_format->setString("mime", "audio/mp4a-latm");
            }

            HYP_VIDEO_MSG_LOW("audio mux meta info bitrate=%d sample-rate= %d codec = %d channel_count = %d",
                               hdvr_session->audio_mux_meta_info.bitrate,
                               hdvr_session->audio_mux_meta_info.samplerate,
                               hdvr_session->audio_mux_meta_info.codec,
                               hdvr_session->audio_mux_meta_info.channel_count);

            break;
        }
        case IOCTL_HDVR_MUX_BUF_INFO:
        {
            hdvr_mux_buf64_info* buf_info = (hdvr_mux_buf64_info* )msg2.data.ioctl_data.payload;

            HYP_VIDEO_MSG_LOW("mux buf info stream_id %d export_id = %d "
                              "buf_size = %d count = %d", buf_info->stream,
                               buf_info->export_id, buf_info->buf_size,
                               buf_info->buf_count);
            va = (char *)hypv_map_add_from_remote(&hdvr_session->habmm_if, &hdvr_session->lookup_queue,
                                                  hdvr_session->habmm_handle, buf_info->buf_size,
                                                  buf_info->export_id);
            if (NULL == va)
            {
                HYP_VIDEO_MSG_ERROR("fail to retrive va from export id %d", buf_info->export_id);
                hdvr_status = HDVR_STATUS_FAIL;
            }
            else
            {
                if (hdvr_session->qti_muxer)
                {
#ifdef ENABLE_DVR_MEDIA_MUXER
                   write_to_qti_muxer(hdvr_session, buf_info, va);
#endif
                }
                else
                {
#ifdef ENABLE_MEDIA_MUXER
                   write_to_media_muxer(hdvr_session, buf_info, va);
#endif
                }
            }

            break;
        }
        default:
        {
            HYP_VIDEO_MSG_ERROR("bad ioctl cmd = %d",
                                 pMsgBufferNode->data.ioctl_data.dvr_ioctl);
            hdvr_status = HDVR_STATUS_FAIL;
        }
    }
    msg2.data.ioctl_data.return_value = hdvr_status;
    msg2.data.ioctl_data.dvr_ioctl = pMsgBufferNode->data.ioctl_data.dvr_ioctl;
    msg2.msg_id = HYPDVR_MSGRESP_IOCTL_RET;
    msg2.data_size = pMsgBufferNode->data_size;

    MM_CriticalSection_Enter(hdvr_session->send_crit_section);
    rc = hdvr_session->habmm_if.pfSend(hdvr_session->habmm_handle, &msg2,
                                       sizeof(hypdvr_msg_type), 0);
    MM_CriticalSection_Leave(hdvr_session->send_crit_section);
    if (0 != rc)
    {
        HYP_VIDEO_MSG_ERROR("habmm socket send failed rc=%d", rc);
    }
    pthread_mutex_unlock(&mutex);
}

/**===========================================================================
FUNCTION hdvr_be_close

@brief  Filemux hypervisor BE close

@param [in] hypdvr_msg_type pointer

@dependencies
  None

@return
  Returns void
===========================================================================*/
static void hdvr_be_close(hdvr_session_t* hdvr_session,
                          hypdvr_msg_type* pMsgBufferNode)
{
    int32 rc = 0;
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;
    hypdvr_msg_type msg2;

    pthread_mutex_lock(&mutex);

    HYP_VIDEO_MSG_LOW("closing dvr BE");

#ifdef ENABLE_DVR_MEDIA_MUXER
    if (TRUE == hdvr_session->qti_muxer)
    {
        FileMux *muxer = (FileMux *)hdvr_session->muxer[hdvr_session->mux_file_index];
        if (NULL != muxer)
        {
            muxer->MUX_end_Processing((void *)hdvr_session);
            WAIT_FOR_RESPONSE();
        }
    }
    else
    {
#ifdef ENABLE_MEDIA_MUXER
        MediaMuxer *muxer = (MediaMuxer*)hdvr_session->muxer[0];
        muxer->stop();
#endif
    }
#endif

    hypv_map_cleanup(&hdvr_session->habmm_if, &hdvr_session->lookup_queue);

    if ( NULL != hdvr_session->signal_handle)
    {
        MM_Signal_Release(hdvr_session->signal_handle);
        MM_SignalQ_Release(hdvr_session->signal_handle_q);
    }

    if (NULL != pMsgBufferNode)
    {
        msg2.msg_id = HYPDVR_MSGRESP_CLOSE_RET;
        msg2.data_size = pMsgBufferNode->data_size;
        msg2.data.close_data.return_status = hdvr_status;

        MM_CriticalSection_Enter(hdvr_session->send_crit_section);
        rc = hdvr_session->habmm_if.pfSend(hdvr_session->habmm_handle, &msg2,
                                           sizeof(hypdvr_msg_type), 0);
        MM_CriticalSection_Leave(hdvr_session->send_crit_section);

        if (0 != rc)
        {
            HYP_VIDEO_MSG_ERROR("habmm socket send failed rc=%d", rc);
        }
    }
    pthread_mutex_unlock(&mutex);
}

/**===========================================================================

FUNCTION hdvr_version_check

@brief   Version validation between FE and BE

@param [in] hdvr session pointer
@param [in] msg pointer

@dependencies
  None

@return
  Returns boolean

===========================================================================*/
static hdvr_status_type hdvr_version_check(hdvr_session_t* hdvr_session,
                                           hypdvr_msg_type* msg)
{
    int32 rc = 0;
    hypdvr_msg_type msg2;
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;

    if (GET_MAJOR_REV(msg->version) != GET_MAJOR_REV(HYP_DVR_VERSION) ||
        GET_MINOR_REV(msg->version) != GET_MINOR_REV(HYP_DVR_VERSION))
    {
        hdvr_status = HDVR_STATUS_FAIL;

        msg2.msg_id = HYPDVR_MSGRESP_OPEN_RET;
        msg2.data.open_data.return_status = HDVR_STATUS_VERSION_MISMATCH;
        msg2.data_size = msg->data_size;
        msg2.virtual_channel = hdvr_session->habmm_handle;
        msg2.pid = (uint32) getpid();
        HYP_VIDEO_MSG_ERROR("Version mismatch FE(%d.%d) BE(%d.%d)",
                             GET_MAJOR_REV(msg->version), GET_MINOR_REV(msg->version),
                             GET_MAJOR_REV(HYP_DVR_VERSION), GET_MINOR_REV(HYP_DVR_VERSION));

        MM_CriticalSection_Enter(hdvr_session->send_crit_section);
        rc = hdvr_session->habmm_if.pfSend(hdvr_session->habmm_handle, &msg2,
                                           sizeof(hypdvr_msg_type), 0);
        MM_CriticalSection_Leave(hdvr_session->send_crit_section);
        if (0 != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to send socket rc %d", rc);
        }
    }

    return hdvr_status;
}

/**===========================================================================

FUNCTION hdvr_handle_request

@brief  Hypervisor DVR handle FE request

@param [in] void pointer

@dependencies
  None

@return
  Returns hdvr_status_type

===========================================================================*/
static hdvr_status_type hdvr_handle_request(hdvr_session_t* hdvr_session)
{
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;
    boolean done = FALSE;
    hypdvr_msg_type msg;
    uint32 msg_size;
    int32 rc = 0;
    uint32 retry = 0;

    while (!done)
    {
        // habmm_recv needs the msg_size for the allocated size of the msg
        msg_size =  sizeof(hypdvr_msg_type);
        rc = hdvr_session->habmm_if.pfRecv(hdvr_session->habmm_handle,
                                           (void*)&msg, &msg_size, 0, 0);
        if (hdvr_session->exit_resp_handler == TRUE)
        {
           done = TRUE;
        }
        else if (0 == rc)
        {
            retry = 0;
            switch (msg.msg_id)
            {
                case HYPDVR_MSGCMD_OPEN:
                {
                    // Version validation
                    if (HDVR_STATUS_SUCCESS != hdvr_version_check(hdvr_session, &msg))
                    {
                        done = TRUE;
                        break;
                    }
                    hdvr_be_open(hdvr_session, &msg);
                    break;
                }
                case HYPDVR_MSGCMD_IOCTL:
                {
                    hdvr_be_ioctl(hdvr_session, &msg);
                    break;
                }
                case HYPDVR_MSGCMD_CLOSE:
                {
                    hdvr_be_close(hdvr_session, &msg);
                    done = TRUE;
                    break;
                }
                default:
                {
                    HYP_VIDEO_MSG_HIGH("unrecognized msg id %d", msg.msg_id );
                    break;
                }
            }
        }
        else if ((-1 == rc) && (EAGAIN == errno) && (retry++ < MAX_HAB_RECV_RETRY))
        {
            continue;
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("hdvr receive (handle = 0x%x) returned rc = 0x%x",
                                 hdvr_session->habmm_handle, rc );
            HABMM_MEMSET((void *)&msg, 0, sizeof(hypdvr_msg_type));
            hdvr_be_close(hdvr_session, NULL);
            done = TRUE;
        }
    }
    // wait a while for the ack message to go through the hypervisor channel
    if (0 == rc)
    {
        HYP_VIDEO_MSG_LOW("500ms sleep before exit handle %d",
                           hdvr_session->habmm_handle);
        MM_Timer_Sleep(500);
    }

    return hdvr_status;
}

int main(void)
{
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;
    habIf habmm_if;
    void* dl_handle = NULL;
    int32 rc = 0;

    MM_Debug_Initialize();

    HYP_VIDEO_MSG_INFO("Hypervisor dvr server started");
    add_kpi_marker("M - DVR:Started app");

    HABMM_MEMSET((void *)&habmm_if, 0, sizeof(habmm_if));
    dl_handle = dlopen(HAB_LIB, RTLD_LOCAL);

    if (NULL == dl_handle)
    {
        HYP_VIDEO_MSG_ERROR("load hab lib %s failed", HAB_LIB);
        MM_Debug_Deinitialize();
        hdvr_status = HDVR_STATUS_FAIL;
    }
    else
    {
        HYP_VIDEO_MSG_HIGH("dlopen load hab lib %s successful", HAB_LIB );

        habmm_if.pfOpen = (hyp_habmm_socket_open)dlsym(dl_handle, "habmm_socket_open");
        habmm_if.pfClose = (hyp_habmm_socket_close)dlsym(dl_handle, "habmm_socket_close");
        habmm_if.pfSend = (hyp_habmm_socket_send)dlsym(dl_handle, "habmm_socket_send");
        habmm_if.pfRecv = (hyp_habmm_socket_recv)dlsym(dl_handle, "habmm_socket_recv");
        habmm_if.pfImport = (hyp_habmm_import)dlsym(dl_handle, "habmm_import");
        habmm_if.pfExport = (hyp_habmm_export)dlsym(dl_handle, "habmm_export");
        habmm_if.pfUnImport = (hyp_habmm_unimport)dlsym(dl_handle, "habmm_unimport");
        habmm_if.pfUnExport = (hyp_habmm_unexport)dlsym(dl_handle, "habmm_unexport");

        if (!habmm_if.pfOpen     ||
            !habmm_if.pfClose    ||
            !habmm_if.pfSend     ||
            !habmm_if.pfRecv     ||
            !habmm_if.pfImport   ||
            !habmm_if.pfExport   ||
            !habmm_if.pfUnImport ||
            !habmm_if.pfUnExport)
        {
            HYP_VIDEO_MSG_ERROR("dlsym hab lib failed" );
            dlclose(dl_handle);
            dl_handle = NULL;
            hdvr_status = HDVR_STATUS_FAIL;
        }
    }

    add_kpi_marker("M - DVR:App Ready");
    while (HDVR_STATUS_SUCCESS == hdvr_status)
    {
        int32 virtual_channel = 0;
        int32 rc = 0;

        HYP_VIDEO_MSG_HIGH("opening socket ");
        rc = habmm_if.pfOpen(&virtual_channel, HAB_MMID_CREATE(MM_VID_2, MUX_MM_SUBID), 0, 0);
        add_kpi_marker("M - DVR:Open from host");

#ifdef _LINUX_
        char *env_ptr = getenv("HDVR_DEBUG_LEVEL");
        debug_level = env_ptr ? atoi(env_ptr) : 1;
#elif defined _ANDROID_
        char property_value[PROPERTY_VALUE_MAX] = {0};

        property_get("vendor.hdvr.debug.level", property_value, "1");
        debug_level = atoi(property_value);
#endif

        if (!rc)
        {
            HYP_VIDEO_MSG_HIGH("got a connection from client");
            hdvr_session_t *hdvr_session = (hdvr_session_t* )HABMM_MALLOC(sizeof(hdvr_session_t));
            if (hdvr_session == NULL)
            {
                HYP_VIDEO_MSG_ERROR("malloc hdvr session failed");
                rc = -1;
            }
            else
            {
                HABMM_MEMSET(hdvr_session, 0, sizeof(hdvr_session_t));

                HABMM_MEMCPY(&hdvr_session->habmm_if, &habmm_if, sizeof(habIf));

                hdvr_session->habmm_handle = virtual_channel;
                hdvr_session->v4l2_write_fd = -1;
                hdvr_session->v4l2_read_fd = -1;
                hdvr_session->display_dump_fd = -1;
                hdvr_session->record_fd = -1;
                hdvr_session->cache_buffer = NULL;
                if (0 != MM_CriticalSection_Create(&hdvr_session->send_crit_section))
                {
                    HYP_VIDEO_MSG_ERROR("failed to create send critical section");
                    rc = -1;
                }
                else
                {
                    hdvr_handle_request(hdvr_session);
                }

                // wait a while for the ack message to go through the hypervisor channel
                HYP_VIDEO_MSG_LOW("500 ms sleep before exit handle %d",
                                   hdvr_session->habmm_handle);
                MM_Timer_Sleep(500);

                HYP_VIDEO_MSG_HIGH("closing habmm handle %d",
                                    hdvr_session->habmm_handle);
                hdvr_session->habmm_if.pfClose(hdvr_session->habmm_handle);
                if (NULL != hdvr_session->send_crit_section)
                {
                    MM_CriticalSection_Release(hdvr_session->send_crit_section);
                }
                if (hdvr_session->cache_buffer)
                {
                    free(hdvr_session->cache_buffer);
                }
                free(hdvr_session);
            }
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("dvr BE connection listener failed rc %d", rc);
            continue;
        }
    }

    if (NULL != dl_handle)
    {
        dlclose(dl_handle);
        dl_handle = NULL;
    }
    add_kpi_marker("M - DVR:Exiting app");
    MM_Debug_Deinitialize();

    return 0;
}

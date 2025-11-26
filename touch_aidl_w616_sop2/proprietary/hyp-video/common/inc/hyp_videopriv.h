/*===========================================================================

*//** @file hyp_videopriv.h
  This file declares datatypes and prototypes for video hypervisor FE

Copyright (c) 2016-2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/


/*===========================================================================
                             Edit History

when        who        what, where, why
--------   --------    -------------------------------------------------------
06/22/23    mm         Add flush lock to avoid race condition issue
06/02/23    mm         Fix stop time out issue
05/24/23    pc         Add HDR10 static metadata support for HEVC encode
11/28/22    tg         Fix timeout to make WAIT_FOR_RESPONSE_ACK really can time out
06/04/22    rq         support 8255 metadata backward compatible with Codec2.1.0
01/14/22    nb         Add a flag to avoid setting secure property more than once
04/01/21    sh         Bringup video on RedBend Hypervisor
03/24/21    rq         Call back only once for the flush command with FLASH_ALL type
08/27/20    sh         Bringup video decode using codec2
07/23/20    sh         Add STOP & RELEASE_RESOURCE IOCTLs to maintain proper driver & FW states
03/03/20    sh         Avoid flush call on the driver when buffers are not registered
01/09/20    sm         Support for secure playback
10/15/19    sm         Support flip and rotation simultaneously
08/23/19    sm         Enable extradata flag only when an extradata is set
06/24/19    sm         Add support for flip and rotation
05/13/19    sh         Avoid calling FTB when flush in progress
04/24/19    sm         Multi stream buffer handling during reconfig
01/30/19    rz         Add decoder dynamic input buffer mode
06/12/18    sm         Add sequence header retrieval in FE
06/05/18    sm         Initial version of hypervisor linux video BE
05/08/18    sm         Add support for 10 bit playback
01/18/18    sm         Add support for passthrough mode feature
11/23/17    sm         Update return type from MM HAB API
09/20/17    sm         Add import/export 4k alignment to meet HAB requirement and update emulation macro
09/18/17    sm         Use official habmm header
09/11/17    sm         Add support for codec config in the first encode ftb
06/23/17    sm         Streamline hypervisor context structure and definitions
05/08/17    sm         Update for new hyp-video architecture
01/30/17    sm         Update BE-FE message passing
01/13/17    sm         Synchronize HAB send calls
08/31/16    hl         Fix HAB_LIB location
08/16/16    rz         Dynamically loading hab library
07/08/16    hl         Isolate video data from ioctl
============================================================================*/


#ifndef __HYP_VIDEOPRIV_H__
#define __HYP_VIDEOPRIV_H__

#include <pthread.h>
#include "hyp_video.h"
#include "MMMalloc.h"
#include "hyp_queue_utility.h"
#include "habmm.h"

#ifdef WIN32
#include "queue.h"
#define HAB_LIB                           "libhabmm.so"
#else
#define HAB_LIB                           "libuhab.so"
#endif

#define VIDEO_HYP_EMULATION_SIGNATURE     "/base/etc/VIDEO_HYP_EMULATION"
#define VIDEO_HYP_EMULATION_HAB_LIB       "libhabmm.so"

#define HYPVID_FLAGS_USE_DYNAMIC_OUTPUT_BUFFER_OFFSET     0
#define HYPVID_FLAGS_USE_DYNAMIC_INPUT_BUFFER_OFFSET      1
#define HYPVID_FLAGS_USE_DYNAMIC_OUTPUT_BUFFER_MASK       (1 << HYPVID_FLAGS_USE_DYNAMIC_OUTPUT_BUFFER_OFFSET)
#define HYPVID_FLAGS_USE_DYNAMIC_INPUT_BUFFER_MASK        (1 << HYPVID_FLAGS_USE_DYNAMIC_INPUT_BUFFER_OFFSET)

#define MAX_MSG_SIZE              (256)
#define MAX_HYP_NAME_LENGTH        16
#define HYPV_THREAD_STACK_SIZE    (8192)
#define MAX_DEV_CMD_BUFFER_SIZE    2048
#define MAX_DPB_BUFFER_COUNT       50
#define MAX_SEQ_HEADER_LEN         1024

/* macros to extract value and info type from v4l2 control for HDR/HDR10+ */

#define INFO_TYPE_OFFSET  28
#define INFO_TYPE_MASK    0xF
#define INFO_VALUE_MASK   0xFFFFFFF

#ifdef WIN32
#define HVFE_TIMEOUT_INTERVAL_IN_MS   0xffffffff
#else
#define HVFE_TIMEOUT_INTERVAL_IN_MS   1000 /* 1 second */
#endif

#define UNUSED(x) (void)(x)

#define V4L2FE_ALIGN(x, to_align) ((((unsigned) x) + (to_align - 1)) & ~(to_align - 1))

typedef void * IO_HANDLE;

/** used by FE/BE to lookup map table */
typedef struct _hypv_lookup_table_t
{
    struct hypv_map_entry_t *linkhead;
    struct hypv_map_entry_t *linktail;
}hypv_lookup_table_t;
typedef struct _hypv_lookup_table_ex_t
{
    struct hypv_map_entry_ex_t *linkhead;
    struct hypv_map_entry_ex_t *linktail;
}hypv_lookup_table_ex_t;

typedef int32_t (*hyp_habmm_socket_open)(int32_t*, uint32_t, uint32_t, uint32_t);
typedef int32_t (*hyp_habmm_socket_close)(int32_t);
typedef int32_t (*hyp_habmm_socket_send)(int32_t, void*, uint32_t, uint32_t);
typedef int32_t (*hyp_habmm_socket_recv)(int32_t, void*, uint32_t*, uint32_t, uint32_t);
typedef int32_t (*hyp_habmm_export)(int32_t, void*, uint32_t, uint32_t*, uint32_t);
typedef int32_t (*hyp_habmm_unexport)(int32_t, uint32_t, uint32_t);
typedef int32_t (*hyp_habmm_import)(int32_t, void**, uint32_t, uint32_t, uint32_t);
typedef int32_t (*hyp_habmm_unimport)(int32_t, uint32_t, void*, uint32_t);

typedef struct
{
    hyp_habmm_socket_open       pfOpen;
    hyp_habmm_socket_close      pfClose;
    hyp_habmm_socket_send       pfSend;
    hyp_habmm_socket_recv       pfRecv;
    hyp_habmm_export            pfExport;
    hyp_habmm_unexport          pfUnExport;
    hyp_habmm_import            pfImport;
    hyp_habmm_unimport          pfUnImport;
} habIf;

typedef struct hypv_session_t hypv_session_t;


/* callback function prototype */
typedef int (*callback_handler_t)
(
    uint8* msg,
    uint32 length,
    void* cd
);

/* hypervisor callback data type */
typedef struct
{
    callback_handler_t handler;
    void* data;
} hypv_callback_t;

/* hypervisor client handle session type */
struct hypv_session_t
{
   int64                  handle_64b;                   /* device open handle */
   void*                  dl_handle;                    /* dlopen handle */
   habIf                  habmm_if;                     /* habmm function pointers */
   MM_HANDLE              hvfe_event_cb_thread;         /* FE event callback thread */
   MM_HANDLE              hvbe_callback_thread;         /* BE callback thread */
   MM_HANDLE              hvbeDaemonThread;             /* BE daemon thread */
   pthread_mutex_t        event_handler_mutex;
   pthread_cond_t         event_handler_cond;
   hypv_callback_t        hvfe_client_cb;               /* registered client callback */
   MM_HANDLE              sendCritSection;
   MM_HANDLE              hEnterLock;
   IoctlHabmmQueueInfo    habmm_queue_info;
   int32                  habmm_handle;
   hypvideo_msg_data_type hypvCmdResponse;               /* response msg for habmm_socket_recv */
   uint32                 hypvid_msg_number;             /* msg number in sequence */
   uint32                 hypvid_time_stamp;             /* system time */
   int32                  time_out;
   hypv_lookup_table_t    lookup_queue;
   hypv_lookup_table_ex_t lookup_queue_ex;
   boolean                exit_resp_handler;
   uint32                 hypvid_flags;
   MM_HANDLE              hvfe_response_cb_thread;       /* FE response callback thread */
   video_session_type     video_session;
   vidc_codec_type        codec;
   uint32                 ftb_count;
   uint32                 etb_count;
   boolean                passthrough_mode;
   hyp_platform_type      hyp_platform;
   boolean                multi_stream_enable;
   boolean                dpb_ftb;
   uint32                 dpb_buf_count;
   uint32                 dpb_buf_size;
   uint32                 dpb_exdata_buf_size;
   uint32                 bit_depth;
   uint8*                 dpb_buf_addr[MAX_DPB_BUFFER_COUNT];
   int                    epollfd;
   hypv_callback_t        hvbe_session_cb;
   int                    num_input_planes;
   int                    num_output_planes;
   boolean                be_thread_stop;
   uint32                 fbd_count;
   vidc_frame_data_type   first_frame_info;
   vidc_rotation_type     rotate;
   vidc_flip_type         flip;
   boolean                input_extradata_enable;
   MM_HANDLE              flush_lock;            /* lock for flush operation */
   uint32                 flush_req;             /* store the flush mode bits requested by client */
   uint32                 flush_clr;             /* stored mode bits to be cleared by flush done events */
   boolean                secure;
   boolean                set_secure_property;
   boolean                in_output_reconfig;
   boolean                export_as_fd;
   hyp_target_variant_type   target_variant;
};

#ifdef WIN32
typedef struct
{
   uint8 msg_buf[MAX_MSG_SIZE];
   int msg_len;
}hypvMsgType;

typedef struct
{
   q_link_type   link;
   hypvMsgType   msg;
}hypvMsgQType;
#endif
#endif /* __HYP_VIDEOPRIV_H__ */

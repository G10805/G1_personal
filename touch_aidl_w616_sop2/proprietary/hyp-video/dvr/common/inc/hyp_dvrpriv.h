/*===========================================================================

*//** @file hyp_dvrpriv.h
  This file declares datatypes and prototypes for dvr hypervisor

Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/


/*===========================================================================
                             Edit History

$Header: $

when        who        what, where, why
--------   --------    -------------------------------------------------------
01/15/20    sh          Bringup DVR on LA GVM on Hana
09/23/19    sh          Allocate local cacheable buffer to reduce CPU load
04/10/19    sh          Disable import/unimport of buffers during muxing
03/28/19    sh          Add support for HEVC
11/22/18    sm          Add support for circular mux file storage
11/16/18    sm          Add support for runtime preview enable/disable
11/13/18    sm          Fix audio playback from container format
10/12/18    sm          Fix offset overflow
09/28/18    sm          Add support for QTI muxer
08/22/18    sm          Initial version of hypervisor DVR
============================================================================*/

#ifndef __HYP_DVRPRIV_H__
#define __HYP_DVRPRIV_H__

#include "AEEstd.h" /* std typedefs, ie. byte, uint16, uint32, etc. and memcpy, memset */
#include "MMMalloc.h"
#include "MMDebugMsg.h"
#include "hyp_dvr.h"
#include "hyp_dvrinf.h"
#include "habmm.h"

#define HAB_LIB "libuhab.so"

#define HDVR_THREAD_STACK_SIZE        (8192)
#define HDVR_FE_TIMEOUT_INTERVAL_IN_MS 1000
#define MAX_DVR_BUFFER_COUNT           128
#define MAX_AUDIO_HEADER_LEN           32
#define MAX_VIDEO_HEADER_LEN           78

#define UNUSED(x) (void)(x)
#define BUF_ALIGN(x, to_align) ((((unsigned) x) + (to_align - 1)) & ~(to_align - 1))

#define HYP_DVR_MAJOR_REV  1
#define HYP_DVR_MINOR_REV  0
#define MINOR_REV_BITS     16
#define MAJOR_REV_MASK     ((1U << MINOR_REV_BITS) - 1)
#define HYP_DVR_VERSION    ((HYP_DVR_MAJOR_REV << MINOR_REV_BITS) | HYP_DVR_MINOR_REV)
#define GET_MAJOR_REV(ver) (ver >> MINOR_REV_BITS)
#define GET_MINOR_REV(ver) (ver & MAJOR_REV_MASK)

#define MAX_NUM_MUX_FILE    5

// Heap mmory usage
#define HABMM_MALLOC malloc
#define HABMM_FREE( mem )               { if((mem)) free(mem); \
                                             (mem) = NULL;}
#define HABMM_MEMSET(src,value,len)      std_memset((src),(value),(len))
#define HABMM_MEMCPY(dest,src,len)       std_memmove((dest),(src),(len))

typedef void * IO_HANDLE;

/** used by FE/BE to lookup map table */
typedef struct _hypv_lookup_table_t
{
    struct hypv_map_entry_t *linkhead;
    struct hypv_map_entry_t *linktail;
} hypv_lookup_table_t;

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
    hyp_habmm_socket_open  pfOpen;
    hyp_habmm_socket_close pfClose;
    hyp_habmm_socket_send  pfSend;
    hyp_habmm_socket_recv  pfRecv;
    hyp_habmm_export       pfExport;
    hyp_habmm_unexport     pfUnExport;
    hyp_habmm_import       pfImport;
    hyp_habmm_unimport     pfUnImport;
} habIf;

/* callback function prototype */
typedef int (*callback_handler_t)
(
    uint8* msg,
    uint32 length,
    void*  cd
);

/* hypervisor callback data type */
typedef struct
{
    callback_handler_t handler;
    void*              data;
} hdvr_callback_t;

/* hypervisor client handle session type */
typedef struct
{
    int64                    handle_64b;                        /* device open handle */
    int64                    preview_handle_64b;                        /* device open handle */
    void*                    dl_handle;                         /* dlopen handle */
    habIf                    habmm_if;                          /* habmm function pointers */
    MM_HANDLE                signal_handle_q;
    MM_HANDLE                signal_handle;
    hdvr_callback_t          hdvr_fe_client_cb;                 /* registered client callback */
    MM_HANDLE                send_crit_section;
    MM_HANDLE                hEnterLock;
    int32                    habmm_handle;
    int32                    habmm_preview_handle;
    hypdvr_msg_data_type     hdvr_cmd_response;                  /* response msg for habmm_socket_recv */
    uint32                   hdvr_msg_number;                    /* msg number in sequence */
    uint32                   hdvr_time_stamp;                    /* system time */
    int32                    time_out;
    hypv_lookup_table_t      lookup_queue;
    boolean                  exit_resp_handler;
    MM_HANDLE                hdvr_fe_response_cb_thread;         /* FE response callback thread */
    boolean                  be_thread_stop;
    void*                    muxer[MAX_NUM_MUX_FILE];            /* muxer handle */
    int                      record_fd;                          /* file descriptor to dump muxed data */
    int                      display_dump_fd;                    /* file descriptor to dump display buffers */
    int                      v4l2_write_fd;                      /* v4l2loopback write fd */
    int                      v4l2_read_fd;                       /* v4l2loopback read fd */
    uint32                   display_buf_size;                   /* display preview size */
    boolean                  display_init_done;                  /* flag to indicate display initialization is done */
    boolean                  recv_video_mux_buf;                 /* flag to indicate recv of non codedconfig video encode buffer */
    boolean                  recv_video_codedconfig;             /* flag to indicate recv of video codedconfig buffer */
    boolean                  recv_audio_mux_buf;                 /* flag to indicate recv of audio encoded buffers */
    boolean                  recv_disp_buf;                      /* flag to indicate recv of display buffer */
    size_t                   video_track;                        /* muxer video track */
    size_t                   audio_track;                        /* muxer audio track */
    boolean                  muxer_start;                        /* flag to indicate muxer has been initialized */
    boolean                  muxer_write;                        /* flag to indicate muxer write has started */
    boolean                  v4l2loopback_enable;                /* flag to indicate v4l2loopback is enabled */
    hdvr_mux_video_meta_info video_mux_meta_info;                /* session mux video meta data */
    hdvr_mux_audio_meta_info audio_mux_meta_info;                /* session mux audio meta data */
    uint64                   start_video_time;                   /* start video time */
    uint64                   start_audio_time;                   /* start audio time */
    boolean                  qti_muxer;                          /* flag to indicate qti muxer is used */
    boolean                  first_preview;                      /* flag to indicate the first preview has been done */
    uint8                    audio_header[MAX_AUDIO_HEADER_LEN]; /* audio header data */
    uint8                    video_header[MAX_VIDEO_HEADER_LEN]; /* video header data */
    uint32                   video_header_len;                   /* video header len */
    int32                    mux_file_index;                     /* index the current muxed file */
    uint32                   mux_file_size;                      /* mux file size */
    boolean                  video_time_update;                  /* flag to indicate video pts time need update */
    boolean                  audio_time_update;                  /* flag to indicate audio pts time need update */
    void*                    cache_buffer;                       /* local buffer to make use of caching */
} hdvr_session_t;

typedef struct
{
    uint32 buf_size;
    uint32 buf_count;
    uint32 fill_len[MAX_DVR_BUFFER_COUNT];
    uint64 pts[MAX_DVR_BUFFER_COUNT];
    uint8  flags[MAX_DVR_BUFFER_COUNT];
    uint32 offset[MAX_DVR_BUFFER_COUNT];
    uint32 stream;
    uint32 export_id;
} hdvr_mux_buf64_info;

typedef struct
{
    uint32 buf_size;
    uint32 fill_len;
    uint32 height;
    uint32 width;
    uint32 format;
    uint32 colorspace;
    uint32 field;
    uint32 sequence;
    uint32 export_id;
} hdvr_disp_buf64_info;

#endif /* __HYP_DVRPRIV_H__ */

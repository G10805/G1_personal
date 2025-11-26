/*===========================================================================

*//** @file hyp_vpppriv.h
This file declares datatypes and prototypes for VPP hypervisor

Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/

#ifndef __HYP_VPP_PRIV_H__
#define __HYP_VPP_PRIV_H__

#include <map>
#include <pthread.h>
#include <sys/time.h>
#include "AEEstd.h" /* std typedefs, ie. byte, uint16, uint32, etc. and memcpy, memset */
#include "MMMalloc.h"
#include "MMDebugMsg.h"
#include "hyp_vpp_queue_util.h"

#include "hyp_vpp.h"
#include "hyp_vppinf.h"
#include "habmm.h"

#include "vpp.h"

#define HAB_LIB "libuhab.so"

#define UNUSED(x) (void)(x)
#define BUF_ALIGN(x, to_align) ((((unsigned) x) + (to_align - 1)) & ~(to_align - 1))
#define FOURCC(a, b, c, d) \
        ( (uint32)(a) | ((uint32) (b)) << 8  | ((uint32) (c)) << 16 | ((uint32) (d)) << 24 )
#define VALID_HANDLE FOURCC('H','V','P','P')

#define HYP_VPP_MAJOR_REV  1
#define HYP_VPP_MINOR_REV  0
#define MINOR_REV_BITS       16
#define MAJOR_REV_MASK       ((1U << MINOR_REV_BITS) - 1)
#define HYP_VPP_VERSION ((HYP_VPP_MAJOR_REV << MINOR_REV_BITS) | HYP_VPP_MINOR_REV)
#define GET_MAJOR_REV(ver) (ver >> MINOR_REV_BITS)
#define GET_MINOR_REV(ver) (ver & MAJOR_REV_MASK)


#define HYPV_THREAD_STACK_SIZE    (8192)
#define HVFE_TIMEOUT_INTERVAL_IN_MS   1000 /* 1 second */

#define UNUSED(x) (void)(x)
#define VPPFE_ALIGN(x, to_align) ((((unsigned) x) + (to_align - 1)) & ~(to_align - 1))

#define MAX_DEVICE_NAME_LEN 32

#define BUF_REQ_LIMIT  10
#define EXTRA_BUF_REQ  6

// Heap mmory usage
#define HABMM_MALLOC malloc
#define HABMM_FREE(mem)                  { if((mem)) free(mem); \
                                              (mem) = NULL; }
#define HABMM_MEMSET(src,value,len)      std_memset((src),(value),(len))
#define HABMM_MEMCPY(dest,src,len)       std_memmove((dest),(src),(len))

typedef void * IO_HANDLE;

/** used by FE/BE to lookup map table */
typedef struct _hypvpp_lookup_table_t
{
    struct hypvpp_map_entry_t *linkhead;
    struct hypvpp_map_entry_t *linktail;
    MM_HANDLE                 mutex;
} hypvpp_lookup_table_t;

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
typedef int (*callback_handler_t) ( uint8* msg, uint32 length, void* context);


/* hypervisor callback data type */
typedef struct
{
    callback_handler_t handler;
    void*              data;
} hypvpp_callback_t;

struct statistics
{
public:
    statistics();
    ~statistics();
    void input_queue();
    void output_done();
    void start();
    static int thread_loop(void*);
    void add_entry(struct vpp_buffer *buf);
    uint64_t get_latency(struct vpp_buffer *buf);

private:

    uint64_t current_time_ms()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (tv.tv_sec * 1e6 + tv.tv_usec) / 1000;
    }

    MM_HANDLE monitor_thread;
    MM_HANDLE monitor_lock;
    MM_HANDLE monitor_sig;
    MM_HANDLE monitor_sig_q;
    uint32_t num_input_queue;
    uint32_t num_output_done;
    uint32_t prev_num_input_queue;
    uint32_t prev_num_output_done;
    uint64_t last_sys_time_ms;
    uint64_t total_latency;
    int32_t timeout; /* ms */
    bool started;
    bool thread_stop;
    std::map<void*, uint64_t> in_out_time_map;
};

/* hypervisor client handle session type */
typedef struct
{
    int32                  handle_header;                /* check handle valid or not */
    int64                  device_handle;                /* device open handle */
    void*                  dl_handle;                    /* dlopen handle */
    habIf                  habmm_if;                     /* habmm function pointers */
    MM_HANDLE              hvfe_event_cb_thread;         /* FE event callback thread */
    MM_HANDLE              hvfe_response_cb_thread;      /* FE response callback thread */
    MM_HANDLE              hvbe_callback_thread;         /* BE callback thread */
    MM_HANDLE              hvbe_daemon_thread;           /* BE daemon thread */
    pthread_mutex_t        event_handler_mutex;
    pthread_cond_t         event_handler_cond;
    MM_HANDLE              send_crit_section;
    MM_HANDLE              hab_lock;                     /* Hab communication lock */

    habmm_queue_info_type  habmm_queue_info;
    int32                  habmm_handle;
    hypvpp_msg_data_type   hypvpp_cmd_resp;                /* response msg for habmm_socket_recv */
    uint32                 hypvpp_msg_number;            /* msg number in sequence */
    uint32                 hypvpp_time_stamp;            /* system time */
    int32                  time_out;
    hypvpp_lookup_table_t  lookup_queue;
    boolean                exit_resp_handler;
    boolean                be_thread_stop;
    boolean                secure;

    MM_HANDLE              lock_buffer;
    MM_HANDLE              state_synch_obj_q;
    MM_HANDLE              state_synch_obj;

    vpp_callbacks          vpp_cb;
    statistics             *stat;
} hypvpp_session_t;

#endif /* __HYP_VPP_PRIV_H__ */

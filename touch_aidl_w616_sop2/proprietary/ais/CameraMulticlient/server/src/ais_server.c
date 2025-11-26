/**
 * @file ais_server.c
 *
 * @brief implementation of ais server which handle both host and hypervisor
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <semaphore.h>
#include <sys/system_properties.h>
#include <linux/kernel.h>

#if defined(__QNXNTO__)
#include <login.h>
#include <libgen.h>
#endif

#include "ais_comm.h"
#include "ais_log.h"

#include "ais_conn.h"
#include "ais_event_queue.h"

#include "CameraResult.h"
#include "CameraOSServices.h"

#ifdef ENABLE_DIAGNOSTIC
#include "ais_diag_server.h"
#endif

#if defined(__AGL__)
#include <limits.h>
#endif

#if defined(__INTEGRITY)
#include "pmem.h"
#include "pmemext.h"

// Object number 10 is defined for server VAS in corresponding INT file
#define AIS_SERV_OBJ_NUM  10
// Maximum thread priority for AIS server
const Value __PosixServerPriority = CAM_POSIXSERVER_PRIORITY;
#define AIS_CMD_THREAD_STACK_SIZE (1 * 16 * 1024) /*16 KB*/
#else
#define AIS_CMD_THREAD_STACK_SIZE (1 * 1024 * 1024) /*1 MB*/
#endif


//Logging macros
#define AIS_LOG_SRV(level, fmt...) AIS_LOG2(SERVER, level, fmt)

#define AIS_LOG_SRV_ERR(fmt...) AIS_LOG(SERVER, ERROR, fmt)
#define AIS_LOG_SRV_WARN(fmt...) AIS_LOG(SERVER, WARN, fmt)
#define AIS_LOG_SRV_HIGH(fmt...) AIS_LOG(SERVER, HIGH, fmt)
#define AIS_LOG_SRV_MED(fmt...) AIS_LOG(SERVER, MED, fmt)
#define AIS_LOG_SRV_LOW(fmt...) AIS_LOG(SERVER, LOW, fmt)
#define AIS_LOG_SRV_DBG(fmt...) AIS_LOG(SERVER, DBG, fmt)
#define AIS_LOG_SRV_DBG1(fmt...) AIS_LOG(SERVER, DBG1, fmt)

#define AIS_LOG_LVL_SRV_API AIS_LOG_LVL_LOW
#define AIS_LOG_SRV_API(fmt...) AIS_LOG2(SERVER, AIS_LOG_LVL_SRV_API, fmt)


// Priority for ais_server worker threads
#if defined(USE_HYP)
#define AIS_SRV_THRD_PRIO CAMERA_THREAD_PRIO_BACKEND
#else
#define AIS_SRV_THRD_PRIO CAMERA_THREAD_PRIO_NORMAL
#endif


#define AIS_SERVER_CLIENT_CTXT_MAGIC  0xA1500A15

static CameraThread signal_thread_handle = NULL;


/**
 * thread parameter for main or work connection
 */
typedef struct
{
    void         *p;  /**< server context */
    unsigned int idx; /**< connection index */
} s_ais_server_thread_param;


/**
 * server management structures
 */
typedef struct
{
    /** client ctxt identifier AIS_SERVER_CLIENT_CTXT_MAGIC */
    uint32 magic;

    /** index of client ctxt */
    uint32 idx;

    volatile qcarcam_hndl_t qcarcam_hndl;

    s_ais_conn_info info;

    /** command connections */
    s_ais_conn cmd_conn[AIS_CONN_CMD_MAX_NUM];

    /** command threads */
    s_ais_server_thread_param cmd_thread_param[AIS_CONN_CMD_MAX_NUM];
    CameraThread  cmd_thread_id[AIS_CONN_CMD_MAX_NUM];
    volatile bool cmd_thread_abort[AIS_CONN_CMD_MAX_NUM];

    /** event connection */
    s_ais_conn event_conn;

    /** event queue between cb and sender */
    s_ais_event_queue event_queue;

    /** event thread */
    CameraThread  event_thread_id;
    volatile bool event_thread_abort;

    /** local content */
    qcarcam_event_cb_t event_cb;        // callback associated with the handle
    void* priv_data;                    // QCARCAM_PARAM_PRIVATE_DATA

    /** ctxt health */
    unsigned int signal_attempts;       // count failed wait for signal attempts
    volatile bool signal_bit;           // used to check if context has been signaled
    volatile bool health_active;        // enables health check for currect context
    volatile bool ctxt_abort;           // set if context is in the process of being closed

    pthread_mutex_t event_queue_lock;   // Mutex to create and destroy event queue
} s_ais_client_ctxt;


/**
 * Function declarations
 */
static int ais_server_exit(void);
static int ais_server_signal_thread(void *arg);
static void ais_server_register_signal_handler(void);
static char* ais_server_get_name(char *p_path);
static int ais_server_singleton_acquire(char *p_name);
static int ais_server_singleton_release(void);
static int ais_server_check_version(unsigned int version);
static int ais_server_find(qcarcam_hndl_t handle);
static int ais_server_create_conns(s_ais_client_ctxt *p);
static int ais_server_create_conns_thread(void *p_arg);
static int ais_server_create_main_conn(s_ais_client_ctxt *p);
static int ais_server_destroy_main_conn(s_ais_client_ctxt *p);
static int ais_server_create_work_thread(s_ais_client_ctxt *p);
static int ais_server_destroy_work_thread(s_ais_client_ctxt *p);
static int ais_server_release_work_thread(s_ais_client_ctxt *p);
static int ais_server_create_work_conn(s_ais_client_ctxt *p);
static int ais_server_destroy_work_conn(s_ais_client_ctxt *p);
static void ais_server_health_event_cb(s_ais_client_ctxt *p, qcarcam_event_t event_id, qcarcam_event_payload_t *p_payload);
static void ais_server_event_cb(qcarcam_hndl_t handle, qcarcam_event_t event_id, qcarcam_event_payload_t *p_payload);
static int ais_server_event_send_thread(void *p_arg);
static int ais_server_create_event_thread(s_ais_client_ctxt *p);
static int ais_server_destroy_event_thread(s_ais_client_ctxt *p);
static int ais_server_release_event_thread(s_ais_client_ctxt *p);
static int ais_server_create_event_conn(s_ais_client_ctxt *p);
static int ais_server_destroy_event_conn(s_ais_client_ctxt *p);
static int ais_server_exchange(s_ais_client_ctxt *p, s_ais_conn *p_conn, s_ais_conn_info *p_info);

static int ais_server_create_client_ctxt(s_ais_conn *p_conn);
static int ais_server_destroy_client_ctxt(s_ais_client_ctxt *p);
static void ais_server_init_client_ctxt(s_ais_client_ctxt *p, uint32 idx);
static s_ais_client_ctxt* ais_server_alloc_client_ctxt(void);
static int ais_server_free_client_ctxt(s_ais_client_ctxt *p);

static CameraResult ais_server_query_inputs(s_ais_client_ctxt *p, s_ais_cmd_query_inputs *p_param);
static CameraResult ais_server_query_inputs_v2(s_ais_client_ctxt *p, s_ais_cmd_query_inputs_v2 *p_param);
static CameraResult ais_server_open(s_ais_client_ctxt *p, s_ais_cmd_open *p_param);
static CameraResult ais_server_close(s_ais_client_ctxt *p, s_ais_cmd_close *p_param);
static CameraResult ais_server_g_param(s_ais_client_ctxt *p, int idx, s_ais_cmd_g_param *p_param);
static CameraResult ais_server_s_param(s_ais_client_ctxt *p, int idx, s_ais_cmd_s_param *p_param);
static CameraResult ais_server_s_buffers(s_ais_client_ctxt *p, int idx, s_ais_cmd_s_buffers *p_param);
static CameraResult ais_server_s_buffers_v2(s_ais_client_ctxt *p, int idx, s_ais_cmd_s_buffers_v2 *p_param);
static CameraResult ais_server_start(s_ais_client_ctxt *p, s_ais_cmd_start *p_param);
static CameraResult ais_server_stop(s_ais_client_ctxt *p, s_ais_cmd_stop *p_param);
static CameraResult ais_server_pause(s_ais_client_ctxt *p, s_ais_cmd_pause *p_param);
static CameraResult ais_server_resume(s_ais_client_ctxt *p, s_ais_cmd_resume *p_param);
static CameraResult ais_server_get_frame(s_ais_client_ctxt *p, s_ais_cmd_get_frame *p_param);
static CameraResult ais_server_get_frame_v2(s_ais_client_ctxt *p, s_ais_cmd_get_frame_v2 *p_param);
static CameraResult ais_server_release_frame(s_ais_client_ctxt *p, s_ais_cmd_release_frame *p_param);
static CameraResult ais_server_release_frame_v2(s_ais_client_ctxt *p, s_ais_cmd_release_frame_v2 *p_param);
static CameraResult ais_server_get_event(s_ais_client_ctxt *p, s_ais_cmd_get_event *p_param);
static int ais_server_process_cmd(s_ais_client_ctxt *p, unsigned int idx, u_ais_cmd *p_param);
static int ais_server_cmd_thread(void *p_arg);


/**
 * all client infos are stored here.
 * all operations to this array are guarded by single variable
 * and operations sequence in client thread.
 * so mutex is not necessary
 * android doesn't support current implementation,
 * need to use other way to handle it later
 */
static char  sgp_singleton_name[64];
#if defined(__QNXNTO__) || defined(__INTEGRITY)
static sem_t *sgsp_singleton_sem = NULL;
#elif defined(__ANDROID__) || defined(__AGL__) || defined(_LINUX)
static int sg_singleton_fd = -1;
#endif

static pthread_mutex_t sgs_mutex = PTHREAD_MUTEX_INITIALIZER;
static s_ais_client_ctxt sgs_ais_client_ctxt[AIS_MAX_USR_CONTEXTS];
static s_ais_conn sgs_ais_server_conn;

/**
 * signal handling
 */
static const int exceptsigs[] = {
    SIGCHLD, SIGIO, SIGURG, SIGWINCH,
    SIGSTOP, SIGTSTP, SIGTTIN, SIGTTOU, SIGCONT, SIGSEGV,
    -1,
};

static volatile int sg_abort = 0;


/**
 * closes all connections and free all resouces.
 *
 * @return 0: success
 *
 */
static int ais_server_exit(void)
{
    int i;

    AIS_LOG_SRV_HIGH("E");

    sg_abort = 1;

    for (i = 0; i < AIS_MAX_USR_CONTEXTS; i++)
    {
        ais_server_destroy_client_ctxt(&sgs_ais_client_ctxt[i]);
    }

    AIS_CONN_API(ais_conn_close)(&sgs_ais_server_conn);

    AIS_LOG_SRV_HIGH("X");

    return 0;
}

/**
 *  signal handle thread
 *
 * @return none
 */
static int ais_server_signal_thread(void *arg)
{
    sigset_t sigset;
    int sig, i, rc = 0;

    CameraDetachThread(signal_thread_handle);

    sigfillset(&sigset);
    for (i = 0; exceptsigs[i] != -1; i++)
    {
        sigdelset(&sigset, exceptsigs[i]);
    }

    AIS_LOG_SRV(AIS_LOG_LVL_HIGH, "E");

    for (;;)
    {
        rc = sigwait(&sigset, &sig);
        if ( rc == 0)
        {
            /**
             * in QNX, after client closes the socket fd, ais_server uses the socket, will receive the SIGPIPE
             * in LV, doesn't come across it
             * */
            if (sig == SIGPIPE)
            {
                AIS_LOG_SRV(AIS_LOG_LVL_ALWAYS, "receive the signal SIGPIPE");
                continue;
            }
            else
            {
                AIS_LOG_SRV(AIS_LOG_LVL_ALWAYS, "receive the signal %d", sig);
                ais_server_exit();
                break;
            }
        }
        else
        {
            AIS_LOG_SRV_ERR("sigwait failed rc=%d", rc);
        }
    }

    AIS_LOG_SRV(AIS_LOG_LVL_ALWAYS, "X");
    return 0;
}

/**
 * registers all signals which needs to be handled.
 *
 * @return none
 *
 */
static void ais_server_register_signal_handler(void)
{
    int i = 0;
    int rc = 0;
    char name[32];
    sigset_t sigset;

    sigfillset(&sigset);
    for (i = 0; exceptsigs[i] != -1; i++)
    {
        sigdelset(&sigset, exceptsigs[i]);
    }
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    AIS_LOG_SRV_HIGH("E");

    snprintf(name, sizeof(name), "ais_server_signal_thread");

    rc = CameraCreateThread(AIS_SRV_THRD_PRIO,
                            0,
                            &ais_server_signal_thread,
                            NULL,
                            0,
                            name,
                            &signal_thread_handle);

    if (rc != 0)
    {
        AIS_LOG_SRV_ERR("CameraCreateThread rc = %d", rc);
    }

    AIS_LOG_SRV(rc == 0 ? AIS_LOG_LVL_HIGH : AIS_LOG_LVL_ERROR,
                "X %d",  rc);
}

/**
 * gets the name of program from the whole path.
 * @param p_path points the path of this program
 *
 * @return the name or NULL
 *
 */
static char* ais_server_get_name(char *p_path)
{
    char *p = NULL;

    if (p_path == NULL)
    {
        goto EXIT_FLAG;
    }

    p = strrchr(p_path, '/');

    if (p == NULL)
    {
        p = strrchr(p_path, ' ');

        if ( p == NULL )
        {
            p = p_path;
        }
        else
        {
            p++;
        }
    }
    else {
        p++;
    }

EXIT_FLAG:

    return p;
}

/**
 * creates and acquires singleton semaphore.
 * @param p_name points the name of this program
 *
 * @return 0: success, others: fail
 *
 */
static int ais_server_singleton_acquire(char *p_name)
{
    int rc = 0;

    AIS_LOG_SRV_HIGH("E %p", p_name);

    if (p_name == NULL)
    {
        goto EXIT_FLAG;
    }

#if defined(__QNXNTO__) || defined(__INTEGRITY)
    // if enable given name, have to start with a slash
    snprintf(sgp_singleton_name, sizeof(sgp_singleton_name), "/%s", p_name);
    AIS_LOG_SRV_MED("%s %p %s", p_name, sgsp_singleton_sem, sgp_singleton_name);

    sgsp_singleton_sem = sem_open(sgp_singleton_name, O_CREAT|O_EXCL, S_IRWXU, 0);
    if (sgsp_singleton_sem == SEM_FAILED)
    {
        rc = -2;
    }
#elif defined(CAMERA_UNITTEST) || defined(__AGL__)
    snprintf(sgp_singleton_name, sizeof(sgp_singleton_name), "%s/%s.lock", AIS_TEMP_PATH, p_name);
    AIS_LOG_SRV_MED("%s %d %s", p_name, sg_singleton_fd, sgp_singleton_name);

    int poll_ms = 0;
    // In early RVC case, ais_server will be launched very early, waiting here for AIS_TEMP_PATH writeable ready,
    // to make sure sem_open() can work properly in time.
    while (access(AIS_TEMP_PATH, W_OK) != 0) {
        if (poll_ms > 2000) {
            AIS_LOG_SRV_ERR("Polling %s writable fail, poll time = %d", AIS_TEMP_PATH, poll_ms);
            break;
        }
        usleep(5 * 1000);
        poll_ms += 5;
    }

    sg_singleton_fd = open(sgp_singleton_name, O_CREAT|O_EXCL, S_IRWXU);
    if (sg_singleton_fd == -1)
    {
        rc = -2;
    }
#elif defined(__ANDROID__)
    snprintf(sgp_singleton_name, sizeof(sgp_singleton_name), "%s/%s.lock", AIS_TEMP_PATH, p_name);
    AIS_LOG_SRV_MED("%s %d %s", p_name, sg_singleton_fd, sgp_singleton_name);

    sg_singleton_fd  = mknod(sgp_singleton_name, S_IFREG | 0666, 0);
    if (sg_singleton_fd  < 0)
    {
        AIS_LOG_SRV_HIGH("Fail to create node errno:%d\n", errno);
        rc = -2;
    }
#endif

EXIT_FLAG:

    AIS_LOG_SRV(rc == 0 ? AIS_LOG_LVL_HIGH : AIS_LOG_LVL_ERROR,
                "X %p %d", p_name, rc);

    return rc;
}

/**
 * releases singleton semaphore.
 *
 * @return 0: success, others: fail
 *
 */
static int ais_server_singleton_release(void)
{
    int rc = 0;

    AIS_LOG_SRV_HIGH("E");

#if defined(__QNXNTO__) || defined(__INTEGRITY)
    if (sgsp_singleton_sem != NULL)
    {
        AIS_LOG_SRV_MED("%p %s", sgsp_singleton_sem, sgp_singleton_name);

        sem_close(sgsp_singleton_sem);
        sgsp_singleton_sem = NULL;

        sem_unlink(sgp_singleton_name);
        sgp_singleton_name[0] = '\0';
    }
#elif defined(CAMERA_UNITTEST) || defined(__AGL__)
    if (sg_singleton_fd != -1)
    {
        AIS_LOG_SRV_MED("%d %s", sg_singleton_fd, sgp_singleton_name);

        close(sg_singleton_fd);
        sg_singleton_fd = -1;

        unlink(sgp_singleton_name);
        sgp_singleton_name[0] = '\0';
    }
#elif defined(__ANDROID__)
        AIS_LOG_SRV_ERR("SINGLETON_NODE_DELETE = %d", sg_singleton_fd);
        sg_singleton_fd = -1;
        if(unlink(sgp_singleton_name) != 0){
                AIS_LOG_SRV_ERR("UNLINK FAILED = %d", sg_singleton_fd);
        }
        sgp_singleton_name[0] = '\0';
#endif

    AIS_LOG_SRV_HIGH("X %d", rc);

    return rc;
}

/**
 * checks if the version is compatible
 *
 * @param version the checking version
 * @return 0: success, others: failed
 *
 */
static int ais_server_check_version(unsigned int version)
{
    int rc = -1;

    if ((version >= AIS_VERSION_MINIMUM_COMPATIBLE) && (version <= AIS_VERSION))
    {
        rc = 0;
    }

    return rc;
}

/**
 * initializes one server context
 *
 * @param p points to a server context
 * @return 0: success, others: failed
 *
 */
static void ais_server_init_client_ctxt(s_ais_client_ctxt *p, uint32 idx)
{
    int i;

    AIS_LOG_SRV_API("E %p", p);

    memset(p, 0, sizeof(s_ais_client_ctxt));

    p->magic = AIS_SERVER_CLIENT_CTXT_MAGIC;
    p->idx = idx;

    for (i = 0; i < AIS_CONN_CMD_MAX_NUM; i++)
    {
        AIS_CONN_API(ais_conn_init)(p->cmd_conn + i);

        p->cmd_thread_param[i].p = p;
        p->cmd_thread_param[i].idx = i;

        p->cmd_thread_id[i] = 0;
        p->cmd_thread_abort[i] = FALSE;
    }

    AIS_CONN_API(ais_conn_init)(&p->event_conn);

    memset(&p->event_queue, 0, sizeof(p->event_queue));

    p->event_thread_id = 0;
    p->event_thread_abort = FALSE;

    p->event_cb = NULL;
    p->priv_data = NULL;

    p->signal_attempts = 0;
    p->signal_bit = FALSE;
    p->health_active = FALSE;
    p->ctxt_abort = FALSE;

    pthread_mutex_init(&p->event_queue_lock, NULL);

    AIS_LOG_SRV_API("X %p", p);

    return;
}

/**
 * Finds unused client context from the global resource and returns it
 *
 * @return pointer to client context (NULL if none are available)
 *
 */
static s_ais_client_ctxt* ais_server_alloc_client_ctxt(void)
{
    s_ais_client_ctxt* p = NULL;
    int i;

    AIS_LOG_SRV_API("E");

    pthread_mutex_lock(&sgs_mutex);

    for (i = 0; i < AIS_MAX_USR_CONTEXTS; i++)
    {
        /* Find unused client ctxt */
        if (sgs_ais_client_ctxt[i].qcarcam_hndl == NULL)
        {
            p = &sgs_ais_client_ctxt[i];

            ais_server_init_client_ctxt(p, i);

            p->qcarcam_hndl = AIS_CONTEXT_IN_USE;

            break;
        }
    }

    pthread_mutex_unlock(&sgs_mutex);

    AIS_LOG_SRV(p == NULL ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_WARN,
                "X %d", (p == NULL) ? -1 : p->idx);

    return p;
}

/**
 * frees one server context to the global resource
 * @parma client ctxt to free
 * @return 0: success, others: failed
 */
static int ais_server_free_client_ctxt(s_ais_client_ctxt *p)
{
    int rc = 0;

    AIS_LOG_SRV_API("E %d", p->idx);

    pthread_mutex_lock(&sgs_mutex);

    pthread_mutex_destroy(&p->event_queue_lock);

    p->qcarcam_hndl = NULL;

    pthread_mutex_unlock(&sgs_mutex);

    AIS_LOG_SRV(rc == 0 ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %d %d", p->idx, rc);

    return rc;
}

/**
 * finds the index of one server context with specific handle
 * @parma handle handle opened
 * @return >=0: index of valid context, <0: failed
 */
static int ais_server_find(qcarcam_hndl_t handle)
{
    int rc = -1;
    int i;

    AIS_LOG_SRV_DBG("E %p", handle);

    for (i = 0; i < AIS_MAX_USR_CONTEXTS; i++)
    {
        if (sgs_ais_client_ctxt[i].qcarcam_hndl == handle)
        {
            rc = i;
            break;
        }
    }

    AIS_LOG_SRV(rc >= 0 ? AIS_LOG_LVL_DBG : AIS_LOG_LVL_ERROR,
                "X %p %d", handle, rc);

    return rc;
}

/**
 * Creates all connections for one client based on exchanged connection ID
 * @param p points to a server context
 * @return 0: success, others: failed
 */
static int ais_server_create_conns_thread(void *p_arg)
{
    int rc = 0;
    s_ais_client_ctxt *p = (s_ais_client_ctxt *)p_arg;

    pthread_detach(pthread_self());

    rc = ais_server_create_main_conn(p);
    if (0 != rc)
    {
        goto EXIT_FLAG;
    }

    rc = ais_server_create_event_conn(p);
    if (0 != rc)
    {
        goto EXIT_FLAG;
    }

EXIT_FLAG:
    AIS_LOG_SRV(rc == 0 ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %d", p, rc);

    return rc;
}

/**
 * Creates thread used to initialize connections for one client
 * @param p points to a server context
 * @return 0: success, others: failed
 */
static int ais_server_create_conns(s_ais_client_ctxt *p)
{
    int rc = 0;
    char name[32];
    CameraThread thread_handl;

    snprintf(name, sizeof(name), "srv_cr_conn_%d", AIS_CONN_ID_CMD_MAIN(p->info.id));

    rc = CameraCreateThread(AIS_SRV_THRD_PRIO,
                            0,
                            &ais_server_create_conns_thread,
                            p,
                            AIS_CMD_THREAD_STACK_SIZE,
                            name,
                            &thread_handl);

    AIS_LOG_SRV(rc == 0 ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %d", p, rc);

    return rc;
}

/**
 * creates the main connection and thread with one client based on the exchanged connection id
 * @param p points to a server context
 * @return 0: success, others: failed
 */
static int ais_server_create_main_conn(s_ais_client_ctxt *p)
{
    int rc = 0;
    char name[32];
    s_ais_conn temp_conn = {0};

    AIS_LOG_SRV_API("E %p", p);

    rc = AIS_CONN_API(ais_conn_open)(&temp_conn, AIS_CONN_ID_CMD_MAIN(p->info.id), 1);
    if (rc != 0)
    {
        //skip close if open fails
        rc = -1;
        goto EXIT_FLAG2;
    }

    rc = AIS_CONN_API(ais_conn_accept)(&temp_conn, &p->cmd_conn[AIS_CONN_CMD_IDX_MAIN], AIS_CONN_MODE_WORK);
    if (rc != 0)
    {
        rc = -2;
        goto EXIT_FLAG;
    }

    AIS_CONN_API(ais_conn_close)(&temp_conn);


    pthread_mutex_lock(&p->event_queue_lock);
    rc = ais_event_queue_init(&p->event_queue, 2, 64, ais_event_queue_event_2_idx);
    if (rc != 0)
    {
        rc = -3;
        pthread_mutex_unlock(&p->event_queue_lock);
        goto EXIT_FLAG;
    }
    pthread_mutex_unlock(&p->event_queue_lock);

    snprintf(name, sizeof(name), "srv_cmd_%d", AIS_CONN_ID_CMD_MAIN(p->info.id));

    if (0 != (rc = CameraCreateThread(AIS_SRV_THRD_PRIO,
                                      0,
                                      &ais_server_cmd_thread,
                                      &p->cmd_thread_param[AIS_CONN_CMD_IDX_MAIN],
                                      AIS_CMD_THREAD_STACK_SIZE,
                                      name,
                                      &p->cmd_thread_id[AIS_CONN_CMD_IDX_MAIN])))
    {
        AIS_LOG_SRV_ERR("CameraCreateThread rc = %d", rc);
    }

EXIT_FLAG:
    if (rc != 0)
    {
        AIS_CONN_API(ais_conn_close)(&p->cmd_conn[AIS_CONN_CMD_IDX_MAIN]);
    }
EXIT_FLAG2:
    AIS_LOG_SRV(rc == 0 ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %d", p, rc);

    return rc;
}

/**
 * destroys the main connection, main thread would exit itself.
 * @param p points to a server context
 * @param mode indicates if it is destroyed inside
 * @return 0: success, others: failed
 */
static int ais_server_destroy_main_conn(s_ais_client_ctxt *p)
{
    int rc = 0;

    AIS_LOG_SRV_API("E %p", p);

    rc = AIS_CONN_API(ais_conn_close)(&p->cmd_conn[AIS_CONN_CMD_IDX_MAIN]);

    if (p->cmd_thread_id[AIS_CONN_CMD_IDX_MAIN] != 0)
    {
        p->cmd_thread_abort[AIS_CONN_CMD_IDX_MAIN] = TRUE;
        p->cmd_thread_id[AIS_CONN_CMD_IDX_MAIN] = 0;
    }

    AIS_LOG_SRV(rc == 0 ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %d", p, rc);

    return rc;
}

/**
 * creates all working threads with one client based on the exchanged connection range
 * @param p points to a server context
 * @return 0: success, others: failed
 */
static int ais_server_create_work_thread(s_ais_client_ctxt *p)
{
    int rc = 0;
    int i = 0;
    char name[32];

    AIS_LOG_SRV_API("E %p", p);

    for (i = AIS_CONN_CMD_IDX_WORK; i < AIS_CONN_CMD_NUM(p->info.cnt); i++)
    {
        snprintf(name, sizeof(name), "srv_cmd_%d", AIS_CONN_ID_CMD_WORK(p->info.id) + i - AIS_CONN_CMD_IDX_WORK);

        if (0 != (rc = CameraCreateThread(AIS_SRV_THRD_PRIO,
                                          0,
                                          &ais_server_cmd_thread,
                                          &p->cmd_thread_param[i],
                                          AIS_CMD_THREAD_STACK_SIZE,
                                          name,
                                          &p->cmd_thread_id[i])))
        {
            AIS_LOG_SRV_ERR("CameraCreateThread rc = %d", rc);
        }
    }

    AIS_LOG_SRV(rc == 0 ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %d", p, rc);

    return rc;
}

/**
 * destroys all the working threads
 * @param p points to a server context
 * @return 0: success, others: failed
 */
static int ais_server_destroy_work_thread(s_ais_client_ctxt *p)
{
    int i;

    AIS_LOG_SRV_API("E %p", p);

    //stop threads
    for (i = AIS_CONN_CMD_IDX_WORK; i < AIS_CONN_CMD_NUM(p->info.cnt); i++)
    {
        if (p->cmd_thread_id[i] != 0)
        {
            p->cmd_thread_abort[i] = TRUE;
        }
    }

    AIS_LOG_SRV_API("X %p", p);

    return 0;
}

/**
 * waits all the working threads exit
 * @param p points to a server context
 * @return 0: success, others: failed
 */
static int ais_server_release_work_thread(s_ais_client_ctxt *p)
{
    int i;

    AIS_LOG_SRV_API("E %p", p);

    for (i = AIS_CONN_CMD_IDX_WORK; i < AIS_CONN_CMD_NUM(p->info.cnt); i++)
    {
        /* Wait until thread is done */
        if (p->cmd_thread_id[i] != 0)
        {
            if (0 != (CameraJoinThread(p->cmd_thread_id[i], NULL)))
            {
                AIS_LOG_SRV_ERR("CameraJoinThread failed");
            }
            p->cmd_thread_id[i] = 0;
        }
    }

    AIS_LOG_SRV_API("X %p", p);

    return 0;
}

/**
 * creates all working connections and threads with one client based on the exchanged connection id
 * @param p points to a server context
 * @return 0: success, others: failed
 */
static int ais_server_create_work_conn(s_ais_client_ctxt *p)
{
    int rc = 0;
    int i;
    s_ais_conn temp_conn = {0};

    AIS_LOG_SRV_API("E %p", p);

    for (i = AIS_CONN_CMD_IDX_WORK; i < AIS_CONN_CMD_NUM(p->info.cnt); i++)
    {
        rc = AIS_CONN_API(ais_conn_open)(&temp_conn, AIS_CONN_ID_CMD_WORK(p->info.id) + i - AIS_CONN_CMD_IDX_WORK, AIS_CONN_CMD_MAX_NUM);
        if (rc != 0)
        {
            rc = -1;
            goto EXIT_FLAG;
        }

        rc = AIS_CONN_API(ais_conn_accept)(&temp_conn, p->cmd_conn + i, AIS_CONN_MODE_WORK);
        if (rc != 0)
        {
            rc = -2;
            goto EXIT_FLAG;
        }

        AIS_CONN_API(ais_conn_close)(&temp_conn);
    }

    rc = ais_server_create_work_thread(p);

EXIT_FLAG:

    if (rc != 0)
    {
        ais_server_destroy_work_thread(p);

        for (i = AIS_CONN_CMD_IDX_WORK; i < AIS_CONN_CMD_NUM(p->info.cnt); i++)
        {
            AIS_CONN_API(ais_conn_close)(p->cmd_conn + i);
        }
    }

    AIS_LOG_SRV(rc == 0 ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %d", p, rc);

    return rc;
}

/**
 * destroys all working connections and threads
 * @param p points to a server context
 * @return 0: success, others: failed
 */
static int ais_server_destroy_work_conn(s_ais_client_ctxt *p)
{
    int i = 0;
    int rc = 0;
    int temp_ret = 0;

    AIS_LOG_SRV_API("E %p", p);

    temp_ret = ais_server_destroy_work_thread(p);
    if (temp_ret != 0)
    {
        rc = temp_ret;
    }

    temp_ret = ais_server_release_work_thread(p);
    if (temp_ret != 0 && rc == 0)
    {
        rc = temp_ret;
    }

    //close connections
    for (i = AIS_CONN_CMD_IDX_WORK; i < AIS_CONN_CMD_NUM(p->info.cnt); i++)
    {
        temp_ret = AIS_CONN_API(ais_conn_close)(p->cmd_conn + i);
        if (temp_ret != 0 && rc == 0)
        {
            rc = temp_ret;
        }
    }


    AIS_LOG_SRV(rc == 0 ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %d", p, rc);

    return rc;
}

/**
 * enqueues events from health monitor.
 * @param index of valid context
 * @param event_id generated event id
 * @param p_payload points to the payload of health message
 * @return none
 */
static void ais_server_health_event_cb(s_ais_client_ctxt *p, qcarcam_event_t event_id,
                                                qcarcam_event_payload_t *p_payload)
{
    s_ais_event event;

    memset(&event, 0, sizeof(event));
    event.event_id = event_id;
    if (p_payload != NULL)
    {
        event.payload = *p_payload;
    }

    AIS_LOG_SRV_DBG("health msg cb event cnt: %d", event.event_cnt);

    ais_event_queue_enqueue(&p->event_queue, &event);
}

/**
 * enqueues events from engine. this is callback, should be registered to engine.
 * @param handle opened handle
 * @param event_id generated event id
 * @param p_payload points to the payload of the specific event
 * @return none
 */
static void ais_server_event_cb(qcarcam_hndl_t handle, qcarcam_event_t event_id,
                                        qcarcam_event_payload_t *p_payload)
{
    int idx = ais_server_find(handle);

    if (idx >= 0)
    {
        s_ais_event event;

        memset(&event, 0, sizeof(event));
        event.event_id = event_id;
        if (p_payload != NULL)
        {
            event.payload = *p_payload;

            if (QCARCAM_EVENT_FRAME_READY == event_id && event.payload.frame_info.flags != 0)
            {
                uint64 ptimestamp = 0;
                CameraGetTime(&ptimestamp);
                AIS_LOG_SRV_HIGH("LATENCY| qcarcam_Hndl %lu frame_cnt %d cur_timestamp %llu latency_from_buffdone %llu us",
                    handle,
                    event.payload.frame_info.seq_no[0],
                    ptimestamp,
                    (ptimestamp - event.payload.frame_info.timestamp_system) / 1000);
                event.payload.frame_info.sof_qtimestamp[0] = ptimestamp;
            }
        }

        AIS_LOG_SRV_DBG("callback event id: %d cnt: %d", event.event_id, event.event_cnt);

        ais_event_queue_enqueue(&sgs_ais_client_ctxt[idx].event_queue, &event);
    }
}

/**
 * dequeues events and sends to a client.
 * this is thread function which should be run in one thread
 * @param p_arg points to a server context.
 * @return NULL
 */
static int ais_server_event_send_thread(void *p_arg)
{
    s_ais_client_ctxt *p = (s_ais_client_ctxt *)p_arg;
    int rc = 0;

    AIS_LOG_SRV_API("E %p", p_arg);

    while (!sg_abort && !p->event_thread_abort)
    {
        s_ais_event event;

        rc = ais_event_queue_dequeue(&p->event_queue, &event, 100);
        if (rc != 0)
        {
            continue;
        }

        AIS_LOG_SRV_DBG("sent event id: %d cnt: %d", event.event_id, event.event_cnt);

        rc = AIS_CONN_API(ais_conn_send)(&p->event_conn, &event, sizeof(event));
    }

    AIS_LOG_SRV_API("X %p", p_arg);

    return 0;
}

/**
 * creates all event threads with one client based on the exchanged connection id
 * @param p points to a server context
 * @return 0: success, others: failed
 */
static int ais_server_create_event_thread(s_ais_client_ctxt *p)
{
    int rc = 0;
    char name[32];

    AIS_LOG_SRV_API("E %p", p);

    snprintf(name, sizeof(name), "srv_evnt_snd_%d", AIS_CONN_ID_EVENT(p->info.id));

    if (0 != (rc = CameraCreateThread(AIS_SRV_THRD_PRIO,
                                      0,
                                      &ais_server_event_send_thread,
                                      p,
                                      0,
                                      name,
                                      &p->event_thread_id)))
    {
        AIS_LOG_SRV_ERR("CameraCreateThread rc = %d", rc);
    }

    AIS_LOG_SRV(rc == 0 ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %d", p, rc);

    return rc;
}

/**
 * destroys all the events threads
 * @param p points to a server context
 * @return 0: success, others: failed
 */
static int ais_server_destroy_event_thread(s_ais_client_ctxt *p)
{
    AIS_LOG_SRV_API("E %p", p);

    //destroys event thread
    if (p->event_thread_id != 0)
    {
        p->event_thread_abort = TRUE;
    }

    AIS_LOG_SRV_API("X %p", p);

    return 0;
}

/**
 * waits all the event threads exit
 * @param p points to a server context
 * @return 0: success, others: failed
 */
static int ais_server_release_event_thread(s_ais_client_ctxt *p)
{
    AIS_LOG_SRV_API("E %p", p);

    //waits event thread exit
    if (p->event_thread_id != 0)
    {
        if (0 != ais_event_queue_signal(&p->event_queue))
        {
            AIS_LOG_SRV_ERR("ais_event_queue_signal failed");
        }

        if (0 != (CameraJoinThread(p->event_thread_id, NULL)))
        {
            AIS_LOG_SRV_ERR("CameraJoinThread failed");
        }
        p->event_thread_id = 0;
    }

    AIS_LOG_SRV_API("X %p", p);

    return 0;
}

/**
 * creates the event connection and thread with one client based on the exchanged connection id
 * @param p points to a server context
 * @return 0: success, others: failed
 */
static int ais_server_create_event_conn(s_ais_client_ctxt *p)
{
    int rc = 0;
    s_ais_conn temp_conn = {0};

    AIS_LOG_SRV_API("E %p", p);

    rc = AIS_CONN_API(ais_conn_open)(&temp_conn, AIS_CONN_ID_EVENT(p->info.id), 1);
    if (rc != 0)
    {
        //skip close if open fails
        rc = -1;
        goto EXIT_FLAG2;
    }

    rc = AIS_CONN_API(ais_conn_accept)(&temp_conn, &p->event_conn, AIS_CONN_MODE_WORK);
    AIS_CONN_API(ais_conn_close)(&temp_conn);
    if (rc != 0)
    {
        rc = -2;
        goto EXIT_FLAG;
    }

    pthread_mutex_lock(&sgs_mutex);
    if (p->ctxt_abort == TRUE)
    {
        pthread_mutex_unlock(&sgs_mutex);
        goto EXIT_FLAG;
    }
    else
    {
        pthread_mutex_lock(&p->event_queue_lock);
        rc = ais_server_create_event_thread(p);
        if (rc != 0)
        {
            ais_event_queue_deinit(&p->event_queue);
        }
        pthread_mutex_unlock(&p->event_queue_lock);
        pthread_mutex_unlock(&sgs_mutex);
    }

EXIT_FLAG:

    if (rc != 0)
    {
        AIS_CONN_API(ais_conn_close)(&p->event_conn);
    }

EXIT_FLAG2:
    AIS_LOG_SRV(rc == 0 ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %d", p, rc);

    return rc;
}

/**
 * destroys the event connection and thread
 * @param p points to a server context
 * @return 0: success, others: failed
 */
static int ais_server_destroy_event_conn(s_ais_client_ctxt *p)
{
    int rc = 0;
    int temp_ret = 0;

    AIS_LOG_SRV_API("E %p", p);

    pthread_mutex_lock(&p->event_queue_lock);
    temp_ret = ais_server_destroy_event_thread(p);
    if (temp_ret != 0)
    {
        rc = temp_ret;
    }

    temp_ret = ais_server_release_event_thread(p);
    if (temp_ret != 0 && rc == 0)
    {
        rc = temp_ret;
    }

    ais_event_queue_deinit(&p->event_queue);
    pthread_mutex_unlock(&p->event_queue_lock);

    temp_ret = AIS_CONN_API(ais_conn_close)(&p->event_conn);
    if (temp_ret != 0 && rc == 0)
    {
        rc = temp_ret;
    }


    AIS_LOG_SRV(rc == 0 ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %d", p, rc);

    return rc;
}

#ifndef AIS_DISABLE_HEALTH
/**
 * checks contexts are active and destroys them otherwise
 * @param void p_arg: no params
 * @return void
 */
static int ais_server_health_thread(void *p_arg)
{
    int i = 0, j = 0;
    int sleep_usec = HEALTH_CHECK_DELAY_MSEC * 1000; // Convert msec to usec

    AIS_LOG_SRV_HIGH("E");

    pthread_detach(pthread_self());

    while (!sg_abort)
    {
        /* Check if active contexts have been signaled */
        for (i = 0; i < AIS_MAX_USR_CONTEXTS; i++)
        {
            s_ais_client_ctxt* p_client = &sgs_ais_client_ctxt[i];
            volatile bool abort_ctxt = FALSE;

            if (p_client->health_active)
            {
                /* Send health event to client */
                ais_server_health_event_cb(&sgs_ais_client_ctxt[i],
                                    (qcarcam_event_t)EVENT_HEALTH_MSG,
                                    NULL);

                /* Check if signal was set */
                if (p_client->signal_bit == TRUE)
                {
                    p_client->signal_bit = FALSE;
                    p_client->signal_attempts = 0;
                }
                else
                {
                    /* If signal was read as false more than MAX_HEALTH_SIGNAL_ATTEMPTS */
                    /* abort threads from such context */
                    if (p_client->signal_attempts > MAX_HEALTH_SIGNAL_ATTEMPTS
                        && !p_client->ctxt_abort)
                    {
                        abort_ctxt = TRUE;

                        AIS_LOG_SRV_WARN("Health signal timeout. Closing context[%d] from %d:%d",
                            i, p_client->info.pid, p_client->info.gid);
                    }
                    p_client->signal_attempts++;
                }
            }

            if (abort_ctxt)
            {
                /* Check for contexts from matching process */
                for (j = 0; j < AIS_MAX_USR_CONTEXTS; j++)
                {
                    s_ais_client_ctxt* p_match = &sgs_ais_client_ctxt[j];

                    if (i != j &&
                        !p_match->ctxt_abort &&
                        p_match->info.gid == p_client->info.gid &&
                        p_match->info.pid == p_client->info.pid &&
                        p_match->qcarcam_hndl &&
                        p_match->qcarcam_hndl != AIS_CONTEXT_IN_USE)
                    {
                        AIS_LOG_SRV_WARN("Health signal timeout. Closing context %d", j);
                        ais_server_destroy_client_ctxt(p_match);
                    }
                }
                /* destroy client monitor at last */
                ais_server_destroy_client_ctxt(p_client);
            }
        }

        if (!sg_abort)
        {
            usleep(sleep_usec);
        }
    }

    AIS_LOG_SRV_HIGH("X %d");
    return 0;
}

static int ais_server_create_health_thread(void)
{
    int rc = 0;
    char name[32];
    CameraThread health_thread_id = NULL;

    AIS_LOG_SRV_API("E");

    snprintf(name, sizeof(name), "srv_health");

    if (0 != (rc = CameraCreateThread(AIS_SRV_THRD_PRIO,
                                      0,
                                      &ais_server_health_thread,
                                      NULL,
                                      0,
                                      name,
                                      &health_thread_id)))
    {
        AIS_LOG_SRV_ERR("CameraCreateThread rc = %d", rc);
    }

    AIS_LOG_SRV(rc == 0 ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %d", rc);

    return rc;
}
#endif //AIS_DISABLE_HEALTH

/**
 * exchanges the infos of connections
 * @param p_conn points to a connection
 * @param p_info points to a connection info which would be sent back to a client
 * @return 0: success, others: failed
 */
static int ais_server_exchange(s_ais_client_ctxt *p, s_ais_conn *p_conn, s_ais_conn_info *p_info)
{
    int rc = 0;
    int version_check = 0;
    s_ais_conn_info info;
    unsigned int size;

    AIS_LOG_SRV_API("E %p %p", p_conn, p_info);

    size = sizeof(s_ais_conn_info);
    rc = AIS_CONN_API(ais_conn_recv)(p_conn, &info, &size, AIS_CONN_RECV_TIMEOUT);
    if (rc != 0 || size == 0)
    {
        rc = -1;
        goto EXIT_FLAG;
    }

    if (info.cnt > p_info->cnt)
    {
        info.cnt = p_info->cnt;
    }

    info.id = p_info->id;
    info.version = AIS_VERSION;
    info.result = CAMERA_SUCCESS;

    if (info.app_version != AIS_VERSION)
    {
        AIS_LOG_SRV_WARN("API version mismatch between Client and Server");
    }
    version_check = ais_server_check_version(info.app_version);
    if (version_check != 0)
    {
        AIS_LOG_SRV_ERR("API version of Client is unsupported by Server");
        info.result = CAMERA_EVERSIONNOTSUPPORT;
    }
    else
    {
        AIS_LOG_SRV_WARN("API version of Client is compatible with version of Server");
    }

    version_check = ais_server_check_version(info.version);
    if (version_check != 0)
    {
        AIS_LOG_SRV_ERR("API version of APP is unsupported by Server");
        info.result = CAMERA_EVERSIONNOTSUPPORT;
    }
    else
    {
        AIS_LOG_SRV_WARN("API version of APP is compatible with version of Server");
    }

    if (NULL != p &&
        CAMERA_SUCCESS == info.result)
    {
        p_info->cnt = info.cnt;
        p_info->pid = info.pid;
        p_info->gid = info.gid;
        p_info->flags = info.flags;
        p->info = *p_info;

        rc = ais_server_create_conns(p);
        if (0 != rc)
        {
            info.result = CAMERA_EFAILED;
            goto SEND_REPLY;
        }

        p->health_active = (p_info->flags & AIS_CONN_FLAG_HEALTH_CONN) ? TRUE : FALSE;
    }
    else
    {
        AIS_LOG_SRV_ERR("API version of Client is %u, API version of Server is %u and "
                "Min compatible API version is %u", info.app_version, AIS_VERSION, AIS_VERSION_MINIMUM_COMPATIBLE);
        info.result = CAMERA_ENOMORE;
    }

SEND_REPLY:
    rc = AIS_CONN_API(ais_conn_send)(p_conn, &info, sizeof(s_ais_conn_info));
    if (rc != 0)
    {
        rc = -2;
        goto EXIT_FLAG;
    }

    if (info.result != CAMERA_SUCCESS)
    {
        rc = -3;
        goto EXIT_FLAG;
    }

EXIT_FLAG:

    AIS_LOG_SRV(rc == 0 ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %p %d", p_conn, p_info, rc);

    return rc;
}

/**
 * creates a server context and setup connection with a client
 * it allocates a server context, assigns connection id and range to a client
 * @param p_conn points to a connection
 * @return 0: success, others: failed
 */
static int ais_server_create_client_ctxt(s_ais_conn *p_conn)
{
    int rc = 0;
    s_ais_client_ctxt *p = NULL;
    s_ais_conn_info info;

    AIS_LOG_SRV_HIGH("E %p", p_conn);

    p = ais_server_alloc_client_ctxt();
    if (p == NULL)
    {
        info.id = -1;
        info.cnt = 0;
    }
    else
    {
        //1 is used for reserving the original one when the first client is connected
        info.id = (p->idx + 1) * AIS_CONN_MAX_NUM;
        info.cnt = AIS_CONN_MAX_NUM;
        AIS_LOG_SRV_MED("%d %d %d", info.id, info.cnt, p->idx);
    }


    rc = ais_server_exchange(p, p_conn, &info);
    if (rc != 0)
    {
        rc = -1;
    }
    else if (p == NULL)
    {
        rc = -2;
    }

    if (rc != 0 && p)
    {
        ais_server_free_client_ctxt(p);
    }

    AIS_LOG_SRV((rc == 0) ? AIS_LOG_LVL_HIGH : AIS_LOG_LVL_ERROR,
                "X %p %d", p_conn, rc);

    return rc;
}

/**
 * destroys a server context and frees all resource
 * @param idx index of a server context
 * @return 0: success, others: failed
 */
static int ais_server_destroy_client_ctxt(s_ais_client_ctxt *p)
{
    int rc = 0;
    int temp_ret = 0;
    int is_aborted = 0;

    AIS_LOG_SRV_HIGH("E %d", p->idx);

    pthread_mutex_lock(&sgs_mutex);
    if (p->ctxt_abort == TRUE)
    {
        is_aborted = 1;
    }
    else
    {
        is_aborted = 0;
        p->ctxt_abort = TRUE;
    }
    pthread_mutex_unlock(&sgs_mutex);

    if (is_aborted == 0)
    {
        //event callback has stopped because of close operation
        //so no race condition is existing for closing event connection
        //if it is not closed by close operation, disable it here,
        //which makes event callback non-functional,
        //and make sure the last event callback is finished.
        if (p->qcarcam_hndl != NULL && p->qcarcam_hndl != AIS_CONTEXT_IN_USE)
        {
            CameraResult ret = ais_close(p->qcarcam_hndl);
            p->qcarcam_hndl = AIS_CONTEXT_IN_USE;

            if (ret != CAMERA_SUCCESS)
            {
                usleep(100 * 10000);
            }
        }

        temp_ret = ais_server_destroy_event_conn(p);
        if (temp_ret != 0)
        {
            rc = temp_ret;
        }

        p->health_active = FALSE;
        p->signal_attempts = 0;
        p->signal_bit = FALSE;

        temp_ret = ais_server_destroy_work_conn(p);
        if(temp_ret != 0 && rc == 0)
        {
            rc = temp_ret;
        }

        temp_ret = ais_server_destroy_main_conn(p);
        if(temp_ret != 0 && rc == 0)
        {
            rc = temp_ret;
        }

        temp_ret = ais_server_free_client_ctxt(p);
        if(temp_ret != 0 && rc == 0)
        {
            rc = temp_ret;
        }
    }

    AIS_LOG_SRV(rc == 0 ? AIS_LOG_LVL_HIGH : AIS_LOG_LVL_ERROR,
                "X %d %d", p->idx, rc);

    return rc;
}

/**
 * processes query_inputs command
 * @param p points to a server context
 * @param p_param points to the coresponding command parameter
 * @return CameraResult
 */
static CameraResult ais_server_query_inputs(s_ais_client_ctxt *p, s_ais_cmd_query_inputs *p_param)
{
    CameraResult rc = CAMERA_SUCCESS;

    AIS_LOG_SRV_API("E %p %p", p, p_param);

    AIS_LOG_SRV_MED("0x%x %d %d %d 0x%x",
                                        p_param->cmd_id,
                                        p_param->result,
                                        p_param->is_p_inputs_set,
                                        p_param->size);

    if (p_param->is_p_inputs_set)
    {
        if (p_param->size > MAX_NUM_AIS_INPUTS)
        {
            p_param->size = MAX_NUM_AIS_INPUTS;
        }
    }

    if (rc != CAMERA_SUCCESS)
    {
        goto EXIT_FLAG;
    }

    rc = ais_query_inputs(p_param->is_p_inputs_set ? p_param->inputs : NULL,
                            p_param->size, &p_param->ret_size);

EXIT_FLAG:

    p_param->result = rc;
    p_param->flags = AIS_VERSION;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %p %d", p, p_param, rc);

    return rc;
}

/**
 * processes query_inputs command
 * @param p points to a server context
 * @param p_param points to the coresponding command parameter
 * @return CameraResult
 */
static CameraResult ais_server_query_inputs_v2(s_ais_client_ctxt *p, s_ais_cmd_query_inputs_v2 *p_param)
{
    CameraResult rc = CAMERA_SUCCESS;

    AIS_LOG_SRV_API("E %p %p", p, p_param);

    AIS_LOG_SRV_MED("0x%x %d %d %d 0x%x",
                                        p_param->cmd_id,
                                        p_param->result,
                                        p_param->is_p_inputs_set,
                                        p_param->size);

    if (p_param->is_p_inputs_set)
    {
        if (p_param->size > MAX_NUM_AIS_INPUTS)
        {
            p_param->size = MAX_NUM_AIS_INPUTS;
        }
    }

    if (rc != CAMERA_SUCCESS)
    {
        goto EXIT_FLAG;
    }

    rc = ais_query_inputs_v2(p_param->is_p_inputs_set ? p_param->inputs : NULL,
                            p_param->size, &p_param->ret_size);

EXIT_FLAG:

    p_param->result = rc;
    p_param->flags = AIS_VERSION;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %p %d", p, p_param, rc);

    return rc;
}

/**
 * processes query_diagnostics command
 * @param p points to a server context
 * @param p_param points to the coresponding command parameter
 * @return CameraResult
 */

static CameraResult ais_server_query_diagnostics(s_ais_client_ctxt *p, int idx, s_ais_cmd_query_diagnostics *p_param)
{
    CameraResult rc = CAMERA_SUCCESS;

    AIS_LOG_SRV_API("E %p %p", p, p_param);
    void *aisDiagInfo = calloc(1, p_param->diag_size);
    unsigned int diag_length = 0;
    unsigned int size = sizeof(diag_length);

    if (!aisDiagInfo)
    {
        AIS_LOG_SRV_ERR("Memory is not allocated");
        //send incorrect size
        diag_length = 1;
        (void)AIS_CONN_API(ais_conn_send)(&p->cmd_conn[idx], &diag_length, size);
        return CAMERA_EFAILED;
    }

    memset(aisDiagInfo, 0x0, p_param->diag_size);
    rc = ais_query_diagnostics(aisDiagInfo, p_param->diag_size);

    if (rc == CAMERA_SUCCESS)
    {
        diag_length = p_param->diag_size;
        rc = AIS_CONN_API(ais_conn_send)(&p->cmd_conn[idx], &diag_length, size);
        if (rc == CAMERA_SUCCESS)
        {
            rc = AIS_CONN_API(ais_conn_send)(&p->cmd_conn[idx], aisDiagInfo, p_param->diag_size);
            if (rc != CAMERA_SUCCESS)
            {
                AIS_LOG_SRV_ERR("send diag failed");
                rc = CAMERA_EFAILED;
                goto EXIT_FLAG;
            }
        }
    }
    else
    {
        //send incorrect size
        AIS_LOG_SRV_ERR("ais_query_diagnostics, send incorrect size");
        diag_length = 1;
        (void)AIS_CONN_API(ais_conn_send)(&p->cmd_conn[idx], &diag_length, size);
    }

EXIT_FLAG:
    free(aisDiagInfo);
    p_param->result = rc;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %p %d", p, p_param, rc);

    return rc;
}

/**
 * processes open command
 * @param p points to a server context
 * @param p_param points to the coresponding command parameter
 * @return CameraResult
 */
static CameraResult ais_server_open(s_ais_client_ctxt *p, s_ais_cmd_open *p_param)
{
    CameraResult rc;
    qcarcam_hndl_t handle;

    AIS_LOG_SRV_API("E %p %p", p, p_param);

    AIS_LOG_SRV_MED("0x%x %d %d",
                                        p_param->cmd_id,
                                        p_param->result,
                                        p_param->input_desc);

    handle = ais_open(p_param->input_desc);
    if (handle != 0)
    {
        p->qcarcam_hndl = handle;

        p_param->handle = handle;
        rc = CAMERA_SUCCESS;
    }
    else
    {
        rc = CAMERA_EFAILED;
    }

    p_param->result = rc;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %p %d", p, p_param, rc);

    return rc;
}

/**
 * processes close command
 * @param p points to a server context
 * @param p_param points to the coresponding command parameter
 * @return CameraResult
 */
static CameraResult ais_server_close(s_ais_client_ctxt *p, s_ais_cmd_close *p_param)
{
    CameraResult rc;

    AIS_LOG_SRV_API("E %p %p", p, p_param);

    AIS_LOG_SRV_MED("0x%x %d %p",
                                        p_param->cmd_id,
                                        p_param->result,
                                        p_param->handle);

    rc = ais_close(p_param->handle);
    p->qcarcam_hndl = AIS_CONTEXT_IN_USE;

    p_param->result = rc;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %p %d", p, p_param, rc);

    return rc;
}

/**
 * processes g_param command
 * @param p points to a server context
 * @param p_param points to the coresponding command parameter
 * @return CameraResult
 */
static CameraResult ais_server_g_param(s_ais_client_ctxt *p, int idx, s_ais_cmd_g_param *p_param)
{
    CameraResult rc;
    unsigned int p_value[QCARCAM_MAX_GAMMA_TABLE] = {};

    AIS_LOG_SRV_API("E %p %p", p, p_param);

    AIS_LOG_SRV_MED("0x%x %d %p %d",
                                        p_param->cmd_id,
                                        p_param->result,
                                        p_param->handle,
                                        p_param->param);

    if (p_param->param == QCARCAM_PARAM_GAMMA &&
        p_param->value.gamma_config.config_type == QCARCAM_GAMMA_KNEEPOINTS)
    {
        p_param->value.gamma_config.gamma.table.p_value = p_value;
    }

    rc = ais_g_param(p_param->handle, p_param->param, &p_param->value);

    if (p_param->param == QCARCAM_PARAM_GAMMA &&
        p_param->value.gamma_config.config_type == QCARCAM_GAMMA_KNEEPOINTS)
    {
        // send length firstly
        unsigned int length = 0;
        unsigned int size_length = sizeof(length);
        if (rc == CAMERA_SUCCESS)
        {
            length = p_param->value.gamma_config.gamma.table.length;
            rc = AIS_CONN_API(ais_conn_send)(&p->cmd_conn[idx], &length, size_length);
            if (rc == CAMERA_SUCCESS)
            {
                length = p_param->value.gamma_config.gamma.table.length;
                unsigned int size_table = sizeof(unsigned int) * length;
                rc = AIS_CONN_API(ais_conn_send)(&p->cmd_conn[idx], p_value, size_table);
            }
        }
        else
        {
            length = 1;
            (void)AIS_CONN_API(ais_conn_send)(&p->cmd_conn[idx], &length, size_length);
        }
    }

    p_param->result = rc;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %p %d", p, p_param, rc);

    return rc;
}

/**
 * processes s_param command
 * @param p points to a server context
 * @param p_param points to the coresponding command parameter
 * @return CameraResult
 */
static CameraResult ais_server_s_param(s_ais_client_ctxt *p, int idx, s_ais_cmd_s_param *p_param)
{
    CameraResult rc = CAMERA_SUCCESS;

    AIS_LOG_SRV_API("E %p %p", p, p_param);

    AIS_LOG_SRV_MED("0x%x %d %p %d",
                                        p_param->cmd_id,
                                        p_param->result,
                                        p_param->handle,
                                        p_param->param);

    if (p_param->param == QCARCAM_PARAM_EVENT_CB)
    {
        p_param->value.ptr_value = ais_server_event_cb;
    }


    if (p_param->param == QCARCAM_PARAM_GAMMA &&
        p_param->value.gamma_config.config_type == QCARCAM_GAMMA_KNEEPOINTS)
    {
        unsigned int length = p_param->value.gamma_config.gamma.table.length;
        unsigned int size = sizeof(unsigned int);
        unsigned int p_value[QCARCAM_MAX_GAMMA_TABLE];

        if (!length || length > QCARCAM_MAX_GAMMA_TABLE)
        {
            rc = CAMERA_EBADPARM;
        }
        if (rc != CAMERA_SUCCESS)
        {
            //send incorrect size to abort
            length++;
            rc = AIS_CONN_API(ais_conn_send)(&p->cmd_conn[idx], &length, size);
        }
        else
        {
            rc = AIS_CONN_API(ais_conn_send)(&p->cmd_conn[idx], &length, size);
            if (rc != 0)
            {
                rc = CAMERA_EFAILED;
                goto EXIT_FLAG;
            }

            p_param->value.gamma_config.gamma.table.p_value = p_value;
            size = length * sizeof(unsigned int);
            rc = AIS_CONN_API(ais_conn_recv)(&p->cmd_conn[idx], p_value, &size, AIS_CONN_RECV_TIMEOUT);
            if (rc != 0) {
                rc = CAMERA_EFAILED;
                goto EXIT_FLAG;
            }
        }

    }

    rc = ais_s_param(p_param->handle, p_param->param, &p_param->value);

EXIT_FLAG:
    p_param->result = rc;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %p %d", p, p_param, rc);

    return rc;
}

/**
 * processes s_buffers command
 * @param p points to a server context
 * @param p_param points to the corresponding command parameter
 * @return CameraResult
 */
static CameraResult ais_server_s_buffers(s_ais_client_ctxt *p, int idx, s_ais_cmd_s_buffers *p_param)
{
    CameraResult rc;
    int ret;
    int i;
    long long p_buf_arr[QCARCAM_MAX_NUM_BUFFERS] = {};
    unsigned int size_arr[QCARCAM_MAX_NUM_BUFFERS] = {};

    AIS_LOG_SRV_API("E %p %d %p", p, idx, p_param);

    if (p_param->buffers.n_buffers > QCARCAM_MAX_NUM_BUFFERS)
    {
        rc = CAMERA_EOUTOFBOUND;
        goto EXIT_FLAG;
    }

    //sending socket would merge multiple messages into one
    //if receiving socket doesn't receive them immediately.
    //use this send/recv pair to synchronize on both sides
    i = p_param->buffers.n_buffers;
    ret = AIS_CONN_API(ais_conn_send)(&p->cmd_conn[idx], &i, sizeof(int));
    if (ret != 0)
    {
        rc = CAMERA_EFAILED;
        goto EXIT_FLAG;
    }

    p_param->buffers.buffers = p_param->buffer;

    AIS_LOG_SRV_MED("%d", p_param->buffers.n_buffers);

    for (i = 0; i < p_param->buffers.n_buffers; i++)
    {
        if (p_param->buffers.buffers[i].n_planes <= QCARCAM_MAX_NUM_PLANES)
        {
            for (uint32 j = 0; j < p_param->buffers.buffers[i].n_planes; ++j)
            {
                if(p_param->buffers.buffers[i].planes[j].size > (UINT32_MAX - size_arr[i]))
                {
                    rc = CAMERA_EOUTOFBOUND;
                    goto EXIT_FLAG;
                }
                else
                {
                    size_arr[i] += p_param->buffers.buffers[i].planes[j].size;
                }
            }
        }
    }

    ret = AIS_CONN_API(ais_conn_import)(&p->cmd_conn[idx],
            p_buf_arr, &p_param->buffers.flags, size_arr,
            p_param->buffers.n_buffers, 1000);

    if(ret != 0)
    {
        AIS_LOG_SRV_ERR("ais_conn_import error ret = %d", ret);
        rc = CAMERA_EBADITEM;
        goto EXIT_FLAG;
    }

    for (i = 0; i < p_param->buffers.n_buffers; i++)
    {
        p_param->buffers.buffers[i].planes[0].p_buf = (void*)(intptr_t)p_buf_arr[i];

        for (uint32 j = 0; j < p_param->buffers.buffers[i].n_planes - 1; ++j)
        {
            p_param->buffers.buffers[i].planes[j+1].p_buf =
                (void*)((uintptr_t)p_param->buffers.buffers[i].planes[j].p_buf + p_param->buffers.buffers[i].planes[j].size);

            AIS_LOG_SRV_HIGH("Importing multi plane %p, size is 0x%x",
                p_param->buffers.buffers[i].planes[j].p_buf, p_param->buffers.buffers[i].planes[j].size);
        }
    }

    rc = ais_s_buffers(p_param->handle, &p_param->buffers);

    AIS_CONN_API(ais_conn_flush)(&p->cmd_conn[idx], (rc == CAMERA_SUCCESS) ? 0 : -1);

EXIT_FLAG:

    p_param->result = rc;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %d %p %d", p, idx, p_param, rc);

    return rc;
}

/**
 * processes s_buffers command
 * @param p points to a server context
 * @param p_param points to the corresponding command parameter
 * @return CameraResult
 */
static CameraResult ais_server_s_buffers_v2(s_ais_client_ctxt *p, int idx, s_ais_cmd_s_buffers_v2 *p_param)
{
    CameraResult rc;
    int ret;
    int i;
    long long p_buf_arr[QCARCAM_MAX_NUM_BUFFERS] = {};
    unsigned int size_arr[QCARCAM_MAX_NUM_BUFFERS] = {};

    AIS_LOG_SRV_API("E %p %d %p", p, idx, p_param);

    if (p_param->bufferlist.n_buffers > QCARCAM_MAX_NUM_BUFFERS)
    {
        rc = CAMERA_EOUTOFBOUND;
        goto EXIT_FLAG;
    }

    //sending socket would merge multiple messages into one
    //if receiving socket doesn't receive them immediately.
    //use this send/recv pair to synchronize on both sides
    i = p_param->bufferlist.n_buffers;
    ret = AIS_CONN_API(ais_conn_send)(&p->cmd_conn[idx], &i, sizeof(int));
    if (ret != 0)
    {
        rc = CAMERA_EFAILED;
        goto EXIT_FLAG;
    }

    p_param->bufferlist.buffers = p_param->buffer;

    AIS_LOG_SRV_MED("%d", p_param->bufferlist.n_buffers);

    for (i = 0; i < p_param->bufferlist.n_buffers; i++)
    {
        if (p_param->bufferlist.buffers[i].n_planes <= QCARCAM_MAX_NUM_PLANES)
        {
            for (uint32 j = 0; j < p_param->bufferlist.buffers[i].n_planes; ++j)
            {
                size_arr[i] += p_param->bufferlist.buffers[i].planes[j].size;
            }
        }
    }

    ret = AIS_CONN_API(ais_conn_import)(&p->cmd_conn[idx],
            p_buf_arr, &p_param->bufferlist.flags, size_arr,
            p_param->bufferlist.n_buffers, 1000);

    if(ret != 0)
    {
        AIS_LOG_SRV_ERR("ais_conn_import error ret = %d", ret);
        rc = CAMERA_EBADITEM;
        goto EXIT_FLAG;
    }

    for (i = 0; i < p_param->bufferlist.n_buffers; i++)
    {
        p_param->bufferlist.buffers[i].planes[0].hndl = (unsigned long long)p_buf_arr[i];
    }

    rc = ais_s_buffers_v2(p_param->handle, &p_param->bufferlist);

    AIS_CONN_API(ais_conn_flush)(&p->cmd_conn[idx], (rc == CAMERA_SUCCESS) ? 0 : -1);

EXIT_FLAG:

    p_param->result = rc;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %d %p %d", p, idx, p_param, rc);

    return rc;
}

/**
 * processes start command
 * @param p points to a server context
 * @param p_param points to the coresponding command parameter
 * @return CameraResult
 */
static CameraResult ais_server_start(s_ais_client_ctxt *p, s_ais_cmd_start *p_param)
{
    CameraResult rc;

    AIS_LOG_SRV_API("E %p %p", p, p_param);

    AIS_LOG_SRV_MED("0x%x %d %p",
                                        p_param->cmd_id,
                                        p_param->result,
                                        p_param->handle);

    rc = ais_start(p_param->handle);

    p_param->result = rc;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %p %d", p, p_param, rc);

    return rc;
}

/**
 * processes stop command
 * @param p points to a server context
 * @param p_param points to the coresponding command parameter
 * @return CameraResult
 */
static CameraResult ais_server_stop(s_ais_client_ctxt *p, s_ais_cmd_stop *p_param)
{
    CameraResult rc;

    AIS_LOG_SRV_API("E %p %p", p, p_param);

    AIS_LOG_SRV_MED("0x%x %d %p",
                                        p_param->cmd_id,
                                        p_param->result,
                                        p_param->handle);

    rc = ais_stop(p_param->handle);

    p_param->result = rc;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %p %d", p, p_param, rc);

    return rc;
}

/**
 * processes pause command
 * @param p points to a server context
 * @param p_param points to the coresponding command parameter
 * @return CameraResult
 */
static CameraResult ais_server_pause(s_ais_client_ctxt *p, s_ais_cmd_pause *p_param)
{
    CameraResult rc;

    AIS_LOG_SRV_API("E %p %p", p, p_param);

    AIS_LOG_SRV_API("0x%x %d %p",
                                        p_param->cmd_id,
                                        p_param->result,
                                        p_param->handle);

    rc = ais_pause(p_param->handle);

    p_param->result = rc;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %p %d", p, p_param, rc);

    return rc;
}

/**
 * processes resume command
 * @param p points to a server context
 * @param p_param points to the coresponding command parameter
 * @return CameraResult
 */
static CameraResult ais_server_resume(s_ais_client_ctxt *p, s_ais_cmd_resume *p_param)
{
    CameraResult rc;

    AIS_LOG_SRV_API("E %p %p", p, p_param);

    AIS_LOG_SRV_MED("0x%x %d %p",
                                        p_param->cmd_id,
                                        p_param->result,
                                        p_param->handle);

    rc = ais_resume(p_param->handle);

    p_param->result = rc;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_SRV_API : AIS_LOG_LVL_ERROR,
                "X %p %p %d", p, p_param, rc);

    return rc;
}

/**
 * processes get_frame command
 * @param p points to a server context
 * @param p_param points to the coresponding command parameter
 * @return CameraResult
 */
static CameraResult ais_server_get_frame(s_ais_client_ctxt *p, s_ais_cmd_get_frame *p_param)
{
    CameraResult rc;

    AIS_LOG_SRV_LOW("E %p %p", p, p_param);

    AIS_LOG_SRV_DBG1("0x%x %d %p %lld %d",
                                        p_param->cmd_id,
                                        p_param->result,
                                        p_param->handle,
                                        p_param->timeout,
                                        p_param->flags);

    rc = ais_get_frame(p_param->handle, &p_param->frame_info, p_param->timeout, p_param->flags);

    p_param->result = rc;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_LOW : AIS_LOG_LVL_ERROR,
                "X %p %p %d", p, p_param, rc);

    return rc;
}

/**
 * processes get_frame_v2 command
 * @param p points to a server context
 * @param p_param points to the coresponding command parameter
 * @return CameraResult
 */
static CameraResult ais_server_get_frame_v2(s_ais_client_ctxt *p, s_ais_cmd_get_frame_v2 *p_param)
{
    CameraResult rc;

    AIS_LOG_SRV_LOW("E %p %p", p, p_param);

    AIS_LOG_SRV_DBG1("0x%x %d %p %lld %d",
                                        p_param->cmd_id,
                                        p_param->result,
                                        p_param->handle,
                                        p_param->timeout,
                                        p_param->flags);

    rc = ais_get_frame_v2(p_param->handle, &p_param->frame_info_v2, p_param->timeout, p_param->flags);

    p_param->result = rc;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_LOW : AIS_LOG_LVL_ERROR,
                "X %p %p %d", p, p_param, rc);

    return rc;
}
/**
 * processes release_frame command
 * @param p points to a server context
 * @param p_param points to the coresponding command parameter
 * @return CameraResult
 */
static CameraResult ais_server_release_frame(s_ais_client_ctxt *p, s_ais_cmd_release_frame *p_param)
{
    CameraResult rc;

    AIS_LOG_SRV_LOW("E %p %p", p, p_param);

    AIS_LOG_SRV_DBG1("0x%x %d %p %d",
                                        p_param->cmd_id,
                                        p_param->result,
                                        p_param->handle,
                                        p_param->idx);

    rc = ais_release_frame(p_param->handle, p_param->idx);

    p_param->result = rc;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_LOW : AIS_LOG_LVL_ERROR,
                "X %p %p %d", p, p_param, rc);

    return rc;
}

/**
 * processes release_frame command
 * @param p points to a server context
 * @param p_param points to the coresponding command parameter
 * @return CameraResult
 */
static CameraResult ais_server_release_frame_v2(s_ais_client_ctxt *p, s_ais_cmd_release_frame_v2 *p_param)
{
    CameraResult rc;

    AIS_LOG_SRV_LOW("E %p %p", p, p_param);

    AIS_LOG_SRV_DBG1("0x%x %d %p $d, %d",
                                        p_param->cmd_id,
                                        p_param->result,
                                        p_param->handle,
                                        p_param->id,
                                        p_param->idx);

    rc = ais_release_frame_v2(p_param->handle, p_param->id, p_param->idx);

    p_param->result = rc;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_LOW : AIS_LOG_LVL_ERROR,
                "X %p %p %d", p, p_param, rc);

    return rc;
}

//TODO: not supported fully, have to change ais engine
static CameraResult ais_server_get_event(s_ais_client_ctxt *p, s_ais_cmd_get_event *p_param)
{
    CameraResult rc = CAMERA_SUCCESS;

    AIS_LOG_SRV_LOW("E %p %p", p, p_param);

    AIS_LOG_SRV_DBG1("0x%x %d %p",
                                        p_param->cmd_id,
                                        p_param->result,
                                        p_param->handle);

//    rc = ais_get_event(p_param->handle, &p_param->event);

    p_param->result = rc;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_LOW : AIS_LOG_LVL_ERROR,
                "X %p %p %d", p, p_param, rc);

    return rc;
}

/**
 * processes all commands
 * @param p points to a server context
 * @param idx indicates which connection should be used
 * @param p_param points to commands' parameter holder
 * @return 0: success, others: failed
 */
static int ais_server_process_cmd(s_ais_client_ctxt *p, unsigned int idx, u_ais_cmd *p_param)
{
    int rc = -1;
    CameraResult ret;
    s_ais_cmd_header *p_header = &p_param->header;

    AIS_LOG_SRV_LOW("E %p %d %p", p, idx, p_param);

    switch (p_header->cmd_id)
    {
    case AIS_CMD_QUERY_INPUTS:
        ret = ais_server_query_inputs(p, &p_param->query_inputs);
        break;

    case AIS_CMD_QUERY_INPUTS_V2:
        ret = ais_server_query_inputs_v2(p, &p_param->query_inputs_v2);
        break;

    case AIS_CMD_OPEN:
        ret = ais_server_open(p, &p_param->open);
        break;

    case AIS_CMD_CLOSE:
        ret = ais_server_close(p, &p_param->close);
        break;

    case AIS_CMD_G_PARAM:
        ret = ais_server_g_param(p, idx, &p_param->g_param);
        break;

    case AIS_CMD_S_PARAM:
        ret = ais_server_s_param(p, idx, &p_param->s_param);
        break;

    case AIS_CMD_S_BUFFERS:
        ret = ais_server_s_buffers(p, idx, &p_param->s_buffers);
        break;

    case AIS_CMD_S_BUFFERS_V2:
        ret = ais_server_s_buffers_v2(p, idx, &p_param->s_buffers_v2);
        break;

    case AIS_CMD_START:
        ret = ais_server_start(p, &p_param->start);
        break;

    case AIS_CMD_STOP:
        ret = ais_server_stop(p, &p_param->stop);
        break;

    case AIS_CMD_PAUSE:
        ret = ais_server_pause(p, &p_param->pause);
        break;

    case AIS_CMD_RESUME:
        ret = ais_server_resume(p, &p_param->resume);
        break;

    case AIS_CMD_GET_FRAME:
        ret = ais_server_get_frame(p, &p_param->get_frame);
        break;

    case AIS_CMD_GET_FRAME_V2:
        ret = ais_server_get_frame_v2(p, &p_param->get_frame_v2);
        break;

    case AIS_CMD_RELEASE_FRAME:
        ret = ais_server_release_frame(p, &p_param->release_frame);
        break;

    case AIS_CMD_RELEASE_FRAME_V2:
        ret = ais_server_release_frame_v2(p, &p_param->release_frame_v2);
        break;

    case AIS_CMD_GET_EVENT:
        ret = ais_server_get_event(p, &p_param->get_event);
        break;

    case AIS_CMD_HEALTH:
        p->health_active = TRUE;
        ret = CAMERA_SUCCESS;
        break;
    case AIS_CMD_QUERY_DIAGNOSTICS:
        ret = ais_server_query_diagnostics(p, idx, &p_param->query_diagnostics);
        break;
    default:
        AIS_LOG_SRV_ERR("unsupported cmd id 0x%x", p_header->cmd_id);
        ret = CAMERA_EUNSUPPORTED;
        p_header->result = ret;
        break;
    }

    rc = (ret == CAMERA_SUCCESS) ? 0 : -1;

    AIS_LOG_SRV(rc == CAMERA_SUCCESS ? AIS_LOG_LVL_LOW : AIS_LOG_LVL_ERROR,
                "X %p %d %p %d", p, idx, p_param, rc);

    return rc;
}

/**
 * processes all commands, this thread is used for main or work connections
 * the main and work threads are processing commands in the same way. the only difference is
 * the main thread would free all resources including those allocated by work and event threads
 * @param p_arg points to a server thread structure
 * @return NULL
 */
static int ais_server_cmd_thread(void *p_arg)
{
    int rc = 0;
    s_ais_server_thread_param *p_param = (s_ais_server_thread_param *)p_arg;
    s_ais_client_ctxt *p = (s_ais_client_ctxt *)p_param->p;
    unsigned int idx = p_param->idx;
    u_ais_cmd cmd;
    int cmd_id;
    unsigned int size;
    int destroy_client_ctxt_flag = 0; // this is set to 1 if the command is AIS_CMD_UNINITIALIZE or AIS_CMD_CLOSE

#if defined(__INTEGRITY)
    int32_t pmem_init_attempts = 0;

    if (idx == AIS_CONN_CMD_IDX_MAIN)
    {
        while (pmem_init())
        {
            usleep(10000);
            pmem_init_attempts++;
            if (pmem_init_attempts > 2)
            {
                AIS_LOG_SRV_ERR("pmem initialization failed");
                break;
            }
        }
    }
#endif

    AIS_LOG_SRV_HIGH("E %p %d", p, idx);

    if (idx == AIS_CONN_CMD_IDX_MAIN)
    {
        pthread_detach(pthread_self());
    }

    while (!sg_abort && !p->cmd_thread_abort[idx])
    {
        size = sizeof(cmd);
        rc = AIS_CONN_API(ais_conn_recv)(&p->cmd_conn[idx], &cmd, &size, AIS_CONN_RECV_TIMEOUT);
        if (rc > 0)
        {
            continue;
        }
        else if (rc < 0)
        {
            rc = -1;
            break;
        }

        cmd_id = cmd.header.cmd_id;

        if (cmd_id != AIS_CMD_UNINITIALIZE)
        {
            rc = ais_server_process_cmd(p, idx, &cmd);
        }

        if (cmd_id != cmd.header.cmd_id)
        {
            AIS_LOG_SRV(AIS_LOG_LVL_CRIT, "%d %d", cmd_id, cmd.header.cmd_id);
        }

        rc = AIS_CONN_API(ais_conn_send)(&p->cmd_conn[idx], &cmd, size);
        if (rc != 0)
        {
            rc = -2;
            break;
        }

        if (cmd.header.cmd_id == AIS_CMD_CLOSE ||
            cmd.header.cmd_id == AIS_CMD_UNINITIALIZE)
        {
            destroy_client_ctxt_flag = 1;
            break;
        }
        else if (cmd.header.cmd_id == AIS_CMD_OPEN)
        {
            if (cmd.header.result == CAMERA_SUCCESS)
            {
                if (0 != (rc = ais_server_create_work_conn(p)))
                {
                    rc = -3;
                    break;
                }
            }
            else
            {
                destroy_client_ctxt_flag = 1;
                rc = -4;
                break;
            }
        }

        /* Signal health monitor */
        p->signal_bit = TRUE;
    }

    if (idx == AIS_CONN_CMD_IDX_MAIN)
    {
        if (destroy_client_ctxt_flag == 1)
        {
            ais_server_destroy_client_ctxt(p);
        }
        else
        {
            AIS_LOG_SRV_HIGH("client monitor context: main thread exits with postponed destroy");
        }
    }

    AIS_LOG_SRV_HIGH("X %p %d %d", p, idx, rc);

    return 0;
}


//To get early prints
void ais_printf(const char *msg){
    FILE* fd = freopen("/dev/kmsg", "w", stdout);
    if(fd != NULL)
    {
      printf("%s", msg);
      fclose(fd);
    }
}

int get_sku_qultivate_ver()
{
   char cmd[128] = "cat /sys/devices/soc0/camera";
   char buffer[8];
   char buf[32] , buf1[32];
   int qultivate_flag = 0;
   FILE* pipe = popen(cmd,"r");
   if(!pipe)
   {
       ais_printf("popen() failed\n");
   }
   else
   {
       while(fgets(buffer,sizeof(buffer),pipe) != NULL)
       {
           snprintf(buf, 32, "soc camera value %s", buffer);
           ais_printf(buf);
       }
       pclose(pipe);
       if(!strncmp(buffer,"0x1",3))
       {
           qultivate_flag = 1;
           ais_printf("Detected Qultative HW");
       }
   }
   return qultivate_flag;
}

#if defined(__QNXNTO__)
static inline bool _secpol_in_use(void)
{
    const char *env = getenv ("SECPOL_ENABLE");
    if (env) {
        return (env[0] == '1');
    } else {
        return 0;
    }
}
#endif

#ifdef CAMERA_UNITTEST
int ais_server_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    int rc = 0;
    uint32 i;
    CameraResult ret;
    s_ais_conn conn;
    char *p_name = NULL;
    qcarcam_init_t qcarcam_init = {
        .flags = AIS_INITIALIZE_DEFER_INPUT_DETECTION |
                AIS_INITIALIZE_POWER_INPUT_GRANULAR
    };

#if defined(__QNXNTO__)
    int poll_ms = 10;
    int delay_ms = 2000;
    int c;

    while((c = getopt(argc, argv, "U:")) != -1)
    {
        switch(c) {
            case 'U':
                if(_secpol_in_use()) {
                    if(set_ids_from_arg(optarg) != EOK)
                    {
                        AIS_LOG_SRV_ERR("UID/GID setting failed");
                        return EXIT_FAILURE;
                    }
                }
                break;
            default:
                break;
        }
    }
#endif

#if defined(__INTEGRITY)
    // Set priority of main thread
    // Task needs to be linked to an object defined in corresponding INT file
    Task mainTask = TaskObjectNumber(AIS_SERV_OBJ_NUM);
    SetTaskPriority(mainTask, CameraTranslateThreadPriority(AIS_SRV_THRD_PRIO), false);
#endif

    p_name = ais_server_get_name(argv[0]);

    ais_log_init(NULL, p_name);

    //Qultivate HW, camera is not supported.
    if(get_sku_qultivate_ver())
    {
       ais_printf("Qultivate HW.. ais_server not allowed");
       rc = -1;
       ais_log_kpi(AIS_EVENT_KPI_QULTIVATE_CAMERA_NOT_ALLOWED);
       goto EXIT_FLAG_SINGLETON;
    }
#ifdef USE_HYP
    ais_log_kpi(AIS_EVENT_KPI_BE_SERVER_INIT);
#else
    ais_log_kpi(AIS_EVENT_KPI_SERVER_INIT);
#endif

    AIS_LOG_SRV_HIGH("AIS SERVER START %x", AIS_VERSION);
    rc = ais_server_singleton_acquire(p_name);
    if (rc != 0)
    {
        rc = -1;
        goto EXIT_FLAG_SINGLETON;
    }

    for (i = 0; i < AIS_MAX_USR_CONTEXTS; i++)
    {
        ais_server_init_client_ctxt(&sgs_ais_client_ctxt[i], i);
    }

    AIS_CONN_API(ais_conn_init)(&sgs_ais_server_conn);
    AIS_CONN_API(ais_conn_init)(&conn);

    ais_server_register_signal_handler();

#ifdef ENABLE_DIAGNOSTIC
    ais_diag_initialize();
#endif

    ret = ais_initialize(&qcarcam_init);
    if (ret != CAMERA_SUCCESS)
    {
        rc = -2;
        goto EXIT_FLAG_INIT;
    }
#ifndef AIS_DISABLE_HEALTH
    ret = ais_server_create_health_thread();
    if (ret != CAMERA_SUCCESS)
    {
        rc = -3;
        goto EXIT_FLAG_INIT;
    }
#endif //AIS_DISABLE_HEALTH

    /* Write boot KPI marker */
#ifdef USE_HYP
    ais_log_kpi(AIS_EVENT_KPI_BE_SERVER_READY);
#else
    ais_log_kpi(AIS_EVENT_KPI_SERVER_READY);
#endif

    AIS_LOG_SRV_HIGH("%d %d %d %d %d %d %d %d %d %d %d %d",
                    (int)sizeof(s_ais_cmd_query_inputs),
                    (int)sizeof(s_ais_cmd_open),
                    (int)sizeof(s_ais_cmd_close),
                    (int)sizeof(s_ais_cmd_g_param),
                    (int)sizeof(s_ais_cmd_s_param),
                    (int)sizeof(s_ais_cmd_s_buffers),
                    (int)sizeof(s_ais_cmd_start),
                    (int)sizeof(s_ais_cmd_stop),
                    (int)sizeof(s_ais_cmd_pause),
                    (int)sizeof(s_ais_cmd_resume),
                    (int)sizeof(s_ais_cmd_get_frame),
                    (int)sizeof(s_ais_cmd_release_frame));

    rc = AIS_CONN_API(ais_conn_open)(&sgs_ais_server_conn, 0, AIS_MAX_USR_CONTEXTS);
    if (rc != 0)
    {
        rc = -4;
        goto EXIT_FLAG;
    }

#if defined(__QNXNTO__)
    AIS_LOG_SRV_DBG("Waiting for /dev/socket");
    if (!waitfor("/dev/socket", delay_ms, poll_ms )) {
        AIS_LOG_SRV_LOW("We have /dev/socket");
    } else {
        AIS_LOG_SRV_ERR("Wait for /dev/socket failed");
        rc = -5;
        goto EXIT_FLAG;
    }
#endif

    while (!sg_abort)
    {
        rc = AIS_CONN_API(ais_conn_accept)(&sgs_ais_server_conn, &conn, AIS_CONN_MODE_LISTEN);
        if (rc != 0)
        {
            break;
        }

        ais_server_create_client_ctxt(&conn);
        AIS_CONN_API(ais_conn_close)(&conn);
    }

    AIS_CONN_API(ais_conn_close)(&sgs_ais_server_conn);

EXIT_FLAG:

    ais_uninitialize();

    AIS_LOG_SRV(AIS_LOG_LVL_ALWAYS,"finish ais_uninitialize");

#ifdef ENABLE_DIAGNOSTIC
    ais_diag_uninitialize();
#endif

EXIT_FLAG_INIT:

    ais_server_singleton_release();

EXIT_FLAG_SINGLETON:

    AIS_LOG_SRV(AIS_LOG_LVL_ALWAYS,"AIS SERVER EXIT %d", rc);

    ais_log_uninit();

    return rc;
}

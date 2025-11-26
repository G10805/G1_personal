/* ===========================================================================
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/version.h>

#include "qcarcam.h"
#include "test_util.h"
#include "test_util_rvc.h"

#if defined(__QNXNTO__)
#include <libgen.h>
#define AIS_SERVER_SOCKET_PATH "/tmp/ais_socket_0"
#endif

#define QCARCAM_TEST_DEFAULT_GET_FRAME_TIMEOUT 500000000
#define NUM_MAX_CAMERAS 1
#define BUFSIZE 32
#define SIGWAIT_TIMEOUT_MS 100
#define DRM_NODE_RETRY_CNT 10000
#define QCARCAM_OPEN_CNT 1000
/*Perf logging macros*/
#define QCARCAM_TEST_ENABLE_LOG_PERF
#ifdef QCARCAM_TEST_ENABLE_LOG_PERF
#define QCARCAM_TEST_LOG_PERF(...) \
    do { \
        uint64 tmpTime = 0; \
        qcarcam_test_get_time(&tmpTime); \
        QCARCAM_ALWZMSG("%llu %s", tmpTime-gCtxt.t_start, ##__VA_ARGS__); \
    }while(0)
#else
#define QCARCAM_TEST_LOG_PERF(...)
#endif

#define SHORT_STRING_MAX 128
char str[SHORT_STRING_MAX] = {0};

static int exitSeconds = 0;

typedef struct
{
    void* qcarcam_thread_handle;
    void* window_thread_handle;

    /*qcarcam context*/
    qcarcam_hndl_t qcarcam_context;
    qcarcam_input_desc_t qcarcam_input_id;

    test_util_buffers_param_t buffers_param;
    qcarcam_buffer_t pBuffers[INPUT_NUM_BUFFERS];

    qcarcam_buffers_t p_buffers_output;

    unsigned long long int frame_timeout;
    int use_event_callback;

    /* test util objects */
    test_util_ctxt_t *test_util_ctxt;
    test_util_window_t *qcarcam_window;
    test_util_window_t *display_window;
    test_util_window_param_t window_params;

    /* buffer management tracking */
    int buf_idx_qcarcam;
    std::list<uint32> release_buf_idx;

    int frameCnt;
    int prev_frame;
    int is_running;
    bool is_first_count;
    bool signal_lost;

    bool is_buf_dequeued[QCARCAM_MAX_NUM_BUFFERS];
    volatile bool is_display_initialized;
} qcarcam_test_input_t;

typedef struct
{
    /*abort condition*/
    pthread_mutex_t mutex_abort;
    pthread_cond_t cond_abort;

    uint64 t_start;
    int disable_display;
    int dumpFrame;
} qcarcam_test_ctxt_t;

///////////////////////////////
/// STATICS
///////////////////////////////
static qcarcam_test_input_t gRvcInput = {};

static qcarcam_test_ctxt_t gCtxt = {};

static volatile int g_aborted = 0;

static const int exceptsigs[] = {
    SIGCHLD, SIGIO, SIGURG, SIGWINCH,
    SIGSTOP, SIGTSTP, SIGTTIN, SIGTTOU, SIGCONT,
    -1,
};

#if defined(__ANDROID__)
void place_marker(char const *name)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0))
    int fd=open("/sys/kernel/boot_kpi/kpi_values", O_WRONLY);
#else
    int fd = open("/dev/kmsg", O_WRONLY);
#endif
    if (fd > 0)
    {
        char earlyapp[128] = {0};
        strlcat(earlyapp, name, sizeof(earlyapp));
        write(fd, earlyapp, strlen(earlyapp));
        close(fd);
    }
}
#endif


int qcarcam_test_get_time(uint64 *pTime)
{
    struct timespec time;
    uint64 msec;

    if (clock_gettime(CLOCK_MONOTONIC, &time) == -1)
    {
        QCARCAM_ERRORMSG("Clock gettime failed");
        return 1;
    }
    msec = ((uint64)time.tv_sec * 1000) + (((uint64)time.tv_nsec / 1000) / 1000);
    *pTime = msec;

    return 0;
}

static void initialize_qcarcam_test_ctxt(void)
{
    pthread_mutex_init(&gCtxt.mutex_abort, NULL);
    pthread_cond_init(&gCtxt.cond_abort, NULL);
    qcarcam_test_get_time(&gCtxt.t_start);
}

static void abort_test(void)
{
    QCARCAM_ERRORMSG("Aborting test");
    ais_log_kpi(AIS_EVENT_KPI_EARLY_RVC_EXIT);
    pthread_mutex_lock(&gCtxt.mutex_abort);
    g_aborted = 1;
    pthread_cond_broadcast(&gCtxt.cond_abort);
    pthread_mutex_unlock(&gCtxt.mutex_abort);
}

static int timer_thread(void *arg)
{
    pthread_detach(pthread_self());
    uint64 timer_start = 0;
    uint64 timer_test = 0;
    qcarcam_test_get_time(&timer_start);
    while(!g_aborted)
    {
        qcarcam_test_get_time(&timer_test);
        if((timer_test-timer_start) >= ((unsigned long)exitSeconds*1000))
        {
            QCARCAM_ALWZMSG("TEST Aborted after running for %d secs successfully!", exitSeconds);
            place_marker("boot_kpi:  M - RVC - TEST Aborted after running for specified secs successfully!");
            abort_test();
            break;
        }
        usleep(1000000);
    }
    return 0;
}

static int signal_thread(void *arg)
{
    sigset_t sigset;
    struct timespec timeout;
    int i;

    pthread_detach(pthread_self());

    sigfillset(&sigset);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    for (i = 0; exceptsigs[i] != -1; i++)
    {
        sigdelset(&sigset, exceptsigs[i]);
    }

    timeout.tv_sec = 0;
    timeout.tv_nsec = SIGWAIT_TIMEOUT_MS * 1000000;

    while (!g_aborted)
    {
        if ((i = sigtimedwait(&sigset, NULL, &timeout)) > 0)
        {
            abort_test();
            break;
        }
    }
    return 0;
}

/**
 * Function to retrieve frame from qcarcam and increase frame_counter
 * @param input_ctxt
 * @return 0 on success, -1 on failure
 */
static int qcarcam_test_get_frame(qcarcam_test_input_t *pInputCtxt)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;;
    qcarcam_frame_info_t frame_info = { };
    static uint64_t prev_msec = 0, cur_msec = 0, diff_time = 0;
    struct timeval current_time = { };
    float time_sec = 0.0;
    ret = qcarcam_get_frame(pInputCtxt->qcarcam_context, &frame_info, pInputCtxt->frame_timeout, 0);
    if (ret == QCARCAM_RET_TIMEOUT)
    {
        QCARCAM_ERRORMSG("qcarcam_get_frame timeout context 0x%p ret %d", pInputCtxt->qcarcam_context, ret);
        return -1;
    }

    if (QCARCAM_RET_OK != ret)
    {
        QCARCAM_ERRORMSG("Get frame context 0x%p ret %d", pInputCtxt->qcarcam_context, ret);
        return -1;
    }

    if (frame_info.idx >= pInputCtxt->buffers_param.n_buffers)
    {
        QCARCAM_ERRORMSG("Get frame context 0x%p ret invalid idx %d", pInputCtxt->qcarcam_context, frame_info.idx);
        return -1;
    }

    if (pInputCtxt->frameCnt == 0)
    {
        uint64 t_to = 0;
        qcarcam_test_get_time(&t_to);
        ais_log_kpi(AIS_EVENT_KPI_CLIENT_FIRST_FRAME);
        ais_log_kpi(AIS_EVENT_KPI_EARLY_RVC);
        printf("Success\n");
        fflush(stdout);
        QCARCAM_ALWZMSG("Get First Frame input_id %d buf_idx %i after : %lu ms, field type: %d",
            (int)pInputCtxt->qcarcam_input_id, pInputCtxt->buf_idx_qcarcam, (t_to - gCtxt.t_start), frame_info.field_type);
        gettimeofday(&current_time, NULL);
        prev_msec = (current_time.tv_sec*1000) + (current_time.tv_usec/1000);
    }

    pInputCtxt->buf_idx_qcarcam = frame_info.idx;

    QCARCAM_DBGMSG("frameCnt:%d idx:%d", pInputCtxt->frameCnt, frame_info.idx);

    pInputCtxt->is_buf_dequeued[pInputCtxt->buf_idx_qcarcam] = 1;
    pInputCtxt->frameCnt++;

    // If we get 30 FPS, we will render 450 frames for every 15 seconds
    if (0==(pInputCtxt->frameCnt % 450))
    {
        gettimeofday(&current_time, NULL);
        cur_msec = (current_time.tv_sec*1000) + (current_time.tv_usec/1000);
        diff_time = cur_msec - prev_msec;
        {
            time_sec = (float)diff_time/1000.0;
            QCARCAM_ALWZMSG("RVC FPS = %.2f", (float)450/time_sec);
        }
        // Save current time
        prev_msec = cur_msec;
    }

    return 0;
}

static int qcarcam_test_post_to_display(qcarcam_test_input_t *pInputCtxt)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    static int first_frame = 1;

    QCARCAM_DBGMSG("Post Frame before buf_idx %i", pInputCtxt->buf_idx_qcarcam);

    if (first_frame)
    {
       ais_log_kpi(AIS_EVENT_KPI_CLIENT_FIRST_FRAME_POST_START);
       first_frame = 0;
    }

    ret = test_util_post_window_buffer(pInputCtxt->test_util_ctxt, pInputCtxt->qcarcam_window, pInputCtxt->buf_idx_qcarcam,
        &pInputCtxt->release_buf_idx, QCARCAM_FIELD_UNKNOWN);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("test_util_post_window_buffer failed");
    }

    if (0 != gCtxt.dumpFrame)
    {
        if (0 == pInputCtxt->frameCnt % gCtxt.dumpFrame)
        {
            char filename[128];
            snprintf(filename, sizeof(filename), DEFAULT_DUMP_LOCATION "frame_%d_%i.raw", pInputCtxt->buf_idx_qcarcam, pInputCtxt->frameCnt);
            test_util_dump_window_buffer(pInputCtxt->test_util_ctxt, pInputCtxt->qcarcam_window, pInputCtxt->buf_idx_qcarcam, filename);
        }
    }

    return 0;
}


/**
 * Release frame back to qcarcam
 * @param input_ctxt
 * @return 0 on success, -1 on failure
 */
static int qcarcam_test_release_frame(qcarcam_test_input_t *pInputCtxt)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK ;

    if (gCtxt.disable_display || pInputCtxt->window_params.is_offscreen)
    {
        /* should release current buffer back to HW immediately if needn't display */
        ret = qcarcam_release_frame(pInputCtxt->qcarcam_context, pInputCtxt->buf_idx_qcarcam);
        if (QCARCAM_RET_OK != ret)
        {
            QCARCAM_ERRORMSG("QCarCamReleaseFrame(%d) failed %d", pInputCtxt->buf_idx_qcarcam, ret);
            return -1;
        }
        return 0;
    }

    while(!pInputCtxt->release_buf_idx.empty())
    {
        uint32 rel_idx = pInputCtxt->release_buf_idx.front();
        pInputCtxt->release_buf_idx.pop_front();

        if (rel_idx >= 0 && rel_idx < pInputCtxt->buffers_param.n_buffers)
        {
            if (pInputCtxt->is_buf_dequeued[rel_idx])
            {
                ret = qcarcam_release_frame(pInputCtxt->qcarcam_context, rel_idx);
                if (QCARCAM_RET_OK != ret)
                {
                    QCARCAM_ERRORMSG("qcarcam_release_frame(%d) failed", rel_idx);
                    return -1;
                }
                pInputCtxt->is_buf_dequeued[rel_idx] = 0;
            }
            else
            {
                QCARCAM_ERRORMSG("qcarcam_release_frame(%d) skipped since buffer not dequeued", rel_idx);
            }
        }
        else
        {
            QCARCAM_ERRORMSG("qcarcam_release_frame(%d) skipped", rel_idx);
        }
    }

    return 0;
}

/**
 * Function to handle routine of fetching, displaying, and releasing frames when one is available
 * @param input_ctxt
 * @return 0 on success, -1 on failure
 */
static int qcarcam_test_handle_new_frame(qcarcam_test_input_t *pInputCtxt)
{

    qcarcam_ret_t ret = QCARCAM_RET_OK;

    if (qcarcam_test_get_frame(pInputCtxt))
    {
        /*if we fail to get frame, we silently continue...*/
        return 0;
    }
    if (true != pInputCtxt->is_display_initialized)
    {
        QCARCAM_ERRORMSG("Display not initialized yet, skiping frame preview on display");
        place_marker("boot_kpi:  M - RVC - Display not initialized yet, skiping frame preview on display");
        /* should release current buffer back to HW immediately if needn't display */
        ret = qcarcam_release_frame(pInputCtxt->qcarcam_context, pInputCtxt->buf_idx_qcarcam);
        if (QCARCAM_RET_OK != ret)
        {
            QCARCAM_ERRORMSG("qcarcam_release_frame (%d) failed %d", pInputCtxt->buf_idx_qcarcam, ret);
            return -1;
        }
    }
    else
    {
        qcarcam_test_post_to_display(pInputCtxt);
        if (qcarcam_test_release_frame(pInputCtxt)) {
            return -1;
        }
    }
    return 0;
}

static int qcarcam_test_handle_input_signal(qcarcam_test_input_t *pInputCtxt, qcarcam_input_signal_t signal_type)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    switch (signal_type) {
    case QCARCAM_INPUT_SIGNAL_LOST:
        QCARCAM_ERRORMSG("SIGNAL LOST: input: %d", pInputCtxt->qcarcam_input_id);

        if (pInputCtxt->is_running == 0) {
            QCARCAM_ERRORMSG("Input %d already stop, break", pInputCtxt->qcarcam_input_id);
            break;
        }

        pInputCtxt->is_running = 0;
        ret = qcarcam_stop(pInputCtxt->qcarcam_context);
        if (ret == QCARCAM_RET_OK) {
            QCARCAM_ERRORMSG("Input %d qcarcam_stop successfully", pInputCtxt->qcarcam_input_id);
        }
        else
        {
            QCARCAM_ERRORMSG("Input %d qcarcam_stop failed: %d !", pInputCtxt->qcarcam_input_id, ret);
        }

        memset(&pInputCtxt->is_buf_dequeued, 0x0, sizeof(pInputCtxt->is_buf_dequeued));
        pInputCtxt->signal_lost = 1;

        break;
    case QCARCAM_INPUT_SIGNAL_VALID:
        QCARCAM_ERRORMSG("SIGNAL VALID: input: %d", pInputCtxt->qcarcam_input_id);

        if (pInputCtxt->is_running == 1) {
            QCARCAM_ERRORMSG("Input %d already running, break", pInputCtxt->qcarcam_input_id);
            break;
        }

        ret = qcarcam_s_buffers(pInputCtxt->qcarcam_context, &pInputCtxt->p_buffers_output);
        if (ret == QCARCAM_RET_OK)
            QCARCAM_ERRORMSG("Input %d qcarcam_s_buffers successfully", pInputCtxt->qcarcam_input_id);
        else
            QCARCAM_ERRORMSG("Input %d qcarcam_s_buffers failed: %d", pInputCtxt->qcarcam_input_id, ret);

        pInputCtxt->is_running = 1;
        ret = qcarcam_start(pInputCtxt->qcarcam_context);
        if (ret == QCARCAM_RET_OK) {
            QCARCAM_ERRORMSG("Input %d qcarcam_start successfully", pInputCtxt->qcarcam_input_id);
            pInputCtxt->signal_lost = 0;
        }
        else
        {
            pInputCtxt->is_running = 0;
            QCARCAM_ERRORMSG("Input %d qcarcam_start failed: %d", pInputCtxt->qcarcam_input_id, ret);
        }

        break;
    default:
        QCARCAM_ERRORMSG("Unknown Event type: %d", signal_type);
        break;
    }

    return ret;
}

/**
 * Qcarcam event callback function
 * @param hndl
 * @param event_id
 * @param p_payload
 */
static void qcarcam_test_event_cb(qcarcam_hndl_t hndl, qcarcam_event_t event_id, qcarcam_event_payload_t *p_payload)
{
    qcarcam_test_input_t *pInputCtxt = &gRvcInput;
    if (hndl != gRvcInput.qcarcam_context)
    {
        QCARCAM_ERRORMSG("event_cb called with invalid qcarcam handle %p", hndl);
        return;
    }

    if (g_aborted)
    {
        QCARCAM_ERRORMSG("Test aborted");
        return;
    }

    switch (event_id)
    {
    case QCARCAM_EVENT_FRAME_READY:
        if (pInputCtxt->is_running)
        {
            QCARCAM_DBGMSG("received QCARCAM_EVENT_FRAME_READY");
            qcarcam_test_handle_new_frame(pInputCtxt);
        }
        break;
    case QCARCAM_EVENT_INPUT_SIGNAL:
        qcarcam_test_handle_input_signal(pInputCtxt, (qcarcam_input_signal_t)p_payload->uint_payload);
        break;
    case QCARCAM_EVENT_ERROR:
        if (p_payload->uint_payload == QCARCAM_CONN_ERROR)
        {
            QCARCAM_ERRORMSG("Connetion to server lost");
            abort_test();
        }
        break;
    default:
        QCARCAM_ERRORMSG("Received unsupported event %d", event_id);
        break;
    }
}

/**
 * Thread to setup and run qcarcam based on test input context
 *
 * @note For single threaded operation, this function only sets up qcarcam context.
 *      qcarcam_start and handling of frames is not executed.
 *
 * @param arg qcarcam_test_input_t* input_ctxt
 */
static int setup_windowing_thread(void *arg)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    qcarcam_test_input_t* pInputCtxt = &gRvcInput;
    int fd = -1;
    unsigned int retry_cnt = 0;

    place_marker("boot_kpi:  M - RVC - setup_windowing_thread Before opening /dev/dri/card2");
    /**
      * We will wait in a loop for 500ms until the DRM node is ready.
      * On some targets, DRM loading takes a significant amount of time.
      * For example, on the SA6155, it takes around 280ms to 300ms.
      *
      * We proceed with display initialization once the DRM node is ready because if we
      * move the same loop to the actual DRM open, we encounter CRTC issues from the DRM module,
      * likely due to accessing and configuring it too quickly from RVC.
      */
    while ((fd == -1) && (retry_cnt < DRM_NODE_RETRY_CNT))
    {
        fd = open(DRM_NODE, O_RDWR, 0);
        if (fd == -1) {
            QCARCAM_ERRORMSG("open(%s) failed: errno %d. %s\n", "msm_drm", errno, strerror(errno));
            usleep(5000); // 5 milliseconds
            retry_cnt++;
       }
    }
    if (fd > 0)  {
        place_marker("boot_kpi:  M - RVC - setup_windowing_thread After opening /dev/dri/card2");
        close (fd);
    } else {
        place_marker("boot_kpi:  M - RVC - Faied opening /dev/dri/card2");
    }

    ret = test_util_init_display(pInputCtxt->test_util_ctxt);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("test_util_init_display failed");
        return -1;
    }
    ret = test_util_init_c2d_buffers(pInputCtxt->test_util_ctxt, pInputCtxt->qcarcam_window);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("test_util_init_c2d_buffers failed");
        return -1;
    }

    pInputCtxt->is_display_initialized = true;
    QCARCAM_TEST_LOG_PERF("window initialization done");
    place_marker("boot_kpi: M - RVC - display_initialization done");

    return 0;

}


/**
 * Thread to setup and run qcarcam APIs
 *
 * @param arg qcarcam_test_input_t* input_ctxt
 */
static int setup_qcarcam_thread(void *arg)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    qcarcam_test_input_t* pInputCtxt = &gRvcInput;
    int nWindowThreadExit = 0;
    int rc = -1;
    unsigned int retry_cnt = 0;

    QCARCAM_INFOMSG("setup_qcarcam_thread input_desc=%d", pInputCtxt->qcarcam_input_id);

    QCARCAM_TEST_LOG_PERF("setup thread start");

    qcarcam_init_t qcarcam_init = { 0 };
    qcarcam_init.version = QCARCAM_VERSION;
    ret = qcarcam_initialize(&qcarcam_init);
    if (ret != QCARCAM_RET_OK)
    {
        place_marker("boot_kpi:  M - RVC - qcarcam_initialize failed");
        QCARCAM_ERRORMSG("qcarcam_initialize failed %d", ret);
        exit(-1);
    }
    QCARCAM_TEST_LOG_PERF("setup qcarcam_initialize done");

    /*Poll until RVC camera is available*/
    uint32 count = 0;
#if defined(__QNXNTO__)
    int poll_ms = 10;
    int delay_ms = 2000;
    QCARCAM_DBGMSG("Waiting for ais_socket_0");
    if (!waitfor(AIS_SERVER_SOCKET_PATH, delay_ms, poll_ms )) {
        QCARCAM_INFOMSG("We have ais_socket_0");
    } else {
        QCARCAM_ERRORMSG("Wait for ais_socket_0 failed");
        goto qcarcam_thread_fail;
    }
#endif
#ifdef ANDROID_T_ABOVE
    place_marker("boot_kpi:  M - RVC - ais probing socket");
    rc = access(AIS_SOCKET, F_OK);
    while ((rc == -1) && (retry_cnt < AIS_SOCKET_ACCESS_RETRY)) {
        rc = access(AIS_SOCKET, F_OK);
        if(rc == -1)
          usleep(5000);
        retry_cnt++;
    }
    if (rc == 0)
    {
        place_marker("boot_kpi:  M - RVC - ais socket is available");
        QCARCAM_DBGMSG("ais socket is available \n");
    } else
    {
        place_marker("boot_kpi: M - RVC - ais socket access failed!!");
        QCARCAM_ERRORMSG("ais socket access failed for path:%s with errno %d. %s",AIS_SOCKET, errno, strerror(errno));
    }
#endif
   place_marker("boot_kpi:  M - RVC - qcarcam_open()");
    do
    {
        pInputCtxt->qcarcam_context = qcarcam_open(pInputCtxt->qcarcam_input_id);
        if (!pInputCtxt->qcarcam_context)
        {
            if (count++ == QCARCAM_OPEN_CNT)
            {
                QCARCAM_ERRORMSG("qcarcam_open() failed");
                place_marker("boot_kpi:  M - RVC - qcarcam_open() failed");
                goto qcarcam_thread_fail;
            }
            QCARCAM_ERRORMSG("qcarcam_open() try again...");
            CameraSleep(100);
        }
    } while(!pInputCtxt->qcarcam_context);

    QCARCAM_TEST_LOG_PERF("qcarcam_open done");

    QCARCAM_INFOMSG("input_desc=%d context=%p", pInputCtxt->qcarcam_input_id, pInputCtxt->qcarcam_context);


    ret = qcarcam_s_buffers(pInputCtxt->qcarcam_context, &pInputCtxt->p_buffers_output);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("qcarcam_s_buffers() failed");
        place_marker("boot_kpi: RVC_KPI_DEBUG qcarcam_s_buffers() failed");
        goto qcarcam_thread_fail;
    }
    QCARCAM_TEST_LOG_PERF("qcarcam_s_buffers done");


    if (pInputCtxt->use_event_callback)
    {
        qcarcam_param_value_t param;
        param.ptr_value = (void *)qcarcam_test_event_cb;
        ret = qcarcam_s_param(pInputCtxt->qcarcam_context, QCARCAM_PARAM_EVENT_CB, &param);

        param.uint_value = QCARCAM_EVENT_FRAME_READY | QCARCAM_EVENT_INPUT_SIGNAL | QCARCAM_EVENT_ERROR;
        ret = qcarcam_s_param(pInputCtxt->qcarcam_context, QCARCAM_PARAM_EVENT_MASK, &param);
    }

    QCARCAM_TEST_LOG_PERF("qcarcam_s_param done");

    pInputCtxt->is_running = 1;
    ret = qcarcam_start(pInputCtxt->qcarcam_context);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("qcarcam_start() failed");
        place_marker("boot_kpi:  M - RVC - qcarcam_start() failed");
        goto qcarcam_thread_fail;
    }

    QCARCAM_TEST_LOG_PERF("qcarcam_start done");

    if (!pInputCtxt->use_event_callback)
    {
        while (!g_aborted)
        {
            if (qcarcam_test_handle_new_frame(pInputCtxt))
                break;
        }
    }
    else
    {
        pthread_mutex_lock(&gCtxt.mutex_abort);
        if (!g_aborted)
        {
            pthread_cond_wait(&gCtxt.cond_abort, &gCtxt.mutex_abort);
        }
        pthread_mutex_unlock(&gCtxt.mutex_abort);
    }

    QCARCAM_INFOMSG("exit setup_input_ctxt_thread");
    return 0;

qcarcam_thread_fail:
    place_marker("boot_kpi:  M - RVC - qcarcam_thread_fail:");
    if (pInputCtxt->qcarcam_context)
    {
        qcarcam_ret_t ret = QCARCAM_RET_OK;
        ret = qcarcam_close(pInputCtxt->qcarcam_context);
        pInputCtxt->qcarcam_context = NULL;
        QCARCAM_INFOMSG("qcarcam_close ret=%d", ret);
    }

    return -1;
}

void display_menu()
{
    printf("\n ========================================================== \n");
    printf(" 'z'.....Exit \n");
    printf("\n =========================================================== \n");
    printf(" Enter your choice\n");
}

void process_cmds(char choice)
{
    switch (choice)
    {
    /* Exit app */
    case 'z':
        abort_test();
        break;
    default:
        QCARCAM_ERRORMSG("Invalid Input");
        break;
    }
}

int main(int argc, char **argv)
{

    place_marker("boot_kpi: M - RVC - Starting qcarcam_rvc");

    ais_log_init(NULL, (char *)"qcarcam_rvc");

    initialize_qcarcam_test_ctxt();

    int rval = EXIT_FAILURE;
    int rc;
    unsigned int i;

    void *signal_thread_handle = NULL;
    void *timer_thread_handle = NULL;

    char thread_name[64];
    const char *tok;
    bool enableMenu = false;
    int ret = -1;
    unsigned int retry_cnt = 0;
    int fail = 0;

    test_util_ctxt_t* test_util_ctxt = NULL;
    test_util_ctxt_params_t test_util_ctxt_params;
    memset(&test_util_ctxt_params, 0x0, sizeof(test_util_ctxt_params));

    QCARCAM_DBGMSG("Arg parse Begin");

    for (int idx = 1; idx < argc; idx++)
    {
        if (!strncmp(argv[idx], "-seconds=", strlen("-seconds=")))
        {
            tok = argv[idx] + strlen("-seconds=");
            exitSeconds = atoi(tok);
        }
        else if (!strncmp(argv[idx], "-menu", strlen("-menu")))
        {
            enableMenu = true;
        }
        else if (!strncmp(argv[idx], "-noDisplay", strlen("-noDisplay")))
        {
            gCtxt.disable_display = true;
        }
        else if (!strncmp(argv[idx], "-dumpFrame=", strlen("-dumpFrame=")))
        {
            tok = argv[idx] + strlen("-dumpFrame=");
            gCtxt.dumpFrame = atoi(tok);
        }
        else
        {
            QCARCAM_ERRORMSG("Invalid argument");
            place_marker("RVC ..  Invalid argument exiting app..");
            exit(-1);
        }
    }

#ifdef RVC_SELF_EXIT
    exitSeconds = 5;
#endif

    QCARCAM_DBGMSG("Arg parse End");

    sigset_t sigset;
    sigfillset(&sigset);
    for (i = 0; exceptsigs[i] != -1; i++)
    {
        sigdelset(&sigset, exceptsigs[i]);
    }
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    /**/
    qcarcam_test_input_t* pInputCtxt = &gRvcInput;
    pInputCtxt->qcarcam_input_id = QCARCAM_INPUT_TYPE_EXT_REAR;
    pInputCtxt->use_event_callback = 1;
    pInputCtxt->is_display_initialized = false;
    /*no timeout when using event callback as frame is ready as soon as we get callback*/
    pInputCtxt->frame_timeout = pInputCtxt->use_event_callback ? 0 : QCARCAM_TEST_DEFAULT_GET_FRAME_TIMEOUT;

    if (gCtxt.disable_display == true)
    {
        test_util_ctxt_params.disable_display = 1;
    }
#if defined(__QNXNTO__)
    int poll_ms = 10;
    int delay_ms = 2000;
    QCARCAM_DBGMSG("Waiting for /dev/screen");
    if (!waitfor("/dev/screen", delay_ms, poll_ms )) {
        QCARCAM_INFOMSG("We have /dev/screen");
    } else {
        QCARCAM_ERRORMSG("Wait for /dev/screen failed");
        goto fail;
    }
#endif
    /*Initialize test utils for corresponding platform*/
    QCARCAM_DBGMSG("test_util_init");
    rc = test_util_init(&test_util_ctxt, &test_util_ctxt_params);
    if (rc != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("test_util_init failed");
        place_marker("boot_kpi: M - RVC - test_util_init failed exiting rvc app");
        exit(-1);
    }

    QCARCAM_TEST_LOG_PERF("test_util_init done");

    pInputCtxt->test_util_ctxt = test_util_ctxt;


    rc = test_util_init_window(pInputCtxt->test_util_ctxt, &pInputCtxt->qcarcam_window);
    if (rc != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("test_util_init_window failed for qcarcam_window");
        place_marker("boot_kpi:  M - RVC - test_util_init_window failed");
        exit(-1);
    }
    pInputCtxt->p_buffers_output.buffers = pInputCtxt->pBuffers;

    ret = test_util_set_window_params(&pInputCtxt->buffers_param, &pInputCtxt->window_params,
        &pInputCtxt->p_buffers_output, pInputCtxt->qcarcam_window);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("test_util_set_window_params failed for qcarcam_window");
        place_marker("boot_kpi: M - RVC - test_util_set_window_params failed exiting rvc app");
        exit(-1);
    }

    ret = test_util_alloc_camera_buffers(pInputCtxt->qcarcam_window, &pInputCtxt->p_buffers_output);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("test_util_alloc_camera_buffers failed");
        place_marker("boot_kpi:  M - RVC - test_util_alloc_camera_buffers failed exiting rvc app");
        exit(-1);
    }
    snprintf(thread_name, sizeof(thread_name), "setup_windowing_thread");
    ret = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &setup_windowing_thread, &gRvcInput, 0, thread_name, &gRvcInput.window_thread_handle);
    if (ret)
    {
        QCARCAM_ERRORMSG("CameraCreateThread failed failed : %s", thread_name);
        fail = 1;
        goto fail;
    }

    /*signal handler thread*/
    snprintf(thread_name, sizeof(thread_name), "signal_thrd");
    rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &signal_thread, 0, 0, thread_name, &signal_thread_handle);
    if (rc)
    {
        QCARCAM_ERRORMSG("CameraCreateThread failed : %s", thread_name);
        fail = 1;
        goto fail;
    }
    QCARCAM_DBGMSG("Screen Setup End");
    QCARCAM_DBGMSG("Create qcarcam_context Begin");
    snprintf(thread_name, sizeof(thread_name), "setup_rvc_thrd");
    rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &setup_qcarcam_thread, &gRvcInput, 0, thread_name, &gRvcInput.qcarcam_thread_handle);
    if (rc)
    {
        QCARCAM_ERRORMSG("CameraCreateThread failed : %s", thread_name);
        fail = 1;
        goto fail;
    }

    if (exitSeconds)
    {
        snprintf(thread_name, sizeof(thread_name), "timer_thread");
        rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &timer_thread, 0, 0, thread_name, &timer_thread_handle);
        if (rc)
        {
            QCARCAM_ERRORMSG("pthread_create failed");
            fail = 1;
            goto fail;
        }
    }
    else if (enableMenu)
    {
        while (!g_aborted)
        {

            fflush(stdin);
            display_menu();

            while (!g_aborted)
            {
                int ch = 0;
                char buf[BUFSIZE];
                fd_set readfds;
                struct timeval tv;
                FD_ZERO(&readfds);
                FD_SET(fileno(stdin), &readfds);

                tv.tv_sec = 0;
                tv.tv_usec = 100000;

                int num = select(1, &readfds, NULL, NULL, &tv);
                if (num > 0)
                {
                    if (fgets(buf, sizeof(buf), stdin) != NULL)
                    {
                        ch = buf[0];
                    }

                    if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z'))
                    {
                        process_cmds(ch);
                        break;
                    }
                }
            }
        }
    }
    CameraJoinThread(gRvcInput.qcarcam_thread_handle, NULL);
    CameraReleaseThread(gRvcInput.qcarcam_thread_handle);
    CameraJoinThread(gRvcInput.window_thread_handle, NULL);
    CameraReleaseThread(gRvcInput.window_thread_handle);

    // success case
    rval = EXIT_SUCCESS;

fail:
   if (fail == 1) {
       place_marker("boot_kpi: M - RVC - qcarcam_rvc failed !");
   }
    // cleanup
    {
        if (gRvcInput.qcarcam_context != NULL)
        {
            (void)qcarcam_stop(gRvcInput.qcarcam_context);
            (void)qcarcam_close(gRvcInput.qcarcam_context);
            gRvcInput.qcarcam_context = NULL;
        }

        if (true == gRvcInput.is_display_initialized)
            test_util_deinit_display(gRvcInput.test_util_ctxt, gRvcInput.qcarcam_window);

        test_util_deinit_window_buffer(gRvcInput.test_util_ctxt, gRvcInput.qcarcam_window);
        test_util_deinit_window(gRvcInput.test_util_ctxt, gRvcInput.qcarcam_window);
    }

    if (!g_aborted)
    {
        abort_test();
    }

    test_util_deinit(gRvcInput.test_util_ctxt);

    qcarcam_uninitialize();

    ais_log_uninit();
    place_marker("boot_kpi: M - RVC - qcarcam_rvc App Exiting");
    return rval;
}

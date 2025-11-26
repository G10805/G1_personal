/* ===========================================================================
 * Copyright (c) 2019, 2021 Qualcomm Technologies, Inc.
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
#include <semaphore.h>
#include <signal.h>


#ifndef C2D_DISABLED
#endif
#include "qcarcam.h"
#include "test_util.h"

#include "ais_v4l2_proxy_util.h"

#include <linux/videodev2.h>
#include <poll.h>

#define NUM_MAX_CAMERAS 16
#define NUM_MAX_DISP_BUFS 3
#define DEFAULT_FRAME_TIMEOUT 500000000
#define DEFAULT_PRINT_DELAY_SEC 10
#define SIZE_ALIGN 64
#define SIGNAL_CHECK_DELAY 33333
#define CSI_ERR_CHECK_DELAY 100000
#define DUMMY_FRAME_INTERVAL_US 33333

#define QCARCAM_MAX_RING_BUFFERS 5

//We are getting poll event value as 0x50 on stream off from v4l2loopback
#define POLL_CLOSE_EVENT 0x50

typedef enum
{
    AIS_BUFFERS_DIRECT_OUTPUT = 0,
    AIS_BUFFERS_C2D_OUTPUT,
    AIS_BUFFERS_MAX
} ais_buffers_list_t;

typedef enum
{
    AIS_PROXY_STREAMING,
    AIS_PROXY_IN_RECOVERY,
    AIS_PROXY_RECOVERY_ABORTED
} ais_proxy_server_input_state_t;

typedef struct
{
    qcarcam_buffers_t p_buffers;
    qcarcam_color_fmt_t format;
    int n_buffers;
    unsigned int width;
    unsigned int height;
} qcarcam_buffers_param_t;


typedef struct
{
    void* thread_handle;
    int idx;
    int query_inputs_idx;
    unsigned long long int frame_timeout;

    /*qcarcam context*/
    qcarcam_hndl_t qcarcam_context;
    qcarcam_input_desc_t qcarcam_input_id;
    qcarcam_buffers_param_t buffers_param[AIS_BUFFERS_MAX];

    /*v4l2 params*/
    qcarcam_v4l2_param_t v4l2_params;

    /*Buffers*/
    qcarcam_buffers_t p_buffers_output;
    qcarcam_buffers_t p_buffers_c2d;

    /* frame buffer objects */
    qcarcam_frame_buffer_t *direct_frame_buffer;
    qcarcam_frame_buffer_t *c2d_frame_buffer;

    /* buffer management tracking */
    int buf_idx_qcarcam;
    int prev_buf_idx_qcarcam;
    int buf_idx_c2d;

    /* diag */
    int frameCnt;
    int prev_frame;
    int is_running;
    bool is_first_count;
    bool is_signal_lost;
    bool is_csi_error;
    ais_proxy_server_input_state_t state;

    bool is_buf_dequeued[QCARCAM_MAX_NUM_BUFFERS];
    bool recovery;
    sem_t sem_recovery;
    pthread_mutex_t mutex_recovery;
} ais_proxy_server_input_t;

///////////////////////////////
/// STATICS
///////////////////////////////
static ais_proxy_server_input_t g_ais_proxy_inputs[NUM_MAX_CAMERAS] = {};
static bool qcarcam_inputs_signal_lost[NUM_MAX_CAMERAS] = {};
static volatile int qcarcam_csi_err_cnt[NUM_MAX_CAMERAS] = {0};
static int numInputs = 0;
static int Qcarcam_Initialized = 0;

/* 0 : post buffers directly to v4l2 node
 * 1 : blit buffers to v4l2 node buffers through c2d
 */
static int enable_c2d = 0;

static pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
#ifndef C2D_DISABLED
static pthread_mutex_t mutex_c2d = PTHREAD_MUTEX_INITIALIZER;
#endif
static int count_test = 0;
static int toggle = 0;
static int dumpFrame = 0;
static char filename[128] = "/vendor/bin/ais_v4l2loopback_config.xml";
static int enableBridgeErrorDetect = 1;
static int enableCSIErrorDetect = 0;

/*recovery handling*/
static int enableDummyFramesInRecovery = 1;

static int fps_print_delay = DEFAULT_PRINT_DELAY_SEC;

/*abort condition*/
static pthread_mutex_t mutex_abort = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_abort = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex_csi_err = PTHREAD_MUTEX_INITIALIZER;
static unsigned long t_start;
static volatile int aborted = 0;

static const int exceptsigs[] = {
    SIGCHLD, SIGIO, SIGURG, SIGWINCH,
    SIGSTOP, SIGTSTP, SIGTTIN, SIGTTOU, SIGCONT,
    -1,
};


int qcarcam_get_system_time(unsigned long *pTime)
{
    struct timespec time;
    unsigned long msec;

    if (clock_gettime(CLOCK_MONOTONIC, &time) == -1)
    {
        QCARCAM_ERRORMSG("Clock gettime failed");
        return 1;
    }
    msec = ((unsigned long)time.tv_sec * 1000) + (((unsigned long)time.tv_nsec / 1000) / 1000);
    *pTime = msec;

    return 0;
}

void qcarcam_calculate_frame_rate(unsigned long *timer1)
{
    float average_fps;
    int input_idx, frames_counted;
    unsigned long timer2;

    qcarcam_get_system_time(&timer2);

    for (input_idx = 0; input_idx < numInputs; ++input_idx)
    {
        if (g_ais_proxy_inputs[input_idx].is_running)
        {
            if (!g_ais_proxy_inputs[input_idx].is_first_count)
            {
                g_ais_proxy_inputs[input_idx].prev_frame = g_ais_proxy_inputs[input_idx].frameCnt;
                g_ais_proxy_inputs[input_idx].is_first_count = 1;
            }
            else
            {
                frames_counted = g_ais_proxy_inputs[input_idx].frameCnt - g_ais_proxy_inputs[input_idx].prev_frame;
                average_fps = frames_counted / ((timer2 - *timer1) / 1000);
                QCARCAM_ERRORMSG("Average FPS: %.2f input_id: %d idx: %d\n ", average_fps, (int)g_ais_proxy_inputs[input_idx].qcarcam_input_id, g_ais_proxy_inputs[input_idx].idx);
                g_ais_proxy_inputs[input_idx].prev_frame = g_ais_proxy_inputs[input_idx].frameCnt;
            }
        }
    }
    fflush(stdout);
    qcarcam_get_system_time(timer1);
}

static int check_signal_loss(bool *signal_lost, bool *signal_lost_check, unsigned int idx)
{
    if (*signal_lost != *signal_lost_check)
    {
        // Check if signal status has changed
        *signal_lost_check = *signal_lost;
        g_ais_proxy_inputs[idx].is_signal_lost = *signal_lost_check;
    }
    else
    {
        if (*signal_lost_check == 1)
        {
            // Posting empty frame to v4l2node
            if (g_ais_proxy_inputs[idx].state == AIS_PROXY_RECOVERY_ABORTED)
            {
                g_ais_proxy_inputs[idx].v4l2_params.buffer_status = V4L2_BUF_FLAG_LAST;
            }
            else
            {
                g_ais_proxy_inputs[idx].v4l2_params.buffer_status = V4L2_BUF_FLAG_ERROR;
            }
            post_ion_buffer_to_v4l2node(g_ais_proxy_inputs[idx].v4l2_params.v4l2sink,
                                        g_ais_proxy_inputs[idx].direct_frame_buffer,
                                        g_ais_proxy_inputs[idx].p_buffers_output.n_buffers,
                                        &g_ais_proxy_inputs[idx].v4l2_params);
        }
    }
    return 0;
}

static int check_signal_loss_thread(void *arg)
{
    pthread_detach(pthread_self());

    bool signal_lost[NUM_MAX_CAMERAS] = {};
    bool signal_lost_check[NUM_MAX_CAMERAS] = {};
    unsigned int signal_check_delay_us = SIGNAL_CHECK_DELAY; // 33 milliseconds

    while (!aborted)
    {
        // Check if signal status has changed
        for (int i = 0; i < numInputs; i++)
        {
            signal_lost[i] = qcarcam_inputs_signal_lost[i];
            check_signal_loss(&signal_lost[i], &signal_lost_check[i], i);
        }
        usleep(signal_check_delay_us);
    }
    return 0;
}

static int verify_csi_error_check(volatile int *csi_err_cnt, volatile int *csi_err_cnt_prev, unsigned int idx)
{
    int ret = 0;
    ais_proxy_server_input_t *ais_proxy_ctxt = &g_ais_proxy_inputs[idx];

    pthread_mutex_lock(&mutex_csi_err);

    if (*csi_err_cnt != *csi_err_cnt_prev)
    {
        // csi error happens, check again next loop
        *csi_err_cnt_prev = *csi_err_cnt;
        ais_proxy_ctxt->is_csi_error = 1;
    }
    else
    {
        // no new csi error comes, can re-start
        if (ais_proxy_ctxt->is_csi_error)
        {
            if (ais_proxy_ctxt->is_running == 1) {
                QCARCAM_ERRORMSG("Input %d already running, return", ais_proxy_ctxt->qcarcam_input_id);
                pthread_mutex_unlock(&mutex_csi_err);
                return ret;
            }

            ais_proxy_ctxt->is_running = 1;
            ret = qcarcam_resume(ais_proxy_ctxt->qcarcam_context);
            if (ret == QCARCAM_RET_OK) {
                QCARCAM_DBGMSG("Client %d Input %d qcarcam_resume successfully", idx, ais_proxy_ctxt->qcarcam_input_id);
                qcarcam_inputs_signal_lost[ais_proxy_ctxt->idx] = 0;
            } else {
                ais_proxy_ctxt->is_running = 0;
                QCARCAM_ERRORMSG("Client %d Input %d qcarcam_resume failed: %d", idx, ais_proxy_ctxt->qcarcam_input_id, ret);
            }

            ais_proxy_ctxt->is_csi_error = 0;
        }
    }

    pthread_mutex_unlock(&mutex_csi_err);

    return ret;
}

static int check_csi_error_thread(void *arg)
{
    volatile int csi_error_prev[NUM_MAX_CAMERAS] = {0};
    unsigned int error_check_delay_us = CSI_ERR_CHECK_DELAY; // 100 milliseconds

    pthread_detach(pthread_self());

    while (!aborted)
    {
        // Check if csi error continues or not
        for (int i = 0; i < numInputs; i++)
        {
            verify_csi_error_check(&qcarcam_csi_err_cnt[i], &csi_error_prev[i], i);
        }
        usleep(error_check_delay_us);
    }
    return 0;
}

/**
 * post a dummy frame while the context is in recovery
 *
 * @param arg ais_proxy_server_input_t* ais_proxy_ctxt
 */
static int post_dummy_frame(ais_proxy_server_input_t *ais_proxy_ctxt)
{
    int result = -1;

    pthread_mutex_lock(&ais_proxy_ctxt->mutex_recovery);

    if (ais_proxy_ctxt->state == AIS_PROXY_RECOVERY_ABORTED || ais_proxy_ctxt->state == AIS_PROXY_IN_RECOVERY) {
        post_ion_buffer_to_v4l2node(ais_proxy_ctxt->v4l2_params.v4l2sink,
                                    ais_proxy_ctxt->direct_frame_buffer,
                                    ais_proxy_ctxt->buf_idx_qcarcam,
                                    &ais_proxy_ctxt->v4l2_params);
        result = 0;
    }

    pthread_mutex_unlock(&ais_proxy_ctxt->mutex_recovery);

    return result;
}

/**
 * thread that checks if a context is in recovery and sends dummy frames at 30fps if it is
 *
 * @param arg void* arg
 */
static int post_dummy_frames_thread(void *arg)
{
    pthread_detach(pthread_self());

    ais_proxy_server_input_t *ais_proxy_ctxt = (ais_proxy_server_input_t *)arg;

    if (!ais_proxy_ctxt)
        return -1;

    while (!aborted)
    {
        sem_wait(&ais_proxy_ctxt->sem_recovery);
        post_dummy_frame(ais_proxy_ctxt);
        usleep(DUMMY_FRAME_INTERVAL_US);
    }

    QCARCAM_INFOMSG("Dummy frame thread stopped");
    return 0;
}

static int framerate_thread(void *arg)
{
    pthread_detach(pthread_self());

    unsigned int fps_print_delay_us = fps_print_delay * 1000000;
    unsigned long timer1;
    qcarcam_get_system_time(&timer1);

    while (!aborted)
    {
        qcarcam_calculate_frame_rate(&timer1);
        usleep(fps_print_delay_us);
    }
    return 0;
}

static void abort_ais_proxy_server(void)
{
    QCARCAM_ERRORMSG("Aborting ais_proxy server");
    pthread_mutex_lock(&mutex_abort);
    aborted = 1;
    pthread_cond_broadcast(&cond_abort);
    pthread_mutex_unlock(&mutex_abort);
}

static int signal_thread(void *arg)
{
    sigset_t sigset;
    int sig;
    int i;

    pthread_detach(pthread_self());
    sigfillset(&sigset);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    for (i = 0; exceptsigs[i] != -1; i++)
    {
        sigdelset(&sigset, exceptsigs[i]);
    }

    for (;;)
    {
        if (sigwait(&sigset, &sig) == 0)
        {
            abort_ais_proxy_server();
            break;
        }
    }
    return 0;
}

/**
 * Function to retrieve frame from qcarcam and increase frame_counter
 * @param ais_proxy_ctxt
 * @return 0 on success, -1 on failure
 */
static int qcarcam_get_new_frame(ais_proxy_server_input_t *ais_proxy_ctxt)
{
    qcarcam_ret_t ret;
    qcarcam_frame_info_t frame_info;
    ret = qcarcam_get_frame(ais_proxy_ctxt->qcarcam_context, &frame_info, ais_proxy_ctxt->frame_timeout, 0);
    if (ret == QCARCAM_RET_TIMEOUT)
    {
        QCARCAM_ERRORMSG("qcarcam_get_frame timeout context 0x%p ret %d\n", ais_proxy_ctxt->qcarcam_context, ret);
        qcarcam_inputs_signal_lost[ais_proxy_ctxt->idx] = 1;
        return -1;
    }

    if (QCARCAM_RET_OK != ret)
    {
        QCARCAM_ERRORMSG("Get frame context 0x%p ret %d\n", ais_proxy_ctxt->qcarcam_context, ret);
        return -1;
    }

    if (frame_info.idx >= ais_proxy_ctxt->buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].n_buffers)
    {
        QCARCAM_ERRORMSG("Get frame context 0x%p ret invalid idx %d\n", ais_proxy_ctxt->qcarcam_context, frame_info.idx);
        return -1;
    }

    if (ais_proxy_ctxt->frameCnt == 0)
    {
        unsigned long t_to;
        qcarcam_get_system_time(&t_to);
        printf("Success\n");
        fflush(stdout);
        QCARCAM_ALWZMSG("Get First Frame input_id %d buf_idx %i after : %lu ms, field type: %d", (int)ais_proxy_ctxt->qcarcam_input_id, ais_proxy_ctxt->buf_idx_qcarcam, (t_to - t_start), frame_info.field_type);
    }

    ais_proxy_ctxt->buf_idx_qcarcam = frame_info.idx;
    qcarcam_inputs_signal_lost[ais_proxy_ctxt->idx] = 0;

    QCARCAM_DBGMSG("context:%d frameCnt:%d idx:%d, input_id = %d", ais_proxy_ctxt->idx, ais_proxy_ctxt->frameCnt, frame_info.idx, ais_proxy_ctxt->qcarcam_input_id);

    ais_proxy_ctxt->is_buf_dequeued[ais_proxy_ctxt->buf_idx_qcarcam] = 1;
    ais_proxy_ctxt->frameCnt++;

    return 0;
}
/**
 * Function to post new frame to v4l2node. May also do color conversion and frame dumps.
 * @param ais_proxy_ctxt
 * @return 0 on success, -1 on failure
 */
static int qcarcam_post_frame_to_v4l2node(ais_proxy_server_input_t *ais_proxy_ctxt)
{
    qcarcam_ret_t ret;
    /**********************
     * Composite to v4l2node
     ********************** */
    /**********************
     * Dump raw if necessary
     ********************** */
    if (0 != dumpFrame)
    {
        if (0 == ais_proxy_ctxt->frameCnt % dumpFrame)
        {
            snprintf(filename, sizeof(filename), DEFAULT_DUMP_LOCATION "frame_%d_%i.raw", ais_proxy_ctxt->idx, ais_proxy_ctxt->frameCnt);
            qcarcam_dump_frame_buffer(ais_proxy_ctxt->direct_frame_buffer, ais_proxy_ctxt->buf_idx_qcarcam, filename);
        }
    }
    /**********************
     * Color conversion if necessary
     ********************** */
    if (!enable_c2d)
    {
        /**********************
         * Post to screen
         ********************** */
        QCARCAM_INFOMSG("Post Frame %d", ais_proxy_ctxt->buf_idx_qcarcam);
        ret = post_ion_buffer_to_v4l2node(ais_proxy_ctxt->v4l2_params.v4l2sink, ais_proxy_ctxt->direct_frame_buffer, ais_proxy_ctxt->buf_idx_qcarcam, &ais_proxy_ctxt->v4l2_params);
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("post_ion_buffer_to_v4l2node failed");
        }
    }
#ifndef C2D_DISABLED
    else
    {
        //for now always go through c2d conversion instead of posting directly to v4l2 since gles composition cannot handle
        // uyvy buffers for now.
        QCARCAM_INFOMSG("%d converting through c2d %d -> %d", ais_proxy_ctxt->idx, ais_proxy_ctxt->buf_idx_qcarcam, ais_proxy_ctxt->buf_idx_c2d);

        C2D_STATUS c2d_status;
        C2D_OBJECT c2dObject;
        memset(&c2dObject, 0x0, sizeof(C2D_OBJECT));
        unsigned int target_id;
        ret = get_c2d_surface_id(ais_proxy_ctxt->direct_frame_buffer, ais_proxy_ctxt->buf_idx_qcarcam, &c2dObject.surface_id);
        ret = get_c2d_surface_id(ais_proxy_ctxt->c2d_frame_buffer, ais_proxy_ctxt->buf_idx_c2d, &target_id);
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("get_c2d_surface_id failed");
        }

        pthread_mutex_lock(&mutex_c2d);
        c2d_status = c2dDraw(target_id, C2D_TARGET_ROTATE_0, 0x0, 0, 0, &c2dObject, 1);

        c2d_ts_handle c2d_timestamp;
        if (c2d_status == C2D_STATUS_OK)
        {
            c2d_status = c2dFlush(target_id, &c2d_timestamp);
        }
        pthread_mutex_unlock(&mutex_c2d);

        if (c2d_status == C2D_STATUS_OK)
        {
            c2d_status = c2dWaitTimestamp(c2d_timestamp);
        }

        QCARCAM_INFOMSG("c2d conversion finished");

        if (c2d_status != C2D_STATUS_OK)
        {
            QCARCAM_ERRORMSG("c2d conversion failed with error %d", c2d_status);
        }
        /**********************
         * Dump if necessary
         ********************** */
        if (0 != dumpFrame)
        {
            if (0 == ais_proxy_ctxt->frameCnt % dumpFrame)
            {
                snprintf(filename, sizeof(filename), DEFAULT_DUMP_LOCATION "frame_v4l2_%d_%i.raw", ais_proxy_ctxt->idx, ais_proxy_ctxt->frameCnt);
                qcarcam_dump_frame_buffer(ais_proxy_ctxt->c2d_frame_buffer, ais_proxy_ctxt->buf_idx_c2d, filename);
            }
        }
        /**********************
         * Post to screen
         ********************** */
        QCARCAM_DBGMSG("Post Frame %d", ais_proxy_ctxt->buf_idx_c2d);
        ret = post_ion_buffer_to_v4l2node(ais_proxy_ctxt->v4l2_params.v4l2sink, ais_proxy_ctxt->c2d_frame_buffer, ais_proxy_ctxt->buf_idx_c2d, &ais_proxy_ctxt->v4l2_params);
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("post_ion_buffer_to_v4l2node failed");
        }

        ais_proxy_ctxt->buf_idx_c2d++;
        ais_proxy_ctxt->buf_idx_c2d %= ais_proxy_ctxt->buffers_param[AIS_BUFFERS_C2D_OUTPUT].n_buffers;
    }
#endif
    QCARCAM_INFOMSG("Post Frame after buf_idx %i", ais_proxy_ctxt->buf_idx_qcarcam);

    return 0;
}
/**
 * Release frame back to qcarcam
 * @param ais_proxy_ctxt
 * @return 0 on success, -1 on failure
 */
static int qcarcam_release_used_frame(ais_proxy_server_input_t *ais_proxy_ctxt)
{
    qcarcam_ret_t ret;
    if (ais_proxy_ctxt->prev_buf_idx_qcarcam >= 0 && ais_proxy_ctxt->prev_buf_idx_qcarcam < ais_proxy_ctxt->buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].n_buffers)
    {
        if (ais_proxy_ctxt->is_buf_dequeued[ais_proxy_ctxt->prev_buf_idx_qcarcam])
        {
            ret = qcarcam_release_frame(ais_proxy_ctxt->qcarcam_context, ais_proxy_ctxt->prev_buf_idx_qcarcam);
            if (QCARCAM_RET_OK != ret)
            {
                QCARCAM_ERRORMSG("qcarcam_release_frame() %d failed", ais_proxy_ctxt->prev_buf_idx_qcarcam);
                return -1;
            }
            ais_proxy_ctxt->is_buf_dequeued[ais_proxy_ctxt->prev_buf_idx_qcarcam] = 0;
        }
        else
        {
            QCARCAM_DBGMSG("qcarcam_release_frame() skipped since buffer %d not dequeued", ais_proxy_ctxt->prev_buf_idx_qcarcam);
        }
    }
    else
    {
        QCARCAM_DBGMSG("qcarcam_release_frame() skipped");
    }

    ais_proxy_ctxt->prev_buf_idx_qcarcam = ais_proxy_ctxt->buf_idx_qcarcam;
    return 0;
}

/**
 * Function to handle routine of fetching and releasing frames when one is available
 * @param ais_proxy_ctxt
 * @return 0 on success, -1 on failure
 */
static int qcarcam_handle_new_frame(ais_proxy_server_input_t *ais_proxy_ctxt)
{
    qcarcam_ret_t ret;

    if (qcarcam_get_new_frame(ais_proxy_ctxt))
    {
        /*if we fail to get frame, we silently continue...*/
        return 0;
    }

    qcarcam_post_frame_to_v4l2node(ais_proxy_ctxt);

    if (qcarcam_release_used_frame(ais_proxy_ctxt))
        return -1;

    return 0;
}

static int qcarcam_handle_input_signal(ais_proxy_server_input_t *ais_proxy_ctxt, qcarcam_input_signal_t signal_type)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    switch (signal_type) {
        case QCARCAM_INPUT_SIGNAL_LOST:
            QCARCAM_DBGMSG("LOST: idx: %d, input: %d", ais_proxy_ctxt->idx, ais_proxy_ctxt->qcarcam_input_id);

            if (ais_proxy_ctxt->is_running == 0) {
                QCARCAM_DBGMSG("Input %d already stop, break", ais_proxy_ctxt->qcarcam_input_id);
                break;
            }

            ais_proxy_ctxt->is_running = 0;
            ret = qcarcam_stop(ais_proxy_ctxt->qcarcam_context);
            if (ret == QCARCAM_RET_OK) {
                QCARCAM_INFOMSG("Input %d qcarcam_stop successfully", ais_proxy_ctxt->qcarcam_input_id);
            } else {
                QCARCAM_ERRORMSG("Input %d qcarcam_stop failed: %d !", ais_proxy_ctxt->qcarcam_input_id, ret);
            }

            ais_proxy_ctxt->prev_buf_idx_qcarcam = -1;
            memset(&ais_proxy_ctxt->is_buf_dequeued, 0x0, sizeof(ais_proxy_ctxt->is_buf_dequeued));
            qcarcam_inputs_signal_lost[ais_proxy_ctxt->idx] = 1;

            break;
        case QCARCAM_INPUT_SIGNAL_VALID:
            QCARCAM_DBGMSG("VALID: idx: %d, input: %d", ais_proxy_ctxt->idx, ais_proxy_ctxt->qcarcam_input_id);

            if (ais_proxy_ctxt->is_running == 1) {
                QCARCAM_DBGMSG("Input %d already running, break", ais_proxy_ctxt->qcarcam_input_id);
                break;
            }

            ret = qcarcam_s_buffers(ais_proxy_ctxt->qcarcam_context, &ais_proxy_ctxt->p_buffers_output);
            if (ret == QCARCAM_RET_OK)
                QCARCAM_INFOMSG("Input %d qcarcam_s_buffers successfully", ais_proxy_ctxt->qcarcam_input_id);
            else
                QCARCAM_ERRORMSG("Input %d qcarcam_s_buffers failed: %d", ais_proxy_ctxt->qcarcam_input_id, ret);

            ais_proxy_ctxt->is_running = 1;
            ret = qcarcam_start(ais_proxy_ctxt->qcarcam_context);
            if (ret == QCARCAM_RET_OK) {
                QCARCAM_INFOMSG("Input %d qcarcam_start successfully", ais_proxy_ctxt->qcarcam_input_id);
                qcarcam_inputs_signal_lost[ais_proxy_ctxt->idx] = 0;
            } else {
                ais_proxy_ctxt->is_running = 0;
                QCARCAM_ERRORMSG("Input %d qcarcam_start failed: %d", ais_proxy_ctxt->qcarcam_input_id, ret);
            }

            break;
        default:
            QCARCAM_ERRORMSG("Unknown Event type: %d", signal_type);
            break;
    }

    return ret;
}

static int qcarcam_handle_csi_error(ais_proxy_server_input_t *ais_proxy_ctxt)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    QCARCAM_ERRORMSG("CSI error: client idx - %d, input id - %d", ais_proxy_ctxt->idx, ais_proxy_ctxt->qcarcam_input_id);

    pthread_mutex_lock(&mutex_csi_err);

    qcarcam_inputs_signal_lost[ais_proxy_ctxt->idx] = 1;
    qcarcam_csi_err_cnt[ais_proxy_ctxt->idx]++;

    if (ais_proxy_ctxt->is_running == 0) {
        QCARCAM_INFOMSG("Input %d already stop, return", ais_proxy_ctxt->qcarcam_input_id);
        pthread_mutex_unlock(&mutex_csi_err);
        return ret;
    }

    ais_proxy_ctxt->is_running = 0;

    ret = qcarcam_pause(ais_proxy_ctxt->qcarcam_context);
    if (ret == QCARCAM_RET_OK) {
        QCARCAM_INFOMSG("Client %d Input %d qcarcam_pause successfully", ais_proxy_ctxt->idx, ais_proxy_ctxt->qcarcam_input_id);
    } else {
        QCARCAM_ERRORMSG("Client %d Input %d qcarcam_pause failed: %d !", ais_proxy_ctxt->idx, ais_proxy_ctxt->qcarcam_input_id, ret);
    }

    pthread_mutex_unlock(&mutex_csi_err);

    return ret;
}

/**
 * Qcarcam event callback function
 * @param hndl
 * @param event_id
 * @param p_payload
 */
static void qcarcam_event_cb(qcarcam_hndl_t hndl, qcarcam_event_t event_id, qcarcam_event_payload_t *p_payload)
{
    int i = 0;
    ais_proxy_server_input_t *ais_proxy_ctxt = NULL;
    for (i = 0; i < numInputs; i++)
    {
        if (hndl == g_ais_proxy_inputs[i].qcarcam_context)
        {
            ais_proxy_ctxt = &g_ais_proxy_inputs[i];
            break;
        }
    }

    if (!ais_proxy_ctxt)
    {
        QCARCAM_ERRORMSG("event_cb called with invalid qcarcam handle %p", hndl);
        return;
    }

    if (aborted)
    {
        QCARCAM_DBGMSG("ais_proxy server aborted");
        return;
    }

    switch (event_id)
    {
        case QCARCAM_EVENT_FRAME_READY:
            if (ais_proxy_ctxt->is_running)
            {
                QCARCAM_DBGMSG("%d received QCARCAM_EVENT_FRAME_READY", ais_proxy_ctxt->idx);
                qcarcam_handle_new_frame(ais_proxy_ctxt);
            }
            break;
        case QCARCAM_EVENT_INPUT_SIGNAL:
            if (enableBridgeErrorDetect)
                qcarcam_handle_input_signal(ais_proxy_ctxt, (qcarcam_input_signal_t)p_payload->uint_payload);
            break;
        case QCARCAM_EVENT_ERROR:
            if (enableCSIErrorDetect)
                qcarcam_handle_csi_error(ais_proxy_ctxt);
            break;
        case QCARCAM_EVENT_RECOVERY:
            if (enableDummyFramesInRecovery) {
                pthread_mutex_lock(&ais_proxy_ctxt->mutex_recovery);
                ais_proxy_ctxt->v4l2_params.buffer_status = V4L2_BUF_FLAG_ERROR;
                ais_proxy_ctxt->state = AIS_PROXY_IN_RECOVERY;
                pthread_mutex_unlock(&ais_proxy_ctxt->mutex_recovery);
                sem_post(&ais_proxy_ctxt->sem_recovery);
            }
            break;
        case QCARCAM_EVENT_RECOVERY_SUCCESS:
            if (enableDummyFramesInRecovery) {
                pthread_mutex_lock(&ais_proxy_ctxt->mutex_recovery);
                ais_proxy_ctxt->v4l2_params.buffer_status = 0;
                ais_proxy_ctxt->state = AIS_PROXY_STREAMING;
                pthread_mutex_unlock(&ais_proxy_ctxt->mutex_recovery);
            }
            break;
        case QCARCAM_EVENT_ERROR_ABORTED:
            if (enableDummyFramesInRecovery) {
                pthread_mutex_lock(&ais_proxy_ctxt->mutex_recovery);
                ais_proxy_ctxt->v4l2_params.buffer_status = V4L2_BUF_FLAG_LAST;
                ais_proxy_ctxt->state = AIS_PROXY_RECOVERY_ABORTED;
                pthread_mutex_unlock(&ais_proxy_ctxt->mutex_recovery);
                while (!aborted)
                {
                    sem_post(&ais_proxy_ctxt->sem_recovery);
                    usleep(DUMMY_FRAME_INTERVAL_US);
                }
            }
            break;
        default:
            QCARCAM_DBGMSG("%d received unsupported event %d", ais_proxy_ctxt->idx, event_id);
            break;
    }
}

int init_camera() {
    pthread_mutex_lock(&mutex_lock);
    if(Qcarcam_Initialized == 0)
    {
        qcarcam_ret_t ret = QCARCAM_RET_OK;
        qcarcam_init_t qcarcam_init = {0};
        qcarcam_init.version = QCARCAM_VERSION;
        ret = qcarcam_initialize(&qcarcam_init);
        if (ret != QCARCAM_RET_OK)
        {
            pthread_mutex_unlock(&mutex_lock);
            QCARCAM_ERRORMSG("qcarcam_initialize failed %d", ret);
            return -1;
        }
        Qcarcam_Initialized = 1;
        QCARCAM_ERRORMSG("Qcarcam Initialize Succeeded");
    }
    pthread_mutex_unlock(&mutex_lock);

    return 0;
}

void uninit_camera() {
    pthread_mutex_lock(&mutex_lock);
    if(Qcarcam_Initialized == 1)
    {
       qcarcam_uninitialize();
       Qcarcam_Initialized = 0;
       QCARCAM_ERRORMSG("Qcarcam unInitialize Succeeded");
    }
    pthread_mutex_unlock(&mutex_lock);
}

int restart_camera(ais_proxy_server_input_t *ais_proxy_ctxt);
int close_camera(ais_proxy_server_input_t *ais_proxy_ctxt);

int start_camera(ais_proxy_server_input_t *ais_proxy_ctxt) {
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    if (ais_proxy_ctxt == NULL) {
        QCARCAM_ERRORMSG("Invalid parameter");
        return -1;
    }

    QCARCAM_ERRORMSG("idx = %d, input_desc=%d start...\n", ais_proxy_ctxt->idx, ais_proxy_ctxt->qcarcam_input_id);

    pthread_mutex_lock(&mutex_lock);
    memset(&ais_proxy_ctxt->is_buf_dequeued, 0x0, sizeof(ais_proxy_ctxt->is_buf_dequeued));

    ais_proxy_ctxt->qcarcam_context = qcarcam_open(ais_proxy_ctxt->qcarcam_input_id);
    if (ais_proxy_ctxt->qcarcam_context == 0)
    {
        QCARCAM_ERRORMSG("qcarcam_open() failed");
        pthread_mutex_unlock(&mutex_lock);
        return -1;
    }

    // For HDMI/CVBS Input
    // NOTE: set HDMI IN resolution in qcarcam_config_single_hdmi.xml before
    // running the server
    if(ais_proxy_ctxt->qcarcam_input_id == QCARCAM_INPUT_TYPE_DIGITAL_MEDIA) {
        qcarcam_param_value_t param;
        param.res_value.width = ais_proxy_ctxt->buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].width;
        param.res_value.height = ais_proxy_ctxt->buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].height;
        ret = qcarcam_s_param(ais_proxy_ctxt->qcarcam_context, QCARCAM_PARAM_RESOLUTION, &param);
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("idx:%d qcarcam_s_param resolution() failed", ais_proxy_ctxt->idx);
            pthread_mutex_unlock(&mutex_lock);
            return -1;
        }
    }

    ret = qcarcam_s_buffers(ais_proxy_ctxt->qcarcam_context, &ais_proxy_ctxt->p_buffers_output);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("idx:%d qcarcam_s_buffers() failed", ais_proxy_ctxt->idx);
        pthread_mutex_unlock(&mutex_lock);
        return -1;
    }

    qcarcam_param_value_t param;
    param.ptr_value = (void *)qcarcam_event_cb;
    ret = qcarcam_s_param(ais_proxy_ctxt->qcarcam_context, QCARCAM_PARAM_EVENT_CB, &param);
    if(ret)
    {
        QCARCAM_ERRORMSG("idx:%d ERROR qcarcam_s_param failed for cb", ais_proxy_ctxt->idx);
        pthread_mutex_unlock(&mutex_lock);
        return -1;
    }

    if (ais_proxy_ctxt->recovery)
    {
        param.uint_value = 1;
        ret = qcarcam_s_param(ais_proxy_ctxt->qcarcam_context, QCARCAM_PARAM_RECOVERY, &param);
        if(ret)
        {
            QCARCAM_ERRORMSG("idx:%d ERROR qcarcam_s_param failed for recovery", ais_proxy_ctxt->idx);
            pthread_mutex_unlock(&mutex_lock);
            return -1;
        }
    }

    if (ais_proxy_ctxt->recovery)
    {
        param.uint_value = QCARCAM_EVENT_FRAME_READY |
            QCARCAM_EVENT_INPUT_SIGNAL |
            QCARCAM_EVENT_ERROR | QCARCAM_EVENT_VENDOR |
            QCARCAM_EVENT_RECOVERY | QCARCAM_EVENT_RECOVERY_SUCCESS |
            QCARCAM_EVENT_ERROR_ABORTED;
    }
    else
    {
        param.uint_value = QCARCAM_EVENT_FRAME_READY | QCARCAM_EVENT_INPUT_SIGNAL | QCARCAM_EVENT_ERROR;
    }

    ret = qcarcam_s_param(ais_proxy_ctxt->qcarcam_context, QCARCAM_PARAM_EVENT_MASK, &param);
    if(ret)
    {
        QCARCAM_ERRORMSG("idx:%d ERROR qcarcam_s_param failed for cb mask", ais_proxy_ctxt->idx);
        pthread_mutex_unlock(&mutex_lock);
        return -1;
    }

    QCARCAM_ERRORMSG("idx:%d qcarcam_start ...", ais_proxy_ctxt->idx);

    param.uint_value = (unsigned int) ais_proxy_ctxt->v4l2_params.opmode;
    ret = qcarcam_s_param(ais_proxy_ctxt->qcarcam_context, QCARCAM_PARAM_OPMODE, &param);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("qcarcam_s_param(QCARCAM_PARAM_OPMODE) failed");
        pthread_mutex_unlock(&mutex_lock);
        return -1;
    }
    QCARCAM_INFOMSG("setting opmode to %u - result = %d", param.uint_value, ret);

    /*single threaded handles frames outside this function*/
    ret = qcarcam_start(ais_proxy_ctxt->qcarcam_context);
    if (ret != QCARCAM_RET_OK){
        int result = 0;
        // try again
        QCARCAM_ERRORMSG("idx:%d qcarcam_start() failed, retry once", ais_proxy_ctxt->idx);
        pthread_mutex_unlock(&mutex_lock);
        result = restart_camera(ais_proxy_ctxt);
        if (result != 0) {
            QCARCAM_ERRORMSG("idx:%d retry start camera failed, so close camera",
                                            ais_proxy_ctxt->idx);
            close_camera(ais_proxy_ctxt);
        }
        return result;
    }

    ais_proxy_ctxt->is_running = 1;
    pthread_mutex_unlock(&mutex_lock);
    QCARCAM_ERRORMSG("idx:%d start over", ais_proxy_ctxt->idx);
    return 0;
}

int restart_camera(ais_proxy_server_input_t *ais_proxy_ctxt) {
#define RESTART_DELAY_TIME 100000  // 100ms
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    if (ais_proxy_ctxt == NULL) {
        QCARCAM_ERRORMSG("Invalid parameter");
        return -1;
    }

    QCARCAM_ERRORMSG("idx:%d Enter", ais_proxy_ctxt->idx);

    pthread_mutex_lock(&mutex_lock);
    ais_proxy_ctxt->is_running = 0;
    ret = qcarcam_stop(ais_proxy_ctxt->qcarcam_context);
    if (ret != QCARCAM_RET_OK) {
        QCARCAM_ERRORMSG("idx:%d qcarcam_stop() failed",
                        ais_proxy_ctxt->idx);
        pthread_mutex_unlock(&mutex_lock);
        return -1;
    }

    usleep(RESTART_DELAY_TIME);

    ret = qcarcam_s_buffers(ais_proxy_ctxt->qcarcam_context, &ais_proxy_ctxt->p_buffers_output);
    if (ret != QCARCAM_RET_OK)
        QCARCAM_ERRORMSG("idx:%d qcarcam_s_buffers failed: %d", ais_proxy_ctxt->idx, ret);

    ret = qcarcam_start(ais_proxy_ctxt->qcarcam_context);
    if (ret != QCARCAM_RET_OK){
        QCARCAM_ERRORMSG("idx:%d qcarcam_start() failed", ais_proxy_ctxt->idx);
        pthread_mutex_unlock(&mutex_lock);
        return -1;
    }
    ais_proxy_ctxt->is_running = 1;
    pthread_mutex_unlock(&mutex_lock);
    return 0;
}

int close_camera(ais_proxy_server_input_t *ais_proxy_ctxt) {
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    if (ais_proxy_ctxt == NULL) {
        QCARCAM_ERRORMSG("Invalid parameter");
        return -1;
    }

    QCARCAM_ERRORMSG("idx:%d Enter", ais_proxy_ctxt->idx);

    pthread_mutex_lock(&mutex_lock);
    ais_proxy_ctxt->is_running = 0;
    ret = qcarcam_stop(ais_proxy_ctxt->qcarcam_context);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("idx:%d qcarcam_stop() failed", ais_proxy_ctxt->idx);
        pthread_mutex_unlock(&mutex_lock);
        return -1;
    }

    ret = qcarcam_close(ais_proxy_ctxt->qcarcam_context);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("idx:%d qcarcam_close() failed", ais_proxy_ctxt->idx);
        ais_proxy_ctxt->qcarcam_context = NULL;
        pthread_mutex_unlock(&mutex_lock);
        return -1;
    }

    ais_proxy_ctxt->qcarcam_context = NULL;
    pthread_mutex_unlock(&mutex_lock);
    return 0;
}

/**
 * v4l2 poll thread to setup and run ais_v4l2_proxy based on input context
 *
 * @note For single threaded operation, this function only sets up ais_v4l2_proxy context.
 *      qcarcam_start and handling of frames is not executed.
 *
 * @param arg ais_proxy_server_input_t* ais_proxy_ctxt
 */
static int ais_v4l2_poll_thread(void *arg)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    int rc = -1;
    int ret_bc, fd;
    short revents;
    struct pollfd pfd;

    ais_proxy_server_input_t *ais_proxy_ctxt = (ais_proxy_server_input_t *)arg;

    if (!ais_proxy_ctxt)
        return -1;

    fd = open(ais_proxy_ctxt->v4l2_params.v4l2_node, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        QCARCAM_ERRORMSG("\n if open of fd is error \n ");
        return -1;
    }

    QCARCAM_ERRORMSG("\n fd open is successful \n ");

    if (init_camera() != 0) {
        QCARCAM_ERRORMSG("\n idx:%d can not init camera, abort\n ", ais_proxy_ctxt->idx);
        close(fd);
        return -1;
    }

    pfd.fd = fd;
    pfd.events = POLLIN|POLLRDNORM|POLLPRI;
    while(!aborted)
    {
        ret_bc = poll(&pfd, 1, -1);
        if (ret_bc < 0) {
            QCARCAM_ERRORMSG("\n polling is fail \n ");
            continue;
        }

        revents = pfd.revents;
        QCARCAM_ERRORMSG("idx:%d, polling revents value : %u", ais_proxy_ctxt->idx, revents);
        if (revents & POLLIN) {
            if (ais_proxy_ctxt->is_running == 1) {
                QCARCAM_ERRORMSG("idx:%d, already streamon, skip", ais_proxy_ctxt->idx);
                continue;
            }

            if (start_camera(ais_proxy_ctxt) != 0) {
                // start failed
                continue;
            }

            QCARCAM_ERRORMSG("idx:%d start over", ais_proxy_ctxt->idx);
            continue;
        } else if(revents & POLLERR) {
            QCARCAM_ERRORMSG("idx:%d POLLERR error", ais_proxy_ctxt->idx);
            /*
            Error condition (only returned in revents; ignored in events).
            This bit is also set for a file descriptor referring to the
            write end of a pipe when the read end has been closed.
            */
            continue;
        } else if (revents & POLL_CLOSE_EVENT) {
            if (ais_proxy_ctxt->is_running == 0) {
                QCARCAM_ERRORMSG("idx:%d, already streamoff, skip", ais_proxy_ctxt->idx);
                continue;
            }
            close_camera(ais_proxy_ctxt);
            QCARCAM_ERRORMSG("idx:%d close over", ais_proxy_ctxt->idx);
        } else {
            QCARCAM_ERRORMSG("\n idx:%d value of revent compare is fail =%d\n",
                                ais_proxy_ctxt->idx, revents);
            continue;
        }
    }
    close(fd);

    pthread_mutex_lock(&mutex_abort);
    if (!aborted)
    {
        pthread_cond_wait(&cond_abort, &mutex_abort);
    }
    pthread_mutex_unlock(&mutex_abort);

    if (enableDummyFramesInRecovery) {
        sem_destroy(&ais_proxy_ctxt->sem_recovery);
    }

    uninit_camera();
    QCARCAM_ERRORMSG("exit idx = %d", ais_proxy_ctxt->idx);
    return 0;
}

int main(int argc, char **argv)
{
    ais_log_init(NULL, (char *)"ais_v4l2_proxy");

    const char *tok;
    qcarcam_ret_t ret;

    int rval = EXIT_FAILURE;
    int i, rc;
    int input_idx;
    int sec = 0;
    int v4l2init = 0;

    void *signal_thread_handle;
    void *fps_thread_handle = NULL;
    void *signal_loss_thread_handle;
    void *csi_error_thread_handle;
    void *post_dummy_frames_thread_handle;
    char thread_name[64];


    QCARCAM_ERRORMSG("ais v4l2loopback source app started");
    QCARCAM_INFOMSG("Arg parse Begin");

    for (i = 1; i < argc; i++)
    {
        if (!strncmp(argv[i], "-config=", strlen("-config=")))
        {
            tok = argv[i] + strlen("-config=");
            snprintf(filename, sizeof(filename), "%s", tok);
        }
        else if (!strncmp(argv[i], "-dumpFrame=", strlen("-dumpFrame=")))
        {
            tok = argv[i] + strlen("-dumpFrame=");
            dumpFrame = atoi(tok);
        }
        else if (!strncmp(argv[i], "-fps", strlen("-fps")))
        {
            fps_print_delay = DEFAULT_PRINT_DELAY_SEC;
        }
        else if (!strncmp(argv[i], "-csiErrDetect", strlen("-csiErrDetect")))
        {
            /* only one of them enabled in real scenario */
            enableBridgeErrorDetect = 0;
            enableCSIErrorDetect = 1;
        }
        else
        {
            QCARCAM_ERRORMSG("Invalid argument");
            exit(-1);
        }
    }

    QCARCAM_INFOMSG("Arg parse End");

    sigset_t sigset;
    sigfillset(&sigset);
    for (i = 0; exceptsigs[i] != -1; i++)
    {
        sigdelset(&sigset, exceptsigs[i]);
    }
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    /*parse xml*/
    ais_proxy_xml_input_t *xml_inputs = (ais_proxy_xml_input_t *)calloc(NUM_MAX_CAMERAS, sizeof(ais_proxy_xml_input_t));
    if (!xml_inputs)
    {
        QCARCAM_ERRORMSG("Failed to allocate xml input struct");
        exit(-1);
    }
    QCARCAM_ERRORMSG("Filename is %s\n", filename);
    numInputs = parse_xml_config_file(filename, xml_inputs, NUM_MAX_CAMERAS);
    if (numInputs <= 0)
    {
	    QCARCAM_ERRORMSG("Failed to parse config file!");
	    exit(-1);
    }

    for (input_idx = 0; input_idx < numInputs; input_idx++)
    {
       ais_proxy_server_input_t* ais_proxy_ctxt = &g_ais_proxy_inputs[input_idx];
       ais_proxy_xml_input_t* p_xml_input = &xml_inputs[input_idx];
       ais_proxy_ctxt->qcarcam_input_id = xml_inputs->qcarcam_input_id;
       ais_proxy_ctxt->recovery = xml_inputs->recovery;
       ais_proxy_ctxt->v4l2_params.pixformat = xml_inputs->pixformat;
       ais_proxy_ctxt->v4l2_params.opmode = xml_inputs->opmode;
       ais_proxy_ctxt->v4l2_params.width = xml_inputs->width;
       ais_proxy_ctxt->v4l2_params.height = xml_inputs->height;
       ais_proxy_ctxt->v4l2_params.crop_top = xml_inputs->crop_top;
       ais_proxy_ctxt->v4l2_params.crop_left = xml_inputs->crop_left;
       ais_proxy_ctxt->v4l2_params.crop_bottom = xml_inputs->crop_bottom;
       ais_proxy_ctxt->v4l2_params.crop_right = xml_inputs->crop_right;
       ais_proxy_ctxt->v4l2_params.aligned_width = TESTUTIL_ALIGN(xml_inputs->width, SIZE_ALIGN);
       ais_proxy_ctxt->v4l2_params.aligned_height = TESTUTIL_ALIGN(xml_inputs->height, SIZE_ALIGN);
       ais_proxy_ctxt->v4l2_params.buffer_status = 0;
       ais_proxy_ctxt->state = AIS_PROXY_STREAMING;
       ais_proxy_ctxt->mutex_recovery = PTHREAD_MUTEX_INITIALIZER;
       strlcpy(ais_proxy_ctxt->v4l2_params.v4l2_node, p_xml_input->v4l2_node, sizeof(ais_proxy_ctxt->v4l2_params.v4l2_node));

    /* Initialize v4l2loopback driver */
    ret = qcarcam_init_v4l2device(&ais_proxy_ctxt->v4l2_params);

    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("qcarcam_init_v4l2device failed for node %s\n", ais_proxy_ctxt->v4l2_params.v4l2_node);
    }
    else
    {
        QCARCAM_ERRORMSG("qcarcam_init_v4l2device success for node %s", ais_proxy_ctxt->v4l2_params.v4l2_node);
        v4l2init=1;
    }

   }

    ais_proxy_server_input_t ais_proxy_dummy[numInputs];

    for (input_idx = 0; input_idx < numInputs; input_idx++)
    {
        ais_proxy_dummy[input_idx] = g_ais_proxy_inputs[input_idx];
        ais_proxy_xml_input_t* p_xml_input = &xml_inputs[input_idx];
        ais_proxy_dummy[input_idx].qcarcam_input_id = xml_inputs->qcarcam_input_id;
        ais_proxy_dummy[input_idx].recovery = xml_inputs->recovery;
        ais_proxy_dummy[input_idx].v4l2_params.pixformat = xml_inputs->pixformat;
        ais_proxy_dummy[input_idx].v4l2_params.opmode = xml_inputs->opmode;
        ais_proxy_dummy[input_idx].v4l2_params.width = xml_inputs->width;
        ais_proxy_dummy[input_idx].v4l2_params.height = xml_inputs->height;
        ais_proxy_dummy[input_idx].v4l2_params.crop_top = xml_inputs->crop_top;
        ais_proxy_dummy[input_idx].v4l2_params.crop_left = xml_inputs->crop_left;
        ais_proxy_dummy[input_idx].v4l2_params.crop_bottom = xml_inputs->crop_bottom;
        ais_proxy_dummy[input_idx].v4l2_params.crop_right = xml_inputs->crop_right;
        ais_proxy_dummy[input_idx].v4l2_params.aligned_width = TESTUTIL_ALIGN(xml_inputs->width, SIZE_ALIGN);
        ais_proxy_dummy[input_idx].v4l2_params.aligned_height = TESTUTIL_ALIGN(xml_inputs->height, SIZE_ALIGN);
        ais_proxy_dummy[input_idx].v4l2_params.buffer_status = 0;
        ais_proxy_dummy[input_idx].state = AIS_PROXY_STREAMING;
        ais_proxy_dummy[input_idx].mutex_recovery = PTHREAD_MUTEX_INITIALIZER;
        strlcpy(ais_proxy_dummy[input_idx].v4l2_params.v4l2_node, p_xml_input->v4l2_node, sizeof(ais_proxy_dummy[input_idx].v4l2_params.v4l2_node));

    }

    /*signal handler thread*/

#ifndef C2D_DISABLED
    if (enable_c2d) {
         QCARCAM_INFOMSG("C2D Setup Begin");

         /*init C2D*/
         C2D_DRIVER_SETUP_INFO set_driver_op = {
                 .max_surface_template_needed = (NUM_MAX_DISP_BUFS + QCARCAM_MAX_NUM_BUFFERS) * NUM_MAX_CAMERAS,
                 };
         C2D_STATUS c2d_status = c2dDriverInit(&set_driver_op);
         if (c2d_status != C2D_STATUS_OK)
         {
               QCARCAM_ERRORMSG("c2dDriverInit failed");
               goto fail;
         }
    }
#endif
    snprintf(thread_name, sizeof(thread_name), "signal_thrd");
    rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &signal_thread, 0, 0, thread_name, &signal_thread_handle);
    if (rc)
    {
        QCARCAM_ERRORMSG("pthread_create failed");
        goto fail;
    }

    if (enableCSIErrorDetect)
    {
        snprintf(thread_name, sizeof(thread_name), "check_csi_error_thread");
        rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &check_csi_error_thread, 0, 0x1000, thread_name, &csi_error_thread_handle);
        if (rc)
        {
            QCARCAM_ERRORMSG("pthread_create failed");
            goto fail;
        }
    }

    /*thread used frame rate measurement*/
    if (fps_print_delay)
    {
        snprintf(thread_name, sizeof(thread_name), "framerate_thrd");
        rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &framerate_thread, 0, 0, thread_name, &fps_thread_handle);
        if (rc)
        {
            QCARCAM_ERRORMSG("pthread_create failed");
            goto fail;
        }
    }

    if (enableDummyFramesInRecovery)
    {
        for (input_idx = 0; input_idx < numInputs; input_idx++)
        {
            sem_init(&g_ais_proxy_inputs[input_idx].sem_recovery, 0, 0);
            snprintf(thread_name, sizeof(thread_name), "post_dummy_frames_thread");
            rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &post_dummy_frames_thread, &g_ais_proxy_inputs[input_idx], 0, thread_name, &post_dummy_frames_thread_handle);
            if (rc)
            {
                QCARCAM_ERRORMSG("pthread_create failed");
                goto fail;
            }
        }
    }

    for (input_idx = 0; input_idx < numInputs; input_idx++)
    {
        ais_proxy_server_input_t* ais_proxy_ctxt = &g_ais_proxy_inputs[input_idx];
        ais_proxy_xml_input_t* p_xml_input = &xml_inputs[input_idx];
        ais_proxy_ctxt->qcarcam_input_id = p_xml_input->qcarcam_input_id;
        ais_proxy_ctxt->recovery = p_xml_input->recovery;
        ais_proxy_ctxt->v4l2_params.pixformat = p_xml_input->pixformat;
        ais_proxy_ctxt->v4l2_params.opmode = p_xml_input->opmode;
        ais_proxy_ctxt->v4l2_params.width = p_xml_input->width;
        ais_proxy_ctxt->v4l2_params.height = p_xml_input->height;
        ais_proxy_ctxt->v4l2_params.crop_top = p_xml_input->crop_top;
        ais_proxy_ctxt->v4l2_params.crop_left = p_xml_input->crop_left;
        ais_proxy_ctxt->v4l2_params.crop_bottom = p_xml_input->crop_bottom;
        ais_proxy_ctxt->v4l2_params.crop_right = p_xml_input->crop_right;
        ais_proxy_ctxt->v4l2_params.aligned_width = TESTUTIL_ALIGN(p_xml_input->width, SIZE_ALIGN);
        ais_proxy_ctxt->v4l2_params.aligned_height = TESTUTIL_ALIGN(p_xml_input->height, SIZE_ALIGN);
        ais_proxy_ctxt->v4l2_params.buffer_status = 0;
        ais_proxy_ctxt->state = AIS_PROXY_STREAMING;
        ais_proxy_ctxt->mutex_recovery = PTHREAD_MUTEX_INITIALIZER;
        strlcpy(ais_proxy_ctxt->v4l2_params.v4l2_node, p_xml_input->v4l2_node, sizeof(ais_proxy_ctxt->v4l2_params.v4l2_node));
        ais_proxy_ctxt->frame_timeout = DEFAULT_FRAME_TIMEOUT;

        if (ais_proxy_ctxt->v4l2_params.pixformat != V4L2_PIX_FMT_UYVY)
        {
            enable_c2d = 0; //make this 1 Idially use c2d for conversions
        }

        ais_proxy_ctxt->query_inputs_idx = input_idx ;

        /* Initialize v4l2loopback driver */
        if (!v4l2init)
        {
            ret = qcarcam_init_v4l2device(&ais_proxy_ctxt->v4l2_params);
            if (ret != QCARCAM_RET_OK)
            {
                QCARCAM_ERRORMSG("qcarcam_init_v4l2device failed for node %s\n", ais_proxy_ctxt->v4l2_params.v4l2_node);
                goto fail;
            }
            QCARCAM_ERRORMSG("qcarcam_init_v4l2device success for node %s", ais_proxy_ctxt->v4l2_params.v4l2_node);
            v4l2init=1;
        }

        /*@todo: move these buffer settings to be queried from qcarcam and not set externally*/
        ais_proxy_ctxt->buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].n_buffers = QCARCAM_MAX_RING_BUFFERS;
        ais_proxy_ctxt->buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].width = p_xml_input->width;
        ais_proxy_ctxt->buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].height = p_xml_input->height;
        if (ais_proxy_ctxt->v4l2_params.opmode == QCARCAM_OPMODE_SHDR)
        {
            ais_proxy_ctxt->buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].format = get_qcarcam_format_from_v4l2(p_xml_input->pixformat);
        }
        else
        {
            ais_proxy_ctxt->buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].format = QCARCAM_FMT_UYVY_8;
        }
	if (enable_c2d)
        {
            ais_proxy_ctxt->buffers_param[AIS_BUFFERS_C2D_OUTPUT].n_buffers = QCARCAM_MAX_RING_BUFFERS;
            ais_proxy_ctxt->buffers_param[AIS_BUFFERS_C2D_OUTPUT].width = ais_proxy_ctxt->buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].width;
            ais_proxy_ctxt->buffers_param[AIS_BUFFERS_C2D_OUTPUT].height = ais_proxy_ctxt->buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].height;
            ais_proxy_ctxt->buffers_param[AIS_BUFFERS_C2D_OUTPUT].format = get_qcarcam_format_from_v4l2(ais_proxy_ctxt->v4l2_params.pixformat);
        }

    }

    QCARCAM_INFOMSG("allocating qcarcam_buffers");

    for (input_idx = 0; input_idx < numInputs; input_idx++)
    {
        ais_proxy_server_input_t *ais_proxy_ctxt = &g_ais_proxy_inputs[input_idx];
        int output_n_buffers = 0;

        // Allocate an additional buffer to be shown in case of signal loss
        ais_proxy_ctxt->prev_buf_idx_qcarcam = -1;
        output_n_buffers = ais_proxy_ctxt->buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].n_buffers + 1;
        ais_proxy_ctxt->p_buffers_output.n_buffers = output_n_buffers;
        ais_proxy_ctxt->p_buffers_output.color_fmt = ais_proxy_ctxt->buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].format;
        ais_proxy_ctxt->p_buffers_output.buffers = (qcarcam_buffer_t *)calloc(ais_proxy_ctxt->p_buffers_output.n_buffers, sizeof(*ais_proxy_ctxt->p_buffers_output.buffers));
        if (ais_proxy_ctxt->p_buffers_output.buffers == 0)
        {
            QCARCAM_ERRORMSG("alloc qcarcam_buffer failed");
            goto fail;
        }

        for (i = 0; i < output_n_buffers; ++i)
        {
            ais_proxy_ctxt->p_buffers_output.buffers[i].n_planes = 1;
            ais_proxy_ctxt->p_buffers_output.buffers[i].planes[0].width = ais_proxy_ctxt->buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].width;
            ais_proxy_ctxt->p_buffers_output.buffers[i].planes[0].height = ais_proxy_ctxt->buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].height;
        }

        ret = qcarcam_init_ion_buffers(&ais_proxy_ctxt->direct_frame_buffer, &ais_proxy_ctxt->p_buffers_output);
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("qcarcam_init_ion_buffers failed for direct_frame_buffer");
            goto fail;
        }

        ais_proxy_ctxt->p_buffers_output.n_buffers = ais_proxy_ctxt->buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].n_buffers;

        if (enable_c2d)
        {
            for (i = 0; i < ais_proxy_ctxt->buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].n_buffers; ++i)
            {
                create_c2d_surface(ais_proxy_ctxt->direct_frame_buffer, i);
                if (ret != QCARCAM_RET_OK)
                {
                    QCARCAM_ERRORMSG("create_c2d_surface failed");
                    goto fail;
                }
            }

            /*c2d buffer*/
            ais_proxy_ctxt->p_buffers_c2d.n_buffers = ais_proxy_ctxt->buffers_param[AIS_BUFFERS_C2D_OUTPUT].n_buffers;
            ais_proxy_ctxt->p_buffers_c2d.color_fmt = ais_proxy_ctxt->buffers_param[AIS_BUFFERS_C2D_OUTPUT].format;
            ais_proxy_ctxt->p_buffers_c2d.buffers = (qcarcam_buffer_t *)calloc(ais_proxy_ctxt->p_buffers_c2d.n_buffers, sizeof(*ais_proxy_ctxt->p_buffers_c2d.buffers));
            if (ais_proxy_ctxt->p_buffers_c2d.buffers == 0)
            {
                QCARCAM_ERRORMSG("alloc qcarcam_buffer failed");
                goto fail;
            }

            for (i = 0; i < ais_proxy_ctxt->buffers_param[AIS_BUFFERS_C2D_OUTPUT].n_buffers; ++i)
            {
                ais_proxy_ctxt->p_buffers_c2d.buffers[i].n_planes = 1;
                ais_proxy_ctxt->p_buffers_c2d.buffers[i].planes[0].width = ais_proxy_ctxt->buffers_param[AIS_BUFFERS_C2D_OUTPUT].width;
                ais_proxy_ctxt->p_buffers_c2d.buffers[i].planes[0].height = ais_proxy_ctxt->buffers_param[AIS_BUFFERS_C2D_OUTPUT].height;
            }

            ret = qcarcam_init_ion_buffers(&ais_proxy_ctxt->c2d_frame_buffer, &ais_proxy_ctxt->p_buffers_c2d);
            if (ret != QCARCAM_RET_OK)
            {
                QCARCAM_ERRORMSG("qcarcam_init_ion_buffers failed");
                goto fail;
            }

            for (i = 0; i < ais_proxy_ctxt->buffers_param[AIS_BUFFERS_C2D_OUTPUT].n_buffers; ++i)
            {
                create_c2d_surface(ais_proxy_ctxt->c2d_frame_buffer, i);
                if (ret != QCARCAM_RET_OK)
                {
                    QCARCAM_ERRORMSG("create_c2d_surface failed");
                    goto fail;
                }
            }
        }
    }

    QCARCAM_INFOMSG("Create qcarcam_context Begin");

    for (input_idx = 0; input_idx < numInputs; input_idx++)
    {
        g_ais_proxy_inputs[input_idx].idx = input_idx;
        g_ais_proxy_inputs[input_idx].v4l2_params.opmode = xml_inputs[input_idx].opmode;
        snprintf(thread_name, sizeof(thread_name), "inpt_ctxt_%d", input_idx);

        rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &ais_v4l2_poll_thread, &g_ais_proxy_inputs[input_idx], 0, thread_name, &g_ais_proxy_inputs[input_idx].thread_handle);
        if (rc)
        {
            QCARCAM_ERRORMSG("pthread_create failed");
            goto fail;
        }
    }

    /*Wait on all of them to join*/
    for (input_idx = 0; input_idx < numInputs; input_idx++)
    {
        int* thread_ret = NULL;
        CameraJoinThread(g_ais_proxy_inputs[input_idx].thread_handle, thread_ret);
    }

fail:
    // cleanup
    for (input_idx = 0; input_idx < numInputs; input_idx++)
    {
        if (g_ais_proxy_inputs[input_idx].qcarcam_context != NULL)
        {
            (void)qcarcam_stop(g_ais_proxy_inputs[input_idx].qcarcam_context);
            (void)qcarcam_close(g_ais_proxy_inputs[input_idx].qcarcam_context);
            g_ais_proxy_inputs[input_idx].qcarcam_context = NULL;
        }

        qcarcam_deinit_ion_buffers(g_ais_proxy_inputs[input_idx].direct_frame_buffer);

        if (enable_c2d)
        {
            qcarcam_deinit_ion_buffers(g_ais_proxy_inputs[input_idx].c2d_frame_buffer);
        }
        if (g_ais_proxy_inputs[input_idx].p_buffers_output.buffers)
        {
            free(g_ais_proxy_inputs[input_idx].p_buffers_output.buffers);
            g_ais_proxy_inputs[input_idx].p_buffers_output.buffers = NULL;
        }

        if (g_ais_proxy_inputs[input_idx].p_buffers_c2d.buffers)
        {
            free(g_ais_proxy_inputs[input_idx].p_buffers_c2d.buffers);
            g_ais_proxy_inputs[input_idx].p_buffers_c2d.buffers = NULL;
        }
        memset(&g_ais_proxy_inputs[input_idx], 0x0, sizeof(g_ais_proxy_inputs[input_idx]));

        if (enableDummyFramesInRecovery) {
            sem_post(&g_ais_proxy_inputs[input_idx].sem_recovery);
            sem_destroy(&g_ais_proxy_inputs[input_idx].sem_recovery);
        }
    }

#ifndef __AGL__
    CameraReleaseThread(signal_thread_handle);
    if (fps_thread_handle != NULL)
        CameraReleaseThread(fps_thread_handle);
#endif


    if (xml_inputs)
    {
        free(xml_inputs);
        xml_inputs = NULL;
    }

    ais_log_uninit();

    return rval;
}

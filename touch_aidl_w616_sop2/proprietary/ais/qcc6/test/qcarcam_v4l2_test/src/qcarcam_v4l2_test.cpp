/* ===========================================================================
 * Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 =========================================================================== */

#include "V4L2Wrapper.h"
#include "qcarcam_types.h"
#include "ais_log.h"
#include "CameraOSServices.h"
#include "test_util.h"

#include <poll.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/select.h>
#include <vector>
#include <fcntl.h>

#include "InputStream.h"

using namespace ais_v4l2_test;

//#define QCARCAM_THRD_PRIO CAMERA_THREAD_PRIO_DEFAULT
#define NUM_MAX_CAMERAS 16
#define SIGWAIT_TIMEOUT_MS 100
#define FPS_PRINT_DELAY 10

using std::vector;

typedef struct
{
    int numInputs;
    vector<InputStream*> inputs;
    int opened_stream_cnt;

    int dumpFrame;

    int disable_display;

    V4L2Wrapper::IO_MODE io_mode;

    int ctrl_interval;

    int exitSeconds;
    CameraSignal abort_signal;

    bool printfps;
    void* fps_thread_handle;
} qcarcam_test_ctxt_t;

static CameraThread signal_thread_handle = NULL;
static sigset_t g_sigset;
static int g_aborted = 0;
static char g_filename[128] = "qcarcam_config.xml";
static qcarcam_test_ctxt_t g_ctxt = {};
test_cfg_t g_test_cfg = {};
test_util_global_config_t g_xml_cfg = {};

static void abort_test()
{
    g_aborted = 1;
    CameraSetSignal(g_ctxt.abort_signal);
}

static int signal_thread(void *arg)
{
    struct timespec timeout;

    pthread_detach(pthread_self());

    timeout.tv_sec = 0;
    timeout.tv_nsec = SIGWAIT_TIMEOUT_MS * 1000000;

    while (!g_aborted)
    {
        if (sigtimedwait(&g_sigset, NULL, &timeout) > 0)
        {
            abort_test();
            break;
        }
    }
    return 0;
}

static int create_signal_thread()
{
    int rc = 0;

    const int exceptsigs[] = {
        SIGCHLD, SIGIO, SIGURG, SIGWINCH,
        SIGTTIN, SIGTTOU, SIGCONT, SIGSEGV,
        -1,
    };

    /* Should be placed before any forked thread */
    sigfillset(&g_sigset);
    for (uint32 i = 0; exceptsigs[i] != -1; i++)
    {
        sigdelset(&g_sigset, exceptsigs[i]);
    }
    pthread_sigmask(SIG_BLOCK, &g_sigset, NULL);

    /*signal handler thread*/
    rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &signal_thread, 0, 0, "signal_thrd", &signal_thread_handle);
    if (rc)
    {
        CAM_MSG(ERROR, "CameraCreateThread failed : signal_thread");
        return 1;
    }

    return 0;
}

static int framerate_thread(void *arg)
{
    pthread_detach(pthread_self());

    unsigned int fps_print_delay_us = FPS_PRINT_DELAY * 1000000;
    unsigned long time1, time2;
    get_system_time(time1);

    while (!g_aborted)
    {
        get_system_time(time2);
        for (uint32 input_idx = 0; input_idx < g_ctxt.inputs.size(); input_idx++)
        {
            InputStream *input_ctxt = g_ctxt.inputs[input_idx];
            if (input_ctxt)
            {
                input_ctxt->printfps(time1, time2);
            }
        }
        fflush(stdout);
        //get_system_time(time1);
        time1 = time2;
        usleep(fps_print_delay_us);
    }
    return 0;
}


static void process_ctrl()
{
    // need to sleep 4s for the input thread ready
    usleep(4000000);
    while (!g_aborted)
    {
        for (uint32 input_idx = 0; input_idx < g_ctxt.numInputs; input_idx++)
        {
            InputStream *input_ctxt = g_ctxt.inputs[input_idx];
            input_ctxt->set_ctrl(InputStream::TEST_VENDOR_EXTENSION);
            input_ctxt->set_ctrl(InputStream::TEST_LATENCY_1);
        }

        usleep(g_ctxt.ctrl_interval);
    }
}

int main(int argc, char **argv)
{
    ais_log_init(NULL, (char *)"qcarcam_v4l2_test");

    test_util_ctxt_t* test_util_ctxt = NULL;
    test_util_ctxt_params_t test_util_ctxt_params = {};
    char *tok = NULL;
    int ret = 0;

    g_ctxt.ctrl_interval = 10000000;  // 10s
    g_ctxt.printfps = true;
    g_test_cfg.test_mode = InputStream::TEST_MODE_PREVIEW;
    g_ctxt.io_mode = V4L2Wrapper::IO_MODE_DMABUF;
    g_test_cfg._use_buf_width = false;
    for (int i = 1; i < argc; i++)
    {
        if (!strncmp(argv[i], "-config=", strlen("-config=")))
        {
            tok = argv[i] + strlen("-config=");
            snprintf(g_filename, sizeof(g_filename), "%s", tok);
        }
        else if (!strncmp(argv[i], "-dumpFrame=", strlen("-dumpFrame=")))
        {
            tok = argv[i] + strlen("-dumpFrame=");
            g_test_cfg._dumpFrame = atoi(tok);
        }
        else if (!strncmp(argv[i], "-noDisplay", strlen("-noDisplay")))
        {
            g_test_cfg._disable_display = 1;
            test_util_ctxt_params.disable_display = 1;
        }
        else if (!strncmp(argv[i], "-ctrl_interval=", strlen("-ctrl_interval=")))
        {
            tok = argv[i] + strlen("-ctrl_interval=");
            g_ctxt.ctrl_interval = atoi(tok);
        }
        else if (!strncmp(argv[i], "-startStop=", strlen("-startStop=")))
        {
            tok = argv[i] + strlen("-startStop=");
            g_test_cfg.test_mode |= InputStream::TEST_MODE_STARTSTOP;
            g_test_cfg.ss_interval_frame_num = atoi(tok);
            CAM_MSG(HIGH, "=====set startstop=============\n");
        }
        else if (!strncmp(argv[i], "-test_params", strlen("-test_params")))
        {
            g_test_cfg.test_mode |= InputStream::TEST_MODE_PARAMS;
        }
        else if (!strncmp(argv[i], "-use_buf_width", strlen("-use_buf_width")))
        {
            g_test_cfg._use_buf_width = true;
        }
        else if(!strncmp(argv[i], "-negative", strlen("-negative")))
        {
            g_test_cfg.test_mode = InputStream::TEST_MODE_NEGATIVE;
            g_ctxt.printfps = false;
        }

    }

    /* =========================init part1: common part ============*/
    if (create_signal_thread())
    {
        CAM_MSG(ERROR, "fail to create signal thread");
        return 1;
    }

    ret = CameraCreateSignal(&g_ctxt.abort_signal);
    if (ret != CAMERA_SUCCESS)
    {
        QCARCAM_ERRORMSG("CameraCreateSignal abort_signal failed");
        exit(-1);
    }

    /*thread used frame rate measurement*/
    if (g_ctxt.printfps)
    {
        ret = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &framerate_thread, 0, 0,
            "framerate_thrd", &g_ctxt.fps_thread_handle);
        if (ret)
        {
            CAM_MSG(ERROR, "pthread_create failed");
            exit(-1);
        }
    }

    QCARCAM_DBGMSG("test_util_init");
    ret = test_util_init(&test_util_ctxt, &test_util_ctxt_params);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("test_util_init failed");
        exit(-1);
    }

    /*parse xml*/
    test_util_xml_input_t *xml_inputs = (test_util_xml_input_t *)calloc(NUM_MAX_CAMERAS, sizeof(test_util_xml_input_t));
    if (!xml_inputs)
    {
        QCARCAM_ERRORMSG("Failed to allocate xml input struct");
        exit(-1);
    }

    g_ctxt.numInputs = test_util_parse_xml_config_file_v2(g_filename, xml_inputs, NUM_MAX_CAMERAS, &g_xml_cfg);
    if (g_ctxt.numInputs <= 0)
    {
        QCARCAM_ERRORMSG("Failed to parse config file!");
        exit(-1);
    }

    if (g_xml_cfg.latency_measurement_mode == CAMERA_LM_MODE_ALL_STEPS)
    {
        CameraEnableMPMTimer();
    }


    /*===============init part2 ===============================*/
    for (uint32 input_idx = 0; input_idx < g_ctxt.numInputs; input_idx++)
    {
        InputStream* input_ctxt = new InputStream(input_idx);
        g_ctxt.inputs.push_back(input_ctxt);

        if (input_ctxt->init(test_util_ctxt, xml_inputs))
        {
            QCARCAM_ERRORMSG("Failed to init input %d", input_idx);
        }
    }

    if (g_test_cfg.test_mode & InputStream::TEST_MODE_PARAMS)
        process_ctrl();


    /*===============release part ===============================*/
    CameraWaitOnSignal(g_ctxt.abort_signal, -1);

    QCARCAM_ALWZMSG("Exit qcarcam_v4l2_test");

    for (uint32 input_idx = 0; input_idx < g_ctxt.numInputs; input_idx++)
    {
        InputStream *input_ctxt = g_ctxt.inputs[input_idx];
        input_ctxt->uninit();
        delete input_ctxt;
    }

    test_util_deinit(test_util_ctxt);

    ais_log_uninit();

    return 0;
}


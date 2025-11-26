/* ===========================================================================
 * Copyright (c) 2019-2023 Qualcomm Technologies, Inc.
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
#include <linux/videodev2.h>
#include <poll.h>
#include <media/ais_v4l2loopback.h>
#include <cutils/properties.h>

#include "qcarcam.h"
#include "ais_v4l2_proxy_util.h"
#include "ais_v4l2_proxy_input.h"
#include "ais_v4l2_proxy_stream.h"


#define AIS_V4L2_PROXY_MAX_VERSION          3
#define AIS_V4L2_PROXY_MIN_VERSION          0
#define AIS_V4L2_PROXY_BUGFIX_VERSION       0

///////////////////////////////
/// STATICS
///////////////////////////////
static CAisV4l2ProxyInput* g_ais_proxy_inputs[NUM_MAX_CAMERAS] = {};
static int numInputs = 0;
static qcarcam_input_t *g_pQcarcam_Inputs = NULL;
ais_v4l2_cfg_t g_v4l2_cfg = {};

#ifdef __ANDROID__
static char filename[128] = "/vendor/bin/ais_v4l2loopback_config.xml";
#elif defined(__AGL__)
#if defined(AIS_DATADIR_ENABLE)
static char filename[128] = "/usr/share/camera/ais_v4l2loopback_config.xml";
#else
static char filename[128] = "/data/misc/camera/ais_v4l2loopback_config.xml";
#endif
#endif

static int fps_print_delay = 0;

/*abort condition*/
static pthread_mutex_t mutex_abort = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_abort = PTHREAD_COND_INITIALIZER;

static const int exceptsigs[] = {
    SIGCHLD, SIGIO, SIGURG, SIGWINCH,
    SIGSTOP, SIGTSTP, SIGTTIN, SIGTTOU, SIGCONT,
    -1,
};

#ifdef QCC_SOC_OVERRIDE

static bool isMakena()
{
    int     socFd = 0;
    char    buf[32]  = { 0 };
    int     chipsetVersion = -1;
    char    fileName[] = "/sys/devices/soc0/soc_id";

    socFd = open(fileName, O_RDONLY);

    if (socFd > 0)
    {
        int ret = read(socFd, buf, sizeof(buf) - 1);

        if (-1 == ret)
            return false;
        else
            chipsetVersion = atoi(buf);

        close(socFd);

        if (chipsetVersion == 460) //CHIP_ID_SA8295P = 460
            return true;
    }

    return false;
}

#endif

ais_proxy_gconfig ais_v4l2_xml_cfg = {};

int qcarcam_get_system_time(unsigned long *pTime)
{
    struct timespec time;
    unsigned long msec;

    if (clock_gettime(CLOCK_MONOTONIC, &time) == -1)
    {
        VPROXY_ERROR("Clock gettime failed");
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

    //todo : calculate the FPS in the proxy side for each stream

    fflush(stdout);
    qcarcam_get_system_time(timer1);
}

static int framerate_thread(void *arg)
{
    pthread_detach(pthread_self());

    unsigned int fps_print_delay_us = fps_print_delay * 1000000;
    unsigned long timer1;
    qcarcam_get_system_time(&timer1);

    while (!g_v4l2_cfg.aborted)
    {
        qcarcam_calculate_frame_rate(&timer1);
        usleep(fps_print_delay_us);
    }
    return 0;
}

static void abort_ais_proxy_server(void)
{
    int tmp = 0;
    VPROXY_INFO("Aborting ais_proxy server, client %d", numInputs);
    pthread_mutex_lock(&mutex_abort);
    g_v4l2_cfg.aborted = 1;
    pthread_cond_broadcast(&cond_abort);
    pthread_mutex_unlock(&mutex_abort);

    CAisV4l2ProxyInput::exit_v4l2_poll();
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

static int checkversion()
{
    VPROXY_INFO("umd version %u %u ; kmd version %u %u",
        AIS_V4L2_PROXY_MAX_VERSION, AIS_V4L2_PROXY_MIN_VERSION,
        AIS_V4L2_DRV_MAX_VERSION, AIS_V4L2_DRV_MIN_VERSION);
    if (AIS_V4L2_PROXY_MAX_VERSION != AIS_V4L2_DRV_MAX_VERSION)
    {
        VPROXY_ERROR("MAX_VERSION mismatch %u %u", AIS_V4L2_PROXY_MAX_VERSION, AIS_V4L2_DRV_MAX_VERSION);
        return 1;
    }

    if (AIS_V4L2_PROXY_MIN_VERSION != AIS_V4L2_DRV_MIN_VERSION)
    {
        VPROXY_ERROR("MIN_VERSION mismatch %u %u", AIS_V4L2_PROXY_MIN_VERSION, AIS_V4L2_DRV_MIN_VERSION);
        return 1;
    }

    if (AIS_V4L2_PROXY_BUGFIX_VERSION != AIS_V4L2_DRV_BUGFIX_VERSION)
    {
        VPROXY_ERROR("BUGFIX_VERSION mismatch %u %u", AIS_V4L2_PROXY_BUGFIX_VERSION, AIS_V4L2_DRV_BUGFIX_VERSION);
        return 1;
    }

    return 0;
}

static int query_inputs(qcarcam_input_t **ppInputs, uint32 &queryFilled)
{
    int rc = 0;
    /*query qcarcam*/
    uint32 queryNumInputs = 0;
    qcarcam_input_t *pInputs;
    uint32 max_query_num = 100;
    uint32 i = 0;

    while (i++ < max_query_num)
    {
        rc = qcarcam_query_inputs(NULL, 0, &queryNumInputs);
        if (QCARCAM_RET_BUSY != rc)
        {
            VPROXY_WARN("success qcarcam_query_inputs number of inputs with ret %d i= %d", rc, i);
            break;
        }
        CameraSleep(200);
    }

    if (QCARCAM_RET_OK != rc || queryNumInputs == 0)
    {
        VPROXY_WARN("Failed qcarcam_query_inputs number of inputs with ret %d queryNumInputs %d", rc, queryNumInputs);
        *ppInputs = NULL;
        queryFilled = 0;
        return rc;
    }

    pInputs = (qcarcam_input_t *)calloc(queryNumInputs, sizeof(*pInputs));
    if (!pInputs)
    {
        VPROXY_ERROR("Failed to allocate pInputs");
        return 1;
    }

    rc = qcarcam_query_inputs(pInputs, queryNumInputs, &queryFilled);
    if (QCARCAM_RET_OK != rc || queryFilled != queryNumInputs)
    {
        VPROXY_ERROR("Failed qcarcam_query_inputs with ret %d %d %d", rc, queryFilled, queryNumInputs);
        return 1;
    }

    VPROXY_DBG1("--- QCarCam Queried Inputs ----");
    for (uint32 i = 0; i < queryFilled; i++)
    {
        VPROXY_INFO("%d: input_id=%d, res=%dx%d fmt=0x%08x fps=%.2f flags=0x%x",
            i, pInputs[i].desc,
            pInputs[i].res[0].width, pInputs[i].res[0].height,
            pInputs[i].color_fmt[0], pInputs[i].res[0].fps, pInputs[i].flags);
    }

    VPROXY_DBG1("-------------------------------");

    if (ppInputs)
    {
        *ppInputs = pInputs;
    }

    return 0;

}

static int init_v4l2_devices(ais_proxy_xml_input_t *xml_inputs, uint32 queryFilled)
{
    int rc = 0;
    VPROXY_INFO("begin to init_v4l2_devices");

    for (uint32 input_idx = 0; input_idx < numInputs; input_idx++)
    {
        uint32 query_inputs_idx = -1;
        ais_proxy_xml_input_t* p_xml_input = &xml_inputs[input_idx];
        qcarcam_input_t* p_qcarcam_input = NULL;

        g_ais_proxy_inputs[input_idx] = new CAisV4l2ProxyInput();

        VPROXY_INFO("begin to init_v4l2_device %u", input_idx);

        for (uint32 i = 0; i < (int)queryFilled; i++)
        {
           if (g_pQcarcam_Inputs[i].desc == p_xml_input->qcarcam_input_id)
           {
               query_inputs_idx = i;
               p_qcarcam_input = &g_pQcarcam_Inputs[i];
               VPROXY_INFO("input id %d query_inputs_idx %d", p_xml_input->qcarcam_input_id, i);
               break;
           }
        }

        //ais_proxy_ctxt->is_buf_align = true;
        // todo: if init fail, what should do?
        g_ais_proxy_inputs[input_idx]->init(p_xml_input, p_qcarcam_input);

        CAisV4l2ProxyInput::add_input(g_ais_proxy_inputs[input_idx]);
    }

    return rc;
}

static void deinit_v4l2_devices()
{
    for (uint32 input_idx = 0; input_idx < numInputs; input_idx++)
    {
        if (g_ais_proxy_inputs[input_idx])
        {
            g_ais_proxy_inputs[input_idx]->uninit();
            delete g_ais_proxy_inputs[input_idx];
            g_ais_proxy_inputs[input_idx] = NULL;
        }
    }
}


int main(int argc, char **argv)
{
    ais_log_init(NULL, (char *)"ais_v4l2_proxy");

    const char *tok;
    qcarcam_ret_t ret;

    int rval = EXIT_FAILURE;
    int i, rc;
    int sec = 0;

    void *signal_thread_handle = NULL;
    void *fps_thread_handle = NULL;
    char thread_name[64];
    uint32 queryFilled = 0;
    ais_proxy_xml_input_t *xml_inputs = NULL;

    char value[92];
    int QCC_FLAG_ENABLED = 0;
    property_get("ro.boot.camera.qcc.version", value, "");
    VPROXY_ERROR("value of property for qcc: %s", value);
    QCC_FLAG_ENABLED = (strlen(value) > 0) ? 1:0;

    if (QCC_FLAG_ENABLED) {
        if ((value[0] - '0') != QCARCAM_VERSION_MAJOR) {
            VPROXY_ERROR("Blocking ais_v4l2_proxy service for QCC.4.0");
            pthread_cond_wait(&cond_abort, &mutex_abort);
            VPROXY_ERROR("Exiting ais_v4l2_proxy service...");
            return 1;
        }
    }
    else {
#ifdef QCC_SOC_OVERRIDE
    if (isMakena()) {
        VPROXY_ERROR("Blocking ais_v4l2_proxy service for QCC.4.0");
        pthread_cond_wait(&cond_abort, &mutex_abort);
        VPROXY_ERROR("Exiting ais_v4l2_proxy service...");
        return 1;
    }
#endif
    }
    VPROXY_ERROR("ais v4l2loopback source (QCC.4.0) app started");
    VPROXY_DBG1("Arg parse Begin");

    if (checkversion())
    {
        VPROXY_ERROR("version mismatch");
        return 1;
    }

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
            g_v4l2_cfg.dumpFrame = atoi(tok);
        }
        else if (!strncmp(argv[i], "-fps", strlen("-fps")))
        {
            fps_print_delay = DEFAULT_PRINT_DELAY_SEC;
        }
        else
        {
            VPROXY_ERROR("Invalid argument");
            exit(-1);
        }
    }

    VPROXY_DBG1("Arg parse End");

    sigset_t sigset;
    sigfillset(&sigset);
    int ready_status = 0;
    const std::string ready_property_name = "vendor.ais.v4l2.proxy.ready";
    for (i = 0; exceptsigs[i] != -1; i++)
    {
        sigdelset(&sigset, exceptsigs[i]);
    }
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    /*parse xml*/
    xml_inputs = (ais_proxy_xml_input_t *)calloc(NUM_MAX_CAMERAS, sizeof(ais_proxy_xml_input_t));
    if (!xml_inputs)
    {
        VPROXY_ERROR("Failed to allocate xml input struct");
        exit(-1);
    }

    VPROXY_ERROR("Filename is %s\n", filename);
    numInputs = parse_xml_config_file(filename, xml_inputs, NUM_MAX_CAMERAS, &ais_v4l2_xml_cfg);
    if (numInputs <= 0)
    {
        VPROXY_ERROR("Failed to parse config file!");
        goto EXIT_FLAG;
    }

    if (ais_v4l2_xml_cfg.latency_measurement_mode == CAMERA_LM_MODE_ALL_STEPS)
    {
        CameraEnableMPMTimer();
    }

    /*signal handler thread*/
    snprintf(thread_name, sizeof(thread_name), "signal_thrd");
    rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &signal_thread, 0, 0, thread_name, &signal_thread_handle);
    if (rc)
    {
        VPROXY_ERROR("pthread_create failed");
        goto EXIT_FLAG;
    }

    /*thread used frame rate measurement*/
    if (fps_print_delay)
    {
        snprintf(thread_name, sizeof(thread_name), "framerate_thrd");
        rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &framerate_thread, 0, 0, thread_name, &fps_thread_handle);
        if (rc)
        {
            VPROXY_ERROR("pthread_create failed");
            goto EXIT_FLAG;
        }
    }

    ret = QcarcamInitialize();
    if (ret != QCARCAM_RET_OK)
    {
        VPROXY_ERROR("qcarcam_initialize failed %d", ret);
#ifdef __ANDROID__
        //If we exit, init will launch again this service which will cause LPM
        //issues, so wait till signal trigger
        pthread_cond_wait(&cond_abort, &mutex_abort);
#endif
        goto EXIT_FLAG;
    }

    rc = query_inputs(&g_pQcarcam_Inputs, queryFilled);
    if (rc != 0)
    {
        VPROXY_ERROR("query_inputs failed %d", rc);
        goto EXIT_FLAG1;
    }

    // Uninitialize qcarcam for LPM
    QcarcamUninitialize();

    // init_v4l2_Devices
    init_v4l2_devices(xml_inputs, queryFilled);
    ready_status = 1;
    rc = property_set(ready_property_name.c_str(),std::to_string(ready_status).c_str());
    if (rc != 0)
    {
       VPROXY_ERROR("Not able to set vendor.ais.v4l2.proxy.ready to %d. property_set failed %d.", ready_status, rc);
       deinit_v4l2_devices();
       goto EXIT_FLAG;
    }

    // wait_events
    CAisV4l2ProxyInput::poll_wait();

    deinit_v4l2_devices();
    ready_status = 0;
    rc = property_set(ready_property_name.c_str(),std::to_string(ready_status).c_str());
    if (rc != 0)
    {
       VPROXY_ERROR("Not able to set vendor.ais.v4l2.proxy.ready to %d. property_set failed %d.", ready_status, rc);
       goto EXIT_FLAG;
    }

EXIT_FLAG1:
    QcarcamUninitialize();

EXIT_FLAG:
    // cleanup
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

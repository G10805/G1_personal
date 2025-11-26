/* ===========================================================================
 * Copyright (c) 2017-2023 Qualcomm Technologies, Inc.
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
#include <queue>

#ifndef C2D_DISABLED
#include "c2d2.h"
#endif

#ifdef USE_VENDOR_EXT_PARAMS
#include "vendor_ext_properties.h"
#endif

#include "qcarcam.h"

#include "test_util.h"
#include "qcarcam_diag_types.h"
#ifdef POST_PROCESS
#include "post_process.h"
#endif

#define NUM_MAX_CAMERAS 28
#define NUM_MAX_DISP_BUFS 3

/*1sec delay before restart */
#define PAUSE_RESUME_USLEEP 1000000
#define START_STOP_USLEEP 1000000

/*print input state as frozen if start and no frames after 1 sec*/
#define QCARCAMTEST_SOF_FREEZE_TIMEOUT 1.0f

#define QCARCAM_TEST_DEFAULT_GET_FRAME_TIMEOUT 500000000
#define DEFAULT_PRINT_DELAY_SEC 10
#define SIGNAL_CHECK_DELAY 33333;
#define CSI_ERR_CHECK_DELAY 100000;
#define NS_TO_MS 0.000001F

#define QCARCAM_TEST_INPUT_INJECTION 11
#define BUFSIZE 10

#define SIGWAIT_TIMEOUT_MS 100
#define TIMER_THREAD_USLEEP 1000000

#define USR_PROCESS_THREAD_USLEEP 1000
#define USR_PROCESS_WAIT_USLEEP 1000

#if defined(__INTEGRITY)
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC CLOCK_REALTIME
#endif
extern "C" const Value __PosixServerPriority = CAM_POSIXSERVER_PRIORITY;
#endif

#define SET_BIT(num, nbit)   ((num) |=  (0x1<<(nbit)))
#define CHECK_BIT(num, nbit) ((num) & (0x1<<(nbit)))

#define QCARCAMTEST_BUFFER_STALE_BIT  4

typedef enum
{
    QCARCAMTEST_BUFFER_STATE_INIT,
    QCARCAMTEST_BUFFER_STATE_QCARCAM,
    QCARCAMTEST_BUFFER_STATE_GET_FRAME,
    QCARCAMTEST_BUFFER_STATE_USR_PROCESS,
    QCARCAMTEST_BUFFER_STATE_USR_PROCESS_DONE,
    QCARCAMTEST_BUFFER_STATE_POST_DISPLAY,
    QCARCAMTEST_BUFFER_STATE_MAX = 0x7FFFFFF
}qcarcam_test_buffer_state_t;

typedef enum
{
    QCARCAM_TEST_BUFFERS_OUTPUT = 0,
    QCARCAM_TEST_BUFFERS_DISPLAY,
    QCARCAM_TEST_BUFFERS_INPUT,
    QCARCAM_TEST_BUFFERS_MAX
} qcarcam_test_buffers_t;

typedef struct
{
    QCarCamBufferList_t p_buffers;

    QCarCamColorFmt_e format;
    unsigned int n_buffers;
    unsigned int width;
    unsigned int height;
} qcarcam_buffers_param_t;

typedef enum
{
    QCARCAMTEST_STATE_INVALID = 0,
    QCARCAMTEST_STATE_INIT,
    QCARCAMTEST_STATE_OPEN,
    QCARCAMTEST_STATE_START,
    QCARCAMTEST_STATE_STOP,
    QCARCAMTEST_STATE_PAUSE,
    QCARCAMTEST_STATE_PAUSE_STOP_PENDING,
    QCARCAMTEST_STATE_ERROR,
    QCARCAMTEST_STATE_CLOSED,
    QCARCAMTEST_STATE_SUSPEND,
    QCARCAMTEST_STATE_RECOVERY,
}qcarcamtest_state_t;

typedef struct
{
    uint32 buf_idx;
    void *p_data;
}timer_usr_data;

typedef struct
{
    timer_usr_data usr_data;
    CameraTimer ptimer;
}qcarcam_test_buf_timer;


typedef struct
{
    uint32_t event_id;
    QCarCamEventPayload_t payload;
} qcarcam_event_msg_t;

typedef struct
{
    int buf_idx;            ///< Buffer index for injection
    int n_total_frames;     ///< Total number of frames for injection
    int n_dump_frames;      ///< Number of processed frames to dump
    uint32_t request_id;  ///< Request ID for next submit request
} qcarcam_test_injection_settings_t;

typedef struct
{
    uint32_t request_id;  ///< Request ID for next submit request
} qcarcam_test_submitrequest_settings_t;


typedef struct
{
    QCarCamInput_t*     pInputDesc;
    QCarCamInputModes_t modesDesc;
}QCarCamTestInputEnumerator_t;

typedef struct
{
    CameraThread thread_handle;
    CameraThread process_cb_event_handle;
    CameraSignal m_eventHandlerSignal;
    CameraThread injection_handle;
    CameraSignal m_injectionHandlerSignal;

    unsigned int idx;
    unsigned int query_inputs_idx;

    pthread_mutex_t mutex;
    qcarcamtest_state_t state;
    bool is_fatal_error;

    /*qcarcam context*/
    QCarCamHndl_t qcarcam_hndl;
    uint32_t qcarcam_input_id;

    qcarcam_buffers_param_t buffers_param[QCARCAM_TEST_BUFFERS_MAX];

    QCarCamBufferList_t p_buffers_output;
    QCarCamBufferList_t p_buffers_disp;
    QCarCamBufferList_t p_buffers_input;

    unsigned long long int frame_timeout;
    int use_event_callback;
    QCarCamOpmode_e op_mode;

    unsigned int num_isp_instances;
    QCarCamIspUsecaseConfig_t isp_config[QCARCAM_MAX_ISP_INSTANCES];

    bool recovery;
    bool request_mode;

    /* test util objects */
    test_util_ctxt_t *test_util_ctxt;
    test_util_window_t *qcarcam_window;
    test_util_window_t *display_window;
    test_util_window_t *injection_window;
    test_util_window_param_t window_params;

    /* buffer management tracking */
    int get_frame_buf_idx;
    int buf_idx_qcarcam;
    std::list<uint32> release_buf_idx;

    int buf_idx_disp;
    int prev_buf_idx_disp;

    /* diag */
    int frameCnt;
    int releaseframeCnt;
    int prev_frameCnt;
    int prev_releaseframeCnt;
    bool is_first_start;
    bool is_injection;
    test_util_injection_param_t injectionParams;
    qcarcam_test_injection_settings_t injectionSettings;
    qcarcam_test_submitrequest_settings_t submitrequestSettings;
    unsigned long long t_start; //start command
    unsigned long long t_start_success;
    unsigned long long t_firstFrame; //first frame time
    unsigned long long t_before;
    unsigned long long t_after;
    test_util_diag_t diag;
    bool dumpNextFrame;

    /* Exposure values */
    float exp_time;
    float gain;
    int manual_exposure;

    /* frame rate parameters */
    QCarCamFrameDropConfig_t frameDropConfig;
    unsigned int num_batch_frames;
    unsigned int detect_first_phase_timer;
    unsigned int frame_increment;

    qcarcam_test_buffer_state_t buf_state[QCARCAM_MAX_NUM_BUFFERS];

    bool skip_post_display;
    QCarCamInterlaceField_e field_type_previous;

    bool signal_lost;
    int fatal_err_cnt;

#ifdef ENABLE_CL_CONVERTER
    void* g_converter = NULL;
    ClConverter_surface_t source_surface;
    ClConverter_surface_t target_surface;
#endif

    /* subscription for changed setting events notification */
    uint64 subscribe_parameter_change;
    bool is_master;

    /* user process */
    uint32 delay_time;
    qcarcam_test_buf_timer buf_timer[QCARCAM_MAX_NUM_BUFFERS];

    std::list<uint32> usr_process_buf_idx;
    int usr_process_frameCnt;
    CameraThread usr_process_thread_handle;
    std::queue<qcarcam_event_msg_t> eventqueue;
    pthread_mutex_t queue_mutex;
#ifdef POST_PROCESS
    void *pPostProcessing;
    void *pSource_surface;
    void *pTarget_surface;
#endif
} qcarcam_test_input_t;

typedef struct
{
    int numInputs;
    qcarcam_test_input_t inputs[NUM_MAX_CAMERAS];
    int opened_stream_cnt;

    QCarCamTestInputEnumerator_t queryInputs[NUM_MAX_CAMERAS];

    /* 0 : post buffers directly to screen
     * 1 : blit buffers to display buffers through c2d
     */
    int enable_c2d;
#ifndef C2D_DISABLED
    pthread_mutex_t mutex_c2d;
#endif


    int dumpFrame;
    int enablePauseResume;
    int enableStartStop;
    int multithreaded;
    int enableStats;
    int enableMenuMode;
    int enableBridgeErrorDetect;
    int enableFatalErrorRecover;
    int enableIFEOverflowhandle;
    int enableRetry;
    int gpioNumber;
    int gpioMode;
    int disable_display;
    int checkDualCsi;
    int enable_csc;
    int enable_cache;

    int exitSeconds;

    int fps_print_delay;
    int vis_value;
    int check_buffer_state;

    /*abort condition*/
    pthread_mutex_t mutex_abort;
    pthread_cond_t cond_abort;
    pthread_mutex_t mutex_csi_err;
    pthread_mutex_t mutex_open_cnt;

    unsigned long long t_start; //program start
    testutil_deinterlace_t enable_deinterlace;
#ifdef POST_PROCESS
    int enable_post_processing; // enable post processing path
    const IPostProcessing *post_processing_interface;
    GetPostProcessingInterfaceType pfnGetPostProcessingInterface;
    void *pPost_processing;

    test_util_pp_ctxt_t post_processing_ctx[NUM_MAX_CAMERAS] = {};
#endif
} qcarcam_test_ctxt_t;

typedef enum
{
    QCARCAM_TEST_MENU_FIRST_ITEM = 1,
    QCARCAM_TEST_MENU_STREAM_OPEN = QCARCAM_TEST_MENU_FIRST_ITEM,
    QCARCAM_TEST_MENU_STREAM_CLOSE,
    QCARCAM_TEST_MENU_STREAM_STOP,
    QCARCAM_TEST_MENU_STREAM_START,
    QCARCAM_TEST_MENU_STREAM_PAUSE,
    QCARCAM_TEST_MENU_STREAM_RESUME,
    QCARCAM_TEST_MENU_STREAM_STOP_ALL,
    QCARCAM_TEST_MENU_STREAM_START_ALL,
    QCARCAM_TEST_MENU_STREAM_PAUSE_ALL,
    QCARCAM_TEST_MENU_STREAM_RESUME_ALL,
    QCARCAM_TEST_MENU_STREAM_ENABLE_CALLBACK,
    QCARCAM_TEST_MENU_STREAM_DISABLE_CALLBACK,
    QCARCAM_TEST_MENU_STREAM_SET_FRAMERATE,
    QCARCAM_TEST_MENU_STREAM_SET_EXPOSURE,
    QCARCAM_TEST_MENU_STREAM_SET_SENSORMODE,
    QCARCAM_TEST_MENU_STREAM_GET_SENSORMODE,
    QCARCAM_TEST_MENU_STREAM_SET_COLOR_PARAM,
    QCARCAM_TEST_MENU_STREAM_GET_COLOR_PARAM,
    QCARCAM_TEST_MENU_STREAM_SET_GAMMA_PARAM,
    QCARCAM_TEST_MENU_STREAM_GET_GAMMA_PARAM,
    QCARCAM_TEST_MENU_STREAM_SET_ISP_PARAM,
    QCARCAM_TEST_MENU_STREAM_GET_ISP_PARAM,
    QCARCAM_TEST_MENU_DUMP_NEXT_FRAME,
    QCARCAM_TEST_MENU_CHECK_BUFFERS,
    QCARCAM_TEST_MENU_STREAM_SET_VENDOR_PARAM,
    QCARCAM_TEST_MENU_STREAM_GET_VENDOR_PARAM,
    QCARCAM_TEST_MENU_STREAM_SET_BRIGHTNESS,
    QCARCAM_TEST_MENU_STREAM_GET_BRIGHTNESS,
    QCARCAM_TEST_MENU_STREAM_SET_CONTRAST,
    QCARCAM_TEST_MENU_STREAM_GET_CONTRAST,
    QCARCAM_TEST_MENU_STREAM_SET_MIRRORING,
    QCARCAM_TEST_MENU_STREAM_GET_MIRRORING,
    QCARCAM_TEST_MENU_STREAM_SUBSCRIBE_CHANGE_EVENT,
    QCARCAM_TEST_MENU_STREAM_UNSUBSCRIBE_CHANGE_EVENT,
    QCARCAM_TEST_MENU_STREAM_SUBSCRIBE_SOF_EVENT,
    QCARCAM_TEST_MENU_STREAM_UNSUBSCRIBE_SOF_EVENT,
    QCARCAM_TEST_MENU_MASTER,
    QCARCAM_TEST_MENU_GET_SYSTEM_DIAGNOSTICS,
    QCARCAM_TEST_MENU_STREAM_GET_COLORSPACE,
    QCARCAM_TEST_MENU_MAX
}qcarcam_test_menu_option_t;

typedef struct
{
    qcarcam_test_menu_option_t id;
    const char* str;
}qcarcam_test_menu_t;

static qcarcam_test_menu_t g_qcarcam_menu[QCARCAM_TEST_MENU_MAX] =
{
    {},
    {QCARCAM_TEST_MENU_STREAM_OPEN,  "Open a stream"},
    {QCARCAM_TEST_MENU_STREAM_CLOSE, "Close a stream"},
    {QCARCAM_TEST_MENU_STREAM_STOP,  "Stop a stream"},
    {QCARCAM_TEST_MENU_STREAM_START, "Start a stream"},
    {QCARCAM_TEST_MENU_STREAM_PAUSE, "Pause a stream"},
    {QCARCAM_TEST_MENU_STREAM_RESUME, "Resume a stream"},
    {QCARCAM_TEST_MENU_STREAM_STOP_ALL, "Stop all streams"},
    {QCARCAM_TEST_MENU_STREAM_START_ALL, "Start all streams"},
    {QCARCAM_TEST_MENU_STREAM_PAUSE_ALL, "Pause all streams"},
    {QCARCAM_TEST_MENU_STREAM_RESUME_ALL, "Resume all streams"},
    {QCARCAM_TEST_MENU_STREAM_ENABLE_CALLBACK, "Enable callback"},
    {QCARCAM_TEST_MENU_STREAM_DISABLE_CALLBACK, "Disable callback"},
    {QCARCAM_TEST_MENU_STREAM_SET_FRAMERATE, "Set frame rate control"},
    {QCARCAM_TEST_MENU_STREAM_SET_EXPOSURE, "Set exposure"},
    {QCARCAM_TEST_MENU_STREAM_SET_SENSORMODE,"Set sensormode"},
    {QCARCAM_TEST_MENU_STREAM_GET_SENSORMODE,"Get sensormode"},
    {QCARCAM_TEST_MENU_STREAM_SET_COLOR_PARAM, "Set color param"},
    {QCARCAM_TEST_MENU_STREAM_GET_COLOR_PARAM, "Get color param"},
    {QCARCAM_TEST_MENU_STREAM_SET_GAMMA_PARAM, "Set Gamma Table"},
    {QCARCAM_TEST_MENU_STREAM_GET_GAMMA_PARAM, "Get Gamma Table"},
    {QCARCAM_TEST_MENU_STREAM_SET_ISP_PARAM, "Set ISP settings"},
    {QCARCAM_TEST_MENU_STREAM_GET_ISP_PARAM, "Get ISP settings"},
    {QCARCAM_TEST_MENU_DUMP_NEXT_FRAME, "Dump Next Frame"},
    {QCARCAM_TEST_MENU_CHECK_BUFFERS, "Check Buffers"},
    {QCARCAM_TEST_MENU_STREAM_SET_VENDOR_PARAM, "Set Vendor Param"},
    {QCARCAM_TEST_MENU_STREAM_GET_VENDOR_PARAM, "Get Vendor Param"},
    {QCARCAM_TEST_MENU_STREAM_SET_BRIGHTNESS, "Set Brightness"},
    {QCARCAM_TEST_MENU_STREAM_GET_BRIGHTNESS, "Get Brightness"},
    {QCARCAM_TEST_MENU_STREAM_SET_CONTRAST, "Set Contrast"},
    {QCARCAM_TEST_MENU_STREAM_GET_CONTRAST, "Get Contrast"},
    {QCARCAM_TEST_MENU_STREAM_SET_MIRRORING, "Set Mirroring"},
    {QCARCAM_TEST_MENU_STREAM_GET_MIRRORING, "Get Mirroring"},
    {QCARCAM_TEST_MENU_STREAM_SUBSCRIBE_CHANGE_EVENT, "Subscribe for an event"},
    {QCARCAM_TEST_MENU_STREAM_UNSUBSCRIBE_CHANGE_EVENT, "Unsubscribe for an event"},
    {QCARCAM_TEST_MENU_STREAM_SUBSCRIBE_SOF_EVENT, "Subscribe for SOF event"},
    {QCARCAM_TEST_MENU_STREAM_UNSUBSCRIBE_SOF_EVENT, "Unsubscribe for SOF event"},
    {QCARCAM_TEST_MENU_MASTER, "Set/Release a client as master"},
    {QCARCAM_TEST_MENU_GET_SYSTEM_DIAGNOSTICS, "Get System diagnostic info"},
    {QCARCAM_TEST_MENU_STREAM_GET_COLORSPACE, "Get color space"},

};

///////////////////////////////
/// STATICS
///////////////////////////////
static qcarcam_test_ctxt_t gCtxt = {};

static test_util_global_config_t g_xml_cfg = {};

static char g_filename[128] = "qcarcam_config.xml";

static volatile int g_aborted = 0;

static sigset_t g_sigset;

static const int exceptsigs[] = {
    SIGCHLD, SIGIO, SIGURG, SIGWINCH,
    SIGTTIN, SIGTTOU, SIGCONT, SIGSEGV,
    -1,
};

static QCarCamRet_e qcarcam_test_event_cb(const QCarCamHndl_t hndl,
        const uint32_t eventId,
        const QCarCamEventPayload_t *pPayload,
        void  *pPrivateData);

static void initialize_qcarcam_test_ctxt(void)
{
    gCtxt.numInputs = 0;
    gCtxt.opened_stream_cnt = 0;

    gCtxt.enable_c2d = 0;
#ifdef POST_PROCESS
    gCtxt.enable_post_processing = 0;
#endif
#ifndef C2D_DISABLED
    pthread_mutex_init(&gCtxt.mutex_c2d, NULL);
#endif

    gCtxt.dumpFrame = 0;
    gCtxt.enablePauseResume = 0;
    gCtxt.enableStartStop = 0;
    gCtxt.multithreaded = 1;
    gCtxt.enableStats = 1;
    gCtxt.enableMenuMode = 1;
    gCtxt.enableBridgeErrorDetect = 1;
    gCtxt.enableFatalErrorRecover = 0;
    gCtxt.enableIFEOverflowhandle = 0;
    gCtxt.enableRetry = 0;
    gCtxt.disable_display = 0;
    gCtxt.checkDualCsi = 0;
    gCtxt.enable_csc = 0;
    gCtxt.enable_cache = 0;

    gCtxt.exitSeconds = 0;
    gCtxt.gpioNumber = 0;
    gCtxt.vis_value = 1;

    gCtxt.fps_print_delay = DEFAULT_PRINT_DELAY_SEC;
    gCtxt.check_buffer_state = 0;

    pthread_mutex_init(&gCtxt.mutex_abort, NULL);
    pthread_cond_init(&gCtxt.cond_abort, NULL);
    pthread_mutex_init(&gCtxt.mutex_csi_err, NULL);
    pthread_mutex_init(&gCtxt.mutex_open_cnt, NULL);

    gCtxt.t_start = 0;
    gCtxt.enable_deinterlace = TESTUTIL_DEINTERLACE_NONE;
}

static void display_valid_stream_ids()
{
    int i;
    qcarcam_test_input_t *input_ctxt = NULL;

    printf("Valid stream ids are\n");
    printf("========================\n");
    printf("Camera id   stream id\n");
    printf("========================\n");
    for (i = 0; i < gCtxt.numInputs; ++i)
    {
        input_ctxt = &gCtxt.inputs[i];
        if (input_ctxt->qcarcam_hndl)
        {
            printf("%d          %d\n", input_ctxt->qcarcam_input_id, i);
        }
    }
    printf("========================\n");
}

static void display_valid_closed_stream_ids()
{
    int i;
    qcarcam_test_input_t *input_ctxt = NULL;

    printf("Valid closed stream ids are\n");
    printf("========================\n");
    printf("Camera id   stream id\n");
    printf("========================\n");
    for (i = 0; i < gCtxt.numInputs; ++i)
    {
        input_ctxt = &gCtxt.inputs[i];
        if (input_ctxt->state ==  QCARCAMTEST_STATE_CLOSED)
        {
            printf("%d          %d\n", input_ctxt->qcarcam_input_id, i);
        }
    }
    printf("========================\n");
}


/**
 * Returns user entered stream idx
 */
static int get_closed_input_stream_id()
{
    int stream_id = gCtxt.numInputs;
    char buf[BUFSIZE];
    char *p = NULL;

    display_valid_closed_stream_ids();

    printf("Enter stream id\n");

    if (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        stream_id = strtol(buf, &p, 10);
    }

    return stream_id;
}
/**
 * Returns user entered stream idx
 */
static int get_input_stream_id()
{
    int stream_id = gCtxt.numInputs;
    char buf[BUFSIZE];
    char *p = NULL;

    display_valid_stream_ids();

    printf("Enter stream id\n");

    if (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        stream_id = strtol(buf, &p, 10);
    }

    return stream_id;
}

static void get_input_sensormode(uint32_t* sensormode_index)
{
    if (sensormode_index != NULL)
    {
        char buf[BUFSIZE];
        char *p = NULL;

        printf("Enter camera sensormode index:[0]30fps mode [3]15fps mode \n");
        if (fgets(buf, sizeof(buf), stdin) != NULL)
        {
            *sensormode_index = strtol(buf, &p, 10);
        }
    }
}

static void get_input_hdr_exposure(QCarCamExposureConfig_t* exposure_config)
{
    char buf[BUFSIZE];
    char *p = NULL;

    printf("numExposures?\n");
    if (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        exposure_config->numExposures = strtol(buf, &p, 10);
    }

    for (uint32_t i = 0; i < exposure_config->numExposures; i++)
    {
        printf("exp%d\n", i);
        if (fgets(buf, sizeof(buf), stdin) != NULL)
        {
            exposure_config->exposureTime[i] = strtof(buf, NULL);
        }

        printf("gain%d\n", i);
        if (fgets(buf, sizeof(buf), stdin) != NULL)
        {
            exposure_config->gain[i] = strtof(buf, NULL);
        }
    }
}

static void get_input_framerate(QCarCamFrameDropConfig_t* frame_rate_config)
{
    if (frame_rate_config != NULL)
    {
        int frame_drop_period = 0;
        int frame_drop_pattern = 0;

        char buf[BUFSIZE];

        printf("Enter period value, maximal 32\n");
        if (fgets(buf, sizeof(buf), stdin) != NULL)
        {
            frame_drop_period = strtol(buf, NULL, 10);
        }
        printf("Enter pattern hex value, e.g. 0x23\n");
        if (fgets(buf, sizeof(buf), stdin) != NULL)
        {
            frame_drop_pattern = strtol(buf, NULL, 16);
        }

        frame_rate_config->frameDropPeriod = (unsigned char)frame_drop_period;
        frame_rate_config->frameDropPattern = frame_drop_pattern;
    }
}

static int get_input_set_flag()
{
    printf("Enter set[1] release[0]");
    char buf[BUFSIZE];
    int flag = 0;
    char *p = NULL;
    if (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        flag = strtol(buf, &p, 10);
    }
    return flag;
}

static int qcarcam_test_get_time(unsigned long long *pTime)
{
    struct timespec time;
    unsigned long long msec;

    if (clock_gettime(CLOCK_MONOTONIC, &time) == -1)
    {
        QCARCAM_ERRORMSG("Clock gettime failed");
        return 1;
    }
    msec = ((unsigned long long)time.tv_sec * 1000) + (((unsigned long long)time.tv_nsec / 1000) / 1000);
    *pTime = msec;

    return 0;
}

static void qcarcam_test_clear_usr_process_list(qcarcam_test_input_t *input_ctxt)
{
    pthread_mutex_lock(&input_ctxt->mutex);
    input_ctxt->state = QCARCAMTEST_STATE_PAUSE_STOP_PENDING;

    /* Wait user process frame in progress to finish */
    while (input_ctxt->usr_process_frameCnt)
    {
        pthread_mutex_unlock(&input_ctxt->mutex);
        usleep(USR_PROCESS_WAIT_USLEEP);
        pthread_mutex_lock(&input_ctxt->mutex);
    }

    /* reclaim user process frames in list */
    while (!input_ctxt->usr_process_buf_idx.empty())
    {
        uint32 idx = input_ctxt->usr_process_buf_idx.front();
        input_ctxt->usr_process_buf_idx.pop_front();
        input_ctxt->buf_state[idx] = QCARCAMTEST_BUFFER_STATE_GET_FRAME;
        input_ctxt->release_buf_idx.push_back(idx);
    }

    pthread_mutex_unlock(&input_ctxt->mutex);
}

static QCarCamRet_e qcarcam_input_open(qcarcam_test_input_t *input_ctxt)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    QCarCamOpen_t openParams = {};

    openParams.opMode = input_ctxt->op_mode;
    openParams.numInputs = 1;
    openParams.inputs[0].inputId = input_ctxt->qcarcam_input_id;

    if (input_ctxt->recovery)
    {
        openParams.flags = QCARCAM_OPEN_FLAGS_RECOVERY;
    }

    ret = QCarCamOpen(&openParams, &input_ctxt->qcarcam_hndl);
    if(ret != QCARCAM_RET_OK || input_ctxt->qcarcam_hndl == QCARCAM_HNDL_INVALID)
    {
        input_ctxt->state = QCARCAMTEST_STATE_ERROR;
        QCARCAM_ERRORMSG("qcarcam_open() failed %d %llu", ret, input_ctxt->qcarcam_hndl);
        goto qcarcam_input_open_error;
    }

    pthread_mutex_lock(&gCtxt.mutex_open_cnt);
    gCtxt.opened_stream_cnt++;
    pthread_mutex_unlock(&gCtxt.mutex_open_cnt);
    input_ctxt->state = QCARCAMTEST_STATE_OPEN;

    //@TODO: change to set sensor mode id
#if 0
    // For HDMI/CVBS Input
    // NOTE: set HDMI IN resolution in qcarcam_config_single_hdmi.xml before
    // running the test
    if (input_ctxt->qcarcam_input_id == QCARCAM_INPUT_TYPE_DIGITAL_MEDIA)
    {
        qcarcam_param_value_t param = {};
        param.res_value.width = input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].width;
        param.res_value.height = input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].height;

        ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_PARAM_RESOLUTION, &param);
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("QCarCamSetParam resolution() failed");
            goto qcarcam_input_open_fail;;
        }
    }
#endif

    ret = QCarCamSetBuffers(input_ctxt->qcarcam_hndl, &input_ctxt->p_buffers_output);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("qcarcam_s_buffers() failed");
        goto qcarcam_input_open_fail;
    }

    if (input_ctxt->is_injection)
    {
        QCARCAM_ERRORMSG("read from input_file - set input buffers");

        ret = QCarCamSetBuffers(input_ctxt->qcarcam_hndl, &input_ctxt->p_buffers_input);
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("QCarCamSetBuffers() failed");
            goto qcarcam_input_open_fail;
        }
    }

    if (input_ctxt->use_event_callback)
    {
        uint32_t param = 0;

        ret = QCarCamRegisterEventCallback(input_ctxt->qcarcam_hndl, &qcarcam_test_event_cb, input_ctxt);
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("QCarCamSetParam(QCARCAM_PARAM_EVENT_CB) failed");
            goto qcarcam_input_open_fail;
        }


        if (input_ctxt->recovery)
        {
            param = QCARCAM_EVENT_FRAME_READY |
                QCARCAM_EVENT_INPUT_SIGNAL |
                QCARCAM_EVENT_ERROR | QCARCAM_EVENT_VENDOR |
                QCARCAM_EVENT_RECOVERY;
        }
        else
        {
            param = QCARCAM_EVENT_FRAME_READY |
                QCARCAM_EVENT_INPUT_SIGNAL |
                QCARCAM_EVENT_ERROR | QCARCAM_EVENT_VENDOR;
        }

        ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK, &param, sizeof(param));
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("QCarCamSetParam(QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK) failed");
            goto qcarcam_input_open_fail;
        }
    }
    if (input_ctxt->frameDropConfig.frameDropPeriod != 0)
    {
        ret = QCarCamSetParam(input_ctxt->qcarcam_hndl,
                QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL,
                &input_ctxt->frameDropConfig,
                sizeof(input_ctxt->frameDropConfig));
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("QCarCamSetParam(QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL) failed");
            goto qcarcam_input_open_fail;
        }
    }
    if (input_ctxt->num_batch_frames > 1)
    {
        QCarCamBatchConfig_t param = {};

        param.numBatchFrames = input_ctxt->num_batch_frames;
        param.frameIncrement = input_ctxt->frame_increment;

        ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE, &param, sizeof(param));
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("QCarCamSetParam(QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE) failed");
            goto qcarcam_input_open_fail;
        }
    }
    for (uint32 idx = 0; idx < input_ctxt->num_isp_instances; idx++)
    {
        ret = QCarCamSetParam(input_ctxt->qcarcam_hndl,
                QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE,
                &input_ctxt->isp_config[idx],
                sizeof(input_ctxt->isp_config[idx]));
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("QCarCamSetParam(QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE) failed");
            goto qcarcam_input_open_fail;
        }
    }

qcarcam_input_open_fail:

    if(ret != QCARCAM_RET_OK)
    {
        QCarCamClose(input_ctxt->qcarcam_hndl);
        input_ctxt->state = QCARCAMTEST_STATE_CLOSED;
        input_ctxt->qcarcam_hndl = QCARCAM_HNDL_INVALID;
        pthread_mutex_lock(&gCtxt.mutex_open_cnt);
        gCtxt.opened_stream_cnt--;
        pthread_mutex_unlock(&gCtxt.mutex_open_cnt);
    }
qcarcam_input_open_error:
    return ret;
}

static QCarCamRet_e qcarcam_input_close(qcarcam_test_input_t *input_ctxt)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    ret = QCarCamClose(input_ctxt->qcarcam_hndl);
    if (ret == QCARCAM_RET_OK)
    {
        input_ctxt->state = QCARCAMTEST_STATE_CLOSED;
        input_ctxt->qcarcam_hndl = QCARCAM_HNDL_INVALID;
        pthread_mutex_lock(&gCtxt.mutex_open_cnt);
        gCtxt.opened_stream_cnt--;
        pthread_mutex_unlock(&gCtxt.mutex_open_cnt);

        QCARCAM_INFOMSG("Client %d Input %d QCarCamClose successfully", input_ctxt->idx, input_ctxt->qcarcam_input_id);
    }
    else
    {
        input_ctxt->state = QCARCAMTEST_STATE_ERROR;
        QCARCAM_ERRORMSG("Client %d Input %d QCarCamClose failed: %d", input_ctxt->idx, input_ctxt->qcarcam_input_id, ret);
    }

    return ret;
}


static QCarCamRet_e qcarcam_input_start(qcarcam_test_input_t *input_ctxt)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    qcarcam_test_get_time(&input_ctxt->t_start);

    ret = QCarCamStart(input_ctxt->qcarcam_hndl);
    if (ret == QCARCAM_RET_OK)
    {
        input_ctxt->state = QCARCAMTEST_STATE_START;
        input_ctxt->frameCnt = 0;
        input_ctxt->releaseframeCnt = 0;
        input_ctxt->prev_frameCnt = 0;
        input_ctxt->prev_releaseframeCnt = 0;
        input_ctxt->signal_lost = 0;

        qcarcam_test_get_time(&input_ctxt->t_start_success);

        QCARCAM_INFOMSG("Client %d Input %d QCarCamStart successfully", input_ctxt->idx, input_ctxt->qcarcam_input_id);

        //test for request_mode
        if (input_ctxt->request_mode)
        {
            QCarCamRequest_t request = {};
            uint32 i = 0;

            for (i = 0; i < input_ctxt->p_buffers_output.nBuffers; i++)
            {
                request.requestId = i;
                request.streamRequests[0].bufferlistId = QCARCAM_BUFFERLIST_ID_OUTPUT_0;
                request.streamRequests[0].bufferId = i;

                ret = QCarCamSubmitRequest(input_ctxt->qcarcam_hndl, &request);
                QCARCAM_DBGMSG("submit req, req id %d, buf id %d, ret %d", request.requestId, request.streamRequests[0].bufferId, ret);
            }
            input_ctxt->submitrequestSettings.request_id = i;
        }
    }
    else
    {
        input_ctxt->state = QCARCAMTEST_STATE_ERROR;
        QCARCAM_ERRORMSG("Client %d Input %d QCarCamStart failed: %d", input_ctxt->idx, input_ctxt->qcarcam_input_id, ret);
    }

    return ret;
}

static QCarCamRet_e qcarcam_input_stop(qcarcam_test_input_t *input_ctxt)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    ret = QCarCamStop(input_ctxt->qcarcam_hndl);
    if (ret == QCARCAM_RET_OK)
    {
        input_ctxt->state = QCARCAMTEST_STATE_STOP;

        input_ctxt->frameCnt = 0;
        input_ctxt->releaseframeCnt = 0;
        input_ctxt->prev_frameCnt = 0;
        input_ctxt->prev_releaseframeCnt = 0;
        input_ctxt->release_buf_idx.clear();
        input_ctxt->usr_process_buf_idx.clear();
        input_ctxt->submitrequestSettings.request_id = 0;

        QCARCAM_INFOMSG("Client %d Input %d QCarCamStop successfully", input_ctxt->idx, input_ctxt->qcarcam_input_id);
    }
    else
    {
        input_ctxt->state = QCARCAMTEST_STATE_ERROR;
        QCARCAM_ERRORMSG("Client %d Input %d QCarCamStop failed: %d", input_ctxt->idx, input_ctxt->qcarcam_input_id, ret);
    }

    memset(&input_ctxt->buf_state, 0x0, sizeof(input_ctxt->buf_state));

    return ret;
}

static QCarCamRet_e qcarcam_input_pause(qcarcam_test_input_t *input_ctxt)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    ret = QCarCamPause(input_ctxt->qcarcam_hndl);
    if (ret == QCARCAM_RET_OK)
    {
        input_ctxt->state = QCARCAMTEST_STATE_PAUSE;

        input_ctxt->frameCnt = 0;
        input_ctxt->prev_frameCnt = 0;
        input_ctxt->releaseframeCnt = 0;
        input_ctxt->prev_releaseframeCnt = 0;

        QCARCAM_INFOMSG("Client %d Input %d QCarCamPause successfully", input_ctxt->idx, input_ctxt->qcarcam_input_id);
    }
    else
    {
        input_ctxt->state = QCARCAMTEST_STATE_ERROR;
        QCARCAM_ERRORMSG("Client %d Input %d QCarCamPause failed: %d", input_ctxt->idx, input_ctxt->qcarcam_input_id, ret);
    }

    return ret;
}

static QCarCamRet_e qcarcam_input_resume(qcarcam_test_input_t *input_ctxt)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    qcarcam_test_get_time(&input_ctxt->t_start);

    ret = QCarCamResume(input_ctxt->qcarcam_hndl);
    if (ret == QCARCAM_RET_OK)
    {
        input_ctxt->state = QCARCAMTEST_STATE_START;

        input_ctxt->frameCnt = 0;
        input_ctxt->releaseframeCnt = 0;
        input_ctxt->prev_frameCnt = 0;
        input_ctxt->prev_releaseframeCnt = 0;
        input_ctxt->signal_lost = 0;
        qcarcam_test_get_time(&input_ctxt->t_start_success);

        QCARCAM_INFOMSG("Client %d Input %d QCarCamResume success", input_ctxt->idx, input_ctxt->qcarcam_input_id);
    }
    else
    {
        input_ctxt->state = QCARCAMTEST_STATE_ERROR;
        QCARCAM_ERRORMSG("Client %d Input %d QCarCamResume failed %d", input_ctxt->idx, input_ctxt->qcarcam_input_id, ret);
    }

    return ret;
}


static void qcarcam_test_check_buffers(qcarcam_test_input_t *input_ctxt)
{
    unsigned int i;
    unsigned int num_buffers = input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].n_buffers;

    for (i = 0; i < num_buffers; i++)
    {
        printf("|    |     | %d  0x%02x  ", i, input_ctxt->buf_state[i]);
        if (CHECK_BIT(input_ctxt->buf_state[i], QCARCAMTEST_BUFFER_STALE_BIT))
        {
            printf(" [stale]");
        }
        else
        {
            printf("        ");
        }
        printf("          |\n");

        input_ctxt->buf_state[i] = (qcarcam_test_buffer_state_t)(input_ctxt->buf_state[i] | (1 << QCARCAMTEST_BUFFER_STALE_BIT));
    }

    fflush(stdout);
}

static void qcarcam_test_get_frame_rate(unsigned long long timer_prev)
{
    float average_fps, average_rel_fps;
    int input_idx = 0;
    unsigned long long timer_now = 0;

    qcarcam_test_get_time(&timer_now);

    printf("--------FPS Report - %.1f sec-----------\n", ((float)(timer_now - gCtxt.t_start) / 1000));
    printf("| id | qid |  state (time)| fps  | rel  |\n");

    for (input_idx = 0; input_idx < gCtxt.numInputs; ++input_idx)
    {
        qcarcam_test_input_t* input_ctxt = &gCtxt.inputs[input_idx];

        pthread_mutex_lock(&input_ctxt->mutex);

        qcarcam_test_get_time(&timer_now);

        printf("| %2u | %2u  | ", input_ctxt->idx, (unsigned int)input_ctxt->qcarcam_input_id);

        if (input_ctxt->state == QCARCAMTEST_STATE_START)
        {
            if (!input_ctxt->prev_frameCnt)
            {
                average_fps = 0.0f;
                average_rel_fps = 0.0f;

                //use first frame time for first report if got a frame
                if (input_ctxt->frameCnt > 1)
                {
                    printf("ok      (%3.1f)", ((float)(timer_now - input_ctxt->t_firstFrame) / 1000));
                    average_fps = (input_ctxt->frameCnt-1) / ((float)(timer_now - input_ctxt->t_firstFrame) / 1000);
                    if (input_ctxt->releaseframeCnt > 1)
                    {
                        average_rel_fps = (input_ctxt->releaseframeCnt-1) / ((float)(timer_now - input_ctxt->t_firstFrame) / 1000);
                    }
                }
                else
                {
                    float time_since_start = ((float)(timer_now - input_ctxt->t_start) / 1000);
                    float time_since_start_success = ((float)(timer_now - input_ctxt->t_start_success) / 1000);
                    if (time_since_start_success > QCARCAMTEST_SOF_FREEZE_TIMEOUT)
                    {
                        //TODO: maybe can add freeze detection here to try and restart?
                        printf("freeze  (%3.1f)", time_since_start);
                    }
                    else
                    {
                        printf("started (%3.1f)", time_since_start);
                    }
                }
            }
            else
            {
                int frames_counted;
                int release_frames_counted;

                frames_counted = input_ctxt->frameCnt - input_ctxt->prev_frameCnt;
                release_frames_counted = input_ctxt->releaseframeCnt - input_ctxt->prev_releaseframeCnt;
                average_fps = frames_counted / ((float)(timer_now - timer_prev) / 1000);
                average_rel_fps = release_frames_counted / ((float)(timer_now - timer_prev) / 1000);

                if (frames_counted)
                {
                    printf("ok           ");
                }
                else
                {
                    //TODO: maybe can add freeze detection here to try and restart?
                    printf("freeze       ");
                }
            }

            printf("| %4.1f | %4.1f |\n", average_fps, average_rel_fps);

            input_ctxt->prev_frameCnt = input_ctxt->frameCnt;
            input_ctxt->prev_releaseframeCnt = input_ctxt->releaseframeCnt;

            if (gCtxt.check_buffer_state)
            {
                qcarcam_test_check_buffers(input_ctxt);
            }
        }
        else if (input_ctxt->state == QCARCAMTEST_STATE_STOP)
        {
            printf("stop         |      |      |\n");
        }
        else if (input_ctxt->state == QCARCAMTEST_STATE_PAUSE)
        {
            printf("pause        |      |      |\n");
        }
        else if (input_ctxt->state == QCARCAMTEST_STATE_ERROR)
        {
            printf("ERROR        |      |      |\n");
        }
        else if (input_ctxt->state == QCARCAMTEST_STATE_INIT)
        {
            printf("init         |      |      |\n");
        }
        else if (input_ctxt->state == QCARCAMTEST_STATE_PAUSE_STOP_PENDING)
        {
            printf("pending      |      |      |\n");
        }
        else if (input_ctxt->state == QCARCAMTEST_STATE_SUSPEND)
        {
            printf("suspend      |      |      |\n");
        }
        else if (input_ctxt->state == QCARCAMTEST_STATE_RECOVERY)
        {
            printf("recovery     |      |      |\n");
        }
        else
        {
            printf("UNKNOWN(%d)  |      |      |\n", input_ctxt->state);
        }

        pthread_mutex_unlock(&input_ctxt->mutex);
    }
    printf("-----------------------------------------\n");
    fflush(stdout);
}

static int test_signal_loss(qcarcam_test_input_t *input_ctxt, bool *signal_lost_check)
{
    if (input_ctxt->signal_lost != *signal_lost_check)
    {
        // Check if signal status has changed
        *signal_lost_check = input_ctxt->signal_lost;
    }
    else if (input_ctxt->signal_lost == 1)
    {
        // wait 1 cycle then post empty frame to display
        test_util_post_window_buffer(input_ctxt->test_util_ctxt,
                input_ctxt->qcarcam_window,
                input_ctxt->p_buffers_output.nBuffers,
                &input_ctxt->release_buf_idx,
                input_ctxt->field_type_previous);
    }

    return 0;
}

static int check_signal_loss_thread(void *arg)
{
    pthread_detach(pthread_self());

    bool signal_lost_check[NUM_MAX_CAMERAS] = {};
    unsigned int signal_check_delay_us = SIGNAL_CHECK_DELAY; // 33 milliseconds

    while (!g_aborted)
    {
        // Check if signal status has changed
        for (int i = 0; i < gCtxt.numInputs; i++)
        {
            test_signal_loss(&gCtxt.inputs[i], &signal_lost_check[i]);
        }
        usleep(signal_check_delay_us);
    }
    return 0;
}

static void test_fatal_error_check(qcarcam_test_input_t *input_ctxt, volatile int *csi_err_cnt_prev)
{
    if (input_ctxt->fatal_err_cnt != *csi_err_cnt_prev)
    {
        // error happened. Set it and wait 1 more iteration before take recovery action
        *csi_err_cnt_prev = input_ctxt->fatal_err_cnt;
        input_ctxt->is_fatal_error = 1;
    }
    else if (input_ctxt->is_fatal_error)
    {
        pthread_mutex_lock(&input_ctxt->mutex);
        if (input_ctxt->state == QCARCAMTEST_STATE_START)
        {
            QCARCAM_ERRORMSG("Input %d already running again, return", input_ctxt->qcarcam_input_id);
        }
        else
        {
            (void)qcarcam_input_start(input_ctxt);
            input_ctxt->is_fatal_error = 0;
        }

        pthread_mutex_unlock(&input_ctxt->mutex);
    }
}

static int check_error_thread(void *arg)
{
    pthread_detach(pthread_self());

    volatile int fatal_error_prev[NUM_MAX_CAMERAS] = { 0 };
    unsigned int error_check_delay_us = CSI_ERR_CHECK_DELAY; // 100 milliseconds

    while (!g_aborted)
    {
        // Check if csi error continues or not
        for (int i = 0; i < gCtxt.numInputs; i++)
        {
            test_fatal_error_check(&gCtxt.inputs[i], &fatal_error_prev[i]);
        }
        usleep(error_check_delay_us);
    }
    return 0;
}

static int framerate_thread(void *arg)
{
    pthread_detach(pthread_self());

    unsigned int fps_print_delay_us = gCtxt.fps_print_delay * 1000000;
    unsigned long long timer1 = 0;

    while (!g_aborted)
    {
        qcarcam_test_get_time(&timer1);
        usleep(fps_print_delay_us);
        qcarcam_test_get_frame_rate(timer1);
    }
    return 0;
}

static void abort_test(void)
{
    QCARCAM_ERRORMSG("Aborting test");
    pthread_mutex_lock(&gCtxt.mutex_abort);
    g_aborted = 1;
    pthread_cond_broadcast(&gCtxt.cond_abort);
    pthread_mutex_unlock(&gCtxt.mutex_abort);
}

static int timer_thread(void *arg)
{
    pthread_detach(pthread_self());

    unsigned long long timer_start = 0;
    unsigned long long timer_test = 0;
    qcarcam_test_get_time(&timer_start);
    while (!g_aborted)
    {
        qcarcam_test_get_time(&timer_test);
        if ((timer_test - timer_start) >= ((unsigned long long)gCtxt.exitSeconds * 1000))
        {
            QCARCAM_ALWZMSG("TEST Aborted after running for %d secs successfully!", gCtxt.exitSeconds);
            abort_test();
            break;
        }
        usleep(TIMER_THREAD_USLEEP);
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
        if ((i = sigtimedwait(&g_sigset, NULL, &timeout)) > 0)
        {
            abort_test();
            break;
        }
    }
    return 0;
}

static void process_deinterlace(qcarcam_test_input_t *input_ctxt, QCarCamInterlaceField_e field_type, testutil_deinterlace_t di_method)
{
    test_util_sw_di_t di_info;

    di_info.qcarcam_window = input_ctxt->qcarcam_window;
    di_info.display_window = input_ctxt->display_window;
    di_info.source_buf_idx = input_ctxt->frameCnt;
    di_info.field_type = field_type;

    switch (di_method) {
    case TESTUTIL_DEINTERLACE_SW_WEAVE:
        /* sw weave 30fps method deinterlacing */
        test_util_di_sw_weave_30fps(&di_info);
        break;
    case TESTUTIL_DEINTERLACE_SW_BOB:
        /* needn't process field into new buffer, display each field in post_window_buffer directly */
        break;
    default:
        /* unsupported deinterlacing method */
        QCARCAM_ERRORMSG("Unknown deinterlacing method");
        break;
    }
}

/**
 * Function to retrieve frame from qcarcam and increase frame_counter
 * @param input_ctxt
 * @return 0 on success, -1 on failure
 */
static int qcarcam_test_get_frame(qcarcam_test_input_t *input_ctxt)
{
    QCarCamRet_e ret;
    QCarCamFrameInfo_t frame_info;
    ret = QCarCamGetFrame(input_ctxt->qcarcam_hndl, &frame_info, input_ctxt->frame_timeout, 0);
    if (ret == QCARCAM_RET_TIMEOUT)
    {
        QCARCAM_ERRORMSG("QCarCamGetFrame timeout context %p ret %d", input_ctxt->qcarcam_hndl, ret);
        input_ctxt->signal_lost = 1;
        return -1;
    }

    if (QCARCAM_RET_OK != ret)
    {
        QCARCAM_ERRORMSG("Get frame context %p ret %d", input_ctxt->qcarcam_hndl, ret);
        return -1;
    }

    if (frame_info.bufferIndex >= input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].n_buffers)
    {
        QCARCAM_ERRORMSG("Get frame context %p ret invalid idx %d", input_ctxt->qcarcam_hndl, frame_info.bufferIndex);
        return -1;
    }

    if (input_ctxt->frameCnt == 0)
    {
        qcarcam_test_get_time(&input_ctxt->t_firstFrame);

        if (input_ctxt->is_first_start)
        {
            ais_log_kpi(AIS_EVENT_KPI_CLIENT_FIRST_FRAME);

            printf("Success - First Frame [%d:%d]\n", input_ctxt->idx, input_ctxt->qcarcam_input_id);
            fflush(stdout);

            QCARCAM_ALWZMSG("[%u:%u] First Frame buf_idx %d after : %lu ms (field type: %d)",
                    input_ctxt->idx, (unsigned int)input_ctxt->qcarcam_input_id, frame_info.bufferIndex, (input_ctxt->t_firstFrame - gCtxt.t_start), frame_info.fieldType);

            input_ctxt->is_first_start = FALSE;
        }
        else
        {
            QCARCAM_ALWZMSG("[%d:%d] restart took %llu ms",
                    input_ctxt->idx, input_ctxt->qcarcam_input_id, (input_ctxt->t_firstFrame - input_ctxt->t_start));
        }

        if (gCtxt.enable_deinterlace)
        {
            input_ctxt->field_type_previous = QCARCAM_INTERLACE_FIELD_UNKNOWN;

            if (frame_info.fieldType == QCARCAM_INTERLACE_FIELD_UNKNOWN)
                input_ctxt->skip_post_display = 1;
            else
                input_ctxt->field_type_previous = frame_info.fieldType;
        }
    }
    else
    {
        if (gCtxt.enable_deinterlace && (input_ctxt->field_type_previous != frame_info.fieldType))
        {
            input_ctxt->skip_post_display = 0;
            QCARCAM_ERRORMSG("Field type changed: %d -> %d @frame_%d", input_ctxt->field_type_previous, frame_info.fieldType, frame_info.seqNo);
            if (frame_info.fieldType == QCARCAM_INTERLACE_FIELD_UNKNOWN)
                frame_info.fieldType = input_ctxt->field_type_previous;
            else
                input_ctxt->field_type_previous = frame_info.fieldType;
        }
    }

    input_ctxt->get_frame_buf_idx = frame_info.bufferIndex;
    input_ctxt->buf_state[input_ctxt->get_frame_buf_idx] = QCARCAMTEST_BUFFER_STATE_GET_FRAME;

    input_ctxt->signal_lost = 0;

    input_ctxt->diag.frame_generate_time[TEST_PREV_BUFFER] = input_ctxt->diag.frame_generate_time[TEST_CUR_BUFFER];
    input_ctxt->diag.frame_generate_time[TEST_CUR_BUFFER] = frame_info.timestamp * NS_TO_MS;

    QCARCAM_DBGMSG("[%d] frameId:%d (seqNo %u) bufId:%d qtime:%llu",
            input_ctxt->idx, input_ctxt->frameCnt, frame_info.seqNo,
        frame_info.bufferIndex, frame_info.sofTimestamp.timestamp);

    if (gCtxt.enable_deinterlace)
        process_deinterlace(input_ctxt, frame_info.fieldType, gCtxt.enable_deinterlace);

    input_ctxt->frameCnt++;

    return 0;
}
/**
 * Function to post new frame to display. May also do color conversion and frame dumps.
 * @param input_ctxt
 * @return 0 on success, -1 on failure
 */
static int qcarcam_test_post_to_display(qcarcam_test_input_t *input_ctxt)
{
    QCarCamRet_e ret;
    /**********************
     * Composite to display
     ********************** */
    QCARCAM_DBGMSG("Post Frame before buf_idx %i", input_ctxt->buf_idx_qcarcam);
    /**********************
     * Dump raw if necessary
     ********************** */
    if (input_ctxt->dumpNextFrame ||
        (input_ctxt->is_injection && input_ctxt->injectionSettings.n_dump_frames > 0) ||
        (gCtxt.dumpFrame && (0 == input_ctxt->frameCnt % gCtxt.dumpFrame)))
    {
        snprintf(g_filename, sizeof(g_filename), DEFAULT_DUMP_LOCATION "frame_%d_%i.raw", input_ctxt->idx, input_ctxt->frameCnt);
#ifdef POST_PROCESS
        test_util_dump_window_buffer(input_ctxt->test_util_ctxt, input_ctxt->display_window, input_ctxt->buf_idx_disp, g_filename);
#else
        test_util_dump_window_buffer(input_ctxt->test_util_ctxt, input_ctxt->qcarcam_window, input_ctxt->buf_idx_qcarcam, g_filename);
#endif

        input_ctxt->dumpNextFrame = FALSE;

        if (input_ctxt->is_injection)
        {
            input_ctxt->injectionSettings.n_dump_frames--;
        }
    }

    if (gCtxt.checkDualCsi)
    {
        test_util_buf_ptr_t buffer = {};
        uint32 i = 0;
        uint32 width = input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].width;
        uint32 height = input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].height;
        uint32 height_incr = 50; //check matching pixels every 50 lines
        uint32 midpoint = width*3/2;
        uint32 stride = 0;
        uint32 err_cnt = 0;
        uint32 check_cnt = 0;
        uint8 *p_buf = NULL;

        buffer.buf_idx = input_ctxt->buf_idx_qcarcam;
        test_util_get_buf_ptr(input_ctxt->qcarcam_window, &buffer);

        p_buf = (uint8 *)buffer.p_va[0];
        stride = (uint32)buffer.stride[0];

        for (i = 0; i < height; i += height_incr)
        {

            if (p_buf[0] != p_buf[midpoint] ||
                p_buf[1] != p_buf[midpoint + 1] ||
                p_buf[2] != p_buf[midpoint + 2])
            {
                err_cnt++;

            }
            check_cnt++;

            p_buf += (stride * height_incr);
        }

        if (err_cnt)
        {
            QCARCAM_ERRORMSG("[FAIL] DUALCSI CHECK %u_%u FAILED %d/%d LINE CHECKS", input_ctxt->idx, input_ctxt->frameCnt,
                err_cnt, check_cnt);
        }
    }

    /**********************
     * Color conversion if necessary
     ********************** */
#ifdef POST_PROCESS
    if (!(gCtxt.enable_c2d || gCtxt.enable_csc|| gCtxt.enable_post_processing))
#else
    if (!(gCtxt.enable_c2d || gCtxt.enable_csc))
#endif
    {
        /**********************
         * Post to screen
         ********************** */
        QCARCAM_DBGMSG("Post Frame %d", input_ctxt->buf_idx_qcarcam);
        if (gCtxt.enable_deinterlace && (gCtxt.enable_deinterlace == TESTUTIL_DEINTERLACE_SW_WEAVE))
        {
            ret = test_util_post_window_buffer(input_ctxt->test_util_ctxt, input_ctxt->display_window, input_ctxt->buf_idx_disp, NULL, input_ctxt->field_type_previous);
            input_ctxt->buf_idx_disp++;
            input_ctxt->buf_idx_disp %= input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].n_buffers;
            input_ctxt->release_buf_idx.push_back(input_ctxt->buf_idx_qcarcam);
        }
        else
        {
            ret = test_util_post_window_buffer(input_ctxt->test_util_ctxt, input_ctxt->qcarcam_window, input_ctxt->buf_idx_qcarcam, &input_ctxt->release_buf_idx, input_ctxt->field_type_previous);
            input_ctxt->buf_state[input_ctxt->buf_idx_qcarcam] = QCARCAMTEST_BUFFER_STATE_POST_DISPLAY;
        }
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("test_util_post_window_buffer failed");
        }
    }
#ifndef C2D_DISABLED
    else
    {
#ifdef POST_PROCESS
        if (!(gCtxt.enable_csc || gCtxt.enable_post_processing))
#else
        if (!(gCtxt.enable_csc))
#endif
        {
            //for now always go through c2d conversion instead of posting directly to  test since gles composition cannot handle
            // uyvy buffers for now.
            QCARCAM_DBGMSG("[%d] converting through c2d %d -> %d", input_ctxt->idx, input_ctxt->buf_idx_qcarcam, input_ctxt->buf_idx_disp);

            C2D_STATUS c2d_status;
            C2D_OBJECT c2dObject;
            memset(&c2dObject, 0x0, sizeof(C2D_OBJECT));
            unsigned int target_id;
            ret = test_util_get_c2d_surface_id(input_ctxt->test_util_ctxt, input_ctxt->qcarcam_window, input_ctxt->buf_idx_qcarcam, &c2dObject.surface_id);
            ret = test_util_get_c2d_surface_id(input_ctxt->test_util_ctxt, input_ctxt->display_window, input_ctxt->buf_idx_disp, &target_id);
            if (ret != QCARCAM_RET_OK)
            {
                QCARCAM_ERRORMSG("test_util_get_c2d_surface_id failed (%d)", ret);
            }

            pthread_mutex_lock(&gCtxt.mutex_c2d);
            c2d_status = c2dDraw(target_id, C2D_TARGET_ROTATE_0, 0x0, 0, 0, &c2dObject, 1);

            c2d_ts_handle c2d_timestamp;
            if (c2d_status == C2D_STATUS_OK)
            {
                c2d_status = c2dFlush(target_id, &c2d_timestamp);
            }
            pthread_mutex_unlock(&gCtxt.mutex_c2d);

            if (c2d_status == C2D_STATUS_OK)
            {
                c2d_status = c2dWaitTimestamp(c2d_timestamp);
            }

            QCARCAM_DBGMSG("c2d conversion finished");

            if (c2d_status != C2D_STATUS_OK)
            {
                QCARCAM_ERRORMSG("c2d conversion failed with error %d", c2d_status);
            }
        }
#ifdef ENABLE_CL_CONVERTER
        else
        {
            //Use open CL to convert
            csc_run(input_ctxt->g_converter, input_ctxt->buf_idx_qcarcam, input_ctxt->buf_idx_disp, NULL);
            csc_wait(input_ctxt->g_converter, 0, NULL);
        }
#else
#ifdef POST_PROCESS
    else{
            if(gCtxt.enable_post_processing)
            {
                pp_job_t pp_job = {};
                pp_job.frame_id = input_ctxt->buf_idx_qcarcam;
                pp_job.src_buf_idx[0] = input_ctxt->buf_idx_qcarcam;
                pp_job.tgt_buf_idx[0] =  input_ctxt->buf_idx_disp;

                test_util_post_processing_process_frame(&gCtxt.post_processing_ctx[0], &pp_job);
            }
    }
#endif
#endif

        /**********************
         * Dump if necessary
         ********************** */
        if (0 != gCtxt.dumpFrame)
        {
            if (0 == input_ctxt->frameCnt % gCtxt.dumpFrame)
            {
                snprintf(g_filename, sizeof(g_filename), DEFAULT_DUMP_LOCATION "frame_display_%d_%i.raw", input_ctxt->idx, input_ctxt->frameCnt);
                test_util_dump_window_buffer(input_ctxt->test_util_ctxt, input_ctxt->display_window, input_ctxt->buf_idx_disp, g_filename);
            }
        }
        /**********************
         * Post to screen
         ********************** */
        QCARCAM_DBGMSG("Post Frame %d", input_ctxt->buf_idx_disp);
        ret = test_util_post_window_buffer(input_ctxt->test_util_ctxt,
                                           input_ctxt->display_window,
                                           input_ctxt->buf_idx_disp,
                                           &input_ctxt->release_buf_idx,
                                           input_ctxt->field_type_previous);

        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("test_util_post_window_buffer failed");
        }

        input_ctxt->buf_idx_disp++;
        input_ctxt->buf_idx_disp %= input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].n_buffers;
#ifndef __INTEGRITY
//qcarcam buffer is already added to release queue in test_util_post_window_buffer().
        input_ctxt->release_buf_idx.push_back(input_ctxt->buf_idx_qcarcam);
#endif
    }
#endif
    QCARCAM_DBGMSG("Post Frame after buf_idx %i", input_ctxt->buf_idx_qcarcam);

    return 0;
}
/**
 * Release frame back to qcarcam
 * @param input_ctxt
 * @return 0 on success, -1 on failure
 */
static int qcarcam_test_release_frame(qcarcam_test_input_t *input_ctxt)
{
    QCarCamRet_e ret;

    if (gCtxt.disable_display || input_ctxt->window_params.is_offscreen)
    {
        /* should release current buffer back to HW immediately if needn't display */
        ret = QCarCamReleaseFrame(input_ctxt->qcarcam_hndl, 0, input_ctxt->buf_idx_qcarcam);
        if (QCARCAM_RET_OK != ret)
        {
            QCARCAM_ERRORMSG("QCarCamReleaseFrame(%d) failed %d", input_ctxt->buf_idx_qcarcam, ret);
            return -1;
        }

        input_ctxt->buf_state[input_ctxt->buf_idx_qcarcam] = QCARCAMTEST_BUFFER_STATE_QCARCAM;
        input_ctxt->releaseframeCnt++;
    }
    else
    {
        while(!input_ctxt->release_buf_idx.empty())
        {
            uint32 rel_idx = input_ctxt->release_buf_idx.front();
            input_ctxt->release_buf_idx.pop_front();

            if (rel_idx < input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].n_buffers)
            {
                if (QCARCAMTEST_BUFFER_STATE_GET_FRAME == (input_ctxt->buf_state[rel_idx] & 0xF) ||
                    QCARCAMTEST_BUFFER_STATE_POST_DISPLAY == (input_ctxt->buf_state[rel_idx] & 0xF))
                {
                    ret = QCarCamReleaseFrame(input_ctxt->qcarcam_hndl, 0, rel_idx);
                    if (QCARCAM_RET_OK != ret)
                    {
                        QCARCAM_ERRORMSG("QCarCamReleaseFrame(%d) failed %d", rel_idx, ret);
                        return -1;
                    }
                    input_ctxt->releaseframeCnt++;
                    input_ctxt->buf_state[rel_idx] = QCARCAMTEST_BUFFER_STATE_QCARCAM;
                }
                else
                {
                    QCARCAM_ERRORMSG("QCarCamReleaseFrame(%d) skipped since buffer bad state (%d)", rel_idx, input_ctxt->buf_state[rel_idx]);
                }
            }
            else
            {
                QCARCAM_ERRORMSG("QCarCamReleaseFrame(%d) skipped", rel_idx);
            }
        }
    }

    return 0;
}

static void usr_process_cb(void *arg)
{
    timer_usr_data * usr_data = (timer_usr_data *)arg;
    qcarcam_test_input_t *input_ctxt = (qcarcam_test_input_t *)usr_data->p_data;
    uint32 buf_idx = usr_data->buf_idx;

    pthread_mutex_lock(&input_ctxt->mutex);
    if (input_ctxt->buf_state[buf_idx] != QCARCAMTEST_BUFFER_STATE_USR_PROCESS)
    {
        QCARCAM_ERRORMSG("buf (%d) in error state %d", buf_idx, input_ctxt->buf_state[buf_idx]);
    }

    input_ctxt->buf_state[buf_idx] = QCARCAMTEST_BUFFER_STATE_USR_PROCESS_DONE;
    input_ctxt->usr_process_buf_idx.push_back(buf_idx);
    input_ctxt->usr_process_frameCnt--;

    QCARCAM_DBGMSG("buf (%d) usr process done, usr_process_frameCnt %d", buf_idx, input_ctxt->usr_process_frameCnt);

    pthread_mutex_unlock(&input_ctxt->mutex);

}

/**
 * Function to handle routine of fetching, displaying, and releasing frames when one is available
 * @param input_ctxt
 * @return 0 on success, -1 on failure
 */
static int qcarcam_test_handle_new_frame(qcarcam_test_input_t *input_ctxt)
{
#ifdef ACCESS_BUF_VA
    uint32 *pbuf = NULL;
    test_util_buf_ptr_t buffer = {};
#endif

    if (qcarcam_test_get_frame(input_ctxt))
    {
        /*if we fail to get frame, we silently continue...*/
        return 0;
    }

    if (gCtxt.enablePauseResume && 0 == (input_ctxt->frameCnt % gCtxt.enablePauseResume))
    {
        QCarCamRet_e ret = QCARCAM_RET_OK;

        if (input_ctxt->delay_time)
            qcarcam_test_clear_usr_process_list(input_ctxt);

        //release frame before pause
        qcarcam_test_release_frame(input_ctxt);

        pthread_mutex_lock(&input_ctxt->mutex);
        if (input_ctxt->state != QCARCAMTEST_STATE_CLOSED)
        {
            QCARCAM_INFOMSG("pause...");
            ret = qcarcam_input_pause(input_ctxt);
            if (ret == QCARCAM_RET_OK)
            {
                pthread_mutex_unlock(&input_ctxt->mutex);

                usleep(PAUSE_RESUME_USLEEP);

                pthread_mutex_lock(&input_ctxt->mutex);
                if (input_ctxt->state != QCARCAMTEST_STATE_CLOSED)
                {
                    QCARCAM_INFOMSG("resume...");
                    ret = qcarcam_input_resume(input_ctxt);
                }
            }
        }
        pthread_mutex_unlock(&input_ctxt->mutex);

        return ret;
    }
    else if (gCtxt.enableStartStop && 0 == (input_ctxt->frameCnt % gCtxt.enableStartStop))
    {
        QCarCamRet_e ret = QCARCAM_RET_OK;

        if (input_ctxt->delay_time)
            qcarcam_test_clear_usr_process_list(input_ctxt);

        pthread_mutex_lock(&input_ctxt->mutex);
        if (input_ctxt->state != QCARCAMTEST_STATE_CLOSED)
        {
            QCARCAM_INFOMSG("stop...");
            ret = qcarcam_input_stop(input_ctxt);
            if (ret == QCARCAM_RET_OK)
            {
                pthread_mutex_unlock(&input_ctxt->mutex);

                usleep(START_STOP_USLEEP);

                pthread_mutex_lock(&input_ctxt->mutex);
                if (input_ctxt->state != QCARCAMTEST_STATE_CLOSED)
                {
                    QCARCAM_INFOMSG("re-start...");
                    ret = qcarcam_input_start(input_ctxt);
                }
            }
        }
        pthread_mutex_unlock(&input_ctxt->mutex);
        return ret;
    }
    if (input_ctxt->delay_time)
    {
        uint32 idx = input_ctxt->get_frame_buf_idx;
        if (!input_ctxt->buf_timer[idx].ptimer)
        {
            input_ctxt->buf_timer[idx].usr_data.buf_idx = idx;
            input_ctxt->buf_timer[idx].usr_data.p_data = input_ctxt;

            if (CameraCreateTimer(input_ctxt->delay_time, 0, usr_process_cb, &input_ctxt->buf_timer[idx].usr_data, &input_ctxt->buf_timer[idx].ptimer))
            {
                QCARCAM_ERRORMSG("CameraCreateTimer failed for buf %d", idx);
                return -1;
            }
        }
        else
        {
            if (CameraUpdateTimer(input_ctxt->buf_timer[idx].ptimer, input_ctxt->delay_time))
            {
                QCARCAM_ERRORMSG("CameraUpdateTimer failed for buf %d", idx);
                return -1;
            }
        }

        pthread_mutex_lock(&input_ctxt->mutex);
        input_ctxt->usr_process_frameCnt++;
        input_ctxt->buf_state[idx] = QCARCAMTEST_BUFFER_STATE_USR_PROCESS;
        pthread_mutex_unlock(&input_ctxt->mutex);

        return 0;
    }

    input_ctxt->buf_idx_qcarcam = input_ctxt->get_frame_buf_idx;
#ifdef ACCESS_BUF_VA
    buffer.buf_idx = input_ctxt->buf_idx_qcarcam;
    test_util_get_buf_ptr(input_ctxt->qcarcam_window, &buffer);
    pbuf = (uint32*)buffer.p_va[0];
    if (NULL == pbuf)
    {
        QCARCAM_ERRORMSG("NULL Pointer");
    }
    else
    {
        QCARCAM_ERRORMSG("Embedded frame sequence no = %d", pbuf[0]);
    }
#endif
    if (!input_ctxt->skip_post_display)
    {
        qcarcam_test_post_to_display(input_ctxt);
    }

    if (qcarcam_test_release_frame(input_ctxt))
    {
        return -1;
    }

    return 0;
}

static int inpt_ctxt_usr_process_thread(void *arg)
{
    qcarcam_test_input_t *input_ctxt = (qcarcam_test_input_t *)arg;
    pthread_detach(pthread_self());

    while (!g_aborted)
    {
        pthread_mutex_lock(&input_ctxt->mutex);
        if(!input_ctxt->usr_process_buf_idx.empty() && input_ctxt->state == QCARCAMTEST_STATE_START)
        {
            input_ctxt->buf_idx_qcarcam = input_ctxt->usr_process_buf_idx.front();
            input_ctxt->usr_process_buf_idx.pop_front();
            pthread_mutex_unlock(&input_ctxt->mutex);

            if (!input_ctxt->skip_post_display)
                qcarcam_test_post_to_display(input_ctxt);

            qcarcam_test_release_frame(input_ctxt);

            pthread_mutex_lock(&input_ctxt->mutex);
        }

        pthread_mutex_unlock(&input_ctxt->mutex);
        usleep(USR_PROCESS_THREAD_USLEEP);
    }
    return 0;
}

static int qcarcam_test_handle_input_signal(qcarcam_test_input_t *input_ctxt, QCarCamInputSignal_e signal_type)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    switch (signal_type) {
    case QCARCAM_INPUT_SIGNAL_LOST:
        QCARCAM_ERRORMSG("LOST: idx: %d, input: %d", input_ctxt->idx, input_ctxt->qcarcam_input_id);

        /*TODO: offload this to other thread to handle restart recovery*/
        pthread_mutex_lock(&input_ctxt->mutex);
        if (input_ctxt->state == QCARCAMTEST_STATE_STOP) {
            QCARCAM_ERRORMSG("Input %d already stop, break", input_ctxt->qcarcam_input_id);
            pthread_mutex_unlock(&input_ctxt->mutex);
            break;
        }

        input_ctxt->signal_lost = 1;
        qcarcam_input_stop(input_ctxt);

        pthread_mutex_unlock(&input_ctxt->mutex);

        break;
    case QCARCAM_INPUT_SIGNAL_VALID:
        QCARCAM_ERRORMSG("VALID: idx: %d, input: %d", input_ctxt->idx, input_ctxt->qcarcam_input_id);

        pthread_mutex_lock(&input_ctxt->mutex);
        if (input_ctxt->state == QCARCAMTEST_STATE_START)
        {
            QCARCAM_ERRORMSG("Input %d already running, break", input_ctxt->qcarcam_input_id);
            pthread_mutex_unlock(&input_ctxt->mutex);
            break;
        }

        ret = qcarcam_input_start(input_ctxt);

        pthread_mutex_unlock(&input_ctxt->mutex);

        break;
    default:
        QCARCAM_ERRORMSG("Unknown Event type: %d", signal_type);
        break;
    }

    return ret;
}

static int qcarcam_test_handle_fatal_error(qcarcam_test_input_t *input_ctxt, boolean recover)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    QCARCAM_ERRORMSG("Fatal error: client idx - %d, input id - %d", input_ctxt->idx, input_ctxt->qcarcam_input_id);

    pthread_mutex_lock(&input_ctxt->mutex);

    input_ctxt->signal_lost = 1;
    input_ctxt->fatal_err_cnt++;

    if (input_ctxt->state == QCARCAMTEST_STATE_ERROR || input_ctxt->state == QCARCAMTEST_STATE_STOP) {
        QCARCAM_ERRORMSG("Input %d already error state, return", input_ctxt->qcarcam_input_id);
        pthread_mutex_unlock(&input_ctxt->mutex);
        return ret;
    }

    if (recover)
    {
        ret = QCarCamStop(input_ctxt->qcarcam_hndl);
        if (ret == QCARCAM_RET_OK) {
            input_ctxt->state = QCARCAMTEST_STATE_STOP;
            QCARCAM_INFOMSG("Client %d Input %d QCarCamStop successfully", input_ctxt->idx, input_ctxt->qcarcam_input_id);
        }
        else
        {
            input_ctxt->state = QCARCAMTEST_STATE_ERROR;
            QCARCAM_ERRORMSG("Client %d Input %d QCarCamStop failed: %d !", input_ctxt->idx, input_ctxt->qcarcam_input_id, ret);
        }
    }
    else
    {
        input_ctxt->state = QCARCAMTEST_STATE_ERROR;
    }

    pthread_mutex_unlock(&input_ctxt->mutex);

    return ret;
}

static int qcarcam_test_handle_set_event_notification(qcarcam_test_input_t *input_ctxt, QCarCamParamType_e evnt_type)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    switch(evnt_type)
    {
    case QCARCAM_SENSOR_PARAM_EXPOSURE:
    case QCARCAM_SENSOR_PARAM_SATURATION:
    case QCARCAM_SENSOR_PARAM_HUE:
    case QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL:
    case QCARCAM_SENSOR_PARAM_GAMMA:
        QCARCAM_INFOMSG("Input_ctxt is notified for changed setting:%d input_id:%d client_id:%d",
            evnt_type, input_ctxt->qcarcam_input_id, input_ctxt->idx);
        break;
    case QCARCAM_STREAM_CONFIG_PARAM_MASTER:
        QCARCAM_INFOMSG("Master released input_id:%d client_id:%d",
            input_ctxt->qcarcam_input_id, input_ctxt->idx);
        break;
    default:
        QCARCAM_ERRORMSG("Input_ctxt is notified with unknown event");
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
static QCarCamRet_e qcarcam_test_event_cb(const QCarCamHndl_t hndl,
        const uint32_t eventId,
        const QCarCamEventPayload_t *pPayload,
        void  *pPrivateData)
{
    int result = 0;
    qcarcam_test_input_t *input_ctxt = (qcarcam_test_input_t*)pPrivateData;
    qcarcam_event_msg_t event_msg;

    if (!input_ctxt || hndl != input_ctxt->qcarcam_hndl)
    {
        QCARCAM_ERRORMSG("event_cb called with invalid qcarcam handle %p", hndl);
        return QCARCAM_RET_FAILED;
    }

    if (g_aborted)
    {
        QCARCAM_ERRORMSG("Test aborted");
        return QCARCAM_RET_OK;
    }

    event_msg.event_id = (uint32_t)eventId;

    memcpy(&event_msg.payload, pPayload, sizeof(QCarCamEventPayload_t));

    pthread_mutex_lock(&input_ctxt->queue_mutex);
    input_ctxt->eventqueue.push(event_msg);
    pthread_mutex_unlock(&input_ctxt->queue_mutex);

    result = CameraSetSignal(input_ctxt->m_eventHandlerSignal);
    if (result)
    {
        QCARCAM_ERRORMSG("Failed to signal event %d (%d)", eventId, result);
    }

    return QCARCAM_RET_OK;
}

static int injection_thread(void *arg)
{
    qcarcam_test_input_t *input_ctxt = (qcarcam_test_input_t *)arg;

    CameraWaitOnSignal(input_ctxt->m_injectionHandlerSignal, CAM_SIGNAL_WAIT_NO_TIMEOUT);
    while (!g_aborted)
    {
        uint32 sleepTime = (1.0f / input_ctxt->injectionParams.framerate) * 1000000;
        CameraMicroSleep(sleepTime);
        if (input_ctxt->is_injection)
        {
            boolean inject = FALSE;

            if (input_ctxt->injectionParams.repeat != -1)
            {
                if (input_ctxt->injectionSettings.n_total_frames > 0)
                {
                    input_ctxt->injectionSettings.n_total_frames--;
                    inject = TRUE;
                }
            }
            else
            {
                inject = TRUE;
            }

            if (inject)
            {

                QCarCamRequest_t  request = {};
                request.inputBufferIdx = input_ctxt->injectionSettings.buf_idx;
                request.requestId = input_ctxt->injectionSettings.request_id;

                //update buf_index and request id for next submission
                input_ctxt->injectionSettings.buf_idx =
                    (input_ctxt->injectionSettings.buf_idx + 1) % input_ctxt->injectionParams.n_buffers;
                input_ctxt->injectionSettings.request_id++;

                if(QCARCAMTEST_STATE_START == input_ctxt->state)
                {
                    QCarCamRet_e ret;

                    QCARCAM_DBGMSG("SUBMIT_REQUEST: ctx[0x%p]: bufIdx %d reqId %d",
                            input_ctxt, request.inputBufferIdx, request.requestId);
                    ret = QCarCamSubmitRequest(input_ctxt->qcarcam_hndl, &request);
                    if (QCARCAM_RET_OK != ret)
                    {
                        QCARCAM_ERRORMSG("QCarCamSubmitRequest ctx[0x%p]: bufIdx %d reqId %d failed %d",
                                input_ctxt, request.inputBufferIdx, request.requestId, ret);
                    }
                }
            }
            else
            {
                // Abort test once injection of all frames was done
                // Need to add a sleep of 1 sec because there are two
                // events that could occur simultaneously: buffer ready
                // and vendor params report.
                CameraMicroSleep(1000000);
                abort_test();
            }
        }
    }

    return 0;
}

static int qcarcam_test_handle_request_mode(qcarcam_test_input_t *input_ctxt, QCarCamFrameInfo_t *pframeinfo)
{
    int ret = 0;
    QCarCamFrameInfo_t frame_info = *pframeinfo;

    if (input_ctxt->frameCnt == 0)
    {
        qcarcam_test_get_time(&input_ctxt->t_firstFrame);

        if (input_ctxt->is_first_start)
        {
            ais_log_kpi(AIS_EVENT_KPI_CLIENT_FIRST_FRAME);

            printf("Success - First Frame [%d:%d]\n", input_ctxt->idx, input_ctxt->qcarcam_input_id);
            fflush(stdout);

            QCARCAM_ALWZMSG("[%u:%u] First Frame buf_idx %d after : %lu ms (field type: %d)",
                    input_ctxt->idx, (unsigned int)input_ctxt->qcarcam_input_id, frame_info.bufferIndex, (input_ctxt->t_firstFrame - gCtxt.t_start), frame_info.fieldType);

            input_ctxt->is_first_start = FALSE;
        }
        else
        {
            QCARCAM_ALWZMSG("[%d:%d] restart took %llu ms",
                    input_ctxt->idx, input_ctxt->qcarcam_input_id, (input_ctxt->t_firstFrame - input_ctxt->t_start));
        }
    }

    input_ctxt->get_frame_buf_idx = frame_info.bufferIndex;
    input_ctxt->buf_state[input_ctxt->get_frame_buf_idx] = QCARCAMTEST_BUFFER_STATE_GET_FRAME;

    input_ctxt->signal_lost = 0;

    input_ctxt->diag.frame_generate_time[TEST_PREV_BUFFER] = input_ctxt->diag.frame_generate_time[TEST_CUR_BUFFER];
    input_ctxt->diag.frame_generate_time[TEST_CUR_BUFFER] = frame_info.timestamp * NS_TO_MS;

    QCARCAM_DBGMSG("[%d] framecnt:%d (seqNo %u) bufId:%d qtime:%llu requsetid=%d",
            input_ctxt->idx, input_ctxt->frameCnt, frame_info.seqNo,
        frame_info.bufferIndex, frame_info.sofTimestamp.timestamp, frame_info.requestId);

    input_ctxt->frameCnt++;

    input_ctxt->buf_idx_qcarcam = input_ctxt->get_frame_buf_idx;
    if (!input_ctxt->skip_post_display)
    {
        qcarcam_test_post_to_display(input_ctxt);
    }

    if (gCtxt.disable_display || input_ctxt->window_params.is_offscreen)
    {

        QCarCamRequest_t request = {};
        request.requestId = input_ctxt->submitrequestSettings.request_id;
        request.streamRequests[0].bufferlistId = QCARCAM_BUFFERLIST_ID_OUTPUT_0;
        request.streamRequests[0].bufferId = frame_info.bufferIndex;

        input_ctxt->submitrequestSettings.request_id++;

        ret = QCarCamSubmitRequest(input_ctxt->qcarcam_hndl, &request);
        QCARCAM_DBGMSG("req id %d, buf id %d, ret %d", request.requestId, request.streamRequests[0].bufferId, ret);

        if (QCARCAM_RET_OK != ret)
        {
            QCARCAM_ERRORMSG("QCarCamSubmitRequest req id %d, failed %d", request.requestId, ret);
        }

        input_ctxt->buf_state[input_ctxt->buf_idx_qcarcam] = QCARCAMTEST_BUFFER_STATE_QCARCAM;
        input_ctxt->releaseframeCnt++;
    }
    else
    {
        while(!input_ctxt->release_buf_idx.empty())
        {
            uint32 rel_idx = input_ctxt->release_buf_idx.front();
            input_ctxt->release_buf_idx.pop_front();

            if (rel_idx < input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].n_buffers)
            {
                if (QCARCAMTEST_BUFFER_STATE_GET_FRAME == (input_ctxt->buf_state[rel_idx] & 0xF) ||
                    QCARCAMTEST_BUFFER_STATE_POST_DISPLAY == (input_ctxt->buf_state[rel_idx] & 0xF))
                {
                    QCarCamRequest_t request = {};
                    request.requestId = input_ctxt->submitrequestSettings.request_id;
                    request.streamRequests[0].bufferlistId = QCARCAM_BUFFERLIST_ID_OUTPUT_0;
                    request.streamRequests[0].bufferId = rel_idx;

                    input_ctxt->submitrequestSettings.request_id++;

                    ret = QCarCamSubmitRequest(input_ctxt->qcarcam_hndl, &request);
                    QCARCAM_DBGMSG("req id %d, buf id %d, ret %d", request.requestId, request.streamRequests[0].bufferId, ret);

                    if (QCARCAM_RET_OK != ret)
                    {
                        QCARCAM_ERRORMSG("QCarCamSubmitRequest req id %d, failed %d", request.requestId, ret);
                    }
                    input_ctxt->releaseframeCnt++;
                    input_ctxt->buf_state[rel_idx] = QCARCAMTEST_BUFFER_STATE_QCARCAM;
                }
                else
                {
                    QCARCAM_ERRORMSG("QCarCamSubmitRequest(%d) skipped since buffer bad state (%d)", rel_idx, input_ctxt->buf_state[rel_idx]);
                }
            }
            else
            {
                QCARCAM_ERRORMSG("QCarCamSubmitRequest(%d) skipped", rel_idx);
            }
        }
    }

    return 0;
}


static int process_cb_event_thread(void *arg)
{
    qcarcam_test_input_t *input_ctxt = (qcarcam_test_input_t *)arg;
    qcarcam_event_msg_t event_msg;

    while (!g_aborted)
    {
        CameraWaitOnSignal(input_ctxt->m_eventHandlerSignal, CAM_SIGNAL_WAIT_NO_TIMEOUT);

        pthread_mutex_lock(&input_ctxt->queue_mutex);
        if (!input_ctxt->eventqueue.empty())
        {
            event_msg = input_ctxt->eventqueue.front();
            input_ctxt->eventqueue.pop();
        }
        else
        {
            QCARCAM_ERRORMSG("event queue is empty");
            pthread_mutex_unlock(&input_ctxt->queue_mutex);
            continue;
        }

        pthread_mutex_unlock(&input_ctxt->queue_mutex);

        switch (event_msg.event_id)
        {
            case QCARCAM_EVENT_FRAME_READY:
            {
                if (input_ctxt->state == QCARCAMTEST_STATE_START)
                {
                    QCARCAM_DBGMSG("%d received QCARCAM_EVENT_FRAME_READY", input_ctxt->idx);

                    if (!input_ctxt->request_mode)
                    {
                        qcarcam_test_handle_new_frame(input_ctxt);
                    }
                    else //test for request_mode
                    {
                        qcarcam_test_handle_request_mode(input_ctxt, &event_msg.payload.frameInfo);
                    }
                }
                break;
            }
            case QCARCAM_EVENT_INPUT_SIGNAL:
            {
                if (gCtxt.enableBridgeErrorDetect)
                {
                    qcarcam_test_handle_input_signal(input_ctxt, (QCarCamInputSignal_e)event_msg.payload.u32Data);
                }
                break;
            }
            case QCARCAM_EVENT_ERROR:
            {
                switch (event_msg.payload.errInfo.errorId)
                {
                case QCARCAM_ERROR_FATAL:
                    QCARCAM_ERRORMSG("FATAL error %d on id:%d qid:%d",
                            event_msg.payload.errInfo.errorCode, input_ctxt->idx, input_ctxt->qcarcam_input_id);
                    qcarcam_test_handle_fatal_error(input_ctxt, gCtxt.enableFatalErrorRecover);
                    break;
                case QCARCAM_ERROR_WARNING:
                    QCARCAM_ERRORMSG("WARNING error %d on id:%d qid:%d",
                            event_msg.payload.errInfo.errorCode, input_ctxt->idx, input_ctxt->qcarcam_input_id);
                    break;
                default:
                    break;
                }

                break;
            }
            case QCARCAM_EVENT_VENDOR:
            {
                QCARCAM_INFOMSG("Frame[%d]: receive QCARCAM_EVENT_VENDOR data[0]=%u data[1]=%u",
                input_ctxt->frameCnt, event_msg.payload.vendorData.data[0], event_msg.payload.vendorData.data[1]);
                if (input_ctxt->is_injection)
                {
                    if (QCARCAM_ISP_USECASE_SHDR_PREPROCESS == input_ctxt->isp_config[0].usecaseId)
                    {
                        QCarCamExposureConfig_t* pHdrExposure =
                            (QCarCamExposureConfig_t*)(event_msg.payload.vendorData.data);
                        QCARCAM_INFOMSG("Frame[%d]: Injection: Set HDR exposure: mode %d exposure_time (%f,%f,%f) gain (%f,%f,%f)",
                            input_ctxt->frameCnt, pHdrExposure->mode, pHdrExposure->exposureTime[0],
                            pHdrExposure->exposureTime[1], pHdrExposure->exposureTime[2],
                            pHdrExposure->gain[0], pHdrExposure->gain[1], pHdrExposure->gain[2]);
                    }
                }
                break;
            }
            case QCARCAM_EVENT_PROPERTY_NOTIFY:
                qcarcam_test_handle_set_event_notification(input_ctxt, (QCarCamParamType_e)event_msg.payload.u32Data);
                break;
            case QCARCAM_EVENT_FRAME_SOF:
                QCARCAM_INFOMSG("received QCARCAM_EVENT_FRAME_SOF for id:%d qid:%d, qtimestamp:%llu",
                input_ctxt->idx, input_ctxt->qcarcam_input_id, event_msg.payload.hwTimestamp.timestamp);
                break;
            case QCARCAM_EVENT_RECOVERY:
                QCARCAM_INFOMSG("Client%d QCARCAM_EVENT_ID_RECOVERY", input_ctxt->idx);

                switch (event_msg.payload.recovery.msg)
                {
                case QCARCAM_RECOVERY_STARTED:
                    QCARCAM_INFOMSG("Client%d RECOVERY START", input_ctxt->idx);
                    input_ctxt->state = QCARCAMTEST_STATE_RECOVERY;
                    break;
                case QCARCAM_RECOVERY_SUCCESS:
                    QCARCAM_INFOMSG("Client%d RECOVERY SUCCESS", input_ctxt->idx);
                    input_ctxt->state = QCARCAMTEST_STATE_START;
                    break;
                case QCARCAM_RECOVERY_FAILED:
                    QCARCAM_INFOMSG("Client%d RECOVERY FAILED", input_ctxt->idx);
                    input_ctxt->state = QCARCAMTEST_STATE_ERROR;
                    break;
                default:
                    break;
                }
                break;

            default:
                QCARCAM_ERRORMSG("%d received unsupported event %d", input_ctxt->idx, event_msg.event_id);
                break;
        }
    }

    return 0;
}



/**
 * Qcarcam gpio interrupt callback function for visibility mode
 */
static void qcarcam_test_gpio_interrupt_cb()
{
    int idx;
    unsigned long long t_before = 0;
    unsigned long long t_after = 0;

    gCtxt.vis_value = !gCtxt.vis_value;

    qcarcam_test_get_time(&t_before);

    for (idx = 0; idx < gCtxt.numInputs; ++idx)
    {
        qcarcam_test_input_t *input_ctxt = &gCtxt.inputs[idx];

        if (input_ctxt->qcarcam_window != NULL)
        {
            test_util_set_param(input_ctxt->qcarcam_window, TEST_UTIL_VISIBILITY, gCtxt.vis_value);
        }

        if (input_ctxt->display_window != NULL)
        {
            test_util_set_param(input_ctxt->display_window, TEST_UTIL_VISIBILITY, gCtxt.vis_value);
        }

        input_ctxt->window_params.visibility = gCtxt.vis_value;
    }

    qcarcam_test_get_time(&t_after);
    QCARCAM_PERFMSG(gCtxt.enableStats, "Time for visbility toggle : %lu ms", (t_after - t_before));
}

/**
 * Qcarcam gpio interrupt callback function for play/pause mode
 */
static void qcarcam_test_gpio_play_pause_cb()
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    int stream_id;
    unsigned long long t_before = 0;
    unsigned long long t_after = 0;

    qcarcam_test_get_time(&t_before);

    for (stream_id = 0; stream_id < gCtxt.numInputs; ++stream_id)
    {
        qcarcam_test_input_t *input_ctxt = &gCtxt.inputs[stream_id];

        pthread_mutex_lock(&input_ctxt->mutex);

        if (input_ctxt->state == QCARCAMTEST_STATE_PAUSE)
        {
            ret = qcarcam_input_resume(input_ctxt);
        }
        else if (input_ctxt->state == QCARCAMTEST_STATE_START)
        {
            ret = qcarcam_input_pause(input_ctxt);
        }
        else
        {
            QCARCAM_ERRORMSG("bad state %d", input_ctxt->state);
            ret = QCARCAM_RET_BADSTATE;
        }

        pthread_mutex_unlock(&input_ctxt->mutex);

        if (ret)
        {
            QCARCAM_ERRORMSG("failed gpio toggle (%d)", input_ctxt->state);
        }
    }

    qcarcam_test_get_time(&t_after);
    QCARCAM_PERFMSG(gCtxt.enableStats, "Time for play/pause toggle : %lu ms",  (t_after - t_before));
}

/**
 * Qcarcam gpio interrupt callback function for stop/start mode
 */
static void qcarcam_test_gpio_start_stop_cb()
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    int stream_id;
    unsigned long long t_before = 0;
    unsigned long long t_after = 0;

    qcarcam_test_get_time(&t_before);

    for (stream_id = 0; stream_id < gCtxt.numInputs; ++stream_id)
    {
        qcarcam_test_input_t *input_ctxt = &gCtxt.inputs[stream_id];

        pthread_mutex_lock(&input_ctxt->mutex);

        if (input_ctxt->state == QCARCAMTEST_STATE_STOP)
        {
            ret = qcarcam_input_start(input_ctxt);
        }
        else if (input_ctxt->state == QCARCAMTEST_STATE_START)
        {
            ret = qcarcam_input_stop(input_ctxt);
        }
        else
        {
            QCARCAM_ERRORMSG("bad state %d", input_ctxt->state);
            ret = QCARCAM_RET_BADSTATE;
        }

        pthread_mutex_unlock(&input_ctxt->mutex);

        if (ret)
        {
            QCARCAM_ERRORMSG("failed gpio toggle (%d)", input_ctxt->state);
        }
    }

    qcarcam_test_get_time(&t_after);
    QCARCAM_PERFMSG(gCtxt.enableStats, "Time for start/stop toggle : %lu ms", (t_after - t_before));
}

/**
 * API to set/release a client as master
 *
 *
 * @param qcarcam_test_input_t* input_ctxt
 * @param unsigned int flag to set(1)/release(0) master
 */

static void qcarcam_test_set_master(qcarcam_test_input_t *input_ctxt, unsigned int flag)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    uint32_t param = flag;
    ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_STREAM_CONFIG_PARAM_MASTER, &param, sizeof(param));
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("This client can't be set(1)/release(0) master:%d, it's not fatal", flag);
    }
    else
    {
        if (flag == 1)
        {
            input_ctxt->is_master = true;
            printf("Client [Stream ID %u  Input ID %u] has been set as master \n",
                    input_ctxt->idx, (unsigned int)input_ctxt->qcarcam_input_id);
        }
        else
        {
            input_ctxt->is_master = false;
            printf("Client [Stream ID %u  Input ID %u] has been released \n",
                    input_ctxt->idx, (unsigned int)input_ctxt->qcarcam_input_id);
        }
    }
}

/**
 * API to subscribe for events to get notifications
 *
 *
 * @param qcarcam_test_input_t* input_ctxt
 * @param change_events  Bitmask of events
 */

static int qcarcam_test_subscribe_input_params_change(qcarcam_test_input_t *input_ctxt, uint64 change_events)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    uint64_t param = change_events;

    ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_SUBSCRIBE, &param, sizeof(param));
    if(ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("QCarCamSetParam(QCARCAM_PARAM_EVENT_SUBSCRIBE) failed for events:%llu with ret:%d", change_events, ret);
        return -1;
    }
    return 0;
}

/**
 * API to unsubscribe for events to get notifications
 *
 *
 * @param qcarcam_test_input_t* input_ctxt
 * @param change_events  Bitmask of events
 */

static int qcarcam_test_unsubscribe_input_params_change(qcarcam_test_input_t *input_ctxt, uint64 change_events)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    uint64_t param = change_events;

    ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_UNSUBSCRIBE, &param, sizeof(param));
    if(ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("QCarCamSetParam(QCARCAM_PARAM_EVENT_UNSUBSCRIBE) failed");
        return -1;
    }
    return 0;
}

/**
 * API to subscribe for SOF event notification
 *
 *
 * @param qcarcam_test_input_t* input_ctxt
 */

static int qcarcam_test_subscribe_SOF_event(qcarcam_test_input_t *input_ctxt)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    uint32_t param;

    /* Get the applied mask. */
    ret = QCarCamGetParam(input_ctxt->qcarcam_hndl, QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK, &param, sizeof(param));
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("QCarCamGetParam failed for stream id %d, ret = %d",input_ctxt->qcarcam_input_id, ret);
        return -1;
    }

    /* Set the SOF mask. */
    param |= QCARCAM_EVENT_FRAME_SOF;
    ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK, &param, sizeof(param));
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("QCarCamSetParam failed for stream id %d, ret = %d",input_ctxt->qcarcam_input_id, ret);
        return -1;
    }

    return 0;
}

/**
 * API to unsubscribe for SOF event notification
 *
 *
 * @param qcarcam_test_input_t* input_ctxt
 */

static int qcarcam_test_unsubscribe_SOF_event(qcarcam_test_input_t *input_ctxt)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    uint32_t param;

    /* Get the applied mask. */
    ret = QCarCamGetParam(input_ctxt->qcarcam_hndl, QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK, &param, sizeof(param));
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("QCarCamGetParam failed for stream id %d, ret = %d",input_ctxt->qcarcam_input_id, ret);
        return -1;
    }

    /* Unset the SOF mask. */
    param &= ~QCARCAM_EVENT_FRAME_SOF;
    ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK, &param, sizeof(param));
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("QCarCamSetParam failed for stream id %d, ret = %d",input_ctxt->qcarcam_input_id, ret);
        return -1;
    }

    return 0;
}

/**
 * Thread to setup and run qcarcam based on test input context
 *
 * @note For single threaded operation, this function only sets up qcarcam context.
 *      QCarCamStart and handling of frames is not executed.
 *
 * @param arg qcarcam_test_input_t* input_ctxt
 */
static int qcarcam_test_setup_input_ctxt_thread(void *arg)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    qcarcam_test_input_t *input_ctxt = (qcarcam_test_input_t *)arg;
    unsigned long long t_before = 0;
    unsigned long long t_after = 0;

    if (!input_ctxt)
        return -1;

    QCARCAM_INFOMSG("setup_input_ctxt_thread idx = %d, input_desc=%d", input_ctxt->idx, input_ctxt->qcarcam_input_id);

    qcarcam_test_get_time(&t_before);

    QCarCamOpen_t openParams = {};

    openParams.opMode = input_ctxt->op_mode;
    if (openParams.opMode == QCARCAM_OPMODE_RDI_CONVERSION || openParams.opMode == QCARCAM_OPMODE_PAIRED_INPUT)
    {
        openParams.numInputs = 2;
    }
    else
    {
        openParams.numInputs = 1;
    }

    for (int i = 0; i < openParams.numInputs; i++)
    {
        openParams.inputs[i].inputId = input_ctxt->qcarcam_input_id;
    }

    if (input_ctxt->recovery)
    {
        openParams.flags = QCARCAM_OPEN_FLAGS_RECOVERY;
    }
    else if (input_ctxt->request_mode)
    {
        openParams.flags = QCARCAM_OPEN_FLAGS_REQUEST_MODE;
    }


    ret = QCarCamOpen(&openParams, &input_ctxt->qcarcam_hndl);
    if(ret != QCARCAM_RET_OK || input_ctxt->qcarcam_hndl == QCARCAM_HNDL_INVALID)
    {
        input_ctxt->state = QCARCAMTEST_STATE_ERROR;
        QCARCAM_ERRORMSG("QCarCamOpen() failed");
        goto qcarcam_thread_fail;
    }

    pthread_mutex_lock(&gCtxt.mutex_open_cnt);
    gCtxt.opened_stream_cnt++;
    pthread_mutex_unlock(&gCtxt.mutex_open_cnt);

    input_ctxt->state = QCARCAMTEST_STATE_OPEN;

    qcarcam_test_get_time(&t_after);
    QCARCAM_PERFMSG(gCtxt.enableStats, "QCarCamOpen (idx %u) : %lu ms", input_ctxt->idx, (t_after - t_before));
    t_before = t_after;

    QCARCAM_INFOMSG("render_thread idx = %d, input_desc=%d context=%p",
            input_ctxt->idx, input_ctxt->qcarcam_input_id, input_ctxt->qcarcam_hndl);

    //@TODO: change to set sensor mode id
#if 0
    // For HDMI/CVBS Input
    // NOTE: set HDMI IN resolution in qcarcam_config_single_hdmi.xml before
    // running the test
    if (input_ctxt->qcarcam_input_id == QCARCAM_INPUT_TYPE_DIGITAL_MEDIA)
    {
        QCarCamParamValue_t param;
        param.res_value.width = input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].width;
        param.res_value.height = input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].height;

        ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_PARAM_RESOLUTION, &param, sizeof(param));
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("QCarCamSetParam resolution() failed");
            goto qcarcam_thread_fail;
        }
        qcarcam_test_get_time(&t_after);
        QCARCAM_PERFMSG(gCtxt.enableStats, "QCarCamSetParam resolution (idx %u) : %lu ms", input_ctxt->idx, (t_after - t_before));
        t_before = t_after;
    }
#endif

    ret = QCarCamSetBuffers(input_ctxt->qcarcam_hndl, &input_ctxt->p_buffers_output);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("QCarCamSetBuffers() failed");
        goto qcarcam_thread_fail;
    }

    if (input_ctxt->is_injection)
    {
        QCARCAM_ALWZMSG("read from input_file - set input buffers");

        ret = QCarCamSetBuffers(input_ctxt->qcarcam_hndl, &input_ctxt->p_buffers_input);
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("QCarCamSetBuffers() failed");
            goto qcarcam_thread_fail;
        }
    }

    qcarcam_test_get_time(&t_after);
    QCARCAM_PERFMSG(gCtxt.enableStats, "QCarCamSetBuffers (idx %u) : %lu ms", input_ctxt->idx, (t_after - t_before));
    t_before = t_after;

    QCARCAM_INFOMSG("QCarCamSetBuffers done, QCarCamStart ...");

    //event callback config
    if (input_ctxt->use_event_callback)
    {
        uint32_t param;

        ret = QCarCamRegisterEventCallback(input_ctxt->qcarcam_hndl, &qcarcam_test_event_cb, input_ctxt);
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("QCarCamSetParam(QCARCAM_PARAM_EVENT_CB) failed");
            goto qcarcam_thread_fail;
        }

        if (input_ctxt->recovery)
        {
            param = QCARCAM_EVENT_FRAME_READY |
                QCARCAM_EVENT_INPUT_SIGNAL |
                QCARCAM_EVENT_ERROR | QCARCAM_EVENT_VENDOR |
                QCARCAM_EVENT_RECOVERY;
                // | QCARCAM_EVENT_FRAME_FREEZE;
        }
        else
        {
            param = QCARCAM_EVENT_FRAME_READY |
                QCARCAM_EVENT_INPUT_SIGNAL |
                QCARCAM_EVENT_ERROR | QCARCAM_EVENT_VENDOR;
                // | QCARCAM_EVENT_FRAME_FREEZE;
        }

        ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK, &param, sizeof(param));
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("QCarCamSetParam(QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK) failed");
            goto qcarcam_thread_fail;
        }

        qcarcam_test_get_time(&t_after);
        QCARCAM_PERFMSG(gCtxt.enableStats, "QCarCamSetParam (idx %u) : %lu ms", input_ctxt->idx, (t_after - t_before));
        t_before = t_after;
    }

    //frame drop control
    if (input_ctxt->frameDropConfig.frameDropPeriod != 0)
    {
        QCarCamFrameDropConfig_t* param = &input_ctxt->frameDropConfig;

        ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL, param, sizeof(*param));
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("QCarCamSetParam(QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL) failed");
            goto qcarcam_thread_fail;
        }

        qcarcam_test_get_time(&t_after);
        QCARCAM_PERFMSG(gCtxt.enableStats, "QCarCamSetParam fps (idx %u): %lu ms", input_ctxt->idx, (t_after - t_before));
        t_before = t_after;

        QCARCAM_INFOMSG("Provided QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL: period = %d, pattern = 0x%x",
            param->frameDropPeriod,
            param->frameDropPattern);
    }

    //batch mode config
    if (input_ctxt->num_batch_frames > 1)
    {
        QCarCamBatchConfig_t param = {};

        param.numBatchFrames = input_ctxt->num_batch_frames;
        param.detectFirstPhaseTimer = input_ctxt->detect_first_phase_timer;
        param.frameIncrement = input_ctxt->frame_increment;

        ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE, &param, sizeof(param));
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("QCarCamSetParam(QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE) failed");
            goto qcarcam_thread_fail;
        }

        QCARCAM_INFOMSG("Provided QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE: batch_frames = %d frame_increment %d",
                        param.numBatchFrames,
                        param.frameIncrement);
    }

    //ISP usecase config
    for (uint32 idx = 0; idx < input_ctxt->num_isp_instances; idx++)
    {
        ret = QCarCamSetParam(input_ctxt->qcarcam_hndl,
                QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE,
                &input_ctxt->isp_config[idx],
                sizeof(input_ctxt->isp_config[idx]));
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("QCarCamSetParam(QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE) failed");
            goto qcarcam_thread_fail;
        }

        qcarcam_test_get_time(&t_after);
        QCARCAM_PERFMSG(gCtxt.enableStats, "QCarCamSetParam usecase (idx %u): %lu ms", input_ctxt->idx, (t_after - t_before));
        t_before = t_after;
    }

    /*single threaded handles frames outside this function*/
    if (gCtxt.multithreaded)
    {
        pthread_mutex_lock(&input_ctxt->mutex);

        input_ctxt->is_first_start = TRUE;
        ret = qcarcam_input_start(input_ctxt);

        pthread_mutex_unlock(&input_ctxt->mutex);

        if (input_ctxt->is_injection)
        {
            int result = 0;
            result = CameraSetSignal(input_ctxt->m_injectionHandlerSignal);
            if (result)
            {
                QCARCAM_ERRORMSG("Failed to signal injection thread (%d)", result);
            }
        }

        qcarcam_test_get_time(&t_after);
        QCARCAM_PERFMSG(gCtxt.enableStats, "QCarCamStart (idx %u) : %lu ms", input_ctxt->idx, (t_after - t_before));
        t_before = t_after;

        if (input_ctxt->manual_exposure != -1)
        {
            /* Set exposure configuration */
            QCarCamExposureConfig_t param;
            param.exposureTime[0] = input_ctxt->exp_time;
            param.gain[0] = input_ctxt->gain;
            param.mode = input_ctxt->manual_exposure ? QCARCAM_EXPOSURE_MANUAL : QCARCAM_EXPOSURE_AUTO;

            if (input_ctxt->manual_exposure)
            {
                QCARCAM_INFOMSG("Provided QCARCAM_PARAM_MANUAL_EXPOSURE : time =%f ms, gain = %f", input_ctxt->exp_time, input_ctxt->gain);
            }
            else
            {
                QCARCAM_INFOMSG("AUTO EXPOSURE MODE");
            }

            QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_SENSOR_PARAM_EXPOSURE, &param, sizeof(param));
        }
        else
        {
            QCARCAM_INFOMSG("Exposure is not configured, use default setting");
        }

        qcarcam_test_get_time(&t_after);
        QCARCAM_PERFMSG(gCtxt.enableStats, "qcarcam setexposure (idx %u) : %lu ms", input_ctxt->idx, (t_after - t_before));
        t_before = t_after;

        if (!input_ctxt->use_event_callback)
        {
            while (!g_aborted)
            {
                if (qcarcam_test_handle_new_frame(input_ctxt))
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
    }

    QCARCAM_INFOMSG("exit setup_input_ctxt_thread idx = %d", input_ctxt->idx);
    return 0;

qcarcam_thread_fail:
    if (input_ctxt->qcarcam_hndl)
    {
        QCarCamClose(input_ctxt->qcarcam_hndl);
        input_ctxt->qcarcam_hndl = QCARCAM_HNDL_INVALID;

        input_ctxt->state = QCARCAMTEST_STATE_ERROR;
    }

    return -1;
}

static void display_sensormode_settings(uint32 param)
{
    printf("sensormode = %d\n", param);
}

static void display_color_space_settings(uint32 param)
{
    printf("color space = %d\n",param);
}


static void display_menu()
{
    int i = 0;

    printf("\n ========================================================== \n");

    for (i = QCARCAM_TEST_MENU_FIRST_ITEM; i < QCARCAM_TEST_MENU_MAX; i++)
    {
        printf("'%d'....%s \t\t", g_qcarcam_menu[i].id, g_qcarcam_menu[i].str);

        i++;
        if (i < QCARCAM_TEST_MENU_MAX)
        {
            printf("'%d'....%s\n", g_qcarcam_menu[i].id, g_qcarcam_menu[i].str);
        }
        else
        {
            printf("\n");
            break;
        }
    }

    printf(" 'h'.....display menu \n");
    printf(" 's'.....Dump Image \n");
    printf(" 'e'.....Exit \n");
    printf("\n =========================================================== \n");
    printf(" Enter your choice\n");
}

static qcarcam_test_input_t* get_input_ctxt(int stream_id)
{
    if (stream_id >= 0 && stream_id < gCtxt.numInputs)
    {
        return &gCtxt.inputs[stream_id];
    }

    printf("Wrong stream id entered, please check the input xml\n");

    return NULL;
}

void parse_diagnostics(QCarCamDiagInfo* diagnosticInfo)
{
    if(!diagnosticInfo)
        return;
    //parsing client Diagnostic Info
    QCarCamDiagClientInfo* pDiagClientInfo = diagnosticInfo->aisDiagClientInfo;
    for (uint32 i = 0; i < MAX_USR_CLIENTS; i++)
    {
        qcarcam_test_input_t *input_ctxt = NULL;
        if (!pDiagClientInfo[i].usrHdl)
        {
            break;
        }
        for (uint32 j = 0; j < (uint32)gCtxt.numInputs; j++)
        {
            if (pDiagClientInfo[i].usrHdl == gCtxt.inputs[j].qcarcam_hndl)
            {
                input_ctxt = &gCtxt.inputs[j];
                break;
            }
        }

        if (!input_ctxt)
        {
            QCARCAM_INFOMSG("This handle is invalid %p", pDiagClientInfo[i].usrHdl);
        }
        else
        {
            QCarCamDiagClientInfo *usrInfo = &pDiagClientInfo[i];

            QCARCAM_INFOMSG("usrHdl:%p usr_state:%d inputId:%d opMode:%d inputDevId:%d"
                "csiPhyDevId:%d ifeDevId:%d rdiId:%d timeStampStart:%llu sofCounter:%llu frameCounter:%llu",
                usrInfo->usrHdl, usrInfo->state, usrInfo->inputId, usrInfo->opMode,
                usrInfo->inputDevId, usrInfo->csiphyDevId, usrInfo->ifeDevId, usrInfo->rdiId,
                usrInfo->timeStampStart, usrInfo->sofCounter, usrInfo->frameCounter);
            for (int j = 0; j < QCARCAM_MAX_NUM_BUFFERS; j++)
            {
                QCarCamDiagBufferInfo *bufInfo = &usrInfo->bufInfo[j];
                QCARCAM_INFOMSG("bufId:%d bufStatus:%d", bufInfo->bufId, bufInfo->bufStatus);
            }
        }
    }

    //parsing Input device statistics info
    /*
    QCarCamDiagInputDevInfo* pDiagInputInfo = diagnosticInfo->aisDiagInputDevInfo;
    for(uint32 i = 0; i < MAX_NUM_INPUT_DEVICES; i++)
    {
        QCarCamDiagInputDevInfo* inputInfo = &pDiagInputInfo[i];

        QCARCAM_INFOMSG("DevId:%d numSensors:%d cciDevId:%d cciPortId:%d state:%d srcIdEnableMask:%d", inputInfo->inputDevId, inputInfo->numSensors,
            inputInfo->cciMap.cciDevId, inputInfo->cciMap.cciPortId, inputInfo->state, inputInfo->srcIdEnableMask);

        for (uint32 j = 0; j < (uint32)inputInfo->numSensors; j++)
        {
            QCarCamDiagInputSrcInfo* sourceInfo = &inputInfo->inputSourceInfo[j];

            QCARCAM_INFOMSG("inputSrcId:%d status:%d fps:%.2f width:%d height:%d format:%d",
                sourceInfo->inputSrcId, sourceInfo->status,
                sourceInfo->sensorMode.fps, sourceInfo->sensorMode.res.width,
                sourceInfo->sensorMode.res.height, sourceInfo->sensorMode.colorFmt);

        }
    }
    */
    //parse csiphy device info
    QCarCamDiagCsiDevInfo* pDiagCsiInfo = diagnosticInfo->aisDiagCsiDevInfo;
    for(uint32 i = 0; i < MAX_NUM_CSIPHY_DEVICES; i++)
    {
        QCarCamDiagCsiDevInfo* csiInfo = &pDiagCsiInfo[i];

        QCARCAM_INFOMSG("DevId:%d laneMapping:%x numIfeMap:%d ifeMap:%x", csiInfo->csiphyDevId, csiInfo->csiLaneMapping,
            csiInfo->numIfeMap, csiInfo->ifeMap);
    }

    //parse ife device info
    QCarCamDiagIfeDevInfo* pDiagIfeInfo = diagnosticInfo->aisDiagIfeDevInfo;
    for (uint32 i = 0; i < MAX_NUM_IFE_DEVICES; i++)
    {
        QCarCamDiagIfeDevInfo* ifeInfo = &pDiagIfeInfo[i];

        QCARCAM_INFOMSG("DevId:%d csiDevId:%d numRdi:%d csidPktsReceived:%llu", ifeInfo->ifeDevId, ifeInfo->csiDevId,
            ifeInfo->numRdi, ifeInfo->csidPktsRcvd);

        for(uint32 j = 0; j < (uint32)ifeInfo->numRdi; j++)
        {
            QCarCamDiagRdiInfo *rdiInfo = &ifeInfo->rdiInfo[j];

            QCARCAM_INFOMSG("rdiId:%d rdiStatus:%d", rdiInfo->rdiId, rdiInfo->rdiStatus);
        }
    }

    //parse Error Info
    QCarCamDiagErrorInfo* pDiagErrorInfo = diagnosticInfo->aisDiagErrInfo;
    for(uint32 i = 0; i < MAX_ERR_QUEUE_SIZE; i++)
    {
        QCarCamDiagErrorInfo* errorInfo = &pDiagErrorInfo[i];
        QCARCAM_INFOMSG("errorType:%d errorStatus:%d usrHdl:%p inputSrcId:%d inputDevId:%d csiphyId:%d"
            "ifeDevId:%d rdiId:%d errorTimeStamp:%llu", errorInfo->errorType, errorInfo->payload[0],
            errorInfo->usrHdl, errorInfo->inputSrcId, errorInfo->inputDevId, errorInfo->csiphyDevId,
            errorInfo->ifeDevId, errorInfo->rdiId, errorInfo->errorTimeStamp);
    }
}

static void process_cmds(uint32 option)
{
    int ret = 0;
    int stream_id;
    qcarcam_test_input_t *input_ctxt = NULL;

    switch (option)
    {
    case QCARCAM_TEST_MENU_STREAM_OPEN:
    {
        stream_id = get_closed_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt)
        {
            pthread_mutex_lock(&input_ctxt->mutex);
            ret = qcarcam_input_open(input_ctxt);
            pthread_mutex_unlock(&input_ctxt->mutex);
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_CLOSE:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt)
        {
            pthread_mutex_lock(&input_ctxt->mutex);
            if (input_ctxt->state == QCARCAMTEST_STATE_STOP)
            {
                ret = qcarcam_input_close(input_ctxt);
            }
            pthread_mutex_unlock(&input_ctxt->mutex);
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_START:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt)
        {
            pthread_mutex_lock(&input_ctxt->mutex);
            if (input_ctxt->state == QCARCAMTEST_STATE_STOP || input_ctxt->state == QCARCAMTEST_STATE_OPEN)
            {
                ret = qcarcam_input_start(input_ctxt);
            }
            pthread_mutex_unlock(&input_ctxt->mutex);
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_START_ALL:
    {
        for (stream_id = 0; stream_id < gCtxt.numInputs; ++stream_id)
        {
            input_ctxt = &gCtxt.inputs[stream_id];
            pthread_mutex_lock(&input_ctxt->mutex);
            if (input_ctxt->state == QCARCAMTEST_STATE_STOP)
            {
                ret = qcarcam_input_start(input_ctxt);
            }
            pthread_mutex_unlock(&input_ctxt->mutex);
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_STOP:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt)
        {
            pthread_mutex_lock(&input_ctxt->mutex);
            if (input_ctxt->state == QCARCAMTEST_STATE_START ||
                input_ctxt->state == QCARCAMTEST_STATE_ERROR)
            {
                ret = qcarcam_input_stop(input_ctxt);
            }
            pthread_mutex_unlock(&input_ctxt->mutex);
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_STOP_ALL:
    {
        for (stream_id = 0; stream_id < gCtxt.numInputs; ++stream_id)
        {
            input_ctxt = &gCtxt.inputs[stream_id];

            pthread_mutex_lock(&input_ctxt->mutex);
            if (input_ctxt->state == QCARCAMTEST_STATE_START ||
                input_ctxt->state == QCARCAMTEST_STATE_ERROR)
            {
                ret = qcarcam_input_stop(input_ctxt);
            }
            pthread_mutex_unlock(&input_ctxt->mutex);
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_PAUSE:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt)
        {
            pthread_mutex_lock(&input_ctxt->mutex);
            if (input_ctxt->state == QCARCAMTEST_STATE_START)
            {
                ret = qcarcam_input_pause(input_ctxt);
            }
            pthread_mutex_unlock(&input_ctxt->mutex);
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_PAUSE_ALL:
    {
        for (stream_id = 0; stream_id < gCtxt.numInputs; ++stream_id)
        {
            input_ctxt = &gCtxt.inputs[stream_id];

            pthread_mutex_lock(&input_ctxt->mutex);
            if (input_ctxt->state == QCARCAMTEST_STATE_START)
            {
                ret = qcarcam_input_pause(input_ctxt);
            }
            pthread_mutex_unlock(&input_ctxt->mutex);
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_RESUME:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt)
        {
            pthread_mutex_lock(&input_ctxt->mutex);
            if (input_ctxt->state == QCARCAMTEST_STATE_PAUSE)
            {
                ret = qcarcam_input_resume(input_ctxt);
            }
            pthread_mutex_unlock(&input_ctxt->mutex);
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_RESUME_ALL:
    {
        for (stream_id = 0; stream_id < gCtxt.numInputs; ++stream_id)
        {
            input_ctxt = &gCtxt.inputs[stream_id];
            pthread_mutex_lock(&input_ctxt->mutex);
            if (input_ctxt->state == QCARCAMTEST_STATE_PAUSE)
            {
                ret = qcarcam_input_resume(input_ctxt);
            }
            pthread_mutex_unlock(&input_ctxt->mutex);
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_SET_EXPOSURE:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt && input_ctxt->qcarcam_hndl)
        {
            char buf[BUFSIZE];
            char *p = NULL;
            int param_option = 0;

            printf("Select exposure type: [0]Manual, [1]Auto\n");
            if (fgets(buf, sizeof(buf), stdin) != NULL)
            {
                /* 10 indicates decimal number*/
                param_option = strtol(buf, &p, 10);
            }

            switch (param_option)
            {
                case 0:
                {
                    QCarCamExposureConfig_t param = {};
                    get_input_hdr_exposure(&param);
                    param.mode = QCARCAM_EXPOSURE_MANUAL;
                    ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_SENSOR_PARAM_EXPOSURE, &param, sizeof(param));
                    break;
                }
                case 1:
                {
                    QCarCamExposureConfig_t param = {};
                    param.mode = QCARCAM_EXPOSURE_AUTO;
                    ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_SENSOR_PARAM_EXPOSURE, &param, sizeof(param));
                    break;
                }
                default:
                {
                    printf("Invalid input");
                    break;
                }
            }

            if (ret != QCARCAM_RET_OK)
            {
                QCARCAM_ERRORMSG("QCarCamSetParam (%d) failed for stream id %d, ret = %d", param_option, input_ctxt->qcarcam_input_id, ret);
            }
            else
            {
                QCARCAM_INFOMSG("QCarCamSetParam (%d) success for stream id %d", param_option, input_ctxt->qcarcam_input_id);
            }
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_SET_SENSORMODE:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt && input_ctxt->qcarcam_hndl)
        {
            uint32_t param;
            get_input_sensormode(&param);
            ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_STREAM_CONFIG_PARAM_INPUT_MODE, &param, sizeof(param));
            if (ret != QCARCAM_RET_OK)
            {
                QCARCAM_ERRORMSG("QCarCamSetParam (%d) failed for stream id %d, ret = %d", (int)QCARCAM_TEST_MENU_STREAM_SET_SENSORMODE, input_ctxt->qcarcam_input_id, ret);
            }
            else
            {
                QCARCAM_INFOMSG("QCarCamSetParam (%d) success for stream id %d", (int)QCARCAM_TEST_MENU_STREAM_SET_SENSORMODE, input_ctxt->qcarcam_input_id);
            }
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_GET_SENSORMODE:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt && input_ctxt->qcarcam_hndl)
        {
            uint32_t param = 0;

            ret = QCarCamGetParam(input_ctxt->qcarcam_hndl, QCARCAM_STREAM_CONFIG_PARAM_INPUT_MODE, &param, sizeof(param));
            if (ret != QCARCAM_RET_OK)
            {
                QCARCAM_ERRORMSG("QCarCamGetParam (%d) failed for stream id %d, ret = %d", (int)QCARCAM_TEST_MENU_STREAM_GET_SENSORMODE, input_ctxt->qcarcam_input_id, ret);
            }
            else
            {
                display_sensormode_settings(param);
            }

        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_GET_COLORSPACE:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt && input_ctxt->qcarcam_hndl)
        {
            QCarCamColorSpace_e param = QCARCAM_COLOR_SPACE_UNCORRECTED;
            ret = QCarCamGetParam(input_ctxt->qcarcam_hndl, QCARCAM_SENSOR_PARAM_COLOR_SPACE, &param, sizeof(param));

            if (ret != QCARCAM_RET_OK)
            {
                QCARCAM_ERRORMSG("qcarcam_g_param (%d) failed for stream id %d, ret = %d", (int)QCARCAM_TEST_MENU_STREAM_GET_COLORSPACE, input_ctxt->qcarcam_input_id, ret);
            }
            else
            {
                QCARCAM_ALWZMSG("Color space for the input id %d = %d", input_ctxt->qcarcam_input_id, param);
                display_color_space_settings(param);
            }
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_ENABLE_CALLBACK:
    {
        for (stream_id = 0; stream_id < gCtxt.numInputs; ++stream_id)
        {
            input_ctxt = &gCtxt.inputs[stream_id];
            if (input_ctxt->qcarcam_hndl && input_ctxt->use_event_callback)
            {
                uint32_t param;

                ret = QCarCamRegisterEventCallback(input_ctxt->qcarcam_hndl, &qcarcam_test_event_cb, input_ctxt);
                if (ret != QCARCAM_RET_OK)
                {
                    QCARCAM_ERRORMSG("QCarCamSetParam failed for stream id %d, ret = %d",input_ctxt->qcarcam_input_id, ret);
                    break;
                }

                param = QCARCAM_EVENT_FRAME_READY | QCARCAM_EVENT_INPUT_SIGNAL | QCARCAM_EVENT_ERROR;
                ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK, &param, sizeof(param));
                if (ret != QCARCAM_RET_OK)
                {
                    QCARCAM_ERRORMSG("QCarCamSetParam failed for stream id %d, ret = %d",input_ctxt->qcarcam_input_id, ret);
                }
                else
                {
                    QCARCAM_INFOMSG("QCarCamSetParam success for stream id %d",input_ctxt->qcarcam_input_id);
                }
            }
            else
            {
                QCARCAM_ERRORMSG("Callback is disabled in xml, please check the input xml");
            }
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_DISABLE_CALLBACK:
    {
        for (stream_id = 0; stream_id < gCtxt.numInputs; ++stream_id)
        {
            input_ctxt = &gCtxt.inputs[stream_id];
            if (input_ctxt->qcarcam_hndl && input_ctxt->use_event_callback)
            {
                uint32_t param;

                ret = QCarCamUnregisterEventCallback(input_ctxt->qcarcam_hndl);
                if (ret != QCARCAM_RET_OK)
                {
                    QCARCAM_ERRORMSG("QCarCamSetParam failed for stream id %d, ret = %d",input_ctxt->qcarcam_input_id, ret);
                    break;
                }

                param = 0x0;
                ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK, &param, sizeof(param));
                if (ret != QCARCAM_RET_OK)
                {
                    QCARCAM_ERRORMSG("QCarCamSetParam failed for stream id %d, ret = %d",input_ctxt->qcarcam_input_id, ret);
                }
                else
                {
                    QCARCAM_INFOMSG("QCarCamSetParam success for stream id %d",input_ctxt->qcarcam_input_id);
                }
            }
            else
            {
                QCARCAM_ERRORMSG("Callback is disabled in xml, please check the input xml");
            }
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_SET_FRAMERATE:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt && input_ctxt->qcarcam_hndl)
        {
            QCarCamFrameDropConfig_t param;
            get_input_framerate(&param);
            ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL, &param, sizeof(param));
            if (ret != QCARCAM_RET_OK)
            {
                QCARCAM_ERRORMSG("QCarCamSetParam framerate failed for stream id %d, ret = %d",input_ctxt->qcarcam_input_id, ret);
            }
            else
            {
                QCARCAM_INFOMSG("QCarCamSetParam framerate success for stream id %d",input_ctxt->qcarcam_input_id);
            }
            QCARCAM_INFOMSG("Provided QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL: period = %d, pattern = 0x%x",
                param.frameDropPeriod,
                param.frameDropPattern);
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_SET_COLOR_PARAM:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt && input_ctxt->qcarcam_hndl)
        {
            char buf[BUFSIZE];
            char *p = NULL;
            int param_option = 0;
            float param_value = 0.0;
            float param;

            printf("Select param: [0] Saturation, [1] Hue.\n");
            if (fgets(buf, sizeof(buf), stdin) != NULL)
            {
                /* 10 indicates decimal number*/
                param_option = strtol(buf, &p, 10);
            }

            switch (param_option)
            {
                case 0:
                    printf("Enter saturation value. Valid range (-1.0 to 1.0)\n");
                    if (fgets(buf, sizeof(buf), stdin) != NULL)
                    {
                        param_value = strtof(buf, NULL);
                    }

                    param = param_value;
                    ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_SENSOR_PARAM_SATURATION, &param, sizeof(param));
                    if (ret != QCARCAM_RET_OK)
                    {
                        QCARCAM_ERRORMSG("QCarCamSetParam failed for stream id %d! ret = %d\n",
                        input_ctxt->qcarcam_input_id, ret);
                    }
                    break;
                case 1:
                    printf("Enter hue value. Valid range (-1.0 to 1.0)\n");
                    if (fgets(buf, sizeof(buf), stdin) != NULL)
                    {
                        param_value = strtof(buf, NULL);
                    }

                    param = param_value;
                    ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_SENSOR_PARAM_HUE, &param, sizeof(param));
                    if (ret != QCARCAM_RET_OK)
                    {
                        QCARCAM_ERRORMSG("QCarCamSetParam failed for stream id %d! ret = %d\n",
                        input_ctxt->qcarcam_input_id, ret);
                    }
                    break;
                default:
                    printf("Param not available\n");
                    break;
            }
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_GET_COLOR_PARAM:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt && input_ctxt->qcarcam_hndl)
        {
            char buf[BUFSIZE];
            char *p = NULL;
            int param_option = 0;
            float param = 0.0f;

            printf("Select param: [0] Saturation, [1] Hue.\n");
            if (fgets(buf, sizeof(buf), stdin) != NULL)
            {
                /* 10 indicates decimal number*/
                param_option = strtol(buf, &p, 10);
            }

            switch (param_option)
            {
                case 0:
                    ret = QCarCamGetParam(input_ctxt->qcarcam_hndl, QCARCAM_SENSOR_PARAM_SATURATION, &param, sizeof(param));
                    if (ret != QCARCAM_RET_OK)
                    {
                        QCARCAM_ERRORMSG("QCarCamGetParam failed for stream id %d! ret = %d",
                        input_ctxt->qcarcam_input_id, ret);
                    }
                    break;
                case 1:
                    ret = QCarCamGetParam(input_ctxt->qcarcam_hndl, QCARCAM_SENSOR_PARAM_HUE, &param, sizeof(param));
                    if (ret != QCARCAM_RET_OK)
                    {
                        QCARCAM_ERRORMSG("QCarCamGetParam failed for stream id %d! ret = %d",
                        input_ctxt->qcarcam_input_id, ret);
                    }
                    break;
                default:
                    printf("Param not available\n");
                    ret = -1;
                    break;
            }

            if (ret == 0)
            {
                printf("Param value returned = %.2f\n", param);
                QCARCAM_ALWZMSG("QCarCamGetParam value returned = %.2f", param);
            }
        }
        break;
    }
    case QCARCAM_TEST_MENU_DUMP_NEXT_FRAME:
    {
        for (stream_id = 0; stream_id < gCtxt.numInputs; ++stream_id)
        {
            input_ctxt = &gCtxt.inputs[stream_id];

            input_ctxt->dumpNextFrame = TRUE;
        }
        break;
    }
    case QCARCAM_TEST_MENU_CHECK_BUFFERS:
        gCtxt.check_buffer_state = !gCtxt.check_buffer_state;
        break;
    case QCARCAM_TEST_MENU_STREAM_SET_VENDOR_PARAM:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt && input_ctxt->qcarcam_hndl)
        {
            QCarCamVendorParam_t param;
#ifndef USE_VENDOR_EXT_PARAMS
            param.data[0] = (uint8_t)0x12345678;
            param.data[1] = (uint8_t)0xfedcba98;
#else
            vendor_ext_property_t *p_isp_prop = (vendor_ext_property_t*)&(param.data[0]);
            p_isp_prop->type = VENDOR_EXT_PROP_TEST;
            p_isp_prop->value.uint_val = 0x12345678;
#endif

            ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_VENDOR_PARAM, &param, sizeof(param));
            if (QCARCAM_RET_OK != ret)
            {
                QCARCAM_ERRORMSG("QCarCamSetParam QCARCAM_VENDOR_PARAM failed for stream id %d! ret = %d",
                    input_ctxt->qcarcam_input_id, ret);
            }
            else
            {
#ifndef USE_VENDOR_EXT_PARAMS
                QCARCAM_INFOMSG("QCarCamSetParam QCARCAM_VENDOR_PARAM succeed for stream id %d: data[0]=%u data[1]=%u",
                    input_ctxt->qcarcam_input_id, param.data[0],
                    param.data[1]);
#else
                QCARCAM_INFOMSG("QCarCamSetParam QCARCAM_VENDOR_PARAM succeed for stream id %d: isp prop type=%u,val=%u",
                    input_ctxt->qcarcam_input_id, p_isp_prop->type ,
                    p_isp_prop->value.uint_val);
#endif
            }
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_GET_VENDOR_PARAM:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt && input_ctxt->qcarcam_hndl)
        {
            QCarCamVendorParam_t param;
#ifndef USE_VENDOR_EXT_PARAMS
            param.data[0] = 0;
#else
            vendor_ext_property_t *p_isp_prop = (vendor_ext_property_t*)&(param.data[0]);
            p_isp_prop->type =  VENDOR_EXT_PROP_TEST;
#endif
            ret = QCarCamGetParam(input_ctxt->qcarcam_hndl, QCARCAM_VENDOR_PARAM, &param, sizeof(param));
            if (QCARCAM_RET_OK != ret)
            {
                QCARCAM_ERRORMSG("QCarCamGetParam QCARCAM_VENDOR_PARAM failed for stream id %d! ret = %d",
                    input_ctxt->qcarcam_input_id, ret);
            }
            else
            {
                QCARCAM_INFOMSG("QCarCamGetParam QCARCAM_VENDOR_PARAM succeeds");
#ifdef USE_VENDOR_EXT_PARAMS
                QCARCAM_INFOMSG("QCarCamGetParam QCARCAM_VENDOR_PARAM succeed for stream id %d: isp prop type=%u,val=%u",
                    input_ctxt->qcarcam_input_id, p_isp_prop->type,
                    p_isp_prop->value.uint_val);
                printf("ISP param type = %u, value returned = %u\n", p_isp_prop->type,
                    p_isp_prop->value.uint_val);
#endif
            }
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_SET_BRIGHTNESS:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt && input_ctxt->qcarcam_hndl)
        {
            char buf[BUFSIZE];
            float param_value = 0.0f;
            float param = 0.0f;

            printf("Enter brightness value. Valid range (-1.0 to 1.0)\n");
            if (fgets(buf, sizeof(buf), stdin) != NULL)
            {
                param_value = strtof(buf, NULL);
            }

            param = param_value;

            ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_SENSOR_PARAM_BRIGHTNESS, &param, sizeof(param));
            if (QCARCAM_RET_OK != ret)
            {
                QCARCAM_INFOMSG("QCarCamSetParam QCARCAM_SENSOR_PARAM_BRIGHTNESS succeed for stream id %d: brightness=%f",
                    input_ctxt->qcarcam_input_id, ret);
            }
            else
            {
                QCARCAM_INFOMSG("QCarCamSetParam QCARCAM_SENSOR_PARAM_BRIGHTNESS succeed for stream id %d: brightness=%u",
                    input_ctxt->qcarcam_input_id, param);
            }
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_GET_BRIGHTNESS:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt && input_ctxt->qcarcam_hndl)
        {
            float param = 0.0f;

            ret = QCarCamGetParam(input_ctxt->qcarcam_hndl, QCARCAM_SENSOR_PARAM_BRIGHTNESS, &param, sizeof(param));
            if (QCARCAM_RET_OK != ret)
            {
                QCARCAM_ERRORMSG("QCarCamGetParam QCARCAM_SENSOR_PARAM_BRIGHTNESS failed for stream id %d! ret = %d",
                    input_ctxt->qcarcam_input_id, ret);
            }
            else
            {
                printf("Param value returned = %.2f\n", param);
                QCARCAM_INFOMSG("QCarCamGetParam QCARCAM_SENSOR_PARAM_BRIGHTNESS succeeded for stream id %d: brightness=%f",
                    input_ctxt->qcarcam_input_id, param);
            }
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_SET_CONTRAST:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt && input_ctxt->qcarcam_hndl)
        {
            char buf[BUFSIZE];
            float param_value = 0.0f;
            float param = 0.0f;

            printf("Enter contrast value. Valid range (-1.0 to 1.0)\n");
            if (fgets(buf, sizeof(buf), stdin) != NULL)
            {
                param_value = strtof(buf, NULL);
            }

            param = param_value;

            ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_SENSOR_PARAM_CONTRAST, &param, sizeof(param));
            if (QCARCAM_RET_OK != ret)
            {
                QCARCAM_ERRORMSG("QCarCamSetParam QCARCAM_SENSOR_PARAM_CONTRAST failed for stream id %d! ret = %f",
                    input_ctxt->qcarcam_input_id, ret);
            }
            else
            {
                QCARCAM_INFOMSG("QCarCamSetParam QCARCAM_SENSOR_PARAM_CONTRAST succeed for stream id %d: contrast=%u",
                    input_ctxt->qcarcam_input_id, param);
            }
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_GET_CONTRAST:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt && input_ctxt->qcarcam_hndl)
        {
            float param = 0.0f;

            ret = QCarCamGetParam(input_ctxt->qcarcam_hndl, QCARCAM_SENSOR_PARAM_CONTRAST, &param, sizeof(param));
            if (QCARCAM_RET_OK != ret)
            {
                QCARCAM_ERRORMSG("QCarCamGetParam QCARCAM_SENSOR_PARAM_CONTRAST failed for stream id %d! ret = %d",
                    input_ctxt->qcarcam_input_id, ret);
            }
            else
            {
                printf("Param value returned = %.2f\n", param);
                QCARCAM_INFOMSG("QCarCamGetParam QCARCAM_SENSOR_PARAM_CONTRAST succeeded for stream id %d: contrast=%f",
                    input_ctxt->qcarcam_input_id, param);
            }
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_SET_MIRRORING:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt && input_ctxt->qcarcam_hndl)
        {
            char buf[BUFSIZE];
            char *p = NULL;
            int param_option = 0;
            uint32 param_value = 0;
            uint32_t param = 0;

            printf("Select mirroring type: [0] Horizontal, [1] Vertical.\n");
            if (fgets(buf, sizeof(buf), stdin) != NULL)
            {
                /* 10 indicates decimal number*/
                param_option = strtol(buf, &p, 10);
            }

            switch (param_option)
            {
                case 0:
                    printf("Enter value. [1] Enable \n");
                    if (fgets(buf, sizeof(buf), stdin) != NULL)
                    {
                        param_value = strtoul(buf, NULL,10);
                    }

                    param = param_value;
                    ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_SENSOR_PARAM_MIRROR_H, &param, sizeof(param));
                    if (QCARCAM_RET_OK != ret)
                    {
                        QCARCAM_ERRORMSG("QCarCamSetParam failed for stream id %d! ret = %d\n",
                        input_ctxt->qcarcam_input_id, ret);
                    }
                    break;
                case 1:
                     printf("Enter value. [1] Enable \n");
                    if (fgets(buf, sizeof(buf), stdin) != NULL)
                    {
                        param_value = strtoul(buf, NULL,10);
                    }

                    param = param_value;
                    ret = QCarCamSetParam(input_ctxt->qcarcam_hndl, QCARCAM_SENSOR_PARAM_MIRROR_V, &param, sizeof(param));
                    if (QCARCAM_RET_OK != ret)
                    {
                        QCARCAM_ERRORMSG("QCarCamSetParam failed for stream id %d! ret = %d\n",
                        input_ctxt->qcarcam_input_id, ret);
                    }
                    break;
                default:
                    printf("Param not available\n");
                    break;
            }
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_GET_MIRRORING:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if (input_ctxt && input_ctxt->qcarcam_hndl)
        {
            char buf[BUFSIZE];
            char *p = NULL;
            int param_option = 0;
            uint32_t param = 0;

            printf("Select mirroring type: [0] Horizontal, [1] Vertical.\n");
            if (fgets(buf, sizeof(buf), stdin) != NULL)
            {
                /* 10 indicates decimal number*/
                param_option = strtol(buf, &p, 10);
            }

            switch (param_option)
            {
                case 0:
                    ret = QCarCamGetParam(input_ctxt->qcarcam_hndl, QCARCAM_SENSOR_PARAM_MIRROR_H, &param, sizeof(param));
                    if (QCARCAM_RET_OK != ret)
                    {
                        QCARCAM_ERRORMSG("QCarCamGetParam failed for stream id %d! ret = %d",
                        input_ctxt->qcarcam_input_id, ret);
                    }
                    break;
                case 1:
                    ret = QCarCamGetParam(input_ctxt->qcarcam_hndl, QCARCAM_SENSOR_PARAM_MIRROR_V, &param, sizeof(param));
                    if (QCARCAM_RET_OK != ret)
                    {
                        QCARCAM_ERRORMSG("QCarCamGetParam failed for stream id %d! ret = %d",
                        input_ctxt->qcarcam_input_id, ret);
                    }
                    break;
                default:
                    printf("Param not available\n");
                    ret = -1;
                    break;
            }

            if (ret == 0)
            {
                printf("Param value returned = %u\n", param);
                QCARCAM_ALWZMSG("QCarCamGetParam value returned = %u", param);
            }
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_SUBSCRIBE_CHANGE_EVENT:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);

        if(input_ctxt && input_ctxt->qcarcam_hndl)
        {
            char buf[BUFSIZE];
            char *p = NULL;
            uint64 param_option = 0;
            printf("Select setting in hexadecimal (1<<qcarcam_param_t)\n"
                "EXPOSURE [(1<<17) -- 0x20000] HDR EXPOSURE [1<<20 -- 0x100000]\n"
                "SATURATION [1<<19 -- 0x80000] HUE [1<<18 -- 0x40000]\n"
                "FRAME RATE [1<<9 -- 0x200] GAMMA [1<<21 -- 0x200000]\n");
            if (fgets(buf, sizeof(buf), stdin) != NULL)
            {
                param_option = strtoul(buf, &p, 0);
                qcarcam_test_subscribe_input_params_change(input_ctxt, param_option);
            }
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_UNSUBSCRIBE_CHANGE_EVENT:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);

        if(input_ctxt && input_ctxt->qcarcam_hndl)
        {
            char buf[BUFSIZE];
            char *p = NULL;
            uint64 param_option = 0;
            printf("Select setting in hexadecimal (1<<qcarcam_param_t)\n"
                "EXPOSURE [(1<<17) -- 0x20000] HDR EXPOSURE [1<<20 -- 0x100000]\n"
                "SATURATION [1<<19 -- 0x80000] HUE [1<<18 -- 0x40000]\n"
                "FRAME RATE [1<<9 -- 0x200] GAMMA [1<<21 -- 0x200000]\n");
            if (fgets(buf, sizeof(buf), stdin) != NULL)
            {
                param_option = strtoul(buf, &p, 0);
                qcarcam_test_unsubscribe_input_params_change(input_ctxt, param_option);
            }
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_SUBSCRIBE_SOF_EVENT:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);

        if(input_ctxt && input_ctxt->qcarcam_hndl)
        {
            qcarcam_test_subscribe_SOF_event(input_ctxt);
        }
        break;
    }
    case QCARCAM_TEST_MENU_STREAM_UNSUBSCRIBE_SOF_EVENT:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);

        if(input_ctxt && input_ctxt->qcarcam_hndl)
        {
            qcarcam_test_unsubscribe_SOF_event(input_ctxt);
        }
        break;
    }
    case QCARCAM_TEST_MENU_MASTER:
    {
        stream_id = get_input_stream_id();
        input_ctxt = get_input_ctxt(stream_id);
        if(input_ctxt && input_ctxt->qcarcam_hndl)
        {
            unsigned int flag = get_input_set_flag();
            if (flag == 1)
            {
                // to prevent setting a master stream as master again
                if(input_ctxt->is_master)
                {
                    QCARCAM_INFOMSG("This client is already set as the master");
                }
                else
                {
                    qcarcam_test_set_master(input_ctxt, flag);
                    if(!input_ctxt->is_master)
                    {
                        QCARCAM_INFOMSG("There is already another client set as master");
                    }
                }

            }
            else if (flag == 0)
            {
                if(!input_ctxt->is_master)
                {
                    QCARCAM_INFOMSG("This client is not master in order to release, stream_id:%d", stream_id);
                }
                else
                {
                    qcarcam_test_set_master(input_ctxt, flag);
                }
            }
            else
            {
                QCARCAM_ERRORMSG("Invalid option (%d)", flag);
            }
        }
        break;
    }
    case QCARCAM_TEST_MENU_GET_SYSTEM_DIAGNOSTICS:
    {
        QCarCamDiagInfo* pDiagnosticInfo = (QCarCamDiagInfo*)calloc(1, sizeof(QCarCamDiagInfo));
        if (!pDiagnosticInfo)
        {
            QCARCAM_ERRORMSG("Failed to allocate diagnosticInfo");
            break;
        }

        ret = QCarCamQueryDiagnostics(pDiagnosticInfo);
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("QCarCamQueryDiagnostics failed with ret = %d", ret);
        }
        else
        {
            parse_diagnostics(pDiagnosticInfo);
            QCARCAM_INFOMSG("QCarCamQueryDiagnostics is success and parsed");
        }

        free(pDiagnosticInfo);
        break;
    }
    default:
        QCARCAM_ERRORMSG("Invalid option (%d)", option);
        break;
    }
}

QCarCamRet_e qcarcam_test_power_event_callback(test_util_pm_event_t power_event_id, void* p_usr_ctxt)
{
    QCarCamRet_e rc = QCARCAM_RET_OK;
    int i = 0;
    qcarcam_test_input_t *input_ctxt = NULL;
    qcarcam_test_ctxt_t* p_ctxt = (qcarcam_test_ctxt_t* )p_usr_ctxt;

    switch (power_event_id)
    {
    case TEST_UTIL_PM_SUSPEND:
    {
        for (i = 0; i < p_ctxt->numInputs; ++i)
        {
            input_ctxt = &p_ctxt->inputs[i];
            if (input_ctxt->qcarcam_hndl)
            {
                QCARCAM_ERRORMSG("suspend inputId: %d, idx = %d\n", input_ctxt->qcarcam_input_id, i);
                pthread_mutex_lock(&input_ctxt->mutex);
                if (input_ctxt->state == QCARCAMTEST_STATE_START ||
                    input_ctxt->state == QCARCAMTEST_STATE_ERROR)
                {
                    rc = qcarcam_input_stop(input_ctxt);
                    if (rc == QCARCAM_RET_OK)
                    {
                        input_ctxt->state = QCARCAMTEST_STATE_SUSPEND;
                    }
                }
                pthread_mutex_unlock(&input_ctxt->mutex);
            }
        }

        break;
    }
    case TEST_UTIL_PM_RESUME:
    {
        for (i = 0; i < p_ctxt->numInputs; ++i)
        {
            input_ctxt = &p_ctxt->inputs[i];
            if (input_ctxt->state == QCARCAMTEST_STATE_SUSPEND)
            {
                QCARCAM_ERRORMSG("resume inputId: %d, idx = %d\n", input_ctxt->qcarcam_input_id, i);
                pthread_mutex_lock(&input_ctxt->mutex);
                rc = qcarcam_input_start(input_ctxt);
                pthread_mutex_unlock(&input_ctxt->mutex);
            }
        }
        break;
    }
    default:
        break;
    }

    return rc;
}


int main(int argc, char **argv)
{
#ifdef POST_PROCESS
    char* pLibName = (char*)"libpost_process_c2d_convert.so";
#endif
#if defined(__INTEGRITY)
    // Set priority of main thread
    // Task needs to be linked to an object defined in corresponding INT file
    Task mainTask = TaskObjectNumber(QCARCAM_OBJ_NUM);
    SetTaskPriority(mainTask, CameraTranslateThreadPriority(QCARCAM_THRD_PRIO), false);
#endif

    initialize_qcarcam_test_ctxt();

    unsigned long long t_before = 0;
    unsigned long long t_after = 0;

    qcarcam_test_get_time(&gCtxt.t_start);

    const char *tok = NULL;
    QCarCamRet_e ret = QCARCAM_RET_OK;

    int rval = EXIT_FAILURE;
    int rc = 0;
    int first_single_buf_input_idx = -1;

    CameraThread timer_thread_handle = NULL;
    CameraThread signal_thread_handle = NULL;
    CameraThread fps_thread_handle = NULL;
    CameraThread signal_loss_thread_handle = NULL;
    CameraThread csi_error_thread_handle = NULL;
    char thread_name[64];
    int bprint_diag = 0;

    test_util_intr_thrd_args_t intr_thrd_args;

    int remote_attach_loop = 0; // no looping for remote process attach

    /* Should be placed before any forked thread */
    sigfillset(&g_sigset);
    for (int i = 0; exceptsigs[i] != -1; i++)
    {
        sigdelset(&g_sigset, exceptsigs[i]);
    }
    pthread_sigmask(SIG_BLOCK, &g_sigset, NULL);

    QCarCamInit_t qcarcam_init = {};
    qcarcam_init.apiVersion = QCARCAM_VERSION;
    //qcarcam_init.debug_tag = (char *)"qcarcam_test";

    ret = QCarCamInitialize(&qcarcam_init);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("QCarCamInitialize failed %d", ret);
        exit(-1);
    }

    test_util_ctxt_t* test_util_ctxt = NULL;
    test_util_ctxt_params_t test_util_ctxt_params = {};

    test_util_set_power_callback(qcarcam_test_power_event_callback, &gCtxt);

    QCARCAM_DBGMSG("Arg parse Begin");

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
            gCtxt.dumpFrame = atoi(tok);
        }
        else if (!strncmp(argv[i], "-startStop=", strlen("-startStop=")))
        {
            tok = argv[i] + strlen("-startStop=");
            gCtxt.enableStartStop = atoi(tok);
        }
        else if (!strncmp(argv[i], "-pauseResume=", strlen("-pauseResume=")))
        {
            tok = argv[i] + strlen("-pauseResume=");
            gCtxt.enablePauseResume = atoi(tok);
        }
        else if (!strncmp(argv[i], "-noDisplay", strlen("-noDisplay")))
        {
            gCtxt.disable_display = 1;
            test_util_ctxt_params.disable_display = 1;
        }
        else if (!strncmp(argv[i], "-singlethread", strlen("-singlethread")))
        {
            gCtxt.multithreaded = 0;
        }
        else if (!strncmp(argv[i], "-printfps", strlen("-printfps")))
        {
            tok = argv[i] + strlen("-printfps=");
            gCtxt.fps_print_delay = atoi(tok);
        }
        else if (!strncmp(argv[i], "-checkbuf=", strlen("-checkbuf=")))
        {
            tok = argv[i] + strlen("-checkbuf=");
            gCtxt.check_buffer_state = atoi(tok);
        }
        else if (!strncmp(argv[i], "-showStats=", strlen("-showStats=")))
        {
            tok = argv[i] + strlen("-showStats=");
            gCtxt.enableStats = atoi(tok);
        }
        else if (!strncmp(argv[i], "-c2d=", strlen("-c2d=")))
        {
            tok = argv[i] + strlen("-c2d=");
            gCtxt.enable_c2d = atoi(tok);
            test_util_ctxt_params.enable_c2d = gCtxt.enable_c2d;
        }
        else if (!strncmp(argv[i], "-cache", strlen("-cache")))
        {
            gCtxt.enable_cache = 1;
        }
#ifdef ENABLE_CL_CONVERTER
        else if (!strncmp(argv[i], "-csc=", strlen("-csc=")))
        {
            tok = argv[i] + strlen("-csc=");
            gCtxt.enable_csc = atoi(tok);
            test_util_ctxt_params.enable_csc = gCtxt.enable_csc;
        }
#endif
        else if (!strncmp(argv[i], "-di=", strlen("-di=")))
        {
            tok = argv[i] + strlen("-di=");
            gCtxt.enable_deinterlace = (testutil_deinterlace_t)atoi(tok);
            test_util_ctxt_params.enable_di = gCtxt.enable_deinterlace;
        }
        else if (!strncmp(argv[i], "-nonInteractive", strlen("-nonInteractive")) ||
                !strncmp(argv[i], "-nomenu", strlen("-nomenu")))
        {
            gCtxt.enableMenuMode = 0;
        }
        else if (!strncmp(argv[i], "-fatalErrorRecover", strlen("-fatalErrorRecover")))
        {
            /* only one of them enabled in real scenario */
            gCtxt.enableBridgeErrorDetect = 0;
            gCtxt.enableFatalErrorRecover = 1;
        }
        else if (!strncmp(argv[i], "-checkdualcsi", strlen("-checkdualcsi")))
        {
            gCtxt.checkDualCsi = 1;
        }
        else if (!strncmp(argv[i], "-printlatency", strlen("-printlatency")))
        {
            bprint_diag = 1;
        }
        else if (!strncmp(argv[i], "-seconds=", strlen("-seconds=")))
        {
            tok = argv[i] + strlen("-seconds=");
            gCtxt.exitSeconds = atoi(tok);
        }
        else if (!strncmp(argv[i], "-retry", strlen("-retry")))
        {
            /* Used to wait for query inputs to handoff early RVC */
            gCtxt.enableRetry = 1;
        }
        else if (!strncmp(argv[i], "-gpioNumber=", strlen("-gpioNumber=")))
        {
            tok = argv[i]+ strlen("-gpioNumber=");
            gCtxt.gpioNumber = atoi(tok);
            test_util_ctxt_params.enable_gpio_config = 1;
        }
        else if (!strncmp(argv[i], "-gpioMode=", strlen("-gpioMode=")))
        {
            /**
            *   Mode 0 : Visiblity toggle
            *   Mode 1 : Play/Pause toggle
            *   Mode 2 : Start/Stop toggle
            */
            tok = argv[i] + strlen("-gpioMode=");
            gCtxt.gpioMode = atoi(tok);
        }
        else if (!strncmp(argv[i], "-allocator=", strlen("-allocator=")))
        {
            tok = argv[i] + strlen("-allocator=");
            test_util_ctxt_params.offscreen_allocator = atoi(tok);
        }
#ifdef POST_PROCESS
        else if (!strncmp(argv[i], "-post_process=", strlen("-post_process=")))
        {
            tok = argv[i] + strlen("-post_process=");
            gCtxt.enable_post_processing = atoi(tok);
            test_util_ctxt_params.enable_post_processing = gCtxt.enable_post_processing;
            QCARCAM_ERRORMSG("enabled post processing");
        }

        else if (strncmp(argv[i], "-post_process_lib_name=", strlen("-post_process_lib_name=")) == 0) {
            pLibName = argv[i] + strlen("-post_process_lib_name=");
        }
#endif
        else
        {
            QCARCAM_ERRORMSG("Invalid argument, argv[%d]=%s", i, argv[i]);
            exit(-1);
        }
    }

    QCARCAM_DBGMSG("Arg parse End");

    qcarcam_test_get_time(&t_before);

    QCARCAM_DBGMSG("test_util_init");
    ret = test_util_init(&test_util_ctxt, &test_util_ctxt_params);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("test_util_init failed");
        exit(-1);
    }

    qcarcam_test_get_time(&t_after);
    QCARCAM_PERFMSG(gCtxt.enableStats, "test_util_init : %lu ms", (t_after - t_before));
    t_before = t_after;

    /*parse xml*/
    test_util_xml_input_t *xml_inputs = (test_util_xml_input_t *)calloc(NUM_MAX_CAMERAS, sizeof(test_util_xml_input_t));
    if (!xml_inputs)
    {
        QCARCAM_ERRORMSG("Failed to allocate xml input struct");
        exit(-1);
    }

    /*If exposure is not set in XML file, set manual_exp to -1 to use default exposure setting*/
    for (int i = 0; i < NUM_MAX_CAMERAS; i++)
    {
        xml_inputs[i].exp_params.manual_exp = -1;
    }

    gCtxt.numInputs = test_util_parse_xml_config_file_v2(g_filename, xml_inputs, NUM_MAX_CAMERAS, &g_xml_cfg);
    if (gCtxt.numInputs <= 0)
    {
        QCARCAM_ERRORMSG("Failed to parse config file!");
        exit(-1);
    }

    qcarcam_test_get_time(&t_after);
    QCARCAM_PERFMSG(gCtxt.enableStats, "xml parsing : %lu ms", (t_after - t_before));
    t_before = t_after;


    /*query qcarcam*/
    QCarCamInput_t *pInputs;
    unsigned int queryNumInputs = 0, queryFilled = 0;
    /*retry used for early camera handoff for RVC*/
    do {
        ret = QCarCamQueryInputs(NULL, 0, &queryNumInputs);
        if (QCARCAM_RET_OK != ret || queryNumInputs == 0)
        {
            QCARCAM_ERRORMSG("Failed QCarCamQueryInputs number of inputs with ret %d", ret);
            if (gCtxt.enableRetry == 0)
            {
                exit(-1);
            }
        }
        else
        {
            break;
        }
        if (gCtxt.enableRetry == 1)
        {
            usleep(500000);
        }
    } while (gCtxt.enableRetry == 1);

    pInputs = (QCarCamInput_t *)calloc(queryNumInputs, sizeof(*pInputs));
    if (!pInputs)
    {
        QCARCAM_ERRORMSG("Failed to allocate pInputs");
        exit(-1);
    }

    ret = QCarCamQueryInputs(pInputs, queryNumInputs, &queryFilled);
    if (QCARCAM_RET_OK != ret || queryFilled != queryNumInputs)
    {
        QCARCAM_ERRORMSG("Failed QCarCamQueryInputs with ret %d %d %d", ret, queryFilled, queryNumInputs);
        exit(-1);
    }

    printf("--- QCarCam Queried Inputs ----\n");
    for (unsigned int inputIdx = 0; inputIdx < queryFilled; inputIdx++)
    {
        QCarCamMode_t *pModes = (QCarCamMode_t *)calloc(pInputs[inputIdx].numModes, sizeof(QCarCamMode_t));
        QCarCamInputModes_t *pQueryModes = &gCtxt.queryInputs[inputIdx].modesDesc;

        gCtxt.queryInputs[inputIdx].pInputDesc = &pInputs[inputIdx];
        pQueryModes->numModes = pInputs[inputIdx].numModes;
        pQueryModes->pModes = pModes;

        ret = QCarCamQueryInputModes(pInputs[inputIdx].inputId, pQueryModes);
        if (QCARCAM_RET_OK != ret)
        {
            QCARCAM_ERRORMSG("QCarCamQueryInputModes() failed %d", ret);
            continue;
        }

        printf("%d: input_id=%d, numModes=%d name=%s flags=0x%x currMode=0x%x\n",
                inputIdx, pInputs[inputIdx].inputId,
                pInputs[inputIdx].numModes, pInputs[inputIdx].inputName, pInputs[inputIdx].flags,
                pQueryModes->currentMode);

        for (unsigned int modeIdx = 0; modeIdx < pInputs[inputIdx].numModes; modeIdx++)
        {
            printf("\t { modeId=%d  numSources=%d\n", modeIdx, pModes[modeIdx].numSources);

            for (unsigned int srcIdx = 0; srcIdx < pModes[modeIdx].numSources; srcIdx++)
            {
                printf("\t\t { srcId %d | %dx%d fmt=0x%08x fps=%.2f }\n",
                        pModes[modeIdx].sources[srcIdx].srcId,
                        pModes[modeIdx].sources[srcIdx].width, pModes[modeIdx].sources[srcIdx].height,
                        pModes[modeIdx].sources[srcIdx].colorFmt,
                        pModes[modeIdx].sources[srcIdx].fps);
            }
            printf("\t }\n");
        }
    }

    qcarcam_test_get_time(&t_after);
    QCARCAM_PERFMSG(gCtxt.enableStats, "query inputs : %lu ms", (t_after - t_before));
    printf("-------------------------------\n");

    while (remote_attach_loop)
    {
        volatile int i;
        for (i = 0; i < 1000; i++)
            usleep(100000);
    }

#ifndef C2D_DISABLED
    if (gCtxt.enable_c2d || gCtxt.enable_csc)
    {
        QCARCAM_DBGMSG("C2D Setup Begin");

        /*init C2D*/
        C2D_DRIVER_SETUP_INFO set_driver_op = {};
        set_driver_op.max_surface_template_needed = (NUM_MAX_DISP_BUFS + QCARCAM_MAX_NUM_BUFFERS) * NUM_MAX_CAMERAS;
        C2D_STATUS c2d_status = c2dDriverInit(&set_driver_op);
        if (c2d_status != C2D_STATUS_OK)
        {
            QCARCAM_ERRORMSG("c2dDriverInit failed");
            goto fail;
        }
    }
#endif

    /*signal handler thread*/
    snprintf(thread_name, sizeof(thread_name), "signal_thrd");
    rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &signal_thread, 0, 0, thread_name, &signal_thread_handle);
    if (rc)
    {
        QCARCAM_ERRORMSG("CameraCreateThread failed : %s", thread_name);
        goto fail;
    }

#ifdef __QNXNTO__
    snprintf(thread_name, sizeof(thread_name), "signal_loss_thrd");
    rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &check_signal_loss_thread, 0, 0, thread_name, &signal_loss_thread_handle);
    if (rc)
    {
        QCARCAM_ERRORMSG("CameraCreateThread failed : %s", thread_name);
        goto fail;
    }
#endif

    if (gCtxt.enableFatalErrorRecover)
    {
        snprintf(thread_name, sizeof(thread_name), "check_error_thread");
        rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &check_error_thread, 0, 0, thread_name, &csi_error_thread_handle);
        if (rc)
        {
            QCARCAM_ERRORMSG("CameraCreateThread failed : %s", thread_name);
            goto fail;
        }
    }

    /*if GPIO interrupts have been enabled*/
    if (gCtxt.gpioNumber > 0)
    {
        uint32_t irq;
        test_util_trigger_type_t trigger;

        switch (gCtxt.gpioMode)
        {
        case TESTUTIL_GPIO_MODE_VISIBILITY:
            // gpioMode 0: visibility toggle
            QCARCAM_INFOMSG("Selected GPIO Mode: Visibility toggle");
            trigger = TESTUTIL_GPIO_INTERRUPT_TRIGGER_EDGE;
            intr_thrd_args.cb_func = qcarcam_test_gpio_interrupt_cb;
            break;
        case TESTUTIL_GPIO_MODE_PLAYPAUSE:
            // gpioMode 1: play/pause toggle
            QCARCAM_INFOMSG("Selected GPIO Mode: Play/Pause toggle");
            trigger = TESTUTIL_GPIO_INTERRUPT_TRIGGER_FALLING;
            intr_thrd_args.cb_func = qcarcam_test_gpio_play_pause_cb;
            break;
        case TESTUTIL_GPIO_MODE_STARTSTOP:
            // gpioMode 2: start/stop toggle
            QCARCAM_INFOMSG("Selected GPIO Mode: Start/Stop toggle");
            trigger = TESTUTIL_GPIO_INTERRUPT_TRIGGER_FALLING;
            intr_thrd_args.cb_func = qcarcam_test_gpio_start_stop_cb;
            break;
        default:
            QCARCAM_ERRORMSG("Invalid mode");
            goto fail;
        }

        if (test_util_gpio_interrupt_config(&irq, gCtxt.gpioNumber, trigger) != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("test_util_gpio_interrupt_config failed");
            goto fail;
        }

        intr_thrd_args.irq = irq;

        if (test_util_interrupt_attach(&intr_thrd_args) != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("test_util_interrupt_attach failed");
            goto fail;
        }
    }

    /*thread used frame rate measurement*/
    if (gCtxt.fps_print_delay)
    {
        snprintf(thread_name, sizeof(thread_name), "framerate_thrd");
        rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &framerate_thread, 0, 0, thread_name, &fps_thread_handle);
        if (rc)
        {
            QCARCAM_ERRORMSG("CameraCreateThread failed : %s", thread_name);
            goto fail;
        }
    }

    for (int input_idx = 0; input_idx < gCtxt.numInputs; input_idx++)
    {
        qcarcam_test_input_t* input_ctxt = &gCtxt.inputs[input_idx];
        test_util_xml_input_t* p_xml_input = &xml_inputs[input_idx];
        QCarCamInputModes_t *pQueryModes = NULL;

        pthread_mutex_init(&input_ctxt->mutex, NULL);
        pthread_mutex_init(&input_ctxt->queue_mutex, NULL);
        input_ctxt->state = QCARCAMTEST_STATE_INIT;

        input_ctxt->qcarcam_input_id = p_xml_input->properties.qcarcam_input_id;
        input_ctxt->op_mode = p_xml_input->properties.op_mode;
        input_ctxt->use_event_callback = p_xml_input->properties.use_event_callback;
        input_ctxt->subscribe_parameter_change = p_xml_input->properties.subscribe_parameter_change;
        input_ctxt->delay_time = p_xml_input->properties.delay_time;
        input_ctxt->recovery = p_xml_input->properties.recovery;
        input_ctxt->request_mode = p_xml_input->properties.request_mode;


        /*no timeout when using event callback as frame is ready as soon as we get callback*/
        if (input_ctxt->use_event_callback)
        {
            input_ctxt->frame_timeout = 0;
        }
        else
        {
            input_ctxt->frame_timeout = (p_xml_input->properties.frame_timeout == -1) ?
                QCARCAM_TEST_DEFAULT_GET_FRAME_TIMEOUT :
                p_xml_input->properties.frame_timeout * 1000 * 1000;
        }

        //Find input in list of queried inputs
        for (unsigned int i = 0; i < queryFilled; i++)
        {
            if (pInputs[i].inputId == input_ctxt->qcarcam_input_id)
            {
                input_ctxt->query_inputs_idx = i;
            }
        }
        if (queryFilled == input_ctxt->query_inputs_idx)
        {
            QCARCAM_ERRORMSG("Couldn't find inputId %d in queried input list...", input_ctxt->qcarcam_input_id);
            goto fail;
        }
        pQueryModes = &gCtxt.queryInputs[input_ctxt->query_inputs_idx].modesDesc;

        input_ctxt->num_isp_instances = p_xml_input->isp_params.num_isp_instances;
        for (unsigned int i = 0; i < input_ctxt->num_isp_instances; i++)
        {
            input_ctxt->isp_config[i].id =  p_xml_input->isp_params.isp_config[i].id;
            input_ctxt->isp_config[i].cameraId = p_xml_input->isp_params.isp_config[i].cameraId;
            input_ctxt->isp_config[i].usecaseId = p_xml_input->isp_params.isp_config[i].usecaseId;
        }

        //Set output buffer params
        input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].n_buffers = p_xml_input->output_params.n_buffers;
        if (p_xml_input->output_params.width == -1 || p_xml_input->output_params.height == -1)
        {
            input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].width = pQueryModes->pModes[0].sources[0].width;
            input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].height = pQueryModes->pModes[0].sources[0].height;
        }
        else
        {
            input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].width = p_xml_input->output_params.width;
            input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].height = p_xml_input->output_params.height;
        }

        if (p_xml_input->output_params.format == -1)
        {
            input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].format =
                    pQueryModes->pModes[0].sources[0].colorFmt;
        }
        else
        {
            input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].format =
                p_xml_input->output_params.format;
        }

        input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].n_buffers = p_xml_input->window_params.n_buffers_display;
        input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].width = p_xml_input->output_params.width;
        input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].height = p_xml_input->output_params.height;
        input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].format = QCARCAM_FMT_RGB_888;

        input_ctxt->window_params = p_xml_input->window_params;
        input_ctxt->window_params.buffer_size[0] = input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].width;
        input_ctxt->window_params.buffer_size[1] = input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].height;
        input_ctxt->window_params.visibility = 1;
        //set window debug name to XML input index
        snprintf(input_ctxt->window_params.debug_name, sizeof(input_ctxt->window_params.debug_name), "qcarcam_%d", input_idx);

        input_ctxt->window_params.flags = pInputs[input_ctxt->query_inputs_idx].flags;
        input_ctxt->window_params.is_imported_buffer = 0;

        //Capture exposure params if any
        input_ctxt->exp_time = p_xml_input->exp_params.exposure_time;
        input_ctxt->gain = p_xml_input->exp_params.gain;
        input_ctxt->manual_exposure = p_xml_input->exp_params.manual_exp;

        //Capture frame rate params if any
        input_ctxt->frameDropConfig = p_xml_input->output_params.frame_rate_config;
        input_ctxt->num_batch_frames = p_xml_input->output_params.num_batch_frames;
        input_ctxt->detect_first_phase_timer = p_xml_input->output_params.detect_first_phase_timer;
        input_ctxt->frame_increment = p_xml_input->output_params.frame_increment;

        if (gCtxt.enable_deinterlace && (gCtxt.enable_deinterlace == TESTUTIL_DEINTERLACE_SW_WEAVE))
        {
            if (gCtxt.enable_c2d || gCtxt.enable_csc)
            {
                QCARCAM_ERRORMSG("c2d & deinterlace can't be used at the same time");
                goto fail;
            }

            input_ctxt->window_params.buffer_size[1] = DEINTERLACE_FIELD_HEIGHT * 2;
            input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].height = DEINTERLACE_FIELD_HEIGHT * 2;
        }

        if (p_xml_input->inject_params.filename[0] != '\0')
        {
            size_t size;

            // Check validity of injection parameters
            if ((0 == p_xml_input->inject_params.n_buffers) ||
                (0 == p_xml_input->inject_params.n_frames) ||
                (0 == p_xml_input->inject_params.framerate))
            {
                QCARCAM_ERRORMSG("Invalid injection params: n_buffers %d n_frames %d framerate %d",
                    p_xml_input->inject_params.n_buffers, p_xml_input->inject_params.n_frames,
                    p_xml_input->inject_params.framerate);
                goto fail;
            }

            // Number of buffers should be greater than or equal to number of frames
            if (p_xml_input->inject_params.n_frames > p_xml_input->inject_params.n_buffers)
            {
                QCARCAM_ERRORMSG("Invalid injection params: n_frames %d > n_buffers %d",
                    p_xml_input->inject_params.n_frames, p_xml_input->inject_params.n_buffers);
                goto fail;
            }

            input_ctxt->injectionParams = p_xml_input->inject_params;
            input_ctxt->injectionSettings.buf_idx = 0;
            input_ctxt->injectionSettings.request_id = 1;

            // Calculate total number of frames to inject
            // when no continuous injection is needed
            if (input_ctxt->injectionParams.repeat != -1)
            {
                input_ctxt->injectionSettings.n_total_frames =
                    input_ctxt->injectionParams.n_frames * input_ctxt->injectionParams.repeat;
                input_ctxt->injectionSettings.n_dump_frames = input_ctxt->injectionParams.n_frames;
            }

            // Populate input buffer list parameters for injection
            //@todo change bufferlist id to QCARCAM_BUFFERLIST_ID_INPUT_0 once support is changed;
            //      for now it is expecting id=1 for injection
            input_ctxt->p_buffers_input.id          = 1;
            input_ctxt->p_buffers_input.flags       = QCARCAM_BUFFER_FLAG_OS_HNDL;
            input_ctxt->p_buffers_input.nBuffers    = p_xml_input->inject_params.n_buffers;
            input_ctxt->p_buffers_input.colorFmt    = (QCarCamColorFmt_e)QCARCAM_COLOR_FMT(
                                                                p_xml_input->inject_params.pattern,
                                                                p_xml_input->inject_params.bitdepth,
                                                                p_xml_input->inject_params.pack);
            input_ctxt->p_buffers_input.pBuffers    = (QCarCamBuffer_t*)calloc(
                                                                input_ctxt->p_buffers_input.nBuffers,
                                                                sizeof(QCarCamBuffer_t));
            if (input_ctxt->p_buffers_input.pBuffers == 0)
            {
                QCARCAM_ERRORMSG("Alloc qcarcam_buffer input buffers failed");
                goto fail;
            }
            input_ctxt->p_buffers_input.pBuffers[0].numPlanes = 1;
            input_ctxt->p_buffers_input.pBuffers[0].planes[0].width = p_xml_input->inject_params.buffer_size[0];
            input_ctxt->p_buffers_input.pBuffers[0].planes[0].height = p_xml_input->inject_params.buffer_size[1];
            input_ctxt->p_buffers_input.pBuffers[0].planes[0].stride = p_xml_input->inject_params.stride;

            if ((0 == p_xml_input->inject_params.singlebuf) ||
                (p_xml_input->inject_params.singlebuf && (-1 == first_single_buf_input_idx)))
            {
                if (p_xml_input->inject_params.singlebuf)
                {
                    first_single_buf_input_idx = input_idx;
                }

                ret = test_util_init_window(test_util_ctxt, &input_ctxt->injection_window);
                if (ret != QCARCAM_RET_OK)
                {
                    QCARCAM_ERRORMSG("test_util_init_window failed for injection_window");
                    goto fail;
                }

                size = input_ctxt->p_buffers_input.pBuffers[0].planes[0].stride *
                    input_ctxt->p_buffers_input.pBuffers[0].planes[0].height;
                ret = test_util_allocate_input_buffers(input_ctxt->injection_window,
                                                       &input_ctxt->p_buffers_input,
                                                       size);
                if (ret != QCARCAM_RET_OK)
                {
                    QCARCAM_ERRORMSG("test_util_alloc_input_buffers failed");
                    goto fail;
                }

                ret = test_util_read_input_data(input_ctxt->injection_window,
                                                p_xml_input->inject_params.filename,
                                                p_xml_input->inject_params.n_frames,
                                                size);
                if (ret != QCARCAM_RET_OK)
                {
                    QCARCAM_ERRORMSG("test_util_read_input_data failed");
                    goto fail;
                }
            }
            else
            {
                qcarcam_test_input_t* input0_ctxt = &gCtxt.inputs[first_single_buf_input_idx];
                input_ctxt->p_buffers_input = input0_ctxt->p_buffers_input;
            }

            input_ctxt->is_injection = 1;
        }
    }

    QCARCAM_DBGMSG("Buffer Setup Begin");

    for (int input_idx = 0; input_idx < gCtxt.numInputs; input_idx++)
    {
        qcarcam_test_input_t *input_ctxt = &gCtxt.inputs[input_idx];
        input_ctxt->test_util_ctxt = test_util_ctxt;
        unsigned int output_n_buffers = 0;

        // Allocate an additional buffer to be shown in case of signal loss
        output_n_buffers = input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].n_buffers + 1;
        input_ctxt->p_buffers_output.nBuffers = output_n_buffers;

        input_ctxt->p_buffers_output.colorFmt = input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].format;
        input_ctxt->p_buffers_output.pBuffers = (QCarCamBuffer_t *)calloc(input_ctxt->p_buffers_output.nBuffers,
                                                                          sizeof(QCarCamBuffer_t));
        input_ctxt->diag.bprint = bprint_diag;
        if (!input_ctxt->p_buffers_output.pBuffers)
        {
            QCARCAM_ERRORMSG("alloc qcarcam_buffer failed");
            goto fail;
        }

        qcarcam_test_get_time(&t_before);
#if 0 //@todo: secure buffers
        if (input_ctxt->window_params.flags & QCARCAM_INPUT_FLAG_CONTENT_PROTECTED)
        {
            input_ctxt->p_buffers_output.flags = QCARCAM_BUFFER_FLAG_SECURE;
        }
#endif
        for (unsigned int i = 0; i < output_n_buffers; ++i)
        {
            input_ctxt->p_buffers_output.pBuffers[i].numPlanes = 1;
            input_ctxt->p_buffers_output.pBuffers[i].planes[0].width =
                input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].width;
            input_ctxt->p_buffers_output.pBuffers[i].planes[0].height =
                input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].height *
                input_ctxt->num_batch_frames;
        }

        ret = test_util_init_window(test_util_ctxt, &input_ctxt->qcarcam_window);
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("test_util_init_window failed for qcarcam_window");
            goto fail;
        }

        test_util_set_diag(test_util_ctxt, input_ctxt->qcarcam_window, &input_ctxt->diag);

        // set buffer to cacheable and offscreen
        if (gCtxt.enable_cache)
        {
            input_ctxt->window_params.is_offscreen = 1;
            input_ctxt->p_buffers_output.flags = QCARCAM_BUFFER_FLAG_CACHE;
        }

        if (gCtxt.enable_deinterlace && (gCtxt.enable_deinterlace == TESTUTIL_DEINTERLACE_SW_WEAVE))
        {
            ret = test_util_init_window(test_util_ctxt, &input_ctxt->display_window);
            if (ret != QCARCAM_RET_OK)
            {
                QCARCAM_ERRORMSG("test_util_init_window failed for display_window");
                goto fail;
            }
            test_util_set_diag(test_util_ctxt, input_ctxt->display_window, &input_ctxt->diag);

            ret = test_util_set_window_param(test_util_ctxt, input_ctxt->display_window, &input_ctxt->window_params);
        }
        else
        {
            ret = test_util_set_window_param(test_util_ctxt, input_ctxt->qcarcam_window, &input_ctxt->window_params);
        }

        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("test_util_set_window_param failed");
            goto fail;
        }

        ret = test_util_init_window_buffers(test_util_ctxt, input_ctxt->qcarcam_window, &input_ctxt->p_buffers_output);
        if (ret != QCARCAM_RET_OK)
        {
            QCARCAM_ERRORMSG("test_util_init_window_buffers failed");
            goto fail;
        }

        if (gCtxt.enable_deinterlace && (gCtxt.enable_deinterlace == TESTUTIL_DEINTERLACE_SW_WEAVE))
        {
            input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].format = QCARCAM_FMT_UYVY_8;
            input_ctxt->p_buffers_disp.colorFmt = (QCarCamColorFmt_e)input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].format;
            input_ctxt->p_buffers_disp.nBuffers = input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].n_buffers;
            input_ctxt->p_buffers_disp.pBuffers = (QCarCamBuffer_t *)calloc(input_ctxt->p_buffers_disp.nBuffers, sizeof(*input_ctxt->p_buffers_disp.pBuffers));
            if (!input_ctxt->p_buffers_disp.pBuffers)
            {
                QCARCAM_ERRORMSG("alloc qcarcam_buffer failed");
                goto fail;
            }

            for (unsigned int i = 0; i < input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].n_buffers; ++i)
            {
                input_ctxt->p_buffers_disp.pBuffers[i].numPlanes = 1;
                input_ctxt->p_buffers_disp.pBuffers[i].planes[0].width = input_ctxt->window_params.buffer_size[0];
                input_ctxt->p_buffers_disp.pBuffers[i].planes[0].height = input_ctxt->window_params.buffer_size[1];
            }

            ret = test_util_init_window_buffers(test_util_ctxt, input_ctxt->display_window, &input_ctxt->p_buffers_disp);
            if (ret != QCARCAM_RET_OK)
            {
                QCARCAM_ERRORMSG("test_util_init_window_buffers failed");
                goto fail;
            }
        }

        qcarcam_test_get_time(&t_after);
        QCARCAM_PERFMSG(gCtxt.enableStats, "qcarcam window init (idx %d) : %lu ms", input_ctxt->idx, (t_after - t_before));
        t_before = t_after;

        input_ctxt->p_buffers_output.nBuffers = input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].n_buffers;

#ifdef POST_PROCESS
        if (gCtxt.enable_c2d || gCtxt.enable_csc || gCtxt.enable_post_processing)
#else
        if (gCtxt.enable_c2d || gCtxt.enable_csc)
#endif
        {
            for (unsigned int i = 0; i < input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].n_buffers; ++i)
            {
                test_util_create_source_c2d_surface(input_ctxt->test_util_ctxt, input_ctxt->qcarcam_window, i);
                if (ret != QCARCAM_RET_OK)
                {
                    QCARCAM_ERRORMSG("test_util_create_source_c2d_surface failed");
                    goto fail;
                }
            }

            qcarcam_test_get_time(&t_after);
            QCARCAM_PERFMSG(gCtxt.enableStats, "qcarcam create_source_c2d_surface (idx %d) : %lu ms", input_ctxt->idx, (t_after - t_before));
            t_before = t_after;

            /*display window*/
            input_ctxt->p_buffers_disp.nBuffers = input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].n_buffers;
            input_ctxt->p_buffers_disp.colorFmt = input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].format;
            input_ctxt->p_buffers_disp.pBuffers = (QCarCamBuffer_t *)calloc(input_ctxt->p_buffers_disp.nBuffers, sizeof(*input_ctxt->p_buffers_disp.pBuffers));
            if (!input_ctxt->p_buffers_disp.pBuffers)
            {
                QCARCAM_ERRORMSG("alloc qcarcam_buffer failed");
                goto fail;
            }

            for (unsigned int i = 0; i < input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].n_buffers; ++i)
            {
                input_ctxt->p_buffers_disp.pBuffers[i].numPlanes = 1;
                input_ctxt->p_buffers_disp.pBuffers[i].planes[0].width = input_ctxt->window_params.buffer_size[0];
                input_ctxt->p_buffers_disp.pBuffers[i].planes[0].height = input_ctxt->window_params.buffer_size[1];
            }

            ret = test_util_init_window(test_util_ctxt, &input_ctxt->display_window);
            if (ret != QCARCAM_RET_OK)
            {
                QCARCAM_ERRORMSG("test_util_init_window failed");
                goto fail;
            }

            test_util_set_diag(test_util_ctxt, input_ctxt->display_window, &input_ctxt->diag);

            // set display buffer to non-offscreen
            if (gCtxt.enable_cache)
            {
                input_ctxt->window_params.is_offscreen = 0;
            }


            ret = test_util_set_window_param(test_util_ctxt, input_ctxt->display_window, &input_ctxt->window_params);
            if (ret != QCARCAM_RET_OK)
            {
                QCARCAM_ERRORMSG("test_util_set_window_param failed");
                goto fail;
            }

            ret = test_util_init_window_buffers(test_util_ctxt, input_ctxt->display_window, &input_ctxt->p_buffers_disp);
            if (ret != QCARCAM_RET_OK)
            {
                QCARCAM_ERRORMSG("test_util_init_window_buffers failed ret = %d",ret);
                goto fail;
            }

            qcarcam_test_get_time(&t_after);
            QCARCAM_PERFMSG(gCtxt.enableStats, "display window init (idx %d) : %lu ms", input_ctxt->idx, (t_after - t_before));
            t_before = t_after;

            for (unsigned int i = 0; i < input_ctxt->buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].n_buffers; ++i)
            {
                test_util_create_target_c2d_surface(input_ctxt->test_util_ctxt, input_ctxt->display_window, i);
                if (ret != QCARCAM_RET_OK)
                {
                    QCARCAM_ERRORMSG("test_util_create_target_c2d_surface failed");
                    goto fail;
                }
            }

            qcarcam_test_get_time(&t_after);
            QCARCAM_PERFMSG(gCtxt.enableStats, "display create_target_c2d_surface (idx %d) : %lu ms", input_ctxt->idx, (t_after - t_before));
            t_before = t_after;

#ifdef ENABLE_CL_CONVERTER
            input_ctxt->g_converter = csc_create();

            ret = test_util_init_cl_converter(test_util_ctxt, input_ctxt->qcarcam_window, input_ctxt->display_window, input_ctxt->g_converter, &(input_ctxt->source_surface), &(input_ctxt->target_surface));
            if (ret != QCARCAM_RET_OK)
            {
                QCARCAM_ERRORMSG("cl converter initialization failed");
                goto fail;
            }
            qcarcam_test_get_time(&t_after);
            QCARCAM_PERFMSG(gCtxt.enableStats, "init cl convert : %lu ms", (t_after - t_before));
            t_before = t_after;
#endif
#ifdef POST_PROCESS
            if(gCtxt.enable_post_processing)
            {
                gCtxt.post_processing_ctx[input_idx].pLibName = pLibName;
                ret =  test_util_init_post_processing(&gCtxt.post_processing_ctx[input_idx],
                                                  test_util_ctxt,
                                                  input_ctxt->qcarcam_window,
                                                  input_ctxt->display_window);
                if (ret != QCARCAM_RET_OK)
                {
                    QCARCAM_ERRORMSG("Post processing initialization failed");
                    goto fail;
                }
                qcarcam_test_get_time(&t_after);
                QCARCAM_PERFMSG(gCtxt.enableStats, "init post processing: %lu ms", (t_after - t_before));
                t_before = t_after;
            }
#endif

        }
    }

    QCARCAM_DBGMSG("Buffer Setup End");
    QCARCAM_DBGMSG("Create qcarcam_hndl Begin");

    /*launch threads to do the work for multithreaded*/
    if (gCtxt.multithreaded)
    {
        for (int input_idx = 0; input_idx < gCtxt.numInputs; input_idx++)
        {
            qcarcam_test_input_t *input_ctxt = &gCtxt.inputs[input_idx];

            input_ctxt->idx = input_idx;
            snprintf(thread_name, sizeof(thread_name), "inpt_ctxt_%d", input_idx);

            rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &qcarcam_test_setup_input_ctxt_thread, &gCtxt.inputs[input_idx], 0, thread_name, &input_ctxt->thread_handle);
            if (rc)
            {
                QCARCAM_ERRORMSG("CameraCreateThread failed : %s", thread_name);
                goto fail;
            }

            rc = CameraCreateSignal(&gCtxt.inputs[input_idx].m_eventHandlerSignal);
            if (rc)
            {
                QCARCAM_ERRORMSG("CameraCreateSignal failed, rc=%d", rc);
                goto fail;
            }

            snprintf(thread_name, sizeof(thread_name), "process_cb_event_%d",input_idx);
            rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &process_cb_event_thread, &gCtxt.inputs[input_idx], 0, thread_name, &input_ctxt->process_cb_event_handle);
            if (rc)
            {
                QCARCAM_ERRORMSG("Create cb event process thread failed, rc=%d", rc);
                goto fail;
            }

            if (input_ctxt->is_injection)
            {
                rc = CameraCreateSignal(&gCtxt.inputs[input_idx].m_injectionHandlerSignal);
                if (rc)
                {
                    QCARCAM_ERRORMSG("CameraCreateSignal failed, rc=%d", rc);
                    goto fail;
                }

                snprintf(thread_name, sizeof(thread_name), "injection_%d",input_idx);
                rc = CameraCreateThread(CAMERA_THREAD_PRIO_HIGH_REALTIME, 0, &injection_thread, &gCtxt.inputs[input_idx], 0, thread_name, &input_ctxt->injection_handle);
                if (rc)
                {
                    QCARCAM_ERRORMSG("Create injection thread failed, rc=%d", rc);
                    goto fail;
                }
            }

            if (input_ctxt->delay_time)
            {
                snprintf(thread_name, sizeof(thread_name), "inpt_ctxt_usr_process_%d", input_idx);
                rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &inpt_ctxt_usr_process_thread, &gCtxt.inputs[input_idx], 0, thread_name, &input_ctxt->usr_process_thread_handle);
                if (rc)
                {
                    QCARCAM_ERRORMSG("CameraCreateThread failed : %s", thread_name);
                    goto fail;
                }
            }
        }

        if (gCtxt.exitSeconds)
        {
            snprintf(thread_name, sizeof(thread_name), "timer_thread");
            rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &timer_thread, 0, 0, thread_name, &timer_thread_handle);
            if (rc)
            {
                QCARCAM_ERRORMSG("CameraCreateThread failed, rc=%d", rc);
                goto fail;
            }
        }

        if (gCtxt.enableMenuMode)
        {
            while (!g_aborted)
            {
                if (gCtxt.opened_stream_cnt < gCtxt.numInputs)
                {
                    // Wait till all streams starts
                    usleep(10000);
                    continue;
                }
                fflush(stdin);
                display_menu();

                while (!g_aborted)
                {
                    uint32 option = 0;
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
                            option = buf[0];
                            if ('e' == option)
                            {
                                abort_test();
                            }
                            else if ('s' == option)
                            {
                                process_cmds(QCARCAM_TEST_MENU_DUMP_NEXT_FRAME);
                            }
                            else if ('h' == option)
                            {
                                display_menu();
                            }
                            else
                            {
                                option = strtoul(buf, NULL, 0);
                                process_cmds(option);
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        /*singlethreaded - init and start each camera sequentially*/
        for (int input_idx = 0; input_idx < gCtxt.numInputs; input_idx++)
        {
            qcarcam_test_input_t *input_ctxt = &gCtxt.inputs[input_idx];

            input_ctxt->idx = input_idx;
            if (0 != qcarcam_test_setup_input_ctxt_thread(&gCtxt.inputs[input_idx]))
            {
                QCARCAM_ERRORMSG("qcarcam_test_setup_input_ctxt_thread failed");
                goto fail;
            }
        }

        for (int input_idx = 0; input_idx < gCtxt.numInputs; input_idx++)
        {
            qcarcam_test_input_t *input_ctxt = &gCtxt.inputs[input_idx];

            input_ctxt->is_first_start = TRUE;
            ret = qcarcam_input_start(input_ctxt);
            if (ret != QCARCAM_RET_OK)
            {
                QCarCamClose(input_ctxt->qcarcam_hndl);
                input_ctxt->qcarcam_hndl = QCARCAM_HNDL_INVALID;
            }

            rc = CameraCreateSignal(&gCtxt.inputs[input_idx].m_eventHandlerSignal);
            if (rc)
            {
                QCARCAM_ERRORMSG("CameraCreateSignal failed, rc=%d", rc);
                goto fail;
            }

            snprintf(thread_name, sizeof(thread_name), "process_cb_event_%d",input_idx);
            rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &process_cb_event_thread, &gCtxt.inputs[input_idx], 0, thread_name, &input_ctxt->process_cb_event_handle);
            if (rc)
            {
                QCARCAM_ERRORMSG("Create cb event process thread failed, rc=%d", rc);
                goto fail;
            }

            if (input_ctxt->is_injection)
            {
                rc = CameraCreateSignal(&gCtxt.inputs[input_idx].m_injectionHandlerSignal);
                if (rc)
                {
                    QCARCAM_ERRORMSG("CameraCreateSignal failed, rc=%d", rc);
                    goto fail;
                }

                snprintf(thread_name, sizeof(thread_name), "injection_%d",input_idx);
                rc = CameraCreateThread(CAMERA_THREAD_PRIO_HIGH_REALTIME, 0, &injection_thread, &gCtxt.inputs[input_idx], 0, thread_name, &input_ctxt->injection_handle);
                if (rc)
                {
                    QCARCAM_ERRORMSG("Create injection thread failed, rc=%d", rc);
                    goto fail;
                }
            }
        }

        if (gCtxt.exitSeconds)
        {
            snprintf(thread_name, sizeof(thread_name), "timer_thread");
            rc = CameraCreateThread(QCARCAM_THRD_PRIO, 0, &timer_thread, 0, 0, thread_name, &timer_thread_handle);
            if (rc)
            {
                QCARCAM_ERRORMSG("CameraCreateThread failed, rc=%d", rc);
                goto fail;
            }
        }

        while (!g_aborted)
        {
            for (int input_idx = 0; input_idx < gCtxt.numInputs; input_idx++)
            {
                qcarcam_test_input_t *input_ctxt = &gCtxt.inputs[input_idx];

                if (!input_ctxt->use_event_callback)
                {
                    if (qcarcam_test_handle_new_frame(input_ctxt))
                        break;
                }
                else
                {
                    sched_yield();
                }
            }
        }
    }

fail:

    /*Wait on all of them to join*/
    for (int input_idx = 0; input_idx < gCtxt.numInputs; input_idx++)
    {
        qcarcam_test_input_t *input_ctxt = &gCtxt.inputs[input_idx];

        if (input_ctxt->thread_handle)
            CameraJoinThread(input_ctxt->thread_handle, NULL);

        if (input_ctxt->m_eventHandlerSignal)
            CameraSetSignal(input_ctxt->m_eventHandlerSignal);

        if (input_ctxt->process_cb_event_handle)
            CameraJoinThread(input_ctxt->process_cb_event_handle, NULL);

        if (input_ctxt->injection_handle)
            CameraJoinThread(input_ctxt->injection_handle, NULL);

        if (input_ctxt->m_eventHandlerSignal)
            CameraDestroySignal(input_ctxt->m_eventHandlerSignal);

        if (input_ctxt->m_injectionHandlerSignal)
            CameraDestroySignal(input_ctxt->m_injectionHandlerSignal);

        input_ctxt->m_eventHandlerSignal= NULL;
        input_ctxt->m_injectionHandlerSignal = NULL;
    }

    // cleanup
    for (int input_idx = 0; input_idx < gCtxt.numInputs; input_idx++)
    {
        qcarcam_test_input_t *input_ctxt = &gCtxt.inputs[input_idx];

        if (input_ctxt->qcarcam_hndl != QCARCAM_HNDL_INVALID)
        {
            pthread_mutex_lock(&input_ctxt->mutex);
            (void)QCarCamStop(input_ctxt->qcarcam_hndl);
            (void)QCarCamClose(input_ctxt->qcarcam_hndl);
            input_ctxt->state = QCARCAMTEST_STATE_CLOSED;
            pthread_mutex_unlock(&input_ctxt->mutex);
            input_ctxt->qcarcam_hndl = QCARCAM_HNDL_INVALID;
        }

        if (input_ctxt->qcarcam_window)
        {
            test_util_deinit_window_buffer(test_util_ctxt, input_ctxt->qcarcam_window);
            test_util_deinit_window(test_util_ctxt, input_ctxt->qcarcam_window);
        }

        if (input_ctxt->injection_window)
        {
            test_util_free_input_buffers(input_ctxt->injection_window, &input_ctxt->p_buffers_input);
            test_util_deinit_window(test_util_ctxt, input_ctxt->injection_window);
        }

        if (input_ctxt->delay_time)
        {
            for (unsigned int idx = 0; idx < input_ctxt->p_buffers_output.nBuffers; idx++)
            {
                 if (input_ctxt->buf_timer[idx].ptimer)
                 {
                     CameraReleaseTimer(input_ctxt->buf_timer[idx].ptimer);
                 }
            }
        }

#ifdef POST_PROCESS
        if (gCtxt.enable_post_processing || gCtxt.enable_csc || gCtxt.enable_c2d || (gCtxt.enable_deinterlace && (gCtxt.enable_deinterlace == TESTUTIL_DEINTERLACE_SW_WEAVE)))
#else
        if (gCtxt.enable_csc || gCtxt.enable_c2d || (gCtxt.enable_deinterlace && (gCtxt.enable_deinterlace == TESTUTIL_DEINTERLACE_SW_WEAVE)))
#endif
        {
            if (input_ctxt->display_window)
            {
                test_util_deinit_window_buffer(test_util_ctxt, input_ctxt->display_window);
                test_util_deinit_window(test_util_ctxt, input_ctxt->display_window);
            }
        }
        if (input_ctxt->p_buffers_output.pBuffers)
        {
            free(input_ctxt->p_buffers_output.pBuffers);
            input_ctxt->p_buffers_output.pBuffers = NULL;
        }

        if (input_ctxt->p_buffers_disp.pBuffers)
        {
            free(input_ctxt->p_buffers_disp.pBuffers);
            input_ctxt->p_buffers_disp.pBuffers = NULL;
        }

#ifdef ENABLE_CL_CONVERTER
        if (gCtxt.enable_csc)
        {
            test_util_deinit_cl_converter(input_ctxt->g_converter, &(input_ctxt->source_surface), &(input_ctxt->target_surface));
        }
#endif
#ifdef POST_PROCESS
        if (gCtxt.enable_post_processing)
        {
            test_util_deinit_post_processing(&gCtxt.post_processing_ctx[input_idx]);
        }
#endif
    }

    if (pInputs)
    {
        free(pInputs);
        pInputs = NULL;
    }

    if (xml_inputs)
    {
        free(xml_inputs);
        xml_inputs = NULL;
    }

    if (!g_aborted)
    {
        abort_test();
    }

    for (unsigned int inputIdx = 0; inputIdx < queryFilled; inputIdx++)
    {
        if (gCtxt.queryInputs[inputIdx].modesDesc.pModes)
        {
            free(gCtxt.queryInputs[inputIdx].modesDesc.pModes);
        }
    }

    test_util_deinit(test_util_ctxt);

    QCarCamUninitialize();

    return rval;
}

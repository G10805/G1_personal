
/* ===========================================================================
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 =========================================================================== */

#ifndef INPUT_STREAM_H
#define INPUT_STREAM_H

#include "V4L2Wrapper.h"
#include "qcarcam_types.h"
#include "ais_log.h"
#include "CameraOSServices.h"
#include "test_util.h"

using ais_v4l2_test::V4L2Wrapper;
#define QCARCAM_THRD_PRIO CAMERA_THREAD_PRIO_DEFAULT

typedef struct
{
    uint32 test_mode;
    uint32 ss_interval_frame_num;
    int _disable_display;
    int _dumpFrame;
    int _use_buf_width;
}test_cfg_t;

int get_system_time(unsigned long &rTime);


class InputStream
{
public:
    typedef enum
    {
        TEST_VENDOR_EXTENSION,    // test the vensor extension
        TEST_LATENCY_1,            // test the latency set
        TEST_START,
        TEST_STOP,

    }test_type;

    enum TEST_MODE
    {
        TEST_MODE_PREVIEW = 0X1,    // in this mode, only test preview
        TEST_MODE_PARAMS  = 0X2,    // in this mode, only test the set/get params
        TEST_MODE_ALL     = 0X3,    // in this mode, test all
        TEST_MODE_STARTSTOP = 0X10, // in this mode, test startstop
        TEST_MODE_NEGATIVE = 0X20,  // negative mode for coverage
    };

    typedef enum
    {
        QCARCAMTEST_BUFFER_STATE_INIT,
        QCARCAMTEST_BUFFER_STATE_QCARCAM,
        QCARCAMTEST_BUFFER_STATE_GET_FRAME,
        QCARCAMTEST_BUFFER_STATE_POST_DISPLAY,
        QCARCAMTEST_BUFFER_STATE_MAX = 0x7FFFFFF
    }qcarcam_test_buffer_state_t;

    typedef enum
    {
        QCARCAM_TEST_BUFFERS_OUTPUT = 0,
        QCARCAM_TEST_BUFFERS_DISPLAY,
        QCARCAM_TEST_BUFFERS_INPUT,
        QCARCAM_TEST_BUFFERS_MAX
    } qcarcam_test_buffers_type;

    typedef enum
    {
        STREAM_STATE_INVALID = 0,

        STREAM_STATE_OPEN,
        STREAM_STATE_START,
        STREAM_STATE_STOP,

        STREAM_STATE_ERROR,
        STREAM_STATE_CLOSED,


    }stream_state_t;
    ;

    typedef struct
    {
        QCarCamBufferList_t p_buffers;

        QCarCamColorFmt_e format;
        int n_buffers;
        unsigned int width;
        unsigned int height;
    } qcarcam_buffers_param_t;

public:
    InputStream(int index);
    ~InputStream();

    int init(test_util_ctxt_t* test_util_ctxt, test_util_xml_input_t *xml_inputs);
    void uninit();
    static int stream_thread(void* argv);
    void set_ctrl(test_type type);
    void printfps(unsigned long time1, unsigned long time2);
    int start();
    int stop();
    int get_write_pipe() { return m_pipefd[1]; }
    stream_state_t get_state() { return m_state; }
    int set_batch_mode(unsigned int batch_frames);
    int set_isp_usecase(QCarCamIspUsecaseConfig_t isp_config);

private:
    int init_window_buffer(test_util_ctxt_t* test_util_ctxt, test_util_xml_input_t *xml_inputs);
    int launch_stream_thread();
    void join_stream_thread();
    void uninit_window_buffer();
    int get_frame();
    int post_display();
    int release_buffers();
    int processV4L2Event(int fd);
    int process_pipe_data(int data);
    int handle_input_signal(QCarCamInputSignal_e signal_type);

private:
    CameraThread m_thread_handle;

    int m_idx;
    pthread_mutex_t m_mutex;
    uint32_t m_qcarcam_input_id;
    V4L2Wrapper* m_wrapper;
    int m_pipefd[2];

    stream_state_t m_state;

    qcarcam_buffers_param_t m_buffers_param[QCARCAM_TEST_BUFFERS_MAX];

    QCarCamBufferList_t m_p_buffers_output;
    QCarCamBufferList_t m_p_buffers_disp;
    QCarCamBufferList_t m_p_buffers_input;
    uint32 m_nbufs;

    QCarCamInputSrc_t m_resolution;

    QCarCamOpmode_e m_op_mode;

    unsigned int m_num_isp_instances;
    QCarCamIspUsecaseConfig_t m_isp_config[QCARCAM_MAX_ISP_INSTANCES];

    /* test util objects */
    test_util_ctxt_t *m_test_util_ctxt;
    test_util_window_t *m_qcarcam_window;
    test_util_window_t *m_display_window;
    test_util_window_param_t m_window_params;

    /* buffer management tracking */
    uint32 m_buf_idx_qcarcam;
    std::list<uint32> m_release_buf_idx_list;
    //    uint32 m_releaseframeCnt;
    uint32 m_n_frame;

    //int _frameCnt;
    int m_releaseframeCnt;
    int m_prev_frameCnt;
    int m_prev_releaseframeCnt;
    bool m_is_first_start;
    unsigned long m_start_time;

    qcarcam_test_buffer_state_t m_buf_state[QCARCAM_MAX_NUM_BUFFERS];
    int m_exit_stream_thread;

    CameraTimer m_startstop_timer;
    CameraTimer m_ctrl_timer;

    V4L2Wrapper::BufferDesc m_buf_desc;
    unsigned int m_batch_frames;

    int m_signal_lost;
    V4L2Wrapper::StreamFormat m_v4l2fmt;

};


#endif

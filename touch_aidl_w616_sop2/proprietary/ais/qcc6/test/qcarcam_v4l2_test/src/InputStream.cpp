/* ===========================================================================
 * Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 =========================================================================== */

#include "InputStream.h"
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
#include <media/ais_v4l2loopback.h>
#include <media/cam_defs.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <error.h>
#include <sys/ioctl.h>


static uint8 s_vendor_extension_param[MAX_AIS_V4L2_PARAM_EVNET_SIZE] = { 0x45 };
static const uint32 s_startstop_delay = 1000;
extern test_cfg_t g_test_cfg;
extern test_util_global_config_t g_xml_cfg;

int get_system_time(unsigned long &rTime)
{
    struct timespec time;
    unsigned long msec;

    if (clock_gettime(CLOCK_MONOTONIC, &time) == -1)
    {
        CAM_MSG(ERROR,"Clock gettime failed");
        return 1;
    }
    msec = ((unsigned long)time.tv_sec * 1000) + (((unsigned long)time.tv_nsec / 1000) / 1000);
    rTime = msec;

    return 0;
}

static void timer_cb_start(void *arg)
{
    InputStream* p = (InputStream*)arg;
    if (p)
    {
        QCARCAM_INFOMSG("timer cb called for input");
        int tmp = InputStream::TEST_START;
        write(p->get_write_pipe(), &tmp, sizeof(tmp));
    }
}

static QCarCamRet_e send_ioctrl_cmd(int fd, uint8 op_code, uint32 param_type, unsigned int size, uint8* payload)
{
    int rc = 0;
    struct ais_v4l2_control_t ctrl;
    ctrl.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ctrl.payload = (uint64)payload;
    ctrl.cmd = op_code;
    ctrl.param_type = param_type;
    ctrl.size = size;
    rc = ioctl(fd,
        VIDIOC_CAM_CONTROL,
        &ctrl);
    if (rc < 0)
    {
        QCARCAM_ERRORMSG("send_ioctrl_cmd %d %u %u failed", rc, op_code, param_type);
        return QCARCAM_RET_FAILED;
    }
    return QCARCAM_RET_OK;
}
InputStream::InputStream(int input_idx)
: m_thread_handle(NULL)
, m_idx(input_idx)
, m_state(STREAM_STATE_INVALID)
, m_num_isp_instances(0)
, m_exit_stream_thread(0)
, m_startstop_timer(NULL)
{
    memset(m_isp_config, 0x0, sizeof(m_isp_config));
}

InputStream::~InputStream()
{
}

int InputStream::init(test_util_ctxt_t* test_util_ctxt, test_util_xml_input_t *xml_inputs)
{
    m_qcarcam_input_id = xml_inputs[m_idx].properties.qcarcam_input_id;

    QCARCAM_ALWZMSG("v4l2 device node %s", xml_inputs[m_idx].properties.node_name);
    m_wrapper = new V4L2Wrapper(xml_inputs[m_idx].properties.node_name, V4L2Wrapper::IO_MODE_DMABUF);
    if (NULL == m_wrapper)
    {
        QCARCAM_ERRORMSG("Failed to create V4L2Wrapper");
        return 1;
    }

    if (m_wrapper->connect(false))
    {
        QCARCAM_ERRORMSG("Failed to connect V4L2Wrapper");
        return 1;
    }

    if (init_window_buffer(test_util_ctxt, xml_inputs))
    {
        QCARCAM_ERRORMSG("Failed to init window&buffer");
        return 1;
    }

    if (g_test_cfg.test_mode == TEST_MODE_NEGATIVE)
    {
        start();
        start();
        stop();
        stop();

    }
    else
    {
        if (launch_stream_thread())
        {
            QCARCAM_ERRORMSG("Failed to launch_camerastreams");
            return 1;
        }

        if (g_test_cfg.test_mode & TEST_MODE_STARTSTOP)
        {
            // create the timer
            if (CameraCreateTimer(0, 0, timer_cb_start, this, &m_startstop_timer))
            {
                QCARCAM_ERRORMSG("CameraCreateTimer failed for input %d", m_idx);
                return -1;
            }

            CameraStopTimer(m_startstop_timer);
            QCARCAM_ERRORMSG("CameraCreateTimer succeed for input %d", m_idx);
        }
    }

    return 0;
}

void InputStream::uninit()
{
    int tmp = -1;

    // destory the timer
    if (m_startstop_timer)
    {
        CameraReleaseTimer(m_startstop_timer);
        m_startstop_timer = NULL;
    }

    if (m_thread_handle)
    {
        m_exit_stream_thread = 1;
        write(m_pipefd[1], &tmp, sizeof(tmp));
        join_stream_thread();

        // todo: refine the case when init_window_buffer failed
        uninit_window_buffer();
    }

    if (m_wrapper)
    {
        m_wrapper->disconnect();
        delete m_wrapper;
        m_wrapper = NULL;
    }
}

void InputStream::printfps(unsigned long time1, unsigned long time2)
{
    int frames_counted;
    float average_fps;
    if (m_state == STREAM_STATE_START)
    {
        if (m_is_first_start && time2 > m_start_time)
        {
            m_prev_frameCnt = m_n_frame;
            m_is_first_start = false;
            average_fps = m_n_frame * 1000/(time2 - m_start_time);
            printf("Average FPS: %.2f input_id: %d idx: %d\n ", average_fps, (int)m_qcarcam_input_id, m_idx);
        }
        else
        {
            frames_counted = m_n_frame - m_prev_frameCnt;
            average_fps = frames_counted * 1000 / (time2 - time1) ;
            printf("Average FPS: %.2f input_id: %d idx: %d\n ", average_fps, (int)m_qcarcam_input_id, m_idx);
            m_prev_frameCnt = m_n_frame;
        }
    }
}

int InputStream::start()
{
    int rc = m_wrapper->streamon();
    if (rc)
    {
        CAM_MSG(ERROR,  "fail to start %d", rc);
        return 1;
    }

    CAM_MSG(HIGH, "[input %d %d] start", m_qcarcam_input_id, m_idx);

    m_is_first_start = true;
    m_prev_frameCnt = 0;
    m_n_frame = 0;
    get_system_time(m_start_time);
    m_state = STREAM_STATE_START;
    m_signal_lost = 0;

    return 0;
}

int InputStream::stop()
{
    int rc = m_wrapper->streamoff();
    if (rc)
    {
        CAM_MSG(ERROR,  "fail to stop %d", rc);
        return 1;
    }
    CAM_MSG(HIGH, "[input %d %d] stop", m_qcarcam_input_id, m_idx);

    m_state = STREAM_STATE_STOP;

    return 0;
}

int InputStream::set_batch_mode(unsigned int batch_frames)
{
    int rc = 0;
    int fd = m_wrapper->getfd();

    QCarCamBatchConfig_t param;
    memset(&param, 0, sizeof(param));
    param.numBatchFrames = batch_frames;

    rc = send_ioctrl_cmd(fd, AIS_V4L2_CAPTURE_PRIV_SET_PARAM,
        AIS_V4L2_PARAM_BATCH_MODE, MAX_AIS_V4L2_PAYLOAD_SIZE, (uint8*)&param);
    if (rc)
    {
        CAM_MSG(ERROR,  "VIDIOC_S_PARAM failed %d", rc);
        return rc;
    }
    else
    {
        CAM_MSG(ALWAYS, "batch_frames 0x%x", param.numBatchFrames);
    }

    return rc;

}

int InputStream::set_isp_usecase(QCarCamIspUsecaseConfig_t isp_config)
{
    int rc = 0;
    int fd = m_wrapper->getfd();

    QCarCamIspUsecaseConfig_t param;
    memset(&param, 0, sizeof(param));
    param = isp_config;

    rc = send_ioctrl_cmd(fd, AIS_V4L2_CAPTURE_PRIV_SET_PARAM,
        AIS_V4L2_PARAM_ISP_USECASE, MAX_AIS_V4L2_PAYLOAD_SIZE, (uint8*)&param);
    if (rc)
    {
        CAM_MSG(ERROR,  "VIDIOC_S_PARAM failed %d", rc);
        return rc;
    }
    else
    {
        CAM_MSG(ALWAYS, "Set ISP instance(%d) isp camera id %d, use case %d",
            param.id, param.cameraId, param.usecaseId);
    }

    return rc;

}

static int subscribeEvents(int fd, uint32 type, uint32 id)
{
    int ret = 0;
    struct v4l2_event_subscription sub = {};
    int rc = -1;

    sub.type = type;
    sub.id   = id;

    rc = ioctl(fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    if (0 != rc)
    {
        CAM_MSG(ERROR,  "Subscribe event %d %d failed %d", type, id, rc);
        ret = 1;
    }

    return ret;
}

static int test_vendor_extension(int fd)
{
    int rc = 0;
    QCarCamVendorParam_t payload;

    memcpy(&payload, s_vendor_extension_param, sizeof(s_vendor_extension_param));

    rc = send_ioctrl_cmd(fd, AIS_V4L2_CAPTURE_PRIV_SET_PARAM,
        AIS_V4L2_PARAM_VENDOR, sizeof(s_vendor_extension_param),
        (uint8*)&payload);
    if (rc)
    {
        CAM_MSG(ERROR,  "S_PARAM failed %d", rc);
        return rc;
    }
    else
    {
        CAM_MSG(ALWAYS, "s_param data[0] %x data[1] %x",
            payload.data[0], payload.data[1]);
    }

    memset(&payload, 0, sizeof(payload));
    // for get, also need the size, set it MAX_AIS_V4L2_PARAM_EVNET_SIZE here
    rc = send_ioctrl_cmd(fd, AIS_V4L2_CAPTURE_PRIV_GET_PARAM,
        AIS_V4L2_PARAM_VENDOR, MAX_AIS_V4L2_PARAM_EVNET_SIZE, (uint8*)&payload);
    if (rc)
    {
        CAM_MSG(ERROR,  "G_PARAM failed %d", rc);
        return rc;
    }
    else
    {
        uint8* p = (uint8*)payload.data;
        for (uint32 i = 3; i < MAX_AIS_V4L2_PARAM_EVNET_SIZE; i+=4)
        {
            CAM_MSG(ALWAYS, "g_param data[%u-%u] %d %d %d %d", i-3, i,
                p[i-3], p[i-2],
                p[i-1], p[i]);
        }
    }

    memset(&payload, 0, sizeof(payload));
    // for get, also need the size, set it MAX_AIS_V4L2_PARAM_EVNET_SIZE here
    rc = send_ioctrl_cmd(fd, AIS_V4L2_CAPTURE_PRIV_GET_PARAM,
        AIS_V4L2_PARAM_VENDOR, MAX_AIS_V4L2_PAYLOAD_SIZE-1, (uint8*)&payload);
    if (rc)
    {
        CAM_MSG(ERROR,  "G_PARAM failed %d", rc);
        return rc;
    }
    else
    {
        uint8* p = (uint8*)payload.data;
        for (uint32 i = 3; i < (MAX_AIS_V4L2_PAYLOAD_SIZE-1) ; i+=4)
        {
            CAM_MSG(ALWAYS, "g_param data[%u-%u] %d %d %d %d", i-3, i,
                p[i-3], p[i-2],
                p[i-1], p[i]);
        }
    }

    return 0;

}

static int test_vendor_extension1(int fd)
{
    int rc = 0;

    QCarCamVendorParam_t payload;
    memset(payload.data, 0x45, sizeof(payload));

    rc = send_ioctrl_cmd(fd, AIS_V4L2_CAPTURE_PRIV_SET_PARAM,
        AIS_V4L2_PARAM_VENDOR, MAX_AIS_V4L2_PAYLOAD_SIZE, (uint8*)&payload);
    if (rc)
    {
        CAM_MSG(ERROR,  "VIDIOC_S_PARAM failed %d", rc);
        return rc;
    }
    else
    {
        CAM_MSG(ALWAYS, "s_param data[0] %x data[1] %x",
            payload.data[0], payload.data[1]);
    }

    return 0;

}

static bool check_vendor_extension(uint8* payload)
{
    for (uint32 i = 0 ;i < MAX_AIS_V4L2_PARAM_EVNET_SIZE; ++i)
    {
        if (payload[i] != s_vendor_extension_param[i])
        {
            return false;
        }
    }

    return true;
}

static int test_latency_1(int fd)
{
    int rc = 0;

    unsigned int uint_value = 5; // the latency_max is unsigned int uint_value; in qcarcam_types.h

    rc = send_ioctrl_cmd(fd, AIS_V4L2_CAPTURE_PRIV_SET_PARAM,
        AIS_V4L2_PARAM_LATENCY_MAX, sizeof(uint_value), (uint8*)&uint_value);
    if (rc)
    {
        CAM_MSG(ERROR,  "s_param latency max failed %d", rc);
        return rc;
    }
    else
    {
        CAM_MSG(HIGH, "s_param latency max %u", uint_value);
    }

    uint_value = 0;
    rc = send_ioctrl_cmd(fd, AIS_V4L2_CAPTURE_PRIV_SET_PARAM,
        AIS_V4L2_PARAM_LATENCY_REDUCE_RATE, sizeof(uint_value), (uint8*)&uint_value);
    if (rc)
    {
        CAM_MSG(ERROR,  "s_param LATENCY_REDUCE_RATE failed %d", rc);
        return rc;
    }
    else
    {
        CAM_MSG(HIGH, "s_param LATENCY_REDUCE_RATE %u", uint_value);
    }

    return 0;
}

int InputStream::handle_input_signal(QCarCamInputSignal_e signal_type)
{
    int ret = QCARCAM_RET_OK;

    switch (signal_type) {
    case QCARCAM_INPUT_SIGNAL_LOST:
        QCARCAM_ERRORMSG("LOST: idx: %d, input: %d", m_idx, m_qcarcam_input_id);

        /*TODO: offload this to other thread to handle restart recovery*/
        if (m_state == STREAM_STATE_STOP) {
            QCARCAM_ERRORMSG("Input %d already stop, break", m_qcarcam_input_id);

            break;
        }

        m_signal_lost = 1;
        stop();
        break;
    case QCARCAM_INPUT_SIGNAL_VALID:
        QCARCAM_ERRORMSG("VALID: idx: %d, input: %d", m_idx, m_qcarcam_input_id);
        if (m_state == InputStream::STREAM_STATE_START)
        {
            QCARCAM_ERRORMSG("Input %d already running, break", m_qcarcam_input_id);
            break;
        }

        ret = start();

        break;
    default:
        QCARCAM_ERRORMSG("Unknown Event type: %d", signal_type);
        break;
    }

    return ret;
}


int InputStream::process_pipe_data(int data)
{
    int rc = 0;
    switch(data)
    {
    case TEST_START:   // start command
        return start();
    case TEST_STOP:   // stop
        return stop();
    case TEST_VENDOR_EXTENSION:
        {
            rc = test_vendor_extension(m_wrapper->getfd());
            if (rc)
            {
                CAM_MSG(ERROR, "Failed to test_vendor_extension");
            }
        }
        break;
    case TEST_LATENCY_1:
       {
           rc = test_latency_1(m_wrapper->getfd());
           if (rc)
           {
               CAM_MSG(ERROR, "Failed to test_latency_1");
           }
       }
       break;
    default:
        CAM_MSG(ERROR, "unsurported data %d", data);
        break;
    }

    return 0;
}

int InputStream::stream_thread(void* arg)
{
    uint32 buf_idx = 0;

    uint32 i = 0;
    InputStream *input_ctxt = (InputStream *)arg;
    int rc = 0;

    struct pollfd pollFds[2];
    int pollNumFds;
    int pipe_data;

    input_ctxt->m_n_frame = 0;
    memset(input_ctxt->m_pipefd, 0x0, sizeof(input_ctxt->m_pipefd));

    rc = pipe(input_ctxt->m_pipefd);
    if (rc < 0)
    {
        CAM_MSG(ERROR, "Failed to create pipe %d");
        return 1;
    }

    V4L2Wrapper& wrapper = *(input_ctxt->m_wrapper);
    //V4L2Wrapper::StreamFormat format = { QCARCAM_YUV_UYVY, 1920, 1020, 1920*1020*2};

    pollFds[0].fd     = input_ctxt->m_pipefd[0];
    pollFds[0].events = POLLIN|POLLRDNORM;
    pollFds[1].fd = wrapper.getfd();
    pollFds[1].events = POLLIN|POLLRDNORM|POLLPRI|POLLOUT|POLLERR;
    pollNumFds        = 2;

    //TODO: doesn't support setformat now, needs to support it
    //wrapper.setformat(format);

    if (input_ctxt->start())
    {
        CAM_MSG(ERROR, "Failed to start");
        rc = 1;
        goto stream_fail;
    }

    while (!input_ctxt->m_exit_stream_thread)
    {
        rc = poll(pollFds, pollNumFds, -1);

        CAM_MSG(MEDIUM, "after poll %d %d", pollFds[1].revents & POLLIN, pollFds[1].revents & POLLPRI);
        if (pollFds[0].revents & POLLIN)
        {
            int nRead = 0;
            nRead = read(pollFds[0].fd, &pipe_data, sizeof(pipe_data));
            if (nRead == sizeof(pipe_data))
            {
                CAM_MSG(HIGH, "pipe data is %d", pipe_data);
                if (pipe_data == -1)
                {
                    break;
                }
                else
                {
                    if (input_ctxt->process_pipe_data(pipe_data))
                    {
                        QCARCAM_ERRORMSG("process pipe data failed for input %d", input_ctxt->m_idx);
                        break;
                    }
                }
            }
        }
        else if (pollFds[1].revents & POLLPRI)
        {
            rc = input_ctxt->processV4L2Event(wrapper.getfd());
        }
        else if (pollFds[1].revents & POLLERR)
        {
            QCARCAM_ERRORMSG("[input %d %d] receive POLLERR", input_ctxt->m_qcarcam_input_id, input_ctxt->m_idx);
            break;
        }

        if (pollFds[1].revents & POLLIN)
        {
            QCARCAM_INFOMSG("begin to get frame num %u", input_ctxt->m_n_frame);
            if ((g_test_cfg.test_mode & TEST_MODE_PREVIEW)
                && input_ctxt->get_frame() == 0)
            {
                if (g_test_cfg._dumpFrame && (input_ctxt->m_n_frame % g_test_cfg._dumpFrame) == 0)
                {

                   char filename[128] = "";
                   snprintf(filename, sizeof(filename), "/tmp/frame_%d_%u_%u.raw",
                    input_ctxt->m_idx, input_ctxt->m_n_frame, input_ctxt->m_buf_idx_qcarcam);
                   test_util_dump_window_buffer(input_ctxt->m_test_util_ctxt, input_ctxt->m_qcarcam_window, input_ctxt->m_buf_idx_qcarcam, filename);
                }

                if (!g_test_cfg._disable_display)
                {
                    input_ctxt->post_display();
                }
                else
                {
                    input_ctxt->m_release_buf_idx_list.push_back(input_ctxt->m_buf_idx_qcarcam);
                }

                input_ctxt->release_buffers();
                ++input_ctxt->m_n_frame;
                if (input_ctxt->m_n_frame == 1)
                {
                    printf("[input %d %d] succeed to get the first frame\n", input_ctxt->m_qcarcam_input_id, input_ctxt->m_idx);
                }

                QCARCAM_INFOMSG("[input %d %d] frame num %u", input_ctxt->m_qcarcam_input_id, input_ctxt->m_idx, input_ctxt->m_n_frame);
            }

            if (input_ctxt->m_n_frame == g_test_cfg.ss_interval_frame_num &&
                (g_test_cfg.test_mode & TEST_MODE_STARTSTOP))
            {
                CAM_MSG(HIGH, "[input %d %d] startstop first stop", input_ctxt->m_qcarcam_input_id, input_ctxt->m_idx);
                if (input_ctxt->stop())
                {
                    QCARCAM_ERRORMSG("[input %d %d] stop failed", input_ctxt->m_qcarcam_input_id, input_ctxt->m_idx);
                    break;
                }
                input_ctxt->m_n_frame = 0;
                // start the timer
                if (CameraUpdateTimer(input_ctxt->m_startstop_timer, s_startstop_delay))
                {
                    QCARCAM_ERRORMSG("[input %d %d] CameraUpdateTimer failed ", input_ctxt->m_qcarcam_input_id, input_ctxt->m_idx);
                }
            }

        }
    }

    input_ctxt->stop();
    wrapper.releasebufs();

stream_fail:
    close(input_ctxt->m_pipefd[0]);
    close(input_ctxt->m_pipefd[1]);

    QCARCAM_INFOMSG("Exit inputchannel_thread");

    return 0;
}

void InputStream::set_ctrl(test_type type)
{
    int tmp = type;
    write(get_write_pipe(), &tmp, sizeof(tmp));
}

int InputStream::init_window_buffer(test_util_ctxt_t* test_util_ctxt, test_util_xml_input_t *xml_inputs)
{

    InputStream* input_ctxt = this;
    test_util_xml_input_t* p_xml_input = &xml_inputs[m_idx];
    int ret = 0;
    bool use_buf_width = g_test_cfg._use_buf_width;

    pthread_mutex_init(&input_ctxt->m_mutex, NULL);
   // input_ctxt->state = QCARCAMTEST_STATE_INIT;
    input_ctxt->m_op_mode = p_xml_input->properties.op_mode;

    input_ctxt->m_batch_frames = p_xml_input->output_params.num_batch_frames;
    if (input_ctxt->m_batch_frames < 1)
    {
        input_ctxt->m_batch_frames = 1;
    }

    ret = set_batch_mode(m_batch_frames);
    if (ret)
    {
        CAM_MSG(ERROR, "Failed to set batch mode(ret %d)", ret);
        return ret;
    }

    input_ctxt->m_num_isp_instances = p_xml_input->isp_params.num_isp_instances;
    for (int i = 0; i < input_ctxt->m_num_isp_instances; i++)
    {
        input_ctxt->m_isp_config[i].id = p_xml_input->isp_params.isp_config[i].id;
        input_ctxt->m_isp_config[i].cameraId = p_xml_input->isp_params.isp_config[i].cameraId;
        input_ctxt->m_isp_config[i].usecaseId = p_xml_input->isp_params.isp_config[i].usecaseId;
        ret = set_isp_usecase(input_ctxt->m_isp_config[i]);
        if (ret)
        {
            CAM_MSG(ERROR, "Failed to set isp use case(ret %d)", ret);
            return ret;
        }
    }

    m_wrapper->getformat(m_v4l2fmt);

    //Set output buffer params
    input_ctxt->m_buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].n_buffers = p_xml_input->output_params.n_buffers;
    if (p_xml_input->output_params.width == -1 || p_xml_input->output_params.height == -1)
    {
        input_ctxt->m_buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].width = m_v4l2fmt._uiw;
        input_ctxt->m_buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].height = m_v4l2fmt._uih;
    }
    else
    {
        input_ctxt->m_buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].width = p_xml_input->output_params.width;
        input_ctxt->m_buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].height = p_xml_input->output_params.height * input_ctxt->m_batch_frames;
    }

    m_nbufs = p_xml_input->output_params.n_buffers;


    input_ctxt->m_buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].format =
        p_xml_input->output_params.format == -1 ? V4L2Wrapper::toQcarcamFormat(m_v4l2fmt._format) : p_xml_input->output_params.format;

    // //TODO: need check whether need it or not
    input_ctxt->m_buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].n_buffers = p_xml_input->window_params.n_buffers_display;
    input_ctxt->m_buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].width = p_xml_input->output_params.width;
    input_ctxt->m_buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].height = p_xml_input->output_params.height;
    input_ctxt->m_buffers_param[QCARCAM_TEST_BUFFERS_DISPLAY].format = QCARCAM_FMT_RGB_888;

    input_ctxt->m_window_params = p_xml_input->window_params;
    input_ctxt->m_window_params.buffer_size[0] = input_ctxt->m_buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].width;
    input_ctxt->m_window_params.buffer_size[1] = input_ctxt->m_buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].height;
    input_ctxt->m_window_params.visibility = 1;
    input_ctxt->m_window_params.is_imported_buffer = 1;
    //set window debug name to XML input index
    snprintf(input_ctxt->m_window_params.debug_name, sizeof(input_ctxt->m_window_params.debug_name), "qcarcam_%d", m_idx);


    input_ctxt->m_test_util_ctxt = test_util_ctxt;
    int output_n_buffers = 0;

    // Allocate an additional buffer to be shown in case of signal loss
    output_n_buffers = input_ctxt->m_buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].n_buffers;
    input_ctxt->m_p_buffers_output.nBuffers = output_n_buffers;
    input_ctxt->m_p_buffers_output.colorFmt = input_ctxt->m_buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].format;
    input_ctxt->m_p_buffers_output.pBuffers = (QCarCamBuffer_t *)calloc(input_ctxt->m_p_buffers_output.nBuffers, sizeof(*input_ctxt->m_p_buffers_output.pBuffers));

    if (input_ctxt->m_p_buffers_output.pBuffers == 0)
    {
        QCARCAM_ERRORMSG("alloc qcarcam_buffer failed");
        goto fail;
    }

    for (int i = 0; i < output_n_buffers; ++i)
    {
        input_ctxt->m_p_buffers_output.pBuffers[i].numPlanes = 1;
        input_ctxt->m_p_buffers_output.pBuffers[i].planes[0].width = input_ctxt->m_buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].width;
        input_ctxt->m_p_buffers_output.pBuffers[i].planes[0].height = input_ctxt->m_buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].height;
    }

    ret = test_util_init_window(test_util_ctxt, &input_ctxt->m_qcarcam_window);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("test_util_init_window failed for qcarcam_window");
        goto fail;
    }

    ret = test_util_set_window_param(test_util_ctxt, input_ctxt->m_qcarcam_window, &input_ctxt->m_window_params);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("test_util_set_window_param failed");
        goto fail;
    }

    CAM_MSG(HIGH,  "use_buf_width %d", use_buf_width);
    ret = m_wrapper->requestbuf(output_n_buffers, &input_ctxt->m_p_buffers_output, use_buf_width);
    if (ret)
    {
        CAM_MSG(ERROR,  "fail to reqbuf %d", ret);
        goto fail;
    }

    ret = test_util_init_window_buffers(test_util_ctxt, input_ctxt->m_qcarcam_window, &input_ctxt->m_p_buffers_output);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAM_ERRORMSG("test_util_init_window_buffers failed");
        goto fail;
    }

    input_ctxt->m_p_buffers_output.nBuffers = input_ctxt->m_buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].n_buffers;
    CAM_MSG(HIGH, "qcarcam_input_id %d", (int)m_qcarcam_input_id);

    return 0;

fail:
    return 1;

}

int InputStream::launch_stream_thread()
{
    InputStream* input_ctxt = this;
    int rc = 0;
    int fd = m_wrapper->getfd();
    char szThreadName[256] = {0};

    snprintf(szThreadName, 256, "client_inpt_ctxt_%d", input_ctxt->m_idx);

    subscribeEvents(fd, AIS_V4L2_CLIENT_CAPTURE, AIS_V4L2_PARAM_EVENT);
    subscribeEvents(fd, AIS_V4L2_CLIENT_CAPTURE, AIS_V4L2_EVENT_INPUT_SIGNAL);
    subscribeEvents(fd, AIS_V4L2_CLIENT_CAPTURE, AIS_V4L2_EVENT_ERROR);

    rc = CameraCreateThread(CAMERA_THREAD_PRIO_HIGH_REALTIME, 0,
        &InputStream::stream_thread,
        this, 0,
        szThreadName,
        &input_ctxt->m_thread_handle);
    if (rc)
    {
        QCARCAM_ERRORMSG("CameraCreateThread failed : %s", szThreadName );
        return 1;
    }

    return 0;
}

void InputStream::join_stream_thread()
{
    InputStream* input_ctxt = this;
    bool exit = 0;

    write(input_ctxt->m_pipefd[1], &exit, sizeof(exit));
    CameraJoinThread(input_ctxt->m_thread_handle, NULL);
}

void InputStream::uninit_window_buffer()
{
    InputStream* input_ctxt = this;

    test_util_deinit_window_buffer(input_ctxt->m_test_util_ctxt, input_ctxt->m_qcarcam_window);
    test_util_deinit_window(input_ctxt->m_test_util_ctxt, input_ctxt->m_qcarcam_window);
}

int InputStream::get_frame()
{
    InputStream* input_ctxt = this;
    V4L2Wrapper* wrapper = input_ctxt->m_wrapper;
    int rc = 0;
    struct ais_v4l2_buffer_ext_t ext = {};

    memset(&ext, 0, sizeof(ext));

    rc = wrapper->deqbuf(input_ctxt->m_buf_idx_qcarcam, &m_buf_desc, &ext);
    if (rc < 0)
    {
        CAM_MSG(ERROR, "fail to dequeue buffer");
        return 1;
    }
    if (g_xml_cfg.latency_measurement_mode > CAMERA_LM_MODE_DISABLE)
    {
        uint64 ptimestamp = 0;
        CameraGetTime(&ptimestamp);
        CAM_MSG(HIGH, "LATENCY| index %d Framecnt %d Latency_from_frame_done %llu us (%llu, %llu)",
            input_ctxt->m_idx,
            m_buf_desc._seq,
            (ptimestamp - (m_buf_desc._timestamp.tv_sec * 1000000000 + m_buf_desc._timestamp.tv_usec * 1000)) / 1000,
            ptimestamp,
            (m_buf_desc._timestamp.tv_sec * 1000000000 + m_buf_desc._timestamp.tv_usec * 1000));
    }
    QCARCAM_INFOMSG("get_frame %u size %u",
        input_ctxt->m_buf_idx_qcarcam, input_ctxt->m_p_buffers_output.pBuffers[input_ctxt->m_buf_idx_qcarcam].planes[0].size);

    QCARCAM_INFOMSG("index %u seq %u timestamp %u %u batch_num %d\n",
        input_ctxt->m_buf_idx_qcarcam, m_buf_desc._seq,
        m_buf_desc._timestamp.tv_sec, m_buf_desc._timestamp.tv_usec, ext.batch_num);

    for (uint32 i = 0; i < ext.batch_num; ++i)
    {
        QCARCAM_INFOMSG("index %u seq %u timestamp[%u]  %u",
            input_ctxt->m_buf_idx_qcarcam, m_buf_desc._seq, i,
            ext.timestamp[i]);
    }

    input_ctxt->m_buf_state[input_ctxt->m_buf_idx_qcarcam] = QCARCAMTEST_BUFFER_STATE_GET_FRAME;

    return 0;
}

int InputStream::release_buffers()
{
    InputStream* input_ctxt = this;
    int rc = 0;
    V4L2Wrapper* wrapper = input_ctxt->m_wrapper;
    while(!input_ctxt->m_release_buf_idx_list.empty())
    {
        uint32 rel_idx = input_ctxt->m_release_buf_idx_list.front();
        input_ctxt->m_release_buf_idx_list.pop_front();

        if (rel_idx < input_ctxt->m_buffers_param[QCARCAM_TEST_BUFFERS_OUTPUT].n_buffers)
        {
            if (QCARCAMTEST_BUFFER_STATE_GET_FRAME == (input_ctxt->m_buf_state[rel_idx] & 0xF) ||
                QCARCAMTEST_BUFFER_STATE_POST_DISPLAY == (input_ctxt->m_buf_state[rel_idx] & 0xF))
            {
                rc = wrapper->enqbuf(NULL, rel_idx);
                if (QCARCAM_RET_OK != rc)
                {
                    QCARCAM_ERRORMSG("enqbuf(%d) failed %d", rel_idx, rc);
                    return -1;
                }
                input_ctxt->m_releaseframeCnt++;
                input_ctxt->m_buf_state[rel_idx] = QCARCAMTEST_BUFFER_STATE_QCARCAM;
            }
            else
            {
                QCARCAM_ERRORMSG("enqbuf(%d) skipped since buffer bad state (%d)", rel_idx, input_ctxt->m_buf_state[rel_idx]);
            }
        }
        else
        {
            QCARCAM_ERRORMSG("enqbuf(%d) skipped", rel_idx);
        }
    }

    return rc;
}

int InputStream::post_display()
{
    test_util_post_window_buffer(m_test_util_ctxt,
        m_qcarcam_window,
        m_buf_idx_qcarcam,
        &m_release_buf_idx_list,
        QCARCAM_INTERLACE_FIELD_UNKNOWN);
    m_buf_state[m_buf_idx_qcarcam] = QCARCAMTEST_BUFFER_STATE_POST_DISPLAY;

    return 0;
}

int InputStream::processV4L2Event(int fd)
{
    struct v4l2_event v4l2Event = {};
    int rc = ioctl(fd, VIDIOC_DQEVENT, &v4l2Event);
    if (rc >= 0)
    {
        switch(v4l2Event.id)
        {
        case AIS_V4L2_PARAM_EVENT:
        {
            uint32 code = v4l2Event.u.data[0];
            uint32 size = v4l2Event.u.data[1];
            if (!check_vendor_extension(&v4l2Event.u.data[2]))
                QCARCAM_ERRORMSG("fail to check vendor extension");
            else
                QCARCAM_ALWZMSG("succeed to check vendor extension");


            break;
        }
        case AIS_V4L2_EVENT_INPUT_SIGNAL:
        {
            uint32 code = v4l2Event.u.data[0];
            uint32* value = (uint32*)&v4l2Event.u.data[2];
            QCARCAM_ALWZMSG("input signal %d", *value);
            handle_input_signal((QCarCamInputSignal_e)(*value));
            break;
        }
        case AIS_V4L2_EVENT_ERROR:
        {
            uint32 code = v4l2Event.u.data[0];
            uint32* value = (uint32*)&v4l2Event.u.data[2];
            QCARCAM_ALWZMSG("event error %d", *value);
            break;
        }
        case AIS_V4L2_EVENT_FRAME_DROP:
        {
            uint32 code = v4l2Event.u.data[0];
            uint32* value = (uint32*)&v4l2Event.u.data[2];
            QCARCAM_ALWZMSG("frameId%d dropped", *value);
            break;
        }
        default:
            break;
        }
    }

    return rc;
}



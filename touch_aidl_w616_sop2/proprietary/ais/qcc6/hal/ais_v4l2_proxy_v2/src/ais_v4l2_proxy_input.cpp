/* ===========================================================================
 * Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 =========================================================================== */


#include "ais_v4l2_proxy_input.h"
#include "ais_v4l2_proxy_stream.h"
#include "CameraOSServices.h"

#include <linux/videodev2.h>
#include <poll.h>
#include <media/ais_v4l2loopback.h>
#include <algorithm>

#define OPEN_FAIL_TIMER_DELAY 1000

using std::find;

extern ais_v4l2_cfg_t g_v4l2_cfg;

static pthread_mutex_t client_lock = PTHREAD_MUTEX_INITIALIZER;
static int client_count = 0;
static int Qcarcam_Initialized = 0;
static int openfail_count = 0;
static CameraTimer g_openfail_timer = NULL;

vector<CAisV4l2ProxyInput*> CAisV4l2ProxyInput::s_inputs;
int CAisV4l2ProxyInput::s_pipefd[2] = {};
queue<CAisV4l2ProxyInput::pipe_event_data_t> CAisV4l2ProxyInput::s_pipe_events;


QCarCamRet_e QcarcamInitialize()
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    QCarCamInit_t qcarcam_init = {};
    pthread_mutex_lock(&client_lock);
    if (client_count == 0)
    {
        if (!Qcarcam_Initialized)
        {
            // Initialize qcarcam only for first client
            qcarcam_init.apiVersion = QCARCAM_VERSION;
            //qcarcam_init.debug_tag = "ais_v4l2_proxy";
            ret = QCarCamInitialize(&qcarcam_init);
            if (ret != QCARCAM_RET_OK)
            {
                VPROXY_ERROR("qcarcam_initialize failed %d", ret);
                pthread_mutex_unlock(&client_lock);
                return QCARCAM_RET_FAILED;
            }
            Qcarcam_Initialized = 1;
        }
    }

    client_count++;
    pthread_mutex_unlock(&client_lock);
    return QCARCAM_RET_OK;
}

void QcarcamUninitialize()
{
    pthread_mutex_lock(&client_lock);
    client_count = (client_count > 0) ? (client_count - 1):0;
    if (client_count == 0)
    {
        if (Qcarcam_Initialized)
            QCarCamUninitialize();
        Qcarcam_Initialized = 0;
    }
    pthread_mutex_unlock(&client_lock);
}

static void timer_cb_qcarcamuninit(void *arg)
{
    while(openfail_count)
    {
        pthread_mutex_lock(&client_lock);
        openfail_count--;
        pthread_mutex_unlock(&client_lock);
        QcarcamUninitialize();
    }
}

CAisV4l2ProxyInput::CAisV4l2ProxyInput()
: m_qcarcam_input_id(0x7FFFFFFF)
, m_v4l2_params{}
, m_buffers_param{}
{
}

CAisV4l2ProxyInput::~CAisV4l2ProxyInput()
{
}

int CAisV4l2ProxyInput::init(ais_proxy_xml_input_t *p_xml_input, QCarCamInput_t* p_qcarcam_input, QCarCamMode_t* p_qcarcam_mode)
{
    int rc = 0;

    if (p_xml_input == NULL)
        return -1;

    m_qcarcam_input_id = p_xml_input->qcarcam_input_id;
    m_v4l2_params.opmode = p_xml_input->opmode;
    m_is_buf_algin = p_xml_input->is_buf_align;
    strlcpy(m_v4l2_params.v4l2_node, p_xml_input->v4l2_node, sizeof(m_v4l2_params.v4l2_node));

    // if config the pixformat, use it, else use the qureid color format
    if (p_xml_input->pixformat != 0)
    {
        m_v4l2_params.pixformat = p_xml_input->pixformat;
    }
    else
    {
        m_v4l2_params.pixformat = (p_qcarcam_input == NULL) ?
            QCARCAM_FMT_UYVY_8: (p_qcarcam_mode ? p_qcarcam_mode->sources[0].colorFmt : QCARCAM_FMT_UYVY_8);
    }

    if (p_xml_input->v4l2_format != 0)
    {
        m_v4l2_params.v4l2_format = p_xml_input->v4l2_format;
    }
    else
    {
        m_v4l2_params.v4l2_format = get_v4l2_format_from_qcarcam(
            (QCarCamColorFmt_e)m_v4l2_params.pixformat);
    }

    if (p_xml_input->width == -1)
    {
        m_v4l2_params.width = (p_qcarcam_input == NULL) ?
            1920 : (p_qcarcam_mode ? p_qcarcam_mode->sources[0].width : 1920);
    }
    else
    {
        m_v4l2_params.width = p_xml_input->width;
    }

    if (p_xml_input->height == -1)
    {
        m_v4l2_params.height = (p_qcarcam_input == NULL) ?
            1024: (p_qcarcam_mode ? p_qcarcam_mode->sources[0].height : 1024);
    }
    else
    {
        m_v4l2_params.height = p_xml_input->height;
    }

    m_num_isp_instances = p_xml_input->isp_params.num_isp_instances;
    for (int i = 0; i < m_num_isp_instances; i++)
    {
        m_isp_config[i].id = p_xml_input->isp_params.isp_config[i].id;
        m_isp_config[i].cameraId = p_xml_input->isp_params.isp_config[i].cameraId;
        m_isp_config[i].usecaseId = p_xml_input->isp_params.isp_config[i].usecaseId;
    }

    /* Initialize v4l2loopback driver */
    rc = qcarcam_init_v4l2device(&m_v4l2_params);
    if (rc)
    {
       VPROXY_ERROR("qcarcam_init_v4l2device failed for node %s", m_v4l2_params.v4l2_node);
       return -1;

    }
    else
    {
       VPROXY_INFO("qcarcam_init_v4l2device success for node %s", m_v4l2_params.v4l2_node);
       subscribeEvent(m_v4l2_params.v4l2sink, AIS_V4L2_CLIENT_OUTPUT, AIS_V4L2_OPEN_INPUT);

    }

    // todo: check the below
    m_buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].n_buffers = QCARCAM_MAX_RING_BUFFERS;
    m_buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].width = m_v4l2_params.width;
    m_buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].height = m_v4l2_params.height;
    m_buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].format =
        (QCarCamColorFmt_e)m_v4l2_params.pixformat;

    //create the timer
    if (!g_openfail_timer && CameraCreateTimer(0, 0, timer_cb_qcarcamuninit, this, &g_openfail_timer))
    {
        VPROXY_ERROR("CameraCreateTimer() failed for g_openfail_timer");
    }

    return 0;
}

void CAisV4l2ProxyInput::uninit()
{
    for (uint32 i = 0; i < m_streams.size(); ++i)
    {
        CAisV4l2ProxyStream::destroy_v4l2_stream(m_streams[i]);
        QcarcamUninitialize();
    }

    //destroy the timer
    if(g_openfail_timer)
    {
        CameraReleaseTimer(g_openfail_timer);
        g_openfail_timer = NULL;
    }

    m_streams.clear();

    close(m_v4l2_params.v4l2sink);
}

int CAisV4l2ProxyInput::processV4l2Event()
{
    CameraResult result = CAMERA_SUCCESS;
    struct v4l2_event v4l2Event = {};
    int rc = -1;
    int fd = m_v4l2_params.v4l2sink;

    VPROXY_INFO("[input id %d] process event", this->m_qcarcam_input_id);

    rc = ioctl(fd, VIDIOC_DQEVENT, &v4l2Event);
    if (rc >= 0)
    {
        switch(v4l2Event.id)
        {
        case AIS_V4L2_OPEN_INPUT:
            rc = processOpenEvent();
            break;

        default:
            VPROXY_ERROR("unsupported event");
            break;
        }
    }

    return rc;

}

int CAisV4l2ProxyInput::processOpenEvent()
{
    int fd = m_v4l2_params.v4l2sink;
    int rc = 0;

    rc = QcarcamInitialize();
    if (rc)
    {
        VPROXY_ERROR("qcarcam_initialize() failed");
        send_ioctrl_ret(fd, AIS_V4L2_OUTPUT_PRIV_OPEN_RET, CTL_FAIL_RET);
        return -1;
    }

    QCarCamHndl_t qcarcam_context = 0;
    QCarCamOpen_t openParams = {};

    openParams.opMode = m_v4l2_params.opmode;
    openParams.numInputs = 1;
    openParams.inputs[0].inputId = m_qcarcam_input_id;
    rc = QCarCamOpen(&openParams ,&qcarcam_context);

    if (rc || qcarcam_context == 0)
    {
        VPROXY_ERROR("qcarcam_open() failed");
        send_ioctrl_ret(fd, AIS_V4L2_OUTPUT_PRIV_OPEN_RET, CTL_FAIL_RET);

        //start the timer
        if (CameraUpdateTimer(g_openfail_timer, OPEN_FAIL_TIMER_DELAY))
        {
            VPROXY_ERROR("CameraUpdateTimer() failed for g_openfail_timer");
        }
        pthread_mutex_lock(&client_lock);
        openfail_count++;
        pthread_mutex_unlock(&client_lock);
        return -1;
    }

    int stream_fd = open(m_v4l2_params.v4l2_node, O_WRONLY|O_NONBLOCK);
    if (stream_fd < 0) {
        VPROXY_ERROR("Failed to open v4l2sink device. (%s)", strerror(errno));
        send_ioctrl_ret(fd, AIS_V4L2_OUTPUT_PRIV_OPEN_RET, CTL_FAIL_RET);
        QcarcamUninitialize();
        return -1;
    }

    // create v4l2 stream
    CAisV4l2ProxyStream* pStream = CAisV4l2ProxyStream::create_v4l2_stream(qcarcam_context, stream_fd, this);
    if (pStream)
    {
        send_ioctrl_ret(stream_fd, AIS_V4L2_OUTPUT_PRIV_OPEN_RET, CTL_SUCCEED_RET);
        m_streams.push_back(pStream);
    }
    else
    { // todo: how to handle when fail to create_v4l2_stream
        close(stream_fd);
        VPROXY_ERROR("Failed to create v4l2 stream.");
        send_ioctrl_ret(fd, AIS_V4L2_OUTPUT_PRIV_OPEN_RET, CTL_FAIL_RET);
        QcarcamUninitialize();
        return -1;
    }

    return rc;

}

int CAisV4l2ProxyInput::processCloseEvent(CAisV4l2ProxyStream* pStream)
{
    if (pStream)
    {
        CAisV4l2ProxyStream::destroy_v4l2_stream(pStream);

        QcarcamUninitialize();

        // remove from m_streams
        vector<CAisV4l2ProxyStream*>::iterator it = find(m_streams.begin(), m_streams.end(), pStream);
        if (it!= m_streams.end())
        {
            m_streams.erase(it);
        }

    }

    return 0;
}

int CAisV4l2ProxyInput::poll_wait()
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    int rc = -1;
    int ret_bc = 0;
    short revents = 0;
    struct pollfd pollFds[NUM_MAX_CAMERAS+1];
    int pollNumFds = s_inputs.size() + 1;
    int pipe_data;

    rc = pipe(s_pipefd);
    if (rc < 0)
    {
        VPROXY_ERROR("Failed to create pipe %d");
        return -1;
    }

    VPROXY_INFO("poll_wait %d", pollNumFds);

    pollFds[0].fd     = s_pipefd[0];
    pollFds[0].events = POLLIN|POLLRDNORM;

    for (uint32 i = 1; i < pollNumFds; ++i)
    {
        pollFds[i].fd = s_inputs[i-1]->m_v4l2_params.v4l2sink;
        pollFds[i].events = POLLIN|POLLRDNORM|POLLPRI;
    }
    VPROXY_INFO("before poll");

    while (!g_v4l2_cfg.aborted)
    {

        ret_bc = poll(pollFds, pollNumFds, -1);
        //VPROXY_INFO("poll_wait %d", pollNumFds);
        if (ret_bc < 0)
        {
            VPROXY_ERROR("polling is fail ");
            continue;
        }
        else if (pollFds[0].revents & POLLIN)
        {
            int nRead = 0;
            nRead = read(pollFds[0].fd, &pipe_data, sizeof(pipe_data));
            if (nRead == sizeof(pipe_data))
            {
                VPROXY_INFO("pipe data is %d", pipe_data);
                if (pipe_data == -1)
                { // exit the poll_wait
                    break;
                }
                else
                {
                    if (processPipeEvent((PIPE_EVENT)pipe_data))
                    {
                        VPROXY_ERROR("process pipe event failed");
                        // todo : how to handle the pipe event failure
                        //break;
                    }
                }
            }
        }
        else
        {
            for( uint32 i = 1; i < pollNumFds; ++i)
            {
                revents = pollFds[i].revents;
                VPROXY_DBG1("polling revents value : %u", revents);
                if (revents & POLLPRI)
                {
                    s_inputs[i-1]->processV4l2Event();
                }
                else if (revents & POLLERR)
                { // todo: if pollerr, how to handle?
                    /*
                    Error condition (only returned in revents; ignored in events).
                    This bit is also set for a file descriptor referring to the
                    write end of a pipe when the read end has been closed.
                    */
                    VPROXY_ERROR("poll error");
                    //break;
                }
            }
        }
    }

    close(s_pipefd[0]);
    close(s_pipefd[1]);

    VPROXY_INFO("exit poll_wait");
    return 0;

}

void CAisV4l2ProxyInput::add_input(CAisV4l2ProxyInput* pInput)
{
    s_inputs.push_back(pInput);
}

int CAisV4l2ProxyInput::processPipeEvent(PIPE_EVENT event)
{
    if (!s_pipe_events.empty())
    {
        pipe_event_data_t data = s_pipe_events.front();
        s_pipe_events.pop();
        switch(event)
        {
            case PIPE_EVENT_CLOSE_STREAM:
                data.pInput->processCloseEvent(data.pStream);
                break;
            default:
                break;
        }
    }

    return 0;
}

int CAisV4l2ProxyInput::sendPipeEvent(PIPE_EVENT event, const pipe_event_data_t& data)
{
    int tmp = event;
    s_pipe_events.push(data);
    write(s_pipefd[1], &tmp, sizeof(tmp));

    return 0;
}

void CAisV4l2ProxyInput::exit_v4l2_poll()
{
    int tmp = 0;
    write(s_pipefd[1], &tmp, sizeof(tmp));
}



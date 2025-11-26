/* ===========================================================================
 * Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
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
#include <algorithm>
#include "ais_v4l2_proxy_input.h"
#include "ais_v4l2_proxy_stream.h"


using std::find;

extern ais_v4l2_cfg_t g_v4l2_cfg;
extern ais_proxy_gconfig ais_v4l2_xml_cfg;

vector<CAisV4l2ProxyStream*> CAisV4l2ProxyStream::s_all_streams;

CAisV4l2ProxyStream* CAisV4l2ProxyStream::create_v4l2_stream(qcarcam_hndl_t qcarcam_context, int fd,
            CAisV4l2ProxyInput* pInput)
{
     CAisV4l2ProxyStream* pStream = new CAisV4l2ProxyStream(qcarcam_context, fd);
     if (pStream->init(pInput))
     {
        delete pStream;
        return NULL;
     }

     s_all_streams.push_back(pStream);

     return pStream;
}

void CAisV4l2ProxyStream::destroy_v4l2_stream(CAisV4l2ProxyStream* p)
{
    if (p)
    {
        p->uninit();

        // remove from s_all_streams
        vector<CAisV4l2ProxyStream*>::iterator it = find(s_all_streams.begin(), s_all_streams.end(), p);
        if (it!= s_all_streams.end())
        {
            s_all_streams.erase(it);
        }

        delete p;
    }
}

CAisV4l2ProxyStream::CAisV4l2ProxyStream(qcarcam_hndl_t qcarcam_context, int fd)
    : m_thread_handle(NULL)
    , m_pipefd{}
    , m_qcarcam_context(qcarcam_context)
    , m_qcarcam_input_id(QCARCAM_INPUT_MAX)
    , m_stream_fd(fd)
    , m_v4l2_params{}
    , m_buffers_param{}
    , m_p_buffers_output{}
    , m_qcarcam_buffers{}
    , m_frame_buf_hndl(NULL)
    , m_is_buf_align(true)
    , m_buf_idx_qcarcam(0)
    , m_prev_buf_idx_qcarcam(0)
    , m_frame_info{}
    , m_batch_frames(1)
    , m_frameCnt(0)
    , m_prev_frame(0)
    , m_is_running(0)
    , m_is_first_count(0)
    , m_bufstate_mutex(NULL)
    , m_is_buf_dequeued{}
    , m_pParentInput(NULL)
{

}

CAisV4l2ProxyStream::~CAisV4l2ProxyStream()
{
}

int CAisV4l2ProxyStream::init(CAisV4l2ProxyInput* pInput)
{
    int rc = 0;
    qcarcam_param_value_t param;

    if (pInput == NULL)
        return -1;

    m_pParentInput = pInput;
    m_v4l2_params = pInput->m_v4l2_params;
    m_v4l2_params.v4l2sink = m_stream_fd;
    m_buffers_param[0] = pInput->m_buffers_param[0];
    m_buffers_param[1] = pInput->m_buffers_param[1];
    m_qcarcam_input_id = pInput->m_qcarcam_input_id;
    m_is_buf_align = pInput->m_is_buf_algin;

    rc = CameraCreateMutex(&m_bufstate_mutex);
    if (rc)
    {
        VPROXY_ERROR("CameraCreateMutex failed for node %s", m_v4l2_params.v4l2_node);
        return rc;
    }

    VPROXY_INFO("stream init, input_desc=%d context=%p\n",
        m_qcarcam_input_id, m_qcarcam_context);

#ifndef __AGL__
    // For HDMI/CVBS Input
    // NOTE: set HDMI IN resolution in qcarcam_config_single_hdmi.xml before
    // running the server
    if (m_qcarcam_input_id == QCARCAM_INPUT_TYPE_DIGITAL_MEDIA) {
        //qcarcam_param_value_t param;
        param.res_value.width = m_buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].width;
        param.res_value.height = m_buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].height;

        rc = qcarcam_s_param(m_qcarcam_context, QCARCAM_PARAM_RESOLUTION, &param);
        if (rc != QCARCAM_RET_OK)
        {
            VPROXY_ERROR("qcarcam_s_param resolution() failed");
            return -1;
        }
    }
#endif

    param.ptr_value = (void *)qcarcam_event_cb;
    rc = qcarcam_s_param(m_qcarcam_context, QCARCAM_PARAM_EVENT_CB, &param);
    if (rc)
    {
        VPROXY_ERROR("ERROR qcarcam_s_param failed for cb");
        return -1;
    }

    param.uint_value = QCARCAM_EVENT_FRAME_READY | QCARCAM_EVENT_INPUT_SIGNAL |
        QCARCAM_EVENT_ERROR | QCARCAM_EVENT_VENDOR | QCARCAM_EVENT_FRAME_DROP;
    rc = qcarcam_s_param(m_qcarcam_context, QCARCAM_PARAM_EVENT_MASK, &param);
    if (rc)
    {
        VPROXY_ERROR("ERROR qcarcam_s_param failed for cb mask");
        return -1;
    }

    param.uint_value = (unsigned int) m_v4l2_params.opmode;
    rc = qcarcam_s_param(m_qcarcam_context, QCARCAM_PARAM_OPMODE, &param);
    if (rc != QCARCAM_RET_OK)
    {
        VPROXY_ERROR("qcarcam_s_param(QCARCAM_PARAM_OPMODE) failed");
        return -1;
    }
    VPROXY_DBG1("setting opmode to %u - result = %d", param.uint_value, rc);

    for (uint32 idx = 0; idx < pInput->m_num_isp_instances; idx++)
    {
        qcarcam_param_value_t param;
        param.isp_config = pInput->m_isp_config[idx];
        rc = qcarcam_s_param(m_qcarcam_context, QCARCAM_PARAM_ISP_USECASE, &param);
        if (rc != QCARCAM_RET_OK)
        {
            VPROXY_ERROR("qcarcam_s_param(QCARCAM_PARAM_ISP_USECASE) failed");
            return -1;
        }

        VPROXY_DBG1("setting ISP usecase instance %u isp_camera_id %u use_case %d - result = %d",
            param.isp_config.id, param.isp_config.camera_id, param.isp_config.use_case, rc);
    }

    rc = afterInit();

    return rc;
}

int CAisV4l2ProxyStream::afterInit()
{
    int rc = 0;
    rc = init_gfx_buffers(&m_frame_buf_hndl,
            m_v4l2_params.width,
            m_v4l2_params.height,
            m_v4l2_params.pixformat, m_is_buf_align);
    if (rc)
    {
        VPROXY_ERROR("init_gfx_buffers failed for node %s\n", m_v4l2_params.v4l2_node);
        return rc;
    }
    else
    {
        set_v4l2_format(&m_v4l2_params, m_frame_buf_hndl);
    }

    //subscribeEvent(fd, AIS_V4L2_CLIENT_OUTPUT, AIS_V4L2_OPEN_INPUT);
    subscribeEvent(m_stream_fd, AIS_V4L2_CLIENT_OUTPUT, AIS_V4L2_CLOSE_INPUT);
    subscribeEvent(m_stream_fd, AIS_V4L2_CLIENT_OUTPUT, AIS_V4L2_START_INPUT);
    subscribeEvent(m_stream_fd, AIS_V4L2_CLIENT_OUTPUT, AIS_V4L2_STOP_INPUT);
    subscribeEvent(m_stream_fd, AIS_V4L2_CLIENT_OUTPUT, AIS_V4L2_SET_PARAM);
    subscribeEvent(m_stream_fd, AIS_V4L2_CLIENT_OUTPUT, AIS_V4L2_GET_PARAM);
    subscribeEvent(m_stream_fd, AIS_V4L2_CLIENT_OUTPUT, AIS_V4L2_OUTPUT_BUF_READY);
    subscribeEvent(m_stream_fd, AIS_V4L2_CLIENT_OUTPUT, AIS_V4L2_ALLOC_BUFS);

    // create thread
    char thread_name[256] = {};
    snprintf(thread_name, sizeof(thread_name), "proxy_inpt_ctxt_%d_", m_qcarcam_input_id);

    rc = CameraCreateThread(CAMERA_THREAD_PRIO_HIGH_REALTIME, 0,
        &poll_thread,
        this,
        0,
        thread_name,
        &m_thread_handle);
    if (rc)
    { // todo , when fail, how to handle
        VPROXY_ERROR("pthread_create failed");
    }

    return rc;
}

void CAisV4l2ProxyStream::uninit()
{
    int* thread_ret = NULL;

    VPROXY_INFO("join thread");

    // join thread
    CameraJoinThread(m_thread_handle, thread_ret);
    m_thread_handle = NULL;

    if (m_stream_fd >= 0)
        close(m_stream_fd);

    if (m_frame_buf_hndl != NULL) {
        free_gfx_buffers(m_frame_buf_hndl);
        deinit_gfx_buffers(m_frame_buf_hndl);
        m_frame_buf_hndl = NULL;
    }
    m_qcarcam_context = NULL;
    m_batch_frames = 1;
    m_stream_fd = -1;

    if (m_bufstate_mutex != NULL) {
        CameraDestroyMutex(m_bufstate_mutex);
        m_bufstate_mutex = NULL;
    }

}

int CAisV4l2ProxyStream::processV4l2Event()
{
    CameraResult result = CAMERA_SUCCESS;
    struct v4l2_event v4l2Event = {};
    int rc = -1;

    rc = ioctl(m_stream_fd, VIDIOC_DQEVENT, &v4l2Event);

    VPROXY_DBG1("m_stream_fd %d, [v4l2Event.id %d] succeeded rc %d",
        m_stream_fd, v4l2Event.id, rc);
    if (rc >= 0)
    {
        switch(v4l2Event.id)
        {
        case AIS_V4L2_CLOSE_INPUT:
            rc = processCloseEvent();
            break;
        case AIS_V4L2_START_INPUT:
            rc = processStartEvent();
            break;
        case AIS_V4L2_STOP_INPUT:
            rc = processStopEvent();
            break;

        case AIS_V4L2_GET_PARAM:
            rc = processGetParamEvent(v4l2Event);
            break;
        case AIS_V4L2_SET_PARAM:
            rc = processSetParamEvent(v4l2Event);
            break;
        case AIS_V4L2_ALLOC_BUFS:
            rc = processAllocBufs(v4l2Event);
            break;
        case AIS_V4L2_OUTPUT_BUF_READY:
            rc = processOutputBufReady();
            break;
        }
    }

    return rc;

}

int CAisV4l2ProxyStream::processCloseEvent()
{
    int rc = 0;
    int tmp = 0;

    // to exit the poll thread in time
    write(m_pipefd[1], &tmp, sizeof(tmp));

    rc = qcarcam_close(m_qcarcam_context);
    if (rc != QCARCAM_RET_OK)
    {
        VPROXY_ERROR("[input %d] qcarcam_close() failed", m_qcarcam_input_id);
        send_ioctrl_ret(m_stream_fd, AIS_V4L2_OUTPUT_PRIV_CLOSE_RET, CTL_FAIL_RET);
    }
    else
    {
        VPROXY_INFO("[input %d] qcarcam_close() succeeded", m_qcarcam_input_id);
        send_ioctrl_ret(m_stream_fd, AIS_V4L2_OUTPUT_PRIV_CLOSE_RET, CTL_SUCCEED_RET);

        close(m_stream_fd);

        if (m_frame_buf_hndl != NULL) {
            free_gfx_buffers(m_frame_buf_hndl);
            deinit_gfx_buffers(m_frame_buf_hndl);
            m_frame_buf_hndl = NULL;
        }
        m_qcarcam_context = NULL;
        m_batch_frames = 1;
        m_stream_fd = -1;

        CAisV4l2ProxyInput::pipe_event_data_t data = {m_pParentInput, this};
        CAisV4l2ProxyInput::sendPipeEvent(CAisV4l2ProxyInput::PIPE_EVENT_CLOSE_STREAM, data);
    }

    return rc;
}

int CAisV4l2ProxyStream::processStartEvent()
{
    int rc = 0;
    int buf_fd = 0;

    /*single threaded handles frames outside this function*/
    VPROXY_INFO("[input %d] start input() begin", m_qcarcam_input_id);

    m_is_running = 1;
    rc = qcarcam_start(m_qcarcam_context);
    if (rc != QCARCAM_RET_OK)
    {
        VPROXY_ERROR("[input %d] qcarcam_start() failed", m_qcarcam_input_id);
        send_ioctrl_ret(m_stream_fd, AIS_V4L2_OUTPUT_PRIV_START_RET, CTL_FAIL_RET);
    }
    else
    {
        VPROXY_INFO("[input %d] start input() succeeded", m_qcarcam_input_id);
        send_ioctrl_ret(m_stream_fd, AIS_V4L2_OUTPUT_PRIV_START_RET, CTL_SUCCEED_RET);

    }

    return rc;

}

int CAisV4l2ProxyStream::processStopEvent()
{
    int rc = 0;

    m_is_running = 0;
    rc = qcarcam_stop(m_qcarcam_context);
    if (rc != QCARCAM_RET_OK)
    {
        VPROXY_ERROR("[input %d] qcarcam_stop() %p failed", m_qcarcam_input_id, m_qcarcam_context);
        send_ioctrl_ret(m_stream_fd, AIS_V4L2_OUTPUT_PRIV_STOP_RET, CTL_FAIL_RET);
    }
    else
    {
        VPROXY_INFO("[input %d] qcarcam_stop() %p succeed", m_qcarcam_input_id, m_qcarcam_context);
        send_ioctrl_ret(m_stream_fd, AIS_V4L2_OUTPUT_PRIV_STOP_RET, CTL_SUCCEED_RET);
        memset(m_is_buf_dequeued, 0x0, sizeof(m_is_buf_dequeued));
    }

    return rc;

}

int CAisV4l2ProxyStream::processAllocBufs(struct v4l2_event & v4l2Event)
{
    uint8 nbufs = v4l2Event.u.data[0];
    uint8 use_buf_width = v4l2Event.u.data[1];
    int ret = 0;

    if (nbufs > 0)
    {

        if (use_buf_width == true)
        {
            m_is_buf_align = false;
        }

        if (m_is_buf_align == false)
        {
            if (!set_buf_align(m_frame_buf_hndl, false))
            {
                set_v4l2_format(&m_v4l2_params, m_frame_buf_hndl);
            }
        }
        VPROXY_INFO("use_buf_width = %u, m_is_buf_align = %u", use_buf_width, m_is_buf_align);
        memset(m_is_buf_dequeued, 0x0, sizeof(m_is_buf_dequeued));
        m_prev_buf_idx_qcarcam = -1;

        m_p_buffers_output.color_fmt = m_buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].format;

        m_p_buffers_output.buffers = m_qcarcam_buffers;
        m_p_buffers_output.n_buffers = nbufs;

        for (int i = 0; i < nbufs; ++i)
        {
            m_p_buffers_output.buffers[i].n_planes = 1;
            m_p_buffers_output.buffers[i].planes[0].width = m_buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].width;
            m_p_buffers_output.buffers[i].planes[0].height =
                m_buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].height * m_batch_frames;
        }

        ret = alloc_gfx_buffers(m_frame_buf_hndl, &m_p_buffers_output);
        if (ret != QCARCAM_RET_OK)
        {
            VPROXY_ERROR("qcarcam_init_gfx_buffers failed for direct_frame_buffer");
            ret = send_ioctrl_bufs(m_stream_fd,
                                AIS_V4L2_OUTPUT_PRIV_SET_BUFS, CTL_FAIL_RET,
                                0,  NULL);
            if (ret)
            {
                VPROXY_ERROR("AIS_V4L2_OUTPUT_PRIV_SET_BUFS failed");
            }

        }
        else
        {
            //send result to kernel
            struct ais_v4l2_buffers_t bufs;
            bufs.nbufs = nbufs;
            for(uint32 i = 0 ; i < nbufs; ++i)
            {
                bufs.fds[i] = (int)(uintptr_t)m_p_buffers_output.buffers[i].planes[0].p_buf;
            }

            ret = send_ioctrl_bufs(m_stream_fd,
                                AIS_V4L2_OUTPUT_PRIV_SET_BUFS, CTL_SUCCEED_RET,
                                sizeof(bufs), (__u8*)&bufs);
            if (ret)
            {
                VPROXY_ERROR("AIS_V4L2_OUTPUT_PRIV_SET_BUFS failed %u", sizeof(bufs));
            }

            ret = qcarcam_s_buffers(m_qcarcam_context, &m_p_buffers_output);
            if (ret != QCARCAM_RET_OK)
            {
                VPROXY_ERROR("qcarcam_s_buffers() failed");

            }
        }
    }
    else
    { //when allocbuf num is 0, free the buffers
        if (m_frame_buf_hndl != NULL) {
            free_gfx_buffers(m_frame_buf_hndl);
            deinit_gfx_buffers(m_frame_buf_hndl);
            m_frame_buf_hndl = NULL;
        }
        m_p_buffers_output.buffers = NULL;
    }

    return ret;

}

int CAisV4l2ProxyStream::processGetParamEvent(struct v4l2_event & v4l2Event)
{
    uint8 param_type = v4l2Event.u.data[0];
    qcarcam_param_value_t param = {};
    uint32 size = v4l2Event.u.data[1];
    int rc = 0;

    if (size > 0 && size <= MAX_AIS_V4L2_PARAM_EVNET_SIZE)
    {
        memcpy(&param.vendor_param, &v4l2Event.u.data[2], size);
    }
    else if (size > MAX_AIS_V4L2_PARAM_EVNET_SIZE)
    {
        rc = send_ioctrl_cmd(m_stream_fd,
                            AIS_V4L2_OUTPUT_PRIV_GET_PARAM, param_type,
                            0, (__u8*)&param);
        VPROXY_DBG1("AIS_V4L2_GET_PARAM %u %u %u", param_type,
            param.vendor_param.data[0], param.vendor_param.data[1]);
        if (rc)
        {
            VPROXY_ERROR("AIS_V4L2_SET_PARAM failed");
            return rc;
        }
    }

    //TODO: map param_type from V4L2 to qcarcam_types.h
    rc = qcarcam_g_param(m_qcarcam_context, (qcarcam_param_t)param_type,
        &param);
    if (rc != QCARCAM_RET_OK)
    {
        VPROXY_ERROR("g_param() failed %u", param_type);
        size = 0;
    }
    VPROXY_DBG1("AIS_V4L2_GET_PARAM %u %u %u", param_type,
        param.vendor_param.data[0], param.vendor_param.data[1]);

    rc = send_ioctrl_cmd(m_stream_fd,
                        AIS_V4L2_OUTPUT_PRIV_SET_PARAM2, param_type,
                        size, (__u8*)&param);
    if (rc < 0)
    {
        VPROXY_ERROR("g_param() step2 failed");
    }

    return rc;

}

int CAisV4l2ProxyStream::processSetParamEvent(struct v4l2_event & v4l2Event)
{
    uint8 param_type = v4l2Event.u.data[0];
    qcarcam_param_value_t param = {};
    int rc = 0;
    int ret = CTL_SUCCEED_RET;
    rc = send_ioctrl_cmd(m_stream_fd,
                        AIS_V4L2_OUTPUT_PRIV_GET_PARAM, param_type,
                        0, (__u8*)&param);

    VPROXY_INFO("AIS_V4L2_SET_PARAM %u %u %u", param_type,
        param.vendor_param.data[0], param.vendor_param.data[1]);
    if (rc)
    {
        VPROXY_ERROR("AIS_V4L2_SET_PARAM failed");
    }
    else
    {
        //TODO: map param_type from V4L2 to qcarcam_types.h
        rc = qcarcam_s_param(m_qcarcam_context, (qcarcam_param_t)param_type,
                &param);
        if (rc != QCARCAM_RET_OK)
        {
            VPROXY_ERROR("s_param() failed");
            ret = CTL_FAIL_RET;
        }
#ifdef __AGL__
        if (param_type == QCARCAM_PARAM_BATCH_MODE)
        {
            qcarcam_batch_mode_config_t* batch_cfg = (qcarcam_batch_mode_config_t*)&param;
            m_batch_frames = batch_cfg->num_batch_frames;
            VPROXY_INFO("s_batch_mode %d", batch_cfg->num_batch_frames);
            if (m_batch_frames > 1)
            {
                set_gfx_buf_size(m_frame_buf_hndl, m_buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].width,
                    m_buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].height * m_batch_frames);
                set_v4l2_format(&m_v4l2_params, m_frame_buf_hndl);
            }
        }
#endif

        rc = send_ioctrl_ret(m_stream_fd,
                        AIS_V4L2_OUTPUT_PRIV_SET_PARAM_RET, ret);
        if (rc)
        {
            VPROXY_ERROR("AIS_V4L2_SET_PARAM_RET failed");
        }
    }

    return rc;
}

int CAisV4l2ProxyStream::processOutputBufReady()
{
    int rc = 0;
    int idx = 0;

    if (!dqbuf(m_stream_fd, idx))
    {
        // don't need the lock here, it is reading while event_cb thread writes
        if (m_is_buf_dequeued[idx])
        {
            CameraLockMutex(m_bufstate_mutex);
            m_is_buf_dequeued[idx] = 0;
            CameraUnlockMutex(m_bufstate_mutex);
            VPROXY_DBG1("[input %d] release frame %d begin", m_qcarcam_input_id, idx);
            rc = qcarcam_release_frame(m_qcarcam_context, idx);
            if (rc)
            {
                VPROXY_WARN("[input %d] fail to release %u", m_qcarcam_input_id, idx);
                CameraLockMutex(m_bufstate_mutex);
                m_is_buf_dequeued[idx] = 1;
                CameraUnlockMutex(m_bufstate_mutex);
                return -1;
            }

            VPROXY_DBG1("[input %d] release frame %d", m_qcarcam_input_id, idx);
        }
        else
        {

            VPROXY_WARN("[input %d] idx %d has queue to server", m_qcarcam_input_id, idx);

        }
    }

    return 0;
}

int CAisV4l2ProxyStream::poll_thread(void *arg)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    int rc = -1;
    int ret_bc, fd;
    short revents;
    struct pollfd pollFds[2];
    int pollNumFds;

    CAisV4l2ProxyStream *stream_ctxt = (CAisV4l2ProxyStream *)arg;

    if (!stream_ctxt)
        return -1;

    rc = pipe(stream_ctxt->m_pipefd);
    if (rc < 0)
    {
        CAM_MSG(ERROR, "Failed to create pipe %d");
        return -1;
    }

    fd = stream_ctxt->m_stream_fd;
    if (fd < 0)
    {
        VPROXY_ERROR("open fd error");
        return -1;
    }
    else
    {
        VPROXY_INFO("fd %d open is successful", fd);
    }

    pollFds[0].fd     = stream_ctxt->m_pipefd[0];
    pollFds[0].events = POLLIN|POLLRDNORM;
    pollFds[1].fd = fd;
    pollFds[1].events = POLLIN|POLLRDNORM|POLLPRI;
    pollNumFds        = 2;

    while (!g_v4l2_cfg.aborted)
    {
        ret_bc = poll(pollFds, pollNumFds, -1);
        if (ret_bc < 0)
        {
            VPROXY_ERROR("polling is fail ");
            continue;
        }
        else if (pollFds[0].revents & POLLIN)
        {
            VPROXY_INFO("exit loop when recive pipe data");
            break;
        }

        revents = pollFds[1].revents;
        VPROXY_DBG1("fd %d, polling revents value : %u", fd, revents);
        if (revents & POLLPRI)
        {
            stream_ctxt->processV4l2Event();

            VPROXY_DBG1("end processV4l2Event");
        }
        else if (revents & POLLERR)
        {
            /*
            Error condition (only returned in revents; ignored in events).
            This bit is also set for a file descriptor referring to the
            write end of a pipe when the read end has been closed.
            */
            VPROXY_ERROR("poll error");
            break;
        }
    }

    if (stream_ctxt->m_qcarcam_context)
    {
        qcarcam_close(stream_ctxt->m_qcarcam_context);
        stream_ctxt->m_qcarcam_context = NULL;
    }

    close(stream_ctxt->m_pipefd[0]);
    close(stream_ctxt->m_pipefd[1]);

    VPROXY_INFO("exit setup_input_ctxt_thread idx = %p", stream_ctxt);
    return 0;

}

/**
 * Function to retrieve frame from qcarcam and increase frame_counter
 * @param ais_proxy_ctxt
 * @return 0 on success, -1 on failure
 */
int CAisV4l2ProxyStream::qcarcam_get_new_frame()
{
    qcarcam_ret_t ret;
    qcarcam_frame_info_v2_t frame_info;

    if (m_batch_frames > 1)
    {
        ret = qcarcam_get_frame_v2(m_qcarcam_context, &frame_info, DEFAULT_FRAME_TIMEOUT, 0);
        if (ret == QCARCAM_RET_TIMEOUT)
        {
            VPROXY_ERROR("qcarcam_get_frame timeout context 0x%p ret %d\n", m_qcarcam_context, ret);
            //qcarcam_inputs_signal_lost[m_idx] = 1;
            return -1;
        }
    }
    else
    {
        qcarcam_frame_info_t frame_info_v1;
        ret = qcarcam_get_frame(m_qcarcam_context, &frame_info_v1, DEFAULT_FRAME_TIMEOUT, 0);
        if (ret == QCARCAM_RET_TIMEOUT)
        {
            VPROXY_ERROR("qcarcam_get_frame timeout context 0x%p ret %d\n", m_qcarcam_context, ret);
            //qcarcam_inputs_signal_lost[m_idx] = 1;
            return -1;
        }
        memset(&frame_info, 0, sizeof(frame_info));

        frame_info.idx = frame_info_v1.idx;
        frame_info.flags = frame_info_v1.flags;
        frame_info.field_type = frame_info_v1.field_type;
        frame_info.seq_no[0] = frame_info_v1.seq_no;
        frame_info.sof_qtimestamp[0] = frame_info_v1.sof_qtimestamp;
        frame_info.timestamp = frame_info_v1.timestamp;
        frame_info.timestamp_system = frame_info_v1.timestamp_system;
    }

    if (QCARCAM_RET_OK != ret)
    {
        VPROXY_ERROR("Get frame context 0x%p ret %d\n", m_qcarcam_context, ret);
        return -1;
    }

    if (frame_info.idx >= m_buffers_param[AIS_BUFFERS_DIRECT_OUTPUT].n_buffers)
    {
        VPROXY_ERROR("Get frame context 0x%p ret invalid idx %d\n", m_qcarcam_context, frame_info.idx);
        return -1;
    }

    if (m_frameCnt == 0)
    {
        unsigned long t_to;
        // todo: need refine here for the get first frame
        //qcarcam_get_system_time(&t_to);
        printf("[input %d] Succeed to get first frame\n", m_qcarcam_input_id);
        fflush(stdout);
        //VPROXY_ALWAYS("[input %d] Get First Frame buf_idx %i after : %lu ms, field type: %d", (int)m_qcarcam_input_id, m_buf_idx_qcarcam, (t_to - t_start), frame_info.field_type);
    }

    m_prev_buf_idx_qcarcam = m_buf_idx_qcarcam;
    m_buf_idx_qcarcam = frame_info.idx;
    m_frame_info = frame_info;

    CameraLockMutex(m_bufstate_mutex);
    if (m_is_buf_dequeued[frame_info.idx])
    {
        VPROXY_ERROR("[input %d] has get frame %d", m_qcarcam_input_id, frame_info.idx);
    }
    m_is_buf_dequeued[frame_info.idx] = 1;
    CameraUnlockMutex(m_bufstate_mutex);

    VPROXY_DBG1("[input %d] get frame %d", m_qcarcam_input_id, frame_info.idx);

    m_frameCnt++;

    return 0;
}

/**
 * Function to post new frame to v4l2node. May also do color conversion and frame dumps.
 * @param ais_proxy_ctxt
 * @return 0 on success, -1 on failure
 */
int CAisV4l2ProxyStream::qcarcam_post_frame_to_v4l2node()
{
    int ret;

    /**********************
     * Dump raw if necessary
     ********************** */
    if (0 != g_v4l2_cfg.dumpFrame)
    {
        if (0 == m_frameCnt % g_v4l2_cfg.dumpFrame)
        {
            char filename[256] = {};
            snprintf(filename, sizeof(filename), DEFAULT_DUMP_LOCATION "frame_%d_%i.raw", m_qcarcam_input_id, m_frameCnt);
            qcarcam_dump_frame_buffer(m_frame_buf_hndl, m_buf_idx_qcarcam, filename);
        }
    }

    /**********************
     * Post to v4l2node
     ********************** */

    if (ais_v4l2_xml_cfg.latency_measurement_mode == CAMERA_LM_MODE_ALL_STEPS)
    {
        uint64 ptimestamp = 0;
        CameraGetTime(&ptimestamp);
        VPROXY_INFO("LATENCY| input_id %d Framecnt %d Framedone_timestamp: %llu, Cur_timestamp: %llu, latency from framedone: %llu us",
            m_qcarcam_input_id,
            m_frameCnt,
            m_frame_info.timestamp_system,
            ptimestamp,
            (ptimestamp - m_frame_info.timestamp_system) / 1000);
    }

    VPROXY_DBG1("[input %d] Post Frame %d", m_qcarcam_input_id, m_buf_idx_qcarcam);
    ret = post_gfx_buffer_to_v4l2node(m_v4l2_params.v4l2sink, m_frame_info, m_batch_frames);
    if (ret)
    {
        VPROXY_ERROR("[input %d] post_gfx_buffer_to_v4l2node failed", m_qcarcam_input_id);
        ret = qcarcam_release_frame(m_qcarcam_context, m_buf_idx_qcarcam);
        if (ret)
        {
            VPROXY_WARN("[input %d] fail to release %u", m_qcarcam_input_id, m_buf_idx_qcarcam);
            return -1;
        }

        VPROXY_INFO("[input %d] release frame %d", m_qcarcam_input_id, m_buf_idx_qcarcam);
        CameraLockMutex(m_bufstate_mutex);
        m_is_buf_dequeued[m_buf_idx_qcarcam] = 0;
        CameraUnlockMutex(m_bufstate_mutex);
    }

    VPROXY_DBG1("[input %d] Post Frame after buf_idx %i", m_qcarcam_input_id, m_buf_idx_qcarcam);

    return 0;
}

/**
 * Function to handle routine of fetching and releasing frames when one is available
 * @param ais_proxy_ctxt
 * @return 0 on success, -1 on failure
 */
int CAisV4l2ProxyStream::qcarcam_handle_new_frame()
{
    if (ais_v4l2_xml_cfg.latency_measurement_mode == CAMERA_LM_MODE_ALL_STEPS)
    {
        uint64 t_before_get, t_after_get;
        CameraGetTime(&t_before_get);
        if (qcarcam_get_new_frame())
        {
            /*if we fail to get frame, we silently continue...*/
            return 0;
        }
        CameraGetTime(&t_after_get);
        VPROXY_INFO("LATENCY| input_id %d Framecnt %d before_timestamp: %llu after_timestamp: %llu Latency_of_getframe: %llu us",
            m_qcarcam_input_id,
            m_frameCnt,
            t_before_get,
            t_after_get,
            (t_after_get - t_before_get) / 1000);

        qcarcam_post_frame_to_v4l2node();

        return 0;
    }
    else
    {
        if (qcarcam_get_new_frame())
        {
            /*if we fail to get frame, we silently continue...*/
            return 0;
        }

        qcarcam_post_frame_to_v4l2node();

        return 0;
    }

    return 0;
}

void CAisV4l2ProxyStream::qcarcam_event_cb(qcarcam_hndl_t hndl, qcarcam_event_t event_id, qcarcam_event_payload_t *p_payload)
{
    int i = 0;
    int rc = 0;

    // no userdata, need to traverse all the list and find the CAisV4l2ProxyStream
    CAisV4l2ProxyStream *stream_ctxt = find_stream(hndl);

    if (!stream_ctxt)
    {
        VPROXY_ERROR("event_cb called with invalid qcarcam handle %p", hndl);
        return;
    }

    switch (event_id)
    {
        case QCARCAM_EVENT_FRAME_READY:
            if (stream_ctxt->m_is_running)
            {
               if (ais_v4l2_xml_cfg.latency_measurement_mode == CAMERA_LM_MODE_ALL_STEPS)
                {
                    uint64 ptimestamp = 0;
                    CameraGetTime(&ptimestamp);
                    VPROXY_DBG1("%d received QCARCAM_EVENT_FRAME_READY", stream_ctxt->m_qcarcam_input_id);
                    stream_ctxt->qcarcam_handle_new_frame();

                    // todo: refine it for latency measurement
                    VPROXY_INFO("LATENCY| qcarcam_Hndl %lu input_id %d Framecnt (%d, %d) aisbeserver_timestamp: %llu Cur_timestamp: %llu Latency_from_aisbeserver: %llu us",
                        hndl,
                        stream_ctxt->m_qcarcam_input_id,
                        stream_ctxt->m_frameCnt,
                        p_payload->frame_info.seq_no[0],
                        p_payload->frame_info.sof_qtimestamp[0],
                        ptimestamp,
                        (ptimestamp - p_payload->frame_info.sof_qtimestamp[0]) / 1000);
                }
                else
                {
                    VPROXY_DBG1("%d received QCARCAM_EVENT_FRAME_READY", stream_ctxt->m_qcarcam_input_id);
                    stream_ctxt->qcarcam_handle_new_frame();
                }
            }
            break;
        case QCARCAM_EVENT_INPUT_SIGNAL:
        {
            VPROXY_WARN("[input %d] received input signal event %d", stream_ctxt->m_qcarcam_input_id, event_id);
            rc = send_ioctrl_cmd(stream_ctxt->m_stream_fd,
                AIS_V4L2_OUTPUT_PRIV_SET_INPUT_SIGNAL_EVENT, 0,
                sizeof(p_payload->uint_payload), (__u8*)&p_payload->uint_payload);
            if (rc < 0)
            {
                VPROXY_ERROR("[input %d] AIS_V4L2_OUTPUT_PRIV_INPUT_SIGNAL failed", stream_ctxt->m_qcarcam_input_id);
            }
            else
            {
                VPROXY_INFO("[input %d] AIS_V4L2_OUTPUT_PRIV_INPUT_SIGNAL succeed", stream_ctxt->m_qcarcam_input_id);
            }
            break;
        }
        case QCARCAM_EVENT_ERROR:
        {
            VPROXY_WARN("[input %d] received event error %d", stream_ctxt->m_qcarcam_input_id, event_id);
            rc = send_ioctrl_cmd(stream_ctxt->m_stream_fd,
                AIS_V4L2_OUTPUT_PRIV_SET_ERROR_EVENT, 0,
                sizeof(p_payload->uint_payload), (__u8*)&p_payload->uint_payload);
            if (rc < 0)
            {
                VPROXY_ERROR("[input %d] AIS_V4L2_OUTPUT_PRIV_ERROR failed", stream_ctxt->m_qcarcam_input_id);
            }
            else
            {
                VPROXY_INFO("[input %d] send AIS_V4L2_OUTPUT_PRIV_ERROR %d succeed", stream_ctxt->m_qcarcam_input_id,
                    p_payload->uint_payload);
            }

            if (p_payload->uint_payload == QCARCAM_CONN_ERROR)
            {
                VPROXY_ERROR("Connection to the server got lost doing uninitialization");
                QcarcamUninitialize();
            }
            break;
        }
        case QCARCAM_EVENT_VENDOR:
        {
            rc = send_ioctrl_cmd(stream_ctxt->m_stream_fd,
                AIS_V4L2_OUTPUT_PRIV_SET_PARAM_EVENT, AIS_V4L2_PARAM_VENDOR,
                MAX_AIS_V4L2_PARAM_EVNET_SIZE, (__u8*)p_payload->array);
            if (rc < 0)
            {
                VPROXY_ERROR("[input %d] AIS_V4L2_SET_PARAM_EVENT failed", stream_ctxt->m_qcarcam_input_id);
            }
            else
            {
                VPROXY_INFO("[input %d] AIS_V4L2_SET_PARAM_EVENT succeed", stream_ctxt->m_qcarcam_input_id);
            }
            break;
        }
        case QCARCAM_EVENT_FRAME_DROP:
        {
            rc = send_ioctrl_cmd(stream_ctxt->m_stream_fd,
                AIS_V4L2_OUTPUT_PRIV_SET_FRAME_DROP_EVENT, AIS_V4L2_PARAM_VENDOR,
                sizeof(p_payload->frame_info.seq_no[0]), (__u8*)&p_payload->frame_info.seq_no[0]);
            if (rc < 0)
            {
                VPROXY_ERROR("[input %d] QCARCAM_EVENT_FRAME_DROP failed", stream_ctxt->m_qcarcam_input_id);
            }
            else
            {
                VPROXY_INFO("[input %d] QCARCAM_EVENT_FRAME_DROP succeed", stream_ctxt->m_qcarcam_input_id);
            }
            break;
        }
        default:
            VPROXY_WARN("%p received unsupported event %d", stream_ctxt, event_id);
            break;
    }

}

CAisV4l2ProxyStream* CAisV4l2ProxyStream::find_stream(qcarcam_hndl_t qcarcam_context)
{
    vector<CAisV4l2ProxyStream*>::iterator it = s_all_streams.begin();
    for (; it != s_all_streams.end(); ++it)
    {
        if ((*it)->m_qcarcam_context == qcarcam_context)
        {
            return (*it);
        }
    }

    return NULL;
}



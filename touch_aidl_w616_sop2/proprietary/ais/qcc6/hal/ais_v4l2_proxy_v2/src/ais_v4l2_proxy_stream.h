//*************************************************************************************************
// Copyright (c) 2021 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//*************************************************************************************************

#ifndef _CAMERA_AIS_V4L2_PROXY_STREAM_H_
#define _CAMERA_AIS_V4L2_PROXY_STREAM_H_

#include "qcarcam.h"
#include "ais_v4l2_proxy_util.h"
#include <vector>
using std::vector;


class CAisV4l2ProxyInput;
class CAisV4l2ProxyStream
{
public:
    int init(CAisV4l2ProxyInput* pInput);
    int afterInit();
    void uninit();

    int processV4l2Event();

    static CAisV4l2ProxyStream* create_v4l2_stream(QCarCamHndl_t qcarcam_context, int fd, CAisV4l2ProxyInput* );
    static void destroy_v4l2_stream(CAisV4l2ProxyStream* p);
    static CAisV4l2ProxyStream* find_stream(QCarCamHndl_t qcarcam_context);

private:
    CAisV4l2ProxyStream(QCarCamHndl_t qcarcam_context, int fd);
    ~CAisV4l2ProxyStream();
    int processOpenEvent();
    int processCloseEvent();
    int processStartEvent();
    int processStopEvent();
    int processAllocBufs(struct v4l2_event & v4l2Event);
    int processGetParamEvent(struct v4l2_event & v4l2Event);
    int processSetParamEvent(struct v4l2_event & v4l2Event);
    int processOutputBufReady();

    static int poll_thread(void *arg);
    static QCarCamRet_e qcarcam_event_cb(QCarCamHndl_t hndl, uint32_t event_id, const QCarCamEventPayload_t *p_payload, void *pPrivateData);

private:
    int qcarcam_post_frame_to_v4l2node();
    int qcarcam_get_new_frame();
    int qcarcam_handle_new_frame();

private:
    void* m_thread_handle;
    int m_pipefd[2];
    QCarCamHndl_t m_qcarcam_context;
    uint32_t m_qcarcam_input_id;
    int m_stream_fd;
    qcarcam_v4l2_param_t m_v4l2_params;
    qcarcam_buffers_param_t m_buffers_param[AIS_BUFFERS_MAX];

    QCarCamBufferList_t m_p_buffers_output;
    QCarCamBuffer_t  m_qcarcam_buffers[QCARCAM_MAX_NUM_BUFFERS];

    frame_buffer_handle_t m_frame_buf_hndl;
    bool m_is_buf_align;
    int m_buf_idx_qcarcam;
    int m_prev_buf_idx_qcarcam;
    QCarCamFrameInfo_t m_frame_info;

    unsigned int m_batch_frames;

    int m_frameCnt;
    int m_prev_frame;
    int m_is_running;
    bool m_is_first_count;

    CameraMutex m_bufstate_mutex;
    bool m_is_buf_dequeued[QCARCAM_MAX_NUM_BUFFERS];

    CAisV4l2ProxyInput* m_pParentInput;

    static vector<CAisV4l2ProxyStream*> s_all_streams;

};


#endif


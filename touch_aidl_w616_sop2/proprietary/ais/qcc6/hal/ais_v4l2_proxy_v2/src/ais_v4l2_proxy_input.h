//*************************************************************************************************
// Copyright (c) 2021 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//*************************************************************************************************

#ifndef _CAMERA_AIS_V4L2_PROXY_INPUT_H_
#define _CAMERA_AIS_V4L2_PROXY_INPUT_H_

#include "qcarcam.h"
#include "ais_v4l2_proxy_util.h"


#include <vector>
#include <queue>
using std::vector;
using std::queue;

typedef struct
{
    volatile int aborted;
    int dumpFrame;

} ais_v4l2_cfg_t;

QCarCamRet_e QcarcamInitialize();
void QcarcamUninitialize();

class CAisV4l2ProxyStream;

class CAisV4l2ProxyInput
{
public:
    typedef enum
    {
        PIPE_EVENT_CLOSE_STREAM = 0,
    } PIPE_EVENT;

    typedef struct
    {
        CAisV4l2ProxyInput* pInput;
        CAisV4l2ProxyStream* pStream;
    } pipe_event_data_t;

    friend class CAisV4l2ProxyStream;

public:
    CAisV4l2ProxyInput();
    ~CAisV4l2ProxyInput();

    int init(ais_proxy_xml_input_t *p_xml_input, QCarCamInput_t* p_qcarcam_input, QCarCamMode_t* p_qcarcam_mode);
    void uninit();

    int processV4l2Event();
    int processOpenEvent();
    int processCloseEvent(CAisV4l2ProxyStream*);

    static int poll_wait();
    static void add_input(CAisV4l2ProxyInput* pInput);
    static int processPipeEvent(PIPE_EVENT event);
    static int sendPipeEvent(PIPE_EVENT event, const pipe_event_data_t& data);
    static void exit_v4l2_poll();

private:
    uint32_t m_qcarcam_input_id;
    qcarcam_v4l2_param_t m_v4l2_params;
    qcarcam_buffers_param_t m_buffers_param[AIS_BUFFERS_MAX];
    vector<CAisV4l2ProxyStream*> m_streams;
    bool m_is_buf_algin;

    unsigned int m_num_isp_instances;
    QCarCamIspUsecaseConfig_t m_isp_config[QCARCAM_MAX_ISP_INSTANCES];

    static vector<CAisV4l2ProxyInput*> s_inputs;
    static int s_pipefd[2];
    static queue<pipe_event_data_t> s_pipe_events;
};


#endif


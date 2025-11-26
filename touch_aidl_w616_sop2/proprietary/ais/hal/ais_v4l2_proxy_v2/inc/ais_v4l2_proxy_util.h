/* ===========================================================================
 * Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 =========================================================================== */

#ifndef _AIS_V4L2_UTIL_H
#define _AIS_V4L2_UTIL_H
#include <map>
#include <fcntl.h>

#include <errno.h>
#include "qcarcam.h"
#include "CameraCommonTypes.h"

#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include "ais_log.h"
#include "test_util.h"
#include "ais_dmabuffer.h"

#if defined(__ANDROID__)
#include <QtiGralloc.h>
#endif // ANDROID

#define NUM_MAX_DISP_BUFS 3

#if defined(__ANDROID__)
#ifdef DEFAULT_DUMP_LOCATION
#undef DEFAULT_DUMP_LOCATION
#endif
#define DEFAULT_DUMP_LOCATION "/sdcard/Pictures/"
#else
#define DEFAULT_DUMP_LOCATION "/tmp/"
#endif

#define NUM_MAX_CAMERAS 16
#define NUM_MAX_DISP_BUFS 3
#define DEFAULT_FRAME_TIMEOUT 500000000
#define DEFAULT_PRINT_DELAY_SEC 10

#define QCARCAM_MAX_RING_BUFFERS QCARCAM_MAX_NUM_BUFFERS
#define CTL_SUCCEED_RET 0
#define CTL_FAIL_RET    1


#define VPROXY_DBG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_V4L2_PROXY, AIS_LOG_LVL_DBG,  "AIS_V4L2_PROXY %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_DBG], ##__VA_ARGS__)

#define VPROXY_DBG1(_fmt_, ...) \
    ais_log(AIS_MOD_ID_V4L2_PROXY, AIS_LOG_LVL_MED,  "AIS_V4L2_PROXY %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_MED], ##__VA_ARGS__)

#define VPROXY_INFO(_fmt_, ...) \
    ais_log(AIS_MOD_ID_V4L2_PROXY, AIS_LOG_LVL_HIGH,  "AIS_V4L2_PROXY %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_HIGH], ##__VA_ARGS__)

#define VPROXY_ERROR(_fmt_, ...) \
    ais_log(AIS_MOD_ID_V4L2_PROXY, AIS_LOG_LVL_ERROR,  "AIS_V4L2_PROXY %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_ERROR], ##__VA_ARGS__)

#define VPROXY_ALWAYS(_fmt_, ...) \
    ais_log(AIS_MOD_ID_V4L2_PROXY, AIS_LOG_LVL_ERROR,  "AIS_V4L2_PROXY %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_ERROR], ##__VA_ARGS__)

#define VPROXY_WARN(_fmt_, ...) \
    ais_log(AIS_MOD_ID_V4L2_PROXY, AIS_LOG_LVL_ALWAYS,  "AIS_V4L2_PROXY %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_ALWAYS], ##__VA_ARGS__)


/// @brief XML inputs
/// Defines general parameters for each input that are parsed from XML config files
typedef struct
{
    qcarcam_input_desc_t qcarcam_input_id;      ///< Unique input descriptor.
    /*V4l2 pixel formats*/
    unsigned int v4l2_format;
    unsigned int pixformat;
    /*V4l2 node name*/
    char v4l2_node[16];
    /*V4l2 buffer width*/
    int width;
    /*V4l2 buffer height*/
    int height;
    bool is_buf_align;
    /*Operation mode*/
    qcarcam_opmode_type opmode;
    /* ISP params */
    test_util_isp_param_t isp_params;
} ais_proxy_xml_input_t;

typedef struct
{
    char v4l2_node[16];
    int v4l2sink;
    unsigned int pixformat;
    unsigned int v4l2_format;
    unsigned int width;
    unsigned int height;
    qcarcam_opmode_type opmode;
    uint32 size;
} qcarcam_v4l2_param_t;

typedef struct
{
    CameraLatencyMeasurementModeType latency_measurement_mode;
} ais_proxy_gconfig;

typedef enum
{
    AIS_BUFFERS_DIRECT_OUTPUT = 0,
    AIS_BUFFERS_C2D_OUTPUT,
    AIS_BUFFERS_MAX
} ais_buffers_list_t;

typedef struct
{
    qcarcam_buffers_t p_buffers;
    qcarcam_color_fmt_t format;
    int n_buffers;
    unsigned int width;
    unsigned int height;
} qcarcam_buffers_param_t;

///////////////////////////////////////////////////////////////////////////////
/// post_gfx_buffer_to_v4l2node
///
/// @brief Send frame to display
///
/// @param ctxt             Pointer to display context
/// @param user_ctxt        Pointer to structure containing size parameters
/// @param idx              Frame ID number
/// @param rel_idx          Pointer to previous frame ID number
/// @param field_type     Field type in current frame buffer if interlaced
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
int post_gfx_buffer_to_v4l2node(unsigned int v4l2sink, const qcarcam_frame_info_v2_t& frame_info, uint32 batch_frames);

///////////////////////////////////////////////////////////////////////////////
/// qcarcam_dump_frame_buffer
///
/// @brief Dump frame to a file
///
/// @param ctxt             Pointer to display context
/// @param user_ctxt        Pointer to structure containing size parameters
/// @param idx              Frame ID number
/// @param filename         Char pointer to file name to be dumped
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
int qcarcam_dump_frame_buffer(frame_buffer_handle_t hndl, unsigned int idx, const char *filename);

///////////////////////////////////////////////////////////////////////////////
/// parse_xml_config_file
///
/// @brief Parse XML config file for ais proxy server
///
/// @param filename         Char pointer to XML config file name
/// @param inputs           Pointer to struct to store parsed inputs parameters
/// @param max_num_inputs   Maximum number of inputs to be parsed from config file
/// @param config           Global xml configurations
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
int parse_xml_config_file(const char *filename, ais_proxy_xml_input_t *inputs, unsigned int max_num_inputs, ais_proxy_gconfig *config);

int qcarcam_init_v4l2device(qcarcam_v4l2_param_t *v4l2_params);
int set_v4l2_format(qcarcam_v4l2_param_t *v4l2_params, frame_buffer_handle_t hndl);
qcarcam_color_fmt_t get_qcarcam_format_from_v4l2(unsigned int fmt);
unsigned int get_v4l2_format_from_qcarcam(qcarcam_color_fmt_t fmt);

int send_ioctrl_cmd(int fd, uint8 op_code, uint32 param_type, unsigned int size, uint8* payload);

int send_ioctrl_ret(int fd, uint8 op_code, uint32 ret);

int send_ioctrl_bufs(int fd, uint8 op_code, uint32 ret, unsigned int size, uint8* payload);
int qbuf(int fd, const qcarcam_frame_info_v2_t& frame_info, uint32 batch_frames);
int dqbuf(int fd, int& idx);
int subscribeEvent(int fd, uint32 type, uint32 id);



#endif /* _AIS_V4L2_UTIL_H */

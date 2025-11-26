/* ===========================================================================
 * Copyright (c) 2019, 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 =========================================================================== */

#ifndef _AIS_V4L2_UTIL_H
#define _AIS_V4L2_UTIL_H
#include <map>
#include <fcntl.h>
#if defined(ANDROID_Q_AOSP)
#include <gralloc_priv.h>
#else
#include <QtiGralloc.h>
#endif // ANDROID_Q_AOSP
#include <errno.h>
#include "qcarcam.h"

#include <sys/ioctl.h>
#include <linux/videodev2.h>

#ifndef C2D_DISABLED
#include <linux/ion.h>
#include <linux/msm_ion.h>
#include "c2d2.h"
#endif

#include "ais_log.h"
#include "test_util.h"

#define NUM_MAX_DISP_BUFS 3

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__ANDROID__)
#ifdef DEFAULT_DUMP_LOCATION
#undef DEFAULT_DUMP_LOCATION
#endif
#define DEFAULT_DUMP_LOCATION "/sdcard/Pictures/"
#else
#define DEFAULT_DUMP_LOCATION "/tmp/"
#endif


/// @brief XML inputs
/// Defines general parameters for each input that are parsed from XML config files
typedef struct
{
    qcarcam_input_desc_t qcarcam_input_id;      ///< Unique input descriptor.
    /*V4l2 pixel formats*/
    unsigned int pixformat;
    /*V4l2 node name*/
    char v4l2_node[16];
    /*V4l2 buffer width*/
    int width;
    /*V4l2 buffer height*/
    int height;
    /*V4l2 buffer crop top*/
    unsigned int crop_top;
    /*V4l2 buffer crop left*/
    unsigned int crop_left;
    /*V4l2 buffer crop bottom*/
    unsigned int crop_bottom;
    /*V4l2 buffer crop right*/
    unsigned int crop_right;
    /*Operation mode*/
    qcarcam_opmode_type opmode;
    /*Whether recovery is enabled*/
    int recovery;
} ais_proxy_xml_input_t;

typedef struct
{
    int fenceFd;
    int fd;
    void* ptr;
    int size;
    unsigned int c2d_surface_id;
    int is_dequeud;
}ais_buffer_t;

struct qcarcam_frame_buffer_t
{
    /*buffers*/
    ais_buffer_t* buffers;
    int n_buffers;

    int buffer_size[2];
    int stride;
    int format;
    void **pbuf;

    int min_num_buffers;
    int ion_dev;
};

typedef struct
{
    char v4l2_node[16];
    int v4l2sink;
    unsigned int pixformat;
    unsigned int width;
    unsigned int height;
    unsigned int aligned_width;
    unsigned int aligned_height;
    unsigned int crop_top;
    unsigned int crop_left;
    unsigned int crop_bottom;
    unsigned int crop_right;
    unsigned int buffer_status;
    unsigned char *yuv420_buf;
    qcarcam_opmode_type opmode;
} qcarcam_v4l2_param_t;


///////////////////////////////////////////////////////////////////////////////
/// qcarcam_init_ion_buffers
///
/// @brief Initialize new ion buffer
///
/// @param ctxt             Pointer to util context
/// @param user_ctxt        Pointer to new ion buffer
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
qcarcam_ret_t qcarcam_init_ion_buffers(qcarcam_frame_buffer_t **user_ctxt, qcarcam_buffers_t *buffers);

///////////////////////////////////////////////////////////////////////////////
/// qcarcam_deinit_ion_buffers
///
/// @brief Destroy ion
///
/// @param ctxt             Pointer to display context
/// @param user_ctxt        Pointer to structure containing ion parameters
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
qcarcam_ret_t qcarcam_deinit_ion_buffers(qcarcam_frame_buffer_t *user_ctxt);

///////////////////////////////////////////////////////////////////////////////
/// post_ion_buffer_to_v4l2node
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
qcarcam_ret_t post_ion_buffer_to_v4l2node(unsigned int v4l2sink, qcarcam_frame_buffer_t *user_ctxt, unsigned int idx, qcarcam_v4l2_param_t *ctxt);

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
qcarcam_ret_t qcarcam_dump_frame_buffer(qcarcam_frame_buffer_t *user_ctxt, unsigned int idx, const char *filename);

///////////////////////////////////////////////////////////////////////////////
/// create_c2d_surface
///
/// @brief Create a C2D surface
///
/// @param ctxt             Pointer to display context
/// @param user_ctxt        Pointer to structure containing window parameters
/// @param idx              Frame ID number
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
qcarcam_ret_t create_c2d_surface(qcarcam_frame_buffer_t *user_ctxt, unsigned int idx);

///////////////////////////////////////////////////////////////////////////////
/// get_c2d_surface_id
///
/// @brief Get the ID from a C2D surface
///
/// @param ctxt             Pointer to display context
/// @param user_ctxt        Pointer to structure containing window parameters
/// @param idx              Frame ID number
/// @param surface_id       Pointer to C2D sruface ID number
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
qcarcam_ret_t get_c2d_surface_id(qcarcam_frame_buffer_t *user_ctxt, unsigned int idx, unsigned int *surface_id);

///////////////////////////////////////////////////////////////////////////////
/// parse_xml_config_file
///
/// @brief Parse XML config file for ais proxy server
///
/// @param filename         Char pointer to XML config file name
/// @param inputs           Pointer to struct to store parsed inputs parameters
/// @param max_num_inputs   Maximum number of inputs to be parsed from config file
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
int parse_xml_config_file(const char *filename, ais_proxy_xml_input_t *inputs, unsigned int max_num_inputs);

qcarcam_ret_t qcarcam_init_v4l2device(qcarcam_v4l2_param_t *ctxt);
qcarcam_color_fmt_t get_qcarcam_format_from_v4l2(unsigned int fmt);
#ifdef __cplusplus
}
#endif

#endif /* _AIS_V4L2_UTIL_H */

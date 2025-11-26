/* ===========================================================================
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */
#ifndef _TEST_UTIL_RVC_H_
#define _TEST_UTIL_RVC_H_


#ifdef AIS_EARLYSERVICE
#ifdef ANDROID_T_ABOVE
#define ION_NODE "/dev/ion"
#define DRM_NODE "/dev/dri/card2"
#define RVC_NODE "/vendor_early_services/vendor/bin/rvc"
#define AIS_SOCKET "/dev/socket/camera/ais_socket_0"
#else
#define ION_NODE "/early_services/dev/ion"
#define DRM_NODE "/early_services/dev/dri/card2"
#define RVC_NODE "/early_services/rvc"
#define AIS_SOCKET "/early_services/dev/socket/camera/ais_socket_0"
#endif
#else
#define ION_NODE "/dev/ion"
#define DRM_NODE "/dev/dri/card2"
#define AIS_SOCKET "/dev/socket/camera/ais_socket_0"
#endif

#define AIS_SOCKET_ACCESS_RETRY 10000

/*Configuration Parameters*/
#define INPUT_WIDTH 1920
#define INPUT_HEIGHT 1024
#define INPUT_FORMAT QCARCAM_FMT_UYVY_8
#define INPUT_NUM_BUFFERS 4
#define DISPLAY_FORMAT TESTUTIL_FMT_UYVY_8

///////////////////////////////////////////////////////////////////////////////
/// test_util_init_display
///
/// @brief Initialize display and graphics.
///
/// @param ctxt   Pointer to the context to store display/graphics params.
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
qcarcam_ret_t test_util_init_display(test_util_ctxt_t *pCtxt);

///////////////////////////////////////////////////////////////////////////////
/// test_util_set_window_params
///
/// @brief Initialize buffer and windows params.
///
/// @param p_buffer_params   Store buffer params like width, height etc.
/// @param p_window_params   Store window params like size, pos etc. .
/// @param p_buffers_output  Store output buffer size, format and per plane info .
/// @param p_window          Fill buffer info in qcarcam window.
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
qcarcam_ret_t test_util_set_window_params(test_util_buffers_param_t* p_buffer_params, test_util_window_param_t* p_window_params, qcarcam_buffers_t* p_buffers_output, test_util_window_t *p_window);

///////////////////////////////////////////////////////////////////////////////
/// test_util_alloc_camera_buffers
///
/// @brief Allocate camera buffers from dma heap.
///
/// @param p_window  Qcarcam window to store dma node fd and buffer informations.
/// @param buffers   Pointer to store allocated buffer FDs.
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
qcarcam_ret_t test_util_alloc_camera_buffers(test_util_window_t *p_window, qcarcam_buffers_t *buffers);

///////////////////////////////////////////////////////////////////////////////
/// test_util_init_c2d_buffers
///
/// @brief Initialize buffers for display
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
qcarcam_ret_t test_util_init_c2d_buffers(test_util_ctxt_t *ctxt, test_util_window_t *p_window);

#endif //_TEST_UTIL_RVC_H_


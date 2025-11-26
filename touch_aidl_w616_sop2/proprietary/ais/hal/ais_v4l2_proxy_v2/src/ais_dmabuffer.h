/* ===========================================================================
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 =========================================================================== */

#ifndef _AIS_DMABUFFER_H
#define _AIS_DMABUFFER_H

#include <stdint.h>

typedef void* frame_buffer_handle_t;

typedef struct
{
    uint32 w;
    uint32 h;
    uint32 size;
    uint32 bytesperline;
}buf_info_t;

unsigned int v4l2_get_size(frame_buffer_handle_t hndl);
int v4l2_get_buf_info(frame_buffer_handle_t hndl, buf_info_t& info);


///////////////////////////////////////////////////////////////////////////////
/// init_gfx_buffers
///
/// @brief Initialize new gfx buffer
///
/// @param phndl           Pointer to frame_buffer_handle_t
/// @param w               buffer width
/// @param h               buffer height
/// @param format          buffer format
/// @param is_align        not used currently
///
/// @return 0 if successful
///////////////////////////////////////////////////////////////////////////////
int init_gfx_buffers(frame_buffer_handle_t *phndl, uint32 w, uint32 h, uint32 format, bool is_align = 1);
int set_gfx_buf_size(frame_buffer_handle_t hndl, uint32 w, uint32 h);
int set_buf_align(frame_buffer_handle_t hndl, bool is_align=true);

int alloc_gfx_buffers(frame_buffer_handle_t hndl, qcarcam_buffers_t *buffers);

int free_gfx_buffers(frame_buffer_handle_t hndl);

///////////////////////////////////////////////////////////////////////////////
/// deinit_gfx_buffers
///
/// @brief Destroy gfx
///
/// @param hndl             Pointer to frame_buffer_handle_t
///
/// @return 0 if successful
///////////////////////////////////////////////////////////////////////////////
int deinit_gfx_buffers(frame_buffer_handle_t hndl);
void deinit_gfx_device();

#endif
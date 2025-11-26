/*========================================================================

*//** @file hyp_video_fe.h

@par FILE SERVICES:
      Hypervisor video FE interface header


@par EXTERNALIZED FUNCTIONS:
      See below.

Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---     -------------------------------------------------------
02/05/19    rz     Bringup changes for 8155
05/08/17    sm     Update for new hyp-video architecture
========================================================================== */

/* =======================================================================

INCLUDE FILES FOR MODULE

========================================================================== */

#ifndef __HYP_VIDEO_FE_H__
#define __HYP_VIDEO_FE_H__

extern "C" {

#include "hyp_video.h"

#define HVFE_INVALID_HANDLE -1

typedef void *HVFE_HANDLE;

typedef int (*hvfe_callback_handler_t)(void* context, void* message);

typedef struct
{
    hvfe_callback_handler_t handler;
    void* context;
}hvfe_callback_t;

HVFE_HANDLE video_fe_open(const char* str, int flag, hvfe_callback_t* cb);
hypv_status_type video_fe_ioctl(HVFE_HANDLE handle, int cmd, void* data);
hypv_status_type video_fe_close(HVFE_HANDLE handle);

}

#endif

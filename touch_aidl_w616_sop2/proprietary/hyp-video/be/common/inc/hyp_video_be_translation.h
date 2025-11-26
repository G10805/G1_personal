/*===========================================================================

*//** @file hyp_video_be_translation.h
   This file defines the video hypervisor translation between video BE and HAB.

Copyright (c) 2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/

/*===========================================================================
                             Edit History

$Header: $

when(mm/dd/yy)     who         what, where, why
-------------      --------    -------------------------------------------------------
06/23/17           sm          Streamline hypervisor context structure and definitions
05/08/17           sm          Update for new hyp-video architecture
07/08/16           hl          Isolate video data from ioctl
06/20/16           henryl      Clean up and add error messages
06/09/16           henryl      Add the IOCTL_MSG_XXX macro here to be used by BE
06/01/16           hl          Add FE and BE to support Hypervisor interface
============================================================================*/
#ifndef __HYP_VIDEO_BE_TRANSLATION_H__
#define __HYP_VIDEO_BE_TRANSLATION_H__
#include "hyp_videopriv.h"

 /*========================================================================
 Enumerations
 ========================================================================*/

 /*========================================================================
 Functions
 ========================================================================*/
hypv_status_type translate_hvbe_to_habmm(hypv_session_t* hypv_session, int msg_id, void* aInBuf, void* aOutBuf);
hypv_status_type translate_habmm_to_hvbe(hypv_session_t* hypv_session, int msg_id, void* aInBuf, void* aOutBuf);

#endif /* __HYP_VIDEO_BE_TRANSLATION_H__ */

/*========================================================================

*//** @file hyp_video_be.h
   This file defines video hypervisor BE interface.

Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---    --------------------------------------------------------
06/05/18   sm     Initial version of hypervisor linux video BE
06/23/17   sm     Streamline hypervisor context structure and definitions
05/08/17   sm     Update for new hyp-video architecture
 ========================================================================== */

/*========================================================================

                     INCLUDE FILES FOR MODULE

==========================================================================*/

#ifndef __HYP_VIDEO_BE_H__
#define __HYP_VIDEO_BE_H__

void plt_hvbe_open(hypv_session_t* hypv_session, habmm_msg_desc_t* pMsgBufferNode);
void plt_hvbe_ioctl(hypv_session_t* hypv_session, habmm_msg_desc_t* pMsgBufferNode);
void plt_hvbe_close(hypv_session_t* hypv_session, habmm_msg_desc_t* pMsgBufferNode);
hypv_status_type hvbe_callback_handler(uint8 *msg, uint32 length, void *cd);

#endif

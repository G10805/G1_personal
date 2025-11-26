/*========================================================================

*//** @file hyp_video_fe_translation.h

@par FILE SERVICES:
      Hypervisor video FE translator implementation header


@par EXTERNALIZED FUNCTIONS:
      See below.

Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---     -------------------------------------------------------
06/23/17    sm     Streamline hypervisor context structure and definitions
05/08/17    sm     Update for new hyp-video architecture
========================================================================== */

/* =======================================================================

INCLUDE FILES FOR MODULE

========================================================================== */

#ifndef __HYP_VIDEO_FE_TRANSLATION_H__
#define __HYP_VIDEO_FE_TRANSLATION_H__

#ifdef __cplusplus
extern "C"
{
#endif
 /*========================================================================
 Enumerations
 ========================================================================*/

 /*========================================================================
 Functions
 ========================================================================*/
hypv_status_type translate_hvfe_to_habmm(hypv_session_t* hypv_session, int msg_id, void* aInBuf, int nInSize, void* aOutBuf);
hypv_status_type translate_habmm_to_hvfe(hypv_session_t* hypv_session, int msg_id, void* aInBuf, int nInSize, void* aOutBuf);

#ifdef __cplusplus
}
#endif

#endif /* __HYP_VIDEO_FE_TRANSLATION_H__ */

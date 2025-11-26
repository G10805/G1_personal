/*========================================================================

*//** @file hyp_vpp_fe_translation.h
Hypervisor VPP FE translator implementation header

Copyright (c) 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */

#ifndef __HYP_VPP_FE_TRANSLATION_H__
#define __HYP_VPP_FE_TRANSLATION_H__

#ifdef __cplusplus
extern "C"
{
#endif

hypvpp_status_type translate_hvfe_to_habmm(hypvpp_session_t* hypvpp_session, int msg_id, void* pInBuf, int nInSize, void* pOutBuf);
hypvpp_status_type translate_habmm_to_hvfe(hypvpp_session_t* hypvpp_session, int msg_id, void* pInBuf, int nInSize, void* pOutBuf);

#ifdef __cplusplus
}
#endif

#endif /* __HYP_VPP_FE_TRANSLATION_H__ */

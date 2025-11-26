#ifndef __GPR_API_LOG_H__
#define __GPR_API_LOG_H__

/**
 * \file gpr_api_log.h
 * \brief
 *  	This file contains GPR extra APIs - specifically logging for now
 *
 *
 * \copyright
 *  Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //components/rel/gpr.common/1.0/ext/inc/gpr_api_log.h#1 $
*/
// clang-format on

#include "gpr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/


/**
 * Initialize gpr logging
 * \return AR_EOK (0) when successful.
 *
 */
GPR_INTERNAL uint32_t gpr_log_init(void);

/**
 * De-initialize gpr logging, and clean up resources
 * \return AR_EOK (0) when successful.
 *
 */
GPR_INTERNAL uint32_t gpr_log_deinit(void);

/**
 * Log the given gpr packet to gpr_log_history_t
 * \return AR_EOK (0) when successful.
 *
 */
GPR_INTERNAL int32_t gpr_log_packet (gpr_packet_t* packet);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* __GPR_API_LOG_H__ */
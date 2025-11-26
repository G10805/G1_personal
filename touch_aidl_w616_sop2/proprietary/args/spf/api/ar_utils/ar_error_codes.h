#ifndef _AR_ERROR_CODES_H_
#define _AR_ERROR_CODES_H_
/**
 * \file ar_error_codes.h
 * \brief 
 *  	 This file contains common error code definitions to be used across multimedia code bases
 * 
 * \copyright
 *    Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //components/rel/avs.fwk/1.0/api/ar_utils/ar_error_codes.h#2 $
*/
// clang-format on

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "ar_defs.h"
#include "ar_osal_error.h"
/** @addtogroup ar_error_codes
@{ */

/** Status messages (error codes) returned by command responses. */
typedef uint32_t ar_result_t;

#define AR_DID_FAIL(x)    ( AR_EOK != (x) )

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* _AR_ERROR_CODES_H_ */

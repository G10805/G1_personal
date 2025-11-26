/**
 * \file ar_msg.h
 * \brief 
 *  	This file contains mm msg APIs
 *  
 * 
 * \copyright
 *  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //components/rel/gpr.common/1.0/core/inc/ar_msg.h#2 $
*/
// clang-format on

#ifndef _AR_MSG_H
#define _AR_MSG_H

/* =======================================================================
INCLUDE FILES FOR MODULE
========================================================================== */
#include "ar_osal_log.h"
#include "ar_osal_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

//***************************************************************************
// Debug message features
//***************************************************************************

/** @addtogroup ar_logging
@{ */

/*For simultation purposes */
#define DBG_LOW_PRIO 0   /**< Low priority debug message. */
#define DBG_MED_PRIO 1   /**< Medium priority debug message. */
#define DBG_HIGH_PRIO 2  /**< High priority debug message. */
#define DBG_ERROR_PRIO 3 /**< Error priority debug message. */
#define DBG_FATAL_PRIO 4 /**< Fatal priority debug message. */

/* Supporting macros for AR_MSG() logging utility.
 * AR_MSG is supported on both target and simulation builds. */
#undef  AR_MSG

#if !defined(SIM) && defined(__hexagon__)

#include "msg.h"
#include "msgcfg.h"

/* Utility to get variable argument
 * Slide the variable arguments to get the string '_N' to be appended */
#define AR_VA_NUM_ARGS_IMPL(a, b, c, d, e, f, g, h, i, j, _N, ...) _N
#define AR_VA_NUM_ARGS(...) AR_VA_NUM_ARGS_IMPL(, ##__VA_ARGS__, _9, _8, _7, _6, _5, _4, _3, _2, _1, )
/* Concatenates string x with y*/
#define AR_TOKENPASTE(x, y) x##y

/*Macro parses the text to MSG with _N as suffix, where N = {1, 2, 3, .... ,9} */
#define AR_MSG_x(_N) AR_TOKENPASTE(MSG, _N)

#ifndef MSG_SSID_DFLT
#define MSG_SSID_DFLT MSG_SSID_QDSP6
#endif // MSG_SSID_DFLT

/* This Logging macro supports variable arguments*/
#define AR_MSG(xx_ss_mask, xx_fmt, ...)                                                                                \
   AR_MSG_x(AR_VA_NUM_ARGS(__VA_ARGS__))(MSG_SSID_QDSP6, xx_ss_mask, xx_fmt, ##__VA_ARGS__)

#else  // !defined(SIM) && defined(__hexagon__)

#define AR_MSG(xx_ss_mask, xx_fmt, ...)                                    \
      if (xx_ss_mask == DBG_LOW_PRIO) {                                    \
         AR_LOG_LOW("GPR:",xx_fmt, ##__VA_ARGS__);                       \
      }                                                                    \
      else if(xx_ss_mask == DBG_MED_PRIO)                                  \
      {                                                                    \
         AR_LOG_MED("GPR:",xx_fmt, ##__VA_ARGS__);                       \
      }                                                                    \
      else if(xx_ss_mask == DBG_HIGH_PRIO)                                 \
      {                                                                    \
         AR_LOG_HIGH("GPR:",xx_fmt, ##__VA_ARGS__);                      \
      }                                                                    \
      else if(xx_ss_mask == DBG_ERROR_PRIO)                                \
      {                                                                    \
         AR_LOG_ERROR("GPR:",xx_fmt, ##__VA_ARGS__);                     \
      }                                                                    \
      else if(xx_ss_mask == DBG_FATAL_PRIO)                                \
      {                                                                    \
         AR_LOG_FATAL("GPR:",xx_fmt, ##__VA_ARGS__);                     \
      }

/** @} */ /* end_addtogroup ar_logging */

#endif // !defined(SIM) && defined(__hexagon__)


#ifdef __cplusplus
}
#endif //__cplusplus

#endif // #ifndef _AR_MSG_H

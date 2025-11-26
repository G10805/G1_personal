/**
 * \file gpr_main.c
 * \brief
 *  	This file contains GPR main implementation
 *
 *
 * \copyright
 *  Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //components/rel/gpr.common/1.0/core/src/gpr_main.c#25 $
*/
// clang-format on

/******************************************************************************
 * Includes                                                                    *
 *****************************************************************************/

#include "ar_osal_error.h"
#include "ar_types.h"
#include "gpr_api_log.h"
#include "gpr_api_i.h"
#include "ar_msg.h"

/*****************************************************************************
 * Variables                                                                 *
 ****************************************************************************/
bool_t gpr_init_flag = FALSE;

/*****************************************************************************
 * Core Routine Implementations                                              *
 ****************************************************************************/
GPR_EXTERNAL uint32_t gpr_init(void)
{
   uint32_t rc;
   if (gpr_init_flag)
   {
      
      AR_MSG(DBG_HIGH_PRIO, "GPR is already initialized");
      return AR_EOK;
   }
   rc = gpr_drv_init();
#ifndef DISABLE_DEINIT
   if (rc)
   {
      (void)gpr_deinit();
      return AR_EFAILED;
   }
   
   rc = gpr_log_init();
   if (rc)
   {
      (void)gpr_log_deinit();
      (void)gpr_deinit();
      return AR_EFAILED;
   }

#endif
   gpr_init_flag = TRUE;
   return AR_EOK;
}

#ifndef DISABLE_DEINIT
GPR_EXTERNAL uint32_t gpr_deinit(void)
{
  AR_MSG(DBG_HIGH_PRIO, "De-initializing GPR driver");
  (void)gpr_drv_deinit();

  (void)gpr_log_deinit();

   gpr_init_flag = FALSE;
   return AR_EOK;
}
#endif

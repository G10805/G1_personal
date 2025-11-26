#ifndef _HW_CORE_API_H_
#define _HW_CORE_API_H_
/**
 * \file hw_core_api.h
 * \brief 
 *  	 This file contains hw core api
 * 
 * \copyright
 *  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //components/rel/avs.fwk/1.0/api/modules/hw_core_api.h#7 $
*/
// clang-format on

#include "ar_defs.h"
#include "lpass_core_api.h"


/**
   Param ID for hw core
   This ID should only be used under PRM module instance
*/
#define PARAM_ID_RSC_HW_CORE 0x08001032

// uncomment once build errors are fixed on OTTP end
//#define PARAM_ID_RSC_LPASS_CORE 0x0800102B

#include "spf_begin_pack.h"
/** Struct for response for hw core request by HLOS  */
struct hw_core_request_t
{
   uint32_t hw_core_id;
   /**< HW core id
    * @values #HW_CORE_ID_LPASS
    *         #HW_CORE_ID_DCODEC */
}
#include "spf_end_pack.h"
;

typedef struct hw_core_request_t hw_core_request_t;

/* LPASS core id */
#define HW_CORE_ID_LPASS 1

/* DCODEC core id */
#define HW_CORE_ID_DCODEC 2

#endif /* _HW_CORE_API_H_ */

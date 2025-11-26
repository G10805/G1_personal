#ifndef _MOD_CMN_API_H_
#define _MOD_CMN_API_H_
/**
 * \file module_cmn_api.h
 * \brief
 *           this file contains common module definitions
 *
 * \copyright
 *  Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //components/rel/avs.fwk/1.0/api/modules/module_cmn_api.h#9 $
*/
// clang-format on

#include "apm_graph_properties.h"
#include "media_fmt_api.h"

/**
    ID of the Enable parameter used by any audio processing module.
    This generic/common parameter is used to configure or determine the
    state of any audio processing module.
 */
#define PARAM_ID_MODULE_ENABLE                                   0x08001026

/** @h2xmlp_parameter   {"PARAM_ID_MODULE_ENABLE", PARAM_ID_MODULE_ENABLE}
    @h2xmlp_description {Parameter for enabling/disabling the modules}
    @h2xmlp_toolPolicy  {Calibration; RTC} */

/* Structure for Enable parameter for any Audio processing modules. */
typedef struct param_id_module_enable_t param_id_module_enable_t;

#include "spf_begin_pack.h"
struct param_id_module_enable_t
{
   uint32_t enable;
   /**< @h2xmle_description  {Specifies whether the module is to be enabled or disabled.}
        @h2xmle_rangeList    {"Disable"=0;
                              "Enable"=1}
        @h2xmle_default      {0}
        @h2xmle_policy       {Basic} */
}
#include "spf_end_pack.h"
;


#define PARAM_ID_LIB_VERSION                                   0x00010937

/* Structure for Querying module lib version of any Audio processing modules. */
typedef struct lib_version_t lib_version_t;
/** @h2xmlp_parameter   {"PARAM_ID_LIB_VERSION", PARAM_ID_LIB_VERSION}
    @h2xmlp_description {To query the lib version of any audio processing module.}
    @h2xmlp_toolPolicy  {RTC_READONLY}
    @h2xmlp_readOnly    {true}*/

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"

/* Payload of the PARAM_ID_LIB_VERSION parameter used by
 any Audio Processing module
 */
struct lib_version_t
{
   uint32_t lib_version_low;
   /**< @h2xmle_description  {Version of the module LSB.} */


   uint32_t lib_version_high;
    /**< @h2xmle_description  { Version of the module MSB} */

}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;


#endif /* _MOD_CMN_API_H_ */

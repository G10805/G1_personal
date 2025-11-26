/**
 * \file amdb_api.h
 * \brief
 *  	 API header for AMDB.
 *
 * \copyright
 *  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //components/rel/avs.fwk/1.0/api/modules/amdb_api.h#12 $
*/
// clang-format on

#ifndef _AMDB_API_H_
#define _AMDB_API_H_

/*------------------------------------------------------------------------
 * Include files
 * -----------------------------------------------------------------------*/
#include "ar_defs.h"

/** @h2xml_title1           {APIs of Audio Module Data Base (AMDB)}
    @h2xml_title_agile_rev  {APIs of Audio Module Data Base (AMDB)}
    @h2xml_title_date       {October 31, 2018} */
/**
   @h2xmlx_xmlNumberFormat {int}
*/

/** @addtogroup amdb_acdb_module_info
@{ */

#define AMDB_MODULE_REG_STRUCT_V1 1
/** Interface type is Common Audio Processor Interface */
#define AMDB_INTERFACE_TYPE_CAPI 2

/** Interface version is Common Audio Processor Interface version 3. */
#define AMDB_INTERFACE_VERSION_CAPI_V3 3

/** Module is generic, such as a preprocessor, post-processor, etc. */
#define AMDB_MODULE_TYPE_GENERIC 2

/** Module is a decoder. */
#define AMDB_MODULE_TYPE_DECODER 3

/** Module is an encoder. */
#define AMDB_MODULE_TYPE_ENCODER 4

/** Module is a converter. */
#define AMDB_MODULE_TYPE_CONVERTER 5

/** Module is a packetizer. */
#define AMDB_MODULE_TYPE_PACKETIZER 6

/** Module is a de-packetizer. */
#define AMDB_MODULE_TYPE_DEPACKETIZER 7
/** @} */ /* end_addtogroup amdb_acdb_module_info */

#include "spf_begin_pack.h"
struct amdb_module_reg_info_t
{
   uint16_t interface_type;
   /**< Type of the module interface.

      @values
      - #AMDB_INTERFACE_TYPE_CAPI
      @tablebulletend */

   uint16_t interface_version;
   /**< Version of the module interface.

      @values
      - #AMDB_INTERFACE_VERSION_CAPI_V3
      @tablebulletend */

   /*following this struct we have amdb_capi_module_reg_info_t
   when interface type = CAPI and interface_version =3.*/
}
#include "spf_end_pack.h"
;

typedef struct amdb_module_reg_info_t amdb_module_reg_info_t;

#include "spf_begin_pack.h"
struct amdb_capi_module_reg_info_t
{
   uint32_t module_type;
   /**< Type of module.

        @values
        - #AMDB_MODULE_TYPE_GENERIC
        - #AMDB_MODULE_TYPE_DECODER
        - #AMDB_MODULE_TYPE_ENCODER
        - #AMDB_MODULE_TYPE_CONVERTER
        - #AMDB_MODULE_TYPE_PACKETIZER
        - #AMDB_MODULE_TYPE_DEPACKETIZER
        @tablebulletend */

   uint32_t module_id;
   /**< ID of the module */

   uint16_t file_name_len;
   /**< Length of the SO filename (in bytes).

        The element file_name in the structure is of length file_name_len.
        @newpagetable */

   uint16_t tag_len;
   /**< Length of tag in bytes. This tag identifies the entry point function.
        The element tag in the structure is of length tag_len bytes */

   uint32_t error_code;
   /**< Result of the module registration. The spf must be able to write to
        this memory. Populated for out-of-band only    */
}
#include "spf_end_pack.h"
;
typedef struct amdb_capi_module_reg_info_t amdb_capi_module_reg_info_t;

/*
  Followed by amdb_capi_module_reg_info_t, following variable-size arrays are present
  Their size depends on the variables present in the amdb_capi_module_reg_info_t

  uint8_t file_name[0];
     Filename of the so file of length file_name_len bytes.
     Total size of this variable array is 
	 file_name_len * sizeof(uint8_t)

  uint8_t tag[0];
     Tag name of length tag_len
     For CAPIv2, the tag is a prefix for the entry point functions:
        - tag + _get_static_properties
        - tag + _init
     Total size of this variable array is 
	 tag_len * sizeof(uint8_t)

  uint8_t alignment[0];
     for 4-byte alignment
*/

/**
  Instance ID of the AMDB module

  This module instance supports the following command IDs:
  - #AMDB_CMD_REGISTER_MODULES
  - #AMDB_CMD_DEREGISTER_MODULES
  - #AMDB_CMD_LOAD_MODULES
  - #AMDB_CMD_UNLOAD_MODULES

  @calibration_hdr_field
  module_id -- AMDB_MODULE_INSTANCE_ID
 */
#define AMDB_MODULE_INSTANCE_ID 0x00000003

#define AMDB_CMD_REGISTER_MODULES 0x0100101E

/*< Payload for param ID AMDB_CMD_REGISTER_MODULES
    This payload can be both in-band and out-of-band
    Overall structure is :
    {
      amdb_module_registration_t
      amdb_module_reg_info_t[num_modules]
    }
*/
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct amdb_module_registration_t
{
   uint32_t proc_domain;
   /**< Process id of the process in which AMDB is running*/

   uint32_t num_modules;
   /**< Number of modules being registered */

   uint32_t struct_version;
   /**< Version of amdb_module_reg_info_t struct
       @values
      - #AMDB_MODULE_REG_STRUCT_V1
      @tablebulletend*/

   amdb_module_reg_info_t reg_info[0];
   /**< Registration information for num_modules,
        It will be of type reg_info[num_modules].
        Each reg_info (including file_name and tag)
        should be 4 bytes aligned */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;
typedef struct amdb_module_registration_t amdb_module_registration_t;

#define AMDB_CMD_DEREGISTER_MODULES 0x0100101F

/* Payload for param ID AMDB_CMD_DEREGISTER_MODULES
 * This payload can be both in-band and out-of-band
 * Overall structure is :
 * {
 *    amdb_module_deregistration_t
 *    uint32_t[num_modules]
 * }
 * */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct amdb_module_deregistration_t
{
   uint32_t proc_domain;
   /**< Process id of the process in which AMDB is running*/

   uint32_t num_modules;
   /**< Number of modules being registered */

   uint32_t module_id[0];
   /**< ID for num_modules */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;
typedef struct amdb_module_deregistration_t amdb_module_deregistration_t;

#include "spf_begin_pack.h"
struct amdb_load_unload_info_t
{
   uint32_t module_id;
   /**< ID of the module being loaded */

   uint32_t handle_lsw;
   /**< LSW of the Handle to the module loaded */

   uint32_t handle_msw;
   /**< MSW of the Handle to the module loaded. */
}
#include "spf_end_pack.h"
;
typedef struct amdb_load_unload_info_t amdb_load_unload_info_t;

#define AMDB_CMD_LOAD_MODULES 0x01001020

/* Payload for param ID AMDB_CMD_LOAD_MODULES
 * This payload can be both in-band and out-of-band
 * Overall structure is :
 * {
 *    amdb_module_load_unload_t
 *    amdb_load_unload_info_t[num_modules]
 * }
 *
 * AMDB_CMD_RSP_LOAD_MODULES is used for the response
 * For in-band payload, we copy the incoming payload and modify if
 * needed and send the response
 * For out-of-band payload, incoming payload is sent back.
 */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct amdb_module_load_unload_t
{
   uint32_t proc_domain;
   /**< Process id of the process in which AMDB is running*/

   uint32_t error_code;
   /** < Result of the module loading. For out-of-band payload
         this block will be directly updated. */

   uint32_t num_modules;
   /**< Number of modules being registered */

   amdb_load_unload_info_t load_unload_info[0];
   /**< Load/unload information for num_modules.
      It will be of type load_unload_info[num_modules] */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;
typedef struct amdb_module_load_unload_t amdb_module_load_unload_t;

#define AMDB_CMD_RSP_LOAD_MODULES 0x02001008
/* Payload for param ID AMDB_CMD_RSP_LOAD_MODULES
 * This payload can be both in-band and out-of-band
 * Overall structure is :
 * {
 *    amdb_module_load_unload_t
 *    amdb_load_unload_info_t[num_modules]
 * }
 *
 * For in-band payload, we copy the incoming payload and modify if
 * needed and send the response
 * For out-of-band payload, incoming payload is sent back
 */

#define AMDB_CMD_UNLOAD_MODULES 0x01001022

/* Payload for param ID AMDB_CMD_UNLOAD_MODULES
 * This payload can be both in-band and out-of-band
 * Overall structure is :
 * {
 *    amdb_module_load_unload_t
 *    amdb_load_unload_info_t[num_modules]
 * }
 *
 * Basic response is used for the response
 */

#endif //_AMDB_API_H_

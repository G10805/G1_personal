#ifndef _SPR_BUFFER_API_H_
#define _SPR_BUFFER_API_H_
/**
 * \file spr_api.h
 * \brief 
 *  	 This file contains Public APIs for Splitter-Renderer module.
 * 
 * \copyright
 *  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //components/rel/avs.fwk/1.0/api/modules/spr_api.h#24 $
*/
// clang-format on

#include "ar_defs.h"
#include "module_cmn_api.h"
#include "imcl_fwk_intent_api.h"

/**
  @h2xml_title1          {Splitter Renderer Module}
  @h2xml_title_agile_rev {Splitter Renderer Module}
  @h2xml_title_date      {March 19, 2019}
*/
/** @h2xmlp_subStruct */
struct time_us_t
{
   uint32_t value_lsw;
   /**< @h2xmle_description {Lower 32 bits of 64 bit time value in microseconds}
        @h2xmle_range       { 0..0xFFFFFFFF } */
   uint32_t value_msw;
   /**< @h2xmle_description {Upper 32 bits of 64 bit time value in microseconds}
        @h2xmle_range       { 0..0xFFFFFFFF } */
};

/* Structure type def for above payload. */
typedef struct time_us_t time_us_t;

// Indicates that the render start time is immediate. Data is rendered as soon as it is available after
// the render decision
#define SPR_RENDER_MODE_IMMEDIATE                         1

// Indicates that the render start time is at given value of absolute time
#define SPR_RENDER_MODE_ABSOLUTE_TIME                     2

// Indicates that the render start time is at a specified offset from the current absolute time (wall clock)
#define SPR_RENDER_MODE_DELAYED                           3

/** @h2xmlp_subStruct */
struct config_spr_render_mode_t
{
   uint32_t render_mode;
   /**< @h2xmle_description {Mode of rendering that indicates when to start rendering data}
        @h2xmle_rangeList   {"IMMEDIATE"=1;
                             "ABSOLUTE_TIME"=2;
                             "DELAYED"=3}
        @h2xmle_default     {1}  */
   time_us_t render_start_time;
   /**<
       @h2xmle_description { Render start time in microseconds. Value interpreted based on 'render_mode' field.
                             render_mode :
                              IMMEDIATE     - Don't care.
                              ABSOLUTE_TIME - 64-bit absolute wall clock(WC) time in microseconds.
                              DELAYED       - 64-bit time to delay render in microseconds.
                            }
   */
};

/* Structure type def for above payload. */
typedef struct config_spr_render_mode_t config_spr_render_mode_t;

// Sign extended default value (-infinity) for render window start
#define DEFAULT_RENDER_WINDOW_START_MSW                   AR_NON_GUID(0x80000000)
#define DEFAULT_RENDER_WINDOW_START_LSW                   AR_NON_GUID(0x00000000)

// Sign extended default value (+infinity) for render window end
#define DEFAULT_RENDER_WINDOW_END_MSW                     AR_NON_GUID(0x7FFFFFFF)
#define DEFAULT_RENDER_WINDOW_END_LSW                     AR_NON_GUID(0xFFFFFFFF)

/** @h2xmlp_subStruct */
struct config_spr_render_window_t
{
   time_us_t render_window_start;
   /**< @h2xmle_description { Tolerance window start for incoming timestamps used to make rendering decisions.
                              Timestamps behind current window start are held. Supported only in resolution of
                              microseconds. 64 bit render window value is treated as signed value. Sign extension is configurer's
                              responsibility.}
   */

   time_us_t render_window_end;
   /**< @h2xmle_description {
                             Tolerance window end for incoming timestamps used to make rendering decisions.
                             Timestamps ahead of current window end are dropped. Supported only in resolution of
                             microseconds. 64 bit render window value is treated as signed value. Sign extension is configurer's
                             responsibility.}
  */
};

/* Structure type def for above payload. */
typedef struct config_spr_render_window_t config_spr_render_window_t;

// Indicates that the render decision is based on the primary device render rate (or session clock mode)
#define SPR_RENDER_REFERENCE_DEFAULT                      1

// Indicates that the render decision is based on the local wall clock reference.
#define SPR_RENDER_REFERENCE_WALL_CLOCK                   2

/** @h2xmlp_subStruct */
struct config_spr_render_reference_t
{
   uint32_t render_reference;
   /**< @h2xmle_description {
                             Payload field indicating choice of render decision reference for incoming data.
                             DEFAULT     - Default mode, decision based on rendering rate of primary device served by SPR.
                             WALL_CLOCK  - Local wall clock mode, decision based on Local wall clock. Recommended
                                          for loopback usecases.
                            }
        @h2xmle_rangeList   {"DEFAULT"=1;
                             "WALL_CLOCK"=2}
        @h2xmle_default     {1}  */
};

/* Structure type def for above payload. */
typedef struct config_spr_render_reference_t config_spr_render_reference_t;

/** @h2xmlp_subStruct */
struct config_spr_render_hold_duration_t
{
   time_us_t max_hold_buffer_duration;
   /**< @h2xmle_description {
                             Maximum hold buffer size in microseconds when render decision is to hold.
                             Supported only in resolution of microseconds.
                             Client can choose to ignore setting up this size for SPR module to choose a default value.
                            }
        @h2xmle_default     {0}
   */
};

/* Structure type def for above payload. */
typedef struct config_spr_render_hold_duration_t config_spr_render_hold_duration_t;

/** Definition of render mode bitmask. */
#define SPR_RENDER_BIT_MASK_RENDER_MODE                   AR_NON_GUID(0x00000001)

/** Definition of render mode shift value. */
#define SPR_RENDER_SHIFT_RENDER_MODE                      0

/** Definition of render reference bitmask. */
#define SPR_RENDER_BIT_MASK_RENDER_REFERENCE              AR_NON_GUID(0x00000002)

/** Definition of render reference shift value. */
#define SPR_RENDER_SHIFT_RENDER_REFERENCE                 1

/** Definition of render window bitmask. */
#define SPR_RENDER_BIT_MASK_RENDER_WINDOW                 AR_NON_GUID(0x00000004)

/** Definition of render window shift value. */
#define SPR_RENDER_SHIFT_RENDER_WINDOW                    2

/** Definition of render hold duration. */
#define SPR_RENDER_BIT_MASK_HOLD_DURATION                 AR_NON_GUID(0x00000008)

/** Definition of render hold duration shift value. */
#define SPR_RENDER_SHIFT_HOLD_DURATION                    3

#define PARAM_ID_SPR_DELAY_PATH_END 0x080010C4
/**
  @h2xmlp_parameter   {"PARAM_ID_SPR_DELAY_PATH_END", PARAM_ID_SPR_DELAY_PATH_END}
  @h2xmlp_description {Parameter that defines end of the delay path.\n
  SPR will use delay from itself to the end module mentioned here.\n

  The SPR module needs path delay in order to perform accurate AVsync or return session time to the client.\n
  A path is defined as a series of connections from one module to another through which data flows.\n
  Path delay is the sum of the various delays suffered by the data as it flows through the path.\n
  Various contributions for path delay are a) algorithmic delays b) buffering delays etc.\n
  In case of SPR, path delay required is the aggregation of delay from SPR to the end point module.\n
  SPR needs to be told about the end point through the below API.\n

  In future, we might introduce API to set fixed path delay; but to be more accurate delay has to be\n
  automatically calculated, which is possible with this API.\n
  SPR might have multiple output ports. spf will search and\n
  find the appropriate output port that leads to the given end point.\n
  If there are multiple paths that lead to the same end point (due to splits and mergers),\n
  then first path will be chosen (which can be arbitrary).\n

  For multiple end points being fed from SPR, the same param is sent multiple times.\n
  SPR will store delay per output port by auto-detecting the output port corresponding to the end point.\n
  In case, two different end points are fed from same output port (due to a splitter downstream of SPR),\n
  then the latest end point will be used by SPR.\n}

  @h2xmlp_toolPolicy  {Calibration} */
#include "spf_begin_pack.h"
struct param_id_spr_delay_path_end_t
{
   uint32_t module_instance_id;
   /**< @h2xmle_description {instance ID of the module at the end of the path used for delay calculations}
        @h2xmle_range       { 0..0xFFFFFFFF }
        @h2xmle_default     {0}  */
}
#include "spf_end_pack.h"
;

typedef struct param_id_spr_delay_path_end_t param_id_spr_delay_path_end_t;

/*==============================================================================
   Constants
==============================================================================*/

/** Bitmask for the Timestamp Valid flag in param_id_session_time_t */
#define PARAM_ID_SESSION_TIME_BIT_MASK_IS_TIMESTAMP_VALID AR_NON_GUID(0x80000000UL)

/** Shift value for the Timestamp Valid flag in param_id_session_time_t */
#define PARAM_ID_SESSION_TIME_SHIFT_IS_TIMESTAMP_VALID    31

/*==============================================================================
   API definitions
==============================================================================*/
/* ID of the parameter used to query the session time */
#define PARAM_ID_SPR_SESSION_TIME                         0x0800113D

/**
    @h2xmlp_parameter   {"PARAM_ID_SPR_SESSION_TIME", PARAM_ID_SPR_SESSION_TIME}
    @h2xmlp_description { Used only for Get Parameter Calls.\n
    Provides information related to the current session time.\n
    The session time value must be interpreted by clients as a signed 64-bit
    value. A negative session time can be returned in cases where the client
    wants to start running a session in the future, but first queries for the
    session clock.\n
    The absolute time value in the command response must be interpreted by
    clients as an unsigned 64-bit value. The absolute time can be in the past
    or future.\n
    The timestamp field is populated using the last processed timestamp.\n
    This field follows certain rules:\n
    - For an uninitialized or invalid timestamp, the value reads 0, and the
      flag that follows this timestamp field is set to 0 to indicate
      Uninitialized/Invalid.\n
    - During steady state rendering, it reflects the last processed timestamp.\n
    The timestamp value in the command response must be interpreted by
    clients as an unsigned 64-bit value.\n

    If the client does not enable the AVSync feature via PARAM_ID_SPR_AVSYNC_CONFIG,\n
    then these values will be returned as 0}\n

    @h2xmlp_toolPolicy  {Calibration} */
/*==============================================================================
   Type definitions
==============================================================================*/
#include "spf_begin_pack.h"
struct param_id_spr_session_time_t
{
   time_us_t session_time;
   /**<
        @h2xmle_description { Value of the current session time in microseconds.}
        @h2xmle_range       { 0..0xFFFFFFFF }
        @h2xmle_default     {0}  */

   time_us_t absolute_time;
   /**<
        @h2xmle_description {Value of the absolute time in microseconds.\n
                            This is the time when the sample corresponding to session time is rendered at hardware.\n
                            It can be slightly in the future or past depending on when this is queried.}

        @h2xmle_range       { 0..0xFFFFFFFF } 
        @h2xmle_default     {0}  */

   time_us_t timestamp;
   /**<
        @h2xmle_description {Value of the last processed time stamp in microseconds.\n
         The 64 bit number formed are treated as signed}
        @h2xmle_range       { 0..0xFFFFFFFF } 
        @h2xmle_default     {0}  */

   uint32_t flags;
   /**< @h2xmle_description {Configures additional properties associated with this payload.\n
                             Bit 31 indicates if the time stamp is valid or not\n
                             - 0 -- The value of last processed timestamp is invalid/uninitialized\n
                             - 1 -- The value of last processed timestamp is valid\n
                             All other bits are reserved and are set to 0}
        @h2xmle_rangeList   { "TS_INVALID"=0x00000000;
                              "TS_VALID"=0x80000000 }
        @h2xmle_default     {0}  */
}
#include "spf_end_pack.h"
;

/* Payload associated with PARAM_ID_SPR_SESSION_TIME */
typedef struct param_id_spr_session_time_t param_id_spr_session_time_t;

#define PARAM_ID_SPR_AVSYNC_CONFIG                        0x0800115B
/**
  @h2xmlp_parameter   {"PARAM_ID_SPR_AVSYNC_CONFIG", PARAM_ID_SPR_AVSYNC_CONFIG}
  @h2xmlp_description { Parameter that configures information needed for the AVSync\n
  functionalities for SPR including data render decision.\n

  SPR module makes render decisions on incoming data.\n
  Render decision involves rendering(allowing data to output), holding or dropping the incoming data.\n
  SPR module needs to know if the avsync feature itself has to be enabled. If so, the following additional\n
  information is required a) when to render c) what to gate/compare against and c) render tolerance window\n

  Since SPR is performing the operation of gating input to output, GRAPH START on the subgraph hosting\n
  the SPR module does not guarantee immediate data flow in the subgraph if the render decision parameter\n
  is not configured to render immediately.\n

  This parameter is expected to be set before the SPR module is moved to start state.\n
     If this parameter is not set by then, then no AVSync functionality is enabled for the current\n
         run\n
     If this parameter is set with enable = 1 AND the client wishes to use the default values for\n
          all the other render parameters for this particular run, then the payload values expected are\n
          enable = 1 AND render_mask = 0. The individual param descriptions contain the default values.\n
     If the client wishes to configure fields specifically, they can set enable = 1 and setup the\n
          render_mask to reflect the values that have to be updated by the SPR. For the fields that\n
          were not configured, the SPR chooses the default values\n

  Once the subgraph hosting the SPR is moved to start state, no changes in this configuration is\n
  allowed except for the following case :\n
     If the client enabled AVSync before the start state transition, then only render window is\n
     allowed to be updated in run time.\n
     Any other changes are rejected and will result in a failure for the set param command\n

  Till the subgraph hosting the SPR is started, the client can set this param multiple times.\n
  The last configured value is taken into consideration.\n

  If a subgraph hosting SPR is stopped and started/paused and resumed, the AVSync functionality will\n
  proceed with the previous configuration (if any) and use the first buffer to perform the\n
  first sample rendering. The  client can always configure this parameter before starting the session\n
  again to ensure the desired behavior.\n

  This parameter only supports set configuration, get configuration is not supported.\n}

  @h2xmlp_toolPolicy  {Calibration} */
#include "spf_begin_pack.h"
struct param_id_spr_avsync_config_t
{
   uint16_t enable;
   /**< @h2xmle_description {Indicates if the AVSync functionality has to be enabled.\n
                             This can be enabled\disabled till the subgraph hosting the\n
                             SPR container is started. The value of this variable most recently\n
                             configured is used to decide if the AVSync feature has to be\n
                             enabled or not.\n
                             If this is set to 0, then the rest of the values are\n
                             ignored by SPR and any configured fields are reset}
        @h2xmle_rangeList   {"DISABLE"= 0,
                             "ENABLE" = 1}
        @h2xmle_default     {0}  */

   uint16_t render_mask;
   /**< @h2xmle_description {Render mask with bitfields to selectively configure fields. \n
                             All other bits are reserved, clients must set them to 0.\n
                             if any of these bits are set to ignore, then the last configured\n
                             value or the default is used as long as avsync enable=1}
        @h2xmle_range       {0x0000..0xFFFF}

        @h2xmle_bitField    {0x0001}
          @h2xmle_bitName     {Bit_0_Render_Mode_Config}
          @h2xmle_description {Configures the render mode for the SPR. Effective only if enable=1}
          @h2xmle_rangeList   {"IGNORE"= 0,
                               "CONFIGURE" = 1}
        @h2xmle_bitFieldEnd

        @h2xmle_bitField    {0x0002}
          @h2xmle_bitName     {Bit_1_Render_Reference_Config}
          @h2xmle_description {Configures the render reference for the SPR. Effective only if enable=1}
          @h2xmle_rangeList   {"IGNORE"= 0,
                               "CONFIGURE" = 1}
        @h2xmle_bitFieldEnd

        @h2xmle_bitField    {0x0004}
          @h2xmle_bitName     {Bit_2_Render_Window_Config}
          @h2xmle_description {Configures the render window for the SPR. Effective only if enable=1}
          @h2xmle_rangeList   {"IGNORE"= 0,
                               "CONFIGURE" = 1}
        @h2xmle_bitFieldEnd

        @h2xmle_bitField    {0x0008}
          @h2xmle_bitName     {Bit_3_Hold_Duration_Config}
          @h2xmle_description {Configures the maximum hold duration expected for the SPR. Effective only if enable=1}
          @h2xmle_rangeList   {"IGNORE"= 0,
                               "CONFIGURE" = 1}
        @h2xmle_bitFieldEnd

        @h2xmle_bitfield    {0xFFF0}
          @h2xmle_bitName     {Bit_15_to_4_RESERVED}
          @h2xmle_description {Reserved Bits[15:4]}
          @h2xmle_rangeList   {"RESERVED"= 0}
          @h2xmle_visibility  {hide}
        @h2xmle_bitfieldEnd
   */

   config_spr_render_mode_t render_mode_config;
   /**< @h2xmle_description {Render mode and start time configuration parameter } */

   config_spr_render_reference_t render_reference_config;
   /**< @h2xmle_description {Render reference configuration parameter } */

   config_spr_render_window_t render_window;
   /**< @h2xmle_description {Render window configuration parameter } */

   config_spr_render_hold_duration_t render_hold_buffer_size;
   /**< @h2xmle_description {Render hold buffer size } */
}
#include "spf_end_pack.h"
;

typedef struct param_id_spr_avsync_config_t param_id_spr_avsync_config_t;


/** @h2xmlp_subStruct
    @h2xmlp_description  { This structure defines each of the control port to output data port mappings.
                           Each output port ID must be linked to only one control port ID. }*/
#include "spf_begin_pack.h"
struct spr_port_binding_t
{
   uint32_t                control_port_id;
   /**< @h2xmle_description { Control port ID of the module.}
        @h2xmle_default     { 0} */

   uint32_t                output_port_id;
   /**< @h2xmle_description { The output port ID to which control port is linked to. }
        @h2xmle_default     { 0} */
}
#include "spf_end_pack.h"
;

typedef struct spr_port_binding_t spr_port_binding_t;

#define PARAM_ID_SPR_CTRL_TO_DATA_PORT_MAP 0x0800113A

/** @h2xmlp_parameter   {"PARAM_ID_SPR_CTRL_TO_DATA_PORT_MAP",
                          PARAM_ID_SPR_CTRL_TO_DATA_PORT_MAP}
    @h2xmlp_description { This param ID defines the control ID to data port bindings. The control ports in the
                          SPR module that are connected to the EP, must be coupled to an output port ID it needs to be associated with.
                          Each of these control ports can be coupled to only one output port ID, hence number of bindings is same as
                          number of output ports.}
    @h2xmlp_toolPolicy  {Calibration} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_spr_ctrl_to_data_port_map_t
{
   uint32_t               num_ctrl_ports;
   /**< @h2xmle_description { Number of control port IDs attached to the module.}
        @h2xmle_default     {0} */
#if defined(__H2XML__)
   spr_port_binding_t        ctrl_to_outport_map[0];
   /**< @h2xmle_description { Control port to Data port map. }
        @h2xmle_variableArraySize  { num_ctrl_ports } */
#endif
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

typedef struct param_id_spr_ctrl_to_data_port_map_t param_id_spr_ctrl_to_data_port_map_t;

/* Indicates that the underrun occurred due to input not being available */
#define UNDERRUN_STATUS_INPUT_NOT_AVAILABLE      0x1

/* Indicates that the underrun occurred due to input being held after the render decision (AVSync) */
#define UNDERRUN_STATUS_INPUT_HOLD               0x2

/* Indicates that the underrun occurred due to input being dropped after the render decision (AVSync) */
#define UNDERRUN_STATUS_INPUT_DROP               0x3

/* GUID for the underrun event. */
#define EVENT_ID_SPR_UNDERRUN                   0x080011E6

/** @h2xmlp_parameter   {"EVENT_ID_SPR_UNDERRUN",
                          EVENT_ID_SPR_UNDERRUN}
    @h2xmlp_description { Underrun event raised by the SPR module. Upto 1 client can register to
                            listen to this event. }
    @h2xmlp_toolPolicy  { NO_SUPPORT } */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct event_id_spr_underrun_t
{
   uint32_t    status;
   /**< @h2xmle_description { Status of the underrun event being raised by the SPR module }
        @h2xmle_range     {1..3} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/* Payload structure returned during EVENT_ID_SPR_UNDERRUN */
typedef struct event_id_spr_underrun_t event_id_spr_underrun_t;

/**
 * Module ID of the Splitter Renderer Module.
 *
 * Until input media format is available, the output media format
 * will be arbitrarily decided.
 *
 *
 * Supported Input Media Format:
 *  - Data Format          : FIXED_POINT
 *  - fmt_id               : MEDIA_FMT_ID_PCM
 *  - Sample Rates         : any
 *  - Number of channels   : any
 *  - Bit Width            : any
 *  - Interleaving         : deinterleaved unpacked
 *  - Endianess            : little
 */
#define MODULE_ID_SPR 0x07001032

/**
    @h2xmlm_module         {"MODULE_ID_SPR", MODULE_ID_SPR}
    @h2xmlm_displayName    {"Splitter Renderer"}
    @h2xmlm_description    {
                            This module is used to split an audio stream into multiple devices.
                            It's used for AV Sync and rendering decision as well.
                            This module is meant to be used in audio playback only.
                            This module is supported only in container whose position is APM_CONT_GRAPH_POS_STREAM.
                            }
    @h2xmlm_dataMaxInputPorts    { 1 }
    @h2xmlm_dataMaxOutputPorts   { -1 }
    @h2xmlm_toolPolicy     {Calibration}
    @h2xmlm_ctrlDynamicPortIntent  { "Drift Info IMCL"=INTENT_ID_TIMER_DRIFT_INFO, maxPorts= -1 }
    @h2xmlm_supportedContTypes  { APM_CONTAINER_TYPE_GC }
    @h2xmlm_isOffloadable        {false}
    @h2xmlm_stackSize            { 1024 }

    @{                     <-- Start of the Module -->
    @h2xml_Select          {param_id_spr_delay_path_end_t}
    @h2xmlm_InsertParameter
    @h2xml_Select          {param_id_spr_session_time_t}
    @h2xmlm_InsertParameter
    @h2xml_Select          {param_id_spr_avsync_config_t}
    @h2xmlm_InsertParameter
    @h2xml_Select        {param_id_spr_avsync_config_t::render_window::render_window_start}
    @h2xmle_defaultList  {0x00000000,0x80000000}
    @h2xml_Select        {param_id_spr_avsync_config_t::render_window::render_window_end}
    @h2xmle_defaultList  {0xFFFFFFFF,0x7FFFFFFF}
    @h2xml_Select          {param_id_spr_ctrl_to_data_port_map_t}
    @h2xmlm_InsertParameter
    @h2xml_Select          {time_us_t}
    @h2xmlm_InsertStructure
    @h2xml_Select          {config_spr_render_mode_t}
    @h2xmlm_InsertStructure
    @h2xml_Select          {config_spr_render_window_t}
    @h2xmlm_InsertStructure
    @h2xml_Select          {config_spr_render_reference_t}
    @h2xmlm_InsertStructure
    @h2xml_Select          {config_spr_render_hold_duration_t}
    @h2xmlm_InsertStructure
    @h2xml_Select          {spr_port_binding_t}
    @h2xmlm_InsertStructure
    @}                     <-- End of the Module -->
*/

#endif /* _SPR_BUFFER_API_H_ */

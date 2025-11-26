/*========================================================================

*//** @file hyp_vinput_types.h

@par FILE SERVICES:
      Hypervisor video input common types definition
      This file contains data types, constants, and methods that make up the
      internal hypervisor video input Driver API.

@par EXTERNALIZED FUNCTIONS:
      See below.

Copyright (c) 2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---     -------------------------------------------------------
05/08/17   sm     Update for new hyp-video architecture
02/22/17   rz      create file

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#ifndef __HYP_VINPUT_TYPES_H__
#define __HYP_VINPUT_TYPES_H__

#ifdef __QNXNTO__
#include "AEEstd.h"
#endif
#include "hyp_video.h"

/*===========================================================================

                      DEFINITIONS AND DECLARATIONS

===========================================================================*/

/* -----------------------------------------------------------------------
** Constant and Macros
** ----------------------------------------------------------------------- */

// input port connection type
#define     AVIN_PORT_TYPE_ANALOG  ( 0x1 << 0 )           // analog input port
#define     AVIN_PORT_TYPE_HDMI    ( 0x1 << 1 )           // HDMI receiver port
#define     AVIN_PORT_TYPE_MHL     ( 0x1 << 2 )           // MHL receiver port
#define     AVIN_PORT_TYPE_TTL     ( 0x1 << 3 )           // digital video input port


// input port analog connect type
#define     AVIN_ANALOG_CVBS_SINGLE_ENDED             ( 0x1 << 0 )
#define     AVIN_ANALOG_CVBS_PSEDUO_DIFFERENTIAL      ( 0x1 << 1 )
#define     AVIN_ANALOG_CVBS_FULLY_DIFFERENTIAL       ( 0x1 << 2 )
#define     AVIN_ANALOG_Y_C                           ( 0x1 << 3 )
#define     AVIN_ANALOG_Y_PB_PR                       ( 0x1 << 4 )
#define     AVIN_ANALOG_COMPONENT_RGB                 ( 0x1 << 5 )

// input port status
#define     AVIN_PORT_CONNECTED     ( 0x1 << 0 )
#define     AVIN_PORT_SELECED       ( 0x1 << 1 )
#define     AVIN_PORT_STREAM_ON     ( 0x1 << 2 )

/* -----------------------------------------------------------------------
** Variables
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Typedefs
** ----------------------------------------------------------------------- */
typedef void * avin_hdl_type;

typedef void * avin_crit_sect_type;

typedef enum
{
   AVIN_COLORFMT_CBYCRY_PACKED_16BITS   = 0x1,
   AVIN_COLORFMT_RGB565_16BITS,
   AVIN_COLORFMT_RGB888_24BITS,
   AVIN_COLORFMT_MAX                    = 0x7fffffff
} avin_video_color_format_type;

/* vinput color coding type */
typedef enum
{
   AVIN_COLOR_NTSC  = 0x1,
   AVIN_COLOR_PAL,
   AVIN_COLOR_SECAM,
   AVIN_COLOR_MAX    = 0x7fffffff
} avin_video_color_coding_type;

/* vinput port priority type */
typedef enum
{
   AVIN_PORT_PRIORITY_LOW   = 0,        // default
   AVIN_PORT_PRIORITY_MEDIUM,
   AVIN_PORT_PRIORITY_HIGH,
} avin_port_priority_type;

/*vinput video interlaced type */
typedef enum
{
   AVIN_VIDEO_PROGRESSIVE  = 0,
   AVIN_VIDEO_INTERLEAVED_TOP_FIELD_FIRST,
   AVIN_VIDEO_INTERLEAVED_BOTTOM_FIELD_FIRST,
   AVIN_VIDEO_TOP_FIELD_FIRST,
   AVIN_VIDEO_BOTTOM_FIELD_FIRST,
} avin_video_interlaced_type;

/* vinput property type for get/set API */
typedef enum
{
   AVIN_PROP_NUMBER_PORTS     = 1,           // get
   AVIN_PROP_PORT_STATUS,                     // get
   AVIN_PROP_PORT_CONNECT_TYPE,               // get
   AVIN_PROP_PORT_ANALOG_TYPE,                // get
   AVIN_PROP_PORT_PRIORITY,                   // set
   AVIN_PROP_PORT_ANALOG_COLOR_CODING,        // get
   AVIN_PROP_PORT_RESOLUTION,                 // get/set
   AVIN_PROP_PORT_COLORFMT,                   // get/set
   AVIN_PROP_PORT_INTERLACEFMT,               // get/set
   AVIN_PROP_PORT_FRAMERATE                   // get/set
} avin_property_id_type;

/* vinput event types that clients can register for notification */
typedef enum
{
   AVIN_EVT_PORT_CONNECTED = 0x1,      // both condition needs to be met:  1. port selected 2. Lock signal (cable connected) AND signal detected (HDMI)
   AVIN_EVT_PORT_DISCONNECTED,         // 1.  client call close + cable stay connected:  NO event, close return success
                                       // 2.  client didn't call close + cable disconnectd:  send event, client should call close
   AVIN_EVT_PORT_SETTINGS_CHANGED,
   AVIN_EVT_CABLE_DETECT,
} avin_event_type;

typedef struct
{
   uint32 numerator;
   uint32 denomerator;
} avin_frac_number_type;

/* vinput resolution type */
typedef struct
{
   uint32 width;
   uint32 height;
} avin_resolution_type;

/* vinput property header type for set/get API */
typedef struct
{
   avin_property_id_type  prop_id;
   uint32                 port;
   uint32                 size;
} avin_property_hdr_type;

/* vinput port status query type */
typedef struct
{
   uint32                  status;
} avin_property_port_status_type;

/* vinput port priority type */
typedef struct
{
   avin_port_priority_type  priority;
} avin_property_port_priority_type;

/* vinput port connect type */
typedef struct
{
   uint32                  connect_type;
} avin_property_port_connect_type;

/* vinput port resolution type */
typedef struct
{
   avin_resolution_type    res;
} avin_property_port_resolution_type;

/* vinput port color format type */
typedef struct
{
   avin_video_color_format_type  clr_fmt;
} avin_property_port_clrfmt_type;

/* vinput port color coding type */
typedef struct
{
   avin_video_color_coding_type  coding;
} avin_property_port_clrcoding_type;

/* vinput port interlace format tyoe */
typedef struct
{
   avin_video_interlaced_type    interlace_fmt;
} avin_property_port_interlacefmt_type;

/* vinput port framerate type */
typedef struct
{
   avin_frac_number_type         frame_rate;
} avin_property_port_framerate_type;

/* vinput callback function prototype */
typedef void ( *avin_callback_type )( avin_event_type    event,
                                      hypv_status_type   status,
                                      void*              info,
                                      uint32             size,
                                      avin_hdl_type      handle,
                                      void* const        client_data );

#endif /* __HYP_VINPUT_TYPES_H__ */

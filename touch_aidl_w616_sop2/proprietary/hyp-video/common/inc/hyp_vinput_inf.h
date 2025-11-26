/*========================================================================

            Hypervisor video input Interface

*//** @file hyp_vinput_inf.h
   This file defines hypervisor video input interface.

Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---    --------------------------------------------------------
12/13/17   sm     Add payload in msg event
05/08/17   sm     Update for new hyp-video architecture
03/17/17   rz     Created file.


 ========================================================================== */

/*========================================================================

                     INCLUDE FILES FOR MODULE

==========================================================================*/
#ifndef __HYP_VINPUT_INF_H__
#define __HYP_VINPUT_INF_H__

#include "hyp_vinput_types.h"

/* -----------------------------------------------------------------------
**
** CONSTANTS, MACROS & TYPEDEFS
**
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
**
** IOCTL DEFINES
**
** ----------------------------------------------------------------------- */

#define AVIN_IOCTL_BASE                0x0
#define AVIN_IOCTL_GET_PROPERTY        (AVIN_IOCTL_BASE + 1)
#define AVIN_IOCTL_SET_PROPERTY        (AVIN_IOCTL_BASE + 2)
#define AVIN_IOCTL_CONNECT             (AVIN_IOCTL_BASE + 3)
#define AVIN_IOCTL_START               (AVIN_IOCTL_BASE + 4)
#define AVIN_IOCTL_PAUSE               (AVIN_IOCTL_BASE + 5)
#define AVIN_IOCTL_RESUME              (AVIN_IOCTL_BASE + 6)
#define AVIN_IOCTL_STOP                (AVIN_IOCTL_BASE + 7)

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   msg_payload_t
----------------------------------------------------------------------------*/
/**
 * message payload
 */
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   avin_drv_msg_info_type_t
----------------------------------------------------------------------------*/
/**
 * Response message type
 */


typedef struct ba_drv_msg_info_type
{
   uint32   port_id;
} ba_drv_msg_info_type;

typedef struct avin_drv_msg_type
{
   avin_event_type         event;
   hypv_status_type        status;
   ba_drv_msg_info_type    info;
}avin_drv_msg_type;


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   AVIN_drv_property_type_t
----------------------------------------------------------------------------*/
/**
 * A wrapper structure to encapulate avin_property_hdr_type and property id
 * associated structure.
 */
typedef struct avin_drv_property_type
{
   avin_property_hdr_type  prop_hdr;
   uint8                   prop_data[1];
}avin_drv_property_type;


#endif /* __HYP_VINPUT_INF_H__ */

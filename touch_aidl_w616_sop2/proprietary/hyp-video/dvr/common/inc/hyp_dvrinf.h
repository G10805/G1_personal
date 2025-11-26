/*===========================================================================

*//** @file hyp_dvrinf.h
  This file declares datatypes and prototypes for dvr hypervisor

Copyright (c) 2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/


/*===========================================================================
                             Edit History

$Header: $

when        who        what, where, why
--------   --------    -------------------------------------------------------
08/22/18    sm          Initial version of hypervisor DVR
============================================================================*/
#ifndef __HYP_DVRINF_H__
#define __HYP_DVRINF_H__

#include "hyp_dvr.h"

#define MAX_HAB_RECV_RETRY          10
#define MAX_IOCTL_DATA_PAYLOAD_SIZE 4096

typedef struct hypdvr_ioctl_data
{
    hdvr_status_type return_value;
    int64            io_handle;
    uint32           dvr_ioctl;
    uint8            payload[MAX_IOCTL_DATA_PAYLOAD_SIZE];
} hypdvr_ioctl_data_type;

typedef struct hypdvr_close_data
{
    hdvr_status_type return_status;
    int64            io_handle;
} hypdvr_close_data_type;

typedef enum dvr_session
{
    SESSION_UNKNOWN = 0,
    SESSION_MUX     = 1,
} hypdvr_session_type;

typedef struct hypdvr_open_data
{
    hdvr_status_type    return_status;
    int64               return_io_handle;
    hypdvr_session_type drv_session;
} hypdvr_open_data_type;

typedef struct hypdvr_msg_data
{
    /* HYPDVR_MSGCMD_OPEN, HYPDVR_MSGRESP_OPEN_RET */
    hypdvr_open_data_type     open_data;

    /* HYPDVR_MSGCMD_IOCTL, HYPDVR_MSGRESP_IOCTL_RET */
    hypdvr_ioctl_data_type    ioctl_data;

    /* HYPDVR_MSGCMD_CLOSE, HYPDVR_MSGRESP_CLOSE_RET */
    hypdvr_close_data_type    close_data;
} hypdvr_msg_data_type;

typedef enum
{
    HYPDVR_MSGCMD_OPEN       = 0x00000001,
    HYPDVR_MSGCMD_IOCTL      = 0x00000002,
    HYPDVR_MSGCMD_CLOSE      = 0x00000003,
    HYPDVR_MSGRESP_OPEN_RET  = 0x80008001,
    HYPDVR_MSGRESP_IOCTL_RET = 0x80008002,
    HYPDVR_MSGRESP_CLOSE_RET = 0x80000003
} hypdvr_msg_id_type;

/***************************************************************************
 *
 * hypdvr_msg_type
 *
 * Define the dvr command message, respond, and event messages
 *
 ***************************************************************************
 */
typedef struct
{
    uint32                    version;

    /* the virtual_channle is the abstract handle returned from HAB open */
    uint32                    virtual_channel;

    /* identify different message */
    uint32                    msg_id;

    /* message number in sequence for debug purpose */
    uint32                    message_number;

    /* the send and receive time in ns for debug purpose */
    uint32                    time_stamp_ns;

    /* the size of supporting data in unit of bytes */
    uint32                    data_size;

    /* the payload corresponding to the token */
    hypdvr_msg_data_type      data;

    /* pid of the client */
    uint32                    pid;

} hypdvr_msg_type;

#define SIZE_OF_IOCTL_DATA_HEADER_IN_MSG (sizeof(hdvr_status_type)+sizeof(int64)+sizeof(uint32))
#define GET_MSG_DATASIZE_FROM_IOCTL_PAYLOAD_SIZE(ps) (ps + SIZE_OF_IOCTL_DATA_HEADER_IN_MSG)
#define GET_IOCTL_PAYLOAD_SIZE_FROM_MSG_DATASIZE(mds) (mds-SIZE_OF_IOCTL_DATA_HEADER_IN_MSG)

#endif /* __HYP_DVRINF_H__ */

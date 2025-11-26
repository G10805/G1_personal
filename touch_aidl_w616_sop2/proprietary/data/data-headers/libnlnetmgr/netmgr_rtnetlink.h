/******************************************************************************

                    N E T M G R _ R T N E T L I N K . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_rtnetlink.h
  @brief   Network Manager Route netlink header file

  DESCRIPTION
  Header file for route netlink module.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2018, 2020-2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#define NETMGR_FLOW_ENABLE 1
#define NETMGR_FLOW_DISABLE 0

#include "netmgr.h"
#include "netmgr_kif.h"

typedef enum netmgr_nl_requests_e
{
  NETMGR_ADD_NETLINK,
  NETMGR_DEL_NETLINK,
  NETMGR_CHANGE_NETLINK,
  NETMGR_REPLACE_NETLINK
} netmgr_nl_requests_t;

#ifndef FEATURE_DS_LINUX_AUTO_IVI
/*===========================================================================
  FUNCTION  netmgr_rtnetlink_flow_control
===========================================================================*/
/*!
@brief
  Sends a netlink route message to kernel to setup a tc qdisc prio flow

@return
  NETMGR_SUCCESS on success
  NETMGR_FAILURE on failure

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rtnetlink_flow_control
(
  const char* devname,
  uint32_t parent,
  uint32_t handle,
  uint8_t enable,
  uint8_t flags
);

/*==========================================================================
  FUNCTION  netmgr_rtnetlink_tc_qdisc
===========================================================================*/
/*!
@brief
  Sends a netlink route message to kernel to setup a tc qdisc

@return
  NETMGR_SUCCESS on success
  NETMGR_FAILURE on failure

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rtnetlink_tc_qdisc
(
  const char* devname,
  const char *kind,
  uint32_t parent,
  uint32_t handle,
  uint8_t flags
);

/*==========================================================================
  FUNCTION  netmgr_rtnetlink_tc_prio
===========================================================================*/
/*!
@brief
  Sends a netlink route message to kernel to setup a tc prio qdisc

@return
  NETMGR_SUCCESS on success
  NETMGR_FAILURE on failure

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rtnetlink_tc_prio
(
  const char* devname,
  uint32_t parent,
  uint32_t handle,
  uint8_t flags
);

int
netmgr_rtnetlink_addr_cfg
(
  char *ifname,
  netmgr_ip_address_t *addr_ptr,
  unsigned int prefix_len,
  netmgr_kif_lft_change_t lft_change,
  unsigned int lifetime
);

int
netmgr_rtnetlink_gso_max_size
(
  const char* devname,
  uint16_t gso_max_size
);

int
netmgr_rtnetlink_add_ip_rule
(
  uint32_t output_mark,
  uint32_t output_mask,
  uint32_t table,
  uint32_t priority
);

int
netmgr_rtnetlink_del_ip_rule
(
  uint32_t output_mark,
  uint32_t output_mask,
  uint32_t table,
  uint32_t priority
);

int
netmgr_rtnetlink_del_ip_rule_by_prio
(
  uint32_t priority
);
#else /* FEATURE_DS_LINUX_AUTO_IVI */
static int
netmgr_rtnetlink_flow_control
(
  const char* devname,
  uint32_t parent,
  uint32_t handle,
  uint8_t enable,
  uint8_t flags
)
{
    return 0;
}

static int
netmgr_rtnetlink_tc_qdisc
(
  const char* devname,
  const char *kind,
  uint32_t parent,
  uint32_t handle,
  uint8_t flags
)
{
    return 0;
}

static int
netmgr_rtnetlink_tc_prio
(
  const char* devname,
  uint32_t parent,
  uint32_t handle,
  uint8_t flags
)
{
    return 0;
}

static int
netmgr_rtnetlink_addr_cfg
(
  char *ifname,
  netmgr_ip_address_t *addr_ptr,
  unsigned int prefix_len,
  netmgr_kif_lft_change_t lft_change,
  unsigned int lifetime
)
{
    return 0;
}

static int
netmgr_rtnetlink_gso_max_size
(
  const char* devname,
  uint16_t gso_max_size
)
{
    return 0;
}
#endif /* FEATURE_DS_LINUX_AUTO_IVI */

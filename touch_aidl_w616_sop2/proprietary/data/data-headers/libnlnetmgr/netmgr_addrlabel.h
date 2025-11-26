/******************************************************************************

                    N E T M G R _ A D D R L A B E L . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_addrlabel.h
  @brief   Network manager Address Label Netlink Module

  DESCRIPTION
  Header file for Address Label netlink module.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef __NETMGR_ADDRLABEL_H__
#define __NETMGR_ADDRLABEL_H__

#include <stdint.h>

#include "netmgr_defs.h"

#ifndef FEATURE_DS_LINUX_AUTO_IVI
int netmgr_addrlabel_add
(
  netmgr_ip_address_t *addr_ptr,
  unsigned int prefix_len,
  uint32_t label,
  unsigned int ifindex
);

int netmgr_addrlabel_del
(
  netmgr_ip_address_t *addr_ptr,
  unsigned int prefix_len,
  uint32_t label,
  unsigned int ifindex
);

int netmgr_addrlabel_generate
(
  netmgr_ip_addr_t addr_type,
  boolean default_label,
  uint32_t *label
);

int netmgr_addrlabel_purge_label
(
  netmgr_ip_address_t *addr,
  unsigned int prefix_len,
  unsigned int ifindex
);
#else /* FEATURE_DS_LINUX_AUTO_IVI */
static int netmgr_addrlabel_add
(
  netmgr_ip_address_t *addr_ptr,
  unsigned int prefix_len,
  uint32_t label,
  unsigned int ifindex
)
{
    return 0;
}

static int netmgr_addrlabel_del
(
  netmgr_ip_address_t *addr_ptr,
  unsigned int prefix_len,
  uint32_t label,
  unsigned int ifindex
)
{
    return 0;
}

static int netmgr_addrlabel_generate
(
  netmgr_ip_addr_t addr_type,
  boolean default_label,
  uint32_t *label
)
{
    return 0;
}

static int netmgr_addrlabel_purge_label
(
  netmgr_ip_address_t *addr,
  unsigned int prefix_len,
  unsigned int ifindex
)
{
   return 0;
}

#endif /* FEATURE_DS_LINUX_AUTO_IVI */
#endif /* __NETMGR_ADDRLABEL_H__*/

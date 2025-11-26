/******************************************************************************

               N E T M G R _ D A T A P A T H _ P R O X Y . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_datapath_proxy.h
  @brief   Network Manager datapath proxy module header file

  DESCRIPTION
  Header file for datapath proxy module.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef __NETMGR_DATAPATH_PROXY_H__
#define __NETMGR_DATAPATH_PROXY_H__

#include "netmgr_defs.h"
#include "data_filter_service_v01.h"

#ifndef FEATURE_DS_LINUX_AUTO_IVI
int netmgr_datapath_proxy_set_config_info
(
  char *wlan_device,
  char *reverse_device,
  char *reverse_address,
  dfs_request_socket_info_type_v01 *ports,
  uint32_t port_count,
  uint8_t network_type
);

int netmgr_datapath_proxy_unset_config_info
(
  char *wlan_device,
  char *reverse_device,
  char *reverse_address,
  dfs_request_socket_info_type_v01 *ports,
  uint32_t port_count,
  uint8_t network_type
);

int netmgr_datapath_proxy_update_config_info
(
  dfs_request_socket_info_type_v01 *add_ports,
  uint32_t add_port_count,
  dfs_request_socket_info_type_v01 *delete_ports,
  uint32_t delete_ports_count
);

int netmgr_datapath_proxy_add_ipsec_info
(
  dfs_ipsec_info_type_v01 *ipsec_info,
  char *reverse_address
);

int netmgr_datapath_proxy_delete_ipsec_info
(
  dfs_ipsec_info_type_v01 *ipsec_info,
  char *reverse_address
);

int netmgr_datapath_proxy_set_udp_encap_port
(
  uint16_t udp_encap_port
);

int netmgr_datapath_proxy_unset_udp_encap_port
(
  uint16_t udp_encap_port
);

int netmgr_datapath_proxy_flush_config_info
(
  void
);

int netmgr_datapath_proxy_unset_reverse_dev_mapping
(
  uint8_t network_type,
  char *reverse_device,
  char *reverse_address
);

int netmgr_datapath_proxy_act_encap_port_pass_through
(
  uint16_t udp_encap_port
);

int netmgr_datapath_proxy_act_encap_port_drop
(
  uint16_t udp_encap_port
);

int netmgr_datapath_proxy_add_ll_dst_addr
(
  char *source_addr,
  char *dest_addr,
  uint16_t source_port,
  uint16_t dest_port
);

int netmgr_datapath_proxy_flush_ll_dst_addr
(
  void
);

int netmgr_datapath_proxy_get_port_tuple
(
  void
);

#else /* FEATURE_DS_LINUX_AUTO_IVI */
static int netmgr_datapath_proxy_set_config_info
(
  char *wlan_device,
  char *reverse_device,
  char *reverse_address,
  dfs_request_socket_info_type_v01 *ports,
  uint32_t port_count
)
{
    return 0;
}

static int netmgr_datapath_proxy_unset_config_info
(
  char *wlan_device,
  char *reverse_device,
  char *reverse_address,
  dfs_request_socket_info_type_v01 *ports,
  uint32_t port_count
)
{
    return 0;
}

static int netmgr_datapath_proxy_update_config_info
(
  dfs_request_socket_info_type_v01 *add_ports,
  uint32_t add_port_count,
  dfs_request_socket_info_type_v01 *delete_ports,
  uint32_t delete_ports_count
)
{
    return 0;
}

static int netmgr_datapath_proxy_add_ipsec_info
(
  dfs_ipsec_info_type_v01 *ipsec_info,
  char *reverse_address
)
{
    return 0;
}
static int netmgr_datapath_proxy_delete_ipsec_info
(
  dfs_ipsec_info_type_v01 *ipsec_info,
  char *reverse_address
)
{
    return 0;
}

static int netmgr_datapath_proxy_set_udp_encap_port
(
  uint16_t udp_encap_port
)
{
    return 0;
}

static int netmgr_datapath_proxy_unset_udp_encap_port
(
  uint16_t udp_encap_port
)
{
    return 0;
}

static int netmgr_datapath_proxy_flush_config_info
(
  void
)
{
    return 0;
}

static int netmgr_datapath_proxy_unset_reverse_dev_mapping
(
  char *reverse_device,
  char *reverse_address
)
{
    return 0;
}

static int netmgr_datapath_proxy_act_encap_port_pass_through
(
  uint16_t udp_encap_port
)
{
    return 0;
}

static int netmgr_datapath_proxy_act_encap_port_drop
(
  uint16_t udp_encap_port
)
{
    return 0;
}
int netmgr_datapath_proxy_add_ll_dst_addr
(
  char *source_addr,
  char *dest_addr,
  uint16_t source_port,
  uint16_t dest_port
)
{
    return 0;
}

int netmgr_datapath_proxy_flush_ll_dst_addr
(
  void
)
{
    return 0;
}

static int netmgr_datapath_proxy_get_port_tuple
(
  void
)
{
    return 0;
}

#endif /* FEATURE_DS_LINUX_AUTO_IVI */
#endif /* __NETMGR_DATAPATH_PROXY_H__ */

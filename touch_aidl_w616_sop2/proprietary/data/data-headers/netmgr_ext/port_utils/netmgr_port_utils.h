/******************************************************************************

                  N E T M G R _ P O R T _ U T I L S . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_port_utils.c
  @brief   Network manager port utilities header file

  DESCRIPTION
  Exposed header of port utilities.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#include "data_filter_service_v01.h"

#ifndef FEATURE_DS_LINUX_AUTO_IVI
/* request & allocated must be allocated by caller and must not be NULL.*/
int netmgr_port_utils_reserve_ports
(
  dfs_remote_socket_request_ind_msg_v01 *request,
  dfs_remote_socket_allocated_req_msg_v01 *allocated
);

/* indication must allocated by caller and must not be NULL.*/
void netmgr_port_utils_release_ports
(
  dfs_remote_socket_release_ind_msg_v01 *indication
);

/* sockets will be allocated by netmgr_port_utils_get_all_dfs_sockets()
 * sockets must be freed by caller
 * count must be allocated by caller and must not be NULL */
void netmgr_port_utils_get_all_dfs_sockets
(
  dfs_request_socket_info_type_v01 **sockets,
  int *count
);

void netmgr_port_utils_release_all
(
  void
);

void netmgr_port_utils_reserve_specific_port
(
  uint16_t port
);

void netmgr_port_tree_node_delete_specific_port
(
  uint16_t port
);

int netmgr_port_get_port_utils_from_handle
(
  uint32_t handle
);

void netmgr_port_utils_remove_handle
(
  uint32_t handle
);

int netmgr_port_utils_delete_udp_encap_by_handle
(
  uint32_t handle
);

int netmgr_port_utils_add_udp_encap_by_port
(
  int ip_family,
  uint16_t source_port,
  char *source_addr,
  char *dest_addr,
  uint8_t link
);

int netmgr_port_utils_get_fd_by_encap_port
(
  uint16_t source_port
);

int netmgr_port_utils_add_port_from_kernel
(
  uint16_t port,
  uint8_t ip_proto,
  uint8_t trans_proto
);

int netmgr_port_utils_cast_handle_to_dfs_request_socket
(
  dfs_request_socket_info_type_v01 *dfs_request_socket,
  uint32_t handle
);
#else /* FEATURE_DS_LINUX_AUTO_IVI */
/* request & allocated must be allocated by caller and must not be NULL.*/
static int netmgr_port_utils_reserve_ports
(
  dfs_remote_socket_request_ind_msg_v01 *request,
  dfs_remote_socket_allocated_req_msg_v01 *allocated
)
{
    return 0;
}

/* indication must allocated by caller and must not be NULL.*/
static void netmgr_port_utils_release_ports
(
  dfs_remote_socket_release_ind_msg_v01 *indication
)
{
}

/* sockets will be allocated by netmgr_port_utils_get_all_dfs_sockets()
 * sockets must be freed by caller
 * count must be allocated by caller and must not be NULL */
static void netmgr_port_utils_get_all_dfs_sockets
(
  dfs_request_socket_info_type_v01 **sockets,
  int *count
)
{
}

static void netmgr_port_utils_release_all
(
  void
)
{
}

static void netmgr_port_utils_reserve_specific_port
(
  uint16_t port
)
{
}

static void netmgr_port_tree_node_delete_specific_port
(
  uint16_t port
)
{
}

static int netmgr_port_get_port_utils_from_handle
(
  uint32_t handle
)
{
    return 0;
}

static void netmgr_port_utils_remove_handle
(
  uint32_t handle
)
{
}

static int netmgr_port_utils_delete_udp_encap_by_handle
(
  uint32_t handle
)
{
    return 0;
}

static int netmgr_port_utils_add_udp_encap_by_port
(
  int ip_family,
  uint16_t source_port,
  char *source_addr,
  char *dest_addr,
  uint8_t link
)
{
    return 0;
}

static int netmgr_port_utils_get_fd_by_encap_port
(
  uint16_t source_port
)
{
    return 0;
}

static int netmgr_port_utils_cast_handle_to_dfs_request_socket
(
  dfs_request_socket_info_type_v01 *dfs_request_socket,
  uint32_t handle
)
{
     return 0;
}

static int netmgr_port_utils_add_port_from_kernel
(
  uint16_t port,
  uint8_t ip_proto,
  uint8_t trans_proto
)
{
    return 0;
}

#endif /* FEATURE_DS_LINUX_AUTO_IVI */

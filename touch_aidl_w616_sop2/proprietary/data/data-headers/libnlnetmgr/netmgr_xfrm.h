/******************************************************************************

                        N E T M G R _ X F R M . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_xfrm.h
  @brief   Network Manager XFRM Netlink module header file

  DESCRIPTION
  Header file for XFRM netlink module.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef __NETMGR_XFRM_H__
#define __NETMGR_XFRM_H__

#include "netmgr_defs.h"
#include "wireless_data_service_v01.h"

#ifndef FEATURE_DS_LINUX_AUTO_IVI

int netmgr_xfrm_add_policies_from_addr_info
(
  netmgr_address_info_t *addr_info_ptr,
  int ip_family
);

int netmgr_xfrm_add_state_from_addr_set
(
  netmgr_address_set_t  *addr_set,
  bool is_tx,
  wds_technology_name_enum_v01 tech_name
);

int netmgr_xfrm_add_states
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count,
  wds_technology_name_enum_v01 tech_name
);

int netmgr_xfrm_remove_states
(
  netmgr_xfrm_state_id_t *state
);

int netmgr_xfrm_add_policies
(
  netmgr_xfrm_state_id_t *sa_config,
  char *policy_saddr,
  char *policy_daddr,
  char *policy_proto,
  int is_tx
);

int netmgr_xfrm_remove_policies
(
  char *policy_saddr,
  char *policy_daddr,
  char *policy_proto,
  int is_tx
);

int netmgr_xfrm_add_policy_ex
(
  uint8_t dir
);

int netmgr_xfrm_remove_policy_ex
(
  uint8_t dir
);

#else /* FEATURE_DS_LINUX_AUTO_IVI */

static int netmgr_xfrm_add_policies_from_addr_info
(
  netmgr_address_info_t *addr_info_ptr,
  int ip_family
)
{
    return 0;
}

static int netmgr_xfrm_add_state_from_addr_set
(
  netmgr_address_set_t  *addr_set,
  bool is_tx,
  wds_technology_name_enum_v01 tech_name
)
{
    return 0;
}

static int netmgr_xfrm_add_states
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count,
  wds_technology_name_enum_v01 tech_name
)
{
    return 0;
}

static int netmgr_xfrm_remove_states
(
  netmgr_xfrm_state_id_t *state
)
{
    return 0;
}

static int netmgr_xfrm_add_policies
(
  netmgr_xfrm_state_id_t *sa_config,
  char *policy_saddr,
  char *policy_daddr,
  char *policy_proto,
  int is_tx
)
{
    return 0;
}

static int netmgr_xfrm_remove_policies
(
  char *policy_saddr,
  char *policy_daddr,
  char *policy_proto,
  int is_tx
)
{
    return 0;
}

static int netmgr_xfrm_add_policy_ex
(
  uint8_t dir
)
{
    return 0;
}
static int netmgr_xfrm_remove_policy_ex
(
  uint8_t dir
)
{
    return 0;
}
#endif /* FEATURE_DS_LINUX_AUTO_IVI */
#endif /* __NETMGR_XFRM_H__ */

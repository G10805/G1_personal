/******************************************************************************

                          N E T M G R _ S S C M 3 . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_sscm3.h
  @brief   Network Manager SSC mode 3 module header file

  DESCRIPTION
  Header file for SSC mode 3 module.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef __NETMGR_SSCM3_H__
#define __NETMGR_SSCM3_H__

#include "netmgr_defs.h"
#include "netmgr_qmi.h"
#include "netmgr_exec.h"
#include "wireless_data_service_v01.h"

struct netmgr_sscm3_ops
{
  struct netmgr_main_cfg_s    *cfg;
  netmgr_exec_cmd_t *        (*exec_get_cmd)(void);
  void                       (*exec_release_cmd)(netmgr_exec_cmd_t *cmd_buf);
  int                        (*exec_put_cmd)(const netmgr_exec_cmd_t *cmdbuf);
  void                       (*submit_flow_info)(int link,
                                                 netmgr_qmi_qos_flow_info_t * flow_info);
  int                        (*get_modem_link_info)(uint8 link,
                                                  wds_ip_family_enum_v01 evt_ip_family,
                                                  unsigned int pdu_id);
  netmgr_wds_qmi_drv_info_t *(*get_link_wds_info)(int link,
                                                  int pdu_idx);
  netmgr_address_info_t *    (*get_addr_info)(int link,
                                              int pdu_idx);
  netmgr_address_set_t *     (*get_addr_set)(netmgr_address_info_t *addr_info,
                                             netmgr_ip_addr_t addr_family,
                                             netmgr_addr_type addr_type);
};

#ifndef FEATURE_DS_LINUX_AUTO_IVI
void netmgr_sscm3_register
(
  struct netmgr_sscm3_ops *ops
);

void netmgr_sscm3_try_submit_cached_flows
(
  int                   link,
  netmgr_link_info_t   *link_info
);

int netmgr_sscm3_hold_flow_info
(
  netmgr_link_info_t         *link_info,
  netmgr_qmi_qos_flow_info_t *flow_info
);

boolean netmgr_sscm3_maybe_hold_flow_info
(
  netmgr_link_info_t         *link_info,
  netmgr_qmi_qos_flow_info_t *flow_info
);

void netmgr_sscm3_clear_flow_cache
(
  netmgr_link_info_t  *link_info,
  int                  pdu
);

void
netmgr_sscm3_wds_pkt_srvc_connected
(
  int link,
  wds_ip_family_enum_v01 ip_family,
  netmgr_qmi_client_type_t clnt,
  netmgr_link_info_t *link_info,
  netmgr_address_set_t *address_set_ptr,
  netmgr_address_set_t *sec_address_set_ptr,
  const wds_pkt_srvc_status_ind_msg_v01 *ind_data
);

netmgr_nl_event_info_t *
netmgr_sscm3_event_notify
(
  int link,
  const char *link_name,
  wds_ip_family_enum_v01 ip_family,
  int modem_subs_id,
  const wds_pkt_srvc_status_ind_msg_v01 *ind_data
);

void
netmgr_sscm3_wds_pkt_srvc_disconnected_cmd
(
  int link,
  wds_ip_family_enum_v01 ip_family,
  netmgr_wds_qmi_drv_info_t *secondary_wds_info,
  netmgr_exec_cmd_t *cmd
);

int netmgr_sscm3_addr_store
(
  int link,
  const netmgr_nl_msg_t *nlmsg,
  netmgr_address_set_t **addr_ptr
);

int netmgr_sscm3_addr_del
(
  int link,
  const netmgr_nl_msg_t *nlmsg,
  netmgr_address_set_t **addr_ptr,
  boolean *purge_pdu
);

void netmgr_sscm3_link_info_reset
(
  int link,
  unsigned int ifindex
);

void netmgr_sscm3_candidate_promote
(
  int link,
  netmgr_ip_addr_t ip_family
);

void netmgr_sscm3_secondary_promote
(
  int link,
  netmgr_ip_addr_t ip_family
);

#else /* FEATURE_DS_LINUX_AUTO_IVI */
static void netmgr_sscm3_register
(
  struct netmgr_sscm3_ops *ops
)
{
}

static void netmgr_sscm3_try_submit_cached_flows
(
  int                   link,
  netmgr_link_info_t   *link_info
)
{
}

static int netmgr_sscm3_hold_flow_info
(
  netmgr_link_info_t         *link_info,
  netmgr_qmi_qos_flow_info_t *flow_info
)
{
    return 0;
}
static boolean netmgr_sscm3_maybe_hold_flow_info
(
  netmgr_link_info_t         *link_info,
  netmgr_qmi_qos_flow_info_t *flow_info
)
{
    return 0;
}

static void netmgr_sscm3_clear_flow_cache
(
  netmgr_link_info_t  *link_info,
  int                  pdu
)
{
}

static void
netmgr_sscm3_wds_pkt_srvc_connected
(
  int link,
  wds_ip_family_enum_v01 ip_family,
  netmgr_qmi_client_type_t clnt,
  netmgr_link_info_t *link_info,
  netmgr_address_set_t *address_set_ptr,
  netmgr_address_set_t *sec_address_set_ptr,
  const wds_pkt_srvc_status_ind_msg_v01 *ind_data
)
{
}

static netmgr_nl_event_info_t *
netmgr_sscm3_event_notify
(
  int link,
  const char *link_name,
  wds_ip_family_enum_v01 ip_family,
  int modem_subs_id,
  const wds_pkt_srvc_status_ind_msg_v01 *ind_data
)
{
    return NULL;
}

static void
netmgr_sscm3_wds_pkt_srvc_disconnected_cmd
(
  int link,
  wds_ip_family_enum_v01 ip_family,
  netmgr_wds_qmi_drv_info_t *secondary_wds_info,
  netmgr_exec_cmd_t *cmd
)
{
}

static int netmgr_sscm3_addr_store
(
  int link,
  const netmgr_nl_msg_t *nlmsg,
  netmgr_address_set_t **addr_ptr
)
{
    return 0;
}

static int netmgr_sscm3_addr_del
(
  int link,
  const netmgr_nl_msg_t *nlmsg,
  netmgr_address_set_t **addr_ptr,
  boolean *purge_pdu
)
{
    return 0;
}

static void netmgr_sscm3_link_info_reset
(
  int link,
  unsigned int ifindex
)
{
}

static void netmgr_sscm3_candidate_promote
(
  int link,
  netmgr_ip_addr_t ip_family
)
{
}

static void netmgr_sscm3_secondary_promote
(
  int link,
  netmgr_ip_addr_t ip_family
)
{
}
#endif /* FEATURE_DS_LINUX_AUTO_IVI */
#endif /* __NETMGR_SSCM3_H__ */

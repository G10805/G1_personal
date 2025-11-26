/******************************************************************************

                  N E T M G R _ M O D E M _ P R O X Y . H

******************************************************************************/
/*===========================================================================

  Copyright (c) 2015-2018, 2021-2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef _NETMGR_MODEM_PROXY_H_
#define _NETMGR_MODEM_PROXY_H_

#ifndef FEATURE_DS_LINUX_AUTO_IVI
void netmgr_modem_proxy_init(int restart);

int netmgr_modem_proxy_register(void);

int netmgr_modem_proxy_is_initialized(void);

int netmgr_modem_proxy_add_connected_udp_encap
(
  int type_type,
  int ip_family,
  uint16_t source_port,
  char *source_addr,
  char *dest_addr,
  char *ifname
);
#else /* FEATURE_DS_LINUX_AUTO_IVI */
static void netmgr_modem_proxy_init(int restart)
{
}

static int netmgr_modem_proxy_register(void)
{
    return 0;
}

static int netmgr_modem_proxy_is_initialized(void)
{
    return 0;
}

static int netmgr_modem_proxy_add_connected_udp_encap
(
  int type_type,
  int ip_family,
  uint16_t source_port,
  char *source_addr,
  char *dest_addr,
  char *ifname
)
{
    return 0;
}
#endif /* FEATURE_DS_LINUX_AUTO_IVI */
#endif /* _NETMGR_MODEM_PROXY_H_ */

/******************************************************************************

                          N E T M G R _ R M N E T . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_rmnet.h
  @brief   Network Manager RmNet Data configuration

  DESCRIPTION
  Network Manager RmNet Data configuration

******************************************************************************/
/*===========================================================================

  Copyright (c) 2013-2015, 2017-2019, 2021, 2023 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#include <stdlib.h>
#include <stdint.h>
#include <linux/msm_rmnet.h>        /* RmNet Kernel Constants */
#include "netmgr_defs.h"
#include "netmgr_platform.h"
#include "netmgr_util.h"
#include "netmgr_qmi_wda.h"
#include <netmgr_rmnet.h>     /* NetMgr RmNet Constants */
#include "librmnetctl.h"         /* RmNet Configuration Library */
#include <cutils/properties.h>
#include <string.h>
#include <dlfcn.h>
#define NETMGR_INIT_MAX_RETRY 5
#define NETMGR_INIT_BASE_DELAY_US 100000

uint32_t netmgr_rmnet_data_device_qmi_offset;
int netmgr_vnd_last_flags = 0;

pthread_mutex_t netmgr_rmnet_mutex = PTHREAD_MUTEX_INITIALIZER;

/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  netmgr_rmnet_convert_rmnet_edf_to_ioctl_edf
===========================================================================*/
/*!
@brief
  Convert RmNet Data format flags to IOCTL format flags (egress)
*/
/*=========================================================================*/
uint32_t
netmgr_rmnet_convert_rmnet_edf_to_ioctl_edf
(
  uint32_t flags
)
{
  uint32_t flags_ioctl = RMNET_IOCTL_EGRESS_FORMAT_MAP | RMNET_IOCTL_EGRESS_FORMAT_MUXING;

  if ((flags & NETMGR_RMNET_EGRESS_MAP_CKSUMV3_FLAG) ||
      (flags & NETMGR_RMNET_EGRESS_MAP_CKSUMV4_FLAG) ||
      (flags & NETMGR_RMNET_EGRESS_MAP_CKSUMV5_FLAG))
  {
    flags_ioctl |= RMNET_IOCTL_EGRESS_FORMAT_CHECKSUM;
  }

  if (flags & NETMGR_RMNET_EGRESS_AGGREGATION_FLAG)
  {
    flags_ioctl |= RMNET_IOCTL_EGRESS_FORMAT_AGGREGATION;
  }

  return flags_ioctl;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_convert_rmnet_idf_to_ioctl_idf
===========================================================================*/
/*!
@brief
  Convert RmNet Data format flags to IOCTL format flags (ingress)
*/
/*=========================================================================*/
uint32_t
netmgr_rmnet_convert_rmnet_idf_to_ioctl_idf
(
  uint32_t flags
)
{
  uint32_t flags_ioctl = RMNET_IOCTL_INGRESS_FORMAT_MAP | RMNET_IOCTL_INGRESS_FORMAT_DEMUXING;

  if ((flags & NETMGR_RMNET_INGRESS_MAP_CKSUMV4_FLAG) ||
      (flags & NETMGR_RMNET_INGRESS_MAP_CKSUMV5_FLAG))
  {
    flags_ioctl |= RMNET_IOCTL_INGRESS_FORMAT_CHECKSUM;
  }

  if (flags & NETMGR_RMNET_INGRESS_DEAGGREGATION_FLAG)
  {
    flags_ioctl |= RMNET_IOCTL_INGRESS_FORMAT_DEAGGREGATION;
  }

  return flags_ioctl;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_get_device_qmi_offset
===========================================================================*/
/*!
@brief
  Gets the netmgr_rmnet_data_device_qmi_offset.

@arg qmi_offset qmi offset of the rmnet data device

@return
  int - NETMGR_SUCCESS if successful
        NETMGR_FAILURE if the operation fails
*/
/*=========================================================================*/
int
netmgr_rmnet_get_device_qmi_offset
(
  uint32_t          *qmi_offset
)
{
  int result = NETMGR_FAILURE;
  NETMGR_LOG_FUNC_ENTRY;

  if (!qmi_offset)
  {
    netmgr_log_err("%s(): Arguments cannot be NULL!\n", __func__);
    goto error;
  }

  *qmi_offset = netmgr_rmnet_data_device_qmi_offset;
  result = NETMGR_SUCCESS;
error:
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_set_device_qmi_offset
===========================================================================*/
/*!
@brief
  Sets the netmgr_rmnet_data_device_qmi_offset.

@arg qmi_offset qmi offset to be set for the rmnet data device

@return
  void
*/
/*=========================================================================*/
void
netmgr_rmnet_set_device_qmi_offset
(
  uint32_t          qmi_offset
)
{
  netmgr_rmnet_data_device_qmi_offset = qmi_offset;
}


/*===========================================================================
  FUNCTION  netmgr_rmnet_configure_ep_parameters
===========================================================================*/
/*!
@brief
  Configure ep parameters for IPA based targets

@return
  int - NETMGR_SUCCESS always

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rmnet_configure_ep_params()
{
  int i;

  /* This configuration is specific for IPA transport */
  if(0 != strncmp(NETMGR_PHYS_NET_DEV_RMNET_IPA, netmgr_main_cfg.phys_net_dev,
             sizeof(NETMGR_PHYS_NET_DEV_RMNET_IPA)))
  {
    /* Bail for Non IPA targets */
    netmgr_log_med("%s: Not a IPA target. Ignore ep configuration", __func__);
    return NETMGR_SUCCESS;
  }

  for (i = 0; i < NETMGR_PHYS_TXPORT_INIT_RETRIES; i++)
  {
    if (NETMGR_SUCCESS !=
        netmgr_kif_configure_ep_params(netmgr_main_cfg.phys_net_dev))
    {
      netmgr_log_err("%s: Failed to configure ep parameters for [%s]. Sleeping 1s before retry",
                     __func__, netmgr_main_cfg.phys_net_dev);
      sleep(1);
    }
    else
    {
      netmgr_log_med("%s: ep config completed successfully", __func__);
      break;
    }
  }

  if(i == NETMGR_PHYS_TXPORT_INIT_RETRIES)
  {
    netmgr_log_err("%s: unable to configure ep_id for transport [%s]",
                   __func__, netmgr_main_cfg.phys_net_dev);
  }
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_configure_embedded_link
===========================================================================*/
/*!
@brief Creates a single rmnet embedded link with RTM_NETLINK messages to
       underlying rmnet kernel driver using librmnetctl library

@param
  int - link_num - number of the link (ex: rmnet_dataX)

@return
  int - NETMGR_SUCCESS if link was created, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rmnet_configure_embedded_link
(
  int link_num
)
{
  int iwlan_link_num;
  int rc = NETMGR_FAILURE;
  uint16_t err = NETMGR_FAILURE;
  rmnetctl_hndl_t *rmnet_cfg_handle = NULL;
  char vnd_name[NETMGR_IF_NAME_MAX_LEN];
  NETMGR_LOCK_MUTEX(netmgr_rmnet_mutex);

  if (rtrmnet_ctl_init(&rmnet_cfg_handle, &err) < 0)
  {
    netmgr_log_err("%s(): Error on rtrmnet lib init ", __func__);
    NETMGR_UNLOCK_MUTEX(netmgr_rmnet_mutex);
    return rc;
  }

  if (link_num >=0 && link_num < netmgr_main_cfg.rev_link_start)
  {
    snprintf(vnd_name, NETMGR_IF_NAME_MAX_LEN,"%s%d",
             NETMGR_MAIN_RMNET_DATA_PREFIX, link_num);

    if(rtrmnet_ctl_newvnd(rmnet_cfg_handle, netmgr_main_cfg.phys_net_dev,
                          vnd_name , &err, 1 + link_num, 0) < 0)
    {
      netmgr_log_err("%s(): Error on rtrmnet lib init Error code:%d",
                     __func__, err);
      goto bail;
    }
    else
    {
      netmgr_log_med("%s(): rtrmnet created new fwd vnd %d",
                     __func__, link_num);
      rc = NETMGR_SUCCESS;
      goto bail;
    }
  }
  else
  {
    netmgr_log_med("%s(): invalid forward link num %d, maybe iwlan link",
                   __func__, link_num);
  }

#ifdef FEATURE_DATA_IWLAN
  if (link_num >= netmgr_main_cfg.rev_link_start &&
      link_num < netmgr_main_cfg.max_links_in_total)
  {
    iwlan_link_num = link_num - netmgr_main_cfg.rev_link_start;
    snprintf(vnd_name, NETMGR_IF_NAME_MAX_LEN,"%s%d",
             NETMGR_MAIN_REV_RMNET_DATA_PREFIX, iwlan_link_num);

    if(rtrmnet_ctl_newvnd(rmnet_cfg_handle, netmgr_main_cfg.phys_net_dev,
                          vnd_name , &err,
                          netmgr_main_cfg.rev_mux_offset + iwlan_link_num + 1,
                          0) < 0)
    {
      netmgr_log_err("%s(): Error on rtrmnet lib init Error code:%d",
                     __func__, err);
      goto bail;
    }
    else
    {
      netmgr_log_med("%s(): rtrmnet created new iwlan vnd %d",
                     __func__, iwlan_link_num);
      rc = NETMGR_SUCCESS;
      goto bail;
    }
  }
  else
  {
    netmgr_log_med("%s(): invalid iwlan link num %d", __func__, link_num);
  }
#endif /* FEATURE_DATA_IWLAN */

bail:
  rtrmnet_ctl_deinit(rmnet_cfg_handle);

  NETMGR_UNLOCK_MUTEX(netmgr_rmnet_mutex);
  return rc;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_configure_embedded_links
===========================================================================*/
/*!
@brief Creates rmnet embedded links with RTM_NETLINK messages to underlying
       rmnet kernel driver using librmnetctl library


@return
  int - NETMGR_SUCCESS always

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rmnet_configure_embedded_links
(
  void
)
{
  unsigned int mux_offset;
  unsigned int rev_mux_offset;
  unsigned int num_data_ports = netmgr_main_cfg.num_data_ports;
  unsigned int num_rev_data_ports = netmgr_main_cfg.num_rev_data_ports;
  int i;
  uint16_t err = NETMGR_FAILURE;
  netmgr_data_format_t *data_format = &netmgr_main_cfg.data_format;
  rmnetctl_hndl_t *rmnet_cfg_handle = NULL;
  char vnd_name[NETMGR_IF_NAME_MAX_LEN];
  NETMGR_LOCK_MUTEX(netmgr_rmnet_mutex);


  mux_offset = NETMGR_RMNET_START;
  for(i = 0; i < NETMGR_PHYS_TXPORT_INIT_RETRIES &&
                 NETMGR_GET_MODEM_SSR_STATE() != NETMGR_MODEM_OOS_STATE; i++)
  {
    if(NETMGR_SUCCESS !=
         netmgr_kif_init_physical_transport(netmgr_main_cfg.phys_net_dev, data_format))
    {
       netmgr_log_err("%s(): Failed to init physical transport %s. Sleeping 1s before retry\n",
                       __func__,
                       netmgr_main_cfg.phys_net_dev);
       sleep(1);
    }
    else
    {
       netmgr_log_med("%s() Successfully init physical transport %s\n",
                       __func__,
                       netmgr_main_cfg.phys_net_dev);
       break;
    }
  }

  if (rtrmnet_ctl_init(&rmnet_cfg_handle, &err) < 0)
  {
    netmgr_log_err("%s(): Error on rtrmnet lib init ", __func__);
  }

  for (i = mux_offset; i < num_data_ports; i++)
  {
    snprintf(vnd_name, NETMGR_IF_NAME_MAX_LEN,"%s%d",
             NETMGR_MAIN_RMNET_DATA_PREFIX, i);

    if(rtrmnet_ctl_newvnd(rmnet_cfg_handle, netmgr_main_cfg.phys_net_dev,
                          vnd_name , &err, 1 + i, 0) < 0)
    {
      netmgr_log_err("%s(): Error on rtrmnet lib init Error code:%d",
                     __func__, err);
    }
    else
    {
      netmgr_log_med("%s(): rtrmnet created new vnd %d", __func__, i);
    }
  }
#ifdef FEATURE_DATA_IWLAN
  rev_mux_offset = netmgr_main_cfg.rev_mux_offset;
  for (i = 0; i < (unsigned char)num_rev_data_ports; i++)
  {
    snprintf(vnd_name, NETMGR_IF_NAME_MAX_LEN, "%s%d",
             NETMGR_MAIN_REV_RMNET_DATA_PREFIX, i);

    if(rtrmnet_ctl_newvnd(rmnet_cfg_handle, netmgr_main_cfg.phys_net_dev,
                          vnd_name , &err, rev_mux_offset + i +1, 0) < 0)
    {
      netmgr_log_err("%s(): Error on rtrmnet lib init Error code:%d",
                     __func__, err);
    }
    else
    {
      netmgr_log_med("%s(): rtrmnet created new iwlan vnd %d ", __func__, i);
    }
  }

#endif /* FEATURE_DATA_IWLAN */

  rtrmnet_ctl_deinit(rmnet_cfg_handle);

  NETMGR_UNLOCK_MUTEX(netmgr_rmnet_mutex);
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_change_embedded_link
===========================================================================*/
/*!
@brief Changes flag config of an existing link with RTM_NETLINK messages to
       underlying rmnet kernel driver with use of librmnetctl library

@param
  int - link number

@return
  int - NETMGR_SUCCESS always

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rmnet_change_embedded_link
(
  int link_num
)
{
  int iwlan_link_num;
  int flag = 0;
  int rc = NETMGR_FAILURE;
  uint16_t err = NETMGR_FAILURE;
  rmnetctl_hndl_t *rmnet_cfg_handle = NULL;
  char vnd_name[NETMGR_IF_NAME_MAX_LEN];
  NETMGR_LOCK_MUTEX(netmgr_rmnet_mutex);

  flag = netmgr_vnd_last_flags;

  if (rtrmnet_ctl_init(&rmnet_cfg_handle, &err) < 0)
  {
    netmgr_log_err("%s(): Error on rtrmnet lib init ", __func__);
    NETMGR_UNLOCK_MUTEX(netmgr_rmnet_mutex);
    return rc;
  }

  if (link_num >=0 && link_num < netmgr_main_cfg.rev_link_start)
  {
    snprintf(vnd_name, NETMGR_IF_NAME_MAX_LEN,"%s%d",
             NETMGR_MAIN_RMNET_DATA_PREFIX, link_num);

    if(rtrmnet_ctl_changevnd(rmnet_cfg_handle, netmgr_main_cfg.phys_net_dev,
                             vnd_name , &err, 1 + link_num, flag) < 0)
    {
      netmgr_log_err("%s(): Error on rtrmnet lib changevnd Error code:%d",
                     __func__, err);
      goto bail;
    }
    else
    {
      netmgr_log_high("%s(): rtrmnet changed vnd %d with flag %d",
                      __func__, link_num, flag);
      rc = NETMGR_SUCCESS;
      goto bail;
    }
  }
  else
  {
    netmgr_log_med("%s(): invalid forward link num %d, maybe iwlan link",
                   __func__, link_num);
  }

#ifdef FEATURE_DATA_IWLAN
  if (link_num >= netmgr_main_cfg.rev_link_start &&
      link_num < netmgr_main_cfg.max_links_in_total)
  {
    iwlan_link_num = link_num - netmgr_main_cfg.rev_link_start;
    snprintf(vnd_name, NETMGR_IF_NAME_MAX_LEN,"%s%d",
             NETMGR_MAIN_REV_RMNET_DATA_PREFIX, iwlan_link_num);

    if(rtrmnet_ctl_changevnd(rmnet_cfg_handle, netmgr_main_cfg.phys_net_dev,
                             vnd_name , &err,
                             netmgr_main_cfg.rev_mux_offset + iwlan_link_num + 1,
                             flag) < 0)
    {
      netmgr_log_err("%s(): Error on rtrmnet lib changevnd Error code:%d",
                     __func__, err);
      goto bail;
    }
    else
    {
      netmgr_log_high("%s(): rtrmnet changed iwlan vnd %d with flag %d",
                      __func__, link_num, flag);
      rc = NETMGR_SUCCESS;
      goto bail;
    }
  }
  else
  {
    netmgr_log_med("%s(): invalid iwlan link num %d", __func__, link_num);
  }
#endif /* FEATURE_DATA_IWLAN */

bail:
  rtrmnet_ctl_deinit(rmnet_cfg_handle);

  NETMGR_UNLOCK_MUTEX(netmgr_rmnet_mutex);
  return rc;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_change_embedded_links
===========================================================================*/
/*!
@brief Changes flag config of an existing link with RTM_NETLINK messages to
       underlying rmnet kernel driver with use of librmnetctl library

@param
  int - flag configuration for links

@return
  int - NETMGR_SUCCESS always

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rmnet_change_embedded_links
(
  int flag
)
{
  unsigned int mux_offset;
  unsigned int rev_mux_offset;
  int i;
  uint16_t err = NETMGR_FAILURE;
  unsigned int num_data_ports = netmgr_main_cfg.num_data_ports;
  unsigned int num_rev_data_ports = netmgr_main_cfg.num_rev_data_ports;
  rmnetctl_hndl_t *rmnet_cfg_handle = NULL;
  char vnd_name[NETMGR_IF_NAME_MAX_LEN];
  NETMGR_LOCK_MUTEX(netmgr_rmnet_mutex);

  netmgr_vnd_last_flags = flag;

  mux_offset = NETMGR_RMNET_START;

  if (rtrmnet_ctl_init(&rmnet_cfg_handle, &err) < 0)
  {
    netmgr_log_err("%s(): Error on rtrmnet lib init ", __func__);
  }
  mux_offset = NETMGR_RMNET_START;

  for (i = mux_offset; i < num_data_ports; i++)
  {
    snprintf(vnd_name, NETMGR_IF_NAME_MAX_LEN,"%s%d",
             NETMGR_MAIN_RMNET_DATA_PREFIX, i);

    if(rtrmnet_ctl_changevnd(rmnet_cfg_handle, netmgr_main_cfg.phys_net_dev,
                             vnd_name , &err, 1 + i, flag) < 0){
      netmgr_log_err("%s(): Error on rtrmnet lib init Error code:%d",
                     __func__, err);
    }
    else {
      netmgr_log_high("%s(): rtrmnet changed vnd %d with flag %d",
                       __func__, i, flag);
    }
  }
#ifdef FEATURE_DATA_IWLAN
  rev_mux_offset = netmgr_main_cfg.rev_mux_offset;
  for (i = 0; i < num_rev_data_ports; i++)
  {
    snprintf(vnd_name, NETMGR_IF_NAME_MAX_LEN, "%s%d",
             NETMGR_MAIN_REV_RMNET_DATA_PREFIX, i);

    if(rtrmnet_ctl_changevnd(rmnet_cfg_handle, netmgr_main_cfg.phys_net_dev,
                             vnd_name , &err, rev_mux_offset + i + 1, flag) < 0)
    {
      netmgr_log_err("%s(): Error on rtrmnet lib init Error code:%d", __func__,
                     err);
    }
    else
    {
      netmgr_log_high("%s(): rtrmnet changed iwlan vnd %d with flag %d",
                      __func__, i, flag);
    }
  }

#endif /* FEATURE_DATA_IWLAN */

  rtrmnet_ctl_deinit(rmnet_cfg_handle);

  NETMGR_UNLOCK_MUTEX(netmgr_rmnet_mutex);
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_remove_embedded_link
===========================================================================*/
/*!
@brief Removes rmnet embedded link with RTM_NETLINK messages from underlying
       rmnet kernel driver with the help of librmnetctl library

@return
  int - NETMGR_SUCCESS if vnd is removed, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rmnet_remove_embedded_link
(
  int link_num
)
{
  int iwlan_link_num;
  int rc = NETMGR_FAILURE;
  uint16_t err = NETMGR_FAILURE;
  rmnetctl_hndl_t *rmnet_cfg_handle = NULL;
  char vnd_name[NETMGR_IF_NAME_MAX_LEN];
  NETMGR_LOCK_MUTEX(netmgr_rmnet_mutex);

  if (rtrmnet_ctl_init(&rmnet_cfg_handle, &err) < 0)
  {
    netmgr_log_err("%s(): Error on rtrmnet lib init ", __func__);
    NETMGR_UNLOCK_MUTEX(netmgr_rmnet_mutex);
    return rc;
  }

  if (link_num >=0 && link_num < netmgr_main_cfg.rev_link_start)
  {
    snprintf(vnd_name, NETMGR_IF_NAME_MAX_LEN,"%s%d",
             NETMGR_MAIN_RMNET_DATA_PREFIX, link_num);

    if (rtrmnet_ctl_delvnd(rmnet_cfg_handle, vnd_name , &err) < 0)
    {
      netmgr_log_err("%s(): Error on rtrmnet lib init Error code:%d",
                     __func__, err);
      goto bail;
    }
    else
    {
      netmgr_log_med("%s(): rtrmnet deleted fwd vnd %d",
                     __func__, link_num);
      rc = NETMGR_SUCCESS;
      goto bail;
    }
  }
  else
  {
    netmgr_log_med("%s(): invalid forward link num %d, maybe iwlan link",
                   __func__, link_num);
  }

#ifdef FEATURE_DATA_IWLAN
  if (link_num >= netmgr_main_cfg.rev_link_start &&
      link_num < netmgr_main_cfg.max_links_in_total)
  {
    iwlan_link_num = link_num - netmgr_main_cfg.rev_link_start;
    snprintf(vnd_name, NETMGR_IF_NAME_MAX_LEN,"%s%d",
             NETMGR_MAIN_REV_RMNET_DATA_PREFIX, iwlan_link_num);

    if (rtrmnet_ctl_delvnd(rmnet_cfg_handle, vnd_name , &err) < 0)
    {
      netmgr_log_err("%s(): Error on rtrmnet lib delvnd Error code:%d",
                     __func__, err);
      goto bail;
    }
    else
    {
      netmgr_log_med("%s(): rtrmnet deleted iwlan vnd %d",
                     __func__, iwlan_link_num);
      rc = NETMGR_SUCCESS;
      goto bail;
    }
  }
  else
  {
    netmgr_log_med("%s(): invalid iwlan link num %d", __func__, link_num);
  }
#endif /* FEATURE_DATA_IWLAN */

bail:
  rtrmnet_ctl_deinit(rmnet_cfg_handle);

  NETMGR_UNLOCK_MUTEX(netmgr_rmnet_mutex);
  return rc;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_remove_embedded_links
===========================================================================*/
/*!
@brief Removes rmnet embedded links with RTM_NETLINK messages from underlying
       rmnet kernel driver with the help of librmnetctl library

@return
  int - NETMGR_SUCCESS always

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_rmnet_remove_embedded_links(void)
{
  unsigned int mux_offset;
  int i;
  uint16_t err;
  rmnetctl_hndl_t *rmnet_cfg_handle;
  unsigned int num_data_ports = netmgr_main_cfg.num_data_ports;
  unsigned int num_rev_data_ports = netmgr_main_cfg.num_rev_data_ports;
  char vnd_name[NETMGR_IF_NAME_MAX_LEN];
  NETMGR_LOCK_MUTEX(netmgr_rmnet_mutex);

  mux_offset = NETMGR_RMNET_START;

  if (rtrmnet_ctl_init(&rmnet_cfg_handle, &err) < 0)
  {
    netmgr_log_err("%s(): Error on rtrmnet lib init ", __func__);
  }

  mux_offset = NETMGR_RMNET_START;

  for (i = mux_offset; i < num_data_ports; i++)
  {
    snprintf(vnd_name, NETMGR_IF_NAME_MAX_LEN, "%s%d",
             NETMGR_MAIN_RMNET_DATA_PREFIX, i);

    if (rtrmnet_ctl_delvnd(rmnet_cfg_handle, vnd_name , &err) < 0){
      netmgr_log_err("%s(): Error on rtrmnet lib init Error code:%d",
                     __func__, err);
    }
    else
    {
      netmgr_log_err("%s(): rtrmnet del new vnd %d", __func__, i);
    }
  }
#ifdef FEATURE_DATA_IWLAN
  for (i = 0; i < num_rev_data_ports; i++)
  {
    snprintf(vnd_name, NETMGR_IF_NAME_MAX_LEN, "%s%d",
             NETMGR_MAIN_REV_RMNET_DATA_PREFIX, i);

    if (rtrmnet_ctl_delvnd(rmnet_cfg_handle, vnd_name , &err) < 0)
    {
      netmgr_log_err("%s(): Error on rtrmnet lib init Error code:%d",
                     __func__, err);
    }
    else
    {
      netmgr_log_err("%s(): rtrmnet del new iwlan vnd %d", __func__, i);
    }
  }

#endif /* FEATURE_DATA_IWLAN */

  rtrmnet_ctl_deinit(rmnet_cfg_handle);

  NETMGR_UNLOCK_MUTEX(netmgr_rmnet_mutex);
  return NETMGR_SUCCESS;
}


/*===========================================================================
  FUNCTION  netmgr_rmnet_ext_init
===========================================================================*/
/*!
@brief Init Netmgr rmnet extention

@return
  int - NETMGR_SUCCESS/NETMGR_FAILURE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/

/* Rmnet ext init flags:
   Bits 0-3: Kernel flow control mode
   Bits 4: QMI power collapse enabled
   Bits 5: Kernel QMAP flow control enabled
   Bits 6: Powersave ext
*/
#define NETMGR_KFC_MASK 0x0F
#define NETMGR_QPC_MASK 0x10
#define NETMGR_QMAP_MASK 0x20
#define NETMGR_PS_EXT_MASK 0x40

#define NETMGR_RMNET_EXT_LIB "libnetmgr_rmnet_ext.so"
#define NETMGR_RMNET_EXT_INIT_FUNC "__netmgr_rmnet_ext_init"
typedef int netmgr_rmnet_ext_init_f(int flags);

int netmgr_rmnet_ext_init()
{
  void *dl_handle = NULL;
  char *err = NULL;
  netmgr_rmnet_ext_init_f *rmnet_ext_init = NULL;
  int flags =0;

  if (netmgr_main_cfg.kfc_mode)
    flags = (netmgr_main_cfg.kfc_mode & NETMGR_KFC_MASK);

  if (netmgr_main_cfg.qmi_pc)
    flags |= NETMGR_QPC_MASK;

  if (netmgr_main_cfg.kfc_qmap)
    flags |= NETMGR_QMAP_MASK;

  if (netmgr_main_cfg.dfc_ps_ext)
    flags |= NETMGR_PS_EXT_MASK;

  if (0 == flags)
  {
    return NETMGR_SUCCESS;
  }

  dl_handle = dlopen(NETMGR_RMNET_EXT_LIB, RTLD_NOW);
  if (NULL == dl_handle)
  {
    netmgr_log_err("Can't open %s", NETMGR_RMNET_EXT_LIB);
    goto bail;
  }

  rmnet_ext_init = (netmgr_rmnet_ext_init_f *)
             dlsym(dl_handle, NETMGR_RMNET_EXT_INIT_FUNC);

  if (NULL == rmnet_ext_init)
  {
      err = (char *)dlerror();
      if (err != NULL)
      {
          netmgr_log_err("%s not found in %s: %s",
                  NETMGR_RMNET_EXT_INIT_FUNC, NETMGR_RMNET_EXT_LIB, err);
      }
      goto bail;
  }

  if (NETMGR_SUCCESS != rmnet_ext_init(flags))
  {
    netmgr_log_err("Failed rmnet ext init with flags 0x%x", flags);
    goto bail;
  }

  return NETMGR_SUCCESS;

bail:

  netmgr_main_cfg.kfc_mode = 0;
  netmgr_main_cfg.qmi_pc = 0;
  netmgr_main_cfg.kfc_qmap = 0;
  netmgr_main_cfg.dfc_ps_ext = 0;

  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_set_ko_loading_property
===========================================================================*/
/*!
@brief
  Sets android properties to trigger loading of netmgr kernel modules

@param
  prop_name - Name of the property

@param
  prop_value - value to be set in the property
               0 - for disabling
               1 - for enabling

@return
  int - NETMGR_SUCCESS if properties were set correctly
        NETMGR_FAILURE otherwise
*/
/*=========================================================================*/
int netmgr_rmnet_set_ko_loading_property(const char *prop_name, const char *prop_value)
{
  int ret_val = -1;

  if (!prop_name)
  {
    netmgr_log_err("%s(): invalid property name received!", __func__);
    return NETMGR_FAILURE;
  }

  ret_val = property_set(prop_name, prop_value);
  if (ret_val != 0)
  {
    netmgr_log_err("%s(): could not set the property [%s] to [%s]!",
                   __func__, prop_name, prop_value);
    return NETMGR_FAILURE;
  }
  else
  {
    netmgr_log_low("%s(): set the property [%s] to [%s]!", __func__, prop_name, prop_value);
  }

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_rmnet_set_shsusr_loading_property
===========================================================================*/
/*!
@brief
  Sets android properties to trigger loading of shsusrd module

@param
  prop_name - Name of the property

@param
  prop_value - value to be set in the property
               0 - for disabling
               1 - for enabling

@return
  int - NETMGR_SUCCESS if properties were set correctly
        NETMGR_FAILURE otherwise
*/
/*=========================================================================*/
int netmgr_rmnet_set_shsusr_loading_property(const char *prop_name,
                                             const char *prop_value)
{
  int ret_val = -1;

  if (!prop_name)
  {
    netmgr_log_err("%s(): invalid property name received!", __func__);
    return NETMGR_FAILURE;
  }

  ret_val = property_set(prop_name, prop_value);
  if (ret_val != 0)
  {
    netmgr_log_err("%s(): could not set the property [%s] to [%s]!",
                   __func__, prop_name, prop_value);
    return NETMGR_FAILURE;
  }
  else
  {
    netmgr_log_low("%s(): set the property [%s] to [%s]!", __func__, prop_name, prop_value);
  }

  return NETMGR_SUCCESS;

}

/*===========================================================================
  FUNCTION netmgr_rmnet_update_embedded_ul_aggregation
===========================================================================*/
/*!
@brief
  Updates the aggregation parameters on an RmNet Data egress device

@arg *phys_dev physical device corresponding to a transport to modem
@arg ul_agg_count New aggregation packet count
@arg ul_agg_size New aggregation buffer size count
@arg ul_agg_time New aggregation time limit in nanoseconds

@return
  int - NETMGR_RMNET_SUCCESS if successful
  NETMGR_RMNET_BAD_ARGS if null/invalid arguments are passed
  NETMGR_RMNET_LIB_FAILURE if failed to configure kernel
*/
/*=========================================================================*/
int
netmgr_rmnet_update_embedded_ul_aggregation
(
  const char *phys_dev,
  unsigned int ul_agg_count,
  unsigned int ul_agg_size,
  unsigned int ul_agg_time,
  unsigned int ul_agg_features
)
{
  int result, rc;
  rmnetctl_hndl_t *rmnet_cfg_handle;
  uint16_t status_code;
  char *vnd_name = NETMGR_MAIN_RMNET_DATA_PREFIX "0";

  NETMGR_LOG_FUNC_ENTRY;
  result = NETMGR_RMNET_SUCCESS;

  if (!phys_dev)
  {
    netmgr_log_err("%s(): Internal error: dev cannot be null!\n", __func__);
    NETMGR_LOG_FUNC_EXIT;
    return NETMGR_RMNET_BAD_ARGS;
  }

  rc = rtrmnet_ctl_init(&rmnet_cfg_handle, &status_code);
  if (rc < 0)
  {
    netmgr_log_err("%s(): rtrmnet handle init failed [rc:%d][status_code:%d]\n",
                   __func__, rc, status_code);
    NETMGR_LOG_FUNC_EXIT;
    return NETMGR_RMNET_LIB_FAILURE;
  }

  netmgr_log_low("%s(): Updated UL agg params: agg_size=%d, agg_count=%d, "
                 "agg_time=%d agg_features=0x%02x\n", __func__, ul_agg_size, ul_agg_count,
                 ul_agg_time, ul_agg_features);

  rc = rtrmnet_set_uplink_aggregation_params(rmnet_cfg_handle, phys_dev,
                                             vnd_name, (uint8_t)ul_agg_count,
                                             (uint16_t)ul_agg_size,
                                             (uint32_t)ul_agg_time,
                                             (uint8_t)ul_agg_features,
                                             &status_code);

  if (rc != RMNETCTL_SUCCESS)
  {
    netmgr_log_err("%s(): rmnet_set_uplink_aggregation_params failed "
                   "[rc:%d][status_code:%d]\n", __func__, rc, status_code);
    result = NETMGR_RMNET_LIB_FAILURE;
  }

  if (netmgr_main_cfg.llc.enabled)
  {
    rc = rtrmnet_set_ll_uplink_aggregation_params(
                        rmnet_cfg_handle, (char *)phys_dev, vnd_name,
                        (uint8_t)netmgr_main_cfg.llc.ul_agg_cnt,
                        (uint16_t)netmgr_main_cfg.llc.ul_agg_size,
                        (uint32_t)netmgr_main_cfg.llc.ul_agg_time,
                        (uint8_t)netmgr_main_cfg.llc.ul_agg_features,
                        &status_code);

    if (rc != RMNETCTL_SUCCESS)
    {
      netmgr_log_err("%s(): set ll agg params failed [rc:%d][status_code:%d]\n",
                     __func__, rc, status_code);
    }
  }

  rtrmnet_ctl_deinit(rmnet_cfg_handle);
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

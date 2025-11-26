/*!
  @file
  dsi_netctrl_multimodem.c

  @brief
  This file implements multi modem specific routines for
  dsi_netctrli.c

*/

/*===========================================================================

  Copyright (c) 2010-2015, 2019, 2021-2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //depot/restricted/linux/android/ril/qcril_dsi.c $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
04/07/10   js      created file

===========================================================================*/
#include <inttypes.h>
#include "dsi_netctrl.h"
#include "dsi_netctrli.h"
#include "qmi_nas_srvc.h"
#include "wireless_data_service_v01.h"

typedef struct dsi_mm_route_rsp_s
{
  dsi_qmi_wds_route_look_up_rsp_type qmi_rsp_data;
  int modem;
} dsi_mm_route_rsp;

/* following table assists in priting more readable
 * debug msgs */
char * dsi_modem_id_tbl[DSI_MAX_MODEMS] =
{
  "DSI_RADIO_MODEM_2", /* index 0 */
  "DSI_RADIO_MODEM_1"  /* index 1 */
};

/*===========================================================================
  FUNCTION:  dsi_mm_get_pref_tech
===========================================================================*/
/*!
    @brief
    demo purpose only
    following function will determine based on
    NAS query as what modem to use for
    call bring up

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mm_get_pref_modem()
{
  int qmi_err_code;
  int ret;

  qmi_nas_serving_system_info_type serving_system_info;

  if ((ret = qmi_nas_get_serving_system(nas_clnt_id,&serving_system_info,&qmi_err_code)) < 0)
  {
    DSI_LOG_ERROR("qmi_nas_get_serving_system() returned error [%d], qmi_err [%d]",
                  ret, qmi_err_code);
    DSI_LOG_DEBUG("assuming 3GMODEM [%d] is not available", DSI_RADIO_3GMODEM);
    return DSI_RADIO_4GMODEM;
  }

  DSI_LOG_DEBUG("dsi_mm_get_pref_tech: NAS reg state is [%d]", serving_system_info.reg_state);
  DSI_LOG_DEBUG("dsi_mm_get_pref_tech: NAS num_radio_interfaces are [%d]",
                serving_system_info.num_radio_interfaces);
  DSI_LOG_DEBUG("dsi_mm_get_pref_tech: NAS network_type is [%d]",
                serving_system_info.network_type);
  DSI_LOG_DEBUG("dsi_mm_get_pref_tech: NAS cs_attach_state is [%d]",
                serving_system_info.cs_attach_state);
  DSI_LOG_DEBUG("dsi_mm_get_pref_tech: NAS ps_attach_state is [%d]",
                serving_system_info.ps_attach_state);

  if(serving_system_info.num_radio_interfaces > 0 &&
     serving_system_info.reg_state == QMI_NAS_REGISTERED &&
     serving_system_info.ps_attach_state == QMI_NAS_PS_ATTACHED)
  {
    DSI_LOG_DEBUG("using 3G MODEM");
    return DSI_RADIO_3GMODEM;
  }
  else
  {
    DSI_LOG_DEBUG("using 4G MODEM");
    return DSI_RADIO_4GMODEM;
  }
}

/*===========================================================================
  FUNCTION:  dsi_mm_iface_look_up
===========================================================================*/
/*!
    @brief
    following function will determine based on
    multi modem route look up, as which dsi
    interface can be used for SNI (start network
    interface). the interface is returned in piface.
    the selected modem is returned in pmodem.
    the selected technology is returned in ptech.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mm_iface_look_up
(
  dsi_store_t * st_hndl,
  int *piface,
  int *pmodem,
  int *ptech
)
{
  int ret = DSI_ERROR;
  dsi_mm_route_rsp rsp_data[DSI_MAX_MODEMS];
  dsi_qmi_wds_route_look_up_params_type route_lookup_params;
  int iface, modem;
  int other_ip_selected_modem = DSI_INVALID_MODEM;
  boolean companion_ip_rl_up = DSI_FALSE;
  wds_ip_family_preference_enum_v01 requested_qmi_family;
  int port_incall = FALSE;
  dsi_link_status_type link_state_v4 = WDS_CONNECTION_STATUS_DISCONNECTED_V01;
  dsi_link_status_type link_state_v6 = WDS_CONNECTION_STATUS_DISCONNECTED_V01;
  netmgr_nl_event_info_t  *resp_info = NULL;
  netmgr_user_cmd_data_t  *usr_cmd   = NULL;
  int result = DSI_ERROR;

  DSI_LOG_DEBUG("dsi_mm_iface_look_up: entry");

  do
  {
    if (!(DSI_IS_HNDL_VALID(st_hndl)) ||
        NULL == piface ||
        NULL == pmodem)
    {
      DSI_LOG_ERROR("invalid params received");
      break;
    }

    *piface = DSI_INVALID_IFACE;

    /* prepare route look up parameters */
    if (DSI_ERROR ==
        dsi_prepare_route_lookup_params(st_hndl,
                                        &route_lookup_params))
    {
      DSI_LOG_ERROR("dsi_prepare_route_lookup_params failed");
      break;
    }
    
    requested_qmi_family = route_lookup_params.ip_family_pref;

    switch(route_lookup_params.ip_family_pref)
    {
    case WDS_IP_FAMILY_PREF_IPV4_V01:
      companion_ip_rl_up = DSI_TRUE;
      break;

    case WDS_IP_FAMILY_PREF_IPV6_V01:
      companion_ip_rl_up = DSI_TRUE;
      break;

    case WDS_IP_FAMILY_PREF_UNSPEC_V01:
      companion_ip_rl_up = DSI_TRUE;
      route_lookup_params.ip_family_pref = WDS_IP_FAMILY_PREF_IPV4_V01;
      break;

    default:
      companion_ip_rl_up = DSI_FALSE;
      break;
    }

   if ((resp_info = malloc(sizeof(netmgr_nl_event_info_t))) == NULL)
   {
     DSI_LOG_ERROR("%s(): Failed to allocate buffer", __func__);
     break;
   }

   memset(resp_info,0x0, sizeof(netmgr_nl_event_info_t));

   usr_cmd = malloc(sizeof(netmgr_user_cmd_data_t));
   if ( NULL == usr_cmd)
   {
     DSI_LOG_ERROR("Error: out of memory");
     break;
   }

    memset(usr_cmd,0x0, sizeof(netmgr_user_cmd_data_t));

    usr_cmd->modem_subs_id = dsi_get_modem_subs_id( st_hndl );
    memcpy(&(usr_cmd->rl_params), &route_lookup_params, sizeof(usr_cmd->rl_params));
    usr_cmd->app_type = st_hndl->priv.app_type;

    DSI_LOG_DEBUG("%s DSI route lookup %d pmask %d pi %d pi3gpp2 %d "
                  "tpref %d xt_t_pref %d ippref %d apn %s datagin %d "
                  "bringup_by_apn_name %d bringup_by_apn_type %d apn_type %d"
                  " apn_type mask 0x%"PRIu64"",
                  __func__,
                  route_lookup_params.route_lookup,
                  route_lookup_params.params_mask,
                  route_lookup_params.profile_index,
                  route_lookup_params.profile_index_3gpp2,
                  route_lookup_params.tech_pref,
                  route_lookup_params.xtended_tech_pref,
                  route_lookup_params.ip_family_pref,
                  route_lookup_params.apn_name,
                  route_lookup_params.data_call_origin,
                  route_lookup_params.bringup_by_apn_name,
                  route_lookup_params.bringup_by_apn_type,
                  route_lookup_params.apn_type,
                  route_lookup_params.apn_type_mask);

    usr_cmd->cmd_id = NETMGR_USER_CMD_QUERY_ROUTE_LOOK_UP;
    result = netmgr_client_send_user_cmd(NETMGR_USER_CMD_QUERY_ROUTE_LOOK_UP,
                                         usr_cmd,
                                         resp_info,
                                         NETMGR_UNIX_MESSAGE_RECV_TIMEOUT);

    if(resp_info->cmd_data.txn.txn_status != NETMGR_USER_CMD_STATUS_SUCCESS)
    {
      DSI_LOG_ERROR("%s() Route lookup failed in Netmgr", __func__);
      break;
    }

    *pmodem = resp_info->cmd_data.rl_resp.pmodem;
    *ptech  = resp_info->cmd_data.rl_resp.ptech;
    *piface = resp_info->cmd_data.rl_resp.qmi_inst;

    if(resp_info->cmd_data.rl_resp.is_companion_ip == TRUE)
    {
        /* Set Companion IP call flag for single IP call request only */
        if((requested_qmi_family == WDS_IP_FAMILY_PREF_IPV4_V01)|| 
           (requested_qmi_family == WDS_IP_FAMILY_PREF_IPV6_V01))
        {
          st_hndl->priv.companion_ip_call = TRUE;
          DSI_LOG_DEBUG("Found a valid Companion IP call");
        }
    }

    if(resp_info->cmd_data.rl_resp.is_qmi_inst_valid == FALSE)
    {
        st_hndl->priv.rl_qmi_inst      = DSI_INVALID_QMI_INST;
        st_hndl->priv.rl_port_incall   = FALSE;
        st_hndl->priv.rl_link_state_v4 = WDS_CONNECTION_STATUS_DISCONNECTED_V01;
        st_hndl->priv.rl_link_state_v6 = WDS_CONNECTION_STATUS_DISCONNECTED_V01;
    }
    else
    {
      st_hndl->priv.rl_qmi_inst = (int)resp_info->cmd_data.rl_resp.qmi_inst;

      if(DSI_SUCCESS == dsi_is_qmiport_incall( *piface, &port_incall,
                                                 &link_state_v4,&link_state_v6))
      {
         if(port_incall == TRUE)
         {
           st_hndl->priv.rl_port_incall = TRUE;
           st_hndl->priv.rl_link_state_v4 = link_state_v4;
           st_hndl->priv.rl_link_state_v6 = link_state_v6;
         }
      }

      DSI_LOG_DEBUG("rl_qmi_inst = [%d] v4 state [%d] v6 state [%d]",st_hndl->priv.rl_qmi_inst,
                                                              st_hndl->priv.rl_link_state_v4,
                                                              st_hndl->priv.rl_link_state_v6);

    }

    if (!DSI_IS_ID_VALID(*piface))
    {
      DSI_LOG_ERROR("couldn't find a dsi id on modem");
      break;
    }

    ret = DSI_SUCCESS;

  } while (0);

  if(resp_info)
    free(resp_info);

  if(usr_cmd)
    free(usr_cmd);

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_DEBUG("dsi_mm_iface_look_up: exit with success");
  }
  else
  {
    DSI_LOG_ERROR("dsi_mm_iface_look_up: exit with error");
  }

  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_mm_check_matching_call
===========================================================================*/
/*!
    @brief
    following function will determine based on
    multi modem route look up, as which dsi
    interface can be used for SNI (start network
    interface). the interface is returned in piface.
    the selected modem is returned in pmodem.
    the selected technology is returned in ptech.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mm_check_matching_call
(
  dsi_store_t               *st_hndl,
  dsi_matching_call_check_t *call_params,
  char                      *device_name,
  int                       device_name_len
)
{
  int ret = DSI_ERROR;
  int result = DSI_ERROR;
  size_t bytes = 0;
  netmgr_nl_event_info_t *resp_info = NULL;
  netmgr_user_cmd_data_t *usr_cmd = NULL;
  dsi_qmi_wds_route_look_up_params_type route_lookup_params;

  do
  {
    if (!DSI_IS_HNDL_VALID_R(st_hndl))
    {
      DSI_LOG_ERROR("dsi_mm_check_matching_call: received invalid hndl");
      break;
    }

    if (!call_params || !device_name || device_name_len == 0)
    {
      DSI_LOG_ERROR("%s(): received invalid params", __func__);
      break;
    }

    if ((resp_info = malloc(sizeof(netmgr_nl_event_info_t))) == NULL)
    {
      DSI_LOG_ERROR("%s(): Failed to allocate buffer", __func__);
      break;
    }

    usr_cmd = malloc(sizeof(netmgr_user_cmd_data_t));
    if ( NULL == usr_cmd)
    {
      DSI_LOG_ERROR("%s(): Failed to allocate user buffer", __func__);
      break;
    }

    memset(usr_cmd, 0x0, sizeof(netmgr_user_cmd_data_t));
    memset(resp_info, 0x0, sizeof(netmgr_nl_event_info_t));

    result = dsi_prepare_call_match_lookup_params(call_params, &route_lookup_params);
    if (DSI_SUCCESS != result)
    {
      DSI_LOG_ERROR("%s(): route_lookup_params failed!", __func__);
      break;
    }

    usr_cmd->modem_subs_id = dsi_get_modem_subs_id(st_hndl);
    memcpy(&(usr_cmd->rl_params), &route_lookup_params, sizeof(usr_cmd->rl_params));

    DSI_LOG_DEBUG("%s DSI route lookup %d pmask %d pi %d "
                  "tpref %d xt_t_pref %d ippref %d apn %s datagin %d "
                  "bringup_by_apn_name %d bringup_by_apn_type %d apn_type %d"
                  " apn_type mask 0x%"PRIu64"",
                  __func__,
                  route_lookup_params.route_lookup,
                  route_lookup_params.params_mask,
                  route_lookup_params.profile_index,
                  route_lookup_params.tech_pref,
                  route_lookup_params.xtended_tech_pref,
                  route_lookup_params.ip_family_pref,
                  route_lookup_params.apn_name,
                  route_lookup_params.data_call_origin,
                  route_lookup_params.bringup_by_apn_name,
                  route_lookup_params.bringup_by_apn_type,
                  route_lookup_params.apn_type,
                  route_lookup_params.apn_type_mask);

    usr_cmd->cmd_id = NETMGR_USER_CMD_QUERY_MATCHING_CALL_IFACE;
    result = netmgr_client_send_user_cmd(NETMGR_USER_CMD_QUERY_MATCHING_CALL_IFACE,
                                         usr_cmd,
                                         resp_info,
                                         NETMGR_UNIX_MESSAGE_RECV_TIMEOUT);
    if((NETMGR_SUCCESS != result)
        || (resp_info->cmd_data.txn.txn_status != NETMGR_USER_CMD_STATUS_SUCCESS))
    {
      DSI_LOG_ERROR("%s() Matching call query failed in Netmgr", __func__);
      break;
    }

    if (FALSE == resp_info->cmd_data.matching_call_check_resp.device_name_valid)
    {
      DSI_LOG_ERROR("%s(): could not find any matching call!", __func__);
      break;
    }

    bytes = strlcpy(device_name,
                    resp_info->cmd_data.matching_call_check_resp.device_name,
                    device_name_len);
    if (bytes >= device_name_len)
    {
      DSI_LOG_ERROR("%s(): string truncation occurred!", __func__);
      break;
    }

    ret = DSI_SUCCESS;
  } while (0);

  if (usr_cmd)
  {
    free(usr_cmd);
  }

  if (resp_info)
  {
    free(resp_info);
  }

  return ret;
}

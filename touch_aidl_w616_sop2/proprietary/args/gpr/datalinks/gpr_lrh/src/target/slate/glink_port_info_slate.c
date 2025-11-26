/*=============================================================================
Copyright (c) 2022 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

#include "gpr_glink_i.h"

/* Names of subsystems */

#define GPR_REMOTE_SS_LPASS     ("lpass")
#define GPR_REMOTE_SS_APPS      ("apss")

/* Unique Names for subsystem connections */
#define GPR_IPC_ADSP_APPS_PORT_NAME  ("adsp_apps")

#define GPR_GLINK_PORT_PAIRS 1

/******************************************************************************
 * Variables                                                              *
 *****************************************************************************/
/* Glink related port data for all domains
 * Each entry in the table corresponds to one domain id pair so totally num_domain*num_domain entries*/
static gpr_glink_port_info_t glink_port_data_slate[GPR_GLINK_PORT_PAIRS] = {
		{ GPR_IDS_DOMAIN_ID_ADSP_V, GPR_REMOTE_SS_LPASS, GPR_IDS_DOMAIN_ID_APPS_V,     GPR_REMOTE_SS_APPS,     GPR_IPC_ADSP_APPS_PORT_NAME}
 };

void get_glink_port_data_info(gpr_glink_port_data_info_t *port_data_info)
{
	port_data_info->gpr_glink_port_pairs = GPR_GLINK_PORT_PAIRS;
	port_data_info->glink_port_data = &glink_port_data_slate[0];
}

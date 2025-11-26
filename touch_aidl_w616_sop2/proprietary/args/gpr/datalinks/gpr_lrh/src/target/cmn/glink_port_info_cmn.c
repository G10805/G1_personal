/*=============================================================================
Copyright (c) 2022, 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

#include "gpr_glink_i.h"

/* Names of subsystems */

#define GPR_REMOTE_SS_LPASS     ("lpass")
#define GPR_REMOTE_SS_APPS      ("apss")
#define GPR_REMOTE_SS_MODEM     ("mpss")
#define GPR_REMOTE_SS_SDSP      ("ssc")
#define GPR_REMOTE_SS_CDSP      ("cdsp")
#define GPR_REMOTE_SS_GDSP0     ("gpdsp0")
#define GPR_REMOTE_SS_GDSP1     ("gpdsp1")


/* Unique Names for subsystem connections */
#define GPR_IPC_ADSP_APPS_PORT_NAME  ("adsp_apps")
#define GPR_IPC_ADSP_MODEM_PORT_NAME ("adsp_modem")
#define GPR_IPC_ADSP_SDSP_PORT_NAME  ("adsp_sdsp")
#define GPR_IPC_ADSP_CDSP_PORT_NAME  ("adsp_cdsp")
#define GPR_IPC_MODEM_APPS_PORT_NAME ("modem_apps")
#define GPR_IPC_MODEM_SDSP_PORT_NAME ("modem_sdsp")
#define GPR_IPC_MODEM_CDSP_PORT_NAME ("modem_cdsp")
#define GPR_IPC_APPS_SDSP_PORT_NAME  ("apps_sdsp")
#define GPR_IPC_APPS_CDSP_PORT_NAME  ("apps_cdsp")
#define GPR_IPC_SDSP_CDSP_PORT_NAME  ("sdsp_cdsp")
#define GPR_IPC_GDSP0_APPS_PORT_NAME ("gdsp0_apps")
#define GPR_IPC_GDSP1_APPS_PORT_NAME ("gdsp1_apps")

#define GPR_GLINK_PORT_PAIRS (12 + 1) // Add supported entries only. Update the size as neeeded.

/******************************************************************************
 * Variables                                                              *
 *****************************************************************************/
/* Glink related port data for all domains
 * Each entry in the table corresponds to one domain id pair so totally num_domain*num_domain entries*/
static gpr_glink_port_info_t glink_port_data_cmn[GPR_GLINK_PORT_PAIRS] = {
    { GPR_IDS_DOMAIN_ID_ADSP_V, GPR_REMOTE_SS_LPASS, GPR_IDS_DOMAIN_ID_APPS_V,     GPR_REMOTE_SS_APPS,     GPR_IPC_ADSP_APPS_PORT_NAME},
    { GPR_IDS_DOMAIN_ID_ADSP_V, GPR_REMOTE_SS_LPASS, GPR_IDS_DOMAIN_ID_MODEM_V,    GPR_REMOTE_SS_MODEM,    GPR_IPC_ADSP_MODEM_PORT_NAME},
    { GPR_IDS_DOMAIN_ID_ADSP_V, GPR_REMOTE_SS_LPASS, GPR_IDS_DOMAIN_ID_SDSP_V,     GPR_REMOTE_SS_SDSP,     GPR_IPC_ADSP_SDSP_PORT_NAME},
    { GPR_IDS_DOMAIN_ID_ADSP_V, GPR_REMOTE_SS_LPASS, GPR_IDS_DOMAIN_ID_CDSP_V,     GPR_REMOTE_SS_CDSP,     GPR_IPC_ADSP_CDSP_PORT_NAME},
    { GPR_IDS_DOMAIN_ID_MODEM_V,GPR_REMOTE_SS_MODEM, GPR_IDS_DOMAIN_ID_APPS_V,     GPR_REMOTE_SS_APPS,     GPR_IPC_MODEM_APPS_PORT_NAME},
    { GPR_IDS_DOMAIN_ID_MODEM_V,GPR_REMOTE_SS_MODEM, GPR_IDS_DOMAIN_ID_SDSP_V,     GPR_REMOTE_SS_SDSP,     GPR_IPC_MODEM_SDSP_PORT_NAME},
    { GPR_IDS_DOMAIN_ID_MODEM_V,GPR_REMOTE_SS_MODEM, GPR_IDS_DOMAIN_ID_CDSP_V,     GPR_REMOTE_SS_CDSP,     GPR_IPC_MODEM_CDSP_PORT_NAME},
    { GPR_IDS_DOMAIN_ID_APPS_V, GPR_REMOTE_SS_APPS,  GPR_IDS_DOMAIN_ID_SDSP_V,     GPR_REMOTE_SS_SDSP,     GPR_IPC_APPS_SDSP_PORT_NAME},
    { GPR_IDS_DOMAIN_ID_APPS_V, GPR_REMOTE_SS_APPS,  GPR_IDS_DOMAIN_ID_CDSP_V,     GPR_REMOTE_SS_CDSP,     GPR_IPC_APPS_CDSP_PORT_NAME},
    { GPR_IDS_DOMAIN_ID_SDSP_V, GPR_REMOTE_SS_SDSP,  GPR_IDS_DOMAIN_ID_CDSP_V,     GPR_REMOTE_SS_CDSP,     GPR_IPC_SDSP_CDSP_PORT_NAME},
    { GPR_IDS_DOMAIN_ID_GDSP0_V,GPR_REMOTE_SS_GDSP0, GPR_IDS_DOMAIN_ID_APPS_V,     GPR_REMOTE_SS_APPS,     GPR_IPC_GDSP0_APPS_PORT_NAME},
    { GPR_IDS_DOMAIN_ID_GDSP1_V,GPR_REMOTE_SS_GDSP1, GPR_IDS_DOMAIN_ID_APPS_V,     GPR_REMOTE_SS_APPS,     GPR_IPC_GDSP1_APPS_PORT_NAME}
 };

void get_glink_port_data_info(gpr_glink_port_data_info_t *port_data_info)
{
    port_data_info->gpr_glink_port_pairs = GPR_GLINK_PORT_PAIRS;
    port_data_info->glink_port_data = &glink_port_data_cmn[0];
}

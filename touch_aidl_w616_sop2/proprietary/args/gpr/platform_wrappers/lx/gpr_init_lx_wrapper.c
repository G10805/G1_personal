/*
 * gpr_init_lx_wrapper.c
 *
 * This file has implementation platform wrapper for the GPR datalink layer
 *
 * Copyright (c) 2019, 2021, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#define LOG_TAG "gpr_lx_wrapper"
#include "gpr_api_i.h"
#include "gpr_lx.h"

#ifdef FEATURE_IPQ_OPENWRT
#include <syslog.h>
#ifndef ALOGD
#define ALOGD(fmt, arg...) syslog (LOG_NOTICE, fmt, ##arg)
#endif
#else
#include <log/log.h>
#endif

#define GPR_NUM_PACKETS_TYPE 3

#define GPR_NUM_PACKETS_1 ( 100 )
#define GPR_DRV_BYTES_PER_PACKET_1 ( 512 )
#define GPR_NUM_PACKETS_2 ( 4 )
#define GPR_DRV_BYTES_PER_PACKET_2 ( 4096 )
#define GPR_NUM_PACKETS_3 ( 0 )
#define GPR_DRV_BYTES_PER_PACKET_3 ( 65536 )

#ifdef PLATFORM_SLATE
#undef GPR_NUM_PACKETS_2
#define GPR_NUM_PACKETS_2 ( 8 )

#undef GPR_NUM_PACKETS_3
#define GPR_NUM_PACKETS_3 ( 2 )
#endif

/*****************************************************************************
 * Global variables                                                          *
 ****************************************************************************/
/* GPR IPC table containing init,deinit functions for datalink layers depending on
domains a given src domain wishes to establish a link with and the availability
of shared memory */

#ifdef AMS_GPR_EN
struct ipc_dl_v2_t gpr_lx_ipc_dl_table[GPR_PL_NUM_TOTAL_DOMAINS_V]= {
   { GPR_IDS_DOMAIN_ID_MODEM_V, ipc_dl_lx_init, ipc_dl_lx_deinit, TRUE},
   { GPR_IDS_DOMAIN_ID_APPS_V, ipc_dl_local_init, ipc_dl_local_deinit, TRUE},
};
#else
struct ipc_dl_v2_t gpr_lx_ipc_dl_table[GPR_PL_NUM_TOTAL_DOMAINS_V]= {
   { GPR_IDS_DOMAIN_ID_ADSP_V, ipc_dl_lx_init, ipc_dl_lx_deinit, TRUE},
/*   { GPR_IDS_DOMAIN_ID_MODEM_V, ipc_dl_lx_init, ipc_dl_lx_deinit},*/
#ifdef PLATFORM_SLATE
   { GPR_IDS_DOMAIN_ID_CC_DSP_V, ipc_dl_lx_init, ipc_dl_lx_deinit, FALSE},
#endif
   { GPR_IDS_DOMAIN_ID_APPS_V, ipc_dl_local_init, ipc_dl_local_deinit, TRUE},
};
#endif
struct gpr_packet_pool_info_v2_t gpr_lx_packet_pool_table[GPR_NUM_PACKETS_TYPE]={
   { GPR_HEAP_INDEX_DEFAULT, 0, 0, GPR_NUM_PACKETS_1, GPR_DRV_BYTES_PER_PACKET_1},
   { GPR_HEAP_INDEX_DEFAULT, 0, 0, GPR_NUM_PACKETS_2, GPR_DRV_BYTES_PER_PACKET_2},
   { GPR_HEAP_INDEX_DEFAULT, 1, 0, GPR_NUM_PACKETS_3, GPR_DRV_BYTES_PER_PACKET_3},
};

/*****************************************************************************
 * Local function definitions                                                *
 ****************************************************************************/
GPR_INTERNAL uint32_t gpr_drv_init(void)
{
   ALOGD("GPR INIT START");
   uint32_t  rc;
   uint32_t num_domains = sizeof(gpr_lx_ipc_dl_table)/sizeof(ipc_dl_v2_t);
   uint32_t num_packet_pools =
             sizeof(gpr_lx_packet_pool_table)/sizeof(gpr_packet_pool_info_v2_t);

   rc = gpr_drv_internal_init_v2(GPR_IDS_DOMAIN_ID_APPS_V,
                                 num_domains,
                                 gpr_lx_ipc_dl_table,
                                 num_packet_pools,
                                 gpr_lx_packet_pool_table);
   return rc;
}

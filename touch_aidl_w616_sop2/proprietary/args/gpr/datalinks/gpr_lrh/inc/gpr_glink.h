#ifndef _GPR_GLINK_H_
#define _GPR_GLINK_H_
/*=============================================================================
Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

/******************************************************************************
 * Includes                                                                   *
 *****************************************************************************/
#include "gpr_comdef.h"
#include "ipc_dl_api.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/******************************************************************************
 * Defines                                                                    *
 *****************************************************************************/
/*IPC datalink init function called from gpr layer for glink*/
GPR_INTERNAL uint32_t ipc_dl_glink_init(uint32_t                 src_domain_id,
                                        uint32_t                 dest_domain_id,
                                        const gpr_to_ipc_vtbl_t *p_gpr_to_ipc_vtbl,
                                        ipc_to_gpr_vtbl_t **     pp_ipc_to_gpr_vtbl);

/*IPC datalink de-init function called from gpr layer for glink*/
GPR_INTERNAL uint32_t ipc_dl_glink_deinit (uint32_t src_domain_id, uint32_t dest_domain_id);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* _GPR_GLINK_H_ */

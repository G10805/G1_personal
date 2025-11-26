#ifndef GSL_SHMEM_MGR_H
#define GSL_SHMEM_MGR_H
/**
 * \file gsl_shmem_mgr.h
 *
 * \brief
 *   Manages shared memory that is mapped to Spf, tries to reuse existing
 *    unused memory blocks that are already mapped with Spf across multiple
 *   GSL sessions
 *
 * \copyright
 *  Copyright (c) 2022, 2024 by Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "ar_osal_error.h"
#include "ar_osal_types.h"

#define GSL_SHMEM_MAP_UNCACHED 0x4
#define GSL_SHMEM_MAP_CACHED   0x10

typedef struct gsl_shmem_page *gsl_shmem_handle_t;

struct gsl_shmem_alloc_data {
	gsl_shmem_handle_t handle; /*< shmem mgr handle */
	void *v_addr; /*< address to be used by client for accessing buffer */
	uint32_t spf_mmap_handle; /*< handle to be passed to Spf */
	uint64_t spf_addr; /*< spf addr <msw,lsw> populated from ipa */
	uint64_t metadata; /*< metadata returned from OSAL */
};

int32_t gsl_shmem_alloc_ext(uint32_t size_bytes, uint32_t spf_ss_mask,
	uint32_t flags, uint32_t platform_info, uint32_t master_proc_id,
	struct gsl_shmem_alloc_data *alloc_data);

int32_t gsl_shmem_free(struct gsl_shmem_alloc_data *alloc_data);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif

#ifndef AMS_OSAL_SHMEM_H
#define AMS_OSAL_SHMEM_H

/**
 * \file ams_osal_shmem.h
 * \brief
 *     Defines public APIs for shared memory allocation for DSP.

 * \copyright
 *  Copyright (c) 2018-2021, 2023 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/


typedef struct ams_shmem_handle_data {
#ifdef AMS_USE_DMABUF
  int32_t heap_fd;
  bool dma_sync_flag;
#else
  /*ion fd created on ion memory allocation*/
  int64_t ion_mem_fd;
#ifndef TARGET_ION_ABI_VERSION
  /* ion userspace handle used in legagcy ion implementation*/
  int32_t ion_user_handle;
#endif
#endif
} ams_shmem_handle_data_t;

 /** enum for shmem memory type*/
typedef enum ams_shmem_memory_type
{
	/** 0  Shared physical memory allocation.*/
	AMS_SHMEM_PHYSICAL_MEMORY = 0,
	/** 1  Shared virtual memory allocation */
	AMS_SHMEM_VIRTUAL_MEMORY = 1
}ams_shmem_memory_type_t;

/** enum for shmem cache type*/
typedef enum ams_shmem_cache_type
{
	/** 0  cached.*/
	AMS_SHMEM_CACHED = 0,
	/** 1  uncached.*/
	AMS_SHMEM_UNCACHED = 1
}ams_shmem_cache_type_t;

/** enum for shmem offset/address buffer index type*/
typedef enum ams_shmem_buffer_index_type
{
	/** 0  use physical or virtual addresses.*/
	AMS_SHMEM_BUFFER_ADDRESS = 0,
	/** 1  use offsets, the offset is from the base address.*/
	AMS_SHMEM_BUFFER_OFFSET = 1
}ams_shmem_buffer_index_type_t;

#define AMS_SS_ID_MODEM_DSP 1

#define AMS_SS_ID_APSS 3

/**< Bits to indicate if hardware accelerator is enabled/disabled */
#define AMS_SHMEM_HW_ACCELERATOR_ENABLED 0x1
#define AMS_SHMEM_HW_ACCELERATOR_DISABLED 0x0

/**< Bit mask for hardware accelerator flag */
#define AMS_SHMEM_BIT_MASK_HW_ACCELERATOR_FLAG 0x1
/**< Shift amount for hardware accelerator setup flag */
#define AMS_SHMEM_SHIFT_HW_ACCELERATOR_FLAG 0x0

 /**
 * Shared memory info structure
 */
typedef struct ams_shmem_info_t
{
	ams_shmem_cache_type_t            cache_type;     /**< in, cache type, cached or uncached memory */
	size_t                           buf_size;       /**< in, shared buffer size, should be a minimum of 4K and multiple of 4K only*/
	ams_shmem_memory_type_t           mem_type;       /**< out, shmem memory type, virtual or physical for ar_shmem_alloc()*/
													 /**< in, shmem memory type, virtual or physical for ar_shmem_map()*/
	ams_shmem_buffer_index_type_t     index_type;     /**< out, DSP to operate on buffer offsets or on address pointers.*/
	uint32_t                         ipa_lsw;        /**< out, smmu mapped ipa lsw, output for ar_shmem_alloc()/ar_shmem_map() */
	uint32_t                         ipa_msw;        /**< out, smmu mapped ipa msw, output for ar_shmem_alloc()/ar_shmem_map() */
	uint32_t                         pa_lsw;         /**< out, physical address lsw, alignment requirements apply, like 4k,
															   start address multiple of 64, output of ar_shmem_alloc()*/
															   /**< in, physical address lsw, alignment requirements apply, like 4k,
															   start address multiple of 64, input for ar_shmem_map()*/
	uint32_t                         pa_msw;         /**< out, physical address msw, alignment requirements apply, like 4k,
															   start address multiple of 64, output of ar_shmem_alloc()*/
															   /**< in, physical address lsw, alignment requirements apply, like 4k
															    start address multiple of 64, input for ar_shmem_map()*/
	void                            *vaddr;          /**< out, virtual address 64bit/32bit, alignment requirements apply, like 4k,
															   start address multiple of 64, input for ar_shmem_alloc()*/
	uint64_t                         metadata;       /**< out opt, pointer address to metadata structure defined by each platform
																   for ar_shmem_alloc()*/
	uint8_t                          num_sys_id;     /**< in, number of subsystem IDs provided with ar_shmem_alloc()/ar_shmem_map() call.*/
	uint8_t                         *sys_id;         /**< in, pointer to array of size num_sys_id for sub-system Ids provided
														  in ar_osal_sys_id.h, used to allocate shared memory between the given
														  list of sys_id provided with ar_shmem_alloc()/ar_shmem_map() call.*/
	uint32_t                         platform_info;  /**< in opt, optional field for passing platform specific data to OSAL, this can be
														  used for example to communicate some heap properties provided only for ar_shmem_alloc()*/
	uint32_t                         flags;          /**< in, Bit field for flags. @values{for bit 0}
                                                            - 1 -- hardware accelerator enabled, use AR_SHMEM_HW_ACCELERATOR_ENABLED
                                                            - 0 -- hardware accelerator disabled, use AR_SHMEM_HW_ACCELERATOR_DISABLED
                                                            - To set this bit, use #AR_SHMEM_BIT_MASK_HW_ACCELERATOR_FLAG and
                                                              AR_SHMEM_SHIFT_HW_ACCELERATOR_FLAG
                                                            All other bits are reserved; must be set to 0. */
} ams_shmem_info;

/**
 * \brief ams_shmem_init
 *        initialize shared memory interface.
 * \return
 *  0 -- Success
 *  Nonzero -- Failure
 */
int32_t ams_shmem_init(void);

/**
 * \brief Allocates shared memory.
 *  Only non cached memory allocation supported.
 *  Size if multiple of 4KB and the returned is aligned to 4KB boundary.
 *  Buffer start address should be atleast 64bit multiple aligned.
 *
 * \param[in_out] info: pointer to ar_shmem_info.
 *
 * \return
 *  0 -- Success
 *  Nonzero -- Failure
 *
 */
int32_t ams_shmem_alloc(ams_shmem_info *info);

/**
 * Frees shared memory.
 *
 * \param[in] info: pointer to ar_shmem_info.
 *
 * \return
 *  0 -- Success
 *  Nonzero -- Failure
 */
int32_t ams_shmem_free(ams_shmem_info *info);

/**
 * \brief ar_shmem_deinit.
 *
 * \return
 *  0 -- Success
 *  Nonzero -- Failure
 */
int32_t ams_shmem_deinit(void);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* #ifndef AMS_OSAL_SHMEM_H */


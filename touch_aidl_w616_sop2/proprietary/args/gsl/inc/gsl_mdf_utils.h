#ifndef GSL_MDF_UTILS_H
#define GSL_MDF_UTILS_H
/**
 * \file gsl_mdf_utils.h
 *
 * \brief
 *      Multi-DSP framework utilities library
 *
 *  Copyright (c) 2018-2021, Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


 /**
  * @h2xml_title_date {08/01/2018}
  */
 #include "ar_osal_types.h"

#define GSL_MULTI_DSP_FWK_DATA_MID 0x2005

 /**
  * @h2xmlm_module  {"GSL_MULTI_DSP_FWK_DATA_MID", GSL_MULTI_DSP_FWK_DATA_MID}
  * @h2xmlm_displayName  {"GSL_MULTI_DSP_FWK_DATA_MID"}
  * @h2xmlm_description  {Processor group shared memory information \n}
  *
  * -  This module supports the following parameter IDs: \n
  * - #PARAM_ID_PROC_GROUP_INFO_PARAMS \n
  *
  * @{                   <-- Start of the Module -->
  */
#define PARAM_ID_PROC_GROUP_INFO_PARAMS 0x0001


/** @h2xmle_description {Shared memory info of each group}
 * @h2xmlp_subStruct
 */
struct per_group_shmem_info_t {
	/** @h2xmle_description {Size of the shared memory}
	 * @h2xmle_default     {1024}
	 */
	uint32_t shmem_size;

	/** @h2xmle_description {Number of satellite procs in this group}
	 * @h2xmle_default     {1}
	 */
	uint32_t num_satellite_procs;

	/** @h2xmle_description {Master proc in this group}
	 * @h2xmle_default     {1}
	 */
	uint32_t master_proc;

	/** @h2xmle_description {Satellite processor IDs.}
	 * @h2xmle_variableArraySize {num_satellite_procs}
	 */
	uint32_t satellite_proc_ids[];

};

/* Shared memory information for each processor group. */
typedef struct per_group_shmem_info_t per_group_shmem_info_t;


/**
 * @h2xmlp_parameter   {"PARAM_ID_PROC_GROUP_INFO_PARAMS",
 *   PARAM_ID_PROC_GROUP_INFO_PARAMS}
 * @h2xmlp_description {Used to configure processor group info}
 */

struct proc_group_info_params_t {
	/** @h2xmle_description  {Number of processor group.}
	 * @h2xmle_default      {1}
	 */
	uint32_t num_proc_groups;

#ifdef __H2XML__
	/**
	 * @h2xmle_variableArraySize {num_proc_groups}
	 */
	per_group_shmem_info_t shmem_info[];
#endif
};

/**
 * @}
 */
typedef struct proc_group_info_params_t proc_group_info_params_t;

/*
 * \brief Returns all master proc ids
 *  Returns number of SPF masters if master_proc_ids is NULL
 *  Else, fills master_proc_ids array with SPF master proc ids
 *
 * \param[out] num_master_procs: number of SPF masters.
 * \param[out] master_proc_ids: Array of SPF master proc ids
 */
int32_t gsl_mdf_utils_get_master_proc_ids(uint32_t *num_master_procs,
					  uint32_t master_proc_ids[]);

/*
 * \brief Returns the susbsytem mask of master proc and it's satellites
 *
 * \param[in] master_proc: SPF master proc id
 * \param[out] supported_ss_mask: Subsystem mask including master proc mask
 *
 */
int32_t gsl_mdf_utils_get_supported_ss_info_from_master_proc(
						     uint32_t master_proc,
						     uint32_t *supported_ss_mask);

/*
 * \brief Returns if the subsystem mask is of a master proc
 *
 * \param[in] supported_ss_mask: Subsystem mask
 *
 */
bool_t gsl_mdf_utils_is_master_proc(uint32_t spf_ss_mask);

/*
 * \brief Returns the master proc corresponding to the mask.
 *  Returns 0 if it doesn't exist in proc group of any master proc.
 *
 * \param[in] supported_ss_mask: Subsystem mask
 *
 */
uint32_t gsl_mdf_utils_get_master_proc_id(uint32_t spf_ss_mask);

/*
 * \brief Looks up the spf susbsytem ids from ACDB
 *
 * \param[in] sg_list: subgraph id list comprising a graph
 * \param[in] num_sg: number of subgraphs in sg list
 * \param[out] ss_mask: mask comprising this subsystems this graph requires,
 * use macros from gsl_spf_ss_state.h to test if a subsystem is present
 * \return AR_EOK on success, error code otherwise
 */
uint32_t gsl_mdf_utils_query_graph_ss_mask(uint32_t *sg_id_list,
	uint32_t num_sgs, uint32_t *ss_mask);

/*
 * \brief allocate the loaned MDF shmem for a given ss_mask if not already
 * done so
 *
 *  \param[in] ss_mask: mask representing the set of subsystems for which to
 * allocate the shared loaned memory
 * \param[in] master_proc: SPF master proc id
 *
 */
int32_t gsl_mdf_utils_shmem_alloc(uint32_t ss_mask, uint32_t master_proc);

/*
 * \brief notify mdf utils that some ss have been restarted. This is to remap
 * the loaned memory to those ss.
 *
 *  \param[in] restarted_ss_mask: mask representing the set of subsystems which
 * have been restarted
 */
void gsl_mdf_utils_notify_ss_restarted(uint32_t restarted_ss_mask);

/*
 * \brief initialize MDF utils, internally queries ACDB to get supported SS info
 *
 */
int32_t gsl_mdf_utils_init(void);

int32_t gsl_mdf_utils_deinit(void);
#endif

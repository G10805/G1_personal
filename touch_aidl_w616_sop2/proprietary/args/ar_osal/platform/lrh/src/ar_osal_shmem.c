/*
  Copyright (c) 2020,2022-2023 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "audio_smmu_helper.h"
#include "ar_osal_shmem.h"
#include "ar_osal_error.h"
#include "ar_osal_log.h"
#include "plat_dmabuf.h"

/*
#include "ssassign.h"
*/

#define AR_OSAL_SHMEM_LOG_TAG "AROSH"
#define AR_OSAL_SHMEM_ALIGNMENT 4096
#define MDF_MEM_BASE_ADDR 0xAE000000
#define MDF_MEM_SIZE_TOTAL 16 * 1024 * 1024  /* 16MB contiguous mem reserved */

static void *mdf_mem_base_phy_addr = NULL;
static size_t mdf_mem_size = 0;
static bool_t mdf_mem_assigned_to_subsys = FALSE;

// count the total number of subsys IDs
#define COUNT_SUBSYSID  (1+ AR_SUB_SYS_ID_LAST - AR_SUB_SYS_ID_FIRST)

#if ENABLE_HYP_ASSIGN
#define PERM_ALL (HYP_PROT_READ | HYP_PROT_WRITE | HYP_PROT_EXEC)
// vmid: Makena is VMID_SSC_Q6/VMID_ADSP_Q6, Lemans is VMID_ADSP_Q6/VMID_GPDSP0/VMID_GPDSP1
enum_dst_vmid_t dest_vmids[COUNT_SUBSYSID] = {0, };
int dest_perms[COUNT_SUBSYSID] = { [0 ... (COUNT_SUBSYSID-1)] = PERM_ALL };
/* map sysid (from AR_SUB_SYS_ID_FIRST to AR_SUB_SYS_ID_LAST) to vmid.
 SysID 5-9 have no corresponding VMID, thus, there are serveral zeros in the array.
 Tested ok on Makena and Lemans.
*/
const enum_dst_vmid_t vmid_sysid_map[COUNT_SUBSYSID] =
 {VMID_MSS_MBA, VMID_ADSP_Q6, VMID_HLOS, VMID_SSC_Q6, 0,0,0,0,0, VMID_GPDSP0, VMID_GPDSP1};
#endif // ENABLE_HYP_ASSIGN

typedef struct ar_shmem_pdata {
  int fd;
} ar_shmem_pdata_t;

static ar_shmem_pdata_t *pdata = NULL;

static int32_t ar_shmem_validate_sys_id(uint8_t num_sys_id, uint8_t* sys_id, bool_t *is_mdf)
{
  int32_t rc = AR_EOK;
  if (num_sys_id == 0 || num_sys_id >= COUNT_SUBSYSID || sys_id == NULL)
  {
    rc = AR_EBADPARAM;
    goto end;
  }

  for (uint8_t i = 0; i < num_sys_id; i++)
  {
    if (AR_AUDIO_DSP != sys_id[i] &&
        AR_MODEM_DSP != sys_id[i] &&
        AR_SENSOR_DSP != sys_id[i] &&
        AR_COMPUTE_DSP != sys_id[i] &&
        AR_GP_DSP0 != sys_id[i] &&
        AR_GP_DSP1 != sys_id[i] &&
        AR_APSS != sys_id[i])
    {
      rc = AR_EBADPARAM;
      break;
    }

    if (AR_MODEM_DSP == sys_id[i] || AR_SENSOR_DSP == sys_id[i] || AR_GP_DSP0 == sys_id[i] || AR_GP_DSP1 == sys_id[i])
    {
     AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "mdf mode set");
    *is_mdf = TRUE;
    }
#ifdef ENABLE_HYP_ASSIGN
    // populate the vmid for valid sysid
    dest_vmids[i] = vmid_sysid_map[sys_id[i] - AR_SUB_SYS_ID_FIRST];
#endif // ENABLE_HYP_ASSIGN
  }

end:
  return rc;
}

int32_t ar_shmem_get_mdf_mem_info(void** ret_phys_addr, size_t* ret_size)
{
  *ret_phys_addr = MDF_MEM_BASE_ADDR;
  *ret_size = MDF_MEM_SIZE_TOTAL;

  return AR_EOK;
}

/**
 * \brief ar_shmem_init
 *        initialize shared memory interface.
 * \return
 *  0 -- Success
 *  Nonzero -- Failure
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
int32_t ar_shmem_init()
{
  int32_t rc = AR_EOK;

  rc = audio_smmu_helper_init();
  if (rc)
  {
    AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "audio_smmu_helper_init failed with rc : %d", rc);
    goto end;
  }

  pdata = (ar_shmem_pdata_t *) malloc(sizeof(ar_shmem_pdata_t));
  if (NULL == pdata)
  {
    AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG,"%s: malloc failed\n", __func__);
    rc = AR_ENOMEMORY;
    goto end;
  }

  pdata->fd = dmabufheap_init(ID_DMA_BUF_HEAP_CACHED);
  if (pdata->fd < 0)
  {
    AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "Failed to open devfs for dmabuf heap");
    rc = AR_EFAILED;
    goto fail;
  }

  AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "dmabufheap_init success with fd : %d", pdata->fd);

  rc = ar_shmem_get_mdf_mem_info(&mdf_mem_base_phy_addr, &mdf_mem_size);
  if (AR_EOK != rc)
  {
    AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "get mdf memory info failed");
    goto mdf_fail;
  }
  AR_LOG_INFO(AR_OSAL_SHMEM_LOG_TAG, "mdf memory base 0x%016X, size %x",
        (uintptr_t)mdf_mem_base_phy_addr,
        mdf_mem_size);

  return rc;

mdf_fail:
  dmabufheap_release(pdata->fd);
fail:
  free(pdata);
end:
  return rc;
}

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
_IRQL_requires_max_(PASSIVE_LEVEL)
int32_t ar_shmem_alloc(_Inout_ ar_shmem_info *info)
{
  int32_t rc;
  uint32_t total_size;
  void* dev_addr = NULL;
  void* virt_addr = NULL;
  uint64_t addr;
  bool_t is_mdf = FALSE;
  audio_smmu_helper_t* asmmu = NULL;
  int32_t heap_fd;

  if (info == NULL || info->buf_size == 0)
  {
    AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "info(NULL)|buf_size(0)");
    rc = AR_EBADPARAM;
    goto end;
  }

  rc = ar_shmem_validate_sys_id(info->num_sys_id, info->sys_id, &is_mdf);
  if (rc)
  {
    AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "invalid sys_id");
    rc = AR_EBADPARAM;
    goto end;
  }

  if (TRUE == is_mdf)
  {
    if (info->buf_size > mdf_mem_size)
    {
        AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "insuffient mdf memory req_size %d reserved_size %d",
            info->buf_size, mdf_mem_size);
        rc = AR_EBADPARAM;
        goto end;
    }
#if ENABLE_HYP_ASSIGN
    if (FALSE == mdf_mem_assigned_to_subsys)
    {
        rc = ss_drv_hyp_assign_hlos_mem_to_multiple_vmid((uint64_t)(mdf_mem_base_phy_addr),
                   (uint32_t)(mdf_mem_size),
                   &dest_vmids[0],
                   &dest_perms[0], (int)info->num_sys_id);
        if (AR_EOK != rc)
        {
            AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "hyp_assign_phys failed rc %d", rc);
            rc = AR_EFAILED;
            goto end;
        }
    mdf_mem_assigned_to_subsys = TRUE;
    }
#endif // ENABLE_HYP_ASSIGN
    addr = (uint64_t)(mdf_mem_base_phy_addr);
    info->mem_type = AR_SHMEM_PHYSICAL_MEMORY;
    info->index_type = AR_SHMEM_BUFFER_OFFSET;
    info->ipa_lsw = (uint32_t)(addr & 0xFFFFFFFF);
    info->ipa_msw = (uint32_t)(addr >> 32);
    info->pa_lsw = info->ipa_lsw;
    info->pa_msw = info->ipa_msw;
    info->vaddr = (void*)addr;//virt_addr;we don't have actual virt_addr here, return phy_addr
    info->metadata = 0;
  }
  else
  {
    total_size = (info->buf_size + AR_OSAL_SHMEM_ALIGNMENT - 1)&~(AR_OSAL_SHMEM_ALIGNMENT - 1);

    asmmu = audio_smmu_helper_create(NULL);
    if (asmmu == NULL)
    {
      AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "audio_smmu_helper_create failed");
      rc = AR_ENOMEMORY;
      goto end;
    }

    rc = dmabufheap_alloc(pdata->fd, total_size, O_RDWR | O_CLOEXEC, &heap_fd);
    if (rc < 0 || heap_fd < 0)
    {
        AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "Invalid heap_fd %d or rc : 0x%x \n", heap_fd, rc);
        rc = AR_ENOMEMORY;
        goto dmabuf_fail;
    }

    virt_addr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, heap_fd, 0);
    if (virt_addr == (void*)-1)
    {
        AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG,"%s: mmap failed with err: %d\n", __func__, errno);
        rc = AR_ENOMEMORY;
        goto mmap_fail;
    }

    asmmu->smmu_ctx.dmabuf_fd = heap_fd;

    dev_addr = audio_smmu_helper_map(asmmu);
    if (dev_addr == NULL)
    {
      AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "audio_smmu_helper_map failed\n");
      rc = AR_EFAILED;
      goto fail;
    }

    addr = (uint64_t)(dev_addr);

    info->mem_type = AR_SHMEM_PHYSICAL_MEMORY;
    info->index_type = AR_SHMEM_BUFFER_ADDRESS;
    info->ipa_lsw = (uint32_t)(addr & 0xFFFFFFFF);
    info->ipa_msw = (uint32_t)(addr >> 32);
    info->pa_lsw = info->ipa_lsw;
    info->pa_msw = info->ipa_msw;
    info->vaddr = virt_addr;
    info->metadata = (uint64_t)asmmu;

    AR_LOG_INFO(AR_OSAL_SHMEM_LOG_TAG, "shmem alloc - heap_fd : 0x%x, total size : %u, virt_addr : 0x%x, DA : 0x%x",
      asmmu->smmu_ctx.dmabuf_fd, total_size, info->vaddr, dev_addr);
  }

  return AR_EOK;

fail:
  munmap(virt_addr, total_size);
mmap_fail:
  dmabufheap_free(heap_fd);
dmabuf_fail:
  audio_smmu_helper_destroy(asmmu);
end:
  return rc;
}

/**
 * Frees shared memory.
 *
 * \param[in] info: pointer to ar_shmem_info.
 *
 * \return
 *  0 -- Success
 *  Nonzero -- Failure
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
int32_t ar_shmem_free(_In_ ar_shmem_info *info)
{
  int32_t rc = AR_EOK;
  uint64_t dev_addr;
  uint32_t total_size;
  bool_t is_mdf = FALSE;
  audio_smmu_helper_t* asmmu = NULL;
  int32_t heap_fd;

  if (info == NULL )
  {
    AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "info(NULL)");
    return AR_EBADPARAM;
  }

  rc = ar_shmem_validate_sys_id(info->num_sys_id, info->sys_id, &is_mdf);
  if (rc)
  {
    AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "invalid sys_id");
    return AR_EBADPARAM;
  }

  if (TRUE != is_mdf)
  {
    total_size = (info->buf_size + AR_OSAL_SHMEM_ALIGNMENT - 1)&~(AR_OSAL_SHMEM_ALIGNMENT - 1);
    dev_addr = ((uint64_t)info->ipa_msw) << 32 | (uint64_t)info->ipa_lsw;

    asmmu = (audio_smmu_helper_t*)(intptr_t)info->metadata;
    rc = audio_smmu_helper_unmap(asmmu);
    if (rc)
    {
      AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "audio_smmu_helper_unmap failed\n");
      return AR_EFAILED;
    }

    AR_LOG_INFO(AR_OSAL_SHMEM_LOG_TAG, "shmem free - heap_fd : 0x%x, total size : %u, virt_addr : 0x%x, DA : 0x%x",
      asmmu->smmu_ctx.dmabuf_fd, total_size, info->vaddr, dev_addr);

    heap_fd = asmmu->smmu_ctx.dmabuf_fd;

    munmap(info->vaddr, total_size);
    dmabufheap_free(heap_fd);
    audio_smmu_helper_destroy(asmmu);
  }

  return rc;
}


/**
 * \brief Helps map memory with SMMU an already allocated shared memory
 * for a give sub system.
 * Size should be multiple of 4KB boundary.
 * Buffer start address should be 64bit aligned.
 *
 * \param[in_out] info: pointer to ar_shmem_info.
 *                     required input parameters in ar_shmem_info
 *                     ar_shmem_info_t.cache_type
 *                     ar_shmem_info_t.buf_size
 *                     ar_shmem_info_t.pa_lsw
 *                     ar_shmem_info_t.pa_msw
 *                     ar_shmem_info_t.num_sys_id
 *                     ar_shmem_info_t.sys_id
 *                     required output parameters in ar_shmem_info
 *                     ar_shmem_info_t.ipa_lsw
 *                     ar_shmem_info_t.ipa_msw
 *
 *\return
 * 0 -- Success
 * Nonzero -- Failure
 *
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
int32_t ar_shmem_map(_Inout_ ar_shmem_info *info)
{
  int32_t rc;
  uint32_t total_size;
  void* dev_addr = NULL;
  uint64_t addr;
  bool_t is_mdf = FALSE;
  audio_smmu_helper_t* asmmu = NULL;

  if (info == NULL || info->buf_size == 0)
  {
    AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "info(NULL)|buf_size(0)");
    return AR_EBADPARAM;
  }

  rc = ar_shmem_validate_sys_id(info->num_sys_id, info->sys_id, &is_mdf);
  if (rc)
  {
    AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "invalid sys_id");
    return AR_EBADPARAM;
  }

  if (TRUE == is_mdf)
  {
    return AR_EFAILED;
  }
  else
  {
    total_size = (info->buf_size + AR_OSAL_SHMEM_ALIGNMENT - 1)&~(AR_OSAL_SHMEM_ALIGNMENT - 1);

    /*
     *This api is called by clients when the clients already have allocated shared memory
     *and now we just need to map it with DSP. On LRH platform what we recieve from clients
     *is the heap fd, which is passed in the form of pa_lsw and pa_msw. Hence we extract the same
     *and pass it on to the audio smmu helper.
     */

    asmmu = audio_smmu_helper_create(NULL);
    if (asmmu == NULL)
    {
      AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "audio_smmu_helper_create failed");
      rc = AR_ENOMEMORY;
      goto end;
    }

    asmmu->smmu_ctx.dmabuf_fd = ((uint64_t)info->pa_msw << 32) | info->pa_lsw;
    dev_addr = audio_smmu_helper_map(asmmu);
    if (dev_addr == NULL)
    {
      AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "audio_smmu_helper_map failed\n");
      return AR_EFAILED;
      goto fail;
    }

    addr = (uint64_t)(dev_addr);

    info->mem_type = AR_SHMEM_PHYSICAL_MEMORY;
    info->index_type = AR_SHMEM_BUFFER_ADDRESS;
    info->ipa_lsw = (uint32_t)(addr & 0xFFFFFFFF);
    info->ipa_msw = (uint32_t)(addr >> 32);
    info->pa_lsw = info->ipa_lsw;
    info->pa_msw = info->ipa_msw;
    info->metadata = (uint64_t)asmmu;

    AR_LOG_INFO(AR_OSAL_SHMEM_LOG_TAG, "shmem map - heap_fd : 0x%x, total size : %u, virt_addr : 0x%x, DA : 0x%x",
      asmmu->smmu_ctx.dmabuf_fd, total_size, info->vaddr, dev_addr);
  }

  return rc;

fail:
  audio_smmu_helper_destroy(asmmu);
end:
  return rc;
}

/**
 *\brief Helps unmap the shared memory allocated externally with SMMU
 *
 *\param[in] info: pointer to ar_shmem_info.
 *            required input parameters in ar_shmem_info
 *                     ar_shmem_info_t.cache_type
 *                     ar_shmem_info_t.buf_size
 *                     ar_shmem_info_t.pa_lsw
 *                     ar_shmem_info_t.pa_msw
 *                     ar_shmem_info_t.num_sys_id
 *                     ar_shmem_info_t.sys_id
 *
 *\return
 * 0 -- Success
 * Nonzero -- Failure
 *
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
int32_t ar_shmem_unmap(_In_ ar_shmem_info *info)
{
  int32_t rc = AR_EOK;
  uint64_t dev_addr;
  uint32_t total_size;
  bool_t is_mdf = FALSE;
  audio_smmu_helper_t* asmmu = NULL;

  if (info == NULL )
  {
    AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "info(NULL)");
    return AR_EBADPARAM;
  }

  rc = ar_shmem_validate_sys_id(info->num_sys_id, info->sys_id, &is_mdf);
  if (rc)
  {
    AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "invalid sys_id");
    return AR_EBADPARAM;
  }

  if (TRUE == is_mdf)
  {
    return AR_EFAILED;
  }
  else
  {
    total_size = (info->buf_size + AR_OSAL_SHMEM_ALIGNMENT - 1)&~(AR_OSAL_SHMEM_ALIGNMENT - 1);
    dev_addr = ((uint64_t)info->ipa_msw) << 32 | (uint64_t)info->ipa_lsw;

    asmmu = (audio_smmu_helper_t*)(intptr_t)info->metadata;

    AR_LOG_INFO(AR_OSAL_SHMEM_LOG_TAG, "shmem unmap - heap_fd : 0x%x, total size : %u, virt_addr : 0x%x, DA : 0x%x",
      asmmu->smmu_ctx.dmabuf_fd, total_size, info->vaddr, dev_addr);
    rc = audio_smmu_helper_unmap(asmmu);
    if (rc)
    {
      AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG, "audio_smmu_helper_unmap failed\n");
      rc = AR_EFAILED;
    }
    audio_smmu_helper_destroy(asmmu);
  }
  return rc;
}

/*
 * Helps to hyp assign physical memory between source and destination sub systems.
 *
 * \param[in] info: info: pointer to ar_shmem_hyp_assign_phys_info.
 *
 * \return
 *  0 -- Success
 *  Nonzero -- Failure
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
int32_t ar_shmem_hyp_assign_phys(ar_shmem_hyp_assign_phys_info *info)
{
    //todo, required for MDF
    return AR_ENOTIMPL;
}

int32_t ar_shmem_get_uid(uint64_t alloc_handle, uint64_t* uid)
{
    //todo
    return AR_ENOTIMPL;
}

/**
 * \brief ar_shmem_deinit.
 *
 * \return
 *  0 -- Success
 *  Nonzero -- Failure
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
int32_t ar_shmem_deinit()
{
  int32_t rc = AR_EOK;
  if (NULL == pdata)
  {
    AR_LOG_ERR(AR_OSAL_SHMEM_LOG_TAG,"%s:not in init state\n", __func__);
    rc = AR_EBADPARAM;
    goto end;
  }

  dmabufheap_release(pdata->fd);
  free(pdata);
  pdata = NULL;
  audio_smmu_helper_deinit();

end:
  return rc;
}


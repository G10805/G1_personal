/**
 *
 * \file ams_osal_shmem.c
 *
 * \brief
 *      This file has implementation of shared memory allocation for DSP.
 * \copyright
 *  Copyright (c) 2018-2021, 2022 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "ams_osal_shmem"
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/msm_ion.h>
#include <sys/ioctl.h>
#include <log/log.h>
#include <linux/msm_audio.h>

#if TARGET_ION_ABI_VERSION >= 2
#include <ion/ion.h>
#endif

#include "ams_osal_shmem.h"
#include "ams_osal_error.h"

/* Memory pointer to be 4K aligned for ADSP, MDSP,SDSP and CDSP */
#define SHMEM_4K_ALIGNMENT 0x1000
#define ION_DRIVER_PATH "/dev/msm_audio_ion"
#define ION_DRIVER_PATH_CMA "/dev/msm_audio_ion_cma"

/* Systems without SMMU use AUDIO_HEAP for ion allocations */
#ifdef USE_AUDIO_HEAP_ID
#define HEAP_MASK ION_HEAP(ION_AUDIO_HEAP_ID)
#else
#define HEAP_MASK ION_HEAP(ION_SYSTEM_HEAP_ID)
#endif

#define HEAP_MASK_CMA ION_HEAP(ION_AUDIO_ML_HEAP_ID)

typedef struct ams_shmem_pdata
{
    int32_t ion_handle;
    int ion_fd;
    int ion_fd_cma;
} ams_shmem_pdata_t;

static ams_shmem_pdata_t *pdata = NULL;

static pthread_mutex_t ams_shmem_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 *\brief ams_shmem_init
 *        initialize shared memory interface.
 * \return
 * 0 -- Success
 *  Nonzero -- Failure
 */
int32_t ams_shmem_init(void)
{
    int32_t status = AMS_EOK;
    ALOGE("%s: entry", __func__);
    pthread_mutex_lock(&ams_shmem_lock);
    if (NULL != pdata)
    {
        ALOGI("%s: init already done", __func__);
        goto end;
    }

    pdata = (ams_shmem_pdata_t *)malloc(sizeof(ams_shmem_pdata_t));
    if (NULL == pdata)
    {
        ALOGE("%s: malloc failed", __func__);
        status = AMS_ENOMEMORY;
        goto end;
    }
    /*initialize the ion_handle to invalid handle */
    pdata->ion_handle = -1;
#if TARGET_ION_ABI_VERSION >= 2
    pdata->ion_handle = ion_open();
#else
    pdata->ion_handle = open("/dev/ion", O_RDONLY | O_DSYNC);
#endif
    if (pdata->ion_handle < 0)
    {
        ALOGE("%s:ion dev open failed errno:%d", __func__, pdata->ion_handle);
        status = AMS_ENOTEXIST;
        free(pdata);
        goto end;
    }

    pdata->ion_fd_cma = open(ION_DRIVER_PATH_CMA, O_RDWR);
    if (pdata->ion_fd_cma < 0)
    {
        ALOGE("%s cma ion fd open failed %s, open ion fd",
              __func__, ION_DRIVER_PATH_CMA);

        pdata->ion_fd_cma = open(ION_DRIVER_PATH, O_RDWR);
        if (pdata->ion_fd_cma < 0)
        {
            status = AMS_ENOTEXIST;
            ALOGE("%s ion fd open failed %s status %d", __func__, ION_DRIVER_PATH, status);
            goto end;
        }
        else
        {
            status = AMS_EOK;
        }
    }
    else
    {
        ALOGE("%s cma ion fd open success %s", __func__, ION_DRIVER_PATH_CMA);
    }

end:
    pthread_mutex_unlock(&ams_shmem_lock);
    return status;
}

/*
 * \brief Allocates shared memory.
 *  Only non cached memory allocation supported.
 *  Size if multiple of 4KB and the returned is aligned to 4KB boundary.
 *  Buffer start address should be atleast 64bit multiple aligned.
 *
 * \param[in_out] info: pointer to ams_shmem_info.
 *
 * \return
 *  0 -- Success
 *  Nonzero -- Failure
 *
 */
int32_t ams_shmem_alloc(ams_shmem_info *info)
{
    int32_t status = AMS_EOK;
    ams_shmem_cache_type_t ctype;
    ams_shmem_handle_data_t *shmem_handle = NULL;
    uint32_t heap_mask;
#ifndef TARGET_ION_ABI_VERSION
    struct ion_fd_data fd_data;
    struct ion_allocation_data alloc_data;
#endif
    bool is_mdf = false;

    ALOGE("%s: entry", __func__);
    pthread_mutex_lock(&ams_shmem_lock);
    if (NULL == pdata)
    {
        ALOGE("%s:not in init state", __func__);
        status = AMS_EUNSUPPORTED;
        goto end;
    }

    if (NULL == info || 0 == info->buf_size)
    {
        ALOGE("Error: info(NULL)|buf_size(0) passed");
        status = AMS_EBADPARAM;
        goto end;
    }

    shmem_handle = (ams_shmem_handle_data_t *)
        malloc(sizeof(ams_shmem_handle_data_t));
    if (NULL == shmem_handle)
    {
        ALOGE("%s:malloc for shmem handle failed", __func__);
        status = AMS_ENOMEMORY;
        goto end;
    }

    info->ipa_lsw = 0;
    info->ipa_msw = 0;
    info->metadata = 0;
    info->pa_lsw = 0;
    info->pa_msw = 0;
    info->index_type = AMS_SHMEM_BUFFER_OFFSET;
    info->mem_type = AMS_SHMEM_PHYSICAL_MEMORY;

    if (info->cache_type == AMS_SHMEM_CACHED)
    {
        ctype = 1;
    }
    else
    {
        ctype = 0;
    }

    heap_mask = ((info->flags) & (AMS_SHMEM_BIT_MASK_HW_ACCELERATOR_FLAG << AMS_SHMEM_SHIFT_HW_ACCELERATOR_FLAG)) ? HEAP_MASK_CMA : HEAP_MASK;

#if TARGET_ION_ABI_VERSION >= 2
    status = ion_alloc_fd(pdata->ion_handle, info->buf_size,
                          SHMEM_4K_ALIGNMENT, heap_mask,
                          ctype, &shmem_handle->ion_mem_fd);

    if (status)
    {
        ALOGE("%s: ION alloc failed errno: %d",
              __func__, status);
        shmem_handle->ion_mem_fd = -1;
        status = AMS_ENOMEMORY;
        goto end;
    }
#else
    alloc_data.len = info->buf_size;
    alloc_data.align = SHMEM_4K_ALIGNMENT;
    alloc_data.heap_id_mask = heap_mask;
    alloc_data.flags = ctype;
    /* Initialize handle to 0 */
    alloc_data.handle = 0;
    status = ioctl(pdata->ion_handle, ION_IOC_ALLOC, &alloc_data);
    if (status)
    {
        ALOGE("%s: ION alloc failed, errno: %d",
              __func__, status);
        status = AMS_ENOMEMORY;
        goto end;
    }
    fd_data.handle = alloc_data.handle;
    shmem_handle->ion_user_handle = (uint32_t)alloc_data.handle;
    status = ioctl(pdata->ion_handle, ION_IOC_SHARE, &fd_data);
    if (status)
    {
        ALOGE("%s: ION_IOC_SHARE, errno: %d",
              __func__, status);
        status = AMS_ENOMEMORY;
        goto err1;
    }
    shmem_handle->ion_mem_fd = fd_data.fd;
#endif
    info->vaddr = mmap(0, info->buf_size,
                       PROT_READ | PROT_WRITE, MAP_SHARED,
                       shmem_handle->ion_mem_fd, 0);
    if (info->vaddr == MAP_FAILED)
    {
        ALOGE("%s: mmap failed ", __func__);
        status = AMS_ENOMEMORY;
        goto err2;
    }

    if ((uint64_t)info->vaddr % SHMEM_4K_ALIGNMENT)
    {
        ALOGE("Error: 4k alignment check failed vaddr(0x%pK)", info->vaddr);
        info->vaddr = NULL;
        status = AMS_EUNEXPECTED;
        goto err2;
    }
    info->metadata = (uint64_t)shmem_handle;
    info->ipa_lsw = shmem_handle->ion_mem_fd;
    ALOGE(" SHMEM: cache_type(0x%x)|memory_type(0x%x)|index_type(0x%x) buf_size(0x%x)|vaddr(0x%pK)| ipa_lsw(0x%x)| status(0x%x) ",
          ctype, info->mem_type, info->index_type, info->buf_size, info->vaddr, info->ipa_lsw, status);

    status = ioctl(pdata->ion_fd_cma, IOCTL_MAP_AMS_MEM, info->ipa_lsw);

    if (status)
    {
        ALOGE("%s:Physical address map failed status %d", __func__, status);
        goto err2;
    }
    goto end;

err2:
    close(shmem_handle->ion_mem_fd);
#ifndef TARGET_ION_ABI_VERSION
err1:
    status = ioctl(pdata->ion_handle, ION_IOC_FREE,
                   &shmem_handle->ion_user_handle);
    if (status < 0)
        ALOGE("%s: ION Free failed %d", __func__, status);
#endif
    free(shmem_handle);
end:
    pthread_mutex_unlock(&ams_shmem_lock);
    return status;
}

/*
 * Frees shared memory.
 *
 * \param[in] info: pointer to ams_shmem_info.
 *
 * \return
 *  0 -- Success
 *  Nonzero -- Failure
 */
int32_t ams_shmem_free(ams_shmem_info *info)
{
    int32_t status = AMS_EOK;
    ams_shmem_handle_data_t *shmem_handle;
    pthread_mutex_lock(&ams_shmem_lock);
    if (NULL == pdata)
    {
        ALOGE("%s:not in init state", __func__);
        status = AMS_EBADPARAM;
        goto end;
    }

    if (NULL == info || NULL == info->vaddr)
    {
        ALOGE("%s:invalid info %pK", __func__, info);
        status = AMS_EBADPARAM;
        goto end;
    }

    if (0 == info->metadata || 0 == info->buf_size)
    {
        ALOGE("%s:invalid params handle %pK buf_size %zu",
              __func__, info->metadata, info->buf_size);
        status = AMS_EBADPARAM;
        goto end;
    }

    shmem_handle = (ams_shmem_handle_data_t *)(intptr_t)info->metadata;
    munmap(info->vaddr, info->buf_size);

    status = ioctl(pdata->ion_fd_cma, IOCTL_UNMAP_AMS_MEM, shmem_handle->ion_mem_fd);

    if (status)
    {
        ALOGE("%s:unmap failed. status %d", __func__, status);
    }
    if (shmem_handle->ion_mem_fd)
    {
        close(shmem_handle->ion_mem_fd);
    }
    else
    {
        ALOGE("%s Invalid ion_mem_fd ", __func__);
    }
#ifndef TARGET_ION_ABI_VERSION
    status = ioctl(pdata->ion_handle, ION_IOC_FREE,
                   &shmem_handle->ion_user_handle);
    if (status < 0)
        ALOGE("%s: ION Free failed %d", __func__, status);
#endif
    free(shmem_handle);
    info->metadata = 0;
    info->vaddr = NULL;
end:
    pthread_mutex_unlock(&ams_shmem_lock);
    return status;
}

/*
 * \brief ams_shmem_deinit.
 *
 * \return
 *  0 -- Success
 *  Nonzero -- Failure
 */
int32_t ams_shmem_deinit(void)
{
    pthread_mutex_lock(&ams_shmem_lock);
    if (pdata == NULL)
    {
        ALOGE("%s not in init state", __func__);
        return AMS_EOK;
    }
    if (pdata->ion_handle)
    {
#if TARGET_ION_ABI_VERSION >= 2
        ion_close(pdata->ion_handle);
#else
        close(pdata->ion_handle);
#endif
    }
    if (pdata->ion_fd)
        close(pdata->ion_fd);
    if (pdata->ion_fd_cma)
        close(pdata->ion_fd_cma);
    free(pdata);
    pdata = NULL;
    pthread_mutex_unlock(&ams_shmem_lock);
    return AMS_EOK;
}

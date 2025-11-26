/**
 *
 * \file ams_osal_shmem_db.c
 *
 * \brief
 *      This file has implementation of shared memory allocation for DSP.
 * \copyright
 *  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *  All rights reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#define AMS_OSAL_SHMEM_LOG_TAG "ams_osal_shmem_db"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <log/log.h>
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <linux/msm_audio.h>
#include <cutils/properties.h>

#include "ams_osal_shmem.h"
#include "ams_osal_error.h"

#define SHMEM_4K_ALIGNMENT 0x1000
#define ION_DRIVER_PATH "/dev/msm_audio_ion"
#define ION_DRIVER_PATH_CMA "/dev/msm_audio_ion_cma"
#define DMABUF_SYS_HEAP_PATH "/dev/dma_heap/system"
#define DMABUF_SYS_HEAP_PATH_CMA "/dev/dma_heap/qcom,audio-ml"

typedef struct ams_shmem_pdata
{
    int dmabuf_fd;
    int dmabuf_fd_cma;
    int ion_fd;
    int ion_fd_cma;
    bool dmabuf_cma_mem_enabled;
} ams_shmem_pdata_t;

static ams_shmem_pdata_t *pdata = NULL;

static pthread_mutex_t ams_shmem_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 *\brief ar_shmem_ss_masks
 *        internal function to generate ss_mask from sys_ids
 * \return
 * true -- MDF usecase
 * false -- non-MDF usecase
 */
static uint64_t ams_shmem_ss_masks( ams_shmem_info *info);
static uint64_t ams_shmem_ss_masks( ams_shmem_info *info)
{
    uint32_t ss_masks = 0;
    for (uint8_t i = 0; i < info->num_sys_id; i++)
    {
        ss_masks |=  1 << (info->sys_id[i] - 1);
    }
    return ss_masks;
}
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

    pthread_mutex_lock(&ams_shmem_lock);
    if (NULL != pdata)
    {
        ALOGE("%s: init already done\n", __func__);
        goto end;
    }

    pdata = (ams_shmem_pdata_t *)malloc(sizeof(ams_shmem_pdata_t));
    if (NULL == pdata)
    {
        ALOGE("%s: malloc failed\n", __func__);
        status = AMS_ENOMEMORY;
        goto end;
    }
    /*Check if dma_buf heap initialize the dmabuf_handle to invalid handle */
    pdata->dmabuf_fd = open(DMABUF_SYS_HEAP_PATH, O_RDONLY | O_CLOEXEC);
    if (pdata->dmabuf_fd < 0)
    {
        ALOGE("%s:dmabuf dev open failed errno:%d\n", __func__, pdata->dmabuf_fd);
        status = AMS_ENOTEXIST;
        goto end;
    }

    pdata->ion_fd = open(ION_DRIVER_PATH, O_RDWR);
    if (pdata->ion_fd < 0)
    {
        status = AMS_ENOTEXIST;
        ALOGE("%s ion fd open failed %s status %d\n", __func__, ION_DRIVER_PATH, status);
        close(pdata->dmabuf_fd);
        goto end;
    }
    ALOGD("%s ion fd open success %s\n", __func__, ION_DRIVER_PATH);

    pdata->dmabuf_cma_mem_enabled =
        property_get_bool("vendor.audio.feature.dmabuf.cma.memory.enable", false);

    if (pdata->dmabuf_cma_mem_enabled)
    {
        /*Check if dma_buf heap initialize the dmabuf_handle to invalid handle */
        pdata->dmabuf_fd_cma = open(DMABUF_SYS_HEAP_PATH_CMA, O_RDONLY | O_CLOEXEC);
        if (pdata->dmabuf_fd_cma <= 0)
        {
            ALOGE("%s:dmabuf fd cma dev open failed errno:%d\n",
                  __func__, pdata->dmabuf_fd_cma);
            status = AMS_ENOTEXIST;
            close(pdata->ion_fd);
            close(pdata->dmabuf_fd);
            goto end;
        }

        pdata->ion_fd_cma = open(ION_DRIVER_PATH_CMA, O_RDWR);
        if (pdata->ion_fd_cma <= 0)
        {
            status = AMS_ENOTEXIST;
            ALOGE("%s ion fd cma open failed %s status %d\n",
                  __func__, ION_DRIVER_PATH_CMA, status);
            close(pdata->ion_fd);
            close(pdata->dmabuf_fd);
            close(pdata->dmabuf_fd_cma);
            goto end;
        }
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
    ams_shmem_handle_data_t *shmem_handle = NULL;
    struct dma_heap_allocation_data heap_data;
    struct dma_buf_sync sync;
    struct msm_mdf_data mdf_data = {0};
    memset(&heap_data, 0, sizeof heap_data);
    memset(&sync, 0, sizeof sync);

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

    shmem_handle = (ams_shmem_handle_data_t *)calloc(1, sizeof(ams_shmem_handle_data_t));
    if (NULL == shmem_handle)
    {
        ALOGE("%s:malloc for shmem handle failed\n", __func__);
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

    heap_data.len = info->buf_size;
    heap_data.fd_flags = O_RDWR;

    if (!((info->flags) & (1 << AMS_SHMEM_SHIFT_HW_ACCELERATOR_FLAG)))
    {
        ALOGE("Alloc dmabuf fd\n");
        status = ioctl(pdata->dmabuf_fd, DMA_HEAP_IOCTL_ALLOC, &heap_data);
        if (status)
        {
            ALOGE("%s: DMA heap alloc failed, errno: %d\n",
                  __func__, status);
            status = AMS_ENOMEMORY;
            goto end;
        }
    }
    else if (pdata->dmabuf_cma_mem_enabled)
    {
        ALOGE("Alloc dmabuf fd cma\n");
        status = ioctl(pdata->dmabuf_fd_cma, DMA_HEAP_IOCTL_ALLOC, &heap_data);
        if (status)
        {
            ALOGE("%s: DMA heap alloc failed, errno: %d\n",
                  __func__, status);
            status = AMS_ENOMEMORY;
            goto end;
        }
    }

    info->vaddr = mmap(0, info->buf_size,
                       PROT_READ | PROT_WRITE, MAP_SHARED,
                       heap_data.fd, 0);
    if (info->vaddr == MAP_FAILED)
    {
        ALOGE("%s: mmap failed \n", __func__);
        status = AMS_ENOMEMORY;
        goto err1;
    }

    if ((uint64_t)info->vaddr % SHMEM_4K_ALIGNMENT)
    {
        ALOGE("Error: 4k alignment check failed vaddr(0x%pK)", info->vaddr);
        info->vaddr = NULL;
        status = AMS_EUNEXPECTED;
        goto err2;
    }

    sync.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
    status = ioctl(heap_data.fd, DMA_BUF_IOCTL_SYNC, &sync);
    if (status)
    {
        ALOGE("%s: DMA heap set access failed, errno: %d\n",
              __func__, status);
        status = AMS_EUNEXPECTED;
        goto err2;
    }

    shmem_handle->heap_fd = heap_data.fd;
    shmem_handle->dma_sync_flag = true;
    info->metadata = (uint64_t)shmem_handle;
    info->ipa_lsw = shmem_handle->heap_fd;
    ALOGD(" SHMEM: cached|memory_type(0x%x)|index_type(0x%x) buf_size(0x%x)|vaddr(0x%pK)| ipa_lsw(0x%x)| status(0x%x) ",
          info->mem_type, info->index_type, info->buf_size, info->vaddr, info->ipa_lsw, status);
    if ((info->flags) & (1 << AMS_SHMEM_SHIFT_HW_ACCELERATOR_FLAG))
    {
        if (pdata->ion_fd_cma)
        {
            status = ioctl(pdata->ion_fd_cma, IOCTL_MAP_PHYS_ADDR, info->ipa_lsw);
        }
        else
        {
            status = AMS_ENOTEXIST;
            ALOGE("%s:ION CMA memory does not exist  %d\n",
                  __func__, status);
        }
    }
    else
        status = ioctl(pdata->ion_fd, IOCTL_MAP_PHYS_ADDR, info->ipa_lsw);
    if (status)
    {
        ALOGE("%s:Physical address map failed status %d\n",
              __func__, status);
        goto err2;
    }
    mdf_data.mem_fd = info->ipa_lsw;
    mdf_data.ss_masks = ams_shmem_ss_masks(info);

    if (pdata->ion_fd_cma) {
        status = ioctl(pdata->ion_fd_cma, IOCTL_MAP_HYP_ASSIGN_V2, &mdf_data);
        if (status) {
            ALOGE("%s:HYP ASSIGN failed for AMS mask %08x status %d\n",
                        __func__, mdf_data.ss_masks, status);
            goto err2;
        }
    } else {
        status = AMS_ENOTEXIST;
        ALOGE("%s:CMA memory does not exist for AMS %d\n",
                    __func__, status);
    }

    goto end;

err2:
    munmap(info->vaddr, info->buf_size);
    shmem_handle->dma_sync_flag = false;

err1:
    close(shmem_handle->heap_fd);
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
    struct dma_buf_sync sync;
    struct msm_mdf_data mdf_data = {0};
    pthread_mutex_lock(&ams_shmem_lock);

    if (NULL == pdata)
    {
        ALOGE("%s:not in i state\n", __func__);
        status = AMS_EBADPARAM;
        goto end;
    }

    if (NULL == info || NULL == info->vaddr)
    {
        ALOGE("%s:invalid info %pK\n", __func__, info);
        status = AMS_EBADPARAM;
        goto end;
    }

    if (0 == info->metadata || 0 == info->buf_size)
    {
        ALOGE("%s:invalid params handle %pK buf_size %zu\n",
              __func__, info->metadata, info->buf_size);
        status = AMS_EBADPARAM;
        goto end;
    }

    shmem_handle = (ams_shmem_handle_data_t *)(intptr_t)info->metadata;
    mdf_data.mem_fd = shmem_handle->heap_fd;
    mdf_data.ss_masks = ams_shmem_ss_masks(info);
    if (pdata->ion_fd_cma) {
            ALOGD("%s: reclaim AMS shmem memory mask %08x\n",
                        __func__,mdf_data.ss_masks);
            status = ioctl(pdata->ion_fd_cma, IOCTL_UNMAP_HYP_ASSIGN_V2, &mdf_data);
        } else {
            status = AMS_ENOTEXIST;
            ALOGE("%s:DMA_BUF CMA does not exist %d\n",
                        __func__, status);
        }
    if (shmem_handle && shmem_handle->dma_sync_flag)
    {
        sync.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW;
        status = ioctl(shmem_handle->heap_fd, DMA_BUF_IOCTL_SYNC, &sync);
        if (status)
        {
            ALOGE("%s: DMA heap set access failed, errno: %d\n",
                  __func__, status);
        }
        shmem_handle->dma_sync_flag = false;
    }
    munmap(info->vaddr, info->buf_size);
    if ((info->flags) & (1 << AMS_SHMEM_SHIFT_HW_ACCELERATOR_FLAG))
    {
        if (pdata->ion_fd_cma)
        {
            status = ioctl(pdata->ion_fd_cma, IOCTL_UNMAP_PHYS_ADDR, shmem_handle->heap_fd);
        }
        else
        {
            status = AMS_ENOTEXIST;
            ALOGE("%s:ION CMA memory does not exist  %d\n",
                  __func__, status);
        }
    }
    else
        status = ioctl(pdata->ion_fd, IOCTL_UNMAP_PHYS_ADDR, shmem_handle->heap_fd);
    if (status)
    {
        ALOGE("%s:unmap failed. status %d\n", __func__, status);
    }
    if (shmem_handle->heap_fd)
    {
        close(shmem_handle->heap_fd);
    }
    else
    {
        ALOGE("%s Invalid dmabuf_fd \n", __func__);
    }
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
        ALOGE("%s not in init state\n", __func__);
        pthread_mutex_unlock(&ams_shmem_lock);
        return AMS_EOK;
    }

    if (pdata->dmabuf_fd)
    {
        close(pdata->dmabuf_fd);
    }

    if (pdata->ion_fd)
    {
        close(pdata->ion_fd);
    }

    if (pdata->dmabuf_fd_cma)
    {
        close(pdata->dmabuf_fd_cma);
    }

    if (pdata->ion_fd_cma)
    {
        close(pdata->ion_fd_cma);
    }

    free(pdata);
    pdata = NULL;
    pthread_mutex_unlock(&ams_shmem_lock);

    return AMS_EOK;
}

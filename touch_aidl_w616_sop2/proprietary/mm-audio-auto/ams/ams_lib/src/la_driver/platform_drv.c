/**
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/procfs.h>
#include <sys/mman.h>

#include "ams.h"
#include "ams_platform.h"
#include "ams_util.h"
#include "ams_client_proxy.h"
#ifdef AMS_SSR_SUPPORT_EN
#include "ams_osal_ssr.h"
#endif

#define ALIGND(x, y) ((x + (y - 1)) & ~(y - 1))

static int32_t platform_drv_ercode = 0;
#define AMS_CORE_DRV_MEM_ALLOC
int32_t platform_drv_ready(
    struct platform_drv *pdrv)
{
    uint32_t handle;
    int32_t r;
    handle = ams_core_open();
    if (handle)
    {
        pdrv->h = handle;
        r = 1;
    }
    else
    {
        r = 0;
    }
    return r;
}

ams_status_t ams_platform_drv_open(
    struct platform_drv *pdrv)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    platform_drv_ercode = ams_core_init();
    if (0 != platform_drv_ercode)
    {
        AMS_LIB_LOGE("ams initialization failed");
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }

    // init handle
    pdrv->h = ams_core_open();
    if (pdrv->h == 0)
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("ams open ret NULL platform handle");
        goto exit;
    }
    // set length to default value
    r = ams_platform_drv_set_smem_size(pdrv, AMS_SMEM_BUF_SIZE);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Error when setting smem size!");
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }
exit:
    return r;
}

ams_status_t ams_platform_drv_close(
    struct platform_drv *pdrv)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    uint32_t ri = 0;
    if (pdrv == NULL)
    {
        AMS_LIB_LOGE("Null platform handle!");
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    // close handle
    if (pdrv->h)
        ri = ams_core_close(pdrv->h);
    if (ri)
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("ams close ret %d", ri);
    }

exit:
    platform_drv_ercode = ri;
    return r;
}

ams_status_t ams_platform_drv_util_check_fw_version(
    struct platform_drv *pdrv)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    if (pdrv == NULL)
    {
        AMS_LIB_LOGE("Null platform handle!");
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    if (pdrv->version >= AMS_FW_VERSION_MIN)
    {
        r = AMS_STATUS_SUCCESS;
    }
    else
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("Wrong fw version %d, min expected %d", pdrv->version, AMS_FW_VERSION_MIN)
    }
exit:
    return r;
}

ams_status_t ams_platform_drv_ioctl(
    struct platform_drv *pdrv,
    uint32_t cmd,
    void *params,
    uint32_t size)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    uint32_t ri = 0;
    if (pdrv == NULL)
    {
        AMS_LIB_LOGE("Null platform handle!");
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    ri = ams_core_ioctl(pdrv->h, cmd, params, size);
    if (ri != DSP_AMS_STATUS_OK)
    {
        AMS_LIB_LOGE("ams core ioctl ret %d", ri);
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }

exit:
    platform_drv_ercode = ri;
    return r;
}
ams_status_t ams_platform_drv_set_smem_size(
    struct platform_drv *pdrv,
    uint32_t size)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    if (pdrv == NULL)
    {
        AMS_LIB_LOGE("Null platform handle!");
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    if (size == 0)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Null memory size provided!");
        goto exit;
    }
    pdrv->smem.len = ALIGND(size, PMEM_PAGESIZE);
exit:

    return r;
}

ams_status_t ams_platform_drv_alloc_smem(
    struct platform_drv *pdrv)
{
    ams_status_t r = AMS_STATUS_SUCCESS;

    ams_shmem_alloc_t param = {0};
    param.req.len = pdrv->smem.len; // must be multiple of 4K!
    param.req.client_id = 0;
    void *pmem = NULL;
    // param.req.pid = getpid(); // PORTME

    r = ams_platform_drv_ioctl(pdrv, AMS_CORE_CMD_ALLOC_SHMEM, &param, sizeof(param));

    if (r == AMS_STATUS_SUCCESS)
    {
        pdrv->smem.mem_map_handle = param.resp.mem_map_handle;
        pdrv->smem.pa_lsw = param.resp.pa_lsw;
        pdrv->smem.pa_msw = param.resp.pa_msw;
        pdrv->smem.ion_mem_fd = param.resp.heap_fd;
        pmem = mmap(NULL, pdrv->smem.len,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED, param.resp.heap_fd, 0);
        if (!pmem)
        {
            AMS_LIB_LOGD("Cannot map fd!");
        }
        else
        {
            pdrv->smem.address_lsw = (uint32_t)((uint64_t)pmem & (0xffffffff));
            pdrv->smem.address_msw = (uint32_t)((uint64_t)pmem >> 32);
        }
        AMS_LIB_LOGD("va_lsw=%08x", param.resp.va_lsw);
        AMS_LIB_LOGD("va_msw=%08x", param.resp.va_msw);
        AMS_LIB_LOGD("mem_map_handle=%08x", param.resp.mem_map_handle);
        AMS_LIB_LOGD("ion_mem_fd=%08x", param.resp.heap_fd);
    }
    else
    {
        AMS_LIB_LOGE("Shmem allocation by audio driver failed (err=%d)!", r);
    }
    return r;
}

ams_status_t ams_platform_drv_free_smem(
    struct platform_drv *pdrv)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    ams_shmem_free_t param = {0};
    // param.pid = getpid(); // PORTME
    if (!pdrv || !pdrv->smem.mem_map_handle)
        goto exit;
    param.va_lsw = pdrv->smem.address_lsw;
    param.va_msw = pdrv->smem.address_msw;
    param.mem_map_handle = pdrv->smem.mem_map_handle;
    param.pa_lsw = pdrv->smem.pa_lsw;
    param.pa_msw = pdrv->smem.pa_msw;

    r = ams_platform_drv_ioctl(pdrv, AMS_CORE_CMD_FREE_SHMEM, &param, sizeof(param));
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Shmem freeing by audio driver failed (err=%d)!", r);
    }
    else
    {
        pdrv->smem.address_lsw = 0;
        pdrv->smem.address_msw = 0;
        pdrv->smem.mem_map_handle = 0;
        pdrv->smem.pa_lsw = 0;
        pdrv->smem.pa_msw = 0;
        pdrv->smem.len = 0;
    }
exit:
    return r;
}

#ifdef AMS_SSR_SUPPORT_EN
ams_status_t ams_platform_drv_ssr_init(
    char *client_name,
    struct ssr_drv *ssr,
    int (*event_handler)(uint32_t ss_id_mask, uint32_t event_id, void *ctx),
    void *ctx)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    int32_t ri = 0;

    ri = ams_osal_ssr_init(client_name, event_handler, ctx);

    if (ri){
        AMS_LIB_LOGE("SSR setup failed!");
        r = AMS_STATUS_GENERAL_ERROR;
    } else {
        ssr->hssr  = (void *)1;//dummy handle
    }

    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Failed to Register Callback function!");
    }
    else
    {
        AMS_LIB_LOGD("Register Callback function Successful, handle =%ld.", (uint64_t)ssr->hssr);
    }
    return r;
}

ams_status_t ams_platform_drv_ssr_deinit(struct ssr_drv *ssr)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    int32_t ri = 0;
    if (!ssr->hssr)
        goto exit;
    ri = ams_osal_ssr_deinit();
    ssr->hssr  = (void *)NULL;//dummy handle
    if (ri){
        AMS_LIB_LOGE("Failed to unregister from SSR!");
        r = AMS_STATUS_GENERAL_ERROR;
    }
exit:
    return r;
}
#else
ams_status_t ams_platform_drv_ssr_init(
    char *client_name,
    struct ssr_drv *ssr,
    int (*event_handler)(uint32_t ss_id_mask, uint32_t event_id, void *ctx),
    void *ctx)
{
    return 0;
}
ams_status_t ams_platform_drv_ssr_deinit(struct ssr_drv *ssr)
{
    return 0;
}
#endif

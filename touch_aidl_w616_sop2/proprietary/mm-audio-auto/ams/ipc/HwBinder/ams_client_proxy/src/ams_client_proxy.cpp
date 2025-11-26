/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#define LOG_TAG "ams_client_proxy"
#include <log/log.h>
#include <pthread.h>

#include <vendor/qti/hardware/AMSIPC/1.0/IAMS.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <stdint.h>

using ::android::sp;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;

#include "ams_core_ioctl.h"
#include "ams_client_proxy.h"
#include "ams_osal_error.h"

using android::hardware::hidl_vec;
using android::hardware::Return;
using vendor::qti::hardware::AMSIPC::V1_0::IAMS;

// static bool ams_server_died = false;
static pthread_mutex_t amsclient_init_lock = PTHREAD_MUTEX_INITIALIZER;
static android::sp<IAMS> ams_client = NULL;

android::sp<IAMS> get_ams_server()
{
    pthread_mutex_lock(&amsclient_init_lock);
    if (ams_client == NULL)
    {
        ams_client = IAMS::getService();
        if (ams_client == nullptr)
        {
            ALOGE("AMS service not initialized\n");
            goto done;
        }
        // if(Server_death_notifier == NULL)
        // {
        //     Server_death_notifier = new server_death_notifier();
        //     agm_client->linkToDeath(Server_death_notifier,0);
        //     ALOGI("%s : server linked to death \n", __func__);
        // }
    }
done:
    pthread_mutex_unlock(&amsclient_init_lock);
    return ams_client;
}

int32_t ams_core_init(void)
{
    int32_t ret = AMS_EOK;
    ALOGD("%s:", __func__);
    android::sp<IAMS> ams_client = get_ams_server();
    if (ams_client)
    {
        ret = ams_client->ipc_ams_init();
    }
    else
    {
        ALOGD("%s No AMS Server available %d:", __func__, ret);
        ret = AMS_EFAILED;
    }
    ALOGD("%s ret %d:", __func__, ret);
    return ret;
}

uint32_t ams_core_open(void)
{
    uint32_t ret=AMS_EOK;
    ALOGD("%s:", __func__);
    android::sp<IAMS> ams_client = get_ams_server();
    if (ams_client)
    {
        ret = ams_client->ipc_ams_open();
    }
    else
    {
        ALOGE("%s No AMS Server available %d:", __func__, ret);
        ret = AMS_EFAILED;
    }
    ALOGD("%s ret %d:", __func__, ret);
    return ret;
}

int32_t ams_core_ioctl(uint32_t handle, uint32_t cmd, void *params, uint32_t size)
{
    int32_t ret=AMS_EOK;
    android::sp<IAMS> ams_client = get_ams_server();
    if (!ams_client)
    {
        ALOGE("%s No AMS Server available %d:", __func__, ret);
        ret = AMS_EFAILED;
        goto exit;
    }
    ALOGD("%s:cmd %08x, size %d", __func__, cmd, size);
    switch (cmd)
    {
    case AMS_CORE_CMD_TUNNEL:
    {
        hidl_vec<uint8_t> req_l;
        ams_core_tunnel_req_t *req = ((ams_core_tunnel_t *)params)->req;
        uint32_t req_sz = req->payload_size + sizeof(ams_core_tunnel_req_t);
        ams_core_tunnel_rsp_t *rsp = ((ams_core_tunnel_t *)params)->rsp;
        // set response size from request
        uint32_t rsp_sz = ((ams_core_tunnel_t *)params)->req->response_size + sizeof(ams_core_tunnel_rsp_t);
        req_l.resize(req_sz);
        memcpy(req_l.data(), req, req_sz);
        auto status = ams_client->ipc_ams_ioctl_out2(handle,
                                                     (::vendor::qti::hardware::AMSIPC::V1_0::ipc_ams_cmd_e)cmd, req_l, req_sz,
                                                     [&](int32_t _ret, hidl_vec<uint8_t> _respbuf, uint32_t _respbuf_sz)
                                                     {
                                                         ret = _ret;
                                                         if (!ret)
                                                         {
                                                             if (params != nullptr)
                                                             {
                                                                 memcpy(((ams_core_tunnel_t *)params)->rsp, _respbuf.data(), _respbuf_sz);
                                                             }
                                                         }
                                                     });
        if (ret)
        {
            ALOGE("%s: HIDL call failed. ret=%d\n", __func__, ret);
        }
        break;
    }
    case AMS_CORE_CMD_GET_CACHED_GRAPH_INFO:
    case AMS_CORE_CMD_GET_CACHED_GRAPH_NUM:
    case AMS_CORE_CMD_GET_CACHED_GRAPH_DESCR:
    case AMS_CORE_CMD_OPERATE_DEVICE_QUERY:
    {
        // ioctl3
        hidl_vec<uint8_t> req_l;
        req_l.resize(size);
        memcpy(req_l.data(), params, size);
        auto status = ams_client->ipc_ams_ioctl_out3(handle,
                                                     (::vendor::qti::hardware::AMSIPC::V1_0::ipc_ams_cmd_e)cmd, req_l, size,
                                                     [&](int32_t _ret, hidl_vec<uint8_t> _respbuf, uint32_t _respbuf_sz)
                                                     {
                                                         ret = _ret;
                                                         if (!ret)
                                                         {
                                                             if (params != nullptr)
                                                             {
                                                                 ALOGD("%s: copy respbuf size=%d\n", __func__, _respbuf_sz);
                                                                 memcpy(params, _respbuf.data(), _respbuf_sz);
                                                             }
                                                         }
                                                     });
        if (ret)
        {
            ALOGE("%s: HIDL call failed. ret=%d\n", __func__, ret);
        }
        break;
    }
    case AMS_CORE_CMD_ALLOC_SHMEM:
    {
        ams_core_shmem_alloc_t *buf_alloc = (ams_core_shmem_alloc_t *)params;
        hidl_vec<uint8_t> req_l;
        const native_handle *datahandle = nullptr;
        uint32_t req_sz = size; // size=struct+payload
        req_l.resize(size);
        memcpy(req_l.data(), params, size);
        auto status = ams_client->ipc_ams_alloc_shmem_buf(handle, req_l, req_sz,
                                                          [&](int32_t _ret, hidl_vec<::vendor::qti::hardware::AMSIPC::V1_0::ipc_ams_shmem_info> ret_buf_info_hidl)
                                                          {
                                                              ret = _ret;
                                                              if (!ret)
                                                              {
                                                                  datahandle = ret_buf_info_hidl.data()->shmem.handle();
                                                                  buf_alloc->resp.heap_fd = dup(datahandle->data[0]);
                                                                  buf_alloc->resp.mem_map_handle = ret_buf_info_hidl.data()->mem_map_handle;
                                                              }
                                                          });
        if (ret)
        {
            ALOGE("%s: HIDL call failed. ret=%d\n", __func__, ret);
        }
        break;
    }
    case AMS_CORE_CMD_ENABLE_ENDPOINT:
    case AMS_CORE_CMD_DISABLE_ENDPOINT:
    case AMS_CORE_CMD_SET_CLK_ATTR:
    case AMS_CORE_CMD_FREE_SHMEM:
    {
        // ioctl1
        hidl_vec<uint8_t> req_l;
        req_l.resize(size);
        memcpy(req_l.data(), params, size);
        ret = ams_client->ipc_ams_ioctl_out1(handle,
                                             (::vendor::qti::hardware::AMSIPC::V1_0::ipc_ams_cmd_e)cmd, req_l, size);
        if (ret)
        {
            ALOGE("%s: HIDL call failed. ret=%d\n", __func__, ret);
        }
        break;
    }
    default:
    {
        ALOGE("%s: Unknown ioctl command!\n", __func__);
        ret = EOPNOTSUPP;
        break;
    }
    }
exit:
    return ret;
}

int32_t ams_core_close(uint32_t hndl)
{
    int32_t ret=AMS_EOK;
    ALOGD("%s:", __func__);
    android::sp<IAMS> ams_client = get_ams_server();
    if (ams_client)
    {
        ret = ams_client->ipc_ams_close(hndl);
    }
    else
    {
        ALOGD("%s No AMS Server available %d:", __func__, ret);
        ret = AMS_EFAILED;
    }
    ALOGD("%s:ret %d", __func__, ret);
    return ret;
}
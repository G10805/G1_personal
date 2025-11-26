/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#include <log/log.h>
#include "AMS.h"
// #include "ams_core.h"
#include "ams_core_ioctl.h"
#include <vendor/qti/hardware/AMSIPC/1.0/IAMS.h>
namespace vendor::qti::hardware::AMSIPC::implementation
{

    // Methods from ::vendor::qti::hardware::AMSIPC::V1_0::IAMS follow.
    Return<int32_t> AMS::ipc_ams_init()
    {
        int32_t ret = 0;
        if (!AMS::ams_initialized)
        {
            ret = ams_core_init();
            AMS::ams_initialized = ret == 0 ? true : false;
            ALOGE("AMS initialized ret %d.\n", ret);
        }
        else
        {
            ALOGD("AMS alreaydy initialized.\n");
        }

        return ret;
    }

    Return<int32_t> AMS::ipc_ams_deinit()
    {
        int32_t ret = 0;
        if (AMS::ams_initialized)
        {
            ret = ams_core_deinit();
            AMS::ams_initialized = false;
        }
        else
        {
            ALOGE("AMS not initialized!\n");
            ret = EINVAL;
        }
        return ret;
    }

    Return<uint32_t> AMS::ipc_ams_open()
    {
        uint32_t ret = 0;
        if (AMS::ams_initialized)
            ret = ams_core_open();
        else
        {
            ALOGE("%s: AMS Core not initialized!\n");
            ret = EINVAL;
        }
        return ret;
    }

    Return<int32_t> AMS::ipc_ams_close(uint32_t hndl)
    {
        int32_t ret = 0;
        if (AMS::ams_initialized)
        {
            ret = ams_core_close(hndl);
        }
        else
        {
            ALOGE("%s: AMS Core not initialized!\n");
            ret = EINVAL;
        }
        return ret;
    }

    Return<int32_t> AMS::ipc_ams_ioctl_out1(uint32_t hndl, ::vendor::qti::hardware::AMSIPC::V1_0::ipc_ams_cmd_e cmd, const hidl_vec<uint8_t> &params, uint32_t sz)
    {
        int32_t ret = 0;
        if (!AMS::ams_initialized)
        {
            ALOGE("%s: AMS Core not initialized!\n");
            return EINVAL;
        }
        void *param_l = calloc(1, sz);
        if (param_l == nullptr)
        {
            ALOGE("%s: Cannot allocate memory for param_l\n", __func__);
            return ENOMEM;
        }
        memcpy(param_l, params.data(), sz);
        ret = ams_core_ioctl(hndl, (uint32_t)cmd, (void *)param_l, sz);
        free(param_l);
        return ret;
    }

    Return<void> AMS::ipc_ams_ioctl_out3(uint32_t hndl, ::vendor::qti::hardware::AMSIPC::V1_0::ipc_ams_cmd_e cmd,
                                         const hidl_vec<uint8_t> &params, uint32_t sz, ipc_ams_ioctl_out3_cb _hidl_cb)
    {
        // Not needed(can be replaced by _out4)?
        int32_t ret = 0;
        hidl_vec<uint8_t> resp_ret;
        uint32_t resp_ret_sz = 0;
        if (!AMS::ams_initialized)
        {
            ALOGE("%s: AMS Core not initialized!\n");
            return Void();
        }
        void *param_l = calloc(1, sz);
        if (param_l == nullptr)
        {
            ALOGE("%s: Cannot allocate memory for param_l\n", __func__);
            // cleanup
            _hidl_cb(ENOMEM, resp_ret, resp_ret_sz);
            return Void();
        }
        memcpy(param_l, params.data(), sz);
        ret = ams_core_ioctl(hndl, (uint32_t)cmd, (void *)param_l, sz);
        resp_ret_sz = sz;
        resp_ret.resize(resp_ret_sz);
        memcpy(resp_ret.data(), param_l, resp_ret_sz);
        _hidl_cb(ret, resp_ret, resp_ret_sz);
        free(param_l);
        return Void();
    }

    Return<void> AMS::ipc_ams_ioctl_out2(uint32_t hndl, ::vendor::qti::hardware::AMSIPC::V1_0::ipc_ams_cmd_e cmd,
                                         const hidl_vec<uint8_t> &req, uint32_t req_sz,
                                         ipc_ams_ioctl_out2_cb _hidl_cb)
    {
        // implement tunnel command
        int32_t ret = 0;
        ams_core_tunnel_req_t *req_l;
        ams_core_tunnel_rsp_t *rsp_l;
        ams_core_tunnel_t param_l = {0};
        hidl_vec<uint8_t> resp_ret;
        uint32_t resp_ret_sz = 0;
        if (!AMS::ams_initialized)
        {
            ALOGE("%s: AMS Core not initialized!\n");
            return Void();
        }
        // TODO: check if req_sz=sizeof(ams_core_tunnel_rsp_t+payload_size)
        req_l = (ams_core_tunnel_req_t *)calloc(1, req_sz);
        if (req_l == nullptr)
        {
            ALOGE("%s: Cannot allocate memory for req_l\n", __func__);
            // cleanup
            _hidl_cb(ENOMEM, resp_ret, resp_ret_sz);
            return Void();
        }
        memcpy(req_l, req.data(), req_sz);

        rsp_l = (ams_core_tunnel_rsp_t *)calloc(1, req_l->response_size + sizeof(ams_core_tunnel_rsp_t));
        if (rsp_l == nullptr)
        {
            ALOGE("%s: Cannot allocate memory for rsp_l\n", __func__);
            // cleanup
            free(req_l);
            _hidl_cb(ENOMEM, resp_ret, resp_ret_sz);
            return Void();
        }
        // memcpy(rsp_l, rsp.data(), rsp_sz);
        rsp_l->response_size = req_l->response_size;
        param_l.req = req_l;
        param_l.rsp = rsp_l;

        ret = ams_core_ioctl(hndl, (uint32_t)cmd, (void *)&param_l, sizeof(param_l));
        resp_ret_sz = param_l.rsp->response_size + sizeof(ams_core_tunnel_rsp_t);
        resp_ret.resize(resp_ret_sz);
        memcpy(resp_ret.data(), param_l.rsp, resp_ret_sz);

        _hidl_cb(ret, resp_ret, resp_ret_sz);
        free(req_l);
        free(rsp_l);
        return Void();
    }

    Return<void> AMS::ipc_ams_alloc_shmem_buf(uint32_t hndl, const hidl_vec<uint8_t> &req, uint32_t req_sz,
                                              ipc_ams_alloc_shmem_buf_cb _hidl_cb)
    {

        ALOGV("%s : hndl = %d, size = %d\n", __func__, hndl, req_sz);
        int32_t ret = -1;
        ams_core_shmem_alloc_t buf_info_local = {0};
        hidl_vec<::vendor::qti::hardware::AMSIPC::V1_0::ipc_ams_shmem_info> out_buf_hidl;
        native_handle_t *dataHidlHandle = nullptr;
        if (!AMS::ams_initialized)
        {
            ALOGE("%s: AMS Core not initialized!\n");
            return Void();
        }
        memcpy(&buf_info_local, req.data(), req_sz);
        // buf_info_local.req.client_id = buf->req.client_id;
        // buf_info_local.req.len = buf->req.len;
        // buf_info_local.req.pid = buf->req.pid;

        ret = ams_core_ioctl(hndl, AMS_CORE_CMD_ALLOC_SHMEM, &buf_info_local, sizeof(buf_info_local));
        if (ret == 0)
        {
            out_buf_hidl.resize(sizeof(::vendor::qti::hardware::AMSIPC::V1_0::ipc_ams_shmem_info));
            dataHidlHandle = native_handle_create(1, 0);
            if (!dataHidlHandle)
            {
                ALOGE("%s native_handle_create fails", __func__);
                goto exit;
            }
            dataHidlHandle->data[0] = buf_info_local.resp.heap_fd;
            out_buf_hidl.data()->shmem = hidl_memory("ams_shmem_data_buf",
                                                     hidl_handle(dataHidlHandle), buf_info_local.req.len);
            out_buf_hidl.data()->mem_map_handle = buf_info_local.resp.mem_map_handle;
            out_buf_hidl.data()->size = buf_info_local.req.len;
        }

        _hidl_cb(ret, out_buf_hidl);

    exit:
        if (dataHidlHandle != nullptr)
            native_handle_delete(dataHidlHandle);
        return Void();
    }

    // Methods from ::android::hidl::base::V1_0::IBase follow.

    // IAMS* HIDL_FETCH_IAMS(const char* /* name */) {
    // return new AMS();
    //}
    //
} // namespace vendor::qti::hardware::AMSIPC::implementation

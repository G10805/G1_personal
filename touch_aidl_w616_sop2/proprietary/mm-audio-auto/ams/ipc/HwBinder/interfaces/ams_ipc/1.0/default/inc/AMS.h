#pragma once
/**
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#include <vendor/qti/hardware/AMSIPC/1.0/IAMS.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include "ams_core.h"
namespace vendor::qti::hardware::AMSIPC::implementation
{

    using ::android::sp;
    using ::android::hardware::hidl_array;
    using ::android::hardware::hidl_handle;
    using ::android::hardware::hidl_memory;
    using ::android::hardware::hidl_string;
    using ::android::hardware::hidl_vec;
    using ::android::hardware::Return;
    using ::android::hardware::Void;

    struct AMS : public V1_0::IAMS
    {
        // Methods from ::vendor::qti::hardware::AMSIPC::V1_0::IAMS follow.
    public:
        AMS()
        {
            ams_initialized = false;
        }
        Return<int32_t> ipc_ams_init() override;
        Return<int32_t> ipc_ams_deinit() override;
        Return<uint32_t> ipc_ams_open() override;
        Return<int32_t> ipc_ams_close(uint32_t hndl) override;
        Return<int32_t> ipc_ams_ioctl_out1(uint32_t hndl, ::vendor::qti::hardware::AMSIPC::V1_0::ipc_ams_cmd_e cmd, const hidl_vec<uint8_t> &params, uint32_t sz) override;
        Return<void> ipc_ams_ioctl_out3(uint32_t hndl, ::vendor::qti::hardware::AMSIPC::V1_0::ipc_ams_cmd_e cmd, const hidl_vec<uint8_t> &params, uint32_t sz, ipc_ams_ioctl_out3_cb _hidl_cb) override;
        Return<void> ipc_ams_ioctl_out2(uint32_t hndl, ::vendor::qti::hardware::AMSIPC::V1_0::ipc_ams_cmd_e cmd, const hidl_vec<uint8_t> &req, uint32_t req_sz, ipc_ams_ioctl_out2_cb _hidl_cb) override;
        Return<void> ipc_ams_alloc_shmem_buf(uint32_t hndl, const hidl_vec<uint8_t> &req, uint32_t req_sz, ipc_ams_alloc_shmem_buf_cb _hidl_cb) override;
        // Methods from ::android::hidl::base::V1_0::IBase follow.
        int is_ams_initialized() { return ams_initialized; }

    private:
        // sp<client_death_notifier> mDeathNotifier = NULL;
        bool ams_initialized;
    };

    // FIXME: most likely delete, this is only for passthrough implementations
    // extern "C" IAMS* HIDL_FETCH_IAMS(const char* name);

} // namespace vendor::qti::hardware::AMSIPC::implementation

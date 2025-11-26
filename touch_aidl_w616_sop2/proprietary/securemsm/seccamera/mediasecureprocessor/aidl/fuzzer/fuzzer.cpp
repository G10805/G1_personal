/**
 * Copyright (c) 2020-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include "SecureProcessorQTEEMink.h"

#include <fuzzbinder/libbinder_ndk_driver.h>
#include <fuzzer/FuzzedDataProvider.h>
#include <log/log.h>

using ::aidl::vendor::qti::hardware::secureprocessor::device::SecureProcessorQTEEMink;

std::shared_ptr<SecureProcessorQTEEMink> service;

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv) {
    service = ndk::SharedRefBase::make<SecureProcessorQTEEMink>("seccamdemo2");
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (service == nullptr) return 0;

    android::fuzzService(service->asBinder().get(),FuzzedDataProvider(data,size));

    return 0;
}

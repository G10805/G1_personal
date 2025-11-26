/**
 * Copyright (c) 2020-2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#include <aidl/vendor/qti/hardware/trustedui/ITrustedInput.h>
#include <aidl/vendor/qti/hardware/trustedui/ITrustedUI.h>

#include "TrustedUIFactory.h"

#include <fuzzbinder/libbinder_ndk_driver.h>
#include <fuzzer/FuzzedDataProvider.h>
#include <log/log.h>

using aidl::vendor::qti::hardware::trustedui::ITrustedUI;
using aidl::vendor::qti::hardware::trustedui::ITrustedInput;
using aidl::vendor::qti::hardware::trustedui::TrustedUIFactory;
using aidl::vendor::qti::hardware::trustedui::TrustedInputFactory;

std::shared_ptr<ITrustedUI> serviceUI;
std::shared_ptr<ITrustedInput> serviceInput;

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv) {
    const std::string implementation = "default";
    serviceUI = TrustedUIFactory::GetInstance(implementation);
    serviceInput = TrustedInputFactory::GetInstance(implementation);;
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if ( serviceUI == nullptr || serviceInput == nullptr) return 0;

    FuzzedDataProvider provider = FuzzedDataProvider(data,size);
    uint32_t index = provider.ConsumeIntegralInRange(0,1);

    if (index == 0 ) {
        android::fuzzService(serviceUI->asBinder().get(),std::move(provider));
    } else if (index == 1) {
        android::fuzzService(serviceInput->asBinder().get(),std::move(provider));
    }

    return 0;
}
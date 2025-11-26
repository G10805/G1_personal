/* ===========================================================================
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#include <android/binder_process.h>
#include <android/binder_manager.h>
#include <android-base/logging.h>
#include <cutils/properties.h>
#include <iostream>
#include <algorithm>
#include <memory>
#include <hidl/HidlLazyUtils.h>
#include <hidl/HidlTransportSupport.h>
#include "PowerPolicyService.h"

#if (defined (__GNUC__) && !defined(__INTEGRITY))
#define PUBLIC_API __attribute__ ((visibility ("default")))
#else
#define PUBLIC_API
#endif

using ::aidl::android::frameworks::automotive::powerpolicy::CarPowerPolicy;
using ::aidl::android::frameworks::automotive::powerpolicy::CarPowerPolicyFilter;
using ::aidl::android::frameworks::automotive::powerpolicy::ICarPowerPolicyChangeCallback;
using ::aidl::android::frameworks::automotive::powerpolicy::ICarPowerPolicyServer;
using ::aidl::android::frameworks::automotive::powerpolicy::PowerComponent;

using ::ndk::ScopedAStatus;
using ::ndk::SpAIBinder;
using ::ndk::SharedRefBase;

constexpr PowerComponent kCpuComponent     = PowerComponent::CPU;
constexpr PowerComponent kDisplayComponent = PowerComponent::DISPLAY;

bool PowerPolicyService::hasPowerComponent(
          const std::vector<::aidl::android::frameworks::automotive::powerpolicy::PowerComponent>& components,
          ::aidl::android::frameworks::automotive::powerpolicy::PowerComponent ComponentName) {

    std::vector<PowerComponent>::const_iterator it =
         std::find(components.cbegin(), components.cend(), ComponentName);
    return (it != components.cend());

}

const char* kPowerPolicyDaemon =
          "android.frameworks.automotive.powerpolicy.ICarPowerPolicyServer/default";

::ndk::ScopedAStatus PowerPolicyService::onPolicyChanged(const CarPowerPolicy& powerPolicy) {
    int ret = 0;

    LOG(ERROR) << "Got a PowerPolicy change Event calling the callback";

    if (mEventCallback == NULL) {
        LOG(ERROR) << " mEventCallback is NULL no function assigned";
        return ScopedAStatus::ok();
    }

    if(hasPowerComponent(powerPolicy.enabledComponents,kDisplayComponent)) {
        LOG(ERROR) << "Got a PowerPolicy change Event calling AIS_PM_RESUME";
        ret = mEventCallback(AIS_PM_RESUME, mUsrCtxt);
    }
    else if(hasPowerComponent(powerPolicy.disabledComponents, kDisplayComponent)) {
        LOG(ERROR) << "Got a PowerPolicy change Event calling AIS_PM_SUSPEND";
        ret = mEventCallback(AIS_PM_SUSPEND, mUsrCtxt);
    }

    if(ret == 0) {
        LOG(ERROR) << "Completed the callback event.";
        return ScopedAStatus::ok();
    }
    else {
        LOG(ERROR) << "Got the Failure from the callback.";
        return ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }
}

int PowerPolicyService::RegisterPowerEventCb(PowerEventCb Callback, void *usr_ctxt){
    LOG(ERROR) << " Registering the Callback for the PowerEventCb \n";
    mEventCallback = Callback;
    mUsrCtxt = usr_ctxt;

    return 0;
}

PowerPolicyService::PowerPolicyService(){
    mEventCallback = NULL;
    mUsrCtxt = NULL;
}

PowerPolicyService::~PowerPolicyService() {
    LOG(ERROR) << "Calling PowerPolicyService Destructor %lld", *((uint64_t*)(mUsrCtxt));
    mEventCallback = NULL;
    mUsrCtxt = NULL;
}

bool PowerPolicyService::Init() {
    ndk::SpAIBinder binder(AServiceManager_getService(kPowerPolicyDaemon));

    if (binder.get() == nullptr) {
        return false;
    }

    std::shared_ptr<ICarPowerPolicyServer> server = ICarPowerPolicyServer::fromBinder(binder);
    if (server == nullptr) {
        LOG(ERROR) << "Failed to connect to car power policy daemon";
        return false;
    }

    // This class is implementing ICarPowerPolicyChangeCallback and used as client.
    binder = this->asBinder();
    if (binder.get() == nullptr) {
        LOG(ERROR) << "Failed to get car power policy client binder object";
         return false;
    }

    std::shared_ptr<ICarPowerPolicyChangeCallback> client =
            ICarPowerPolicyChangeCallback::fromBinder(binder);
    if (client == nullptr) {
        LOG(ERROR) << "Failed to get ICarPowerPolicyChangeCallback from binder";
        return false;
    }

    CarPowerPolicyFilter filter;
    filter.components.push_back(kCpuComponent);
    filter.components.push_back(kDisplayComponent);

    // Register the power policy callback to the daemon
    server->registerPowerPolicyChangeCallback(client, filter);
    LOG(ERROR) << "Successfully registered the client to car power policy daemon";

    return true;
}


PUBLIC_API std::shared_ptr<PowerPolicyService> PowerEventInit(PowerEventCb cb, void *pUsrCtxt) {
    int ret = 0;

    LOG(ERROR) << "POWER SERVICE Starting\n";

    // Create and initialize a power policy client.
    std::shared_ptr<PowerPolicyService> gPowerService = ndk::SharedRefBase::make<PowerPolicyService>();
    if (!gPowerService->Init()) {
        LOG(ERROR) << "Failed to initialize power policy client";
        ret = -1;
        return nullptr;
    }

    ret = gPowerService->RegisterPowerEventCb(cb, pUsrCtxt);
    if (ret != 0) {
        LOG(ERROR) << "POWER SERVICE Register of callback Failed";
        ret = -2;
    }

    LOG(ERROR) << "POWER SERVICE Ending\n";

    return gPowerService;
}

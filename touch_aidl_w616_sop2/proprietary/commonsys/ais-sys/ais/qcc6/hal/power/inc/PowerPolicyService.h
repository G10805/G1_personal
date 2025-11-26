/* ===========================================================================
 *Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *All rights reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#ifndef _POWER_POLICY_SERVICE_H_
#define _POWER_POLICY_SERVICE_H_

#include <aidl/android/frameworks/automotive/powerpolicy/BnCarPowerPolicyChangeCallback.h>
#include <aidl/android/frameworks/automotive/powerpolicy/BnCarPowerPolicyServer.h>
#include <aidl/android/frameworks/automotive/powerpolicy/CarPowerPolicyFilter.h>
#include <aidl/android/frameworks/automotive/powerpolicy/CarPowerPolicy.h>
#include <android-base/result.h>
#include <utils/StrongPointer.h>

#include <shared_mutex>
#include <thread>
#include <vector>

#include "PowerEvent.h"

using ::std::string;
using ::std::vector;

class PowerPolicyService : public
     ::aidl::android::frameworks::automotive::powerpolicy::BnCarPowerPolicyChangeCallback {
public:
    PowerEventCb mEventCallback;
    void *mUsrCtxt;

    PowerPolicyService();
    ~PowerPolicyService();

    bool hasPowerComponent(const
        std::vector<::aidl::android::frameworks::automotive::powerpolicy::PowerComponent>& components,
        ::aidl::android::frameworks::automotive::powerpolicy::PowerComponent ComponentName );

    ::ndk::ScopedAStatus onPolicyChanged(
        const ::aidl::android::frameworks::automotive::powerpolicy::CarPowerPolicy&) override;

     bool Init();

    int RegisterPowerEventCb(PowerEventCb, void *usr_ctxt);
};

#endif //_POWER_POLICY_SERVICE_H_

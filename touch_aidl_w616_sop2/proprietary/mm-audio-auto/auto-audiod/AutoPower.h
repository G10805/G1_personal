/*
 * Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "PowerPolicyClientBase.h"

class AutoPower : public ::android::frameworks::automotive::powerpolicy::PowerPolicyClientBase {

public:
    AutoPower();
    virtual ~AutoPower();
    void onInitFailed();
    std::vector<::aidl::android::frameworks::automotive::powerpolicy::PowerComponent>
    getComponentsOfInterest() override;
    ::ndk::ScopedAStatus onPolicyChanged(
            const ::aidl::android::frameworks::automotive::powerpolicy::CarPowerPolicy&) override;
    void initialize();
    bool checkPowerPolicy();
private:
    std::shared_ptr<AutoPower> mAutoPower;
};

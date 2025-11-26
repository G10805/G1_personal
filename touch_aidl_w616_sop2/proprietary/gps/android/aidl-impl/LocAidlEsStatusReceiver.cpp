/*
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * Not a Contribution
 */
/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 */
/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "LocSvc_AIDL_AidlEsStatusReceiver"
#define LOG_NDEBUG 0

#include <log_util.h>
#include <OSFramework.h>
#include "LocAidlUtils.h"
#include "LocAidlEsStatusReceiver.h"


using namespace izat_manager;

namespace aidl {
namespace vendor {
namespace qti {
namespace gnss {
namespace implementation {

void EsStatusReceiverDied(void* cookie) {
    LOC_LOGe("LocAidlEsStatusReceiver died.");
}

class EsListener : public IOSListener {
    std::shared_ptr<ILocAidlEsStatusCallback> mAidlCbIface;
    IzatStreamType mStreamType;
public:
    inline EsListener(const std::shared_ptr<ILocAidlEsStatusCallback>& aidlCbIface,
                    IzatStreamType streamType) :
        IOSListener(), mAidlCbIface(aidlCbIface),
        mStreamType(streamType) {}
    inline ~EsListener() {}

    inline virtual IzatListenerMask listensTo() const override {
        return mStreamType;
    }

    inline std::shared_ptr<ILocAidlEsStatusCallback>& getAidlCbIface() {
        return mAidlCbIface;
    }

    inline virtual void onLocationChanged(
            const izat_manager::IzatLocation* location,
            const izat_manager::IzatLocationStatus) override {};

    virtual void onStatusChanged(
            const izat_manager::IzatProviderStatus status) override;

    inline virtual void onNmeaChanged(const IzatNmea*) override {}
};

void EsListener::onStatusChanged (
        const izat_manager::IzatProviderStatus status) {
    if (mAidlCbIface == nullptr) {
        LOC_LOGE("mHidlCbIface null !");
        return;
    }

    TO_AIDL_CLIENT();
    mAidlCbIface->onEsStatusChanged(IZAT_PROVIDER_EMERGENCY_MODE == status);
}

// Methods from ::vendor::qti::gnss::ILocAidlEsStatusReceiver follow.
::ndk::ScopedAStatus LocAidlEsStatusReceiver::setCallback(
        const std::shared_ptr<ILocAidlEsStatusCallback>& callback, bool* _aidl_return) {
    FROM_AIDL_CLIENT();

    if (mIzatManager != NULL && callback != NULL) {
        mListener = new EsListener(callback, IZAT_STREAM_EMERGENCY_STATUS);

        // Register death recipient
        if (mDeathRecipient == nullptr) {
            mDeathRecipient = make_shared<LocAidlDeathRecipient>(EsStatusReceiverDied);
        }
        mDeathRecipient->registerToPeer(callback->asBinder().get(), this);
        mIzatManager->subscribeListener(mListener);
        *_aidl_return = true;
    } else {
        *_aidl_return = false;
    }
    return ndk::ScopedAStatus::ok();
}

LocAidlEsStatusReceiver::LocAidlEsStatusReceiver() :
        mListener(nullptr), mIzatManager(getIzatManager(OSFramework::getOSFramework())){}

LocAidlEsStatusReceiver::~LocAidlEsStatusReceiver() {
    mDeathRecipient = nullptr;
    auto listener = mListener;
    if (listener != nullptr) {
        mIzatManager->unsubscribeListener(listener);
    }
    mListener = nullptr;
}


}  // namespace implementation
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
}  // namespace aidl

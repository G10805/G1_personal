/*
 * Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#define LOG_TAG "LocSvc_AIDL_IzatProvider"
#define LOG_NDEBUG 0

#include <log_util.h>
#include <OSFramework.h>
#include <IzatManager.h>
#include "LocAidlIzatProvider.h"

using namespace izat_manager;

namespace aidl {
namespace vendor {
namespace qti {
namespace gnss {
namespace implementation {

using ::aidl::vendor::qti::gnss::ILocAidlIzatProviderCallback;
using ::aidl::vendor::qti::gnss::LocAidlIzatStreamType;
using ::aidl::vendor::qti::gnss::LocAidlIzatLocation;
using ::aidl::vendor::qti::gnss::LocAidlIzatProviderStatus;
using ::aidl::vendor::qti::gnss::LocAidlIzatRequest;

class Listener : public IOSListener {
    std::shared_ptr<ILocAidlIzatProviderCallback> mAidlCbIface;
    IzatStreamType mStreamType;
    bool mIsEnabled;
public:
    inline Listener(const std::shared_ptr<ILocAidlIzatProviderCallback>& aidlCbIface,
                    IzatStreamType streamType) :
        IOSListener(), mAidlCbIface(aidlCbIface),
        mStreamType(streamType), mIsEnabled (false) {}
    inline ~Listener() { mIsEnabled = false; }

    inline bool isEnabled() { return mIsEnabled; }
    inline void setEnabled(bool enabled) { mIsEnabled = enabled; }

    inline virtual IzatListenerMask listensTo() const override {
        return mStreamType;
    }

    inline std::shared_ptr<ILocAidlIzatProviderCallback>& getHidlCbIface() {
        return mAidlCbIface;
    }

    virtual void onLocationChanged(
            const izat_manager::IzatLocation* location,
            const izat_manager::IzatLocationStatus) override;

    virtual void onStatusChanged(
            const izat_manager::IzatProviderStatus status) override;

    inline virtual void onNmeaChanged(const IzatNmea*) override {}
};

void Listener::onLocationChanged(
        const izat_manager::IzatLocation * location,
        const izat_manager::IzatLocationStatus) {

    if (mAidlCbIface == nullptr) {
        LOC_LOGE("mAidlCbIface null !");
        return;
    }

    if (mIsEnabled) {
        LocAidlIzatLocation gnssLocation;
        LocAidlUtils::izatLocationToGnssIzatLocation(*location, gnssLocation);

        TO_AIDL_CLIENT();
        mAidlCbIface->onLocationChanged(gnssLocation);
    }
}

void Listener::onStatusChanged (
        const izat_manager::IzatProviderStatus status) {

    if (mAidlCbIface == nullptr) {
        LOC_LOGE("mAidlCbIface null !");
        return;
    }

    if (mIsEnabled) {
        LocAidlIzatProviderStatus gnssStatus = LocAidlUtils::izatStatusToGnssIzatStatus(status);

        TO_AIDL_CLIENT();
        mAidlCbIface->onStatusChanged(gnssStatus);
    }
}

std::shared_ptr<LocAidlDeathRecipient> LocAidlIzatProvider::mDeathRecipient = nullptr;

LocAidlIzatProvider::LocAidlIzatProvider(LocAidlIzatStreamType privderType) :
        mListener(NULL),
        mIzatManager(getIzatManager(OSFramework::getOSFramework())),
        mProviderType(LocAidlUtils::gnssIzatStreamToIzatStreamType(privderType)) {
    if (mIzatManager && (IZAT_STREAM_FUSED == mProviderType)) {
        IzatManager* izatMgr = static_cast<IzatManager*>(mIzatManager);
        izatMgr->setIzatFusedProviderOverride(true);
    }
}

void LocAidlIzatProvider::createIzatRequest(IzatRequest& request,
                                     const LocAidlIzatRequest& gnssRequest) {
    IzatHorizontalAccuracy izatAcc;

    switch (gnssRequest.suggestedHorizontalAccuracy) {
        case LocAidlIzatHorizontalAccuracy::FINE:
            izatAcc = IZAT_HORIZONTAL_FINE;
            break;
        case LocAidlIzatHorizontalAccuracy::BLOCK:
            izatAcc = IZAT_HORIZONTAL_BLOCK;
            break;
        default:
            izatAcc = IZAT_HORIZONTAL_NONE;
            break;
    }

    request.setProvider(mProviderType);
    request.setHorizontalAccuracy(izatAcc);

    request.setNumUpdates(gnssRequest.numUpdates);
    request.setInterval(gnssRequest.timeIntervalBetweenFixes);
    request.setDistance(gnssRequest.smallestDistanceBetweenFixes);
}

void IzatProviderServiceDied(void* cookie) {
    LOC_LOGe("ILocAidlIzatProviderCallback died.");
    auto thiz = static_cast<LocAidlIzatProvider*>(cookie);
    if (nullptr != thiz) {
        thiz->deinit();
    }
}

// Methods from ::aidl::vendor::qti::gnss::IIzatProvider follow.
::ndk::ScopedAStatus LocAidlIzatProvider::init(
        const std::shared_ptr<ILocAidlIzatProviderCallback>& callback, bool* _aidl_return) {
    FROM_AIDL_CLIENT();
    if (mIzatManager != NULL && callback != NULL) {
        mListener = new Listener(callback, mProviderType);
        // Register death recipient
        if (mDeathRecipient == nullptr) {
            mDeathRecipient = make_shared<LocAidlDeathRecipient>(IzatProviderServiceDied);
        }
        mDeathRecipient->registerToPeer(callback->asBinder().get(), this);
        mIzatManager->subscribeListener(mListener);
    }

    *_aidl_return = (mListener != nullptr);
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus LocAidlIzatProvider::deinit() {
    //onDisable();
    FROM_AIDL_CLIENT();
    auto listener = mListener;
    mListener = nullptr;
    if (listener != nullptr) {
        mIzatManager->unsubscribeListener(listener);
    }
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus LocAidlIzatProvider::onEnable(bool* _aidl_return) {
    FROM_AIDL_CLIENT();

    bool success = false;
    if (mListener != NULL) {
        if (!mListener->isEnabled()) {
            mIzatManager->enableProvider(mProviderType);
            mListener->setEnabled(true);
        }
        success = true;
    }

    *_aidl_return = success;
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus LocAidlIzatProvider::onDisable(bool* _aidl_return) {
    FROM_AIDL_CLIENT();

    bool success = false;
    if (mListener != NULL) {
        if (mListener->isEnabled()) {
            mIzatManager->disableProvider(mProviderType);
            mListener->setEnabled(false);
        }
        success = true;
    }

    *_aidl_return = success;
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus LocAidlIzatProvider::onAddRequest(
        const LocAidlIzatRequest& gnssRequest, bool* _aidl_return) {

    FROM_AIDL_CLIENT();

    bool success = false;
    if (mListener != NULL && mListener->isEnabled()) {
        IzatRequest request;
        createIzatRequest(request, gnssRequest);
        mIzatManager->addRequest(&request);
        success = true;
    }

    *_aidl_return = success;
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus LocAidlIzatProvider::onRemoveRequest(
        const LocAidlIzatRequest& gnssRequest, bool* _aidl_return) {

    FROM_AIDL_CLIENT();

    bool success = false;
    if (mListener != NULL && mListener->isEnabled()) {
        IzatRequest request;
        createIzatRequest(request, gnssRequest);
        mIzatManager->removeRequest(&request);
        success = true;
    }

    *_aidl_return = success;
    return ndk::ScopedAStatus::ok();
}

}  // namespace implementation
}  // namespace AIDL
}  // namespace gnss
}  // namespace qti
}  // namespace vendor

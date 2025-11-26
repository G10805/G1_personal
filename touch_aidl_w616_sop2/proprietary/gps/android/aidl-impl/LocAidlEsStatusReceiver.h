/*
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef VENDOR_QTI_GNSS_LOCAIDLESSTATUSRECEIVER_H
#define VENDOR_QTI_GNSS_LOCAIDLESSTATUSRECEIVER_H

#include <aidl/vendor/qti/gnss/BnLocAidlEsStatusReceiver.h>
#include <aidl/vendor/qti/gnss/ILocAidlEsStatusCallback.h>
#include <aidl/vendor/qti/gnss/LocAidlIzatStreamType.h>
#include <IOSListener.h>
#include <IIzatManager.h>
#include <IzatTypes.h>


namespace aidl {
namespace vendor {
namespace qti {
namespace gnss {
namespace implementation {

using ::aidl::vendor::qti::gnss::ILocAidlEsStatusReceiver;
using ::aidl::vendor::qti::gnss::ILocAidlEsStatusCallback;

class EsListener;
struct LocAidlEsStatusReceiver : public BnLocAidlEsStatusReceiver {
    LocAidlEsStatusReceiver();
    virtual ~LocAidlEsStatusReceiver();
    // Methods from ::vendor::qti::gnss::ILocAidlEsStatusReceiver follow.
    ::ndk::ScopedAStatus setCallback(const std::shared_ptr<ILocAidlEsStatusCallback>& callback,
                                     bool* _aidl_return) override;

private:
    EsListener* mListener;
    IIzatManager* mIzatManager;
    std::shared_ptr<LocAidlDeathRecipient> mDeathRecipient = nullptr;
};


}  // namespace implementation
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
}  // namespace aidl
#endif  // VENDOR_QTI_GNSS_LOCAIDLESSTATUSRECEIVER_H

/*
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef VENDOR_QTI_GNSS_LOCAIDLQESDKSERVICE_H
#define VENDOR_QTI_GNSS_LOCAIDLQESDKSERVICE_H

#include "LocAidlUtils.h"
#include <memory>
#include <aidl/vendor/qti/gnss/BnLocAidlQesdkTracking.h>
#include <location_interface.h>
#include "LocQesdkPPEProxyBase.h"

using ILocAidlQesdkTrackingCallback =
        ::aidl::vendor::qti::gnss::ILocAidlQesdkTrackingCallback;
using LocAidlQesdkSessionParams = ::aidl::vendor::qti::gnss::LocAidlQesdkSessionParams;
using LocAidlQesdkSessionPrecision = ::aidl::vendor::qti::gnss::LocAidlQesdkSessionPrecision;

namespace aidl {
namespace vendor {
namespace qti {
namespace gnss {
namespace implementation {

struct LocAidlQesdkTracking : public BnLocAidlQesdkTracking {
    LocAidlQesdkTracking();
    virtual ~LocAidlQesdkTracking() = default;

    ::ndk::ScopedAStatus setCallback(
            const std::shared_ptr<ILocAidlQesdkTrackingCallback>& in_callback,
            int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus requestLocationUpdates(const LocAidlQesdkSessionParams& params,
            int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus removeLocationUpdates(const LocAidlQesdkSessionParams& params,
            int32_t* _aidl_return) override;

    void handleAidlClientSsr();

private:
    std::shared_ptr<location_qesdk::LocQesdkPPEProxyBase> mQesdkProxyPtr;
    std::shared_ptr<LocAidlDeathRecipient> mDeathRecipient;
    std::shared_ptr<ILocAidlQesdkTrackingCallback> mCallbackIface = nullptr;
    bool mWaitingForSsrReconnection = false;
};


}
}
}
}
}
#endif

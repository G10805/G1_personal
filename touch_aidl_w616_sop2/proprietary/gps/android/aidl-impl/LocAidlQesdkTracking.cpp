/*
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "LocSvc_AIDL_QesdkService"
#define LOG_NDEBUG 0

#include "LocAidlQesdkTracking.h"


#include <log_util.h>
#include "LocationDataTypes.h"
#include <dlfcn.h>
#include "loc_misc_utils.h"

namespace aidl {
namespace vendor {
namespace qti {
namespace gnss {
namespace implementation {

void static LocAidlQesdkTrackingCallbackDied(void* cookie) {
    LOC_LOGe("ILocAidlQesdkTrackingCallback died.");
    auto thiz = static_cast<LocAidlQesdkTracking*>(cookie);
    if (nullptr != thiz) {
        thiz->handleAidlClientSsr();
    }
}

void LocAidlQesdkTracking::handleAidlClientSsr() {
    ENTRY_LOG();

    // clear state
    if (mCallbackIface != nullptr && mDeathRecipient != nullptr) {
        mDeathRecipient->unregisterFromPeer(mCallbackIface->asBinder().get(), this);
    }
    mDeathRecipient = nullptr;
    mCallbackIface = nullptr;
    mWaitingForSsrReconnection = true;
}

LocAidlQesdkTracking::LocAidlQesdkTracking() {

    typedef std::shared_ptr<location_qesdk::LocQesdkPPEProxyBase>
        (getLocationQesdkProxy)();
    if (nullptr == mQesdkProxyPtr) {
        LOC_LOGd("loading liblocation_qesdk");
        void * handle = nullptr;
        getLocationQesdkProxy* getter = (getLocationQesdkProxy*)dlGetSymFromLib(handle,
                "liblocation_qesdk.so", "_Z16getQesdkPPEProxyv");
        if (NULL != getter) {
            mQesdkProxyPtr = (*getter)();
        } else {
            LOC_LOGe("loading liblocation_qesdki failed!");
        }
    }
}

::ndk::ScopedAStatus LocAidlQesdkTracking::setCallback(
        const std::shared_ptr<ILocAidlQesdkTrackingCallback>& in_callback,
        int32_t* _aidl_return) {
    ENTRY_LOG();
    if (mQesdkProxyPtr != nullptr) {
        mQesdkProxyPtr->setLocationReqCallback(
                [=](uint32_t pid, uint32_t uid, bool isStart, uint32_t minIntervalMillis,
                    bool isPrecise){
                    if (in_callback != nullptr) {
                        LocAidlQesdkSessionParams params;
                        if (isPrecise) {
                            params.precision = LocAidlQesdkSessionPrecision::FINE;
                        } else {
                            params.precision = LocAidlQesdkSessionPrecision::COARSE;
                        }
                        params.pid = pid;
                        params.uid = uid;
                        params.minIntervalMillis = minIntervalMillis;
                        if (isStart) {
                            in_callback->requestLocationUpdatesCb(params);
                        } else {
                            in_callback->removeLocationUpdatesCb(params);
                        }
                    }
                });

        mQesdkProxyPtr->setUserConsentCallback(
                [=](uint32_t pid, uint32_t uid, bool userConsent){
                    if (in_callback != nullptr) {
                        LocAidlQesdkSessionParams params;
                        params.precision = LocAidlQesdkSessionPrecision::COARSE;
                        params.pid = pid;
                        params.uid = uid;
                        params.minIntervalMillis = 0;
                        in_callback->setUserConsent(params, userConsent);
                    }
                });

        // Register death recipient
        if (mDeathRecipient == nullptr) {
            mDeathRecipient = make_shared<LocAidlDeathRecipient>(LocAidlQesdkTrackingCallbackDied);
        } else if (mCallbackIface != nullptr) {
            mDeathRecipient->unregisterFromPeer(mCallbackIface->asBinder().get(), this);
        }
        mCallbackIface = in_callback;
        if (in_callback != nullptr) {
            mDeathRecipient->registerToPeer(in_callback->asBinder().get(), this);
        } else {
            LOC_LOGe("callback is nullptr!");
        }
        // Report AIDL Client recovery after SSR to QESDK daemon
        // We do not wish to notify QESDK daemon on SSR occurence, but only when
        // the AIDL client process comes back up ( and invokes this setCallback )
        if (mWaitingForSsrReconnection) {
            mQesdkProxyPtr->handleAidlClientSsr();
            mWaitingForSsrReconnection = false;
        }

        *_aidl_return = 1;
    } else {
        *_aidl_return = 0;
    }
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus LocAidlQesdkTracking::requestLocationUpdates(
        const LocAidlQesdkSessionParams& params,
        int32_t* _aidl_return) {
    ENTRY_LOG();
    if (mQesdkProxyPtr != nullptr) {
        int ret = -1;
        if (params.precision == LocAidlQesdkSessionPrecision::FINE) {
            ret = mQesdkProxyPtr->requestPreciseLocationUpdates(params.pid, params.uid,
                    params.minIntervalMillis);
        } else if (params.precision == LocAidlQesdkSessionPrecision::COARSE) {
            if ((int)params.minIntervalMillis >= 0) {
                ret = mQesdkProxyPtr->requestGtpLocationUpdates(params.pid, params.uid,
                        params.minIntervalMillis);
            } else if ((int)params.minIntervalMillis == -1) {
                ret = mQesdkProxyPtr->requestGtpPassiveLocationUpdates(params.pid, params.uid);
            } else if ((int)params.minIntervalMillis == -2) {
                ret = mQesdkProxyPtr->notifyUserConsent(params.pid, params.uid, true);
            } else if ((int)params.minIntervalMillis == -3) {
                ret = mQesdkProxyPtr->notifyUserConsent(params.pid, params.uid, false);
            } else {
                LOC_LOGe("Invalid minInterval value %d", (int)params.minIntervalMillis);
            }
        }
        *_aidl_return = ret;
    } else {
        *_aidl_return = 0;
    }

    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus LocAidlQesdkTracking::removeLocationUpdates(
        const LocAidlQesdkSessionParams& params,
        int32_t* _aidl_return) {
    ENTRY_LOG();
    if (mQesdkProxyPtr != nullptr) {
        int ret = -1;
        if (params.precision == LocAidlQesdkSessionPrecision::FINE) {
            ret = mQesdkProxyPtr->removePreciseLocationUpdates(params.pid, params.uid);
        } else if (params.precision == LocAidlQesdkSessionPrecision::COARSE) {
            if ((int)params.minIntervalMillis >= 0) {
                ret = mQesdkProxyPtr->removeGtpLocationUpdates(params.pid, params.uid);
            } else {
                ret = mQesdkProxyPtr->removeGtpPassiveLocationUpdates(params.pid, params.uid);
            }
        }
        *_aidl_return = ret;
    } else {
        *_aidl_return = 0;
    }

    return ndk::ScopedAStatus::ok();
}

}
}
}
}
}

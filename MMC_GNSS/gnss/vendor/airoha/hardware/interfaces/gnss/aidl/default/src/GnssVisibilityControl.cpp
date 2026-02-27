/*
 * Copyright (C) 2021 The Android Open Source Project
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
#define LOG_TAG "GnssVisibilityControl"
#include "GnssVisibilityControl.h"
#include <log/log.h>
#include "GnssAidlImpl.h"
#include "air_parcel.h"
using airoha::Parcel;
namespace aidl::android::hardware::gnss::visibility_control {
std::shared_ptr<IGnssVisibilityControlCallback>
    GnssVisibilityControl::sCallback = nullptr;
std::vector<std::string> GnssVisibilityControl::sProxyApps;
ndk::ScopedAStatus GnssVisibilityControl::enableNfwLocationAccess(
        const std::vector<std::string>& proxyApps) {
    std::lock_guard<std::recursive_mutex> locker(
        GnssAidlImpl::getInstance()->getLock());
    Parcel parcel;
    std::string os;
    bool first = true;
    for (const auto& proxyApp : proxyApps) {
        if (first) {
            first = false;
        } else {
            os += " ";
        }
        os += proxyApp;
        parcel.writeString(proxyApp.c_str());
    }
    ALOGD("enableNfwLocationAccess proxyApps[%zu]: %s", proxyApps.size(),
          os.c_str());
    sProxyApps = proxyApps;
    GnssAidlImpl::getInstance()->sendVisibilityControlProxyApps(parcel.data(),
                                                                parcel.size());
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus GnssVisibilityControl::setCallback(
        const std::shared_ptr<IGnssVisibilityControlCallback>& callback) {
    ALOGD("GnssVisibilityControl::setCallback");
    GnssAidlImpl::getInstance()->lock();
    sCallback = callback;
    GnssAidlImpl::getInstance()->unlock();
    return ndk::ScopedAStatus::ok();
}
void GnssVisibilityControl::reportNfwNotification(
    const ::aidl::android::hardware::gnss::visibility_control::
        IGnssVisibilityControlCallback::NfwNotification& notification) {
    GnssAidlImpl::getInstance()->lock();
    auto visibilityCallback(sCallback);
    GnssAidlImpl::getInstance()->unlock();
    ::ndk::ScopedAStatus status;
    ALOGD(
        "report "
        "nfw:package=%s,stack=%d,otherstack=%s,requestor=%d,requestid=%s,is_"
        "emergency=%d,is_cache=%d, response=%d",
        notification.proxyAppPackageName.c_str(),
        (int)notification.protocolStack,
        notification.otherProtocolStackName.c_str(),
        (int)notification.requestor, notification.requestorId.c_str(),
        notification.inEmergencyMode, notification.isCachedLocation,
        (int)notification.responseType);
    if (visibilityCallback) {
        status = visibilityCallback->nfwNotifyCb(notification);
    }
    if (!status.isOk()) {
        ALOGE("report nfw notify error.");
    }
}
void GnssVisibilityControl::syncVisibilityControlProxy() {
    std::lock_guard<std::recursive_mutex> locker(
        GnssAidlImpl::getInstance()->getLock());
    Parcel parcel;
    std::string os;
    bool first = true;
    for (const auto& proxyApp : sProxyApps) {
        if (first) {
            first = false;
        } else {
            os += " ";
        }
        os += proxyApp;
        parcel.writeString(proxyApp.c_str());
    }
    ALOGD("resync enableNfwLocationAccess proxyApps: %s", os.c_str());
    GnssAidlImpl::getInstance()->sendVisibilityControlProxyApps(parcel.data(),
                                                                parcel.size());
}
}  // namespace aidl::android::hardware::gnss::visibility_control

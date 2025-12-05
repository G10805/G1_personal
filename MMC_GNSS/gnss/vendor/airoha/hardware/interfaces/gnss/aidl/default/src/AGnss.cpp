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
#define LOG_TAG "AGnssAidl"
#include "AGnss.h"
#include <inttypes.h>
#include <log/log.h>
#include "GnssAidlImpl.h"
namespace aidl::android::hardware::gnss {

std::shared_ptr<IAGnssCallback> AGnss::sCallback = nullptr;

ndk::ScopedAStatus AGnss::setCallback(const std::shared_ptr<IAGnssCallback>& callback) {
    ALOGD("AGnss::setCallback");
    GnssAidlImpl::getInstance()->lock();
    sCallback = callback;
    GnssAidlImpl::getInstance()->unlock();
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus AGnss::dataConnClosed() {
    anld_status_t anld_ret;
    anld_ret = GnssAidlImpl::getInstance()->sendDataConnClose();
    if (anld_ret != ANLD_STATUS_OK) {
        ALOGE("dataConnClosed ERROR");
        return ndk::ScopedAStatus::fromStatus(STATUS_UNKNOWN_ERROR);
    }
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus AGnss::dataConnFailed() {
    anld_status_t anld_ret;
    anld_ret = GnssAidlImpl::getInstance()->sendDataConnFailed();
    if (anld_ret != ANLD_STATUS_OK) {
        ALOGE("dataConnFailed ERROR");
        return ndk::ScopedAStatus::fromStatus(STATUS_UNKNOWN_ERROR);
    }
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus AGnss::setServer(AGnssType type, const std::string& hostname, int port) {
    ALOGD("AGnss::setServer: type: %s, hostname: %s, port: %d", toString(type).c_str(),
          hostname.c_str(), port);
    anld_status_t anld_ret;
    anld_ret = GnssAidlImpl::getInstance()->sendSetServer(
        static_cast<int>(type), port, hostname.c_str());
    if (anld_ret != ANLD_STATUS_OK) {
        ALOGE("dataConnFailed ERROR");
        return ndk::ScopedAStatus::fromStatus(STATUS_UNKNOWN_ERROR);
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus AGnss::dataConnOpen(int64_t networkHandle, const std::string& apn,
                                       ApnIpType apnIpType) {
    ALOGD("%s", __FUNCTION__);
    ALOGD("nethdr:%" PRIu64 ", apn:%s, apniptype:%d", networkHandle,
          apn.c_str(), (int)apnIpType);
    anld_status_t ret;
    ret = GnssAidlImpl::getInstance()->sendDataConnOpenIpType(
        apn.c_str(), (int)apnIpType, true, networkHandle);
    if (ret != ANLD_STATUS_OK) {
        ALOGE("dataConnOpen ERROR");
        return ndk::ScopedAStatus::fromStatus(STATUS_UNKNOWN_ERROR);
    }
    return ndk::ScopedAStatus::ok();
}
void AGnss::requestDataConnection(PreAGnssType type) {
    GnssAidlImpl::getInstance()->lock();
    auto callback_2_0(sCallback);
    GnssAidlImpl::getInstance()->unlock();
    IAGnssCallback::AGnssType aospType = IAGnssCallback::AGnssType::SUPL;
    switch (type) {
        case PreAGnssType::SUPL: {
            aospType = AGnssType::SUPL;
            break;
        }
        case PreAGnssType::C2K: {
            aospType = AGnssType::C2K;
            break;
        }
        case PreAGnssType::SUPL_EIMS: {
            aospType = AGnssType::SUPL_EIMS;
            break;
        }
        case PreAGnssType::SUPL_IMS: {
            aospType = AGnssType::SUPL_IMS;
            break;
        }
    }
    if (callback_2_0 != nullptr) {
        callback_2_0->agnssStatusCb(aospType,
                                    IAGnssCallback::AGnssStatusValue::REQUEST_AGNSS_DATA_CONN);
    }
}
void AGnss::releaseDataConnection(PreAGnssType type) {
    GnssAidlImpl::getInstance()->lock();
    auto callback_2_0(sCallback);
    GnssAidlImpl::getInstance()->unlock();
    IAGnssCallback::AGnssType aospType = IAGnssCallback::AGnssType::SUPL;
    switch (type) {
        case PreAGnssType::SUPL: {
            aospType = AGnssType::SUPL;
            break;
        }
        case PreAGnssType::C2K: {
            aospType = AGnssType::C2K;
            break;
        }
        case PreAGnssType::SUPL_EIMS: {
            aospType = AGnssType::SUPL_EIMS;
            break;
        }
        case PreAGnssType::SUPL_IMS: {
            aospType = AGnssType::SUPL_IMS;
            break;
        }
    }
    if (callback_2_0 != nullptr) {
        callback_2_0->agnssStatusCb(aospType,
                                    IAGnssCallback::AGnssStatusValue::RELEASE_AGNSS_DATA_CONN);
    }
}
}  // namespace aidl::android::hardware::gnss

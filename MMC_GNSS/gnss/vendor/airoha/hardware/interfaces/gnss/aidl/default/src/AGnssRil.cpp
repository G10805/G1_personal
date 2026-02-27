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
#define LOG_TAG "AGnssRilAidl"
#include "AGnssRil.h"
#include <inttypes.h>
#include <log/log.h>
#include "Gnss.h"
#include "GnssAidlImpl.h"
namespace aidl::android::hardware::gnss {

std::shared_ptr<IAGnssRilCallback> AGnssRil::sCallback = nullptr;

ndk::ScopedAStatus AGnssRil::setCallback(const std::shared_ptr<IAGnssRilCallback>& callback) {
    GnssAidlImpl::getInstance()->lock();
    sCallback = callback;
    GnssAidlImpl::getInstance()->unlock();
    return ndk::ScopedAStatus::ok();
}
void AGnssRil::requestSetId(int flag) {
    GnssAidlImpl::getInstance()->lock();
    auto sCallback_1_0(sCallback);
    GnssAidlImpl::getInstance()->unlock();
    if (sCallback_1_0 == nullptr) return;
    sCallback_1_0->requestSetIdCb(flag);
}
void AGnssRil::requestRefLoc() {
    GnssAidlImpl::getInstance()->lock();
    auto sCallback_1_0(sCallback);
    GnssAidlImpl::getInstance()->unlock();
    if (sCallback_1_0 == nullptr) return;
    sCallback->requestRefLocCb();
}
ndk::ScopedAStatus AGnssRil::setRefLocation(const AGnssRefLocation& agnssReflocation) {
    const AGnssRefLocationCellID& cellInfo = agnssReflocation.cellID;
    ALOGD("AGnssRil::setRefLocation: type: %s, mcc: %d, mnc: %d, lac: %d, cid: %" PRId64
          ", tac: %d, pcid: "
          "%d, arfcn: %d",
          toString(agnssReflocation.type).c_str(), cellInfo.mcc, cellInfo.mnc, cellInfo.lac,
          cellInfo.cid, cellInfo.tac, cellInfo.pcid, cellInfo.arfcn);
    anld_status_t anld_ret;
    ALOGD("ril set ref loc: %u, mcc=%u, mnc=%u, lac=%u, cid=%" PRId64,
          (unsigned int)agnssReflocation.type, agnssReflocation.cellID.mcc,
          agnssReflocation.cellID.mnc, agnssReflocation.cellID.lac,
          agnssReflocation.cellID.cid);
    auto& cellID = agnssReflocation.cellID;
    anld_ret = GnssAidlImpl::getInstance()->sendRefLocation(
        static_cast<int>(agnssReflocation.type), cellID.mcc, cellID.mnc,
        cellID.lac, cellID.cid);
    if (anld_ret != ANLD_STATUS_OK) {
        ALOGE("setRefLocation ERROR");
        return ndk::ScopedAStatus::fromStatus(STATUS_UNKNOWN_ERROR);
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus AGnssRil::setSetId(SetIdType type, const std::string& setid) {
    ALOGD("AGnssRil::setSetId: type:%s, setid: %s", toString(type).c_str(), setid.c_str());
    anld_status_t anld_ret;
    std::string secretString = setid;
    secretString = secretString.substr(0, 6);
    ALOGD("set set id: %u, %s, len=%zu", (unsigned int)type,
          secretString.c_str(), setid.size());
    anld_ret = GnssAidlImpl::getInstance()->sendSetId(static_cast<int>(type),
                                                      setid.c_str());
    if (anld_ret != ANLD_STATUS_OK) {
        ALOGE("setSetId ERROR");
        return ndk::ScopedAStatus::fromStatus(STATUS_UNKNOWN_ERROR);
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus AGnssRil::updateNetworkState(const NetworkAttributes& attributes) {
    ALOGD("AGnssRil::updateNetworkState: networkHandle:%" PRId64
          ", isConnected: %d, capabilities: %d, "
          "apn: %s",
          attributes.networkHandle, attributes.isConnected, attributes.capabilities,
          attributes.apn.c_str());
    anld_status_t anld_ret;
    anld_ret = GnssAidlImpl::getInstance()->sendUpdateNetworkStateExt(
        attributes.networkHandle, attributes.isConnected,
        attributes.capabilities, attributes.apn.c_str());
    if (attributes.isConnected) {
        Gnss::requestLocation();
    }
    if (anld_ret != ANLD_STATUS_OK) {
        ALOGE("updateNetworkState ERROR");
        return ndk::ScopedAStatus::fromStatus(STATUS_UNKNOWN_ERROR);
    }
    return ndk::ScopedAStatus::ok();
}
static bool checkSuplMessage(const std::vector<uint8_t>& in_msgData) {
    for (uint8_t c : in_msgData) {
        if (c > 0) return true;
    }
    return false;
}
#ifdef AIR_ANDROID_14_IMPL
::ndk::ScopedAStatus AGnssRil::injectNiSuplMessageData(
    const std::vector<uint8_t>& in_msgData, int32_t in_slotIndex) {
    ALOGD("injectNiSuplMessageData %zu, slot=%" PRId32, in_msgData.size(),
          in_slotIndex);
    if (!checkSuplMessage(in_msgData)) {
        return ndk::ScopedAStatus::fromStatus(STATUS_UNKNOWN_ERROR);
    }
    GnssAidlImpl::getInstance()->sendNiMessage(in_msgData.data(),
                                               in_msgData.size());
    return ndk::ScopedAStatus::ok();
}
#endif
}  // namespace aidl::android::hardware::gnss

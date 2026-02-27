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

#define LOG_TAG "GnssAntennaInfoAidl"

#include "GnssAntennaInfo.h"
#include <aidl/android/hardware/gnss/BnGnss.h>
#include <log/log.h>


namespace aidl::android::hardware::gnss {


using Row = IGnssAntennaInfoCallback::Row;
using Coord = IGnssAntennaInfoCallback::Coord;

std::shared_ptr<IGnssAntennaInfoCallback> GnssAntennaInfo::sCallback = nullptr;

GnssAntennaInfo::GnssAntennaInfo() : mMinIntervalMs(1000) {}

GnssAntennaInfo::~GnssAntennaInfo() {
    stop();
}

// Methods from ::android::hardware::gnss::V2_1::IGnssAntennaInfo follow.
ndk::ScopedAStatus GnssAntennaInfo::setCallback(
        const std::shared_ptr<IGnssAntennaInfoCallback>& callback) {
    sCallback = callback;

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus GnssAntennaInfo::close() {

    return ndk::ScopedAStatus::ok();
}

void GnssAntennaInfo::start() {
    ALOGD("start");

}

void GnssAntennaInfo::stop() {

}



}  // namespace aidl::android::hardware::gnss

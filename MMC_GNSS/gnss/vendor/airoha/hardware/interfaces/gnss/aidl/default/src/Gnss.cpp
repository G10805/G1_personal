/*
 * Copyright (C) 2020 The Android Open Source Project
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
#define LOG_TAG "GnssAidl"
#include "Gnss.h"
#include <inttypes.h>
#include <log/log.h>
#include <utils/Timers.h>
#include "AGnss.h"
#include "AGnssRil.h"
#include "GnssAidlImpl.h"
#include "GnssAntennaInfo.h"
#include "GnssBatching.h"
#include "GnssConfiguration.h"
#include "GnssDebug.h"
#include "GnssGeofence.h"
#include "GnssNavigationMessageInterface.h"
#include "GnssPsds.h"
#include "GnssVisibilityControl.h"
#include "MeasurementCorrectionsInterface.h"

#include "GnssSimulate.h"

#include <cutils/properties.h>

#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <algorithm>
#include <cstring>
#include <fstream>

#include <android/log.h>
#include <cstdint>
 
#define YEAR_OF_HARDWARE_1_0 2015
#define YEAR_OF_HARDWARE_2_0 2020
#define SUPPORT_GNSS_SIGNAL_GPS_L1CA
#define SUPPORT_GNSS_SIGNAL_GPS_L5Q
#define SUPPORT_GNSS_SIGNAL_GLONASS_L1
#define SUPPORT_GNSS_SIGNAL_GALILEO_E1_BC
#define SUPPORT_GNSS_SIGNAL_GALILEO_E5A
#define SUPPORT_GNSS_SIGNAL_BEIDOU_B1I
#define SUPPORT_GNSS_SIGNAL_BEIDOU_B2A
#define SUPPORT_GNSS_SIGNAL_QZSS_L1CA
#define SUPPORT_GNSS_SIGNAL_QZSS_L5Q
#define SUPPORT_GNSS_SIGNAL_NAVIC_L5
#define SUPPORT_GNSS_SIGNAL_SBAS_L1
namespace aidl::android::hardware::gnss {

using ndk::ScopedAStatus;
using GnssSvInfo = IGnssCallback::GnssSvInfo;
std::shared_ptr<IGnssCallback> Gnss::sGnssCallback = nullptr;
bool Gnss::sEnableNmea = false;
bool Gnss::sEnableSvStatus = false;

//NMEA monitor globals and helper for DTC logging
std::atomic<int64_t> gLastNmeaTimeMs{0};    // epoch ms of last NMEA received
std::atomic<bool>    gDtcLogged{false};     // whether DTC was logged
std::once_flag       gNmeaMonitorInitFlag;
std::mutex           gDtcFileMutex;

constexpr uint32_t DTCCode = 0x931E;
constexpr uint32_t FaultType = 0x49;

constexpr uint32_t combined_DTC = (DTCCode << 8) | FaultType; // 0x931E49

constexpr uint32_t logDTC = 0;
constexpr uint32_t clearDTC = 1;

const int64_t        kNmeaTimeoutMs = 30 * 1000; // 30 seconds

// Helper: monotonic/steady now in milliseconds
static inline int64_t steadyNowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

//Logging DTC
static void logDtc(int64_t steadyTsMs) {
    bool expected = false;
    if (!gDtcLogged.compare_exchange_strong(expected, true)) {
        // already logged;
        return;
    }
    std::lock_guard<std::mutex> lk(gDtcFileMutex);
    ALOGE("NMEA DTC: no NMEA frames for >= %d seconds. steady_timestamp_ms=%" PRId64,
          (int)(kNmeaTimeoutMs / 1000), steadyTsMs);
    DTCLOGE("0x%X:%u", combined_DTC, logDTC);
}

//Removing DTC
static void clearDtc() {
    bool expected = true;
    if (!gDtcLogged.compare_exchange_strong(expected, false)) {
        //already Looged
        return;
    }
    std::lock_guard<std::mutex> lk(gDtcFileMutex);
    ALOGE("NMEA DTC cleared (previously logged).");
    DTCLOGE("0x%X:%u", combined_DTC, clearDTC);
}

static void startNmeaMonitorThread() {
    std::thread([](){
        ALOGD("Starting NMEA monitor thread (monotonic clock)");
        // Ensure we don't immediately report DTC if no NMEA was seen yet:
        // if gLastNmeaTimeMs == 0, initialize it to now so timer starts from monitor start.
        int64_t expected = 0;
        int64_t nowInit = steadyNowMs();
        if (gLastNmeaTimeMs.compare_exchange_strong(expected, nowInit)) {
            ALOGE("Initialized last-NMEA steady timestamp to monitor start time: %" PRId64, nowInit);
        }

        while (true) {
            int64_t lastMs = gLastNmeaTimeMs.load();
            int64_t nowMs = steadyNowMs();
            if (lastMs == 0) {
                // If still zero, set to now to avoid immediate DTC
                gLastNmeaTimeMs.store(nowMs);
            } else {
                if ((nowMs - lastMs) >= kNmeaTimeoutMs) {
                    if (!gDtcLogged.load()) {
                        // log DTC (ALOGE instead of file write)
                        ALOGE("NMEA: No NMEA frames received for >= %d seconds. Logging DTC.",
                              (int)(kNmeaTimeoutMs / 1000));
                        logDtc(nowMs);
                        gDtcLogged.store(true);
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }).detach();
}

Gnss::Gnss() {}
ScopedAStatus Gnss::setCallback(const std::shared_ptr<IGnssCallback>& callback) {
    ALOGD("GNSS AIDL setCallback");
    if (callback == nullptr) {
        ALOGE("%s: Null callback ignored", __func__);
        return ScopedAStatus::fromExceptionCode(STATUS_INVALID_OPERATION);
    }
    GnssAidlImpl::getInstance()->lock();
    sGnssCallback = callback;
    GnssAidlImpl::getInstance()->unlock();
    reportYearCapName();
    GnssAidlImpl::getInstance()->sendGnssLocationInit();
    GnssPsds::clearCallback();
    std::call_once(gNmeaMonitorInitFlag, startNmeaMonitorThread);
    return ScopedAStatus::ok();
}
ScopedAStatus Gnss::start() {
    ALOGD("gnss start");
    anld_status_t anld_ret;
    anld_ret = GnssAidlImpl::getInstance()->sendGnssLocationStartOnly();
    if (anld_ret != ANLD_STATUS_OK) {
        ALOGE("power control ERROR");
        // Don't need to return error because IPC layer will backup state.
    }
    ALOGD("start OK, wating for response");
    return ScopedAStatus::ok();
}
ScopedAStatus Gnss::stop() {
    ALOGD("gnss stop");
    anld_status_t anld_ret;
    anld_ret = GnssAidlImpl::getInstance()->sendGnssLocationStopOnly();
    if (anld_ret != ANLD_STATUS_OK) {
        ALOGE("power control ERROR");
        // return false;
    }
    ALOGD("stop OK, wating for response");
    return ScopedAStatus::ok();
}
ScopedAStatus Gnss::close() {
    ALOGD("GNSS AIDL close");
    GnssAidlImpl::getInstance()->lock();
    sGnssCallback = nullptr;
    GnssAidlImpl::getInstance()->unlock();
    GnssAidlImpl::getInstance()->sendGnssLocationCleanup();
    return ScopedAStatus::ok();
}
ndk::ScopedAStatus Gnss::startSvStatus() {
    ALOGD("Gnss::startSvStatus");
    {
        std::lock_guard<std::recursive_mutex> lock(
            GnssAidlImpl::getInstance()->getLock());
        sEnableSvStatus = true;
    }
    GnssAidlImpl::getInstance()->sendStartSvStatus();
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Gnss::stopSvStatus() {
    ALOGD("Gnss::stopSvStatus");
    {
        std::lock_guard<std::recursive_mutex> lock(
            GnssAidlImpl::getInstance()->getLock());
        sEnableSvStatus = false;
    }
    GnssAidlImpl::getInstance()->sendStopSvStatus();
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Gnss::startNmea() {
    {
        std::lock_guard<std::recursive_mutex> lock(
            GnssAidlImpl::getInstance()->getLock());
        sEnableNmea = true;
    }
    GnssAidlImpl::getInstance()->sendStartNmea();
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Gnss::stopNmea() {
    {
        std::lock_guard<std::recursive_mutex> lock(
            GnssAidlImpl::getInstance()->getLock());
        sEnableNmea = false;
    }
    GnssAidlImpl::getInstance()->sendStopNmea();
    return ndk::ScopedAStatus::ok();
}
ScopedAStatus Gnss::getExtensionAGnss(std::shared_ptr<IAGnss>* iAGnss) {
    ALOGD("Gnss::getExtensionAGnss");
    *iAGnss = SharedRefBase::make<AGnss>();
    return ndk::ScopedAStatus::ok();
}

ScopedAStatus Gnss::injectTime(int64_t timeMs, int64_t timeReferenceMs, int uncertaintyMs) {
    ALOGD("injectTime. timeMs:%" PRId64 ", timeReferenceMs:%" PRId64 ", uncertaintyMs:%d", timeMs,
          timeReferenceMs, uncertaintyMs);
    NavTime_t timeInfo;
    timeInfo.timeMs = timeMs;
    timeInfo.timeReferenceMs = timeReferenceMs;
    timeInfo.uncertaintyMs = uncertaintyMs;
    GnssAidlImpl::getInstance()->sendInjectTime(&timeInfo);
    return ScopedAStatus::ok();
}

ScopedAStatus Gnss::getExtensionAGnssRil(std::shared_ptr<IAGnssRil>* iAGnssRil) {
    ALOGD("Gnss::getExtensionAGnssRil");
    *iAGnssRil = SharedRefBase::make<AGnssRil>();
    return ndk::ScopedAStatus::ok();
}

ScopedAStatus Gnss::injectLocation(const GnssLocation& location) {
    ALOGD("injectLocation. lat:%lf, lng:%lf, acc:%f", location.latitudeDegrees,
          location.longitudeDegrees, location.horizontalAccuracyMeters);
    PreGnssLocation platformLocation = {
        .flags = (uint16_t)location.gnssLocationFlags,
        .latitudeDegrees = location.latitudeDegrees,
        .longitudeDegrees = location.longitudeDegrees,
        .altitudeMeters = location.altitudeMeters,
        .speedMetersPerSec = (float)location.speedMetersPerSec,
        .bearingDegrees = (float)location.bearingDegrees,
        .horizontalAccuracyMeters = (float)location.horizontalAccuracyMeters,
        .verticalAccuracyMeters = (float)location.verticalAccuracyMeters,
        .speedAccuracyMetersPerSecond =
            (float)location.speedAccuracyMetersPerSecond,
        .bearingAccuracyDegrees = (float)location.bearingAccuracyDegrees,
        .timestamp = location.timestampMillis,
        .bestLocation = 0};
    GnssAidlImpl::getInstance()->sendInjectLocation(&platformLocation);
    return ScopedAStatus::ok();
}

ScopedAStatus Gnss::injectBestLocation(const GnssLocation& location) {
    ALOGD("injectBestLocation. lat:%lf, lng:%lf, acc:%f", location.latitudeDegrees,
          location.longitudeDegrees, location.horizontalAccuracyMeters);
    PreGnssLocation platformLocation = {
        .flags = (uint16_t)location.gnssLocationFlags,
        .latitudeDegrees = location.latitudeDegrees,
        .longitudeDegrees = location.longitudeDegrees,
        .altitudeMeters = location.altitudeMeters,
        .speedMetersPerSec = (float)location.speedMetersPerSec,
        .bearingDegrees = (float)location.bearingDegrees,
        .horizontalAccuracyMeters = (float)location.horizontalAccuracyMeters,
        .verticalAccuracyMeters = (float)location.verticalAccuracyMeters,
        .speedAccuracyMetersPerSecond =
            (float)location.speedAccuracyMetersPerSecond,
        .bearingAccuracyDegrees = (float)location.bearingAccuracyDegrees,
        .timestamp = location.timestampMillis,
        .bestLocation = 1};
    GnssAidlImpl::getInstance()->sendInjectLocation(&platformLocation);
    return ScopedAStatus::ok();
}

ScopedAStatus Gnss::deleteAidingData(GnssAidingData aidingDataFlags) {
    ALOGD("deleteAidingData. flags:%d", (int)aidingDataFlags);
    PreGnssAidingData aiding = 0;
    aiding = static_cast<PreGnssAidingData>(aidingDataFlags);
    GnssAidlImpl::getInstance()->sendDeleteAidingData(&aiding);
    return ScopedAStatus::ok();
}

ScopedAStatus Gnss::setPositionMode(const PositionModeOptions& options) {
    ALOGD("setPositionMode. minIntervalMs:%d, lowPowerMode:%d", options.minIntervalMs,
          (int)options.lowPowerMode);
    PreGnssPosisionPreference prefer;
    memset(&prefer, 0, sizeof(prefer));
    prefer.mode = static_cast<PreGnssPositionMode>(options.mode);
    prefer.recurrence =
        static_cast<PreGnssPositionRecurrence>(options.recurrence);
    prefer.minIntervalMs = options.minIntervalMs;
    prefer.preferredTimeMs = options.preferredTimeMs;
    prefer.preferredAccuracyMeters = options.preferredAccuracyMeters;
    prefer.lowPowerMode = options.lowPowerMode;
    ALOGI(
        "set pos mode 1.1: "
        "m=%u,rec=%u,minInterval=%u,preAcc=%u,preTime=%u,lowP=%d",
        (unsigned int)options.mode, (unsigned int)options.recurrence,
        (unsigned int)options.minIntervalMs,
        (unsigned int)options.preferredAccuracyMeters,
        (unsigned int)options.preferredTimeMs,
        (unsigned int)options.lowPowerMode);
    GnssAidlImpl::getInstance()->sendPositionConfiguration(&prefer);
    return ScopedAStatus::ok();
}

ScopedAStatus Gnss::getExtensionPsds(std::shared_ptr<IGnssPsds>* iGnssPsds) {
    ALOGD("getExtensionPsds");
    if (mGnssPsds == nullptr) {
        mGnssPsds = SharedRefBase::make<GnssPsds>();
    }
    *iGnssPsds = mGnssPsds;
    return ScopedAStatus::ok();
}

ScopedAStatus Gnss::getExtensionGnssConfiguration(
        std::shared_ptr<IGnssConfiguration>* iGnssConfiguration) {
    ALOGD("getExtensionGnssConfiguration");
    if (mGnssConfiguration == nullptr) {
        mGnssConfiguration = SharedRefBase::make<GnssConfiguration>();
    }
    *iGnssConfiguration = mGnssConfiguration;
    return ScopedAStatus::ok();
}

ScopedAStatus Gnss::getExtensionGnssPowerIndication(
        std::shared_ptr<IGnssPowerIndication>* iGnssPowerIndication) {
    ALOGD("getExtensionGnssPowerIndication");
    if (mGnssPowerIndication == nullptr) {
        mGnssPowerIndication = SharedRefBase::make<GnssPowerIndication>();
    }

    *iGnssPowerIndication = mGnssPowerIndication;
    return ScopedAStatus::ok();
}

ScopedAStatus Gnss::getExtensionGnssMeasurement(
        std::shared_ptr<IGnssMeasurementInterface>* iGnssMeasurement) {
    ALOGD("getExtensionGnssMeasurement");
    if (mGnssMeasurementInterface == nullptr) {
        mGnssMeasurementInterface = SharedRefBase::make<GnssMeasurementInterface>();
    }
    *iGnssMeasurement = mGnssMeasurementInterface;
    return ScopedAStatus::ok();
}

ScopedAStatus Gnss::getExtensionGnssBatching(std::shared_ptr<IGnssBatching>* iGnssBatching) {
    ALOGD("getExtensionGnssBatching");
    *iGnssBatching = nullptr;
    return ScopedAStatus::fromServiceSpecificError(ERROR_GENERIC);
}
ScopedAStatus Gnss::getExtensionGnssGeofence(
    std::shared_ptr<IGnssGeofence>* iGnssGeofence) {
    ALOGD("getExtensionGnssGeofence");
    *iGnssGeofence = nullptr;
    return ScopedAStatus::fromServiceSpecificError(ERROR_GENERIC);
}
ScopedAStatus Gnss::getExtensionGnssNavigationMessage(
        std::shared_ptr<IGnssNavigationMessageInterface>* iGnssNavigationMessage) {
    ALOGD("getExtensionGnssNavigationMessage");
    if (mGnssNavigationMessageInterface == nullptr) {
        mGnssNavigationMessageInterface =
            SharedRefBase::make<GnssNavigationMessageInterface>();
    }
    *iGnssNavigationMessage = mGnssNavigationMessageInterface;
    return ScopedAStatus::ok();
}

ndk::ScopedAStatus Gnss::getExtensionGnssDebug(std::shared_ptr<IGnssDebug>* iGnssDebug) {
    ALOGD("Gnss::getExtensionGnssDebug");

    *iGnssDebug = SharedRefBase::make<GnssDebug>();
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Gnss::getExtensionGnssVisibilityControl(
        std::shared_ptr<visibility_control::IGnssVisibilityControl>* iGnssVisibilityControl) {
    ALOGD("Gnss::getExtensionGnssVisibilityControl");

    *iGnssVisibilityControl = SharedRefBase::make<visibility_control::GnssVisibilityControl>();
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Gnss::getExtensionGnssAntennaInfo(
        std::shared_ptr<IGnssAntennaInfo>* iGnssAntennaInfo) {
    ALOGD("Gnss::getExtensionGnssAntennaInfo");

    *iGnssAntennaInfo = SharedRefBase::make<GnssAntennaInfo>();
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Gnss::getExtensionMeasurementCorrections(
        std::shared_ptr<measurement_corrections::IMeasurementCorrectionsInterface>*
                iMeasurementCorrections) {
    ALOGD("Gnss::getExtensionMeasurementCorrections");
    *iMeasurementCorrections = nullptr;
    return ScopedAStatus::fromServiceSpecificError(ERROR_GENERIC);
}
// Vendor Implementation
void Gnss::reportYearCapName() {
    reportYear();
    reportCap();
    // reportName();
}
void Gnss::reportYear() {
    ALOGD("Report Year %d %d", YEAR_OF_HARDWARE_2_0, YEAR_OF_HARDWARE_1_0);
    GnssAidlImpl::getInstance()->lock();
    auto gnssCb(sGnssCallback);
    GnssAidlImpl::getInstance()->unlock();
    ::aidl::android::hardware::gnss::IGnssCallback::GnssSystemInfo info;
    info.yearOfHw = YEAR_OF_HARDWARE_2_0;
    info.name = "Airoha. Version AIDL";
    ndk::ScopedAStatus status;
    if (gnssCb != nullptr) {
        status = gnssCb->gnssSetSystemInfoCb(info);
    }
    if (!status.isOk()) {
        ALOGE("reportYear failed");
    }
    return;
}
void Gnss::reportCap() {
    GnssAidlImpl::getInstance()->lock();
    auto gnssCb(sGnssCallback);
    GnssAidlImpl::getInstance()->unlock();
    int32_t capabilities =
        IGnssCallback::CAPABILITY_SCHEDULING |
        IGnssCallback::CAPABILITY_MEASUREMENTS | IGnssCallback::CAPABILITY_MSA |
        IGnssCallback::CAPABILITY_MSB | IGnssCallback::CAPABILITY_NAV_MESSAGES;
    // Need MSA/MSB?
    ndk::ScopedAStatus status;
    if (gnssCb != nullptr) {
        status = gnssCb->gnssSetCapabilitiesCb(capabilities);
    }
    if (!status.isOk()) {
        ALOGE("reportCap failed");
    }
#ifdef AIR_ANDROID_14_IMPL
    // Report GNss SignalType Capabilities.
    std::vector<GnssSignalType> signalTypes;
#ifdef SUPPORT_GNSS_SIGNAL_GPS_L1CA
    {
        // GPS L1 C/A
        GnssSignalType signal;
        signal.constellation = GnssConstellationType::GPS;
        signal.codeType = GnssSignalType::CODE_TYPE_C;
        signal.carrierFrequencyHz = 1575420000.0;
        signalTypes.push_back(signal);
    }
#endif
#ifdef SUPPORT_GNSS_SIGNAL_GPS_L5Q
    {
        // GPS L1 C/A
        GnssSignalType signal;
        signal.constellation = GnssConstellationType::GPS;
        signal.codeType = GnssSignalType::CODE_TYPE_Q;
        signal.carrierFrequencyHz = 1176450000.0;
        signalTypes.push_back(signal);
    }
#endif
#ifdef SUPPORT_GNSS_SIGNAL_GLONASS_L1
    {
        // GPS L1 C/A
        GnssSignalType signal;
        signal.constellation = GnssConstellationType::GLONASS;
        signal.codeType = GnssSignalType::CODE_TYPE_C;
        signal.carrierFrequencyHz = 1602000000.0;
        signalTypes.push_back(signal);
    }
#endif
#ifdef SUPPORT_GNSS_SIGNAL_GALILEO_E1_BC
    {
        // GPS L1 C/A
        GnssSignalType signal;
        signal.constellation = GnssConstellationType::GALILEO;
        signal.codeType = GnssSignalType::CODE_TYPE_C;
        signal.carrierFrequencyHz = 1575420000.0;
        signalTypes.push_back(signal);
    }
#endif
#ifdef SUPPORT_GNSS_SIGNAL_GALILEO_E5A
    {
        // GPS L1 C/A
        GnssSignalType signal;
        signal.constellation = GnssConstellationType::GALILEO;
        signal.codeType = GnssSignalType::CODE_TYPE_B;
        signal.carrierFrequencyHz = 1176450000.0;
        signalTypes.push_back(signal);
    }
#endif
#ifdef SUPPORT_GNSS_SIGNAL_BEIDOU_B1I
    {
        // GPS L1 C/A
        GnssSignalType signal;
        signal.constellation = GnssConstellationType::BEIDOU;
        signal.codeType = GnssSignalType::CODE_TYPE_I;
        signal.carrierFrequencyHz = 1561098000.0;
        signalTypes.push_back(signal);
    }
#endif
#ifdef SUPPORT_GNSS_SIGNAL_BEIDOU_B2A
    {
        // GPS L1 C/A
        GnssSignalType signal;
        signal.constellation = GnssConstellationType::BEIDOU;
        signal.codeType = GnssSignalType::CODE_TYPE_Q;
        signal.carrierFrequencyHz = 1176450000.0;
        signalTypes.push_back(signal);
    }
#endif
#ifdef SUPPORT_GNSS_SIGNAL_NAVIC_L5
    {
        // GPS L1 C/A
        GnssSignalType signal;
        signal.constellation = GnssConstellationType::IRNSS;
        signal.codeType = GnssSignalType::CODE_TYPE_C;
        signal.carrierFrequencyHz = 1176450000.0;
        signalTypes.push_back(signal);
    }
#endif
#ifdef SUPPORT_GNSS_SIGNAL_QZSS_L1CA
    {
        // GPS L1 C/A
        GnssSignalType signal;
        signal.constellation = GnssConstellationType::QZSS;
        signal.codeType = GnssSignalType::CODE_TYPE_C;
        signal.carrierFrequencyHz = 1575420000.0;
        signalTypes.push_back(signal);
    }
#endif
#ifdef SUPPORT_GNSS_SIGNAL_QZSS_L5Q
    {
        // GPS L1 C/A
        GnssSignalType signal;
        signal.constellation = GnssConstellationType::QZSS;
        signal.codeType = GnssSignalType::CODE_TYPE_Q;
        signal.carrierFrequencyHz = 1176450000.0;
        signalTypes.push_back(signal);
    }
#endif
#ifdef SUPPORT_GNSS_SIGNAL_SBAS_L1
    {
        // GPS L1 C/A
        GnssSignalType signal;
        signal.constellation = GnssConstellationType::SBAS;
        signal.codeType = GnssSignalType::CODE_TYPE_C;
        signal.carrierFrequencyHz = 1575420000.0;
        signalTypes.push_back(signal);
    }
#endif
    if (gnssCb != nullptr) {
        gnssCb->gnssSetSignalTypeCapabilitiesCb(signalTypes);
    }
#endif
}
void Gnss::reportName(const char* name) {
    GnssAidlImpl::getInstance()->lock();
    auto gnssCb(sGnssCallback);
    GnssAidlImpl::getInstance()->unlock();
    std::string vendorName = "Airoha. Version AIDL";
    if (name) {
        vendorName += ",";
        vendorName += name;
    }
    ::aidl::android::hardware::gnss::IGnssCallback::GnssSystemInfo info;
    info.yearOfHw = YEAR_OF_HARDWARE_2_0;
    info.name = vendorName;
    ndk::ScopedAStatus status;
    if (gnssCb != nullptr) {
        status = gnssCb->gnssSetSystemInfoCb(info);
    }
    if (!status.isOk()) {
        ALOGE("reportYear failed");
    }
}

//Visteon Modified
void Gnss::reportLocation(const PreGnssLocation* location) {
    GnssAidlImpl::getInstance()->lock();
    auto gnssCb(sGnssCallback);
    GnssAidlImpl::getInstance()->unlock();
    GnssLocation gnssLocation;
    gnssLocation.gnssLocationFlags =
        static_cast<::android::hardware::hidl_bitfield<
            ::android::hardware::gnss::V1_0::GnssLocationFlags>>(
            location->flags);
    gnssLocation.latitudeDegrees = location->latitudeDegrees;
    gnssLocation.longitudeDegrees = location->longitudeDegrees;
    gnssLocation.altitudeMeters = location->altitudeMeters;
    gnssLocation.speedMetersPerSec = location->speedMetersPerSec;
    gnssLocation.bearingDegrees = location->bearingDegrees;
    gnssLocation.horizontalAccuracyMeters = location->horizontalAccuracyMeters;
    gnssLocation.verticalAccuracyMeters = location->verticalAccuracyMeters;
    gnssLocation.speedAccuracyMetersPerSecond =
        location->speedAccuracyMetersPerSecond;
    gnssLocation.bearingAccuracyDegrees = location->bearingAccuracyDegrees;
    gnssLocation.timestampMillis = location->timestamp;
    gnssLocation.elapsedRealtime.flags = location->elapsedTime.flags;
    gnssLocation.elapsedRealtime.timestampNs =
        location->elapsedTime.timestampNs;
    ndk::ScopedAStatus status;
    if (gnssCb != nullptr) {
        //Visteon Added: 25/07
        ALOGD("HAL: reportLocation: lat:%lf, lng:%lf, acc:%f, alt:%lf, "
              "speed:%f, bearing:%f, hAcc:%f, vAcc:%f, "
              "sAcc:%f, bAcc:%f, ts:%" PRId64 ", "
              "elapsedTimeNs:%" PRIu64 ", elapsedTimeUncertaintyNs:%" PRIu64,
              gnssLocation.latitudeDegrees, gnssLocation.longitudeDegrees,
              gnssLocation.horizontalAccuracyMeters, gnssLocation.altitudeMeters,
              gnssLocation.speedMetersPerSec, gnssLocation.bearingDegrees,
              gnssLocation.horizontalAccuracyMeters,
              gnssLocation.verticalAccuracyMeters,
              gnssLocation.speedAccuracyMetersPerSecond,
              gnssLocation.bearingAccuracyDegrees, gnssLocation.timestampMillis,
              gnssLocation.elapsedRealtime.timestampNs,
              gnssLocation.elapsedRealtime.timeUncertaintyNs);
        status = gnssCb->gnssLocationCb(gnssLocation);
    }
    if (!status.isOk()) {
        ALOGE("reportLocation failed");
    }
}

//Visteon Modified
void Gnss::reportSvStatus(const PreGnssSvStatus* svStatusBlock) {
    GnssAidlImpl::getInstance()->lock();
    auto gnssCb(sGnssCallback);
    bool toReportSvStatus(sEnableSvStatus);
    GnssAidlImpl::getInstance()->unlock();
    if (!toReportSvStatus) {
        return;
    }
    std::vector<::aidl::android::hardware::gnss::IGnssCallback::GnssSvInfo>
        in_svInfoList;
    uint32_t i = 0;
    // gnssSvStatus.numSvs = svStatusBlock->numSvs;
    ALOGD("Satellite Status Output %" PRIu32, svStatusBlock->numSvs);
    // 1.x limit 64 else no limit ;
    // if ((gnssCb_1_0 != nullptr || gnssCb_1_1 != nullptr) &&
    //     svStatusBlock->numSvs > PRE_GNSS_MAX_SVS) {
    //     svInfo_1_0.numSvs = PRE_GNSS_MAX_SVS;
    // } else {
    //     svInfo_1_0.numSvs = svStatusBlock->numSvs;
    // }
    char sPropValue[PROP_VALUE_MAX] = {0};
    if (__system_property_get("persist.vendor.gnss.simulation", sPropValue) > 0 &&
            strcmp(sPropValue, "1") == 0) {
        ALOGD("svstatus simulation prop value: %s ", sPropValue);
        for (i = 0; i < 8; i++) {
            ::aidl::android::hardware::gnss::IGnssCallback::GnssSvInfo svInfo;
            svInfo.svid = 10 + i;
            svInfo.constellation = static_cast<GnssConstellationType>(1);
            if (i % 2 == 0)
                svInfo.cN0Dbhz = 35 + i;
            else
                svInfo.cN0Dbhz = 40 + i;
            svInfo.elevationDegrees = 45 + i;
            svInfo.azimuthDegrees = 90 + i;
            svInfo.carrierFrequencyHz = 1;
            svInfo.svFlag = 0x07;
            svInfo.basebandCN0DbHz = 30 + i;
            in_svInfoList.push_back(svInfo);
        }
    } else {
        for (i = 0; i < svStatusBlock->numSvs; i++) {
            ::aidl::android::hardware::gnss::IGnssCallback::GnssSvInfo svInfo;
            svInfo.svid = svStatusBlock->gnssSvList[i].svid;
            // temp.v2_0.v1_0.svid = svStatusBlock->gnssSvList[i].svid;
            svInfo.constellation = static_cast<GnssConstellationType>(
                svStatusBlock->gnssSvList[i].constellation);
            // temp.v2_0.v1_0.constellation =
            //     static_cast<::android::hardware::gnss::V1_0::GnssConstellationType>(
            //         svStatusBlock->gnssSvList[i].constellation);
            svInfo.cN0Dbhz = svStatusBlock->gnssSvList[i].cN0Dbhz;
            // temp.v2_0.v1_0.cN0Dbhz = svStatusBlock->gnssSvList[i].cN0Dbhz;
            svInfo.elevationDegrees = svStatusBlock->gnssSvList[i].elevationDegrees;
            // temp.v2_0.v1_0.elevationDegrees =
            //     svStatusBlock->gnssSvList[i].elevationDegrees;
            svInfo.azimuthDegrees = svStatusBlock->gnssSvList[i].azimuthDegrees;
            // temp.v2_0.v1_0.azimuthDegrees =
            //     svStatusBlock->gnssSvList[i].azimuthDegrees;
            svInfo.carrierFrequencyHz =
                svStatusBlock->gnssSvList[i].carrierFrequencyHz;
            // temp.v2_0.v1_0.carrierFrequencyHz =
            //     svStatusBlock->gnssSvList[i].carrierFrequencyHz;
            svInfo.svFlag = svStatusBlock->gnssSvList[i].svFlag;
            // temp.v2_0.v1_0.svFlag =
            // static_cast<::android::hardware::hidl_bitfield<
            //     ::android::hardware::gnss::V1_0::IGnssCallback::GnssSvFlags>>(
            //     svStatusBlock->gnssSvList[i].svFlag);
            // temp.v2_0.constellation =
            //     static_cast<::android::hardware::gnss::V2_0::GnssConstellationType>(
            //         svStatusBlock->gnssSvList[i].constellation);
            svInfo.basebandCN0DbHz = svStatusBlock->gnssSvList[i].basebandCN0DbHz;
            // temp.basebandCN0DbHz = svStatusBlock->gnssSvList[i].basebandCN0DbHz;
            in_svInfoList.push_back(svInfo);
            // svInfoList_V2_0.push_back(temp.v2_0);
            // if (i < PRE_GNSS_MAX_SVS) {
            //     svInfo_1_0.gnssSvList[i] = temp.v2_0.v1_0;
            // }
        }
    }
    ndk::ScopedAStatus status;
    if (gnssCb != nullptr) {
        //Visteon added: 25/07
        ALOGD("HAL: reportSvStatus: numSvs:%" PRIu32
              ", svInfoList size:%zu",
              svStatusBlock->numSvs, in_svInfoList.size());
        for (const auto& svInfo : in_svInfoList) {
            ALOGD("HAL: reportSvStatus: svid:%" PRId32
                  ", constellation:%s, cN0Dbhz:%f, elevationDegrees:%f, "
                  "azimuthDegrees:%f, carrierFrequencyHz:%f, svFlag:%d, "
                  "basebandCN0DbHz:%f",
                  svInfo.svid, toString(svInfo.constellation).c_str(),
                  svInfo.cN0Dbhz, svInfo.elevationDegrees,
                  svInfo.azimuthDegrees, svInfo.carrierFrequencyHz,
                  svInfo.svFlag, svInfo.basebandCN0DbHz);
        }
        status = gnssCb->gnssSvStatusCb(in_svInfoList);
    }
    if (!status.isOk()) {
        ALOGE("reportSvStatus failed");
    }
}

//Visteon Added
void checkAndManageGnssFolder() {
    const char* folder = "/data/gnss";
    const size_t MAX_SIZE = 10 * 1024 * 1024; // 10MB

    DIR* dir = opendir(folder);
    if (!dir) return;

    std::vector<std::pair<std::string, time_t>> files;
    size_t totalSize = 0;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type != DT_REG) continue;
        std::string fname = entry->d_name;
        if (fname.find("location_nmea_") != 0) continue;

        std::string fullpath = std::string(folder) + "/" + fname;
        struct stat st;
        if (stat(fullpath.c_str(), &st) == 0) {
            totalSize += st.st_size;
            files.emplace_back(fullpath, st.st_mtime);
        }
    }
    closedir(dir);

    if (totalSize > MAX_SIZE && !files.empty()) {
        // Sort by mtime (oldest first)
        std::sort(files.begin(), files.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });

        if (files.size() == 1) {
            // Only one file, delete it
            remove(files[0].first.c_str());
        } else {
            // Delete oldest file
            remove(files[0].first.c_str());
        }
    }
}

//Visteon Modified
void Gnss::reportNMEA(const NavNmea* nmeaBlock) {
    ALOGD("Reporting NMEA from HAL");
    static bool nSimulation = false;
    GnssAidlImpl::getInstance()->lock();
    auto gnssCb(sGnssCallback);
    bool toReportNmea(sEnableNmea);
    GnssAidlImpl::getInstance()->unlock();
    if (!toReportNmea) {
        return;
    }
    ndk::ScopedAStatus status;
    // Simulation: Only run if simulation property is set
    char propValue[PROP_VALUE_MAX] = {0};
    if (__system_property_get("persist.vendor.gnss.simulation", propValue) > 0) {
        std::string value(propValue);
        ALOGD("[%s]: prop value: %s - prop %s", __func__, value.c_str(), propValue);
        if (value == "1") {
            ALOGD("Simulation is Active");
            if (nmeaBlock != nullptr && nmeaBlock->nmea[0] == '$' &&
                nmeaBlock->nmea[1] == 'G' && nmeaBlock->nmea[2] == 'N' &&
                nmeaBlock->nmea[3] == 'R' && nmeaBlock->nmea[4] == 'M' &&
                nmeaBlock->nmea[5] == 'C') {
                // Check if the fix status is 'V' (invalid)
                if (nmeaBlock->nmea[17] == 'V') {
                    ALOGD("Invalid RMC - report from simulation.kml");
                    nSimulation = true;
                    GnssSimulate::simulateFromKml(
                        "/vendor/etc/simulation.kml",
                        reportNMEA,
                        reportSvStatus,
                        reportLocation);
                } else {
                        // If the fix status is valid, we do not simulate
                        ALOGD("NMEA fix status is valid, not simulating.");
                        nSimulation = false;
                }
            }
        }
    }

    // Update last-received timestamps (use monotonic clock) and clear DTC if needed.
    if (nmeaBlock != nullptr) {
        // Use monotonic steady clock for timeout tracking (avoid wall-clock jumps)
        int64_t tsSteadyMs = steadyNowMs();
        gLastNmeaTimeMs.store(tsSteadyMs);

        // If a DTC was previously logged, clear it now that we received NMEA.
        if (gDtcLogged.load()) {
            ALOGD("NMEA received, clearing previously logged DTC.");
            clearDtc();
            gDtcLogged.store(false);
        }

        // Ensure NMEA monitor thread is running
        std::call_once(gNmeaMonitorInitFlag, startNmeaMonitorThread);
    }

    //Visteon Added: 25/07
    ALOGD("GNSS HAL: reportNMEA: timestamp:%" PRId64 ", nmea:%s",
              nmeaBlock->timestamp, nmeaBlock->nmea);

    // Log non-simulation NMEA frames to file
    if (!nSimulation && nmeaBlock != nullptr && nmeaBlock->nmea[0] == '$') {
        // Ensure /data/gnss/ exists
        struct stat st;
        memset(&st, 0, sizeof(st));
        if (stat("/data/gnss", &st) == -1) {
            mkdir("/data/gnss", 0770);
        }

        // Periodically check and manage folder size
        checkAndManageGnssFolder();

        // Get current date as YYYYMMDD_<time>
        time_t now = time(nullptr);
        struct tm tm_now;
        localtime_r(&now, &tm_now);
        char dateStr[64];
        strftime(dateStr, sizeof(dateStr), "%Y%m%d", &tm_now);

        // Build filename
        char filename[256];
        snprintf(filename, sizeof(filename), "/data/gnss/location_nmea_%s.txt", dateStr);

        // Append NMEA frame to file
        std::ofstream ofs(filename, std::ios::app);
        if (ofs.is_open()) {
            ofs << nmeaBlock->nmea << std::endl;
            ofs.close();
        } else {
            ALOGE("Failed to open NMEA log file: %s", filename);
        }
    }

    // If simulation is active, we do not report only GGA and RMC NMEA
    if (nSimulation) {
        //only skip GGA RMC
        if (nmeaBlock != nullptr && 
            (strncmp(nmeaBlock->nmea, "$GNGGA", 6) == 0 ||
             strncmp(nmeaBlock->nmea, "$GNRMC", 6) == 0)) {
            ALOGD("Skipping GGA or RMC NMEA during simulation.");
            return;
        }
    }

    if (gnssCb != nullptr) {
        status = gnssCb->gnssNmeaCb(nmeaBlock->timestamp, nmeaBlock->nmea);
    } else {
        ALOGE("[%s]:gnssCb is Null", __func__);
    }

    if (!status.isOk()) {
        ALOGE("reportNMEA failed");
    }
}

void Gnss::reportStatus(
    ::aidl::android::hardware::gnss::IGnssCallback::GnssStatusValue value) {
    GnssAidlImpl::getInstance()->lock();
    auto gnssCb(sGnssCallback);
    GnssAidlImpl::getInstance()->unlock();
    std::string log = "Report Status: ";
    log += toString(value);
    ALOGD("%s", log.c_str());
    ndk::ScopedAStatus status;
    if (gnssCb != nullptr) {
        //Visteon Added: 25/07
        ALOGE("HAL: reportStatus: value:%s", toString(value).c_str());
        status = gnssCb->gnssStatusCb(value);
    }
    if (!status.isOk()) {
        ALOGE("report Status failed");
    }
}
void Gnss::requestTime() {
    GnssAidlImpl::getInstance()->lock();
    auto gnssCb(sGnssCallback);
    GnssAidlImpl::getInstance()->unlock();
    ALOGD("requestTime: AIDL");
    ndk::ScopedAStatus status;
    if (gnssCb != nullptr) {
        status = gnssCb->gnssRequestTimeCb();
    }
    if (!status.isOk()) {
        ALOGE("requestTime failed");
    }
}
void Gnss::requestLocation() {
    GnssAidlImpl::getInstance()->lock();
    auto gnssCb(sGnssCallback);
    GnssAidlImpl::getInstance()->unlock();
    ALOGD("requestLocation: AIDL");
    ndk::ScopedAStatus status;
    if (gnssCb != nullptr) {
        status = gnssCb->gnssRequestLocationCb(true, false);
    }
    if (!status.isOk()) {
        ALOGE("requestTime failed");
    }
}
void Gnss::requestConfigurationSync() {
    // We sync nmea and sv status in IHidlTransportInterface
    // if (sEnableNmea) {
    //     sendNavigationCommand2ANLD(ANLD_NAV_GNSS_START_NMEA);
    // } else {
    //     sendNavigationCommand2ANLD(ANLD_NAV_GNSS_STOP_NMEA);
    // }
    // if (sEnableSvStatus) {
    //     sendNavigationCommand2ANLD(ANLD_NAV_GNSS_START_SV_STATUS);
    // } else {
    //     sendNavigationCommand2ANLD(ANLD_NAV_GNSS_STOP_SV_STATUS);
    // }
}
}  // namespace aidl::android::hardware::gnss

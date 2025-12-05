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
#define LOG_TAG "GnssMeasIfaceAidl"
#include "GnssMeasurementInterface.h"
#include <aidl/android/hardware/gnss/BnGnss.h>
#include <log/log.h>
#include "GnssAidlImpl.h"
#include "airo_gps.h"
#include "utils/SystemClock.h"
using Airoha::Gnss::PreGnssClock;
using Airoha::Gnss::PreGnssClockFlags;
using Airoha::Gnss::PreGnssData;
using Airoha::Gnss::PreGnssMeasurement;
using Airoha::Gnss::PreGnssMeasurementStateBit;
namespace aidl::android::hardware::gnss {
std::shared_ptr<IGnssMeasurementCallback> GnssMeasurementInterface::sCallback = nullptr;
GnssMeasurementInterface::GnssMeasurementInterface() {}
GnssMeasurementInterface::~GnssMeasurementInterface() {
    
}

ndk::ScopedAStatus GnssMeasurementInterface::setCallback(
        const std::shared_ptr<IGnssMeasurementCallback>& callback, const bool enableFullTracking,
        const bool enableCorrVecOutputs) {
    ALOGD("setCallback: enableFullTracking: %d enableCorrVecOutputs: %d", (int)enableFullTracking,
          (int)enableCorrVecOutputs);
    GnssAidlImpl::getInstance()->lock();
    sCallback = callback;
    GnssAidlImpl::getInstance()->unlock();
    PreGnssMeasurementOptions hal_options;
    hal_options.enableCorrVecOutputs = enableCorrVecOutputs;
    hal_options.enableFullTracking = enableFullTracking;
    hal_options.intervalMs = 1000;
    start(hal_options);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus GnssMeasurementInterface::setCallbackWithOptions(
        const std::shared_ptr<IGnssMeasurementCallback>& callback, const Options& options) {
    ALOGD("setCallbackWithOptions: fullTracking:%d, corrVec:%d, intervalMs:%d",
          (int)options.enableFullTracking, (int)options.enableCorrVecOutputs, options.intervalMs);
    GnssAidlImpl::getInstance()->lock();
    sCallback = callback;
    GnssAidlImpl::getInstance()->unlock();
    PreGnssMeasurementOptions hal_options;
    hal_options.enableCorrVecOutputs = options.enableCorrVecOutputs;
    hal_options.enableFullTracking = options.enableFullTracking;
    hal_options.intervalMs = options.intervalMs;
    start(hal_options);
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus GnssMeasurementInterface::close() {
    ALOGD("close measurement");
    stop();
    GnssAidlImpl::getInstance()->lock();
    sCallback = nullptr;
    ALOGD("gnss measurement close");
    GnssAidlImpl::getInstance()->unlock();
    return ndk::ScopedAStatus::ok();
}
void GnssMeasurementInterface::start(const PreGnssMeasurementOptions& options) {
    ALOGD("start measurement");
    GnssAidlImpl::getInstance()->sendMeasurementUpdateRequest(options);
}
void GnssMeasurementInterface::stop() {
    ALOGD("stop measurement");
    GnssAidlImpl::getInstance()->sendClearMeasurementRequest();
}
void GnssMeasurementInterface::airohaGnssMeasurementCb(
    const PreGnssData* legacyGnssData) {
    GnssAidlImpl::getInstance()->lock();
    auto measCb(sCallback);
    GnssAidlImpl::getInstance()->unlock();
    if ((measCb == nullptr)) {
        ALOGE("%s: GNSSMeasurement Callback Interface configured incorrectly",
              __func__);
        return;
    }
    if (legacyGnssData == nullptr) {
        ALOGE("%s: Invalid GnssData from GNSS HAL", __func__);
        return;
    }
    ::aidl::android::hardware::gnss::GnssData in_gnssData;
    ::aidl::android::hardware::gnss::GnssData::GnssAgc gnssAgc;
    // IGnssMeasurementCallback::GnssData gnssData_2_0;
    // std::vector<V2_1::IGnssMeasurementCallback::GnssMeasurement> tempVec;
    // std::vector<V2_0::IGnssMeasurementCallback::GnssMeasurement>
    // tempVec_V2_0;
    for (size_t i = 0; i < legacyGnssData->measurement_count; i++) {
        const PreGnssMeasurement& entry = legacyGnssData->measurements[i];
        uint32_t tempState = entry.state;
        if (tempState & PreGnssMeasurementStateBit::STATE_TOW_DECODED) {
            tempState |= PreGnssMeasurementStateBit::STATE_TOW_KNOWN;
        }
        if (tempState & PreGnssMeasurementStateBit::STATE_GLO_TOD_DECODED) {
            tempState |= PreGnssMeasurementStateBit::STATE_GLO_TOD_KNOWN;
        }
        GnssMeasurement in_gnssMeasurement = {
            .flags = (int32_t)entry.flags,
            .svid = entry.svid,
            .signalType =
                {
                    .constellation =
                        static_cast<GnssConstellationType>(entry.constellation),
                    .carrierFrequencyHz = entry.carrier_frequency_hz,
                    .codeType = entry.codeType,
                },
            .timeOffsetNs = entry.time_offset_ns,
            .state = (int32_t)tempState,
            .receivedSvTimeInNs = entry.received_sv_time_in_ns,
            .receivedSvTimeUncertaintyInNs =
                entry.received_sv_time_uncertainty_in_ns,
            .antennaCN0DbHz = entry.c_n0_dbhz,
            .basebandCN0DbHz = entry.baseband_c_no_dbhz,
            .pseudorangeRateMps = entry.pseudorange_rate_mps,
            .pseudorangeRateUncertaintyMps =
                entry.pseudorange_rate_uncertainty_mps,
            .accumulatedDeltaRangeState = entry.accumulated_delta_range_state,
            .accumulatedDeltaRangeM = entry.accumulated_delta_range_m,
            .accumulatedDeltaRangeUncertaintyM =
                entry.accumulated_delta_range_uncertainty_m,
            .carrierCycles = entry.carrier_cycles,
            .carrierPhase = entry.carrier_phase,
            .carrierPhaseUncertainty = entry.carrier_phase_uncertainty,
            .multipathIndicator =
                static_cast<GnssMultipathIndicator>(entry.multipath_indicator),
            .snrDb = entry.snr_db,
            .agcLevelDb = entry.agc_level_db,
            .fullInterSignalBiasNs = entry.full_inter_signal_bias_ns,
            .fullInterSignalBiasUncertaintyNs =
                entry.full_inter_signal_bias_uncertainty_ns,
            .satelliteInterSignalBiasNs = entry.satellite_inter_signal_bias_ns,
            .satelliteInterSignalBiasUncertaintyNs =
                entry.satellite_inter_signal_bias_uncertainty_ns,
        };
        // V1_1::IGnssMeasurementCallback::GnssMeasurement gnssMeasurement_1_1 =
        // {
        //     .v1_0 =
        //         {
        //             // .flags =
        //             static_cast<::android::hardware::hidl_bitfield<
        //             //
        //             V1_0::IGnssMeasurementCallback::GnssMeasurementFlags>>(
        //             //     entry.flags),
        //             // .svid = entry.svid,
        //             // .constellation = static_cast<
        //             //
        //             ::android::hardware::gnss::V1_0::GnssConstellationType>(
        //             //     entry.constellation),
        //             // .timeOffsetNs = entry.time_offset_ns,
        //             // .state =
        //             static_cast<::android::hardware::hidl_bitfield<
        //             //     ::android::hardware::gnss::V1_0::
        //             // IGnssMeasurementCallback::GnssMeasurementState>>(
        //             //     tempState & GNSS_MEAS_STATE_1_1_MASK),
        //             // .receivedSvTimeInNs = entry.received_sv_time_in_ns,
        //             // .receivedSvTimeUncertaintyInNs =
        //             //     entry.received_sv_time_uncertainty_in_ns,
        //             // .cN0DbHz = entry.c_n0_dbhz,
        //             //.pseudorangeRateMps = entry.pseudorange_rate_mps,
        //             // .pseudorangeRateUncertaintyMps =
        //             //     entry.pseudorange_rate_uncertainty_mps,
        //             // .accumulatedDeltaRangeState =
        //             //     entry.accumulated_delta_range_state,
        //             // .accumulatedDeltaRangeM =
        //             entry.accumulated_delta_range_m,
        //             // .accumulatedDeltaRangeUncertaintyM =
        //             //     entry.accumulated_delta_range_uncertainty_m,
        //             //.carrierFrequencyHz = entry.carrier_frequency_hz,
        //             // .carrierCycles = entry.carrier_cycles,
        //             // .carrierPhase = entry.carrier_phase,
        //             // .carrierPhaseUncertainty =
        //             entry.carrier_phase_uncertainty,
        //             // .multipathIndicator = static_cast<
        //             //
        //             V1_0::IGnssMeasurementCallback::GnssMultipathIndicator>(
        //             //     entry.multipath_indicator),
        //             // .snrDb = entry.snr_db,
        //             // .agcLevelDb = entry.agc_level_db,
        //         },
        //     //.accumulatedDeltaRangeState =
        //     entry.accumulated_delta_range_state,
        // };
        // V2_0::IGnssMeasurementCallback::GnssMeasurement gnssMeasurement_2_0 =
        // {
        //     .v1_1 = gnssMeasurement_1_1,
        //     // .codeType = entry.codeType,
        //     // .state = tempState,
        //     // .constellation = static_cast<
        //     //     ::android::hardware::gnss::V2_0::GnssConstellationType>(
        //     //     entry.constellation),
        // };
        // V2_1::IGnssMeasurementCallback::GnssMeasurement gnssMeasurement_2_1 =
        // {
        //     // .v2_0 = gnssMeasurement_2_0,
        //     // .flags = static_cast<::android::hardware::hidl_bitfield<
        //     //     V1_0::IGnssMeasurementCallback::GnssMeasurementFlags>>(
        //     //     entry.flags),
        //     // .fullInterSignalBiasNs = entry.full_inter_signal_bias_ns,
        //     // .fullInterSignalBiasUncertaintyNs =
        //     //     entry.full_inter_signal_bias_uncertainty_ns,
        //     // .satelliteInterSignalBiasNs =
        //     entry.satellite_inter_signal_bias_ns,
        //     // .satelliteInterSignalBiasUncertaintyNs =
        //     //     entry.satellite_inter_signal_bias_uncertainty_ns,
        //     //.basebandCN0DbHz = entry.c_n0_dbhz
        // };
        // if (measCb_2_1 != nullptr) {
        //     tempVec.push_back(gnssMeasurement_2_1);
        // } else if (measCb_2_0 != nullptr) {
        //     tempVec_V2_0.push_back(gnssMeasurement_2_0);
        // }
        in_gnssData.measurements.push_back(in_gnssMeasurement);
    }
    // ALOGD("Measurement, size:%zu|%zu", tempVec.size(), tempVec_V2_0.size());
    const PreGnssClock& clockVal = legacyGnssData->clock;
    char codeType[2] = {0};
    codeType[0] = clockVal.referenceSignalTypeForIsb.codeType;
    GnssClock in_gnssClock = {
        .gnssClockFlags = clockVal.flags,
        .leapSecond = clockVal.leap_second,
        .timeNs = clockVal.time_ns,
        .timeUncertaintyNs = clockVal.time_uncertainty_ns,
        .fullBiasNs = clockVal.full_bias_ns,
        .biasNs = clockVal.bias_ns,
        .biasUncertaintyNs = clockVal.bias_uncertainty_ns,
        .driftNsps = clockVal.drift_nsps,
        .driftUncertaintyNsps = clockVal.drift_uncertainty_nsps,
        .hwClockDiscontinuityCount =
            (int32_t)clockVal.hw_clock_discontinuity_count,
        .referenceSignalTypeForIsb =
            {
                .constellation = static_cast<GnssConstellationType>(
                    clockVal.referenceSignalTypeForIsb.constellation),
                .carrierFrequencyHz =
                    clockVal.referenceSignalTypeForIsb.carrier_frequency_hz,
                .codeType = codeType,
            },
    };
    // V1_0::IGnssMeasurementCallback::GnssClock gnssclock_1_0 = {
    //     // .gnssClockFlags = static_cast<::android::hardware::hidl_bitfield<
    //     //     ::android::hardware::gnss::V1_0::IGnssMeasurementCallback::
    //     //         GnssClockFlags>>(clockVal.flags),
    //     // .leapSecond = clockVal.leap_second,
    //     // .timeNs = clockVal.time_ns,
    //     // .timeUncertaintyNs = clockVal.time_uncertainty_ns,
    //     // .fullBiasNs = clockVal.full_bias_ns,
    //     // .biasNs = clockVal.bias_ns,
    //     // .biasUncertaintyNs = clockVal.bias_uncertainty_ns,
    //     // .driftNsps = clockVal.drift_nsps,
    //     // .driftUncertaintyNsps = clockVal.drift_uncertainty_nsps,
    //     // .hwClockDiscontinuityCount = clockVal.hw_clock_discontinuity_count
    // };
    in_gnssData.clock = in_gnssClock;
    in_gnssData.elapsedRealtime.flags =
        (int32_t)legacyGnssData->elapsedRealtime.flags;
    in_gnssData.elapsedRealtime.timestampNs =
        legacyGnssData->elapsedRealtime.timestampNs;
    // if (measCb_2_1 != nullptr) {
    //     char codeType[2] = {0};
    //     codeType[0] = clockVal.referenceSignalTypeForIsb.codeType;
    //     V2_1::IGnssMeasurementCallback::GnssClock gnssclock_2_1 = {
    //         .v1_0 = gnssclock_1_0,
    //         // .referenceSignalTypeForIsb.constellation = static_cast<
    //         //     ::android::hardware::gnss::V2_0::GnssConstellationType>(
    //         //     clockVal.referenceSignalTypeForIsb.constellation),
    //         // .referenceSignalTypeForIsb.carrierFrequencyHz =
    //         //     clockVal.referenceSignalTypeForIsb.carrier_frequency_hz,
    //         // .referenceSignalTypeForIsb.codeType = codeType,
    //     };
    //     // gnssData_2_1.clock = gnssclock_2_1;
    //     // gnssData_2_1.measurements = tempVec;
    //     // gnssData_2_1.elapsedRealtime.flags =
    //     //     static_cast<::android::hardware::hidl_bitfield<
    //     //         ::android::hardware::gnss::V2_0::ElapsedRealtimeFlags>>(
    //     //         V2_0::ElapsedRealtimeFlags::HAS_TIMESTAMP_NS);
    //     // gnssData_2_1.elapsedRealtime.timestampNs = elapsedRealtimeNano();
    // } else if (measCb_2_0 != nullptr) {
    //     gnssData_2_0.clock = gnssclock_1_0;
    //     gnssData_2_0.elapsedRealtime.flags =
    //         static_cast<::android::hardware::hidl_bitfield<
    //             ::android::hardware::gnss::V2_0::ElapsedRealtimeFlags>>(
    //             V2_0::ElapsedRealtimeFlags::HAS_TIMESTAMP_NS);
    //     gnssData_2_0.elapsedRealtime.timestampNs = elapsedRealtimeNano();
    //     gnssData_2_0.measurements = tempVec_V2_0;
    // }
    // if (measCb_2_1 != nullptr) {
    //     auto ret = measCb_2_1->gnssMeasurementCb_2_1(gnssData_2_1);
    //     if (!ret.isOk()) {
    //         ALOGE("%s: Unable to invoke callback", __func__);
    //     }
    // } else if (measCb_2_0 != nullptr) {
    //     auto ret = measCb_2_0->gnssMeasurementCb_2_0(gnssData_2_0);
    //     if (!ret.isOk()) {
    //         ALOGE("%s: Unable to invoke callback", __func__);
    //     }
    // }
    // Fill GnssAgc
    for (size_t i = 0; i < legacyGnssData->gnssAgcNum; i++) {
        gnssAgc.agcLevelDb = legacyGnssData->gnssAgc[i].agcLevelDb;
        gnssAgc.carrierFrequencyHz =
            legacyGnssData->gnssAgc[i].carrierFrequencyHz;
        gnssAgc.constellation = static_cast<GnssConstellationType>(
            legacyGnssData->gnssAgc[i].constellation);
        in_gnssData.gnssAgcs.push_back(gnssAgc);
    }
#ifdef AIR_ANDROID_14_IMPL
    // default full tracking is true
    in_gnssData.isFullTracking = legacyGnssData->isFullTracking;
#endif
    ALOGD("Report measurement: %zu, fullTracking=%d",
          legacyGnssData->measurement_count, legacyGnssData->isFullTracking);
    ndk::ScopedAStatus status = measCb->gnssMeasurementCb(in_gnssData);
    if (!status.isOk()) {
        ALOGE("report measurement error.");
    }
}
}  // namespace aidl::android::hardware::gnss

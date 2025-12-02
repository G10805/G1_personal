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
#define LOG_TAG "GnssDebugAidl"
#include "GnssDebug.h"
#include <log/log.h>
#include "GnssAidlImpl.h"
#include "airo_gps.h"
namespace aidl::android::hardware::gnss {
IGnssDebug::DebugData GnssDebug::sDebugData;
ndk::ScopedAStatus GnssDebug::getDebugData(DebugData* debugData) {
    ALOGD("GnssDebug::getDebugData");
    *debugData = sDebugData;
    return ndk::ScopedAStatus::ok();
}
void GnssDebug::setDebugData(const PreDebugData* preData) {
    sDebugData.position = {
        .valid = preData->position.valid,
        .latitudeDegrees = preData->position.latitudeDegrees,
        .longitudeDegrees = preData->position.longitudeDegrees,
        .altitudeMeters = preData->position.altitudeMeters,
        .speedMetersPerSec = preData->position.speedMetersPerSec,
        .bearingDegrees = preData->position.bearingDegrees,
        .horizontalAccuracyMeters = preData->position.horizontalAccuracyMeters,
        .verticalAccuracyMeters = preData->position.verticalAccuracyMeters,
        .speedAccuracyMetersPerSecond =
            preData->position.speedAccuracyMetersPerSecond,
        .bearingAccuracyDegrees = preData->position.bearingAccuracyDegrees,
        .ageSeconds = preData->position.ageSeconds};
    std::vector<IGnssDebug::SatelliteData> satVec;
    for (size_t i = 0; i < SVS_NUM_TOTAL; i++) {
        SatelliteData in_SatelliteData = {
            .svid = preData->satelliteDataArray[i].svid,
            .constellation = static_cast<GnssConstellationType>(
                preData->satelliteDataArray[i].constellation),
            .ephemerisType = static_cast<IGnssDebug::SatelliteEphemerisType>(
                preData->satelliteDataArray[i].ephemerisType),
            .ephemerisSource =
                static_cast<SatellitePvt::SatelliteEphemerisSource>(
                    preData->satelliteDataArray[i].ephemerisSource),
            .ephemerisHealth =
                static_cast<IGnssDebug::SatelliteEphemerisHealth>(
                    preData->satelliteDataArray[i].ephemerisHealth),
            .ephemerisAgeSeconds =
                preData->satelliteDataArray[i].ephemerisAgeSeconds,
            .serverPredictionIsAvailable =
                preData->satelliteDataArray[i].serverPredictionIsAvailable,
            .serverPredictionAgeSeconds =
                preData->satelliteDataArray[i].serverPredictionAgeSeconds,
        };
        satVec.push_back(in_SatelliteData);
        //     SatelliteData singleData = {
        //         .v1_0 =
        //             {
        //                 //  .svid = preData->satelliteDataArray[i].svid,
        //                 // .ephemerisType =
        //                 //     static_cast<::android::hardware::gnss::V1_0::
        //                 // IGnssDebug::SatelliteEphemerisType>(
        //                 // preData->satelliteDataArray[i].ephemerisType),
        //                 // .ephemerisSource =
        //                 //     static_cast<::android::hardware::gnss::V1_0::
        //                 // IGnssDebug::SatelliteEphemerisSource>(
        //                 // preData->satelliteDataArray[i].ephemerisSource),
        //                 // .ephemerisHealth =
        //                 //     static_cast<::android::hardware::gnss::V1_0::
        //                 // IGnssDebug::SatelliteEphemerisHealth>(
        //                 // preData->satelliteDataArray[i].ephemerisHealth),
        //                 // .ephemerisAgeSeconds =
        //                 //
        //                 preData->satelliteDataArray[i].ephemerisAgeSeconds,
        //                 // .serverPredictionIsAvailable =
        //                 //     preData->satelliteDataArray[i]
        //                 //         .serverPredictionIsAvailable,
        //                 // .serverPredictionAgeSeconds =
        //                 //     preData->satelliteDataArray[i]
        //                 //         .serverPredictionAgeSeconds,
        //             },
        //         // .constellation = static_cast<
        //         // ::android::hardware::gnss::V2_0::GnssConstellationType>(
        //         //     preData->satelliteDataArray[i].constellation),
        //     };
        //     satVec.push_back(singleData);
        // }
    }
    sDebugData.satelliteDataArray = satVec;
    sDebugData.time.frequencyUncertaintyNsPerSec =
        preData->time.frequencyUncertaintyNsPerSec;
    sDebugData.time.timeEstimateMs = preData->time.timeEstimate;
    sDebugData.time.timeUncertaintyNs = preData->time.timeUncertaintyNs;
}
}  // namespace aidl::android::hardware::gnss

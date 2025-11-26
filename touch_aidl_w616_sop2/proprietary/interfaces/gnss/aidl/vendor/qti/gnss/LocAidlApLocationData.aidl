/*
* Copyright (c) 2021, 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlAltitudeRefType;
import vendor.qti.gnss.LocAidlWiFiRTTCapability;
import vendor.qti.gnss.LocAidlWiFiAPPositionQuality;


@VintfStability
parcelable LocAidlApLocationData {
    long mac_R48b;
    float latitude;
    float longitude;
    float max_antenna_range;
    float horizontal_error;
    byte reliability;
    byte valid_bits;

    float altitude;
    LocAidlAltitudeRefType altRefType;
    LocAidlWiFiRTTCapability rttCapability;
    LocAidlWiFiAPPositionQuality positionQuality;
    float rttRangeBiasInMm;
}


/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlGnssNiType;
import vendor.qti.gnss.LocAidlGnssUserResponseType;
import vendor.qti.gnss.LocAidlGnssNiEncodingType;


@VintfStability
parcelable LocAidlGnssNiNotification {
    int notificationId;
    LocAidlGnssNiType niType;
    int notifyFlags;
    int timeoutSec;
    LocAidlGnssUserResponseType defaultResponse;
    String requestorId;
    String notificationMessage;
    LocAidlGnssNiEncodingType requestorIdEncoding;
    LocAidlGnssNiEncodingType notificationIdEncoding;
    String extras;
    boolean esEnabled;
}


/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlApInfo;
import vendor.qti.gnss.LocAidlUlpLocation;
import vendor.qti.gnss.LocAidlWifiDBListStatus;

@VintfStability
interface ILocAidlWiFiDBReceiverCallback {

    void apListUpdateCallback(in LocAidlApInfo[] apInfoList,
        in int apListSize, in LocAidlWifiDBListStatus apListStatus,
        in LocAidlUlpLocation ulpLocation, in boolean ulpLocValid);

    void attachVmOnCallback();

    void serviceRequestCallback();

    void statusUpdateCallback(in boolean status, in String reason);
}

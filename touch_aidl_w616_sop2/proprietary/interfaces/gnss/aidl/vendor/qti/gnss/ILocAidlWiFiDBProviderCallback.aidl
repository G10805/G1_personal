/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlApCellInfo;
import vendor.qti.gnss.LocAidlApObsData;
import vendor.qti.gnss.LocAidlApScanData;
import vendor.qti.gnss.LocAidlWifiDBListStatus;

@VintfStability
interface ILocAidlWiFiDBProviderCallback {
    void apObsLocDataUpdateCallback(
        in LocAidlApObsData[] apObsLocDataList,
        in int apObsLocDataListSize, in LocAidlWifiDBListStatus apListStatus);

    void attachVmOnCallback();

    void serviceRequestCallback();
}

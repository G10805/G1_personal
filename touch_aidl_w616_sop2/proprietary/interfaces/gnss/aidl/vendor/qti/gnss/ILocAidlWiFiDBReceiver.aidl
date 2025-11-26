/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.ILocAidlWiFiDBReceiverCallback;
import vendor.qti.gnss.LocAidlApLocationData;
import vendor.qti.gnss.LocAidlApSpecialInfo;

@VintfStability
interface ILocAidlWiFiDBReceiver {

    boolean init(in vendor.qti.gnss.ILocAidlWiFiDBReceiverCallback callback);

    void pushAPWiFiDB(
        in vendor.qti.gnss.LocAidlApLocationData[] apLocationDataList,
        in int apLocationDataListSize,
        in vendor.qti.gnss.LocAidlApSpecialInfo[] apSpecialInfoList,
        in int apSpecialInfoListSize, in int daysValid, in boolean isLookup);

    void registerWiFiDBUpdater(in vendor.qti.gnss.ILocAidlWiFiDBReceiverCallback callback);

    void sendAPListRequest(in int expireInDays);

    void sendScanListRequest();

    void unregisterWiFiDBUpdater();
}

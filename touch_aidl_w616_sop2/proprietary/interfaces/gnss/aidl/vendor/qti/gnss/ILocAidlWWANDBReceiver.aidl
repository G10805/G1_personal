/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.ILocAidlWWANDBReceiverCallback;
import vendor.qti.gnss.LocAidlBsLocationData;
import vendor.qti.gnss.LocAidlBsSpecialInfo;
import vendor.qti.gnss.ILocAidlWWANDBReceiverCallback;

@VintfStability
interface ILocAidlWWANDBReceiver {

    boolean init(in ILocAidlWWANDBReceiverCallback callback);

    void pushBSWWANDB(
        in vendor.qti.gnss.LocAidlBsLocationData[] bsLocationDataList,
        in int bsLocationDataListSize,
        in vendor.qti.gnss.LocAidlBsSpecialInfo[] bsSpecialInfoList,
        in int bsSpecialInfoListSize, in int daysValid);

    void registerWWANDBUpdater(in ILocAidlWWANDBReceiverCallback callback);

    void sendBSListRequest(in int expireInDays);

    void unregisterWWANDBUpdater();
}

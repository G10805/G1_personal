/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlBsInfo;
import vendor.qti.gnss.LocAidlUlpLocation;

@VintfStability
interface ILocAidlWWANDBReceiverCallback {

    void attachVmOnCallback();

    void bsListUpdateCallback(in LocAidlBsInfo[] bsInfoList,
        in int bsListSize, in byte status, in LocAidlUlpLocation ulpLocation);

    void serviceRequestCallback();

    void statusUpdateCallback(in boolean status, in String reason);
}

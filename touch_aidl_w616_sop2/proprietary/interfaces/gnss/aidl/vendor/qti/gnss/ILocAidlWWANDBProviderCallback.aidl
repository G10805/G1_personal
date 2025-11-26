/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlBSObsData;
import vendor.qti.gnss.LocAidlBsCellInfo;

@VintfStability
interface ILocAidlWWANDBProviderCallback {
    void attachVmOnCallback();

    void bsObsLocDataUpdateCallback(
        in LocAidlBSObsData[] bsObsLocDataList,
        in int bsObsLocDataListSize, in byte bsListStatus);

    void serviceRequestCallback();
}

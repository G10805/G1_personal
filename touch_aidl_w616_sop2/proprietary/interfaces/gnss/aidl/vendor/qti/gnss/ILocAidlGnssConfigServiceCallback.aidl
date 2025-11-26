/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlGnssConstellationType;
import vendor.qti.gnss.LocAidlRobustLocationInfo;

@VintfStability
interface ILocAidlGnssConfigServiceCallback {

    void getGnssSvTypeConfigCb(
        in LocAidlGnssConstellationType[] disabledSvTypeList);

    void getRobustLocationConfigCb(in LocAidlRobustLocationInfo info);
}

/*
* Copyright (c) 2022 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlQesdkSessionParams;

@VintfStability
interface ILocAidlQesdkTrackingCallback {

    void requestLocationUpdatesCb(in LocAidlQesdkSessionParams params);
    void removeLocationUpdatesCb(in LocAidlQesdkSessionParams params);
    void setUserConsent(in LocAidlQesdkSessionParams params, in boolean userConsent);
}

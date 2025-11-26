/*
* Copyright (c) 2022 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.ILocAidlQesdkTrackingCallback;
import vendor.qti.gnss.LocAidlQesdkSessionParams;

@VintfStability
interface ILocAidlQesdkTracking {

    int setCallback(in ILocAidlQesdkTrackingCallback callback);
    int requestLocationUpdates(in LocAidlQesdkSessionParams params);
    int removeLocationUpdates(in LocAidlQesdkSessionParams params);
}

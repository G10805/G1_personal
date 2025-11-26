/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlLocation;

@VintfStability
interface ILocAidlGeofenceServiceCallback {
    void gnssAddGeofencesCb(in int count, in int[] locationErrorList, in int[] idList);

    void gnssGeofenceBreachCb(in int count, in int[] idList, in LocAidlLocation location,
        in int breachType, in long timestamp);

    void gnssGeofenceStatusCb(in int statusAvailable, in int locTechType);

    void gnssPauseGeofencesCb(in int count, in int[] locationErrorList, in int[] idList);

    void gnssRemoveGeofencesCb(in int count, in int[] locationErrorList, in int[] idList);

    void gnssResumeGeofencesCb(in int count, in int[] locationErrorList, in int[] idList);
}

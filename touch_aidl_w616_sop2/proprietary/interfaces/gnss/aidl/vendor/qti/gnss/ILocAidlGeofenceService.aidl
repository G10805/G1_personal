/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.ILocAidlGeofenceServiceCallback;

@VintfStability
interface ILocAidlGeofenceService {
    void addGeofence(in int id, in double latitude, in double longitude, in double radius,
        in int transitionTypes, in int responsiveness, in int confidence, in int dwellTime,
        in int dwellTimeMask);

    boolean init(in ILocAidlGeofenceServiceCallback callback);

    void pauseGeofence(in int id);

    void removeGeofence(in int id);

    void resumeGeofence(in int id, in int transitionTypes);

    void updateGeofence(in int id, in int transitionTypes, in int responsiveness);
}

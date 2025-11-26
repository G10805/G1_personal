/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.ILocAidlFlpServiceCallback;

@VintfStability
interface ILocAidlFlpService {

    void deleteAidingData(in long flags);

    int getAllBatchedLocations(in int sessionId);

    int getAllSupportedFeatures();

    void getMaxPowerAllocated();

    boolean init(in ILocAidlFlpServiceCallback callback);

    int startFlpSession(in int id, in int flags, in long minIntervalNanos,
        in int minDistanceMetres, in int tripDistanceMeters, in int power_mode, in int tbm_ms);

    int stopFlpSession(in int sessionId);

    int updateFlpSession(in int id, in int flags, in long minIntervalNanos,
        in int minDistanceMetres, in int tripDistanceMeters, in int power_mode, in int tbm_ms);

    void updateXtraThrottle(in boolean enabled);
}

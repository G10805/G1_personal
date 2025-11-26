/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlLocation;

@VintfStability
interface ILocAidlFlpServiceCallback {

    void gnssBatchingStatusCb(in vendor.qti.gnss.LocAidlBatchStatusInfo batchStatusInfo,
        in int[] listOfCompletedTrips);

    void gnssLocationBatchingCb(in vendor.qti.gnss.LocAidlBatchOptions batchOptions,
        in vendor.qti.gnss.LocAidlLocation[] locations);

    void gnssLocationTrackingCb(in LocAidlLocation location);

    void gnssMaxPowerAllocatedCb(in int powerInMW);
}

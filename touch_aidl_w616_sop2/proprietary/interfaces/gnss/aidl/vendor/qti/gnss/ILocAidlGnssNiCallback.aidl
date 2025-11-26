/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlGnssNiNotification;

@VintfStability
interface ILocAidlGnssNiCallback {
    void gnssCapabilitiesCb(in int capabilitiesBitMask);

    /**
     * Callback with a network initiated request.
     *
     * @param notification network initiated request.
     */

    void niNotifyCbExt(in LocAidlGnssNiNotification notification);
}

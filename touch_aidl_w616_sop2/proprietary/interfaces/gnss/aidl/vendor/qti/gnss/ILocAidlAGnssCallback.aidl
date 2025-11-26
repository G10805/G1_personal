/*
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlAGnssStatusIpV4;
import vendor.qti.gnss.LocAidlAGnssStatusIpV6;

@VintfStability
interface ILocAidlAGnssCallback {

    /**
     * Callback with AGNSS(IpV4) status information.
     *
     * @param status Will be of type AGnssStatusIpV4.
     */
    void locAidlAgnssStatusIpV4Cb(in LocAidlAGnssStatusIpV4 status);

    /**
     * Callback with AGNSS(IpV6) status information.
     *
     * @param status Will be of type AGnssStatusIpV6.
     */
    void locAidlAgnssStatusIpV6Cb(in LocAidlAGnssStatusIpV6 status);
}

/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlApnTypeMask;
import vendor.qti.gnss.LocAidlAGnssType;
import vendor.qti.gnss.LocAidlAGnssStatusValue;
import vendor.qti.gnss.LocAidlAGnssSubId;

/**
 * Represents the status of AGNSS augmented to support IPv6.
 */
@VintfStability
parcelable LocAidlAGnssStatusIpV6 {
    LocAidlAGnssType type;
    LocAidlApnTypeMask apnTypeMask;
    LocAidlAGnssStatusValue status;
    /**
     * 128-bit IPv6 address.
     */
    byte[/* 16 */] ipV6Addr;
    LocAidlAGnssSubId subId;
}


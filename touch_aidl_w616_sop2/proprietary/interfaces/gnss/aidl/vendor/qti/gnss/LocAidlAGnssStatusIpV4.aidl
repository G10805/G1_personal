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
 * Represents the status of AGNSS augmented to support IPv4.
 */
@VintfStability
parcelable LocAidlAGnssStatusIpV4 {
    LocAidlAGnssType type;
    LocAidlApnTypeMask apnTypeMask;
    LocAidlAGnssStatusValue status;
    /**
     * 32-bit IPv4 address.
     */
    int ipV4Addr;
    LocAidlAGnssSubId subId;
}


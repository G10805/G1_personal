/**
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.hardware.wifi.qtiwifi;

import vendor.qti.hardware.wifi.qtiwifi.IfaceType;

/**
 * Information of sta/ap interface
 */
@VintfStability
parcelable IfaceInfo {
    /**
     * Type of the interface (STA/AP)
     */
    IfaceType type;

    /**
     * Name of the interface (wlan*, swlan*, etc)
     */
    String name;
}


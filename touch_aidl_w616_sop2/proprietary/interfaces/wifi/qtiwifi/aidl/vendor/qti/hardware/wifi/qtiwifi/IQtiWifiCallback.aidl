/**
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.hardware.wifi.qtiwifi;

/**
 * QtiWifi callback interface
 */
@VintfStability
interface IQtiWifiCallback {
    /**
     * Invoked when vendor event is triggered
     * @param iface Name of the interface that triggers the event
     * @param event Event string
     */
    oneway void onCtrlEvent(String iface, String event);
}

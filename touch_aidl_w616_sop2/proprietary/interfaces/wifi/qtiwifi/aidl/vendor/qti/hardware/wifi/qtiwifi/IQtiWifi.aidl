/**
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.hardware.wifi.qtiwifi;

import vendor.qti.hardware.wifi.qtiwifi.IQtiWifiCallback;
import vendor.qti.hardware.wifi.qtiwifi.IfaceInfo;

/**
 * Top level interface of qtiwifi vendor service
 */
@VintfStability
interface IQtiWifi {
    /**
     * List available sta/ap interfaces
     * @return Array of IfaceInfo specifies available interfaces
     */
    IfaceInfo[] listAvailableInterfaces();

    /**
     * Register callback for vendor events
     * @param callback An instance of IQtiWifiCallback
     */
    void registerQtiWifiCallback(in IQtiWifiCallback callback);

    /**
     * Do vendor commands
     * @param iface Name of the interface that suppose to do the cmd
     * @param cmd Command to be executed
     * @return String of command results
     */
    String doQtiWifiCmd(in String iface, in String cmd);
}


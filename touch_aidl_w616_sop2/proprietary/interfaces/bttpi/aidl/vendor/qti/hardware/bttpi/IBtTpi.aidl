/*
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package vendor.qti.hardware.bttpi;

import vendor.qti.hardware.bttpi.BtTpiState;
import vendor.qti.hardware.bttpi.IBtTpiStatusCb;
import vendor.qti.hardware.bttpi.IBtTpiEventsCb;

@VintfStability
interface IBtTpi {
  /**
   * Set Tpi state
   * @param : state, enable/disable/peak
   * @param : statusCb, callback for the status of command
   * @Note: this API should be called to enable/disable Tpi
   * @return: 0: Success, Else: Different error codes
   */
  byte setTpiState(in BtTpiState state, IBtTpiStatusCb statusCb);

  /**
   * Set active DSI
   * @param : dsi, currently active DSI
   * @param : statusCb, callback for the status of command
   * @Note: this API should be called to set active DSI
   * @return: 0: Success, Else: Different error codes
   */
  byte setActiveDSI(in int dsi, IBtTpiStatusCb statusCb);

  /**
   * Set Airplane Mode
   * @param : airPlaneMode, true:enable, false:disable
   * @param : statusCb, callback for the status of command
   * @Note: this API should be called for setting current air plane mode
   * @return: 0: Success, Else: Different error codes
   */
  byte setAirplaneMode(in boolean airPlaneMode, IBtTpiStatusCb statusCb);

  /**
   * Set parameters for Tpi
   * @param : reqType, type of the Tpi request
   * @param : reqParams, payload for request
   * @param : statusCb, callback for the status of command
   * @Note: this API should be called when BT turns on
   * @return: 0: Success, Else: Different error codes
   */
  byte setTpiParams(in int reqType,
                    in byte[] reqParams,
                    IBtTpiStatusCb statusCb);

  /**
   * Register callback for the Tpi events
   * @param : eventCb, reference to callback function
   * @Note: this API should be called when BT turns on
   * @return: 0: Success, Else: Different error codes
   */
  byte registerTpiEventsCallback(in IBtTpiEventsCb eventCb);

  /**
   * UnRegister callback for the Tpi events
   * @param : None
   * @Note: None
   * @return: 0: Success, Else: Different error codes
   */
  byte UnRegisterTpiEventsCallback();

  /**
   * Set WiFi State
   * @param : wifiState, true:enable, false:disable
   * @param : statusCb, callback for the status of command
   * @Note: this API should be called for setting current WiFi state
   * @return: 0: Success, Else: Different error codes
   */
  byte setWiFiState(in boolean wifiState, IBtTpiStatusCb statusCb);
}

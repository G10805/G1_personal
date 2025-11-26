/*
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package vendor.qti.hardware.bttpi;

@VintfStability
interface IBtTpiEventsCb {
  /**
   * Send Tpi Event
   * @param : eventType, type of Event
   * @param : event, event details
   * @Note  : None
   * @return: Void
   */
  oneway void sendTpiEvent(in int eventType, in byte[] event);
}

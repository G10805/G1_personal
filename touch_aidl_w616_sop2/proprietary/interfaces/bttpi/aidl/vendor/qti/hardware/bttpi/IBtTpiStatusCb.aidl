/*
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package vendor.qti.hardware.bttpi;

@VintfStability
interface IBtTpiStatusCb {
  /**
   * set the status of Tpi request
   * @param : reqType, type of request
   * @param : status, 0:success, else failures
   * @Note  : None
   * @return: void
   */
  void setStatus(in int reqType, byte status);
}

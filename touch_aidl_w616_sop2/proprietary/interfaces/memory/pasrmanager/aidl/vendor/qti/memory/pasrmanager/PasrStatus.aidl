/*!
 * @file IPasrManager.hal
 *
 * @cr
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @services Defines the external interface for PASR Manager.
 */


package vendor.qti.memory.pasrmanager;

@VintfStability
@Backing(type="int")
enum PasrStatus {
    ERROR = -10,
    INCOMPLETE_ONLINE = -2,
    INCOMPLETE_OFFLINE = -1,
    OFFLINE = 1,
    ONLINE = 2,
    SUCCESS = 3,
}

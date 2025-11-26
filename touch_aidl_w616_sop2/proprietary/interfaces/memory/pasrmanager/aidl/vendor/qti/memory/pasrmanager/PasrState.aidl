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
@Backing(type="byte")
enum PasrState {
    MEMORY_ONLINE = 0,
    MEMORY_OFFLINE,
    MEMORY_UNKNOWN,
    MAX_STATE,
}

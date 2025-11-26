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

/*
 * Priority levels of triggering PASR commands
 */
@VintfStability
@Backing(type="int")
enum PasrPriority {
    PASR_PRI_NONE = 0,
    PASR_PRI_LOW,
    PASR_PRI_CRITICAL,
}

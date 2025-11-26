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
 * Source or initiator of PASR trigger
 */
@VintfStability
@Backing(type="long")
enum PasrSrc {
    PASR_SRC_PSI = 0,
    PASR_SRC_POWER,
    PASR_SRC_PERF,
    PASR_SRC_HAL,
    PASR_SRC_MAX,
}

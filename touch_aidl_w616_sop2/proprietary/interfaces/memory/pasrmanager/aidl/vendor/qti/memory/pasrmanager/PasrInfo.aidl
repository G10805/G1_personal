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
 * PASR info: DDR size, granule, total blocks
 */
@VintfStability
parcelable PasrInfo {
    int ddr_size;
    int granule;
    int num_blocks;
    long min_free_mem;
}

/*
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package vendor.qti.hardware.bttpi;

@VintfStability
@Backing(type="byte")
enum BtTpiState {
    TPI_STATE_DISABLE = 0,
    TPI_STATE_ENABLE = 1,
    TPI_STATE_PEAK = 2,
}

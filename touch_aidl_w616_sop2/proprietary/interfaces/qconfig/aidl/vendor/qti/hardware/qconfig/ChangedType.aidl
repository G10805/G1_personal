/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.hardware.qconfig;

/*
 * Changed type for QConfigListener
 *
 */
@VintfStability
@Backing(type="int")
enum ChangedType {
    ADDED,
    MODIFIED,
    REMOVED,
}

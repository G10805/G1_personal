/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

// FIXME: license file, or use the -l option to generate the files with the header.

package vendor.qti.hardware.perf2;

@VintfStability
interface IPerfCallback {
    oneway void notifyCallback(in int hint, in String userDataStr, in int userData1,
        in int userData2, in int[] reserved);
}

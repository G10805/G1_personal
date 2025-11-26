/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.hardware.qteeconnector;
import android.hardware.common.Ashmem;
import android.hardware.common.NativeHandle;

@VintfStability
parcelable QTEECom_ion_fd_data {
    /**
     * fd_handle hidl_handle to transfer file descriptors accross process boundaries
     */
    NativeHandle fd_handle;
    /**
     * cmd_buf_offset offsets within the cmdbuf belonging to the file descriptors identifying
     * shared memory
     */
    int cmd_buf_offset;
}

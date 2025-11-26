/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

import android.hardware.common.NativeHandle;

@VintfStability
parcelable VppMemBuffer {
    /*
     * ! File descriptor of the buffer.
     */
    NativeHandle handleFd;
    /*
     * ! Offset of the buffer from the base address, in bytes
     */
    int offset;
    /*
     * ! Physically allocated buffer length, in bytes
     */
    int allocLen;
    /*
     * ! Filled buffer length, in bytes
     */
    int filledLen;
    /*
     * !
     * The number of bytes that *may* contain data.
     *
     * In the typical scenario where there is both pixel data and extra data in
     * the buffer, this field shall describe the valid data length of each
     * section. That is, for the pixel region, valid_data_len will describe
     * the maximum number of bytes that can be occupied by pixel data, and for
     * the extra data region, valid_data_len will describe the maximum number
     * of bytes that can be occupied by the extra data. In such a case,
     * alloc_len shall be equal for both the pixel data and the extra data
     * regions.
     *
     * In the case where a single physical buffer contains multiple buffers,
     * this field is used to describe just the memory region that is being
     * described by this structure. As an example, if there are multiple extra
     * data buffers in one physical buffer, the valid_data_len for each extra
     * data buffer shall equal the maximum number of bytes that a specific
     * extra data region data may occupy in that buffer. In this case, the
     * alloc_len for each buffer segment in the extradata buffer will be equal.
     */
    int validDataLen;
}

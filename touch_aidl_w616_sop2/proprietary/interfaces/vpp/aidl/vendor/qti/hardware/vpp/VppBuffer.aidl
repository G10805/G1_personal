/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

import vendor.qti.hardware.vpp.VppMemBuffer;
import vendor.qti.hardware.vpp.VppMetaBuffer;

@VintfStability
parcelable VppBuffer {
    VppMemBuffer pixel;
    VppMemBuffer extradata;
    VppMetaBuffer[] metabufs;
    /*
     * ! Buffer flags, as defined by enum vpp_buffer_flag
     */
    int flags;
    /*
     * ! Timestamp in microseconds
     */
    long timestamp;
    /*
     * ! Cookie to be copied from the input buffer to the output buffer.
     */
    long cookieInToOut;
    /*
     * ! cookie associated with this buffer
     */
    long cookie;
}

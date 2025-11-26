/*====================================================================
*  Copyright (c) Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*====================================================================
*/

package vendor.qti.hardware.dsp;

import android.hardware.common.NativeHandle;
import vendor.qti.hardware.dsp.DSPError;

@VintfStability
parcelable FastrpcDev {
    /*
     * Status indicating whether closing or opening device
     * node is successful.
     */
    DSPError err;

    /*
     * Structure to hold the device fd when opening
     * of device node is successful.
     */
    NativeHandle dev_handle;
}

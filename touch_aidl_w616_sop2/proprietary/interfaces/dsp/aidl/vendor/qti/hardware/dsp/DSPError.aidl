/*====================================================================
*  Copyright (c) Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*====================================================================
*/

package vendor.qti.hardware.dsp;

@VintfStability
@Backing(type="int")
enum DSPError {
    /*
     * Status indicating whether closing or opening device
     * node is successful.
     */
    SUCCESS = 0,
    ERROR = -1,
}

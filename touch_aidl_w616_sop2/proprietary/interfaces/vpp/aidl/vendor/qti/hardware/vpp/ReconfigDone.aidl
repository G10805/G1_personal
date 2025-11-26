/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

@VintfStability
parcelable ReconfigDone {
    /*
     * !
     * Status of the reconfigure. If this is set to error, then the
     * client should bypass the VPP.
     */
    int reconfigStatus;
}

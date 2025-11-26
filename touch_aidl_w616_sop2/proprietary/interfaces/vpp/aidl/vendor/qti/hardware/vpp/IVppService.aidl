/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

import vendor.qti.hardware.vpp.IVpp;

@VintfStability
interface IVppService {
    // Changing method name from getNewVppSession_2_0 to getNewVppSession
    // Adding return type to method instead of out param IVpp vppInstance since there is only one return value.
    IVpp getNewVppSession(in int u32Flags);
}

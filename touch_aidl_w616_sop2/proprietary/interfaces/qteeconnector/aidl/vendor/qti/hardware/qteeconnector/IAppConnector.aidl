/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.hardware.qteeconnector;

import vendor.qti.hardware.qteeconnector.IApp;

/**
 * The interface to load a legacy app
 */
@VintfStability
interface IAppConnector {
    /**
     * load a legacy app
     *
     * @param path path where the app resides
     * @param name name of the app
     * @param ionBufferSize size of the ionBuffer
     * @param out status status of the load attempt
     * @param out isApp64 whether the loaded app was determined to be running in 64bit mode
     * @param out app the app object representing the app
     */
    void load(in String path, in String name, in int ionBufferSize,
        out int[] status, out boolean[] isApp64, out IApp[] app);
}

/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.hardware.qteeconnector;

import vendor.qti.hardware.qteeconnector.IGPApp;

/**
 * The interface to load a GP app
 */
@VintfStability
interface IGPAppConnector {
    /**
     * load a gp app
     *
     * @param path path where the app resides
     * @param name name of the app
     * @param ionBufferSize size of the ionBuffer
     * @param out status status of the load attempt
     * @param out isApp64 whether the loaded app was determined to be running in 64bit mode
     * @param out app the app object representing the app
     */
    void load(in String path, in String name, in int ionBufferSize,
        out int[] status, out boolean[] isApp64, out IGPApp[] app);
}

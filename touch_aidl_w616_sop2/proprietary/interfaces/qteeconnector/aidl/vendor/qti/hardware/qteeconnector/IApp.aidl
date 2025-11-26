/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.hardware.qteeconnector;

import vendor.qti.hardware.qteeconnector.QTEECom_ion_fd_info;

/**
 * The interface representing an App
 */
@VintfStability
interface IApp {
    /**
     * send a command to this App
     *
     * @param cmd the command to be sent to the app
     * @param rspLen size of the response buffer
     * @param out status status of the call, 0 on success, errorcode otherwise
     * @param out rsp the response to this command
     */
    void sendCommand(in byte[] cmd, in int rspLen, out int[] status, out byte[] rsp);

    /**
     * send a modified command to this App
     *
     * @param req the request to be sent to the app
     * @param rspLen size of the response buffer
     * @param info file descriptors to be mapped in the command buffer
     * @param out status status of the call, 0 on success, errorcode otherwise
     * @param out rsp the response to this request
     */
    void sendModifiedCommand(in byte[] req, in int rspLen, in QTEECom_ion_fd_info info,
        out int[] status, out byte[] rsp);

    // Adding return type to method instead of out param int status since there is only one return value.
    /**
     * Unload the App
     *
     * @return 0 on success, errorcode otherwise
     */
    int unload();
}

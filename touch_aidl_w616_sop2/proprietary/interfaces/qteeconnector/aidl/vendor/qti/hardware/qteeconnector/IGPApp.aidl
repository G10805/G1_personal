/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.hardware.qteeconnector;

import vendor.qti.hardware.qteeconnector.QTEECom_ion_fd_info;

@VintfStability
interface IGPApp {
    /**
     * close the session
     *
     * @param req the command to be sent to the app
     * @param rspLen size of the response buffer
     * @param out status status of the call, 0 on success, errorcode otherwise
     * @param out rsp the response to the close session
     */
    void closeSession(in byte[] req, in int rspLen, out int[] status, out byte[] rsp);

    /**
     * invoke a Command
     *
     * @param req the request to be sent to the app
     * @param rspLen size of the response buffer
     * @param info file descriptors to be mapped in the command buffer
     * @param out status status of the call, 0 on success, errorcode otherwise
     * @param out rsp the response to this request
     */
    void invokeCommand(in byte[] req, in int rspLen, in QTEECom_ion_fd_info info,
        out int[] status, out byte[] rsp);

    /**
     * open a session
     * @param req the request to be sent to the app
     * @param rspLen size of the response buffer
     * @param info file descriptors to be mapped in the command buffer
     * @param out status status of the call, 0 on success, errorcode otherwise
     * @param out rsp the response to this request
     */
    void openSession(in byte[] req, in int rspLen, in QTEECom_ion_fd_info info,
        out int[] status, out byte[] rsp);

    // Adding return type to method instead of out param int status since there is only one return value.
    /**
     * requestCancellation
     *
     * @param sessionId id of the session to be cancelled
     * @return status of the call, 0 on success, errorcode otherwise
     */
    int requestCancellation(in int sessionId);

    // Adding return type to method instead of out param int status since there is only one return value.
    /**
     * Unload the GP App
     *
     * @return 0 on success, errorcode otherwise
     */
    int unload();
}

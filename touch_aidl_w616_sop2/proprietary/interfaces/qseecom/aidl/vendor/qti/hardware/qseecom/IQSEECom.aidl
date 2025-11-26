/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.hardware.qseecom;
import vendor.qti.hardware.qseecom.IQSEEComCallback;
import android.hardware.common.Ashmem;
import android.hardware.common.NativeHandle;

/**
 * Interface for QSEECom API.
 */
@VintfStability
interface IQSEECom {
    @VintfStability
    parcelable AppHandle {
        long vendorLibHandle;
    }
    @VintfStability
    parcelable AppInfo {
        boolean is64;
        int requiredSGBufferSize;
        byte[64] reserved;
    }
    @VintfStability
    parcelable Buffer {
        int offset;
        int length;
    }
    @VintfStability
    parcelable ModifiedBuffer {
        boolean validFd;
        int offset;
    }
    @VintfStability
    parcelable ModifiedBufferInfo {
        ModifiedBuffer[4] data;
        NativeHandle ionFd;
    }
    // Adding return type to method instead of out param int status since there is only one return value.
    /**
     * Query QSEE to check if app is loaded.
     */
    int appLoadQuery(in AppHandle appHandle, in String appName);

    /**
     * Get app info, including secure app arch type, sg_buffer_size, etc.
     */
    void getAppInfo(in AppHandle appHandle, out int[] status, out AppInfo info);

    // Adding return type to method instead of out param int status since there is only one return value.
    /**
     * Receive a service defined buffer.
     */
    int receiveRequest(in AppHandle appHandle, in Buffer buf);

    /**
     * Register an HLOS listener service.
     */
    void registerListner(in int listnerId, in int sharedBufferSize, in int flags,
        in IQSEEComCallback cbToken,
        out int[] status, out AppHandle appHandle, out NativeHandle sharedBuffer);

    // Adding return type to method instead of out param int status since there is only one return value.
    /**
     * Scale bus bandwidth.
     */
    int scaleBusBandwidth(in AppHandle appHandle, in int mode);

    // Adding return type to method instead of out param int status since there is only one return value.
    /**
     * Send QSAPP a "user" defined buffer (may contain some message/
     * command request) and receives a response from QSAPP in receive buffer.
     */
    int sendCommand(in AppHandle appHandle, in Buffer send, in Buffer rsp);

    // Adding return type to method instead of out param int status since there is only one return value.
    /**
     * Send QSAPP a "user" defined buffer (may contain some message/
     * command request) and receives a response from QSAPP in receive buffer.
     */
    int sendModifiedCommand(in AppHandle appHandle, in Buffer send, in Buffer rsp,
        in ModifiedBufferInfo bufferInfo, in boolean is64);

    // Adding return type to method instead of out param int status since there is only one return value.
    /**
     * A "user" defined API that contains the Ion fd allocated by the
     * listener service, to be communicated with TZ.
     */
    int sendModifiedResponse(in AppHandle appHandle, in Buffer send,
        in ModifiedBufferInfo bufferInfo, in boolean is64);

    // Adding return type to method instead of out param int status since there is only one return value.
    /**
     * Send a response based on the previous receiveRequest.
     */
    int sendResponse(in AppHandle appHandle, in Buffer send);

    // Adding return type to method instead of out param int status since there is only one return value.
    /**
     * Set the bandwidth for QSEE.
     */
    int setBandwidth(in AppHandle appHandle, in boolean high);

    // Adding return type to method instead of out param int status since there is only one return value.
    /**
     * Close the application associated with the handle.
     */
    int shutdownApp(in AppHandle appHandle);

    /**
     * Start App V1 - Load a secure application.
     */
    void startApp(in String path, in String name, in int sharedBufferSize,
        in IQSEEComCallback cbToken,
        out int[] status, out AppHandle appHandle, out NativeHandle sharedBuffer);

    /**
     * Load a secure application from a memory buffer containing the Trusted App.
     */
    void startAppV2(in String name, in Ashmem trustlet,
        in int sharedBufferSize, in IQSEEComCallback cbToken,
        out int[] status, out AppHandle appHandle, out NativeHandle sharedBuffer);

    // Adding return type to method instead of out param int status since there is only one return value.
    /**
     * Unregister a listener service.
     */
    int unregisterListner(in AppHandle appHandle);
}

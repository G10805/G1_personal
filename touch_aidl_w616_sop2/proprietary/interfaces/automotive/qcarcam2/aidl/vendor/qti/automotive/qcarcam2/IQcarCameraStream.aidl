/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

import vendor.qti.automotive.qcarcam2.IQcarCameraStreamCB;
import vendor.qti.automotive.qcarcam2.QCarCamError;
import vendor.qti.automotive.qcarcam2.QCarCamFrameInfo;
import vendor.qti.automotive.qcarcam2.QCarCamParamType;
import vendor.qti.automotive.qcarcam2.QCarCamStreamConfigData;
import vendor.qti.automotive.qcarcam2.QcarcamBuffersInfo;
import vendor.qti.automotive.qcarcam2.QCarCamRequest;
import android.hardware.common.NativeHandle;

/**
 * Represents a automotive camera interfaces.
 */
@VintfStability
interface IQcarCameraStream {
    // Adding return type to method instead of out param QCarCamError Error since there is only one return value.
    /**
     * Request to configure particular input stream.
     *
     * Returns the Error status and Handle to input stream.
     */
    QCarCamError configureStream(in QCarCamParamType Param, in QCarCamStreamConfigData data);

    /**
     * Request frames from particular input stream.
     *
     * Returns the Error status and retrived frame info.
     */
    QCarCamError getFrame(in long timeout, in int flags,
        out QCarCamFrameInfo Info);

    /**
     * Returns particular stream configurations.
     *
     * Returns the Error status and QCarCamStreamConfigData containing stream
     * configurations.
     */
    QCarCamError getStreamConfig(in QCarCamParamType Param, in QCarCamStreamConfigData data,
        out QCarCamStreamConfigData datatype);

    // Adding return type to method instead of out param QCarCamError Error since there is only one return value.
    /**
     * Request to pause particular stream.
     *
     * Returns the Error status and Handle to paused stream. This stream will
     * not produce any frames untill it resumed.
     */
    QCarCamError pauseStream();

    // Adding return type to method instead of out param QCarCamError Error since there is only one return value.
    /**
     * Request clear frames from particular stream.
     *
     * Returns the Error status and Handle to input stream.
     */
    QCarCamError releaseFrame(in int id, in int bufferIdx);

    // Adding return type to method instead of out param QCarCamError Error since there is only one return value.
    /**
     * Request resume particular stream.
     *
     * Returns the Error status and Handle to input stream. This stream will
     * start producing frames after the successful resume.
     */
    QCarCamError resumeStream();

    // Adding return type to method instead of out param QCarCamError Error since there is only one return value.
    /**
     * Request to set output buffers on particular stream.
     *
     * Returns the Error status and Handle to input stream. Allication has to
     * fill in the handle and QcarcamBuffersInfo.
     */
    QCarCamError setStreamBuffers(in NativeHandle hndl, in QcarcamBuffersInfo info);

    // Adding return type to method instead of out param QCarCamError Error since there is only one return value.
    /**
     * Request to start particular input stream.
     *
     * Returns the Error status and Handle to started stream.
     */
    QCarCamError startStream(in IQcarCameraStreamCB streamObj);

    // Adding return type to method instead of out param QCarCamError Error since there is only one return value.
    /**
     * Request to stop particular input stream.
     *
     * Returns the Error status and Handle to stopped stream.
     */
    QCarCamError stopStream();

    // Adding return type to method instead of out param QCarCamError Error since there is only one return value.
    /**
     * Request to reserver particular input stream.
     *
     * Returns the Error status and Handle to reserved stream.
     */
    QCarCamError reserveStream();

    // Adding return type to method instead of out param QCarCamError Error since there is only one return value.
    /**
     * Request to release particular input stream.
     *
     * Returns the Error status and Handle to released stream.
     */
    QCarCamError releaseStream();

    // Adding return type to method instead of out param QCarCamError Error since there is only one return value.
    /**
     * Request to submitRequest particular input stream.
     *
     * Returns the Error status and Handle to released stream.
     */
    QCarCamError submitRequest(in QCarCamRequest Info);
}

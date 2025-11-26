/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.IQcarCameraStreamCB;
import vendor.qti.automotive.qcarcam.QcarcamBuffersInfo;
import vendor.qti.automotive.qcarcam.QcarcamError;
import vendor.qti.automotive.qcarcam.QcarcamStreamConfigData;
import vendor.qti.automotive.qcarcam.QcarcamStreamParam;
import vendor.qti.automotive.qcarcam.IQcarCameraStreamCB;
import vendor.qti.automotive.qcarcam.QcarcamBuffersInfoList;
import vendor.qti.automotive.qcarcam.QcarcamFrameInfov2;
import vendor.qti.automotive.qcarcam.QcarcamStreamConfigData;
import vendor.qti.automotive.qcarcam.QcarcamStreamParam;
import android.hardware.common.NativeHandle;

/**
 * Represents a automotive camera interfaces.
 */
@VintfStability
interface IQcarCameraStream {
    /**
     * Request to configure particular input stream.
     *
     * Returns the Error status and Handle to input stream.
     */
    QcarcamError configureStream_1_1(in QcarcamStreamParam Param,
        in QcarcamStreamConfigData data);

    /**
     * Request frames from particular input stream.
     *
     * Returns the Error status and retrived frame info.
     */
    QcarcamError getFrame_1_1(in long timeout, in int flags,
        out QcarcamFrameInfov2 Info);

    /**
     * Returns particular stream configurations.
     *
     * Returns the Error status and QcarcamStreamConfigData containing stream
     * configurations.
     */
    QcarcamError getStreamConfig_1_1(in QcarcamStreamParam Param,
        out QcarcamStreamConfigData data);

    /**
     * Request to pause particular stream.
     *
     * Returns the Error status and Handle to paused stream. This stream will
     * not produce any frames untill it resumed.
     */
    QcarcamError pauseStream();

    /**
     * Request clear frames from particular stream.
     *
     * Returns the Error status and Handle to input stream.
     */
    QcarcamError releaseFrame_1_1(in int id, in int idx);

    /**
     * Request resume particular stream.
     *
     * Returns the Error status and Handle to input stream. This stream will
     * start producing frames after the successful resume.
     */
    QcarcamError resumeStream();

    /**
     * Request to set output buffers on particular stream.
     *
     * Returns the Error status and Handle to input stream. Allication has to
     * fill in the handle and QcarcamBuffersInfo.
     */
    QcarcamError setStreamBuffers_1_1(in NativeHandle hndl,
        in QcarcamBuffersInfoList info);

    /**
     * Request to start particular input stream.
     *
     * Returns the Error status and Handle to started stream.
     */
    QcarcamError startStream_1_1(in IQcarCameraStreamCB streamObj);

    /**
     * Request to stop particular input stream.
     *
     * Returns the Error status and Handle to stopped stream.
     */
    QcarcamError stopStream_1_1();
}

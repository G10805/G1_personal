/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

import vendor.qti.automotive.qcarcam2.IQcarCameraStream;
import vendor.qti.automotive.qcarcam2.QCarCamError;
import vendor.qti.automotive.qcarcam2.QCarCamInput;
import vendor.qti.automotive.qcarcam2.QCarCamInputModes;
import vendor.qti.automotive.qcarcam2.QCarCamOpenParam;

/**
 * Represents a automotive camera interfaces.
 */
@VintfStability
interface IQcarCamera {
    // Adding return type to method instead of out param QCarCamError Error since there is only one return value.
    /**
     * Request closing of reqiured input stream.
     *
     * Returns the Error status and Handle to closed stream.
     */
    QCarCamError closeStream(in IQcarCameraStream camStream);

    /**
     * Returns supported cameras info.
     *
     * Returns the vector of QcarcamInputInfo. Contains all supported input
     * informations
     */
    QCarCamError getInputStreamList(out QCarCamInput[] inputs);

    /**
     * Returns supported stream modes.
     *
     * Returns QCarCaminputModes. Contains all supported mode info
     */
    QCarCamError getInputStreamMode(in int inputId, out QCarCamInputModes modes);

    /**
     * Request opening of reqiured input stream.
     *
     * Returns the Error status and Handle to opened stream.
     */
    IQcarCameraStream openStream(in QCarCamOpenParam OpenParam);
}

/*
 * Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.IQcarCameraStream;
import vendor.qti.automotive.qcarcam.QcarcamError;
import vendor.qti.automotive.qcarcam.QcarcamInputDesc;
import vendor.qti.automotive.qcarcam.QcarcamInputInfo;
import vendor.qti.automotive.qcarcam.QcarcamInputInfov2;

/**
 * Represents a automotive camera interfaces.
 */
@VintfStability
interface IQcarCamera {
    /**
     * Returns supported cameras info.
     *
     * Returns the vector of QcarcamInputInfo. Contains all supported input
     * informations
     *
     * This API is currently deprecated.
     */
    QcarcamError getInputStreamList_1_1(out QcarcamInputInfov2[] inputs);

    /**
     * Request closing of reqiured input stream.
     *
     * Returns the Error status and Handle to closed stream.
     *
     * This API is currently deprecated.
     */
    QcarcamError closeStream_1_1(in IQcarCameraStream camStream);

    /**
     * Request opening of reqiured input stream.
     *
     * Returns the Error status and Handle to opened stream.
     *
     * This API is currently deprecated.
     */
    IQcarCameraStream openStream_1_1(in QcarcamInputDesc Desc);

    /**
     * Returns supported cameras info.
     *
     * Returns the vector of QcarcamInputInfo. Contains all supported input
     * informations
     */
    QcarcamError getInputStreamList(out QcarcamInputInfov2[] inputs);

    /**
     * Request closing of reqiured input stream.
     *
     * Returns the Error status and Handle to closed stream.
     */
    QcarcamError closeStream(in IQcarCameraStream camStream);

    /**
     * Request opening of reqiured input stream.
     *
     * Returns the Error status and Handle to opened stream.
     */
    IQcarCameraStream openStream(in QcarcamInputDesc Desc);
}

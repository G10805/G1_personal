/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.QcarcamEvent;
import vendor.qti.automotive.qcarcam.QcarcamEventPayload;

/**
 * Implemented on client side to receive asynchronous event deliveries.
 */
@VintfStability
interface IQcarCameraStreamCB {
    /**
     * Receives calls from the HAL for each asynchronous camera events.
     */
    oneway void qcarcam_event_callback_1_1(in QcarcamEvent EventType,
        in QcarcamEventPayload Payload);
}

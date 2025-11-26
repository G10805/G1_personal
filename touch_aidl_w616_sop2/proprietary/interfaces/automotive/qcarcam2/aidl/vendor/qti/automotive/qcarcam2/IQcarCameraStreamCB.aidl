/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

import vendor.qti.automotive.qcarcam2.QCarCamEventPayload;

/**
 * Implemented on client side to receive asynchronous event deliveries.
 */
@VintfStability
interface IQcarCameraStreamCB {
    /**
     * Receives calls from the HAL for each asynchronous camera events.
     */
    oneway void qcarcam_event_callback(in int EventType, in QCarCamEventPayload Payload);
}

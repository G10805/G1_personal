/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

import vendor.qti.automotive.qcarcam2.QCarCamStreamRequest;
import vendor.qti.automotive.qcarcam2.QCarCamBufferRequest;

@VintfStability
parcelable QCarCamRequest {
    int requestId;       
    QCarCamBufferRequest inputBuffer;  
    QCarCamStreamRequest[32] streamRequests; 
    int numStreamRequests; 
    QCarCamBufferRequest inputCommonMetadata; 
    QCarCamBufferRequest[32] inputMetadata;   
    QCarCamBufferRequest[32] outputMetadata;  
    int flags; 
}
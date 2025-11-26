/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

@VintfStability
parcelable QCarCamStreamRequest {
    int bufferlistId; 
    int bufferIdx;    
    int metaBufferlistId; 
    int metaBufferId;
}

    
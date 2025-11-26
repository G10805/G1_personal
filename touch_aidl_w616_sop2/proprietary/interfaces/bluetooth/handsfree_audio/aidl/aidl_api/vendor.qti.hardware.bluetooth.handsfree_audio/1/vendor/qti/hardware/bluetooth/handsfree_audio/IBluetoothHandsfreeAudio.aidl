/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.hardware.bluetooth.handsfree_audio;

import vendor.qti.hardware.bluetooth.handsfree_audio.HandsfreeAudioParameter;

/* Bluetooth Handsfree Audio AIDL interface */
@VintfStability
interface IBluetoothHandsfreeAudio {
    void start(in HandsfreeAudioParameter data);

    void stop();
}

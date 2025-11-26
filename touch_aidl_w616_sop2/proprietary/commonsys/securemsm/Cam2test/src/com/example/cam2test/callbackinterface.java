/*=============================================================================
Copyright (c) 2022 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.qti.cam2test;

import android.util.Log;

public class callbackinterface  extends servicestatus {
    public interface StatusCallback {
        public void finishSession();
    }

    public void enableCallback(StatusCallback statusCallback) {
        super.enableCallback(statusCallback);
    }
}


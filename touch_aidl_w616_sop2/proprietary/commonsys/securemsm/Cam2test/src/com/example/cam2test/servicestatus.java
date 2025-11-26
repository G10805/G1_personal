/*=============================================================================
Copyright (c) 2022 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.qti.cam2test;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import androidx.annotation.Nullable;
import android.util.Log;
import com.qualcomm.qti.cam2test.callbackinterface.StatusCallback;

public class servicestatus extends Service {

    public static callbackinterface.StatusCallback mCallObj = null;

    @Nullable @Override public IBinder onBind(Intent intent) { return null; }

    public void enableCallback(callbackinterface.StatusCallback sb) {
        if (sb != null) {
            mCallObj = sb;
        }
    }
    @Override public void onTaskRemoved(Intent rootIntent)
    {
        if(mCallObj!=null) {
            mCallObj.finishSession();
        }
    }
}



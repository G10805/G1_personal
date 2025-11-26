/******************************************************************************
 @file BootCompleteReceiver.java
 @brief Recevies bootcomplete broadcast and starts service

 ---------------------------------------------------------------------------
 Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 All rights reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qti.carpowerservice;

import android.os.Bundle;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.os.UserHandle;
import android.os.UserManager;

public class BootCompleteReceiver extends BroadcastReceiver {
    private static final String TAG = CarPowerService.TAG;

    @Override
    public void onReceive(Context context, Intent intent) {
        // Run it only once for the system user (u0) and ignore for other users.
        UserManager userManager = context.getSystemService(UserManager.class);
        if (!userManager.isSystemUser()) {
            return;
        }
        Log.d(TAG, "Received Intent " + intent.toString());

        ComponentName comp = new ComponentName(context.getPackageName(),CarPowerService.class.getName());
        if (comp != null) {
            ComponentName service = context.startService(new Intent().setComponent(comp));
            if (service == null) {
                Log.e(TAG, "Could Not Start Service " + comp.toString());
            } else {
                Log.d(TAG, "CarPowerService Started Successfully");
            }
        } else {
            Log.e(TAG, "CarPowerService not Started Successfully");
        }
    }
}

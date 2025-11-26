/*
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.services.systemhelper;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

/**
 * Intent receiver for the following:
 * ACTION_USER_FOREGROUND
 * ACTION_USER_BACKGROUND
 * ACTION_USER_SWITCHED
 * */
public class UserSwitchReceiver extends BroadcastReceiver {

    /*
     * @see android.content.BroadcastReceiver#onReceive(android.content.Context, android.content.Intent)
     */
    @Override
    public void onReceive(Context context, Intent intent) {
        String intentAction = intent.getAction();
        if (intentAction != null) {
            if(intentAction.equals(Intent.ACTION_USER_FOREGROUND) ||
               intentAction.equals(Intent.ACTION_USER_BACKGROUND) ||
               intentAction.equals(Intent.ACTION_USER_SWITCHED)) {
                SysHelperService.sendEventNotification(SysHelperService.ACTION_SCREEN_OFF);
                SysHelperService.sendEventNotification(SysHelperService.ACTION_SCREEN_ON);
            }
        }
    }
}

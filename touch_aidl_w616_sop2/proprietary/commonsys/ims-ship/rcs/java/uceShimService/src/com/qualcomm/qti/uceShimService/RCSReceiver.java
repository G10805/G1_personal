/*********************************************************************
 Copyright (c) 2017,2019,2020, 2023 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

package com.qualcomm.qti.uceShimService;

import java.io.BufferedReader;
import java.io.InputStreamReader;

import com.qualcomm.qti.uceShimService.RCSService;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.os.SystemProperties;

//secure boot complete
public class RCSReceiver extends BroadcastReceiver {

    private final static String logTAG = "RCSService BC-Receiver";
    private static int SINGLE_REG_ENABLE_PROPERTY = 0;
    private final static String BASEBAND_PROPERTY = "ro.baseband";
    private static String[] dataServicesDisabledBands = {"apq", "sda", "qcs"};
    private static int rcsSingleRegEnable = 0;

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent == null) {
            Log.e(logTAG, "Null Intent received");
            return;
        }

        final String intentActionStr = intent.getAction();
        if (intentActionStr == null) {
            Log.e(logTAG, "Null intent action received");
            return;
        }

        if (intentActionStr.equals(RCSService.INTENT_BOOT_COMPLETE_STR) ||
            intentActionStr.equals(Intent.ACTION_LOCKED_BOOT_COMPLETED)) {
                rcsSingleRegEnable = SystemProperties.getInt("persist.vendor.rcs.singlereg.feature", SINGLE_REG_ENABLE_PROPERTY);
                Log.d(logTAG, "Boot Complete Event Received. Is IMS RCS Single Reg enabled" + rcsSingleRegEnable);
                if(rcsSingleRegEnable == 0) {
                    Boolean disabled = false;
                    String basebandStr = SystemProperties.get(BASEBAND_PROPERTY);
                    for(String band : dataServicesDisabledBands) {
                        if(band.equals(basebandStr)) {
                            disabled = true;
                            break;
                        }
                    }
                    if (!disabled) {
                        Intent startRCSServiceIntent = new Intent(context, RCSService.class);
                        Log.d(logTAG, "START RCS SERVICE ON BOOTUP");
                        context.startService(startRCSServiceIntent);
                    } else {
                        Log.d(logTAG, "RCS Service is disabled for baseband type: " + basebandStr);
                    }
                } else {
                    Log.d(logTAG, "DO NOT START RCS SERVICE as IMS RCS Single Reg is enabled");
                }

        } else {
            Log.d(logTAG, "UCE Service is disabled Please check Configuration");
        }
    }
}

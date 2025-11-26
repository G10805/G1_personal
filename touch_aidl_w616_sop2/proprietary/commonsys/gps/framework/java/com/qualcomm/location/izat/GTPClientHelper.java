/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.izat;

import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.os.UserHandle;
import android.app.PendingIntent;
import com.qualcomm.location.izat.IzatService;
import com.qualcomm.location.izatprovider.IzatProvider;

public class GTPClientHelper {
    private static final String TAG = "GTPClientHelper";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);
    private static GTPClientHelper sInstance = null;

    private int mGtpClientsMask;
    private Context mContext;

    // From version 7.4.1, only provider connection set implicit opt-in state
    public static final int GTP_CLIENT_WIFI_PROVIDER = 0x1;
    public static final int GTP_CLIENT_WWAN_PROVIDER = 0x2;

    private GTPClientHelper(Context ctx) {
        mGtpClientsMask = 0;
        mContext = ctx;
    }

    public static void SetClientRegistrationStatus(Context ctx, int clientMask,
            PendingIntent pendingIntent, boolean status) {
        if (sInstance == null) {
            sInstance = new GTPClientHelper(ctx);
        }

        if (status) {
            sInstance.mGtpClientsMask |= clientMask;
        } else {
            sInstance.mGtpClientsMask &= ~clientMask;
        }

        if (pendingIntent != null) {
            IzatService.SsrHandler.get().
                    updateClientPackageName(ctx, pendingIntent, status);
        }
    }

    public static void SendPendingIntent(Context ctx, PendingIntent pendingIntent, String data) {
        if (null != pendingIntent) {
            Intent pdIntent = new Intent();
            pdIntent.putExtra("context-data", data);
            try {
                pendingIntent.send(ctx, 0, pdIntent);
            } catch (PendingIntent.CanceledException e) {
                Log.w(TAG, "Pending intent cancelled.");
            }
        } else {
            Log.w(TAG, "Pending intent is null for " + data);
        }
    }
}

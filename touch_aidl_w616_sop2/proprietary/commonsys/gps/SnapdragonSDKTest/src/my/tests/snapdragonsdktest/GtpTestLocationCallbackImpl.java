/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
package my.tests.snapdragonsdktest;

import android.app.Activity;
import android.location.Location;
import android.util.Log;
import com.qti.location.sdk.IZatGtpService;
import android.os.Handler;
import android.os.Message;
import android.os.Bundle;

public class GtpTestLocationCallbackImpl implements IZatGtpService.IZatGtpServiceCallback {
    private static final String TAG = "GTPTestCallbackInApp";

    private Handler mHandler;
    private String sessionName;
    private int fixCounter;

    private static final int MSG_UPDATE_LOC_REPORT = 1;

    private static GtpTestLocationCallbackImpl sInstance;

    private GtpTestLocationCallbackImpl(String sessionName, Handler handler) {
        this.sessionName = sessionName;
        this.mHandler = handler;
        fixCounter = 0;
    }

    public static GtpTestLocationCallbackImpl getInstance(String sessionName, Handler handler) {
        if (sInstance == null) {
            sInstance = new GtpTestLocationCallbackImpl(sessionName, handler);
        } else {
            sInstance.sessionName = sessionName;
            sInstance.mHandler = handler;
        }
        return sInstance;
    }

    public static void setHandler(Handler handler) {
        if (sInstance != null) {
            sInstance.mHandler = handler;
        }
    }

    @Override
    public void onLocationAvailable(Location loc) {

        Log.d(TAG, "Latitude:" + loc.getLatitude() +
                " Longitude:" + loc.getLongitude() +
                " Accuracy:" + loc.getAccuracy() +
                " UTC Time:" + loc.getTime() +
                " Altitude:" + loc.getAltitude() +
                " Vertical Uncertainity:" + loc.getVerticalAccuracyMeters() +
                " Speed:" + loc.getSpeed() +
                " Speed Accuracy:" + loc.getSpeedAccuracyMetersPerSecond() +
                " Bearing:" + loc.getBearing() +
                " Bearing Accuracy:" + loc.getBearingAccuracyDegrees());
        Bundle extras = loc.getExtras();
        if  (extras == null) {
            extras = new Bundle();
        }
        extras.putString("sessionName", sessionName);
        loc.setExtras(extras);
        mHandler.obtainMessage(MSG_UPDATE_LOC_REPORT, fixCounter, 0, loc).sendToTarget();
        fixCounter++;
    }

}

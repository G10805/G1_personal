/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2020-2021, 2023 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
package com.qualcomm.location.utils;

import android.util.Log;
import android.location.Location;
import android.os.HandlerThread;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.content.Context;
import com.qualcomm.location.izat.IzatService.ISystemEventListener;
import com.qualcomm.location.osagent.OsAgent;
import com.qualcomm.location.izat.flp.FlpServiceProvider;
import com.qti.flp.IFlpService;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.TimeUnit;
import com.qualcomm.location.policy.SessionPolicyManager;

import android.os.SystemClock;
import java.security.SecureRandom;
import java.time.Clock;
import java.util.Random;
import android.location.LocationResult;

public class IZatServiceContext {
    //Handler messages base
    public static final int MSG_FLP_BASE =                  0;
    public static final int MSG_NET_INITIATED_BASE =        100;
    public static final int MSG_LOCATION_SERVICE_BASE =     200;
    public static final int MSG_IZAT_PROVIDER_BASE =        300;
    public static final int MSG_NPPROXY_BASE =              400;
    public static final int MSG_OSAGENT_BASE =              500;
    public static final int MSG_RILINFO_MONITOR_BASE =      600;
    public static final int MSG_GEOCODER_PROXY_BASE =       700;
    public static final int MSG_ALTITUDE_RECEIVER =         800;
    public static final int MSG_POLICY_MANAGER_BASE =       900;

    private static final String TAG = "IZatServiceContext";
    private static final int FEATURE_BIT_PRECISE_LOCATION_IS_SUPPORTED = 0x100;
    private static final int CORE_POOL_SIZE = 1;
    private static final int MAX_POOL_SIZE = 10;
    private static final int KEEP_ALIVE_TIME = 60;
    private static IZatServiceContext sInstance = null;
    private final OsAgent mOsAgent;
    private final Context mContext;
    //** Static flag to track whether the ondevice logging has been initialized or not
    public static boolean sIsDiagJNILoaded = false;
    private int mFlpFeatureMask = -1;

    private final ThreadPoolExecutor mThreadPool = new ThreadPoolExecutor(CORE_POOL_SIZE,
            MAX_POOL_SIZE, KEEP_ALIVE_TIME, TimeUnit.SECONDS, new SynchronousQueue<Runnable>());
    private final HandlerThread mHandlerThd =
            new HandlerThread(IZatServiceContext.class.getSimpleName());
    private final Looper mLooper;
    private Clock mClock = SystemClock.elapsedRealtimeClock();
    static final long OFFSET_UPDATE_INTERVAL_MS = 60 * 60 * 1000;
    // the percentage that we change the random offset at every interval. 0.0 indicates the random
    // offset doesn't change. 1.0 indicates the random offset is completely replaced every interval
    private static final double CHANGE_PER_INTERVAL = 0.03;  // 3% change
    // weights used to move the random offset. the goal is to iterate on the previous offset, but
    // keep the resulting standard deviation the same. the variance of two gaussian distributions
    // summed together is equal to the sum of the variance of each distribution. so some quick
    // algebra results in the following sqrt calculation to weight in a new offset while keeping the
    // final standard deviation unchanged.
    private static final double NEW_WEIGHT = CHANGE_PER_INTERVAL;
    private static final double OLD_WEIGHT = Math.sqrt(1 - NEW_WEIGHT * NEW_WEIGHT);
    // this number actually varies because the earth is not round, but 111,000 meters is considered
    // generally acceptable
    private static final int APPROXIMATE_METERS_PER_DEGREE_AT_EQUATOR = 111_000;
    // we pick a value 1 meter away from 90.0 degrees in order to keep cosine(MAX_LATITUDE) to a
    // non-zero value, so that we avoid divide by zero errors
    private static final double MAX_LATITUDE =
            90.0 - (1.0 / APPROXIMATE_METERS_PER_DEGREE_AT_EQUATOR);

    private final float mAccuracyM = 2000;
    private final Random mRandom = new SecureRandom();

    private double mLatitudeOffsetM;
    private double mLongitudeOffsetM;
    private long mNextUpdateRealtimeMs;

    private Location mCachedFineLocation;
    private Location mCachedCoarseLocation;

    private LocationResult mCachedFineLocationResult;
    private LocationResult mCachedCoarseLocationResult;


    static {
        if (Log.isLoggable(TAG, Log.VERBOSE)) {
            try {
                System.loadLibrary("locsdk_diag_jni");
                sIsDiagJNILoaded = true;
            } catch (Throwable e) {
                Log.e(TAG, "Failed to loadLibrary liblocsdk_diag_jni: " + e);
            }
        }
    }
    private IZatServiceContext(Context ctx) {
        mContext = ctx;
        mHandlerThd.start();
        mLooper = mHandlerThd.getLooper();
        mOsAgent = OsAgent.GetInstance(mContext, mLooper);
    }

    public Looper getLooper() {
        return mLooper;
    }

    public Context getContext() {
        return mContext;
    }

    public boolean isPreciseLocationSupported() {
        if (mFlpFeatureMask == -1) {
            IFlpService flpBinder = FlpServiceProvider.getInstance(mContext).getFlpBinder();
            try {
                mFlpFeatureMask = flpBinder.getAllSupportedFeatures();
            } catch (Exception e) {
                Log.e(TAG, "Failed to call flpBinder.getAllSupportedFeatures()" + e);
            }
        }
        return (mFlpFeatureMask & FEATURE_BIT_PRECISE_LOCATION_IS_SUPPORTED) >= 0;
    }

    public synchronized static IZatServiceContext getInstance(Context ctx) {
        if (null == sInstance) {
            sInstance = new IZatServiceContext(ctx);
        }
        return sInstance;
    }

    public void registerSystemEventListener(int sysEventMsgId, ISystemEventListener listener) {
        mOsAgent.registerObserver(sysEventMsgId, listener);
    }

    public void diagLogBatchedFixes(Location[] locations) {
        if (sIsDiagJNILoaded) {
            native_diag_log_flp_batch(locations);
        }
    }

    public void executeRunnable(Runnable r) {
        mThreadPool.execute(r);
    }

    // convert to coarse location using two technique, random offsets and snap-to-grid.
    public Location createCoarse(Location fineLoc) {
        synchronized (this) {
            if (fineLoc == mCachedFineLocation || fineLoc == mCachedCoarseLocation) {
                fineLoc = mCachedCoarseLocation;
            }
        }

        // update the offsets in use
        updateOffsets();

        Location coarseLoc = new Location(fineLoc);
        // clear any fields that could leak more detailed location information
        coarseLoc.removeBearing();
        coarseLoc.removeSpeed();
        coarseLoc.removeAltitude();
        coarseLoc.setExtras(null);

        double latitude = wrapLatitude(coarseLoc.getLatitude());
        double longitude = wrapLongitude(coarseLoc.getLongitude());

        // add offsets - update longitude first using the non-offset latitude
        longitude += wrapLongitude(metersToDegreesLongitude(mLongitudeOffsetM, latitude));
        latitude += wrapLatitude(metersToDegreesLatitude(mLatitudeOffsetM));

        // quantize location by snapping to a grid. this is the primary means of obfuscation. it
        // gives nice consistent results and is very effective at hiding the true location (as long
        // as you are not sitting on a grid boundary, which the random offsets mitigate).
        //
        // note that we quantize the latitude first, since the longitude quantization depends on the
        // latitude value and so leaks information about the latitude
        double latGranularity = metersToDegreesLatitude(mAccuracyM);
        latitude = wrapLatitude(Math.round(latitude / latGranularity) * latGranularity);
        double lonGranularity = metersToDegreesLongitude(mAccuracyM, latitude);
        longitude = wrapLongitude(Math.round(longitude / lonGranularity) * lonGranularity);

        coarseLoc.setLatitude(latitude);
        coarseLoc.setLongitude(longitude);
        coarseLoc.setAccuracy(Math.max(mAccuracyM, coarseLoc.getAccuracy()));

        synchronized (this) {
            mCachedFineLocation = fineLoc;
            mCachedCoarseLocation = coarseLoc;
        }
        return coarseLoc;
    }

    private static double wrapLatitude(double latitude) {
        if (latitude > MAX_LATITUDE) {
            latitude = MAX_LATITUDE;
        }
        if (latitude < -MAX_LATITUDE) {
            latitude = -MAX_LATITUDE;
        }
        return latitude;
    }

    private static double wrapLongitude(double longitude) {
        longitude %= 360.0;  // wraps into range (-360.0, +360.0)
        if (longitude >= 180.0) {
            longitude -= 360.0;
        }
        if (longitude < -180.0) {
            longitude += 360.0;
        }
        return longitude;
    }

    private synchronized void updateOffsets() {
        long now = mClock.millis();
        if (now < mNextUpdateRealtimeMs) {
            return;
        }

        mLatitudeOffsetM = (OLD_WEIGHT * mLatitudeOffsetM) + (NEW_WEIGHT * nextRandomOffset());
        mLongitudeOffsetM = (OLD_WEIGHT * mLongitudeOffsetM) + (NEW_WEIGHT * nextRandomOffset());
        mNextUpdateRealtimeMs = now + OFFSET_UPDATE_INTERVAL_MS;
    }

    private double nextRandomOffset() {
        return (mAccuracyM / 4.0) * mRandom.nextGaussian();
    }

    private static double metersToDegreesLatitude(double distance) {
        return distance / APPROXIMATE_METERS_PER_DEGREE_AT_EQUATOR;
    }

    // requires latitude since longitudinal distances change with distance from equator.
    private static double metersToDegreesLongitude(double distance, double lat) {
        return distance / APPROXIMATE_METERS_PER_DEGREE_AT_EQUATOR / Math.cos(Math.toRadians(lat));
    }

    // native diag interface API
    public native void native_diag_log_flp_batch(Location[] locations);
}

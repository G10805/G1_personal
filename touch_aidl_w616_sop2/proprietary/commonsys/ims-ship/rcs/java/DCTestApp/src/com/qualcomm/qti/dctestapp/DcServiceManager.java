/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.dctestapp;

import android.content.Context;
import android.util.Log;

import java.util.concurrent.Executor;

import vendor.qti.imsdatachannel.client.ImsDataChannelConnection;
import vendor.qti.imsdatachannel.client.ImsDataChannelServiceManager;
import vendor.qti.imsdatachannel.client.ImsDataChannelServiceManagerImpl;

public class DcServiceManager {
    final static String LOG_TAG = "DCTestApp:DcServiceManager";

    private static ImsDataChannelServiceManager instance = null;

    synchronized public static ImsDataChannelServiceManager getInstance() {
        if (instance == null) {
            Log.e(LOG_TAG, "Need to initialize ImsDataChannelServiceManager with context and executor");
        }
        return instance;
    }

    synchronized public static ImsDataChannelServiceManager getInstance(Context context, Executor executor) {
        if (instance == null) {
            instance = new ImsDataChannelServiceManagerImpl();
            instance.initialize(context, executor);
            Log.d(LOG_TAG,"ImsDataChannelServiceManagerImpl initialized");
        }
        return instance;
    }
}

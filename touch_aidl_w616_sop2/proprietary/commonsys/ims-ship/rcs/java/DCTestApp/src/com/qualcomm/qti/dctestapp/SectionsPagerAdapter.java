/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.dctestapp;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.IntentFilter;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.util.Log;
import android.widget.Toast;

import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentPagerAdapter;

import com.google.android.material.snackbar.Snackbar;

import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.List;

import vendor.qti.imsdatachannel.client.ImsDataChannelServiceConnectionCallback;
import vendor.qti.imsdatachannel.client.ImsDataChannelServiceManager;

/**
 * A [FragmentPagerAdapter] that returns a fragment corresponding to
 * one of the sections/tabs/pages.
 */
public class SectionsPagerAdapter extends FragmentPagerAdapter implements ImsDataChannelServiceConnectionCallback {

    final String LOG_TAG = "DCTestApp:SectionsPagerAdapter:";

    private final Context mContext;

    Executor mExecutor = new ScheduledThreadPoolExecutor(1);

    private int numTabs = 1;
    public final static int SUB_PRIMARY = 1;
    public final static  int SUB_SECOND = 2;
    public final static  int SUB_INVALID = -1;
    public final static  int MAX_SUB_SUPPORTED = 2;
    public SubscriptionFragment[] mSubsTabs;

    private CallReceiver mCallReceiver = null;
    private ImsDataChannelServiceManager mDcManager = null;

    public SectionsPagerAdapter(Context context, FragmentManager fm) {
        super(fm);
        mContext = context;

        // Get subs
        SubscriptionManager sm = (SubscriptionManager)
          mContext.getSystemService(Context.TELEPHONY_SUBSCRIPTION_SERVICE);
        @SuppressLint("MissingPermission") List<SubscriptionInfo> subs =
          sm.getActiveSubscriptionInfoList();
        Log.i(LOG_TAG, "numTabs = " + numTabs);
        numTabs = subs.size();

        mCallReceiver = new CallReceiver(mContext);
        mCallReceiver.registerForCallIntent();
        mDcManager = DcServiceManager.getInstance(mContext, mExecutor);
        Log.d(LOG_TAG,"ImsDataChannelServiceManager got instance");

        // Create tabs
        if (numTabs > MAX_SUB_SUPPORTED) {
            Log.e(LOG_TAG, "Unsupported number of subscriptions");
        } else if (numTabs > 0) {
            mSubsTabs = new SubscriptionFragment[numTabs];
            for (int i = 0; i < numTabs && i <= SUB_SECOND; i++) {
                mSubsTabs[i] = new SubscriptionFragment(subs.get(i));
                mCallReceiver.registerListener(mSubsTabs[i]);
            }
        } else {
            Log.e(LOG_TAG, "Executing in Emulator Scenario");
            numTabs = 2;
            mSubsTabs = new SubscriptionFragment[numTabs];
            mSubsTabs[0] = new SubscriptionFragment(SUB_PRIMARY, 0, "iccid1");
            mSubsTabs[1] = new SubscriptionFragment(SUB_SECOND, 1, "iccid2");
            mCallReceiver.registerListener(mSubsTabs[0]);
            mCallReceiver.registerListener(mSubsTabs[1]);
        }

        mDcManager.connectImsDataChannelService(this);

    }

    @Override
    public Fragment getItem(int position) {
        if (position < 0 || position > numTabs) {
            Log.i(LOG_TAG,":getItem returning NULL for= "+ position);
            return  null;
        }
        return mSubsTabs[position];
    }

    @Nullable
    @Override
    public CharSequence getPageTitle(int position) {
        return "Slot-" + mSubsTabs[position].getSlotId();
    }

    @Override
    public int getCount() {
        return numTabs;
    }

    /*------ Overrides for ImsDataChannelServiceConnectionCallback ------*/

    @Override
    public void onServiceConnected() {
        for (SubscriptionFragment subTab : mSubsTabs) {
            subTab.onServiceConnected();
        }
        Toast.makeText(mContext, "DataChannelServiceCb::onServiceConnected",
                       Toast.LENGTH_SHORT).show();
    }
    @Override
    public void onServiceDisconnected() {
        for (SubscriptionFragment subTab : mSubsTabs) {
            subTab.onServiceConnected();
        }
        Toast.makeText(mContext, "DataChannelServiceCb::onServiceDisconnected",
                       Toast.LENGTH_SHORT).show();
    }
}

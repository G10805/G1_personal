/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.dctestapp;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.view.View;
import android.widget.Toast;
import android.util.Log;

import androidx.viewpager.widget.ViewPager;
import androidx.appcompat.app.AppCompatActivity;

import com.google.android.material.snackbar.Snackbar;
import com.google.android.material.tabs.TabLayout;

import com.qualcomm.qti.dctestapp.SectionsPagerAdapter;

import vendor.qti.imsdatachannel.client.ImsDataChannelServiceManager;

public class MainActivity extends AppCompatActivity {

    final public static String LOG_TAG = "DCTestApp:MainActivity";
    // public static Context mCtx = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        SectionsPagerAdapter sectionsPagerAdapter = new SectionsPagerAdapter(this, getSupportFragmentManager());
        Log.i(LOG_TAG, "App Start");

        // mCtx = getApplicationContext();
        // sectionsPagerAdapter = new SectionsPagerAdapter(mCtx, getSupportFragmentManager());

        ViewPager viewPager = findViewById(R.id.view_pager);
        viewPager.setAdapter(sectionsPagerAdapter);
        TabLayout tabs = findViewById(R.id.tabs);
        tabs.setupWithViewPager(viewPager);
        tabs.addOnTabSelectedListener(new TabLayout.OnTabSelectedListener() {
            @Override
            public void onTabSelected(TabLayout.Tab tab) {
                viewPager.setCurrentItem(tab.getPosition());
            }

            @Override
            public void onTabUnselected(TabLayout.Tab tab) {

            }

            @Override
            public void onTabReselected(TabLayout.Tab tab) {

            }
        });
    }

    @Override
    public void onDestroy() {
        Log.d(LOG_TAG, "onDestroy()");
        ImsDataChannelServiceManager mDcManager = DcServiceManager.getInstance();
        if (mDcManager != null) {
            Log.d(LOG_TAG, "onDestroy() calling disconnectImsDataChannelService()");
            mDcManager.disconnectImsDataChannelService();
        }
    }

}

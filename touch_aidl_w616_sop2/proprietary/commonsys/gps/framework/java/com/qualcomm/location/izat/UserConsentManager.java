/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright  (c) 2022-2023 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

  Not a Contribution
=============================================================================*/
/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.qualcomm.location.izat;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.UserManager;
import android.os.Binder;
import android.app.ActivityManager;
import android.util.Log;
import com.qualcomm.location.izat.IzatService.ISsrNotifier;
import com.qualcomm.location.izat.IzatService.SsrHandler;
import com.qualcomm.location.osagent.OsAgent;
import com.qualcomm.location.utils.IZatServiceContext;

import java.util.ArrayList;
import java.util.List;

/*
 Purpose: This module would be responsible for receiving, aggregating, and restoring
   the user consent flag received from different modules, such as GtpServiceProvider,
   QesdkTrackingServiceProvider, and GnssConfigService.
 */
public class UserConsentManager implements ISsrNotifier, IzatService.ISystemEventListener {

    private static final String TAG = "UserConsentManager";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

    private static UserConsentManager sInstance = null;

    private final Context mContext;
    private IZatServiceContext mIZatServiceCtx;
    private final OsAgent mOsAgent;

    private boolean mIsIzatNetworkProviderLoaded = false;

    // Class capturing the details of a consent provider
    private static class PackageConsentProvider {
        String mPackageName;
        int mUserId;
        int mCurrentUser;

        public PackageConsentProvider(String packageName, int userId) {
            mPackageName = packageName;
            mUserId = userId;
            mCurrentUser = -1;
        }
        public PackageConsentProvider(String packageName, int userId, int currentUser) {
            mPackageName = packageName;
            mUserId = userId;
            mCurrentUser = currentUser;
        }

        @Override
        public boolean equals(Object obj) {
            if (obj == this) return true;
            if (!(obj instanceof PackageConsentProvider)) return false;
            PackageConsentProvider provider = (PackageConsentProvider)obj;
            return (provider != null && mPackageName != null &&
                    provider.mPackageName.equals(mPackageName) &&
                    (provider.mUserId == mUserId || provider.mCurrentUser == mCurrentUser));
        }
    }
    private final List<PackageConsentProvider> mPackageConsentProviders =
            new ArrayList<PackageConsentProvider>();

    // User Consent Listener Interface
    public interface UserConsentListener {
        // User consent is updated for current user, listener can query via getUserConsent
        public void onUserConsentUpdated();
    }
    private final List<UserConsentListener> mUserConsentListeners =
            new ArrayList<UserConsentListener>();

    private void logv(String msg) {
        if (VERBOSE) {
            Log.v(TAG, msg);
        }
    }
    private void logi(String msg) {
        Log.i(TAG, msg);
    }
    private void loge(String msg) {
        Log.e(TAG, msg);
    }

    private UserConsentManager(Context ctx) {

        mContext = ctx;
        mIZatServiceCtx = IZatServiceContext.getInstance(mContext);
        mOsAgent = OsAgent.GetInstance(ctx, mIZatServiceCtx.getLooper());

        if (UserManager.supportsMultipleUsers()) {
            mIZatServiceCtx.registerSystemEventListener(MSG_PKG_REMOVED, this);
            mIZatServiceCtx.registerSystemEventListener(MSG_USER_SWITCH_ACTION_UPDATE, this);
        }
    }
    public static UserConsentManager getInstance(Context ctx) {
        if (sInstance == null) {
            sInstance = new UserConsentManager(ctx);
        }
        return sInstance;
    }

    @Override
    public void notify(int msgId, Object... args) {

        if (msgId == MSG_PKG_REMOVED) {
            Intent intent = (Intent)args[0];
            Uri uri = intent.getData();
            String packageName = uri != null ? uri.getSchemeSpecificPart() : null;
            if (packageName != null) {
                boolean removed = mPackageConsentProviders.removeIf(
                    p -> p.mPackageName.equals(packageName));
                if (removed) {
                    handleConsentUpdate();
                }
            } else {
                loge("Got null package name on PKG_REMOVED intent");
            }
        }

        if (msgId == MSG_USER_SWITCH_ACTION_UPDATE) {
            mOsAgent.sendConsolidatedUserConsent(getPackageUserConsent() || getAospUserConsent());
        }
    }

    @Override
    public void bootupAndSsrNotifier(String savedState) {
        logi("bootupAndSsrNotifier: " + savedState);
        restoreState(savedState);
        mOsAgent.sendConsolidatedUserConsent(getPackageUserConsent() || getAospUserConsent());
    }

    public void setIsIzatNetworkProviderLoaded(boolean isIzatNetworkProviderLoaded) {
        mIsIzatNetworkProviderLoaded = isIzatNetworkProviderLoaded;
    }

    // User Consent API for controlling AOSP sessions.
    public void setAospUserConsent(boolean consent) {

        if (!mIsIzatNetworkProviderLoaded) {
            loge("Ignoring setAospUserConsent since IzatProvider is not NetworkLocationProvider");
            return;
        }

        boolean existingConsent = getAospUserConsent();
        int currentUser = ActivityManager.getCurrentUser();
        int callingUid = Binder.getCallingUid();

        logi("setAospUserConsent [" + callingUid + "][" + currentUser + "] = " +
                existingConsent + "->" + consent);

        if (consent != existingConsent) {

            PackageConsentProvider provider = new PackageConsentProvider(
                "AOSP", callingUid, currentUser);
            boolean providerInList = mPackageConsentProviders.contains(provider);
            // Add/remove provider from list based on consent
            if (!providerInList && consent) {
                mPackageConsentProviders.add(provider);
            } else if (providerInList && !consent) {
                mPackageConsentProviders.remove(provider);
            }

            handleConsentUpdate();
        }
    }

    // Return AOSP consent for current user
    public boolean getAospUserConsent() {

        return mPackageConsentProviders.contains(new PackageConsentProvider(
            "AOSP", Binder.getCallingUid(), ActivityManager.getCurrentUser()));
    }

    // User Consent API for controlling individual application sessions (per application)
    public void setPackageUserConsent(boolean consent, String packageName, int userId) {

        int currentUser = ActivityManager.getCurrentUser();

        logi("setPackageUserConsent [" + packageName + "][" + userId + "][" + currentUser +
                "] = " + consent);

        if (packageName == null || userId == 0 || packageName.trim().length() == 0) {
            loge("Invalid packageName " + packageName + " or user id " + userId);
            return;
        }

        PackageConsentProvider provider = new PackageConsentProvider(
            packageName, userId, currentUser);
        boolean providerInList = mPackageConsentProviders.contains(provider);

        // Add/remove provider from list based on consent
        if (!providerInList && consent) {
            mPackageConsentProviders.add(provider);
            handleConsentUpdate();
        } else if (providerInList && !consent) {
            mPackageConsentProviders.remove(provider);
            handleConsentUpdate();
        }
    }

    // Return consolidated package user consent.
    public boolean getPackageUserConsent() {

        int currentUser = ActivityManager.getCurrentUser();
        for (PackageConsentProvider provider: mPackageConsentProviders) {
            if (!provider.mPackageName.equals("AOSP") && provider.mCurrentUser == currentUser) {
                return true;
            }
        }
        return false;
    }

    // Return per package user consent.
    public boolean getPackageUserConsent(String packageName, int userId) {

        return mPackageConsentProviders.contains(new PackageConsentProvider(packageName, userId));
    }

    // Register a listener for any user consent update
    public void registerUserConsentListener(UserConsentListener listener) {

        if (!mUserConsentListeners.contains(listener)) {
            mUserConsentListeners.add(listener);
        }
    }

    private void handleConsentUpdate() {
        persistState();
        notifyUserConsentListeners();
        mOsAgent.sendConsolidatedUserConsent(getPackageUserConsent() || getAospUserConsent());
    }

    private void notifyUserConsentListeners() {

        for (UserConsentListener listener: mUserConsentListeners) {
            listener.onUserConsentUpdated();
        }
    }

    private void persistState() {

        StringBuilder sb = new StringBuilder();
        for (PackageConsentProvider provider: mPackageConsentProviders) {
            sb.append(provider.mPackageName);
            sb.append(":");
            sb.append(provider.mUserId);
            sb.append(":");
            sb.append(provider.mCurrentUser);
            sb.append(" ");
        }

        String stateStr = sb.toString().trim();
        logv("Saving state: [" + stateStr + "]");
        SsrHandler.get().registerDataForSSREvents(
            mContext, UserConsentManager.class.getName(), stateStr);
    }

    private void restoreState(String persistedState) {

        if (mPackageConsentProviders.size() > 0) {
            loge("restoreState: providers list is not empty, can't restore.");
            return;
        }

        if (persistedState != null && persistedState.trim().length() > 0) {
            String[] providers = persistedState.split(" ");
            for (String provider: providers) {
                String[] pNameAndUid = provider.split(":");
                String packageName = pNameAndUid[0];
                String userId = pNameAndUid[1];
                String currentUser = pNameAndUid[2];
                mPackageConsentProviders.add(new PackageConsentProvider(
                    packageName, Integer.parseInt(userId), Integer.parseInt(currentUser)));
            }
        }
    }
}

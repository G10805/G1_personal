/**********************************************************************
 * Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsrcs;

import android.telephony.ims.aidl.IImsRegistration;
import android.telephony.ims.aidl.IImsServiceController;
import android.telephony.ims.aidl.IImsServiceControllerListener;
import android.telephony.ims.aidl.ISipTransport;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.feature.RcsFeature;
import android.telephony.ims.stub.ImsConfigImplBase;
import android.telephony.ims.stub.ImsFeatureConfiguration;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.telephony.ims.stub.SipTransportImplBase;
import android.util.Log;
import android.util.SparseArray;

import com.android.ims.internal.IImsFeatureStatusCallback;
import com.android.internal.annotations.VisibleForTesting;

import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.HashMap;
import java.util.Map;


import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.telephony.ims.ImsService;
import android.telephony.ims.feature.RcsFeature;
import android.telephony.ims.stub.ImsConfigImplBase;

//import androidx.annotation.VisibleForTesting;

import java.util.ArrayList;
import java.util.List;
//import vendor.qti.ims.SparseArray;

import vendor.qti.imsrcs.config.ImsConfigServiceImpl;
import vendor.qti.imsrcs.siptransport.ImsRcsRegistrationImpl;
import vendor.qti.imsrcs.siptransport.ImsRcsSipTransportImpl;
import vendor.qti.imsrcs.uce.ImsRcsFeatureImpl;

public class ImsRcsService extends android.telephony.ims.ImsService {

    public static final String LOG_TAG = "ImsRcsService";
    private static final int INVALID_SLOT_ID = -1;
    private static final int UNINITIALIZED_VALUE = -1;

    private static ImsConfigServiceImpl mImsConfigSvc[];
    private static ImsRcsServiceMgr mImsRcsServiceMgr;
    private static ImsRcsFeatureImpl mImsRcsFeatureImpl[];
    private static ImsRcsSipTransportImpl mImsRcsSipTransportImpl[];
    private static ImsRcsRegistrationImpl mImsRcsRegistrationImpl[];

    private int mNumPhonesCache = UNINITIALIZED_VALUE;
    //private ImsSubController mSubController;

    Executor mServiceExecutor = new ScheduledThreadPoolExecutor(1);

    public static int MAX_SLOTS = -1;
    //RcsFeatureImpl[] mRcsFeatures;
   // private ImsConfigServiceImpl[] mConfigServices;

    private static final String SINGLE_REG_PROPERTY_STRING = "persist.vendor.rcs.singlereg.feature";
    public static final int RCS_SERVICE_DISABLED = 0;
    public static final int RCS_SERVICE_ENABLED = 1;
    private int mRcsSingleRegEnable = RCS_SERVICE_DISABLED;

    private int getNumSlots() {
        if (mNumPhonesCache == UNINITIALIZED_VALUE) {
            mNumPhonesCache = ((TelephonyManager) getSystemService(
                Context.TELEPHONY_SERVICE)).getSupportedModemCount();
        }
        Log.i (LOG_TAG, "ImsRcsService getNumSlots"+
               String.valueOf(mNumPhonesCache));
        return mNumPhonesCache;
    }

    private void setup() {
        mRcsSingleRegEnable = SystemProperties.getInt(SINGLE_REG_PROPERTY_STRING, RCS_SERVICE_DISABLED);
        Log.i(LOG_TAG, "Single Reg Property: " + mRcsSingleRegEnable);

        final int numSlots = getNumSlots();
        MAX_SLOTS = numSlots;
        Log.i (LOG_TAG, "ImsRcsService "+ String.valueOf(MAX_SLOTS));
        mImsRcsServiceMgr = ImsRcsServiceMgr.getInstance();

        if(mRcsSingleRegEnable==RCS_SERVICE_DISABLED) {
            Log.d(LOG_TAG, "SingleImsReg not enabled, so return without initializing");
            return;
        }

        mImsRcsServiceMgr.Initialize();
        mImsConfigSvc = new ImsConfigServiceImpl[numSlots];
        for (int i = 0; i < numSlots; i++) {
            mImsConfigSvc[i] = new ImsConfigServiceImpl(i,this);
        }

        mImsRcsFeatureImpl = new ImsRcsFeatureImpl[MAX_SLOTS];
        mImsRcsRegistrationImpl = new ImsRcsRegistrationImpl[MAX_SLOTS];
        mImsRcsSipTransportImpl = new ImsRcsSipTransportImpl[MAX_SLOTS];
    }

    private RcsFeature getRcsFeature(int slotId) {
        if(mRcsSingleRegEnable == RCS_SERVICE_DISABLED) {
            Log.d(LOG_TAG, "Single Reg Property disabled so return null");
            return null;
        } else if (slotId > INVALID_SLOT_ID && slotId < getNumSlots()) {
            if(mImsRcsFeatureImpl[slotId] == null) {
                 mImsRcsFeatureImpl[slotId] = new ImsRcsFeatureImpl(slotId, mImsConfigSvc[slotId]);
             }
             Log.i(LOG_TAG, "getRcsFeature :: slotId " + slotId);
             return mImsRcsFeatureImpl[slotId];
        }
        else{
           Log.e(LOG_TAG, "getRcsFeature :: Invalid slotId " + slotId);
        }
        return null;
    }

    private ImsConfigImplBase getConfigImplInternal(int slotId) {
        if(mRcsSingleRegEnable == RCS_SERVICE_DISABLED ) {
            Log.e(LOG_TAG, "getConfigImplInternal :: SingleImsReg disabled");
            return null;
        } else if (slotId > INVALID_SLOT_ID && slotId < getNumSlots()) {
                return mImsConfigSvc[slotId];
        }
        Log.e(LOG_TAG, "getConfigImplInternal :: invalid slotId=" + slotId);
        return null;
    }

    private ImsRegistrationImplBase getRegistrationImplInternal(int slotId) {
        if(mRcsSingleRegEnable == RCS_SERVICE_DISABLED ) {
            Log.e(LOG_TAG, "getRegistration :: SingleImsReg disabled");
            return null;
        } else if (slotId > INVALID_SLOT_ID && slotId < getNumSlots()) {
            if(mImsRcsRegistrationImpl[slotId] == null) {
                mImsRcsRegistrationImpl[slotId] = new ImsRcsRegistrationImpl(slotId);
            }
            return mImsRcsRegistrationImpl[slotId];
        }
        Log.e(LOG_TAG, "getRegistrationImplInternal :: invalid slotId=" + slotId);
        return null;
    }

    static public ImsConfigServiceImpl[] getImsConfigService() {
        return mImsConfigSvc;
    }

    static public ImsRcsRegistrationImpl getImsRcsRegistrationImpl(int slotId){
        return mImsRcsRegistrationImpl[slotId];
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i (LOG_TAG, "ImsRcsService created!======");
        setup();
    }

    @Override
    public Executor getExecutor() {
        Log.i (LOG_TAG, "getExecutor======");
        return mServiceExecutor;
    }

    public @ImsServiceCapability long getImsServiceCapabilities() {
        if( mRcsSingleRegEnable==RCS_SERVICE_ENABLED ) {
            Log.i (LOG_TAG, "ImsRcsService getImsServiceCapabilities");
            return ImsService.CAPABILITY_SIP_DELEGATE_CREATION;
        } else {
            Log.i (LOG_TAG, "ImsRcsService getImsServiceCapabilities returning null");
            return 0L;
        }
    }

    /**
     * When called, provide the {@link ImsFeatureConfiguration} that this {@link ImsService}
     * currently supports. This will trigger the framework to set up the {@link ImsFeature}s that
     * correspond to the {@link ImsFeature}s configured here.
     *
     * Use {@link #onUpdateSupportedImsFeatures(ImsFeatureConfiguration)} to change the supported
     * {@link ImsFeature}s.
     *
     * @return an {@link ImsFeatureConfiguration} containing Features this ImsService supports.
     */
    @Override
    public ImsFeatureConfiguration querySupportedImsFeatures() {
        // These features are also declared in the AndroidManifest file as supported
        ImsFeatureConfiguration.Builder features = new ImsFeatureConfiguration.Builder();
        if(mRcsSingleRegEnable==RCS_SERVICE_ENABLED) {
            Log.i (LOG_TAG, "ImsRcsService  querySupportedImsFeatures FEATURE_RCS");
            for (int i = 0; i < getNumSlots(); i++) {
                features.addFeature(i, ImsFeature.FEATURE_RCS);
            }
        } else {
            Log.i (LOG_TAG, "singleImsReg prop is disabled so not adding features");
        }
        return features.build();
    }


    /**
     * The ImsService has been bound and is ready for ImsFeature creation based on the Features that
     * the ImsService has registered for with the framework, either in the manifest or via
     * {@link #querySupportedImsFeatures()}.
     *
     * The ImsService should use this signal instead of onCreate/onBind or similar to perform
     * feature initialization because the framework may bind to this service multiple times to
     * query the ImsService's {@link ImsFeatureConfiguration} via
     * {@link #querySupportedImsFeatures()}before creating features.
     */
    @Override
    public void readyForFeatureCreation() {
        Log.i(LOG_TAG, "readyForFeatureCreation :: No-op");
        // NOTE: This method is a no-op. IMS Service initializaiton will have to move
        // into this method if/when AOSP starts binding to IMS Service multiple times,
    }


    /**
     * When called, the framework is requesting that a new {@link RcsFeature} is created for the
     * specified slot.
     *
     * @param slotId The slot ID that the RcsFeature is being created for.
     * @return The newly created {@link RcsFeature} associated with the slot or null if the
     * feature is not supported.
     */
    @Override
    public RcsFeature createRcsFeature(int slotId) {
        Log.d(LOG_TAG, "createRcsFeature :: slotId=" + slotId);
        return getRcsFeature(slotId);
    }

    /**
     * When called, the framework is requesting that a new {@link RcsFeature} is created for the
     * specified subscription.
     *
     * @param subscriptionId The subscription ID that the RCS Feature is being created for.
     * @return The newly created {@link RcsFeature} associated with the subscription or null if the
     * feature is not supported.
     */
    @Override
    public RcsFeature createRcsFeatureForSubscription(int slotId, int subsId) {
        Log.d(LOG_TAG, "createRcsFeatureForSubscription :: slotId=" + slotId + " subsId=" + subsId);
        return getRcsFeature(slotId);
    }

    /**
     * Return the {@link ImsConfigImplBase} implementation associated with the provided
     * subscription. This will be used by the platform to get/set specific IMS related
     * configurations.
     *
     * @param subscriptionId The subscription ID that the IMS configuration is associated with.
     * @return ImsConfig implementation that is associated with the specified subscription.
     */
    @Override
    public ImsConfigImplBase getConfigForSubscription(int slotId, int subsId) {
        Log.d(LOG_TAG, "getConfigForSubscription :: slotId=" + slotId + " subsId=" + subsId);
        return getConfigImplInternal(slotId);
    }

    /**
     * Return the {@link ImsConfigImplBase} implementation associated with the provided slot. This
     * will be used by the platform to get/set specific IMS RCS Feature related configurations.
     *
     * @param slotId The slot that the IMS configuration is associated with.
     * @return ImsConfig implementation that is associated with the specified slot.
     */
    @Override
    public ImsConfigImplBase getConfig(int slotId) {
        Log.d(LOG_TAG, "getConfig :: slotId=" + slotId);
        return getConfigImplInternal(slotId);
    }

    /**
     * Return the {@link SipTransportImplBase} implementation associated with the provided slot.
     *
     * @param slotId The slot that is associated with the IMS Registration.
     * @return the ImsRegistration implementation associated with the slot.
     */

    @Override
    public SipTransportImplBase getSipTransport(int slotId) {
        Log.d(LOG_TAG, "getSipTransport :: slotId=" + slotId);
        if(mRcsSingleRegEnable == RCS_SERVICE_DISABLED) {
            Log.e(LOG_TAG, "getSipTransport :: SingleImsReg disabled");
            return null;
        } else if (slotId > INVALID_SLOT_ID && slotId < getNumSlots()) {
           if(mImsRcsSipTransportImpl[slotId] == null) {
                mImsRcsSipTransportImpl[slotId] = new ImsRcsSipTransportImpl(slotId, mImsConfigSvc[slotId]);
           }
           return mImsRcsSipTransportImpl[slotId];
        }
        Log.e(LOG_TAG, "getSipTransport :: Invalid slotId " + slotId);
        return null;
    }

    /**
     * Return the {@link ImsRegistrationImplBase} implementation associated with the provided
     * subscription.
     *
     * @param subscriptionId The subscription ID that is associated with the IMS Registration.
     * @return the ImsRegistration implementation associated with the subscription.
     */
    @Override
    public ImsRegistrationImplBase getRegistrationForSubscription(int slotId, int subsId) {
        Log.d(LOG_TAG, "getRegistrationForSubscription :: slotId=" + slotId + " subsId=" + subsId);
        return getRegistrationImplInternal(slotId);
    }
    /**
     * Return the {@link ImsRegistrationImplBase} implementation associated with the provided slot.
     *
     * @param slotId The slot that is associated with the IMS Registration.
     * @return the ImsRegistration implementation associated with the slot.
     */

    @Override
    public ImsRegistrationImplBase getRegistration(int slotId) {
        Log.d(LOG_TAG, "getRegistration :: slotId=" + slotId);
        return getRegistrationImplInternal(slotId);
    }

    @Override
    public void onDestroy() {
        Log.i(LOG_TAG, "Ims Rcs Service Destroyed Successfully...");
        if (mImsRcsServiceMgr != null) {
            mImsRcsServiceMgr.dispose();
            mImsRcsServiceMgr = null;
        }

        for (int i = 0; i < getNumSlots(); i++) {
            if(mImsConfigSvc != null && mImsConfigSvc[i] != null) {
                mImsConfigSvc[i].dispose();
                mImsConfigSvc[i] = null;
            }
        }
        mImsConfigSvc = null;
        super.onDestroy();
    }
}

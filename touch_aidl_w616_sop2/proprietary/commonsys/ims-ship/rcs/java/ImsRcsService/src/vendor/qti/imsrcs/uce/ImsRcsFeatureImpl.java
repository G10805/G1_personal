/**********************************************************************
 * Copyright (c) 2021,2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/


package vendor.qti.imsrcs.uce;
import android.telephony.ims.feature.CapabilityChangeRequest;
import android.telephony.ims.feature.RcsFeature;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.stub.CapabilityExchangeEventListener;
import android.telephony.ims.stub.RcsCapabilityExchangeImplBase;

import vendor.qti.imsrcs.config.ImsConfigServiceImpl;
import vendor.qti.imsrcs.uce.hidl.ImsPresCapEventListener;
import vendor.qti.imsrcs.uce.hidl.OptionsServiceWrapper;
import vendor.qti.imsrcs.uce.hidl.PresenceServiceWrapper;

import java.util.List;
import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;

import vendor.qti.imsrcs.ImsRcsServiceMgr;
import vendor.qti.imsrcs.ImsRcsService;

import android.util.Log;

import static android.telephony.ims.feature.RcsFeature.RcsImsCapabilities.CAPABILITY_TYPE_NONE;

public class ImsRcsFeatureImpl extends RcsFeature {
    int mSlotId;
    ImsRcsServiceMgr mManager;
    ImsRcsCapabilityExchangeImpl mCapExImpl = null;
    Executor mLocalExecutor = new ScheduledThreadPoolExecutor(1);
    RcsImsCapabilities mRcsCaps = new RcsImsCapabilities(RcsFeature.RcsImsCapabilities.CAPABILITY_TYPE_NONE);
    ImsConfigServiceImpl mConfigService;
    boolean isPresenceRegistered = false;
    boolean isPresenceConfigured = false;

    boolean isOptionsRegistered = false;
    boolean isOptionsConfigured = false;
    private static final int SERVICE_REGISTERED = 0;

    private String LOG_TAG = ImsRcsService.LOG_TAG + ":ImsRcsFeatureImpl";

    public ImsRcsFeatureImpl(int slotId, ImsConfigServiceImpl config) {
        super(new ScheduledThreadPoolExecutor(1));
        mSlotId = slotId;
        mManager = ImsRcsServiceMgr.getInstance();
        mConfigService = config;
        setFeatureState(ImsFeature.STATE_INITIALIZING);
        LOG_TAG = LOG_TAG+ "["+mSlotId+"]";
    }

    @Override
    public void changeEnabledCapabilities(CapabilityChangeRequest request,
                                          CapabilityCallbackProxy c) {
        Log.d(LOG_TAG, "changeEnabledCapabilities :: received");
        ImsConfigServiceImpl.UceCapUpdateCallback l = new ImsConfigServiceImpl.UceCapUpdateCallback() {
            @Override
            public void onUceConfigStatusReceived(RcsImsCapabilities caps) {
                Log.d(LOG_TAG, "onUceConfigStatusReceived :: received");
                onCapsReceived(caps);
            }
        };
        List<CapabilityChangeRequest.CapabilityPair> caplist = request.getCapabilitiesToEnable();
        RcsImsCapabilities capsFromFw = new RcsImsCapabilities(RcsFeature.RcsImsCapabilities.CAPABILITY_TYPE_NONE);
        for(CapabilityChangeRequest.CapabilityPair cap : caplist) {
            capsFromFw.addCapabilities(cap.getCapability());
        }
        Log.d(LOG_TAG, "changeEnabledCapabilities :: Query Modem caps[" +capsFromFw.toString()+"]");
        mConfigService.updateUceCapability(mLocalExecutor, capsFromFw, l);
    }

    @Override
    public void  destroyCapabilityExchangeImpl (
            RcsCapabilityExchangeImplBase rcsCapabilityExchangeImplBase) {
        //@TODO: call closeService on Presence & Options
        mCapExImpl = null;
    }

    @Override
    public RcsCapabilityExchangeImplBase createCapabilityExchangeImpl(
        CapabilityExchangeEventListener capabilityExchangeEventListener) {
        Log.d(LOG_TAG, "createCapabilityExchangeImpl :: invoked");
        if(mCapExImpl == null) {
            if(mPresenceListener == null) {
                mPresenceListener = new PresenceListener(mLocalExecutor);
                PresenceServiceWrapper presence = mManager.getImsPresenceService(mSlotId);
                presence.setCapInfolistener(mPresenceListener);
            }
            if(mOptionsListener == null) {
                mOptionsListener = new OptionsListener(mLocalExecutor);
                OptionsServiceWrapper options = mManager.getImsOptionsService(mSlotId);
                options.setOptionsCapListeners(mOptionsListener);
            }
            mCapExImpl = mManager.getRcsCapExchangeImpl(capabilityExchangeEventListener, mSlotId, mLocalExecutor);
        }
        return mCapExImpl;
    }

    @Override
    public void onFeatureReady() {
        Log.d(LOG_TAG, "onFeatureReady:: Received from FW");
        mLocalExecutor.execute(()->{
            try {
                Thread.sleep(200);
            } catch(InterruptedException e){
                e.printStackTrace();
            }
            setFeatureState(ImsFeature.STATE_READY);
        });
    }

    private void onCapsReceived(RcsImsCapabilities caps) {  //ImsConfig/Modem
        Log.d(LOG_TAG , "onCapsReceived :: caps received from Modem=" + caps.toString());
        //@TODO: check if setUCE will get this cb
        boolean presenceCapability = caps.isCapable(RcsFeature.RcsImsCapabilities.CAPABILITY_TYPE_PRESENCE_UCE);
        if (isPresenceConfigured != presenceCapability){
            Log.d(LOG_TAG , "onCapsReceived :: isPresenceConfigured updated to: "+
                  String.valueOf(presenceCapability));
            isPresenceConfigured = presenceCapability;
            //Check for non-zero capabilities
            if((mManager != null) && (isPresenceConfigured))
              mManager.reinitializePresenceService();
        }

        boolean optionsCapability = caps.isCapable(RcsFeature.RcsImsCapabilities.CAPABILITY_TYPE_OPTIONS_UCE);
        if (isOptionsConfigured != optionsCapability){
            Log.d(LOG_TAG , "onCapsReceived :: isOptionsConfigured updated to: "+
                  String.valueOf(optionsCapability));
                  isOptionsConfigured = optionsCapability;
            //Check for non-zero capabilities
            if((mManager != null) && (isOptionsConfigured))
              mManager.reinitializeOptionsService();
        }
        checkAndNotifyFw();
    }
    private void checkAndNotifyFw() {
        RcsImsCapabilities tempCapabilities =
               new RcsImsCapabilities(RcsFeature.RcsImsCapabilities.CAPABILITY_TYPE_NONE);
        Log.i(LOG_TAG, "checkAndNotifyFw :: [isPresenceConfigured: "+
                       isPresenceConfigured+",isPresenceRegistered: " +
                       isPresenceRegistered +
                       "], [isOptionsConfigured: "+isOptionsConfigured+
                       ",isOptionsRegistered: " +isOptionsRegistered +"]");
        if( isPresenceConfigured && isPresenceRegistered) {
            tempCapabilities.addCapabilities(RcsFeature.RcsImsCapabilities.CAPABILITY_TYPE_PRESENCE_UCE);
        }
        if( isOptionsConfigured && isOptionsRegistered) {
            tempCapabilities.addCapabilities(RcsFeature.RcsImsCapabilities.CAPABILITY_TYPE_OPTIONS_UCE);
        }
        if(!tempCapabilities.equals(mRcsCaps)) {
            Log.d(LOG_TAG, "checkAndNotifyFw :: mRcsCaps: "+tempCapabilities.toString());
            mRcsCaps = tempCapabilities;
            notifyCapabilitiesStatusChanged(new RcsImsCapabilities(mRcsCaps.getMask()));
        }
    }

    private void reset() {
        isPresenceRegistered = false;
        isOptionsRegistered = false;
        RcsFeature.RcsImsCapabilities capabilities = new RcsFeature.RcsImsCapabilities(CAPABILITY_TYPE_NONE);
        onCapsReceived(capabilities);
    }

    private class PresenceListener extends ImsPresCapEventListener {

        public PresenceListener(Executor e) {
            super(e);
        }
        @Override
        public void onServiceStatus(int s) {
            Log.i(LOG_TAG, "presence:onServiceStatus :: " +s);
            isPresenceRegistered = (s == SERVICE_REGISTERED);
            checkAndNotifyFw();
        }
        @Override
        public void onRequestPublishCapabilities(int aospTriggerType) {
            Log.i(LOG_TAG, "presence:publishTrigger :: " +aospTriggerType);
        }
        @Override
        public void onPresenceServiceDied() {
            reset();
        }
    };
    PresenceListener mPresenceListener;
    OptionsListener mOptionsListener;
    private class OptionsListener extends OptionsServiceWrapper.ImsOptionsCapEventListener {

        public OptionsListener(Executor e) {
            super(e);
        }

        @Override
        public void onServiceStatus(int s) {
            Log.d(LOG_TAG, "options:onServiceStatus :: " +s);
            isOptionsRegistered = (s == SERVICE_REGISTERED);
            checkAndNotifyFw();
        }

        @Override
        public void onOptionsServiceDied() {
            //@NOTE: Already handled in onPresenceDied
        }
    };
}

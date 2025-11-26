/**********************************************************************
 * Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsrcs.siptransport;

import android.telephony.ims.DelegateMessageCallback;
import android.telephony.ims.DelegateRequest;
import android.telephony.ims.DelegateStateCallback;
import android.telephony.ims.FeatureTagState;
import android.telephony.ims.SipDelegateManager;
import android.telephony.ims.stub.SipDelegate;
import android.telephony.ims.stub.SipTransportImplBase;
import android.telephony.ims.DelegateRegistrationState;
import android.util.ArraySet;
import android.util.Log;

import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.ScheduledExecutorService;

import vendor.qti.imsrcs.siptransport.hidl.SipDelegateWrapper;
import vendor.qti.imsrcs.siptransport.hidl.SipTransportServiceWrapper;
import vendor.qti.imsrcs.config.ImsConfigServiceImpl;
import vendor.qti.imsrcs.ImsRcsServiceMgr;
import vendor.qti.imsrcs.ImsRcsService;
import vendor.qti.ims.rcssip.V1_0.SipTransportStatusCode;
import vendor.qti.ims.rcssip.V1_0.cmServiceStatus;
import vendor.qti.ims.rcssip.V1_0.configData;
import vendor.qti.ims.rcssip.V1_0.featureTagData;

public class ImsRcsSipTransportImpl extends SipTransportImplBase{

    int mSlotId;
    ImsRcsServiceMgr mManager;
    SipTransportServiceWrapper sipTransport;
    SipTransportCapUpdateListener rcsCapUpdateListener;
    boolean m_isSipTransportEnabled = false;
    Executor mSipTransportExecutor = new ScheduledThreadPoolExecutor(1);
    ArrayList<SipDelegate> mSipDelegateList = new ArrayList<SipDelegate>();
    ImsConfigServiceImpl mConfigService;
    SipTransportServiceListener mSipTransportListener;
    final String LOG_TAG = "ImsRcsSipTransportImpl";

    private static int RCS_CAP_STATUS_UNKNOWN  = -1;
    private static int RCS_CAP_STATUS_DISABLED = 0;
    private static int RCS_CAP_STATUS_ENABLED  = 1;
    private static int SERVICE_STATUS_UNKNOWN = 2;
    private static int SERVICE_STATUS_AVAILABLE = 3;
    private static int SERVICE_STATUS_UNAVAILABLE = 4;

    private int mServiceStatus = SERVICE_STATUS_UNKNOWN;
    private int mRcsCapabilityStatus = RCS_CAP_STATUS_UNKNOWN;

    private int mCreateSipDelegateTimerValue = 10; //in seconds

    private String mUserAgentValue = "";

    public String getUserAgentValue(){
        return mUserAgentValue;
    }

    private void setUserAgentValue(String userAgentValue){
        this.mUserAgentValue = userAgentValue;
    }

    public ImsRcsSipTransportImpl(int slotId, ImsConfigServiceImpl config) {
        super(new ScheduledThreadPoolExecutor(1));
        Log.d(LOG_TAG, ":ctor ImsRcsSipTransportImpl");
        mSlotId = slotId;
        mManager = ImsRcsServiceMgr.getInstance();
        mConfigService = config;
        mSipTransportListener = new
            SipTransportServiceListener(mSipTransportExecutor);
        rcsCapUpdateListener = new SipTransportCapUpdateListener();
        mConfigService.registerForSipTransportCapability(
            mSipTransportExecutor, rcsCapUpdateListener);

        sipTransport = mManager.getSipTransportService(mSlotId,
                                            mSipTransportListener);
    }

    @Override
    public void createSipDelegate(
        final int slotId,
        final DelegateRequest delegateRequest,
        DelegateStateCallback delegateStateCallback,
        DelegateMessageCallback delegateMessageCallback) {
        Log.d(LOG_TAG, ": createSipDelegate");
        if(delegateRequest == null ||
                delegateStateCallback == null ||
                delegateMessageCallback == null) {
            Log.e(LOG_TAG, "null delegateRequest | delegateStateCallback"+
                  " | delegateMessageCallback.. This should never happen");
            //throw exception ?
        }

        Log.d(LOG_TAG, ": createSipDelegate: Before calling Config API");

        while(sipTransport == null) {
            sipTransport = mManager.getSipTransportService(mSlotId,
                                                mSipTransportListener);
        }

        ImsRcsSipDelegateImpl delegateObj = new ImsRcsSipDelegateImpl(
                delegateStateCallback,
                delegateMessageCallback,
                this,
                sipTransport);
        mSipDelegateList.add(delegateObj);

        ArrayList<String> featureTags = new ArrayList<String>();
        featureTags.addAll(delegateRequest.getFeatureTags());
        for(String ft: featureTags)
            Log.i(LOG_TAG,": createSipDelegate : requestedFts: "+ft);

        delegateObj.setRequestedFeatureTagsList(featureTags);

        if( mRcsCapabilityStatus == RCS_CAP_STATUS_ENABLED &&
            mServiceStatus == SERVICE_STATUS_AVAILABLE) {
            createConnectionForDelegate(delegateObj);
        } else {
            // wait for callback to come
            Log.i(LOG_TAG,": createSipDelegate : wait for rcs status"+
                          " callback or onServiceStatus to trigger " +
                          " createConnection");

          /* Run a timer so that, if no Rcs Cap Status is updated or
           * or service_available/service_unavailable is not received,
           * inform the clients.
           */

          ScheduledFuture<?> scheduleFuture =
          ((ScheduledExecutorService)mSipTransportExecutor).schedule(new Runnable() {
                        @Override
                            public void run() {
                                // send onCreated with fts as denied
                                Log.i(LOG_TAG,": createSipDelegate timer expired");
                                if(delegateObj.isCreateSipDelegateRequestPending()) {
                                    Log.i(LOG_TAG,":createSipDelegate timer expired - sending on created with denied fts");
                                    delegateObj.setIsDelegateActive(false);
                                    delegateObj.mSipDelegateStateCb.onCreated(delegateObj,
                                                 generateFeatureTagStateArray(delegateObj,
                                                SipDelegateManager.DENIED_REASON_UNKNOWN));
                                }
                            }}, mCreateSipDelegateTimerValue, TimeUnit.SECONDS);
          delegateObj.setCreateSipDelegateTimer(scheduleFuture);
        }

    }

    @Override
    public void destroySipDelegate(SipDelegate sipDelegate, int reason) {
        Log.i(LOG_TAG,": destroySipDelegate called");
        if(mSipDelegateList.contains(sipDelegate)) {
            ImsRcsSipDelegateImpl sipDelegateObj =
                (ImsRcsSipDelegateImpl) sipDelegate;

            //Check if any connection is queued for restoration
            if(sipDelegateObj.getIsDelegateNeedstoRestore()) {
                Log.i(LOG_TAG,":destroySipDelegate -Abort connection restoration");
                sipDelegateObj.setIsDelegateNeedstoRestore(false);
                mSipTransportExecutor.execute(
                    () -> {
                        Log.d(LOG_TAG,": posting OnDestroyed");
                        sipDelegateObj.mSipDelegateStateCb.onDestroyed(reason);
                        mSipDelegateList.remove(sipDelegateObj);
                    }
                );
            }
            //Check if any connection is in the process of restoration
            else if(sipDelegateObj.getIsDelegateRestoring()) {
                Log.i(LOG_TAG,":destroySipDelegate - wait till connection completes restoration");
                sipDelegateObj.setIsDelegatePendingDestroy(true, reason);
            } else {
                sipDelegateObj.setIsDelegateActive(false);
                if(sipDelegateObj.isConnectionHandleValid()) {
                    sipTransport.closeConnection(
                       reason, sipDelegateObj.mSipDelegateWrapper);
                } else {
                    mSipTransportExecutor.execute(
                       () -> {
                          Log.d(LOG_TAG,": posting OnDestroyed");
                          sipDelegateObj.mSipDelegateStateCb.onDestroyed(reason);
                          mSipDelegateList.remove(sipDelegateObj);
                       }
                    );
                }
            }
        } else {
            //SipDelegate obj cleaned up
            //without sipDelegate obj cant call onDestroyed on the cb objs
        }
        Log.i(LOG_TAG,": destroySipDelegate end");
    }

    private void createConnectionForDelegate(ImsRcsSipDelegateImpl delegate) {
        Log.d(LOG_TAG,": createConnectionForDelegate called");
        delegate.cancelCreateSipDelegateTimer();
        if(delegate.isCreateSipDelegateRequestPending()) {
            ArrayList<String> featureTags = delegate.getRequestedFeatureTagsList();
            StringBuilder featureTagsString = new StringBuilder("");
            for(String ftstring : featureTags) {
                featureTagsString.append(ftstring).append(";");
            }

            delegate.setCreateConnectionUserData(sipTransport.createConnection(
                                                 featureTagsString.toString(),
                                                 delegate.mSipDelegateWrapper));
        }
    }

    private void restoreConnectionForDelegate(ImsRcsSipDelegateImpl delegate) {
        Log.d(LOG_TAG,": restoreConnectionForDelegate called");
        ArrayList<String> featureTags = delegate.getRequestedFeatureTagsList();
        StringBuilder featureTagsString = new StringBuilder("");
        for(String ftstring : featureTags) {
            featureTagsString.append(ftstring).append(";");
        }
        Log.d(LOG_TAG,": restoreConnectionForDelegate before calling createConnection");
        delegate.setCreateConnectionUserData(sipTransport.createConnection(
                                            featureTagsString.toString(),
                                            delegate.mSipDelegateWrapper));
    }

    private void handlePendingCreateConnections() {
        Log.d(LOG_TAG,": handlePendingCreateConnections mRcsCapabilityStatus:"+
                       Integer.toString(mRcsCapabilityStatus)+" mServiceStatus:"+
                       Integer.toString(mServiceStatus));

        if(mRcsCapabilityStatus == RCS_CAP_STATUS_ENABLED &&
           mServiceStatus == SERVICE_STATUS_AVAILABLE) {
            for(SipDelegate delegate : mSipDelegateList) {
                ImsRcsSipDelegateImpl sipDelegateObj =
                    (ImsRcsSipDelegateImpl)delegate;
                if(sipDelegateObj.isSipDelegateNeedsRestoration) {
                    Log.d(LOG_TAG,":handlePendingCreateConnections-"+
                          " isSipDelegateNeedsRestoration true");
                    sipDelegateObj.setIsDelegateRestoring(true);
                    sipDelegateObj.isSipDelegateNeedsRestoration = false;
                    restoreConnectionForDelegate(sipDelegateObj);
                } else {
                    createConnectionForDelegate(sipDelegateObj);
                }
            }
        }
    }

    private ArraySet<FeatureTagState> generateFeatureTagStateArray(ImsRcsSipDelegateImpl sipDelegateObj,
                                                                   int ftStatus) {
        ArrayList<String> featureTags = new ArrayList<String>();
        featureTags.addAll(sipDelegateObj.getRequestedFeatureTags());
        ArraySet<FeatureTagState> featureTagStateArraySet = new ArraySet<>();
        for(String ftstring : featureTags) {
            FeatureTagState ftState = new FeatureTagState(ftstring, ftStatus);
            featureTagStateArraySet.add(ftState);
        }
        return featureTagStateArraySet;
    }

    private class SipTransportCapUpdateListener extends
        ImsConfigServiceImpl.SipTransportCapUpdateCallback {
        @Override
        public void onRcsStatusReceived(boolean isEnabled){
            Log.d(LOG_TAG, ": SipTransportCapUpdateListener -"+
                  "onRcsStatusReceived: "+ Boolean.toString(isEnabled));
            int previousRcsCapStatus = mRcsCapabilityStatus;
            if(isEnabled) {
                mRcsCapabilityStatus = RCS_CAP_STATUS_ENABLED;
            } else {
                mRcsCapabilityStatus = RCS_CAP_STATUS_DISABLED;
                return;
            }
            if( previousRcsCapStatus != mRcsCapabilityStatus) {
                handlePendingCreateConnections();
                mManager.reinitializeSipTransportService();
            }
        }

         @Override
        public void onUserAgentStringReceived(String userAgentString){
            if(userAgentString == null)
               return;
            Log.d(LOG_TAG, ": SipTransportCapUpdateListener -"+
                  "onUserAgentStringReceived: "+ userAgentString);

            if(mUserAgentValue.isEmpty()){
               Log.d(LOG_TAG, ": SipTransportCapUpdateListener mUserAgentValue empty");
               setUserAgentValue(userAgentString);
            }
            else if(!(mUserAgentValue.equals(userAgentString))){
                setUserAgentValue(userAgentString);
                 Log.d(LOG_TAG, ": SipTransportCapUpdateListener -"+
                  "updated value: "+ mUserAgentValue);
                for(SipDelegate sipDelegate : mSipDelegateList) {
                ImsRcsSipDelegateImpl sipDelegateObj =
                    (ImsRcsSipDelegateImpl) sipDelegate;
                    sipDelegateObj.updateUserAgentToFw(userAgentString);
                }

            }

        }
    };

    //use this as a wrapper for AOSP callback as well
    private class SipTransportServiceListener extends
        SipTransportServiceWrapper.ImsSipTransportEventListener {
        static final String LOG_TAG = "SipTransportServiceListener";
        public SipTransportServiceListener(Executor e) {
            super(e);
            Log.d(LOG_TAG, ": SipTransportServiceListener ctor ");
        }

        @Override
        public void onServiceStatus(int status) {
            Log.d(LOG_TAG, ": onServiceStatus called with status " +
                           Integer.toString(status));
            //when service is unavailble we will get cmd status failure
            // for createConnection itself
            if(status == cmServiceStatus.SERVICE_UNAVAILABLE) {
                Log.d(LOG_TAG, ": onServiceStatus with SERVICE_UNAVAILABLE");
                mServiceStatus = SERVICE_STATUS_UNAVAILABLE;
                for(SipDelegate delegate : mSipDelegateList) {
                    sipTransport.clearSipTransportService();
                }
            }
            else if(status == cmServiceStatus.SERVICE_AVAILABLE) {
                Log.d(LOG_TAG, ": onServiceStatus with SERVICE_AVAILABLE");
                mServiceStatus = SERVICE_STATUS_AVAILABLE;
                handlePendingCreateConnections();
            }
        }

        @Override
        public void onSipTransportServiceDied(){
            Log.d(LOG_TAG, ": onSipTransportServiceDied");
            Log.d(LOG_TAG, ": Cache delegate objs to restore connection once service is available");
            for(SipDelegate delegate : mSipDelegateList) {
                Log.d(LOG_TAG, ":  Set Delegate restore flag to true");
                ImsRcsSipDelegateImpl sipDlgt =
                    (ImsRcsSipDelegateImpl)delegate;
                sipDlgt.isSipDelegateNeedsRestoration = true;
                Log.d(LOG_TAG, ": onFeatureTagRegistrationChanged "+
                               "(DEREGISTERED_REASON_NOT_REGISTERED)");
                DelegateRegistrationState.Builder delegateRegState =
                    new DelegateRegistrationState.Builder();
                for(String deregisteredFt: sipDlgt.mSupportedFts) {
                    delegateRegState.addDeregisteredFeatureTag(
                        deregisteredFt,
                        DelegateRegistrationState
                            .DEREGISTERED_REASON_NOT_REGISTERED);
                }
                DelegateRegistrationState regState = delegateRegState.build();
                sipDlgt.mSipDelegateStateCb.onFeatureTagRegistrationChanged(regState);
            }
            //Reset the status flags
            mServiceStatus = SERVICE_STATUS_UNKNOWN;
            mRcsCapabilityStatus = RCS_CAP_STATUS_UNKNOWN;

            ImsRcsRegistrationImpl regImpl = ImsRcsService.getImsRcsRegistrationImpl(mSlotId);
            if(regImpl != null) {
                regImpl.clearRegisteringFeatureTags();
            }
        }

        @Override
        public void onConfigurationChange(configData configData) {
            for(SipDelegate sipDelegate : mSipDelegateList) {
                ImsRcsSipDelegateImpl sipDelegateObj =
                    (ImsRcsSipDelegateImpl) sipDelegate;
                sipDelegateObj.onConfigurationChange(configData);
            }
        }

        @Override
        public void onCmdStatus(int status,int userdata) {
            // only interested in onCmdstatus failure for createConnection
            // we wont receive failure for connectionDestroyed &
            // success means no implications to the FW
            Log.d(LOG_TAG, " onCmdStatus for status: "+status+" userdata: "+userdata);
            if(status != (int) SipTransportStatusCode.SUCCESS){
                for(SipDelegate delegate: mSipDelegateList) {
                    ImsRcsSipDelegateImpl sipDelegateObj =
                        (ImsRcsSipDelegateImpl) delegate;

                        if(sipDelegateObj.getCreateConnectionUserData() ==
                        userdata) {
                            ArraySet<FeatureTagState> featureTagStateArraySet =
                               generateFeatureTagStateArray(sipDelegateObj,
                               SipDelegateManager.DENIED_REASON_UNKNOWN);
                            sipDelegateObj.mSipDelegateStateCb.onCreated(
                               sipDelegateObj,
                               featureTagStateArraySet);
                        }

                }
            }
        }
    };

}

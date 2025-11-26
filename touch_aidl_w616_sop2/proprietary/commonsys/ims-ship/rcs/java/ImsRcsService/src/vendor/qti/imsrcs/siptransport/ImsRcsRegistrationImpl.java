/**********************************************************************
 * Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsrcs.siptransport;

import android.util.Log;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.telephony.ims.ImsRegistrationAttributes;

import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import android.util.ArraySet;

import vendor.qti.imsrcs.siptransport.hidl.SipTransportServiceWrapper;
import vendor.qti.imsrcs.ImsRcsServiceMgr;

public class ImsRcsRegistrationImpl extends ImsRegistrationImplBase {

    int mSlotId;
    ImsRcsServiceMgr mManager;
    SipTransportServiceWrapper mSipTransportWrapper;
    Executor mRcsRegistrationExecutor;
    int mUserData = -1;
    final String LOG_TAG = "ImsRcsRegistrationImpl";
    int mRatType = ImsRegistrationImplBase.REGISTRATION_TECH_NONE;
    ArraySet<String> mRegFeatures = new ArraySet<String>();
    ArraySet<String> mRegisteringFts = new ArraySet<String>();

    public ImsRcsRegistrationImpl(int slotId) {
        super();
        Log.d(LOG_TAG, "ctor ImsRcsRegistrationImpl");
        mRcsRegistrationExecutor = new ScheduledThreadPoolExecutor(1);
        mSlotId = slotId;
        mManager = ImsRcsServiceMgr.getInstance();
    }

    @Override
    public void updateSipDelegateRegistration() {
        Log.d(LOG_TAG, "updateSipDelegateRegistration");
        if(isSipTransportWrapperAvailable())
           mUserData = mSipTransportWrapper.triggerRegistration();
    }

    @Override
    public void triggerSipDelegateDeregistration() {
        // we dont do anything here
        super.triggerSipDelegateDeregistration();
        Log.d(LOG_TAG, "triggerSipDelegateDeregistration");
    }

    @Override
    public void triggerFullNetworkRegistration(int sipCode, String sipReason) {
        Log.d(LOG_TAG, "triggerFullNetworkRegistration");
        if(isSipTransportWrapperAvailable())
            mSipTransportWrapper.triggerRegRestoration(sipCode, sipReason);
    }

    private boolean isSipTransportWrapperAvailable() {
        Log.d(LOG_TAG, "isSipTransportWrapperAvailable");
        mSipTransportWrapper = mManager.getSipTransportService(mSlotId);
        if(mSipTransportWrapper == null) return false;
        return true;
    }

    public void setRatType(int ratType) {
        Log.d(LOG_TAG, "setRatType to: "+ Integer.toString(ratType));
        mRatType = ratType;
    }

    public int getRatType() {
        Log.d(LOG_TAG, "getRatType : "+ Integer.toString(mRatType));
        return mRatType;
    }

    public void setRegisteredFeatureTags(ArraySet<String> ft) {
        mRegFeatures.clear();
        mRegFeatures.addAll(ft);
    }

    public ArraySet<String> getRegisteredFeatureTags() {
        return mRegFeatures;
    }

    public void clearRegisteredFeatureTags() {
        mRegFeatures.clear();
    }

    public void setRegisteringFeatureTags(ArraySet<String> ft) {
        Log.d(LOG_TAG, "setRegisteringFeatureTags");
        for(String featureTag : ft)
           Log.d(LOG_TAG, "setRegistering feature"+featureTag);
        mRegisteringFts.addAll(ft);
    }

    public void removeRegisteringFeatureTags(ArraySet<String> ft) {
        Log.d(LOG_TAG, "removeRegisteringFeatureTags");
        for(String featureTag : ft)
            Log.d(LOG_TAG, "removeRegistering feature"+featureTag);
        mRegisteringFts.removeAll(ft);
    }

    public ArraySet<String> getRegisteringFeatureTags() {
        return mRegisteringFts;
    }

    public void clearRegisteringFeatureTags() {
        Log.d(LOG_TAG, "clearRegisteringFeatureTags");
        mRegisteringFts.clear();
    }


    public void checkToInvokeOnRegistered(ArraySet<String> ft, int ratType) {
        Log.d(LOG_TAG, "checkToInvokeOnRegistered: ratType passed["+
                        Integer.toString(ratType)+"] Existing mRattype["+
                        Integer.toString(mRatType));
        if(ft.isEmpty())
        {
          //invoked from publish trigger
          if(mRatType != ratType)
          {
            Log.d(LOG_TAG, "checkToInvokeOnRegistered: Rattype changed");
            if(mRegFeatures.isEmpty()) {
               // update the ratType to the latest value
               // mFeatures empty indicates there is no sipDelegate associated with any fts
               // Hence onRegistered need not be called for now
               Log.d(LOG_TAG, "checkToInvokeOnRegistered: no fts exists; so updating rattype alone");
               mRatType = ratType;
            } else {
               Log.d(LOG_TAG, "checkToInvokeOnRegistered: Calling onRegistered API with the Fts and ratType");
               mRatType = ratType;
               ImsRegistrationAttributes attr = new ImsRegistrationAttributes.Builder(
                    mRatType).setFeatureTags(mRegFeatures).build();
               this.onRegistered(attr);
            }
          }
        }
        else {
            //invoked from onEventReceived for SipDelegate
            if( (mRatType != ratType) ||
                (!(mRegFeatures.containsAll(ft)))) {
               Log.d(LOG_TAG, "checktoInvokeOnRegistered: rattype or fts changed");
               mRatType = ratType;
               setRegisteredFeatureTags(ft);
               ImsRegistrationAttributes attr = new ImsRegistrationAttributes.Builder(
                    mRatType).setFeatureTags(mRegFeatures).build();
               this.onRegistered(attr);
            }
         }
    }

    public int getUserData() {
        return mUserData;
    }

    public void setUserData(int data) {
        mUserData = data;
    }

    public void postOnRegistering() {
        Log.d(LOG_TAG, " postOnRegistering api called");
        if(mRegisteringFts.size() > 0) {
            ImsRegistrationAttributes attr = new ImsRegistrationAttributes.Builder(mRatType).
                                               setFeatureTags(mRegisteringFts).build();
            Log.d(LOG_TAG, "calling onRegistering");
            this.onRegistering(attr);
        }
    }
}

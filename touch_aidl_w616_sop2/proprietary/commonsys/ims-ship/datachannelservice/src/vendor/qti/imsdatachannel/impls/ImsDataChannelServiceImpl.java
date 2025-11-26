/**********************************************************************
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.impls;

import vendor.qti.imsdatachannel.aidl.IImsDataChannelServiceAvailabilityCallback;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelEventListener;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelTransport;

import vendor.qti.imsdatachannel.base.ImsDataChannelServiceBase;
import vendor.qti.imsdatachannel.ImsDataChannelServiceMgr;
import vendor.qti.imsdatachannel.wrappers.ImsDataChannelServiceWrapper;
import vendor.qti.imsdatachannel.impls.ImsDataChannelTransportImpl;
import vendor.qti.imsdatachannel.ImsDataChannelService;

import vendor.qti.ims.datachannelservice.ServiceState;

import android.telephony.TelephonyManager;
import android.os.RemoteException;
import android.os.IBinder;
import android.os.SystemProperties;
import android.provider.Settings;
import android.content.Context;
import android.util.Log;
import android.util.SparseArray;
import android.content.Intent;

import com.android.internal.annotations.VisibleForTesting;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.HashMap;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;




public class ImsDataChannelServiceImpl extends ImsDataChannelServiceBase {
    private static final String LOG_TAG = ImsDataChannelService.LOG_TAG + "_ImsDataChannelServiceImpl";
    public static int MAX_SLOT_VALUE = -1;
    IImsDataChannelEventListener mImsDcEvtListener;

    ImsDataChannelServiceWrapper mDcServiceWrapper;
    ImsDataChannelServiceMgr mDcServiceMgr;
    Executor mDcServiceImplExec;
    // ImsDataChannelTransportImpl mImsDataChannelTransportImpl;

    Map<Integer, ImsDataChannelServiceCbListener> mImsDcServiceCbListenerMap = new HashMap<>();
    Map<Integer, DcClientDeathReceipient> mImsDcClientDeathReceipientMap = new HashMap<>();
    Map<Integer, ImsDataChannelTransportImpl> mImsDcTransportImplMap = new HashMap<>();
    boolean mWaitingForCleanup = false;


    private boolean isInputDigit(String str) {
        boolean ret = true;
        for(int itr = 0; itr < str.length(); itr++)
        {
            if(!Character.isDigit(str.charAt(itr))) {
                ret = false;
                break;
            }
        }
        Log.i(LOG_TAG, "isInputDigit CallId = ," + str + " return = " + ret);
        return ret;
    }

    public ImsDataChannelServiceImpl(int numSlots){
        MAX_SLOT_VALUE = numSlots;
        mDcServiceMgr = ImsDataChannelServiceMgr.getInstance();
        mDcServiceImplExec = new ScheduledThreadPoolExecutor(1);
    }

    public void setServiceWrapperInst(ImsDataChannelServiceWrapper mDcServiceWrapper){
        this.mDcServiceWrapper = mDcServiceWrapper;
        Log.i(LOG_TAG, "wrapper set");
    }

    @Override
    public void onGetAvailability(int slotId, IImsDataChannelServiceAvailabilityCallback c) {
        mWaitingForCleanup = false;
        mDcServiceImplExec.execute(() -> {
            Log.i(LOG_TAG, "onGetAvailability");

            ImsDataChannelServiceCbListener mImsDcServiceCbListener = new ImsDataChannelServiceCbListener(mDcServiceImplExec);
            Log.i(LOG_TAG, "onGetAvailability mImsDcServiceCbListener addr:" + mImsDcServiceCbListener.toString());
            DcClientDeathReceipient deathRecipient = new DcClientDeathReceipient(slotId);
            try {
                c.asBinder().linkToDeath(deathRecipient, -1);
                mImsDcClientDeathReceipientMap.put(slotId, deathRecipient);
            }
            catch(RemoteException e){
                Log.i(LOG_TAG, "onGetAvailability linkToDeath exception");
                e.printStackTrace();
            }
            mImsDcServiceCbListener.setImsDcServiceAvailabilitycb(c);
            mImsDcServiceCbListenerMap.put(slotId, mImsDcServiceCbListener);
            Log.i(LOG_TAG, "onGetAvailability calling getDcServiceAvailability for slotID" + slotId);
            if (mDcServiceWrapper != null)
                mDcServiceWrapper.getDcServiceAvailability(slotId, mImsDcServiceCbListener);
            else
                Log.i(LOG_TAG, "mDcServiceWrapper is null for " + slotId);
        });
    }

    @Override
    public IImsDataChannelTransport onCreateDataChannelTransport(int slotId, String callId, IImsDataChannelEventListener l) {

        if((slotId >= MAX_SLOT_VALUE) || (callId.length() == 0) || (!isInputDigit(callId)))
        {
            Log.i(LOG_TAG, "onCreateDataChannelTransport: Invalid parameters slot id = " + slotId);
            return null;
        }

        for(Map.Entry <Integer, ImsDataChannelTransportImpl> e : mImsDcTransportImplMap.entrySet())
        {
            if(e.getKey().equals(slotId))
            {
                ImsDataChannelTransportImpl mImsDataChannelTransportImpl = e.getValue();
                if(mImsDataChannelTransportImpl.getCallId().equals(callId))
                {
                    Log.i(LOG_TAG, "onCreateDataChannelTransport: Already found: slot Id = ," + slotId + " Listener = "  + l);
                    mImsDataChannelTransportImpl.setTransportListener(l);
                    return mImsDataChannelTransportImpl.getBinder();
                }
            }
        }


        ImsDataChannelTransportImpl mImsDataChannelTransportImpl = new ImsDataChannelTransportImpl(slotId, callId, this, l);
            mImsDcTransportImplMap.put(slotId, mImsDataChannelTransportImpl);
            mDcServiceImplExec.execute(
            () -> {
                Log.i(LOG_TAG, "onCreateDataChannelTransport");
                mDcServiceWrapper.createDataChannelTransport(slotId,callId,mImsDataChannelTransportImpl.getImsDcTransportWrapperObj());
            });

        Log.i(LOG_TAG, "returning datachannelTransport binder, slot Id = ," + slotId + " call id = ," + callId + " Listener = " + l);
        return mImsDataChannelTransportImpl.getBinder();
    }

    @Override
    public void onCloseDataChannelTransport(IImsDataChannelTransport t) {
        mDcServiceImplExec.execute(
            () -> {
                ImsDataChannelTransportImpl imsDcTransportImpl = getImsDcTransportObj(t);
                if(imsDcTransportImpl != null){
                    Log.i(LOG_TAG, "onCloseDataChannelTransport");
                    mDcServiceWrapper.closeDataChannelTransport(imsDcTransportImpl.getDcTransportHandle());
                }
            }

        );
    }

    public ImsDataChannelTransportImpl getImsDcTransportObj(IImsDataChannelTransport t) {
        for (ImsDataChannelTransportImpl impl: mImsDcTransportImplMap.values()) {
            if (impl.getBinder() == t) {
                Log.i(LOG_TAG, "getImsDcTransportObj: " + t.toString());
                return impl;
            }
        }
        return null;
    }

    /*to be invoked when native service dies */
    public void cleanupAidlService(){
        Log.d(LOG_TAG, "cleanupAidlService");
        mDcServiceWrapper.setAidlDcService(null);
        for (ImsDataChannelTransportImpl imsDcTransportImpl : mImsDcTransportImplMap.values()) {
            imsDcTransportImpl.handleServiceDied();
        }
        mImsDcTransportImplMap.clear();
        mDcServiceWrapper.notifyServiceDied();
        mDcServiceMgr.startQueryForAidlService();
    }

    public void getAllServiceAvailability() {
        Log.d(LOG_TAG, "initiate query for availabilitystatus from service!! ");
        if (mDcServiceWrapper.getAidlDcService() != null)
            mDcServiceWrapper.getAllServiceAvailability();
        else
            Log.d(LOG_TAG, "mDcServiceWrapper.getAidlDcService() is null!! ");
    }


    // Invoked by ImsDataChannelServiceMgr on service onDestroy()
    public void cleanUp() {
        if (mImsDcTransportImplMap.isEmpty()) {
            if (mDcServiceWrapper != null) {
                // Needed to clean up service availability listeners
                mDcServiceWrapper.cleanUp();
            }
        } else {
            mWaitingForCleanup = true;
            //close all the connectionless transports now
            mDcServiceImplExec.execute(() -> {
                for (ImsDataChannelTransportImpl imsDcTransportImpl : mImsDcTransportImplMap.values()) {
                    if (imsDcTransportImpl != null && !imsDcTransportImpl.isActive()) {
                        Log.i(LOG_TAG, "cleanUp onCloseDataChannelTransport");
                        mDcServiceWrapper.closeDataChannelTransport(imsDcTransportImpl.getDcTransportHandle());
                    }
                }
            });
        }
    }

    public void handleClientDied(int slotId){
        Log.i(LOG_TAG, "clientDied mImsDcServiceCbListener slotId:" + slotId);
        mWaitingForCleanup = true;
        // Get rid of all listener objects from client
        if (mImsDcServiceCbListenerMap.get(slotId) != null) {
            mImsDcServiceCbListenerMap.remove(slotId).setImsDcServiceAvailabilitycb(null);
        }
        for(ImsDataChannelTransportImpl imsDcTransportImpl : mImsDcTransportImplMap.values()){
            if (imsDcTransportImpl != null) {
                Log.i(LOG_TAG, "clientDied calling imsDcTransportImpl.clientDied");
                imsDcTransportImpl.handleClientDied();
            }
        }
    }

    public void handleTransportClosed(ImsDataChannelTransportImpl imsDcTransportImpl) {
        if (mWaitingForCleanup) {
            mDcServiceWrapper.cleanUp(imsDcTransportImpl.getSlotId());
        }
        mImsDcTransportImplMap.remove(imsDcTransportImpl.getSlotId());
    }

    private class ImsDataChannelServiceCbListener extends ImsDataChannelServiceWrapper.ImsDataChannelServiceListener{

        private IImsDataChannelServiceAvailabilityCallback mIImsDcServiceAvailabilitycb;

        public ImsDataChannelServiceCbListener(Executor e){
            super(e);
        }

        @Override
        public void onGetServiceStatus(int svcState) {
            if (mIImsDcServiceAvailabilitycb == null) {
                Log.i(LOG_TAG, "mIImsDcServiceAvailabilitycb is null as clientDied, svcState: " + svcState);
                return;
            }
            try {
                if (svcState == ServiceState.AVAILABLE) {
                    Log.i(LOG_TAG, "calling onAvailable svcState: " + svcState);
                    mIImsDcServiceAvailabilitycb.onAvailable();
                }
                else if(svcState == ServiceState.UNAVAILABLE){
                    Log.i(LOG_TAG, "calling onUnAvailable svcState: " + svcState);
                    mIImsDcServiceAvailabilitycb.onUnAvailable();
                }
            } catch(RemoteException e){
                e.printStackTrace();
            }
        }

        public void setImsDcServiceAvailabilitycb(IImsDataChannelServiceAvailabilityCallback mIImsDcServiceAvailabilitycb){
            this.mIImsDcServiceAvailabilitycb = mIImsDcServiceAvailabilitycb;
        }

        public IImsDataChannelServiceAvailabilityCallback getImsDcServiceAvailabilitycb(){
            return mIImsDcServiceAvailabilitycb;
        }

    }

    private class DcClientDeathReceipient implements IBinder.DeathRecipient {

        int mSlotId;

        DcClientDeathReceipient(int slotId) {
            mSlotId = slotId;
        }

        @Override
        public void binderDied() {
            Log.d(LOG_TAG, "DcClientDeathReceipient binderDied() slotId:" + mSlotId);
            mImsDcServiceCbListenerMap.get(mSlotId).getImsDcServiceAvailabilitycb().asBinder().unlinkToDeath(this,0);
            handleClientDied(mSlotId);
            mImsDcClientDeathReceipientMap.remove(mSlotId);
        }
    }

}

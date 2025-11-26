/**********************************************************************
 * Copyright (c) 2021-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/


package vendor.qti.imsrcs.uce.hidl;

import android.net.Uri;
import android.os.RemoteException;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.concurrent.Executor;

import vendor.qti.ims.rcsuce.V1_2.IPresenceListener;
import vendor.qti.ims.rcsuce.V1_2.IPresenceService;
import vendor.qti.ims.rcsuce.V1_1.PresPublishTriggerType;
import vendor.qti.ims.rcsuce.V1_0.SipResponse;
import vendor.qti.ims.rcsuce.V1_0.SubscriptionInfo;
import vendor.qti.ims.rcsuce.V1_0.SubscriptionStatus;
import vendor.qti.ims.rcsuce.V1_0.UceServiceStatus;
import vendor.qti.ims.rcsuce.V1_0.UceStatusCode;

import android.telephony.ims.RcsUceAdapter;

import android.util.Log;
import vendor.qti.imsrcs.ImsRcsService;


public class PresenceServiceWrapper {
    public IPresenceService mHidlPresenceService_1 = null;
    public vendor.qti.ims.rcsuce.V1_0.IPresenceService mHidlPresenceService = null;
    private String LOG_TAG = ImsRcsService.LOG_TAG + ":PresenceServiceWrapper";
    PresenceListener mhidlPresenceListener = new PresenceListener();

    int mPublishTrigger = -1;
    int mServiceAvailable = -1;
    int mSlotId;

    public PresenceServiceWrapper(int SlotId) {
        LOG_TAG = LOG_TAG + "[" +SlotId+"]";
        mSlotId = SlotId;
    }
    public void close() {

    }

    public void presenceDied() {
        for(ImsPresCapEventListener e : mCapEventListnerList) {
            e.handlePresenceServiceDied();
        }
    }
    public void publishCapabilities(String pidfXml, long userData) {
        try {
            if(mHidlPresenceService_1 != null)
              mHidlPresenceService_1.publishCapability(pidfXml,userData);
            else if(mHidlPresenceService != null)
              mHidlPresenceService.publishCapability(pidfXml,userData);
            else
              Log.e(LOG_TAG, " publishCapabilities: hidlPresenceService not initialized");

        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    public void subscribeForCapabilities(Collection<Uri> list, long userData) {
        try {
            ArrayList<String> contacts = new ArrayList<String>();
            for(Uri s : list) {
                contacts.add(s.toString());
            }
            if(mHidlPresenceService_1 != null)
              mHidlPresenceService_1.getContactCapability(contacts,userData);
            else if(mHidlPresenceService != null)
              mHidlPresenceService.getContactCapability(contacts,userData);
            else
              Log.e(LOG_TAG, " subscribeForCapabilities: hidlPresenceService not initialized");

        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    private class PresenceListener extends IPresenceListener.Stub {

        @Override
        public void onServiceStatus(int i) throws RemoteException {
            Log.d(LOG_TAG, "PresenceListener: onServiceStatus :: received[" +i +"]");
            mServiceAvailable = i;
            for(ImsPresCapEventListener e : mCapEventListnerList) {
                e.handleServiceStatus(i);
            }
        }

        @Override
        public void onPublishTrigger(int i) throws RemoteException {
            Log.d(LOG_TAG, "PresenceListener: onPublishTrigger :: received [" +i +"]");
            int version = 0;
            mPublishTrigger = i;
            for(ImsPresCapEventListener e : mCapEventListnerList) {
                e.handlePublishTrigger(i, mSlotId, version);
            }
        }

        @Override
        public void onPublishTrigger_1_1(int i) throws RemoteException {
            Log.d(LOG_TAG, "PresenceListener: onPublishTrigger_1_1 :: received [" +i +"]");
            int version = 1;
            mPublishTrigger = i;
            for(ImsPresCapEventListener e : mCapEventListnerList) {
                e.handlePublishTrigger(i, mSlotId, version);
            }
        }

        @Override
        public void onCmdStatus(long l, int i) throws RemoteException {
            Log.d(LOG_TAG, "PresenceListener: onCmdStatus :: received");
            for(ImsPresCapEventListener e : mCapEventListnerList) {
                e.handleCmdStatus(l,i);
            }
        }

        @Override
        public void onSipResponse(long l, SipResponse sipResponse, short i) throws RemoteException {
            Log.d(LOG_TAG, "PresenceListener: onSipResponse :: received");
            for(ImsPresCapEventListener e : mCapEventListnerList) {
                e.handleSipResponse(l,sipResponse, i);
            }
        }

        @Override
        public void onPublishUpdate(long l, SipResponse sipResponse) throws RemoteException {
            Log.d(LOG_TAG, "PresenceListener: onPublishUpdate :: received");
            for(ImsPresCapEventListener e : mCapEventListnerList) {
                e.handlePublishUpdate(l,sipResponse);
            }
        }

        @Override
        public void onCapInfoReceived(
            long l,
            SubscriptionStatus subscriptionStatus,
            ArrayList<SubscriptionInfo> arrayList) throws RemoteException
        {
            Log.d(LOG_TAG, "PresenceListener: onCapInfoReceived :: received");
            for(ImsPresCapEventListener e : mCapEventListnerList) {
                e.handleCapInfo(l,subscriptionStatus, arrayList);
            }
        }

        @Override
        public void onCapInfoReceived_1_1(
            long l,
            SubscriptionStatus subscriptionStatus,
            ArrayList<SubscriptionInfo> arrayList,
            boolean isMultipartNotify) throws RemoteException
        {
            Log.d(LOG_TAG, "PresenceListener: onCapInfoReceived :: received");
            for(ImsPresCapEventListener e : mCapEventListnerList) {
                e.handleCapInfo_1_1(l,subscriptionStatus, arrayList, isMultipartNotify);
            }
        }


        @Override
        public void onUnpublishSent() throws RemoteException {
            Log.d(LOG_TAG, "PresenceListener: onUnpublishSent :: received");
            for(ImsPresCapEventListener e : mCapEventListnerList) {
                e.handlUnPublish();
            }
        }
    };

    public IPresenceListener getHidlPresenceListener() {
        return mhidlPresenceListener;
    }

    public void setHidlPresenceService(IPresenceService service) {
        mHidlPresenceService_1 = service;
    }
    public void setHidlPresenceService(vendor.qti.ims.rcsuce.V1_0.IPresenceService service) {
        mHidlPresenceService = service;
    }

    List<ImsPresCapEventListener> mCapEventListnerList = new ArrayList<ImsPresCapEventListener>();
    public void setCapInfolistener(ImsPresCapEventListener l) {
        int version = 0;
        mCapEventListnerList.add(l);
        if(mServiceAvailable != -1) {
            l.handleServiceStatus(mServiceAvailable);
        }
        if(mPublishTrigger != -1) {
          if(mHidlPresenceService_1 != null) {
            version = 1;
            l.handlePublishTrigger(mPublishTrigger, mSlotId, version);
          }
          else
            l.handlePublishTrigger(mPublishTrigger, mSlotId, version);
        }
    }

}

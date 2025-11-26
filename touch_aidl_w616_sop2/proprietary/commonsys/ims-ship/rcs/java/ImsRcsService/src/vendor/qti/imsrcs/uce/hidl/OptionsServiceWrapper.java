/**********************************************************************
 * Copyright (c) 2021, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/


package vendor.qti.imsrcs.uce.hidl;

import android.hidl.base.V1_0.DebugInfo;
import android.net.Uri;
import android.os.IHwBinder;
import android.os.NativeHandle;
import android.os.RemoteException;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.concurrent.Executor;

import vendor.qti.ims.rcsuce.V1_0.IOptionsListener;
import vendor.qti.ims.rcsuce.V1_0.IOptionsService;
import vendor.qti.ims.rcsuce.V1_0.SipResponse;
import vendor.qti.ims.rcsuce.V1_0.UceStatusCode;

import android.util.Log;
import vendor.qti.imsrcs.ImsRcsService;

public class OptionsServiceWrapper {

    IOptionsService mhidlOptionsService;
    private OptionsListener mhidlOptionsListener = new OptionsListener();
    private String LOG_TAG = ImsRcsService.LOG_TAG + ":OptionsServiceWrapper";
    int mServiceAvailable = -1;

    public OptionsServiceWrapper(int SlotId) {
        LOG_TAG = LOG_TAG + "[" +SlotId+"]";
    }

    public IOptionsListener getHidlOptionsListener() {
        return mhidlOptionsListener;
    }

    public void setHidlOptionsService(IOptionsService service) {
        mhidlOptionsService = service;
    }

    public void setOptionsCapListeners(ImsOptionsCapEventListener e) {
        mOptionsCapListeners.add(e);
        if(mServiceAvailable != -1) {
            e.handleServiceStatus(mServiceAvailable);
        }
    }
    public void removeOptionsCapListeners(ImsOptionsCapEventListener e) {
        mOptionsCapListeners.remove(e);
    }

    public void close() {

    }
    public void optionsDied() {
        for(ImsOptionsCapEventListener l: mOptionsCapListeners) {
            l.handleOptionsServiceDied();
        }
    }
    public void sendCapabilityRequest(Uri uri,Set<String> list, long userData) {
        ArrayList<String> fts = new ArrayList<String>();
        for(String s : list) {
            fts.add(s);
        }
        if (mhidlOptionsService != null) {
            try {
                mhidlOptionsService.getContactCapability(uri.toString(),fts,userData);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.e(LOG_TAG, " sendCapabilityRequest : mhidlOptionsService is null");
        }
    }

    public void respondToIncomingOptions(int tId, int code, String phrase,
                                         List<String> caps, boolean bContactInBL){
        try {
            SipResponse resp = new SipResponse();
            resp.code = (short)code;
            resp.reasonPhrase = phrase;
            ArrayList<String> fts = new ArrayList<String>();
            for(String s : caps) {
                fts.add(s);
            }
            if (mhidlOptionsService != null) {
                mhidlOptionsService.respondToIncomingOptions(tId,resp, fts,(byte)(bContactInBL?1:0));
            } else {
                Log.e(LOG_TAG, " respondToIncomingOptions : mhidlOptionsService is null");
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    // in future this will host new Hidl versions based classes
    private class OptionsListener extends  IOptionsListener.Stub {


        @Override
        public void onServiceStatus(int i) throws RemoteException {
            Log.d(LOG_TAG, "OptionsListener: onServiceStatus :: received");
            mServiceAvailable = i;
            for(ImsOptionsCapEventListener l: mOptionsCapListeners) {
                l.handleServiceStatus(i);
            }
        }

        @Override
        public void onSipResponse(long userData, SipResponse sipResponse, ArrayList<String> arrayList) throws RemoteException {
            Log.d(LOG_TAG, "OptionsListener: onSipResponse :: received userdata:" +userData);
            for(ImsOptionsCapEventListener l: mOptionsCapListeners) {
                    l.handleSipResonse(userData,sipResponse,arrayList);
            }
        }

        @Override
        public void onCmdStatus(long userData, int i) throws RemoteException {
            if(i == UceStatusCode.SUCCESS) {
                return;
            }
            for(ImsOptionsCapEventListener l: mOptionsCapListeners) {
                    l.handleCmdStatus(userData,i);
            }
        }

        @Override
        public void incomingOptionsRequest(String s, ArrayList<String> arrayList, short i) throws RemoteException {
            Log.d(LOG_TAG, "OptionsListener: incomingOptionsRequest :: received tid: "+i);
            for(ImsOptionsCapEventListener l: mOptionsCapListeners) {
                l.handleRemoteCapReq(i,s,arrayList);
            }
        }
    };

    List<ImsOptionsCapEventListener> mOptionsCapListeners = new ArrayList<ImsOptionsCapEventListener>();
    public static abstract class ImsOptionsCapEventListener {
        protected Executor mExecutor;
        public ImsOptionsCapEventListener(Executor e) {
            mExecutor = e;
        }

        public final void handleServiceStatus(int s) {
            mExecutor.execute(()->{
                onServiceStatus(s);
            });
        }

        public final void handleOptionsServiceDied() {
            mExecutor.execute(()->{
                onOptionsServiceDied();
            });
        }

        public final void handleRemoteCapReq(int tid, String uri, List<String> fts) {
            mExecutor.execute(()->{
                onRemoteCapabilityRequest(tid, uri, fts);
            });
        }

        public final void handleSipResonse(long userdata, SipResponse sipResponse, ArrayList<String> arrayList) {
            mExecutor.execute(()->{
                onSipResponse(userdata,sipResponse.code, sipResponse.reasonPhrase, arrayList);
            });
        }
        public final void handleCmdStatus(long userdata,int i) {
            mExecutor.execute(()->{
                onCmdStatus(userdata,i);
            });
        }
        public void onServiceStatus(int s) {}
        public void onOptionsServiceDied(){}
        public void onRemoteCapabilityRequest(int tid, String uri, List<String>remoteFTs) {}
        public void onCmdStatus(long userdata,int i) {}
        public void onSipResponse(long userdata,int sipCode, String reason, List<String>caps) {}
    };

}

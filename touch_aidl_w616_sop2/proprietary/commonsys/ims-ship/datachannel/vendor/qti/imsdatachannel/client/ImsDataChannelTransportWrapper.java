/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.client;

import android.os.Binder;

import java.util.List;
import java.util.ArrayList;
import android.util.Log;
import android.os.RemoteException;
import android.content.Context;

import vendor.qti.imsdatachannel.aidl.IImsDataChannelConnection;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelResponse;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelTransport;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelAttributes;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelEventListener;
import vendor.qti.imsdatachannel.aidl.ImsReasonCode;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelErrorCode;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelCommandErrorCode;
import vendor.qti.imsdatachannel.client.ImsDataChannelEventListener;
import vendor.qti.imsdatachannel.client.ImsDataChannelTransport;
import vendor.qti.imsdatachannel.client.ImsDataChannelConnection;
import vendor.qti.imsdatachannel.client.ImsDataChannelConnectionWrapper;

import java.util.concurrent.Executor;

public class ImsDataChannelTransportWrapper implements ImsDataChannelTransport{
    private static final String LOG_TAG = "datachannellib:ImsDataChannelTransportWrapper";

    private final Executor mClientAppExecutor;
    private final Executor mExecutor;
    private final ImsDataChannelEventListener mEventListener;
    private final int mSlotId;
    private final String mCallId;
    private IImsDataChannelTransport mDCBinderTransportInstance;

    public ImsDataChannelTransportWrapper(
        Executor localExecutor,
        Executor clientExecutor,
        int slotId,
        String callId,
        ImsDataChannelEventListener evtListnr) {
            mExecutor = localExecutor;
            mClientAppExecutor = clientExecutor;
            mSlotId = slotId;
            mCallId = callId;
            mEventListener = evtListnr;
    }

    private final IImsDataChannelEventListener mEventListenerStub =
            new IImsDataChannelEventListener.Stub() {

        @Override
        public void onDataChannelAvailable(
            ImsDataChannelAttributes attr,
            IImsDataChannelConnection dcConnection) {
            final long token = Binder.clearCallingIdentity();
            ImsDataChannelConnection conn = new ImsDataChannelConnectionWrapper(attr,dcConnection);
            try {
                mClientAppExecutor.execute(() ->
                mEventListener.onDataChannelAvailable(attr, conn));
            } finally {
                Binder.restoreCallingIdentity(token);
            }
        }

        @Override
        public void onDataChannelSetupRequest(
            ImsDataChannelAttributes[] attr) {
            final long token = Binder.clearCallingIdentity();
            try {
                mClientAppExecutor.execute(() ->
                mEventListener.onDataChannelSetupRequest(attr));
            } finally {
                Binder.restoreCallingIdentity(token);
            }
        }

        @Override
        public void onDataChannelCreated(
            ImsDataChannelAttributes attr,
            IImsDataChannelConnection dcConnection) {
            final long token = Binder.clearCallingIdentity();
            ImsDataChannelConnection conn = new ImsDataChannelConnectionWrapper(attr,dcConnection);
            try {
                mClientAppExecutor.execute(() ->
                mEventListener.onDataChannelCreated(attr, conn));
            } finally {
                Binder.restoreCallingIdentity(token);
            }
        }

        @Override
        public void onDataChannelSetupError(String dcId, ImsDataChannelErrorCode code) {
            final long token = Binder.clearCallingIdentity();
            try {
                mClientAppExecutor.execute(() ->
                mEventListener.onDataChannelSetupError(dcId, code));
            } finally {
                Binder.restoreCallingIdentity(token);
            }
        }

        @Override
	    public void onDataChannelTransportClosed(ImsReasonCode reasonCode) {
            final long token = Binder.clearCallingIdentity();
            try {
                mClientAppExecutor.execute(() ->
                mEventListener.onDataChannelTransportClosed(reasonCode));
            } finally {
                Binder.restoreCallingIdentity(token);
            }
        }

        @Override
        public void onDataChannelCommandError(String dcId, ImsDataChannelCommandErrorCode errorCode) {
            final long token = Binder.clearCallingIdentity();
            try {
                mClientAppExecutor.execute(() ->
                mEventListener.onDataChannelCommandError(dcId, errorCode));
            } finally {
                Binder.restoreCallingIdentity(token);
            }
        }
    };

    public void createDataChannel(
        String[] dcIdList,
        String xmlContent) throws RemoteException
    {
        mExecutor.execute(() -> {
            try {
                mDCBinderTransportInstance.createDataChannel(dcIdList, xmlContent);
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "createDataChannel: RemoteException occured");

            }
        });
    }

    public void respondToDataChannelSetUpRequest(
        ImsDataChannelResponse[] resp,
        String xmlContent) throws RemoteException
    {
        mExecutor.execute(() -> {
            try {
                mDCBinderTransportInstance.respondToDataChannelSetUpRequest(resp, xmlContent);
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "respondToDataChannelSetUpRequest: RemoteException occured");

            }
        });
    }

    public void closeDataChannel(
        ImsDataChannelConnection[] dc,
        ImsDataChannelErrorCode code) throws RemoteException
    {
        //typecast ImsDataChannelConnection objs to that of WrapperClass type
        List<ImsDataChannelConnectionWrapper> dcConnectionObjList = new ArrayList<ImsDataChannelConnectionWrapper>();
        for(ImsDataChannelConnection conn: dc) {
            dcConnectionObjList.add((ImsDataChannelConnectionWrapper)conn);
        }

        //Retrieve all binderobjs from the wrapper class
        List<IImsDataChannelConnection> dcConnectionBinderInstanceList = new ArrayList<IImsDataChannelConnection>();
        for(ImsDataChannelConnectionWrapper dcConnectionWrapperObj: dcConnectionObjList) {
            dcConnectionBinderInstanceList.add(
                dcConnectionWrapperObj.getDataChannelBinderConnectionInstance());
        }

        IImsDataChannelConnection[] dcBinderList = dcConnectionBinderInstanceList.toArray(
            new IImsDataChannelConnection[dcConnectionBinderInstanceList.size()]);

        mExecutor.execute(() -> {
            try {
                mDCBinderTransportInstance.closeDataChannel(dcBinderList, code);
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "closeDataChannel: RemoteException occured");

            }
        });
    }

    /**
     * @return Implementation of DataChannelEventListener.
    */
    public IImsDataChannelEventListener getDataChannelEventListener() {
        return mEventListenerStub;
    }

    public int getSlotId() {
        return mSlotId;
    }

    public String getCallId() {
        return mCallId;
    }

    public boolean equals(int slotId, String callId) {
        if(slotId == mSlotId && callId.equals(mCallId))
          return true;
        else
          return false;
    }

    public void setDataChannelTransportBinderInstance(IImsDataChannelTransport dcTransport) {
        mDCBinderTransportInstance = dcTransport;
    }

    public IImsDataChannelTransport getDataChannelTransportBinderInstance() {
        return mDCBinderTransportInstance;
    }
}
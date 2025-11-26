/**********************************************************************
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.client;

import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.ComponentName;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.Binder;
import android.util.Log;
import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.List;
import java.util.ArrayList;

import vendor.qti.imsdatachannel.aidl.IImsDataChannelService;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelServiceAvailabilityCallback;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelEventListener;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelTransport;
import vendor.qti.imsdatachannel.client.ImsDataChannelServiceManager;
import vendor.qti.imsdatachannel.client.ImsDataChannelServiceAvailabilityCallback;
import vendor.qti.imsdatachannel.client.ImsDataChannelEventListener;
import vendor.qti.imsdatachannel.client.ImsDataChannelTransport;
import vendor.qti.imsdatachannel.client.ImsDataChannelServiceConnectionCallback;

public class ImsDataChannelServiceManagerImpl implements ImsDataChannelServiceManager {

    private static final String LOG_TAG = "datachannellib:ImsDataChannelServiceManagerImpl";

    private Context mContext;

    private Executor mCbExecutor;

    private ImsDataChannelManagerServiceConnection mServiceConnection;

    private ImsDataChannelServiceConnectionCallback mDataChannelServiceConnectionCallback;

    private IImsDataChannelService mDataChannelService = null;

    private boolean mIsBound = false;

    private List<ImsDataChannelTransportWrapper> dataChannelTransportList = new ArrayList<ImsDataChannelTransportWrapper>();

    private static final String IMS_DC_SERVICE_PACKAGE = "vendor.qti.imsdatachannel";

    private static final int MAX_DC_SLOTS = 2;

    private static final int PRIMARY_SLOT = 0;

    private static final int SECONDARY_SLOT = 1;

    Executor mDataChannelMgrExecutor = new ScheduledThreadPoolExecutor(1);

    IImsDataChannelTransport transportBinderObj = null;
    ImsDataChannelTransportWrapper transportObj = null;

    private class ImsDcServiceAvailabilityCallback extends IImsDataChannelServiceAvailabilityCallback.Stub {
        int mSlotId;
        private ImsDataChannelServiceAvailabilityCallback mDataChannelServiceAvailabilityCallback;

        ImsDcServiceAvailabilityCallback(int slotId, ImsDataChannelServiceAvailabilityCallback callback) {
            mSlotId = slotId;
            mDataChannelServiceAvailabilityCallback = callback;
        }

        @Override
        public void onAvailable() {
            Log.d(LOG_TAG, "IImsDataChannelServiceAvailabilityCallback:onAvailable()");
            final long token = Binder.clearCallingIdentity();
            try {
                //invoke connectionImplBase
                mCbExecutor.execute(() ->
                mDataChannelServiceAvailabilityCallback.onAvailable());
            } finally {
                Binder.restoreCallingIdentity(token);
            }
        }

        @Override
        public void onUnAvailable() {
            Log.d(LOG_TAG, "IImsDataChannelServiceAvailabilityCallback:onUnAvailable()");
            final long token = Binder.clearCallingIdentity();
            try {
                //invoke connectionImplBase
                mCbExecutor.execute(() ->
                mDataChannelServiceAvailabilityCallback.onUnAvailable());
            } finally {
                Binder.restoreCallingIdentity(token);
            }
        }
    };
    ImsDcServiceAvailabilityCallback[] mServiceAvailabilityCb = new ImsDcServiceAvailabilityCallback[MAX_DC_SLOTS];

    public void initialize(Context context, Executor executor) {
        mContext = context;
        mCbExecutor = executor;
    }

    public class ImsDataChannelManagerServiceConnection implements ServiceConnection {

        public ImsDataChannelManagerServiceConnection() {

        }

        public void onServiceConnected(ComponentName name, IBinder boundService) {
            Log.e(LOG_TAG, "On service connected, get the binder object");
            mDataChannelService = IImsDataChannelService.Stub.asInterface(boundService);

            if(mDataChannelServiceConnectionCallback != null) {
                mDataChannelServiceConnectionCallback.onServiceConnected();
            }
        }

        public void onServiceDisconnected(ComponentName name) {
            Log.e(LOG_TAG, "On Service Disconnected ");
            mDataChannelService = null;

            if(mDataChannelServiceConnectionCallback != null) {
                mDataChannelServiceConnectionCallback.onServiceDisconnected();
            }
        }
    }


    public void connectImsDataChannelService(ImsDataChannelServiceConnectionCallback callback) {

        Log.d(LOG_TAG, "Creating IMS Data Channel Service, if not started start");
        if(mContext == null) {
            Log.e(LOG_TAG, "mContext is null, please invoke initialise method first ");
            return;
        }

        if(mIsBound != true) {
            mDataChannelServiceConnectionCallback = callback;
            Intent intent = new Intent(mContext, IImsDataChannelService.class);
            intent.setComponent(new ComponentName(IMS_DC_SERVICE_PACKAGE, IMS_DC_SERVICE_PACKAGE + ".ImsDataChannelService"));
            //Intent intent = new Intent(mContext, IImsDataChannelService.class);
            //intent.setAction(IImsDataChannelService.class.getName());
            mServiceConnection = new ImsDataChannelManagerServiceConnection();
            mIsBound = mContext.bindService(intent, mServiceConnection, mContext.BIND_AUTO_CREATE);
        } else {
            Log.e(LOG_TAG, "Service is already bound");
        }
    }

    public void disconnectImsDataChannelService() {

        Log.d(LOG_TAG, "Disconnecting from IMS Data Channel Service");
        if(mContext == null) {
            Log.e(LOG_TAG, "mContext is null, please invoke initialise method first ");
            return;
        }

        if(mIsBound) {
            mContext.unbindService(mServiceConnection);
            mIsBound = false;
            /* since onServiceDisconnected() of ServiceConnection will not be called always when service unbinds from framework, adding the callback logic here */
            if(mDataChannelServiceConnectionCallback != null) {
              Log.d(LOG_TAG, "call onServiceDisconnected callback to client");
              mDataChannelServiceConnectionCallback.onServiceDisconnected();
            }
        }
    }

    public void getAvailability(int slotId, ImsDataChannelServiceAvailabilityCallback callback) throws RemoteException {
        Log.d(LOG_TAG, "getAvailability slotId =" + slotId);
        if((mIsBound != true) || (mServiceConnection == null)) {
            Log.e(LOG_TAG, "Either service bind failed or service is not created");
            return;
        }

        if(slotId < PRIMARY_SLOT || slotId > SECONDARY_SLOT) {
            Log.e(LOG_TAG," slotId is incorrect");
            return;
        }
        if(mServiceAvailabilityCb[slotId] == null) {
            mServiceAvailabilityCb[slotId] = new ImsDcServiceAvailabilityCallback(slotId, callback);
        } else {
            Log.e(LOG_TAG," mServiceAvailability for this slot is already created.. This should not happen");
        }

        mDataChannelMgrExecutor.execute(() -> {
            try {
                mDataChannelService.getAvailability(slotId, mServiceAvailabilityCb[slotId]);
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "getAvailability remote exception");
                e.printStackTrace();
            }
        });
    }

    public ImsDataChannelTransport createDataChannelTransport(int slotId,
                                                              String callId,
                                                              ImsDataChannelEventListener listener) throws RemoteException {
        Log.d(LOG_TAG, "createDataChannelTransport slotId =" + slotId + "callId" + callId);
        if((mIsBound != true) || (mServiceConnection == null)) {
            Log.e(LOG_TAG, "Either service bind failed or service is not created");
            return null;
        }

        for(ImsDataChannelTransportWrapper transportObjIter: dataChannelTransportList) {
            if(transportObjIter.equals(slotId, callId)) {
                Log.e(LOG_TAG, "createDataChannelTransport: Object already exists ");
                return transportObjIter;
            }
        }

        transportObj = new ImsDataChannelTransportWrapper(mDataChannelMgrExecutor, mCbExecutor, slotId, callId, listener);
        //IImsDataChannelEventListener evtListener = transportObj.getDataChannelEventListener();

        mDataChannelMgrExecutor.execute(() -> {
          try {
            transportBinderObj = mDataChannelService.createDataChannelTransport(slotId, callId, transportObj.getDataChannelEventListener());
            if(transportBinderObj != null){
              transportObj.setDataChannelTransportBinderInstance(transportBinderObj);
              dataChannelTransportList.add(transportObj);
            }
          } catch(RemoteException e) {
            Log.e(LOG_TAG, "createDataChannelTransport remote exception");
            transportBinderObj = null;
            transportObj = null;
            e.printStackTrace();
            //return null;
          }
        });

        return transportObj;

    }

    public void closeDataChannelTransport(ImsDataChannelTransport dcTransport) throws RemoteException {

        Log.d(LOG_TAG, "closeDataChannelTransport");
        if((mIsBound != true) || (mServiceConnection == null)) {
            Log.e(LOG_TAG, "Either service bind failed or service is not created");
            return;
        }

        ImsDataChannelTransportWrapper dcTransportObj = (ImsDataChannelTransportWrapper)dcTransport;

        mDataChannelMgrExecutor.execute(() -> {
            try {
                mDataChannelService.closeDataChannelTransport(dcTransportObj.getDataChannelTransportBinderInstance());
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "closeDataChannelTransport remote exception");
                e.printStackTrace();
                return;
            }
        });

        if(dataChannelTransportList.contains(dcTransportObj)) {
            dataChannelTransportList.remove(dataChannelTransportList.indexOf(dcTransportObj));
        }
    }
}

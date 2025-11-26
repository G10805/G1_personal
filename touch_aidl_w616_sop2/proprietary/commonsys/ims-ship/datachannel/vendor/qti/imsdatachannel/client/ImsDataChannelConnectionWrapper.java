/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.client;

import android.os.Binder;

import java.util.List;
import android.util.Log;
import android.os.RemoteException;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import android.util.Log;

import vendor.qti.imsdatachannel.aidl.IImsDataChannelStatusCallback;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelMessageCallback;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelMessage;
import vendor.qti.imsdatachannel.aidl.ImsMessageStatusInfo;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelConnection;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelState;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelAttributes;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelCommandErrorCode;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelErrorCode;
import vendor.qti.imsdatachannel.client.ImsDataChannelStatusCallback;
import vendor.qti.imsdatachannel.client.ImsDataChannelMessageCallback;
import vendor.qti.imsdatachannel.client.ImsDataChannelConnection;

import java.util.concurrent.Executor;

public class ImsDataChannelConnectionWrapper implements ImsDataChannelConnection{

    private static final String LOG_TAG = "datachannellib:ImsDataChannelConnectionWrapper";
    private final Executor mExecutor = new ScheduledThreadPoolExecutor(1);
    private Executor mClientExecutor;
    private ImsDataChannelMessageCallback mMessageCb;
    private ImsDataChannelStatusCallback mStatusCb;
    private ImsDataChannelAttributes mDcAttributes;
    private IImsDataChannelConnection mDataChannelConnectionInstance;

    public ImsDataChannelConnectionWrapper(
        ImsDataChannelAttributes attr,
        IImsDataChannelConnection dcConnection)
    {
        mDcAttributes = attr;
        mDataChannelConnectionInstance = dcConnection;
    }

    private final IImsDataChannelStatusCallback mStatusBinder =
            new IImsDataChannelStatusCallback.Stub() {
        @Override
        public void onClosed(ImsDataChannelErrorCode code)
        {
            final long token = Binder.clearCallingIdentity();
            try {
                mClientExecutor.execute(() ->
                mStatusCb.onClosed(code));
            } finally {
                Binder.restoreCallingIdentity(token);
            }
        }

        @Override
        public void onStateChange(ImsDataChannelState dcState)
        {
            final long token = Binder.clearCallingIdentity();
            try {
                mClientExecutor.execute(() ->
                mStatusCb.onStateChange(dcState));
            } finally {
                Binder.restoreCallingIdentity(token);
            }
        }
    };

    private final IImsDataChannelMessageCallback mMessageBinder =
            new IImsDataChannelMessageCallback.Stub() {
        @Override
        public void onMessageReceived(ImsDataChannelMessage msg)
        {
            final long token = Binder.clearCallingIdentity();
            try {
                mClientExecutor.execute(() ->
                mMessageCb.onMessageReceived(msg));
            } finally {
                Binder.restoreCallingIdentity(token);
            }
        }

        @Override
        public void onMessageSendStatus(ImsMessageStatusInfo msgStatusInfo)
        {
            final long token = Binder.clearCallingIdentity();
            try {
                mClientExecutor.execute(() ->
                mMessageCb.onMessageSendStatus(msgStatusInfo));
            } finally {
                Binder.restoreCallingIdentity(token);
            }
        }

        @Override
        public void onMessageSendCommandError(ImsDataChannelCommandErrorCode errorCode)
        {
            final long token = Binder.clearCallingIdentity();
            try {
                mClientExecutor.execute(() ->
                mMessageCb.onMessageSendCommandError(errorCode));
            } finally {
                Binder.restoreCallingIdentity(token);
            }
        }
    };

    public void initialize(
        Executor cbExecutor,
        ImsDataChannelStatusCallback statusCb,
        ImsDataChannelMessageCallback msgCb) throws RemoteException
    {
        mClientExecutor = cbExecutor;
        mMessageCb = msgCb;
        mStatusCb = statusCb;

        mExecutor.execute(() -> {
            try {
                mDataChannelConnectionInstance.initialize(
                mStatusBinder,
                mMessageBinder);
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "initialize: RemoteException occured");
            }
        });
    }

    public void sendMessage(ImsDataChannelMessage dcMessage) throws RemoteException
    {
        mExecutor.execute(() -> {
            try {
                mDataChannelConnectionInstance.sendMessage(dcMessage);
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "sendMessage: RemoteException occured");
            }
        });

    }

    public void notifyMessageReceived(ImsMessageStatusInfo msgStatusInfo) throws RemoteException
	{
        mExecutor.execute(() -> {
            try {
                mDataChannelConnectionInstance.notifyMessageReceived(msgStatusInfo);
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "notifyMessageReceived: RemoteException occured");

            }
        });
    }

    public ImsDataChannelAttributes getConnectionAttributes() {
        return mDcAttributes;
    }

    public IImsDataChannelConnection getDataChannelBinderConnectionInstance() {
        return mDataChannelConnectionInstance;
    }

}
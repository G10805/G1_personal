/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.impls;


import vendor.qti.imsdatachannel.base.ImsDataChannelConnectionBase;
import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;

import vendor.qti.imsdatachannel.wrappers.ImsDataChannelConnectionWrapper;
import vendor.qti.imsdatachannel.impls.ImsDataChannelConnectionImpl;
import vendor.qti.imsdatachannel.ImsDataChannelService;

import vendor.qti.imsdatachannel.aidl.ImsDataChannelErrorCode;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelResponse;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelCommandErrorCode;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelState;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelMessageCallback;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelStatusCallback;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelConnection;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelMessage;
import vendor.qti.imsdatachannel.aidl.ImsMessageStatusInfo;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelAttributes;
import vendor.qti.imsdatachannel.aidl.ImsReasonCode;
import vendor.qti.imsdatachannel.aidl.ImsMessageStatus;

import vendor.qti.ims.datachannelservice.IDataChannelConnection;
import vendor.qti.ims.datachannelservice.DataChannelMessage;
import vendor.qti.ims.datachannelservice.DataChannelCommandErrorCode;
import vendor.qti.ims.datachannelservice.DataChannelErrorCode;
import vendor.qti.ims.datachannelservice.MessageStatusInfo;
import vendor.qti.imsdatachannel.aidl.DataChannelState;
import vendor.qti.ims.datachannelservice.DataChannelInfo;
import vendor.qti.ims.datachannelservice.ReasonCode;
import vendor.qti.ims.datachannelservice.MessageStatus;

import android.os.RemoteException;
import android.os.Parcelable;
import android.util.Log;
import android.util.SparseArray;

import com.android.internal.annotations.VisibleForTesting;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.HashMap;
import java.util.Map;
import android.os.IBinder;


import java.util.ArrayList;
import java.util.List;
//import vendor.qti.ims.SparseArray;

import android.content.Context;
import android.util.Log;



public class ImsDataChannelConnectionImpl extends ImsDataChannelConnectionBase {

    private String LOG_TAG = ImsDataChannelService.LOG_TAG + "_ImsDataChannelConnectionImpl";
    ImsDataChannelConnectionWrapper mImsDcConnectionWrapper;
    private Executor mConnectionExecutor;
    private String mDcId;
    IImsDataChannelStatusCallback mImsDcStatusCb;
    IImsDataChannelMessageCallback mImsDcMessageCb;
    ImsDataChannelConnectionCbListener mImsDcConnectionListener;

    ///thinking of storing the connection state here

    public ImsDataChannelConnectionImpl(DataChannelInfo dcInfo, IDataChannelConnection mAidlDcConnection, ImsDataChannelTransportImpl imsDcTransportImpl){
        mDcId = dcInfo.dcId;
        LOG_TAG += "[" + mDcId + "]";
        mConnectionExecutor = new ScheduledThreadPoolExecutor(1);
        mImsDcConnectionListener = new ImsDataChannelConnectionCbListener(mConnectionExecutor, imsDcTransportImpl);
        mImsDcConnectionWrapper = new ImsDataChannelConnectionWrapper(mAidlDcConnection,mImsDcConnectionListener);
    }

    public String getConnDcId(){
        return mDcId;
    }

    public void setConnDcId(String dcId){
        this.mDcId = dcId;
    }

    public void handleServiceDied() {
        if (mImsDcConnectionWrapper != null) {
            mImsDcConnectionWrapper.setAidlDcConnection(null);
        }
        if (mImsDcConnectionListener != null) {
            mImsDcConnectionListener.clearTransportImpl();
            mImsDcConnectionListener.onConnectionStateChange(DataChannelState.DATA_CHANNEL_CLOSED);
            mImsDcConnectionListener.onConnectionClosed(ImsDataChannelErrorCode.DCS_DC_ERROR_NONE); // #TODO: see if this should change
        }
    }

    public void handleClientDied() {
        mImsDcStatusCb = null;
        mImsDcMessageCb = null;
    }

    @Override
    public void onInitialize(IImsDataChannelStatusCallback c, IImsDataChannelMessageCallback m){
        if(mConnectionExecutor==null)
            return;
        mConnectionExecutor.execute(
            () -> {
                mImsDcStatusCb = c;
                mImsDcMessageCb = m;
                Log.i(LOG_TAG, " calling registerListener");
                mImsDcConnectionWrapper.registerConnectionListener();
            }
        );

    }

    @Override
    public void onSendMessage(ImsDataChannelMessage dcMessage){
        if(mConnectionExecutor==null)
            return;
        mConnectionExecutor.execute(
            () -> {
                DataChannelMessage msg = getAidlDcMessage(dcMessage);
                Log.i(LOG_TAG, " calling sendMessage with datachannelmessage: " + msg.toString());
                mImsDcConnectionWrapper.sendMessage(msg);
            }
        );
    }

    @Override
    public void onNotifyMessageReceived(ImsMessageStatusInfo msgStatusInfo){
        Log.d(LOG_TAG, " empty impl!!! onNotifyMessageReceived with ImsMessageStatusInfo: " + msgStatusInfo.toString());
    }

    private DataChannelMessage getAidlDcMessage(ImsDataChannelMessage dcMessage){
        DataChannelMessage msg = new DataChannelMessage();
        msg.dcId = dcMessage.getDcId();
        msg.protocolId = dcMessage.getProtocolId();
        msg.msgId = dcMessage.getMessageId();
        msg.msgContent = dcMessage.getMessage().clone();
        return msg;
    }

    public void cleanUp() {
        mDcId = null;
        mImsDcConnectionListener.clearTransportImpl();
        mImsDcConnectionListener = null;
        mImsDcConnectionWrapper = null;
        mConnectionExecutor = null;
        mImsDcMessageCb = null;
        mImsDcStatusCb = null;
    }


    public ImsDataChannelMessage getImsDcMessage(DataChannelMessage dcMsg){
        ImsDataChannelMessage imsDcMsg = new ImsDataChannelMessage();
        imsDcMsg.setDcId(dcMsg.dcId);
        imsDcMsg.setProtocolId(dcMsg.protocolId);
        imsDcMsg.setMessageId(dcMsg.msgId);
        imsDcMsg.setMessage(dcMsg.msgContent);
        return imsDcMsg;
    }

    public ImsMessageStatusInfo getImsMessageStatusInfo(MessageStatusInfo msg){
        ImsMessageStatusInfo imsMsgInfo = new ImsMessageStatusInfo();
        imsMsgInfo.setMsgTransactionId(msg.msgId);
        ImsMessageStatus mStatus = new ImsMessageStatus();
        mStatus.setImsMessageStatus(msg.status);
        imsMsgInfo.setMsgStatus(mStatus);
        imsMsgInfo.setMsgStatusErrorCode(msg.messageStatusCode);
        return imsMsgInfo;
    }

    public ImsDataChannelErrorCode getImsDcErrorCode(int code){
        ImsDataChannelErrorCode errCode = new ImsDataChannelErrorCode();
        errCode.setImsDataChannelErrorCode(code);
        return errCode;
    }

     public ImsDataChannelCommandErrorCode getImsDcCommandErrorCode(int code){
        ImsDataChannelCommandErrorCode errCode = new ImsDataChannelCommandErrorCode();
        errCode.setImsDataChannelCommandErrorCode(code);
        return errCode;
    }

    public ImsDataChannelState getImsDataChannelState(int dcState) {
        ImsDataChannelState imsDataChannelState = new ImsDataChannelState();
        imsDataChannelState.setDcId(mDcId);
        DataChannelState dataChannelState = new DataChannelState();
        dataChannelState.setDataChannelState(dcState);
        imsDataChannelState.setState(dataChannelState);
        return imsDataChannelState;
    }

    private class ImsDataChannelConnectionCbListener extends ImsDataChannelConnectionWrapper.ImsDataChannelConnectionListener{

        ImsDataChannelTransportImpl mImsDcTransportImpl;

        public ImsDataChannelConnectionCbListener(Executor e, ImsDataChannelTransportImpl imsDcTransportImpl){
            super(e);
            mImsDcTransportImpl = imsDcTransportImpl;
        }

        public void clearTransportImpl() {
            mImsDcTransportImpl = null;
        }

        @Override
        protected void onConnectionStateChange(int dcState){
            if (mImsDcStatusCb != null) {
                try {
                    Log.i(LOG_TAG, " calling onStateChange with dcState: " + dcState);
                    mImsDcStatusCb.onStateChange(getImsDataChannelState(dcState));
                } catch(RemoteException e) {
                    Log.e(LOG_TAG, "  onConnectionStateChange with exception: " + e);
                    e.printStackTrace();
                }
            }
            if (dcState == DataChannelState.DATA_CHANNEL_CLOSED && mImsDcTransportImpl != null) {
                Log.d(LOG_TAG, "state changed to CLOSED. Calling closeDataChannelConnections on transport");
                // IImsDataChannelConnection[] dcBinderArr = new IImsDataChannelConnection[1];
                // dcBinderArr[0] = getBinder();
                // mImsDcTransportImpl.onCloseDataChannel(dcBinderArr, getImsDcErrorCode(ImsDataChannelErrorCode.DCS_DC_ERROR_NONE));

                String[] dcIds = new String[1];
                dcIds[0] = mDcId;
                mImsDcTransportImpl.closeDataChannelConnections(dcIds, ImsDataChannelErrorCode.DCS_DC_ERROR_NONE);
            }
        }

        @Override
        protected void onMessageReceived(DataChannelMessage msg){
            if (mImsDcMessageCb != null) {
                try {
                    Log.i(LOG_TAG, " calling onMessageReceived with datachannelmessage: " + msg.toString());
                    mImsDcMessageCb.onMessageReceived(getImsDcMessage(msg));
                } catch(RemoteException e) {
                    Log.e(LOG_TAG, "  onMessageReceived with exception: " + e);
                    e.printStackTrace();
                }
            }
        }

        @Override
        protected void onMessageSendStatus(MessageStatusInfo msgStatusInfo){
            if(mImsDcMessageCb != null){
                try {
                    Log.i(LOG_TAG, " calling onMessageSendStatus with msgStatusInfo: " + msgStatusInfo.toString());
                    mImsDcMessageCb.onMessageSendStatus(getImsMessageStatusInfo(msgStatusInfo));
                } catch(RemoteException e) {
                    Log.e(LOG_TAG, "  onMessageSendStatus with exception: " + e);
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onMessageSendCommandError(int errorCode){
            if(mImsDcMessageCb != null){
                try {
                    Log.i(LOG_TAG, " calling onMessageSendCommandError with errorCode: " + errorCode);
                    mImsDcMessageCb.onMessageSendCommandError(getImsDcCommandErrorCode(errorCode));
                } catch(RemoteException e) {
                    Log.e(LOG_TAG, "  onMessageSendCommandError with exception: " + e);
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onConnectionClosed(int code){
            if (mImsDcStatusCb != null) {
                try{
                    Log.i(LOG_TAG, " calling onClosed with errorCode: " + code);
                    mImsDcStatusCb.onClosed(getImsDcErrorCode(code));
                } catch(RemoteException e){
                    Log.e(LOG_TAG, "  onConnectionClosed with exception: " + e);
                    e.printStackTrace();
                }
            }
            cleanUp();
            if (mImsDcTransportImpl != null)
                mImsDcTransportImpl.clearConnectionImpl(mDcId);
            mImsDcTransportImpl = null;
        }

    }
}

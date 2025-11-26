/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.wrappers;
import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;

import vendor.qti.ims.datachannelservice.DataChannelState;
import vendor.qti.ims.datachannelservice.DataChannelMessage;
import vendor.qti.ims.datachannelservice.MessageStatusInfo;
import vendor.qti.ims.datachannelservice.DataChannelErrorCode;
import vendor.qti.ims.datachannelservice.DataChannelCommandErrorCode;
import vendor.qti.ims.datachannelservice.IDataChannelConnectionListener;
import vendor.qti.ims.datachannelservice.IDataChannelConnection;
import vendor.qti.ims.datachannelservice.ReasonCode;
import android.os.IBinder;
import java.util.*;
import android.os.ServiceManager;
import android.util.Log;
import android.os.RemoteException;
import android.os.Parcelable;

public class ImsDataChannelConnectionWrapper{

    private String LOG_TAG = "ImsDataChannelService_ImsDataChannelConnectionWrapper";

    private IDataChannelConnection mAidlDcConnection;
    private DataChannelConnectionListener dcConnectionListener;
    ImsDataChannelConnectionListener mImsDcConnectionListener;

    public ImsDataChannelConnectionWrapper(IDataChannelConnection mAidlDcConnection,ImsDataChannelConnectionListener mImsDcConnectionListener){
        this.mAidlDcConnection = mAidlDcConnection;
        dcConnectionListener = new DataChannelConnectionListener();
        this.mImsDcConnectionListener = mImsDcConnectionListener;
    }

    public void setAidlDcConnection(IDataChannelConnection aidlDcConnection){
        mAidlDcConnection = aidlDcConnection;
    }

    public IDataChannelConnection getAidlDcConnection(){
        return mAidlDcConnection;
    }

    public void registerConnectionListener(){
        if (mAidlDcConnection != null) {
            try{
                Log.d(LOG_TAG, "registerConnectionListener calling to native" + dcConnectionListener.toString());
                mAidlDcConnection.registerListener(dcConnectionListener);
            } catch(RemoteException e){
                e.printStackTrace();
            }
        } else {
            Log.e(LOG_TAG, "registerConnectionListener mAidlDcConnection is null");
        }
    }

    public void sendMessage(DataChannelMessage msg){
        if (mAidlDcConnection != null) {
            try{
                Log.d(LOG_TAG, "sendMessage calling to native");
                mAidlDcConnection.sendMessage(msg);
            } catch(RemoteException e){
                Log.d(LOG_TAG, "sendMessage exception!!");
                e.printStackTrace();
            }
        } else {
            Log.e(LOG_TAG, "sendMessage mAidlDcConnection is null");
        }
    }

    public class DataChannelConnectionListener extends IDataChannelConnectionListener.Stub{
        @Override
        public void onConnectionStateChange(int dcState){
            if(mImsDcConnectionListener != null)
                mImsDcConnectionListener.handleOnConnectionStateChange(dcState);
        }

        @Override
        public void onMessageReceived(DataChannelMessage msg){
            if(mImsDcConnectionListener != null)
                mImsDcConnectionListener.handleOnMessageReceived(msg);
        }

        @Override
        public void onMessageSendStatus(MessageStatusInfo msgStatusInfo){
            if(mImsDcConnectionListener != null)
                mImsDcConnectionListener.handleOnMessageSendStatus(msgStatusInfo);
        }

        @Override
        public void onMessageSendCommandError(int errorCode){
            if(mImsDcConnectionListener != null)
                mImsDcConnectionListener.handleOnMessageSendCommandError(errorCode);
        }

        @Override
        public void onConnectionClosed(int code){
            if(mImsDcConnectionListener != null)
                mImsDcConnectionListener.handleOnConnectionClosed(code);
            mAidlDcConnection = null;
        }

        @Override
        public final int getInterfaceVersion() { return IDataChannelConnectionListener.VERSION; }

        @Override
        public final String getInterfaceHash() { return IDataChannelConnectionListener.HASH; }
    }

    public static abstract class ImsDataChannelConnectionListener{
        protected Executor mExecutor;
        public ImsDataChannelConnectionListener(Executor e){
            mExecutor = e;
        }

        public final void handleOnConnectionStateChange(int dcState){
            mExecutor.execute(
                () -> {
                    onConnectionStateChange(dcState);
                }
            );
        }


        public final void handleOnMessageReceived(DataChannelMessage msg){
            mExecutor.execute(
                () -> {
                    onMessageReceived(msg);
                }
            );
        }


        public final void handleOnMessageSendStatus(MessageStatusInfo msgStatusInfo){
            mExecutor.execute(
                () -> {
                    onMessageSendStatus(msgStatusInfo);
                }
            );
        }

        public final void handleOnMessageSendCommandError(int errorCode){
            mExecutor.execute(
                () -> {
                    onMessageSendCommandError(errorCode);
                }
            );
        }

        public final void handleOnConnectionClosed(int code){
            mExecutor.execute(
                () -> {
                    onConnectionClosed(code);
                }
            );
        }

        protected void onConnectionStateChange(int dcState){}

        protected void onMessageReceived(DataChannelMessage msg){}

        protected void onMessageSendStatus(MessageStatusInfo msgStatusInfo){}

        protected void onMessageSendCommandError(int errorCode){}

        protected void onConnectionClosed(int code){}

    }

}

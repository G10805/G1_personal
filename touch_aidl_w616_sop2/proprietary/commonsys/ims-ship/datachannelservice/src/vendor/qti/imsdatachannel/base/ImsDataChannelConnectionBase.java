/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.base;

import vendor.qti.imsdatachannel.aidl.IImsDataChannelStatusCallback;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelMessageCallback;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelMessage;
import vendor.qti.imsdatachannel.aidl.ImsMessageStatusInfo;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelConnection;

public abstract class ImsDataChannelConnectionBase {

    private final class ImsDataChannelConnectionBinder extends IImsDataChannelConnection.Stub {

        @Override
        public void initialize(IImsDataChannelStatusCallback c, IImsDataChannelMessageCallback m){
           onInitialize(c,m);
        }

        @Override
        public void sendMessage(ImsDataChannelMessage dcMessage){
           onSendMessage(dcMessage);
        }

        @Override
        public void notifyMessageReceived(ImsMessageStatusInfo msgStatusInfo){
           onNotifyMessageReceived(msgStatusInfo);
        }

    }

    private ImsDataChannelConnectionBinder mImsDcConnectionBinder;

    public ImsDataChannelConnectionBinder getBinder(){
        if(mImsDcConnectionBinder == null)
           mImsDcConnectionBinder = new ImsDataChannelConnectionBinder();
        return mImsDcConnectionBinder;
    }


    protected void onInitialize(IImsDataChannelStatusCallback c, IImsDataChannelMessageCallback m){
    }

    protected void onSendMessage(ImsDataChannelMessage dcMessage){
    }

    protected void onNotifyMessageReceived(ImsMessageStatusInfo msgStatusInfo){
    }

}

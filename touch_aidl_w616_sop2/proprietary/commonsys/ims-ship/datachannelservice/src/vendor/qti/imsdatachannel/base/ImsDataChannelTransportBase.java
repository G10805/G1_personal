/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.base;

import vendor.qti.imsdatachannel.aidl.IImsDataChannelConnection;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelResponse;
import vendor.qti.imsdatachannel.base.ImsDataChannelTransportBase;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelTransport;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelErrorCode;

public abstract class ImsDataChannelTransportBase {
    private final class ImsDataChannelTransportBinder extends IImsDataChannelTransport.Stub  {
        @Override
        public void createDataChannel(String[] channelIdList, String mXmlContent){
           onCreateDataChannel(channelIdList,mXmlContent);
        }
        @Override
        public void respondToDataChannelSetUpRequest(ImsDataChannelResponse[] r, String mXmlContent){
           onRespondToDataChannelSetUpRequest(r,mXmlContent);
        }

        @Override
        public void closeDataChannel(IImsDataChannelConnection[] dc, ImsDataChannelErrorCode code){
           onCloseDataChannel(dc,code);
        }

    }

    private ImsDataChannelTransportBinder mImsDcTransportBinder;

    public ImsDataChannelTransportBinder getBinder(){
        if(mImsDcTransportBinder == null)
          mImsDcTransportBinder = new ImsDataChannelTransportBinder();
        return mImsDcTransportBinder;
    }

    protected void onCreateDataChannel(String[] channelIdList, String mXmlContent){
        return;
    }

    protected void onRespondToDataChannelSetUpRequest(ImsDataChannelResponse[] r, String mXmlContent){
        return;
    }

    protected void onCloseDataChannel(IImsDataChannelConnection[] dc, ImsDataChannelErrorCode code){
        return;
    }
}

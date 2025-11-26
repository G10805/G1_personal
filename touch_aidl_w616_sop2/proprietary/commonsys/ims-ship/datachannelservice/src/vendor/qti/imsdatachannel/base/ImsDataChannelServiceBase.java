/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.base;

import vendor.qti.imsdatachannel.aidl.IImsDataChannelService;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelServiceAvailabilityCallback;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelEventListener;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelTransport;

public abstract class ImsDataChannelServiceBase {

    private final class ImsDataChannelServiceBinder extends IImsDataChannelService.Stub {

        @Override
        public void getAvailability(int slotId, IImsDataChannelServiceAvailabilityCallback c) {
          onGetAvailability(slotId,c);
        }

        @Override
        public IImsDataChannelTransport createDataChannelTransport(int slotId, String callId, IImsDataChannelEventListener l) {
          return onCreateDataChannelTransport(slotId,callId,l);
        }

        @Override
        public void closeDataChannelTransport(IImsDataChannelTransport t) {
          onCloseDataChannelTransport(t);
        }
    }

    private ImsDataChannelServiceBinder mDcServiceBinder;

    public ImsDataChannelServiceBinder getBinder(){
      if(mDcServiceBinder == null)
        mDcServiceBinder = new ImsDataChannelServiceBinder();
      return mDcServiceBinder;
    }

    protected void onGetAvailability(int slotId, IImsDataChannelServiceAvailabilityCallback c){

    }

    protected IImsDataChannelTransport onCreateDataChannelTransport(int slotId, String callId, IImsDataChannelEventListener l) {
      return null;
    }

    protected void onCloseDataChannelTransport(IImsDataChannelTransport t) {

    }


}

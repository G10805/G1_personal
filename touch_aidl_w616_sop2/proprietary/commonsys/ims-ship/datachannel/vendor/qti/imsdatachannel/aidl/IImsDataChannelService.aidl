/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.aidl;

import vendor.qti.imsdatachannel.aidl.IImsDataChannelServiceAvailabilityCallback;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelEventListener;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelTransport;


interface IImsDataChannelService {

    oneway void getAvailability(int slotId, in IImsDataChannelServiceAvailabilityCallback c);
    IImsDataChannelTransport createDataChannelTransport(int slotId, in String callId, in IImsDataChannelEventListener listener);
    oneway void closeDataChannelTransport(in IImsDataChannelTransport t);

}

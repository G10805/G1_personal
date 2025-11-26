/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.aidl;


import vendor.qti.imsdatachannel.aidl.ImsDataChannelMessage;
import vendor.qti.imsdatachannel.aidl.ImsMessageStatusInfo;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelStatusCallback;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelMessageCallback;

interface IImsDataChannelConnection {
    oneway void initialize(in IImsDataChannelStatusCallback c, in IImsDataChannelMessageCallback m);
    oneway void sendMessage(in ImsDataChannelMessage dcMessage);
    oneway void notifyMessageReceived(in ImsMessageStatusInfo msgStatusInfo);
}

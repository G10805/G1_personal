/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.aidl;

import vendor.qti.imsdatachannel.aidl.ImsDataChannelMessage;
import vendor.qti.imsdatachannel.aidl.ImsMessageStatusInfo;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelCommandErrorCode;

oneway interface IImsDataChannelMessageCallback {
    void onMessageReceived(in ImsDataChannelMessage msg);
    void onMessageSendStatus(in ImsMessageStatusInfo msgStatusInfo);
    void onMessageSendCommandError(in ImsDataChannelCommandErrorCode errorCode);
}
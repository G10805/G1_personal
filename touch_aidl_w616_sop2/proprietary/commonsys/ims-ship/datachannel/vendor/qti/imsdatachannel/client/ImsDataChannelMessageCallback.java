/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.client;

import android.os.RemoteException;

import vendor.qti.imsdatachannel.aidl.ImsDataChannelMessage;
import vendor.qti.imsdatachannel.aidl.ImsMessageStatusInfo;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelCommandErrorCode;

public interface ImsDataChannelMessageCallback {
    /**
     * Notifies of an incoming message.
     *
     * @param msg
     *    As defined in ImsDataChannelMessage interface.
     *
     * @return None
     */
    void onMessageReceived(ImsDataChannelMessage msg);

    /**
     * Indicates the status of outgoing message sent.
     *
     * @param msgStatusInfo
     *    As defined in ImsMessageStatusInfo interface.
     *
     * @return None
     */
    void onMessageSendStatus(ImsMessageStatusInfo msgStatusInfo);

    /**
     * Notifies error in Message sent.
     *
     * @param errorCode
     *    As defined in ImsDataChannelCommandErrorCode interface.
     *
     * @return None
     */
    void onMessageSendCommandError(ImsDataChannelCommandErrorCode errorode);

}

/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.client;

import android.os.RemoteException;
import java.util.concurrent.Executor;

import vendor.qti.imsdatachannel.aidl.IImsDataChannelConnection;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelMessage;
import vendor.qti.imsdatachannel.aidl.ImsMessageStatusInfo;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelAttributes;
import vendor.qti.imsdatachannel.client.ImsDataChannelStatusCallback;
import vendor.qti.imsdatachannel.client.ImsDataChannelMessageCallback;

/**
  * Ims Data Channel Connection Interface using which clients
  * can exchange message on data channel
  *
  */
public interface ImsDataChannelConnection {
    /**
     * API to initialize the data channel connection instance and
     * register callbacks. This is first method which needs to be
     * invoked after creation of object.
     *
     * @param cbExecutor Executor thread in which callbacks need to
     *                   be sent.
     *
     *
     * @param c          Callback to update data channel status
     *                   {@link ImsDataChannelStatusCallback}
     *
     *
     * @param m          Callback to update Message received or
     *                    message sent status {@link
     *                    ImsDataChannelMessageCallback}
     *
     * @return           None
     *
     */
    void initialize(Executor cbExecutor, ImsDataChannelStatusCallback c, ImsDataChannelMessageCallback m) throws RemoteException;

    /**
     * API to send the message on given data channel.
     *
     * @param dcMessage   message {@link ImsDataChannelMessage}
     *                    which carrier the message buffer .
     *
     *
     *
     * @return            None
     *
     */
    void sendMessage(ImsDataChannelMessage dcMessage) throws RemoteException;

    /**
     * API to notify if message has not been received successfully
     * by clients.
     *
     * @param msgStatusInfo Message failure status {@link
     *                      ImsMessageStatusInfo} to be filled by
     *                      clients if received message has an error
     *
     *
     * @return           None
     *
     */
    void notifyMessageReceived(ImsMessageStatusInfo msgStatusInfo) throws RemoteException;

    /**
     * API to get data channel attributes given instance is
     * associated with.
     *
     * @param            None
     *
     *
     * @return           Data Channel attributes
     *                   {@link ImsDataChannelAttributes}
     *
     */
    ImsDataChannelAttributes getConnectionAttributes();
}

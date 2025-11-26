/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.client;

import android.os.RemoteException;

import vendor.qti.imsdatachannel.aidl.IImsDataChannelTransport;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelResponse;
import vendor.qti.imsdatachannel.client.ImsDataChannelConnection;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelErrorCode;

/**
  * This interface is used by clients to create data channel
  * connection, respond to data channel request initiated from
  * remote side or close data channel connection when data
  * channel call is terminated.
  */
public interface ImsDataChannelTransport {
    /**
     * API to create application data channel connection during call
     * Possible Responses of this API are
     * {@link ImsDataChannelEventListener#onDataChannelCreated()}
     * if data channel is successfully created or
     * {@link
     * ImsDataChannelEventListener#onDataChannelCommandError()} if
     * API fails or {@link
     * ImsDataChannelEventListener#onDataChannelSetupError()} if API
     * succeeded but data channel creation fails
     *
     *
     * @param dcIdList   List of all data channel Ids
     *
     *
     * @param mXmlContent Given XML schema for list of data dhannels
     *
     *
     *
     * @return           None
     */
    void createDataChannel(String[] dcIdList, String mXmlContent) throws RemoteException;

     /**
     * API to respond to application data channel connection request
     * received during call using API
     * {@link
     * ImsDataChannelEventListener#onDataChannelSetupRequest()}
     * Request can be accepted or rejected by clients.Possible
     * Responses of this API are {@link
     * ImsDataChannelEventListener#onDataChannelCreated()} if data
     * channel is successfully created or {@link
     * ImsDataChannelEventListener#onDataChannelCommandError()} if
     * API fails or {@link
     * ImsDataChannelEventListener#onDataChannelSetupError()} if API
     * succeeded but data channel creation fails
     *
     *
     * @param r             List of acceptance or rejection of
     *                      datachannels received
     *
     *
     * @param mXmlContent   Given XML schema for list of data
     *                      dhannels
     *
     *
     *
     * @return              None
     */
    void respondToDataChannelSetUpRequest(ImsDataChannelResponse[] r, String mXmlContent) throws RemoteException;

    /**
     * API to create data channel transport when call is received by
     * UE or initiated from it.
     *
     *
     * @param dc            list of data channel instances {@link
     *                      ImsDataChannelConnection}
     *
     *
     * @param code          error code due to which data channel is
     *                      closed {@link
     *                      ImsDataChannelEventListener}
     *
     *
     * @param listener       Callback Listener to notify data
     *                       channel connection indications
     *                      {@link ImsDataChannelEventListener}
     *
     *
     * @return               None
     *
     */
    void closeDataChannel(ImsDataChannelConnection[] dc, ImsDataChannelErrorCode code) throws RemoteException;

    /**
     * API to get data channel slot id associated with given data
     * channel transport instance
     *
     * @param              None
     *
     *
     * @return             Slot id of the data channel instance
     *
     */
    int getSlotId();

    /**
     * API to get data channel call id associated with given data
     * channel transport instance
     *
     * @param               None
     *
     *
     * @return              Call id of the data channel instance
     *
     *
     */
    String getCallId();
}

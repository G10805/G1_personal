/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.client;

import android.content.Context;
import android.util.Log;

import java.util.concurrent.Executor;
import android.os.RemoteException;

import vendor.qti.imsdatachannel.client.ImsDataChannelServiceAvailabilityCallback;
import vendor.qti.imsdatachannel.client.ImsDataChannelEventListener;
import vendor.qti.imsdatachannel.client.ImsDataChannelTransport;
import vendor.qti.imsdatachannel.client.ImsDataChannelServiceConnectionCallback;

/**
  * Main Ims Data Channel Service Interface using which clients
  * connect to IMS Data Channel Service, get service avaiability
  * status and create data channel transport when data channel
  * call is initiated
  */
public interface ImsDataChannelServiceManager {
     /**
     * API to initialize the data channel service instance and
     * provide client App context.
     *
     * @param context        Client's App context
     *
     *
     *
     * @param executor       Executor thread in which callbacks would
     *                       be sent.
     *
     *
     * @return               None
     */
    void initialize(Context context, Executor executor);

     /**
     * API to connect to the Ims Data Channel Service and
     * register callback using which clients can be notified service
     * about connection status.
     *
     *
     * @param callback      Callback to update Data Channel service
     *                      status {@link
     *                      ImsDataChannelServiceConnectionCallback}
     *
     *
     * @return               None
     */
    void connectImsDataChannelService(ImsDataChannelServiceConnectionCallback callback);

     /**
     * API to disconnect the client to Ims Data Channel Service
     * Clients would be notified through {@link
     * ImsDataChannelServiceConnectionCallback} when client is
     * disconneced.
     *
     *
     * @param           None
     *
     *
     * @return           None
     */
    void disconnectImsDataChannelService();

     /**
     * API to query service availability and register service
     * availability callback. Clients would be notified through
     * {@link ImsDataChannelServiceAvailabilityCallback} when service
     * is available and ready to use
     *
     * @param slotId        slotId for the service availability
     *
     *
     * @param c             Callback to update Data Channel Service
     *                      availability status {@link
     *                      ImsDataChannelServiceAvailabilityCallback}
     *
     *
     * @return              None
     */
    void getAvailability(int slotId, ImsDataChannelServiceAvailabilityCallback c) throws RemoteException;

    /**
     * API to create data channel transport when call is received by
     * UE or initiated from it.
     *
     *
     * @param slotId        slot Id of the call
     *
     *
     * @param callId        call Id of the call
     *
     *
     * @param listener      Callback Listener to notify data channel
     *                      connection indications {@link
     *                      ImsDataChannelEventListener}
     *
     *
     * @return              Transport instance {@link
     *                      ImsDataChannelTransport}
     */
    ImsDataChannelTransport createDataChannelTransport(int slotId, String callId, ImsDataChannelEventListener listener) throws RemoteException;

    /**
     * API to close data Channel transport. Transport needs to be
     * closed when data channel call ends or call is downgraded from
     * data channel call to voice only call
     *
     *
     * @param t          instance of Data Channel transport
     *                   {@link ImsDataChannelTransport}
     *
     *
     * @return           None
     */
    void closeDataChannelTransport(ImsDataChannelTransport t) throws RemoteException;
}

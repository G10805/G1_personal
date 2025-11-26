/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.client;

import android.content.Context;
import android.util.Log;

/**
  * Callback Interface used to notify clients if it connected or
  * disconnected
  */
public interface ImsDataChannelServiceConnectionCallback {

     /**
     * API to notify client if it's connected as a result of API
     * {@link
     * ImsDataChannelServiceManager#connectImsDataChannelService()}
     * invocation
     *
     *
     * @param            None
     *
     *
     * @return           None
     *
     *
     */
    void onServiceConnected();

    /**
     * API to notify client if it's disconnected as a result of API
     * {@link
     * ImsDataChannelServiceManager#disConnectImsDataChannelService()}
     * invocation
     *
     *
     * @param            None
     *
     *
     * @return           None
     *
     *
     */
    void onServiceDisconnected();
}

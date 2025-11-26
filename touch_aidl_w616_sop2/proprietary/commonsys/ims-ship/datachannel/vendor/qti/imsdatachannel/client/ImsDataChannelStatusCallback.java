/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.client;

import android.os.RemoteException;

import vendor.qti.imsdatachannel.aidl.ImsDataChannelState;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelErrorCode;

public interface ImsDataChannelStatusCallback {
    /**
     * Indicates closure of the datachannel connection.
     *
     * @param code
     *    reasoncode as defined in ImsDataChannelErrorCode interface.
     *
     * @return None
     */
    void onClosed(ImsDataChannelErrorCode code);

    /**
     * Indicates the state, of the datachannel connection.
     *
     * @param dcState
     *    state of the datachannel, as defined in
     *    ImsDataChannelState interface.
     *
     * @return None
     */
    void onStateChange(ImsDataChannelState dcState);
}

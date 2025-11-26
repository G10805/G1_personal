/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.client;

import vendor.qti.imsdatachannel.aidl.ImsDataChannelAttributes;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelState;
import vendor.qti.imsdatachannel.aidl.ImsReasonCode;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelErrorCode;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelCommandErrorCode;
import vendor.qti.imsdatachannel.client.ImsDataChannelConnection;

public interface ImsDataChannelEventListener {
    /**
     * Indicates availability of a data channel which was created unsolicitly.
     * This happens in case of bootstrap datachannel creation and in
     * application data channel creation, during pre-call scenario.
     *
     * @param attr
     *    Attributes of the datachannel created, as defined in
     *    ImsDataChannelAttributes interface.
     *
     * @param dcConnection
     *    DataChannel object as defined in ImsDataChannelConnection interface.
     *
     * @return None
     */
    void onDataChannelAvailable(ImsDataChannelAttributes attr, ImsDataChannelConnection dcConnection);

    /**
     * Notifies incoming datachannel setup request from remote.
     *
     * @param attr
     *    Array of attributes as defined in ImsDataChannelAttributes interface.
     *    Size of the array represents, the number of datachannels that is been
     *    requested from the remote side.
     *
     * @return None
     */
    void onDataChannelSetupRequest(ImsDataChannelAttributes[] attr);

    /**
     * Indicates successful creation of a data channel which was either requested for
     * or responded with.
     *
     * @param attr
     *     Attributes of the datachannel created, as defined in
     *     ImsDataChannelAttributes interface.
     *
     * @param dcConnection
     *    DataChannel object as defined in ImsDataChannelConnection interface.
     *
     * @return None
     */
    void onDataChannelCreated(ImsDataChannelAttributes attr, ImsDataChannelConnection dcConnection);

    /**
     * Notifies error in creation of data channel.
     *
     * @param dcId
     *    Datachannel identifier.
     *
     * @param code
     *    error code as defined in ImsDataChannelErrorCode interface.
     *
     * @return None
     */
    void onDataChannelSetupError(String dcId, ImsDataChannelErrorCode code);

    /**
     * Indicates successful closure of the DataChannel transport.
     *
     * @param reasonCode
     *    As defined in ImsReasonCode interface.
     *
     * @return None
     */
    void onDataChannelTransportClosed(ImsReasonCode reasonCode);

    /**
     * Notifies error in any request APIs as defined in IImsDataChannelTransport
     * interface.
     *
     * @param dcId
     *    DataChannel identifier.
     *
     * @param errorCode
     *    error code as defined in ImsDataChannelCommandErrorCode.
     *
     * @return None
     */
    void onDataChannelCommandError(String dcId, ImsDataChannelCommandErrorCode errorCode);
}

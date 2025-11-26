/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.aidl;

import java.util.List;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelAttributes;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelState;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelConnection;
import vendor.qti.imsdatachannel.aidl.ImsReasonCode;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelErrorCode;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelCommandErrorCode;

interface IImsDataChannelEventListener {
    oneway void onDataChannelAvailable(in ImsDataChannelAttributes attr, in IImsDataChannelConnection dcConnection);
    oneway void onDataChannelSetupRequest(in ImsDataChannelAttributes[] attr);
    oneway void onDataChannelCreated(in ImsDataChannelAttributes attr, in IImsDataChannelConnection dcConnection);
    oneway void onDataChannelSetupError(in String dcId, in ImsDataChannelErrorCode code);
    oneway void onDataChannelTransportClosed(in ImsReasonCode reasonCode);
    oneway void onDataChannelCommandError(in String dcId, in ImsDataChannelCommandErrorCode errorCode);
}

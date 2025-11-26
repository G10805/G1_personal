/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.aidl;

import vendor.qti.imsdatachannel.aidl.ImsDataChannelState;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelErrorCode;

oneway interface IImsDataChannelStatusCallback {
    void onClosed(in ImsDataChannelErrorCode code);
    void onStateChange(in ImsDataChannelState dcState);
}

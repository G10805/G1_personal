/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.aidl;

import vendor.qti.imsdatachannel.aidl.IImsDataChannelConnection;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelResponse;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelErrorCode;

interface IImsDataChannelTransport {
    oneway void createDataChannel(in String[] dcIdList, in String mXmlContent);
    oneway void respondToDataChannelSetUpRequest(in ImsDataChannelResponse[] r, in String mXmlContent);
    oneway void closeDataChannel(in IImsDataChannelConnection[] dc, in ImsDataChannelErrorCode code);
}

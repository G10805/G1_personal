/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.aidl;

oneway interface IImsDataChannelServiceAvailabilityCallback {
    void onAvailable();
    void onUnAvailable();
}
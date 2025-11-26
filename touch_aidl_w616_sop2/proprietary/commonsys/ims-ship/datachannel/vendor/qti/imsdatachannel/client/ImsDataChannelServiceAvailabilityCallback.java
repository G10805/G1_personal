/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.client;

public interface ImsDataChannelServiceAvailabilityCallback {
    /**
     * Notifies DataChannel Service status as Available.
     *
     * @return None
     */
    void onAvailable();

    /**
     * Notifies DataChannel Service status as Unavailable.
     *
     * @return None
     */
    void onUnAvailable();
}

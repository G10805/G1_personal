/*
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlAGnssType;
import vendor.qti.gnss.LocAidlApnType;
import vendor.qti.gnss.ILocAidlAGnssCallback;

@VintfStability
interface ILocAidlAGnss {

    /**
     * Notifies that the AGNSS data connection has been closed.
     *
     * @param agnssType Specifies AGnss type for the data connection
     *
     * @return True if the operation is successful.
     */
    boolean dataConnClosedExt(in LocAidlAGnssType agnssType);

    /**
     * Notifies that a data connection is not available for AGNSS.
     *
     * @param agnssType Specifies AGnss type for the data connection
     *
     * @return True if the operation is successful.
     */
    boolean dataConnFailedExt(in LocAidlAGnssType agnssType);

    /**
     * Notifies that a data connection is available and sets the name of the
     * APN, and its IP type, and the AGnss Type value.
     *
     * @param apn Access Point Name(follows regular APN naming convention).
     * @param apnIpType Specifies if SUPL or C2K.
     * @param agnssType Specifies AGnss type where it can't be derived from
     *                  apnIpType
     *
     * @return True if the operation is successful.
     */
    boolean dataConnOpenExt(in String apn, in LocAidlApnType apnIpType,
        in LocAidlAGnssType agnssType);

    /**
     * Opens the AGNSS interface and provides the callback routines to the
     * implementation of this interface.
     *
     * @param callback Handle to the AGNSS status callback interface.
     */
    void setCallbackExt(in ILocAidlAGnssCallback callback);
}

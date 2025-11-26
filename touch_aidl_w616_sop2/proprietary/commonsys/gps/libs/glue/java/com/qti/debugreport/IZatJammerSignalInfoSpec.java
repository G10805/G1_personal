/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  =============================================================================*/

package com.qti.debugreport;

import android.os.Parcel;
import android.os.Parcelable;
import android.os.Bundle;
import android.util.Log;

/** @addtogroup IZatJammerSignalInfoSpec
  @{ */

/**
 * Class IZatJammerSignalInfoSpec.
 *
 * IZatJammerSignalInfoSpec class containing jammerInd and Agc for IzatSignalType.
 */
public class IZatJammerSignalInfoSpec implements Parcelable {
    private static String TAG = "IZatJammerSignalInfoSpec";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);
    private long mJammerInd;
    private long mAgc;
    private int mSignalId;
    public IZatJammerSignalInfoSpec(long jammerInd, long agc, int signalId) {
        mJammerInd = jammerInd;
        mAgc = agc;
        mSignalId = signalId;
    }

    public IZatJammerSignalInfoSpec(Parcel source) {
        mJammerInd = source.readLong();
        mAgc = source.readLong();
        mSignalId = source.readInt();
    }

    public enum IzatSignalType {
        IZAT_GNSS_SIGNAL_TYPE_GPS_L1CA(0),
        IZAT_GNSS_SIGNAL_TYPE_GPS_L1C(1),
        IZAT_GNSS_SIGNAL_TYPE_GPS_L2C_L(2),
        IZAT_GNSS_SIGNAL_TYPE_GPS_L5_Q(3),
        IZAT_GNSS_SIGNAL_TYPE_GLONASS_G1(4),
        IZAT_GNSS_SIGNAL_TYPE_GLONASS_G2(5),
        IZAT_GNSS_SIGNAL_TYPE_GALILEO_E1_C(6),
        IZAT_GNSS_SIGNAL_TYPE_GALILEO_E5A_Q(7),
        IZAT_GNSS_SIGNAL_TYPE_GALILEO_E5B_Q(8),
        IZAT_GNSS_SIGNAL_TYPE_BEIDOU_B1_I(9),
        IZAT_GNSS_SIGNAL_TYPE_BEIDOU_B1C(10),
        IZAT_GNSS_SIGNAL_TYPE_BEIDOU_B2_I(11),
        IZAT_GNSS_SIGNAL_TYPE_BEIDOU_B2A_I(12),
        IZAT_GNSS_SIGNAL_TYPE_QZSS_L1CA(13),
        IZAT_GNSS_SIGNAL_TYPE_QZSS_L1S(14),
        IZAT_GNSS_SIGNAL_TYPE_QZSS_L2C_L(15),
        IZAT_GNSS_SIGNAL_TYPE_QZSS_L5_Q(16),
        IZAT_GNSS_SIGNAL_TYPE_SBAS_L1_CA(17),
        IZAT_GNSS_SIGNAL_TYPE_NAVIC_L5(18),
        IZAT_GNSS_SIGNAL_TYPE_BEIDOU_B2A_Q(19),
        IZAT_GNSS_SIGNAL_TYPE_BEIDOU_B2B_I(20),
        IZAT_GNSS_SIGNAL_TYPE_BEIDOU_B2B_Q(21),
        IZAT_GNSS_SIGNAL_TYPE_MAX(22);
        private final int mValue;
        private IzatSignalType(int value) {
            mValue = value;
        }
        public int getValue() {
            return mValue;
        }
    };

    /**
     * Gets JammerInd for IzatSignalType.
     *
     * @return jammerInd for IzatSignalType.
     */
    public long getJammerInd() {
        return mJammerInd;
    }

    /**
     * Gets Agc for IzatSignalType.
     * @return Agc for IzatSignalType.
     */
    public long getAgc() {
        return mAgc;
    }

    /**
     * Gets IzatSignalType .
     * @return Signal type.
     */
    public IzatSignalType getSignalType() {
        return IzatSignalType.values()[mSignalId];
    }


    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeLong(mJammerInd);
        dest.writeLong(mAgc);
        dest.writeInt(mSignalId);
    }

    /** @cond */

    public static final Parcelable.Creator<IZatJammerSignalInfoSpec> CREATOR =
        new Parcelable.Creator<IZatJammerSignalInfoSpec>() {
            @Override
                public IZatJammerSignalInfoSpec createFromParcel(Parcel source) {
                    return new IZatJammerSignalInfoSpec(source);
                }
            @Override
                public IZatJammerSignalInfoSpec[] newArray(int size) {
                    return new IZatJammerSignalInfoSpec[size];
                }

            /** @endcond */

        };
};
/** @} */ /* end_addtogroup IZatJammerSignalInfoSpec */

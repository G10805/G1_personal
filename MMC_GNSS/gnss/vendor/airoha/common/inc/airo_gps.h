/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
 * its licensors. Without the prior written permission of Airoha and/or its
 * licensors, any reproduction, modification, use or disclosure of Airoha
 * Software, and information contained herein, in whole or in part, shall be
 * strictly prohibited. You may only use, reproduce, modify, or distribute (as
 * applicable) Airoha Software if you have agreed to and been bound by the
 * applicable license agreement with Airoha ("License Agreement") and been
 * granted explicit permission to do so within the License Agreement ("Permitted
 * User").  If you are not a Permitted User, please cease any access or use of
 * Airoha Software immediately. BY OPENING THIS FILE, RECEIVER HEREBY
 * UNEQUIVOCALLY ACKNOWLEDGES AND AGREES THAT AIROHA SOFTWARE RECEIVED FROM
 * AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN "AS-IS"
 * BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
 * ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
 * SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 * RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
 * LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
 * AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND ANY
 * SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR SUCH
 * AIROHA SOFTWARE AT ISSUE.
 */
/**
 * @file airo_gps.h
 * @brief Airoha GPS Struct
 *
 *
 *
 */
#ifndef _AIRO_GPS_H
#define _AIRO_GPS_H
#include <stdint.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string>
#define SVS_NUM_GPS (32)
#define SVS_NUM_QZS (7)
#define SVS_NUM_GLO (24)
#define SVS_NUM_GAL (36)
#define SVS_NUM_BDS (63)
#define SVS_NUM_NIC (14)
#define SVS_NUM_MAX (SVS_NUM_BDS)
#define SVS_NUM_TOTAL                                                      \
    (SVS_NUM_GPS + SVS_NUM_QZS + SVS_NUM_GLO + SVS_NUM_GAL + SVS_NUM_BDS + \
     SVS_NUM_NIC)
//#include "gnss-base.h" //deprecated
namespace Airoha {
namespace Gnss {
// all types are from hardware/interface/gnss, and add a prefix "Pre"
enum {
    GPS_LOCATION_HAS_LAT_LONG = 1,              // 0x0001
    GPS_LOCATION_HAS_ALTITUDE = 2,              // 0x0002
    GPS_LOCATION_HAS_SPEED = 4,                 // 0x0004
    GPS_LOCATION_HAS_BEARING = 8,               // 0x0008
    GPS_LOCATION_HAS_HORIZONTAL_ACCURACY = 16,  // 0x0010
    GPS_LOCATION_HAS_VERTICAL_ACCURACY = 32,    // 0x0020
    GPS_LOCATION_HAS_SPEED_ACCURACY = 64,       // 0x0040
    GPS_LOCATION_HAS_BEARING_ACCURACY = 128,    // 0x0080
};
enum {
    GNSS_CONSTELLATION_UNKNOWN = 0,
    GNSS_CONSTELLATION_GPS = 1,
    GNSS_CONSTELLATION_SBAS = 2,
    GNSS_CONSTELLATION_GLONASS = 3,
    GNSS_CONSTELLATION_QZSS = 4,
    GNSS_CONSTELLATION_BEIDOU = 5,
    GNSS_CONSTELLATION_GALILEO = 6,
    GNSS_CONSTELLATION_IRNSS = 7,  // by Rain , in latest version android
};
/**
 * @brief Gnss constellation for EPH notification.
 *
 */
enum class AirohaGnssConstellation : uint32_t {
    GPS = 0,
    GLONASS = 1,
    GALILEO = 2,
    BEIDOU = 3,
    QZSS = 4,
};
/**
 * @brief Gnss signal for EPH notification.
 *
 */
enum class AirohaGnssSignal : uint32_t {
    L1 = 0,
    L5 = 1,
};
enum PreGnssSvFlags : uint8_t {
    NONE = 0,
    HAS_EPHEMERIS_DATA = 1 /* 1 << 0 */,
    HAS_ALMANAC_DATA = 2 /* 1 << 1 */,
    USED_IN_FIX = 4 /* 1 << 2 */,
    HAS_CARRIER_FREQUENCY = 8 /* 1 << 3 */,
};
typedef uint8_t PreGnssConstellationType;
/** Extended interface for DEBUG support. */
enum PreSatelliteEphemerisType : uint8_t {
    /** Ephemeris is known for this satellite. */
    GNSS_CONSTELLATION_TYPE_EPH,
    /**
     * Ephemeris is not known, but Almanac (approximate location) is known.
     */
    GNSS_CONSTELLATION_TYPE_ALM,
    /**
     * Both ephemeris & almanac are not known (e.g. during a cold start
     * blind search.)
     */
    GNSS_CONSTELLATION_TYPE_NONE
};
enum PreSatelliteEphemerisSource : uint8_t {
    /**
     * The ephemeris (or almanac only) information was demodulated from the
     * signal received on the device
     */
    GNSS_CONSTELLATION_SRC_DEMODULATED,
    /**
     * The ephemeris (or almanac only) information was received from a SUPL
     * server.
     */
    GNSS_CONSTELLATION_SRC_SUPL_PROVIDED,
    /**
     * The ephemeris (or almanac only) information was provided by another
     * server.
     */
    GNSS_CONSTELLATION_SRC_OTHER_SERVER_PROVIDED,
    /**
     * The ephemeris (or almanac only) information was provided by another
     * method, e.g. injected via a local debug tool, from build defaults
     * (e.g. almanac), or is from a satellite
     * with SatelliteEphemerisType::NOT_AVAILABLE.
     */
    GNSS_CONSTELLATION_SRC_OTHER
};
enum PreSatelliteEphemerisHealth : uint8_t {
    /** The ephemeris is known good. */
    GNSS_SATELLITE_EPH_HEALTH,
    /** The ephemeris is known bad. */
    GNSS_SATELLITE_EPH_UNHEALTH,
    /** The ephemeris is unknown to be good or bad. */
    GNSS_SATELLITE_EPH_UNKNOWN
};
enum {
    GNSS_MAX_SVS_COUNT = 64u,  // 64
};
#define MAX_NMEA_LEN 512
typedef struct NAVInfo {
    float acc;
    // TOOD:
} NavInfo_t;
enum GnssMax : uint32_t {
    /** Maximum number of SVs for gnssSvStatusCb(). */
    SVS_COUNT = 64,
};
/** Maximum number of Measurements in gps_measurement_callback(). */
#define PRE_GPS_MAX_MEASUREMENT 64
/** Maximum number of SVs for gps_sv_status_callback(). */
#define PRE_GNSS_MAX_SVS 64
/** Maximum number of Measurements in gnss_measurement_callback(). */
#define PRE_GNSS_MAX_MEASUREMENT 64
#define PRE_GNSS_MAX_AGC_NUM 6
/* http://androidxref.com/9.0.0_r3/xref/hardware/interfaces/gnss/1.0/types.hal
 */
/** NOTICE: all struct will be rename as Pre_ */
enum PreElapsedRealtimeFlags : uint16_t {
    /** A valid timestampNs is stored in the data structure. */
    HAS_TIMESTAMP_NS = 1 << 0,
    /** A valid timeUncertaintyNs is stored in the data structure. */
    HAS_TIME_UNCERTAINTY_NS = 1 << 1,
};
struct PreElapsedRealtime {
    uint16_t flags = 0;
    uint64_t timestampNs = 0;
    uint64_t timeUncertaintyNs = 0;
};
enum PreGnssAidingDataFlag : uint16_t {
    DELETE_EPHEMERIS = 0x0001,
    DELETE_ALMANAC = 0x0002,
    DELETE_POSITION = 0x0004,
    DELETE_TIME = 0x0008,
    DELETE_IONO = 0x0010,
    DELETE_UTC = 0x0020,
    DELETE_HEALTH = 0x0040,
    DELETE_SVDIR = 0x0080,
    DELETE_SVSTEER = 0x0100,
    DELETE_SADATA = 0x0200,
    DELETE_RTI = 0x0400,
    DELETE_CELLDB_INFO = 0x8000,
    DELETE_ALL = 0xFFFF
};
typedef uint16_t PreGnssAidingData;
struct PreGnssLocation {
    /** Contains GnssLocationFlags bits. */
    uint16_t flags = 0;
    /** Represents latitude in degrees. */
    double latitudeDegrees = 0.0;
    /** Represents longitude in degrees. */
    double longitudeDegrees = 0.0;
    /**
     * Represents altitude in meters above the WGS 84 reference ellipsoid.
     */
    double altitudeMeters = 0.0;
    /** Represents speed in meters per second. */
    float speedMetersPerSec = 0.0;
    /** Represents heading in degrees. */
    float bearingDegrees = 0.0;
    /**
     * Represents expected horizontal position accuracy, radial, in meters
     * (68% confidence).
     */
    float horizontalAccuracyMeters = 0.0;
    /**
     * Represents expected vertical position accuracy in meters
     * (68% confidence).
     */
    float verticalAccuracyMeters = 0.0;
    /**
     * Represents expected speed accuracy in meter per seconds
     * (68% confidence).
     */
    float speedAccuracyMetersPerSecond = 0.0;
    /**
     * Represents expected bearing accuracy in degrees
     * (68% confidence).
     */
    float bearingAccuracyDegrees = 0.0;
    /** Timestamp for the location fix. */
    int64_t timestamp = 0;
    PreElapsedRealtime elapsedTime;
    uint8_t bestLocation = 0;
};
struct PreGnssSvInfo {
    /**
     * Pseudo-random or satellite ID number for the satellite, a.k.a. Space
     * Vehicle (SV), or FCN/OSN number for Glonass. The distinction is made by
     * looking at constellation field. Values must be in the range of:
     *
     * - GNSS:    1-32
     * - SBAS:    120-151, 183-192
     * - GLONASS: 1-24, the orbital slot number (OSN), if known.  Or, if not:
     *            93-106, the frequency channel number (FCN) (-7 to +6) offset
     * by
     *            + 100
     *            i.e. report an FCN of -7 as 93, FCN of 0 as 100, and FCN of +6
     *            as 106.
     * - QZSS:    193-200
     * - Galileo: 1-36
     * - Beidou:  1-37
     */
    int16_t svid = 0;
    /**
     * Defines the constellation of the given SV.
     */
    PreGnssConstellationType constellation = GNSS_CONSTELLATION_UNKNOWN;
    /**
     * Carrier-to-noise density in dB-Hz, typically in the range [0, 63].
     * It contains the measured C/N0 value for the signal at the antenna port.
     *
     * This is a mandatory value.
     */
    float cN0Dbhz = 0.0;
    /** Elevation of SV in degrees. */
    float elevationDegrees = 0.0;
    /** Azimuth of SV in degrees. */
    float azimuthDegrees = 0.0;
    /**
     * Carrier frequency of the signal tracked, for example it can be the
     * GPS central frequency for L1 = 1575.45 MHz, or L2 = 1227.60 MHz, L5 =
     * 1176.45 MHz, varying GLO channels, etc. If the field is not set, it
     * is the primary common use central frequency, e.g. L1 = 1575.45 MHz
     * for GPS.
     *
     * For an L1, L5 receiver tracking a satellite on L1 and L5 at the same
     * time, two GnssSvInfo structs must be reported for this same
     * satellite, in one of the structs, all the values related
     * to L1 must be filled, and in the other all of the values related to
     * L5 must be filled.
     *
     * If the data is available, gnssClockFlags must contain
     * HAS_CARRIER_FREQUENCY.
     */
    int64_t carrierFrequencyHz = 0;
    /**
     * Contains additional data about the given SV.
     */
    uint8_t svFlag = 0;
    double basebandCN0DbHz = 0.0;
};
enum PreGnssPositionMode : uint8_t {
    /**
     * Mode for running GNSS standalone (no assistance).
     */
    STANDALONE = 0,
    /**
     * AGNSS MS-Based mode.
     */
    MS_BASED = 1,
    /**
     * AGNSS MS-Assisted mode. This mode is not maintained by the platform
     * anymore. It is strongly recommended to use MS_BASED instead.
     */
    MS_ASSISTED = 2,
};
enum PreGnssPositionRecurrence : uint32_t {
    /**
     * Receive GNSS fixes on a recurring basis at a specified period.
     */
    RECURRENCE_PERIODIC = 0u,
    /**
     * Request a single shot GNSS fix.
     */
    RECURRENCE_SINGLE = 1u,
};
struct PreGnssPosisionPreference {
    PreGnssPositionMode mode = PreGnssPositionMode::STANDALONE;
    PreGnssPositionRecurrence recurrence =
        PreGnssPositionRecurrence::RECURRENCE_PERIODIC;
    uint32_t minIntervalMs = 1000;
    uint32_t preferredAccuracyMeters = 0;
    uint32_t preferredTimeMs = 0;
    bool lowPowerMode = false;
};
typedef struct NavNmea {
    int64_t timestamp;
    char nmea[MAX_NMEA_LEN];  // assume that nmea no longer than 512 bytes
} NavNmea_t;
typedef struct NavTime {
    int64_t timeMs;
    int64_t timeReferenceMs;
    int32_t uncertaintyMs;
} NavTime_t;
struct PreGnssSvStatus {
    /**
     * Number of GNSS SVs currently visible, refers to the SVs stored in sv_list
     */
    uint32_t numSvs;
    /**
     * Pointer to an array of SVs information for all GNSS constellations,
     * except GNSS, which is reported using svList
     */
    PreGnssSvInfo gnssSvList[0];
};
// ======= Gnss Measurement ======
enum class PreGnssAccumulatedDeltaRangeState : uint16_t {
    ADR_STATE_UNKNOWN = 0,
    ADR_STATE_VALID = 1 /* 1 << 0 */,
    ADR_STATE_RESET = 2 /* 1 << 1 */,
    ADR_STATE_CYCLE_SLIP = 4 /* 1 << 2 */,
    ADR_STATE_HALF_CYCLE_RESOLVED = 8 /* 1 << 3 */,
};
enum class PreGnssClockFlags : uint16_t {
    HAS_LEAP_SECOND = 1 /* 1 << 0 */,
    HAS_TIME_UNCERTAINTY = 2 /* 1 << 1 */,
    HAS_FULL_BIAS = 4 /* 1 << 2 */,
    HAS_BIAS = 8 /* 1 << 3 */,
    HAS_BIAS_UNCERTAINTY = 16 /* 1 << 4 */,
    HAS_DRIFT = 32 /* 1 << 5 */,
    HAS_DRIFT_UNCERTAINTY = 64 /* 1 << 6 */,
};
enum PreGnssMultipathIndicator : uint8_t {
    INDICATOR_UNKNOWN = 0,
    INDICATOR_PRESENT = 1,
    INDICATIOR_NOT_PRESENT = 2,
};
enum PreGnssMeasurementStateBit : uint32_t {
    STATE_UNKNOWN = 0u,
    STATE_CODE_LOCK = 1u /* 1 << 0 */,
    STATE_BIT_SYNC = 2u /* 1 << 1 */,
    STATE_SUBFRAME_SYNC = 4u /* 1 << 2 */,
    STATE_TOW_DECODED = 8u /* 1 << 3 */,
    STATE_MSEC_AMBIGUOUS = 16u /* 1 << 4 */,
    STATE_SYMBOL_SYNC = 32u /* 1 << 5 */,
    STATE_GLO_STRING_SYNC = 64u /* 1 << 6 */,
    STATE_GLO_TOD_DECODED = 128u /* 1 << 7 */,
    STATE_BDS_D2_BIT_SYNC = 256u /* 1 << 8 */,
    STATE_BDS_D2_SUBFRAME_SYNC = 512u /* 1 << 9 */,
    STATE_GAL_E1BC_CODE_LOCK = 1024u /* 1 << 10 */,
    STATE_GAL_E1C_2ND_CODE_LOCK = 2048u /* 1 << 11 */,
    STATE_GAL_E1B_PAGE_SYNC = 4096u /* 1 << 12 */,
    STATE_SBAS_SYNC = 8192u /* 1 << 13 */,
    STATE_TOW_KNOWN = 16384u /* 1 << 14 */,
    STATE_GLO_TOD_KNOWN = 32768u /* 1 << 15 */,
    STATE_2ND_CODE_LOCK = 65536u /* 1 << 16 */,  // add in GnssMeasurement 2.0
};
#define GNSS_MEAS_STATE_1_1_MASK                                               \
    (STATE_UNKNOWN | STATE_CODE_LOCK | STATE_BIT_SYNC | STATE_SUBFRAME_SYNC |  \
     STATE_TOW_DECODED | STATE_MSEC_AMBIGUOUS | STATE_SYMBOL_SYNC |            \
     STATE_GLO_STRING_SYNC | STATE_GLO_TOD_DECODED | STATE_BDS_D2_BIT_SYNC |   \
     STATE_BDS_D2_SUBFRAME_SYNC | STATE_GAL_E1BC_CODE_LOCK |                   \
     STATE_GAL_E1C_2ND_CODE_LOCK | STATE_GAL_E1B_PAGE_SYNC | STATE_SBAS_SYNC | \
     STATE_TOW_KNOWN | STATE_GLO_TOD_KNOWN)
struct PreGnssSignalType {
    uint8_t constellation = GNSS_CONSTELLATION_UNKNOWN;  // GPS L1
    float carrier_frequency_hz = 0.0;                    // 1575420000 HZ
    char codeType;               // TYPE C
};
struct PreGnssClock {
    uint16_t flags = 0;
    int16_t leap_second = 0;
    int64_t time_ns = 0;
    double time_uncertainty_ns = 0.0;
    int64_t full_bias_ns = 0;
    double bias_ns = 0.0;
    double bias_uncertainty_ns = 0.0;
    double drift_nsps = 0.0;
    double drift_uncertainty_nsps = 0.0;
    uint32_t hw_clock_discontinuity_count = 0;
    // add for 2.1
    PreGnssSignalType referenceSignalTypeForIsb;
};
/**
 * Represents a GNSS Measurement, it contains raw and computed information.
 *
 * Independence - All signal measurement information (e.g. sv_time,
 * pseudorange_rate, multipath_indicator) reported in this struct should be
 * based on GNSS signal measurements only. You may not synthesize measurements
 * by calculating or reporting expected measurements based on known or estimated
 * position, velocity, or time.
 */
struct PreGnssMeasurement {
    uint32_t flags;
    int16_t svid;
    uint8_t constellation;
    double time_offset_ns;
    uint32_t state;
    int64_t received_sv_time_in_ns;
    int64_t received_sv_time_uncertainty_in_ns;
    double c_n0_dbhz;
    double baseband_c_no_dbhz;
    double pseudorange_rate_mps;
    double pseudorange_rate_uncertainty_mps;
    uint16_t accumulated_delta_range_state;  // this will be set to V1.1
    double accumulated_delta_range_m;
    double accumulated_delta_range_uncertainty_m;
    float carrier_frequency_hz;
    int64_t carrier_cycles;
    double carrier_phase;
    double carrier_phase_uncertainty;
    uint8_t multipath_indicator;
    double snr_db;
    // add for 2.0
    double agc_level_db;
    char codeType[10];
    // add for 2.1
    double full_inter_signal_bias_ns;
    double full_inter_signal_bias_uncertainty_ns;
    double satellite_inter_signal_bias_ns;
    double satellite_inter_signal_bias_uncertainty_ns;
    // double basebandCN0DbHZ  Same as c_n0_dbhz
};
struct PreGnssAgc {
 public:
    double agcLevelDb = 0.000000;
    uint8_t constellation = GNSS_CONSTELLATION_UNKNOWN;
    int64_t carrierFrequencyHz = 0L;
};
// sizeof(PreGnssMeasurement) == 1;
/**
 * Represents a reading of GNSS measurements. For devices where
 * GnssSystemInfo's year_of_hw is set to 2016+, it is mandatory that these
 * be provided, on request, when the GNSS receiver is searching/tracking
 * signals.
 *
 * - Reporting of GPS constellation measurements is mandatory.
 * - Reporting of all tracked constellations are encouraged.
 */
struct PreGnssData {
    /** Number of measurements. */
    size_t measurement_count = 0;
    /** The array of measurements. */
    PreGnssMeasurement
        measurements[PRE_GNSS_MAX_MEASUREMENT];  // to make the total size
                                                 // is constant
    /** The GPS clock time reading. */
    PreGnssClock clock;
    size_t gnssAgcNum = 0;
    PreGnssAgc gnssAgc[PRE_GNSS_MAX_AGC_NUM];
    PreElapsedRealtime elapsedRealtime;
    bool isFullTracking = false;
};
// char (*__kaboom)[sizeof( PreGnssData )] = 1;
typedef int64_t GpsUtcTime;
/** Represents a location. */
typedef struct {
    /** set to sizeof(GpsLocation) */
    size_t size;
    /** Contains GpsLocationFlags bits. */
    uint16_t flags;
    /** Represents latitude in degrees. */
    double latitude;
    /** Represents longitude in degrees. */
    double longitude;
    /**
     * Represents altitude in meters above the WGS 84 reference ellipsoid.
     */
    double altitude;
    /** Represents speed in meters per second. */
    float speed;
    /** Represents heading in degrees. */
    float bearing;
    /** Represents expected accuracy in meters. */
    float accuracy;
    /** Timestamp for the location fix. */
    GpsUtcTime timestamp;
} PreGpsLocation;
typedef struct {
    bool active;
    int32_t geofenceId;
    double latitudeDegrees;
    double longitudeDegrees;
    double radiusMeters;
} PreGnssGeofenceconfig;
typedef struct {
    int32_t geofencenum;
    int32_t geofenceId[4];
    int32_t transition[4];
    int32_t GeofenceAvailability;
} PreGeofenceData;
typedef struct {
    /**
     * Validity of the data in this struct. False only if no
     * latitude/longitude information is known.
     */
    bool valid;
    /** Latitude expressed in degrees */
    double latitudeDegrees;
    /** Longitude expressed in degrees */
    double longitudeDegrees;
    /** Altitude above ellipsoid expressed in meters */
    float altitudeMeters;
    /** Represents horizontal speed in meters per second. */
    float speedMetersPerSec;
    /** Represents heading in degrees. */
    float bearingDegrees;
    /**
     * Estimated horizontal accuracy of position expressed in meters,
     * radial, 68% confidence.
     */
    double horizontalAccuracyMeters;
    /**
     * Estimated vertical accuracy of position expressed in meters, with
     * 68% confidence.
     */
    double verticalAccuracyMeters;
    /**
     * Estimated speed accuracy in meters per second with 68% confidence.
     */
    double speedAccuracyMetersPerSecond;
    /**
     * estimated bearing accuracy degrees with 68% confidence.
     */
    double bearingAccuracyDegrees;
    /**
     * Time duration before this report that this position information was
     * valid.  This can, for example, be a previous injected location with
     * an age potentially thousands of seconds old, or
     * extrapolated to the current time (with appropriately increased
     * accuracy estimates), with a (near) zero age.
     */
    float ageSeconds;
} PrePositionDebug;
typedef struct {
    /** UTC time estimate. */
    GpsUtcTime timeEstimate;
    /** 68% error estimate in time. */
    float timeUncertaintyNs;
    /**
     * 68% error estimate in local clock drift,
     * in nanoseconds per second (also known as parts per billion - ppb.)
     */
    float frequencyUncertaintyNsPerSec;
} PreTimeDebug;
typedef struct {
    /** Satellite vehicle ID number */
    int16_t svid;
    /** Defines the constellation type of the given SV. */
    PreGnssConstellationType constellation;
    /**
     * Defines the standard broadcast ephemeris or almanac availability for
     * the satellite.  To report status of predicted orbit and clock
     * information, see the serverPrediction fields below.
     */
    PreSatelliteEphemerisType ephemerisType;
    /** Defines the ephemeris source of the satellite. */
    PreSatelliteEphemerisSource ephemerisSource;
    /**
     * Defines whether the satellite is known healthy
     * (safe for use in location calculation.)
     */
    PreSatelliteEphemerisHealth ephemerisHealth;
    /**
     * Time duration from this report (current time), minus the
     * effective time of the ephemeris source (e.g. TOE, TOA.)
     * Set to 0 when ephemerisType is NOT_AVAILABLE.
     */
    float ephemerisAgeSeconds;
    /**
     * True if a server has provided a predicted orbit and clock model for
     * this satellite.
     */
    bool serverPredictionIsAvailable;
    /**
     * Time duration from this report (current time) minus the time of the
     * start of the server predicted information.  For example, a 1 day
     * old prediction would be reported as 86400 seconds here.
     */
    float serverPredictionAgeSeconds;
} PreSatelliteDebug;
typedef struct {
    /** Current best known position. */
    PrePositionDebug position;
    /** Current best know time estimate */
    PreTimeDebug time;
    /**
     * Provides a list of the available satellite data, for all
     * satellites and constellations the device can track,
     * including GnssConstellationType UNKNOWN.
     */
    PreSatelliteDebug satelliteDataArray[SVS_NUM_TOTAL];
} PreDebugData;
enum AirohaPair {
    PAIR_GEOFENCESET_CONFIG = 890,
};
#define MAX_NAV_DATA_LENGTH 40
typedef struct {
    uint8_t svid;
    uint8_t status;
    uint16_t type;
    int16_t message_id;
    int16_t submessage_id;
    int16_t data_length;
    uint8_t data[MAX_NAV_DATA_LENGTH + 1];
} PreGnssNavigationMessage;
enum PreBatchingFlag : uint8_t {
    /**
     * If this flag is set, the hardware implementation
     * must wake up the application processor when the FIFO is full, and
     * call IGnssBatchingCallback to return the locations.
     *
     * If the flag is not set, the hardware implementation must drop
     * the oldest data when the FIFO is full.
     */
    WAKEUP_ON_FIFO_FULL = 0x01
};
struct PreBatchingOptions {
    /**
     * Time interval between samples in the location batch, in nano
     * seconds.
     */
    int64_t periodNanos;
    /**
     * Flags controlling how batching should behave.
     */
    uint8_t flags;
};
typedef uint32_t PreGnssCapabilities;
enum PreGnssCapabilitiesFlags : uint32_t {
    /**
     * GNSS HAL schedules fixes for RECURRENCE_PERIODIC mode.
     * If this is not set, then the framework will use 1000ms for
     * minInterval and will call start() and stop() to schedule the GNSS.
     */
    SCHEDULING = 1u /* 1 << 0 */,
    /**
     * GNSS supports MS-Based AGNSS mode
     */
    MSB = 2u /* 1 << 1 */,
    /**
     * GNSS supports MS-Assisted AGNSS mode
     */
    MSA = 4u /* 1 << 2 */,
    /**
     * GNSS supports single-shot fixes
     */
    SINGLE_SHOT = 8u /* 1 << 3 */,
    /**
     * GNSS supports on demand time injection
     */
    ON_DEMAND_TIME = 16u /* 1 << 4 */,
    /**
     * GNSS supports Geofencing
     */
    GEOFENCING = 32u /* 1 << 5 */,
    /**
     * GNSS supports Measurements for at least GPS.
     */
    MEASUREMENTS = 64u /* 1 << 6 */,
    /**
     * GNSS supports Navigation Messages
     */
    NAV_MESSAGES = 128u /* 1 << 7 */,
    /**
     * GNSS supports low power mode
     */
    LOW_POWER_MODE = 256u /* 1 << 8 */,
    /**
     * GNSS supports blacklisting satellites
     */
    SATELLITE_BLACKLIST = 512u /* 1 << 9 */,
    /**
     * GNSS supports measurement corrections
     */
    MEASUREMENT_CORRECTIONS = 1024u /* 1 << 10 */,
};
enum class PreGnssMeasurementFlags : uint32_t {
    /**
     * A valid 'snr' is stored in the data structure.
     */
    HAS_SNR = 1u /* 1 << 0 */,
    /**
     * A valid 'carrier frequency' is stored in the data structure.
     */
    HAS_CARRIER_FREQUENCY = 512u /* 1 << 9 */,
    /**
     * A valid 'carrier cycles' is stored in the data structure.
     */
    HAS_CARRIER_CYCLES = 1024u /* 1 << 10 */,
    /**
     * A valid 'carrier phase' is stored in the data structure.
     */
    HAS_CARRIER_PHASE = 2048u /* 1 << 11 */,
    /**
     * A valid 'carrier phase uncertainty' is stored in the data structure.
     */
    HAS_CARRIER_PHASE_UNCERTAINTY = 4096u /* 1 << 12 */,
    /**
     * A valid automatic gain control is stored in the data structure.
     */
    HAS_AUTOMATIC_GAIN_CONTROL = 8192u /* 1 << 13 */,
};
typedef uint32_t PreSetIdFlags;
enum class PreSetIdFlag : uint32_t {
    IMSI = 1u /* 1 << 0L */,
    MSISDN = 2u /* 1 << 1L */,
};
enum class PreApnIpType : uint8_t {
    INVALID = 0,
    IPV4 = 1,
    IPV6 = 2,
    IPV4V6 = 3,
};
enum class PreSetIDType : uint8_t {
    NONE = 0,
    IMSI = 1,
    MSISDM = 2,
};
enum PreAGnssType : uint8_t {
    SUPL = 1,
    C2K = 2,
    SUPL_EIMS = 3,
    SUPL_IMS = 4,
};
enum PreAGnssStatusValue : uint8_t {
    /** GNSS requests data connection for AGNSS. */
    REQUEST_AGNSS_DATA_CONN = 1,
    /** GNSS releases the AGNSS data connection. */
    RELEASE_AGNSS_DATA_CONN = 2,
    /** AGNSS data connection initiated */
    AGNSS_DATA_CONNECTED = 3,
    /** AGNSS data connection completed */
    AGNSS_DATA_CONN_DONE = 4,
    /** AGNSS data connection failed */
    AGNSS_DATA_CONN_FAILED = 5
};
struct PreAGnssDataConnectionConfig {
    size_t size;
    PreAGnssType agnssType;
    int32_t ipAddr;
    struct sockaddr_storage addr;
    int isEmergency;
};
enum class PreNfwProtocolStack : uint8_t {
    /**
     * Cellular control plane requests
     */
    CTRL_PLANE = 0,
    /**
     * All types of SUPL requests
     */
    SUPL = 1,
    /**
     * All types of requests from IMS
     */
    IMS = 10,
    /**
     * All types of requests from SIM
     */
    SIM = 11,
    /**
     * Requests from other protocol stacks
     */
    OTHER_PROTOCOL_STACK = 100,
};
/*
 * Entity that is requesting/receiving the location information.
 */
enum class PreNfwRequestor : uint8_t {
    /**
     * Wireless service provider
     */
    CARRIER = 0,
    /**
     * Device manufacturer
     */
    OEM = 10,
    /**
     * Modem chipset vendor
     */
    MODEM_CHIPSET_VENDOR = 11,
    /**
     * GNSS chipset vendor
     */
    GNSS_CHIPSET_VENDOR = 12,
    /**
     * Other chipset vendor
     */
    OTHER_CHIPSET_VENDOR = 13,
    /**
     * Automobile client
     */
    AUTOMOBILE_CLIENT = 20,
    /**
     * Other sources
     */
    OTHER_REQUESTOR = 100,
};
/**
 * GNSS response type for non-framework location requests.
 */
enum class PreNfwResponseType : uint8_t {
    /**
     * Request rejected because framework has not given permission for this use
     * case
     */
    REJECTED = 0,
    /**
     * Request accepted but could not provide location because of a failure
     */
    ACCEPTED_NO_LOCATION_PROVIDED = 1,
    /**
     * Request accepted and location provided
     */
    ACCEPTED_LOCATION_PROVIDED = 2,
};
struct PreGnssMeasurementOptions {
    bool enableFullTracking = false;
    bool enableCorrVecOutputs = false;
    int32_t intervalMs = 0;
};
#if 0
struct PreNfwNotification final {
    /**
     * Package name of the Android proxy application representing the
     * non-framework entity that requested location. Set to empty string if
     * unknown.
     *
     * For user-initiated emergency use cases, this field must be set to empty
     * string and the inEmergencyMode field must be set to true.
     */
    char packageName[128];
    /**
     * Protocol stack that initiated the non-framework location request.
     */
    PreProtocolStack protocalStack;
    /**
     * Name of the protocol stack if protocolStack field is set to
     * OTHER_PROTOCOL_STACK. Otherwise, set to empty string.
     *
     * This field is opaque to the framework and used for logging purposes.
     */
    char otherProtocolStackName[64];
    /**
     * Source initiating/receiving the location information.
     */
    PreNfwRequestor requestor;
    /**
     * Identity of the endpoint receiving the location information. For example,
     * carrier name, OEM name, SUPL SLP/E-SLP FQDN, chipset vendor name, etc.
     *
     * This field is opaque to the framework and used for logging purposes.
     */
    ::android::hardware::hidl_string requestorId __attribute__((aligned(8)));
    /**
     * Indicates whether location information was provided for this request.
     */
    ::android::hardware::gnss::visibility_control::V1_0::
        IGnssVisibilityControlCallback::NfwResponseType responseType
        __attribute__((aligned(1)));
    /**
     * Is the device in user initiated emergency session.
     */
    bool inEmergencyMode __attribute__((aligned(1)));
    /**
     * Is cached location provided
     */
    bool isCachedLocation __attribute__((aligned(1)));
};
#endif
/**
 * @brief Nfw notification struct (Never use memcpy function on this struct)
 * @warning Never use memcpy on this struct
 *
 */
struct PreNfwNotification {
    std::string proxyAppPackageName;
    PreNfwProtocolStack protocolStack =
        PreNfwProtocolStack::OTHER_PROTOCOL_STACK;
    std::string otherProtocolStackName;
    PreNfwRequestor requestor = PreNfwRequestor::OTHER_REQUESTOR;
    std::string requestorId;
    PreNfwResponseType responseType = PreNfwResponseType::REJECTED;
    bool inEmergencyMode = false;
    bool isCachedLocation = false;
};
}  // namespace Gnss
}  // namespace Airoha
namespace airoha {
namespace epo {
// EPO Constellation in airoha
enum EpoConstellation {
    EC_GPS = 0,
    EC_GLONASS = 1,
    EC_GALILEO = 2,
    EC_BEIDOU = 3,
    EC_INVALID = 0xFF,
};
enum EpoFileType {
    EFT_QEPO,
    EFT_EPO,
    EFT_ELPO_3,
    EFT_QELPO,
    EFT_UNKNOWN,
};
struct EpoFileFilter {
    EpoConstellation constellation;
    EpoFileType type;
};
}  // namespace epo
}  // namespace airoha
#endif

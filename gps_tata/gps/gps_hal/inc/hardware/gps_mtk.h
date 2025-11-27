/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_INCLUDE_HARDWARE_GPS_MTK_H
#define ANDROID_INCLUDE_HARDWARE_GPS_MTK_H

#if defined (__ANDROID_OS__)
#include <hardware/gps_internal.h>
#elif defined (__LINUX_OS__)
#include "gps_internal.h"
#endif

__BEGIN_DECLS

// MTK extended GpsAidingData values.
#define GPS_DELETE_HOT_STILL 0x2000
#define GPS_DELETE_EPO      0x4000

// ====================vzw debug screen API =================
/**
 * Name for the VZW debug interface.
 */
#define VZW_DEBUG_INTERFACE      "vzw-debug"

#define VZW_DEBUG_STRING_MAXLEN      200

/** Represents data of VzwDebugData. */
typedef struct {
    /** set to sizeof(VzwDebugData) */
    size_t size;

    char  vzw_msg_data[VZW_DEBUG_STRING_MAXLEN];
} VzwDebugData;


typedef void (* vzw_debug_callback)(VzwDebugData* vzw_message);

/** Callback structure for the Vzw debug interface. */
typedef struct {
    vzw_debug_callback vzw_debug_cb;
} VzwDebugCallbacks;


/** Extended interface for VZW DEBUG support. */
typedef struct {
    /** set to sizeof(VzwDebugInterface) */
    size_t          size;

    /** Registers the callbacks for Vzw debug message. */
    int  (*init)( VzwDebugCallbacks* callbacks );

    /** Set Vzw debug screen enable/disable **/
    void (*set_vzw_debug_screen)(bool enabled);
} VzwDebugInterface;

////////////////////// GNSS HIDL v1.0 ////////////////////////////

/** Represents a location. */
typedef struct {
    GpsLocation legacyLocation;
    /**
    * Represents expected horizontal position accuracy, radial, in meters
    * (68% confidence).
    */
    float           horizontalAccuracyMeters;

    /**
    * Represents expected vertical position accuracy in meters
    * (68% confidence).
    */
    float           verticalAccuracyMeters;

    /**
    * Represents expected speed accuracy in meter per seconds
    * (68% confidence).
    */
    float           speedAccuracyMetersPerSecond;

    /**
    * Represents expected bearing accuracy in degrees
    * (68% confidence).
    */
    float           bearingAccuracyDegrees;

} GpsLocation_ext;


typedef struct {
    GnssSvInfo legacySvInfo;

    /// v1.0 ///
    float carrier_frequency;

} GnssSvInfo_ext;

/**
 * Represents SV status.
 */
typedef struct {
    /** set to sizeof(GnssSvStatus) */
    size_t size;

    /** Number of GPS SVs currently visible, refers to the SVs stored in sv_list */
    int num_svs;
    /**
     * Pointer to an array of SVs information for all GNSS constellations,
     * except GPS, which is reported using sv_list
     */
    GnssSvInfo_ext gnss_sv_list[GNSS_MAX_SVS];

} GnssSvStatus_ext;

/**
 * Callback with location information. Can only be called from a thread created
 * by create_thread_cb.
 */
typedef void (* gps_location_ext_callback)(GpsLocation_ext* location);

/**
 * Callback with SV status information.
 * Can only be called from a thread created by create_thread_cb.
 */
typedef void (* gnss_sv_status_ext_callback)(GnssSvStatus_ext* sv_info);

/**
 * The callback associated with the geofence.
 * Parameters:
 *      geofence_id - The id associated with the add_geofence_area.
 *      location    - The current GPS location.
 *      transition  - Can be one of GPS_GEOFENCE_ENTERED, GPS_GEOFENCE_EXITED,
 *                    GPS_GEOFENCE_UNCERTAIN.
 *      timestamp   - Timestamp when the transition was detected.
 *
 * The callback should only be called when the caller is interested in that
 * particular transition. For instance, if the caller is interested only in
 * ENTERED transition, then the callback should NOT be called with the EXITED
 * transition.
 *
 * IMPORTANT: If a transition is triggered resulting in this callback, the GPS
 * subsystem will wake up the application processor, if its in suspend state.
 */
typedef void (*gps_geofence_transition_ext_callback) (int32_t geofence_id,
        GpsLocation_ext* location, int32_t transition, GpsUtcTime timestamp);

/**
 * The callback associated with the availability of the GPS system for geofencing
 * monitoring. If the GPS system determines that it cannot monitor geofences
 * because of lack of reliability or unavailability of the GPS signals, it will
 * call this callback with GPS_GEOFENCE_UNAVAILABLE parameter.
 *
 * Parameters:
 *  status - GPS_GEOFENCE_UNAVAILABLE or GPS_GEOFENCE_AVAILABLE.
 *  last_location - Last known location.
 */
typedef void (*gps_geofence_status_ext_callback) (int32_t status,
        GpsLocation_ext* last_location);

typedef struct {
    gps_geofence_transition_ext_callback geofence_transition_callback;
    gps_geofence_status_ext_callback geofence_status_callback;
    gps_geofence_add_callback geofence_add_callback;
    gps_geofence_remove_callback geofence_remove_callback;
    gps_geofence_pause_callback geofence_pause_callback;
    gps_geofence_resume_callback geofence_resume_callback;
    gps_create_thread create_thread_cb;
} GpsGeofenceCallbacks_ext;

/** Extended interface for GPS_Geofencing support */
typedef struct {
   /** set to sizeof(GpsGeofencingInterface) */
   size_t          size;

   /**
    * Opens the geofence interface and provides the callback routines
    * to the implementation of this interface.
    */
   void  (*init)( GpsGeofenceCallbacks_ext* callbacks );

   /**
    * Add a geofence area. This api currently supports circular geofences.
    * Parameters:
    *    geofence_id - The id for the geofence. If a geofence with this id
    *       already exists, an error value (GPS_GEOFENCE_ERROR_ID_EXISTS)
    *       should be returned.
    *    latitude, longtitude, radius_meters - The lat, long and radius
    *       (in meters) for the geofence
    *    last_transition - The current state of the geofence. For example, if
    *       the system already knows that the user is inside the geofence,
    *       this will be set to GPS_GEOFENCE_ENTERED. In most cases, it
    *       will be GPS_GEOFENCE_UNCERTAIN.
    *    monitor_transition - Which transitions to monitor. Bitwise OR of
    *       GPS_GEOFENCE_ENTERED, GPS_GEOFENCE_EXITED and
    *       GPS_GEOFENCE_UNCERTAIN.
    *    notification_responsiveness_ms - Defines the best-effort description
    *       of how soon should the callback be called when the transition
    *       associated with the Geofence is triggered. For instance, if set
    *       to 1000 millseconds with GPS_GEOFENCE_ENTERED, the callback
    *       should be called 1000 milliseconds within entering the geofence.
    *       This parameter is defined in milliseconds.
    *       NOTE: This is not to be confused with the rate that the GPS is
    *       polled at. It is acceptable to dynamically vary the rate of
    *       sampling the GPS for power-saving reasons; thus the rate of
    *       sampling may be faster or slower than this.
    *    unknown_timer_ms - The time limit after which the UNCERTAIN transition
    *       should be triggered. This parameter is defined in milliseconds.
    *       See above for a detailed explanation.
    */
   void (*add_geofence_area) (int32_t geofence_id, double latitude, double longitude,
       double radius_meters, int last_transition, int monitor_transitions,
       int notification_responsiveness_ms, int unknown_timer_ms);

   /**
    * Pause monitoring a particular geofence.
    * Parameters:
    *   geofence_id - The id for the geofence.
    */
   void (*pause_geofence) (int32_t geofence_id);

   /**
    * Resume monitoring a particular geofence.
    * Parameters:
    *   geofence_id - The id for the geofence.
    *   monitor_transitions - Which transitions to monitor. Bitwise OR of
    *       GPS_GEOFENCE_ENTERED, GPS_GEOFENCE_EXITED and
    *       GPS_GEOFENCE_UNCERTAIN.
    *       This supersedes the value associated provided in the
    *       add_geofence_area call.
    */
   void (*resume_geofence) (int32_t geofence_id, int monitor_transitions);

   /**
    * Remove a geofence area. After the function returns, no notifications
    * should be sent.
    * Parameter:
    *   geofence_id - The id for the geofence.
    */
   void (*remove_geofence_area) (int32_t geofence_id);
} GpsGeofencingInterface_ext;

typedef struct {
    GnssMeasurement legacyMeasurement;

    /**
     * Automatic gain control (AGC) level. AGC acts as a variable gain
     * amplifier adjusting the power of the incoming signal. The AGC level
     * may be used to indicate potential interference. When AGC is at a
     * nominal level, this value must be set as 0. Higher gain (and/or lower
     * input power) must be output as a positive number. Hence in cases of
     * strong jamming, in the band of this signal, this value must go more
     * negative.
     *
     * Note: Different hardware designs (e.g. antenna, pre-amplification, or
     * other RF HW components) may also affect the typical output of of this
     * value on any given hardware design in an open sky test - the
     * important aspect of this output is that changes in this value are
     * indicative of changes on input signal power in the frequency band for
     * this measurement.
     */
    double agc_level_db;
} GnssMeasurement_ext;

/**
 * Represents a reading of GNSS measurements. For devices where GnssSystemInfo's
 * year_of_hw is set to 2016+, it is mandatory that these be provided, on
 * request, when the GNSS receiver is searching/tracking signals.
 *
 * - Reporting of GPS constellation measurements is mandatory.
 * - Reporting of all tracked constellations are encouraged.
 */
typedef struct {
    /** set to sizeof(GnssData) */
    size_t size;

    /** Number of measurements. */
    size_t measurement_count;

    /** The array of measurements. */
    GnssMeasurement_ext measurements[GNSS_MAX_MEASUREMENT];

    /** The GPS clock time reading. */
    GnssClock clock;
} GnssData_ext;

/**
 * The callback for to report measurements from the HAL.
 *
 * Parameters:
 *    data - A data structure containing the measurements.
 */
typedef void (*gnss_measurement_ext_callback) (GnssData_ext* data);

typedef struct {
    /** set to sizeof(GpsMeasurementCallbacks) */
    size_t size;
    gps_measurement_callback measurement_callback;
    gnss_measurement_ext_callback gnss_measurement_callback;
} GpsMeasurementCallbacks_ext;


/////// Gnss debug ////

/** Milliseconds since January 1, 1970 */
typedef int64_t GnssUtcTime;

typedef enum {
    /** Ephemeris is known for this satellite. */
    EPHEMERIS,
    /**
     * Ephemeris is not known, but Almanac (approximate location) is known.
     */
    ALMANAC_ONLY,
    /**
     * Both ephemeris & almanac are not known (e.g. during a cold start
     * blind search.)
     */
    NOT_AVAILABLE
} SatelliteEphemerisType;

typedef enum {
    /**
     * The ephemeris (or almanac only) information was demodulated from the
     * signal received on the device
     */
    DEMODULATED,
    /**
     * The ephemeris (or almanac only) information was received from a SUPL
     * server.
     */
    SUPL_PROVIDED,
    /**
     * The ephemeris (or almanac only) information was provided by another
     * server.
     */
    OTHER_SERVER_PROVIDED,
    /**
     * The ephemeris (or almanac only) information was provided by another
     * method, e.g. injected via a local debug tool, from build defaults
     * (e.g. almanac), or is from a satellite
     * with SatelliteEphemerisType::NOT_AVAILABLE.
     */
    OTHER
} SatelliteEphemerisSource;

typedef enum {
    /** The ephemeris is known good. */
    GOOD,
    /** The ephemeris is known bad. */
    BAD,
    /** The ephemeris is unknown to be good or bad. */
    UNKNOWN
} SatelliteEphemerisHealth;

/**
 * Provides the current best known position from any
 * source (GNSS or injected assistance).
 */
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
} PositionDebug;

/**
 * Provides the current best known UTC time estimate.
 * If no fresh information is available, e.g. after a delete all,
 * then whatever the effective defaults are on the device must be
 * provided (e.g. Jan. 1, 2017, with an uncertainty of 5 years) expressed
 * in the specified units.
 */
typedef struct {
    /** UTC time estimate. */
    GnssUtcTime timeEstimate;
    /** 68% error estimate in time. */
    float timeUncertaintyNs;
    /**
     * 68% error estimate in local clock drift,
     * in nanoseconds per second (also known as parts per billion - ppb.)
     */
    float frequencyUncertaintyNsPerSec;
} TimeDebug;

/**
 * Provides a single satellite info that has decoded navigation data.
 */
typedef struct {
    /** Satellite vehicle ID number */
    int16_t svid;
    /** Defines the constellation type of the given SV. */
    GnssConstellationType constellation;

    /**
     * Defines the standard broadcast ephemeris or almanac availability for
     * the satellite.  To report status of predicted orbit and clock
     * information, see the serverPrediction fields below.
     */
    SatelliteEphemerisType ephemerisType;
    /** Defines the ephemeris source of the satellite. */
    SatelliteEphemerisSource ephemerisSource;
    /**
     * Defines whether the satellite is known healthy
     * (safe for use in location calculation.)
     */
    SatelliteEphemerisHealth ephemerisHealth;
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
} SatelliteData;

/**
 * Provides a set of debug information that is filled by the GNSS chipset
 * when the method getDebugData() is invoked.
 */
typedef struct {
    /** Current best known position. */
    PositionDebug position;
    /** Current best know time estimate */
    TimeDebug time;
    /**
     * Provides a list of the available satellite data, for all
     * satellites and constellations the device can track,
     * including GnssConstellationType UNKNOWN.
     */
    SatelliteData satelliteDataArray[GNSS_MAX_SVS];
} DebugData;


/** Extended interface for DEBUG support. */
typedef struct {
    /** set to sizeof(GpsDebugInterface) */
    size_t          size;

    /**
     * This function should return any information that the native
     * implementation wishes to include in a bugreport.
     */
    // size_t (*get_internal_state)(char* buffer, size_t bufferSize);
    /// v1.0 ///
    bool (*get_internal_state)(DebugData* debugData);
} GpsDebugInterface_ext;


////////////////////// GNSS HIDL v1.1 ////////////////////////////

/**
 * Callback for reporting driver name information.
 */
typedef void (* gnss_set_name_callback)(const char* name, int length);

/**
 * Callback for requesting framework NLP or Fused location injection.
 */
typedef void (* gnss_request_location_callback)(bool independentFromGnss);

/** New GPS callback structure. */
typedef struct {
    /** set to sizeof(GpsCallbacks) */
    size_t      size;
    gps_location_ext_callback location_cb;
    gps_status_callback status_cb;
    gps_sv_status_callback sv_status_cb;
    gps_nmea_callback nmea_cb;
    gps_set_capabilities set_capabilities_cb;
    gps_acquire_wakelock acquire_wakelock_cb;
    gps_release_wakelock release_wakelock_cb;
    gps_create_thread create_thread_cb;
    gps_request_utc_time request_utc_time_cb;

    gnss_set_system_info set_system_info_cb;
    gnss_sv_status_ext_callback gnss_sv_status_cb;

    /////v1.1////
    gnss_set_name_callback set_name_cb;
    gnss_request_location_callback request_location_cb;
} GpsCallbacks_ext;


/** Represents the standard GPS interface. */
typedef struct {
    /** set to sizeof(GpsInterface) */
    size_t          size;
    /**
     * Opens the interface and provides the callback routines
     * to the implementation of this interface.
     */
    /// v1.0 ///
//    int   (*init)( GpsCallbacks* callbacks );
    /// v1.1 ///
    int   (*init)( GpsCallbacks_ext* callbacks );

    /** Starts navigating. */
    int   (*start)( void );

    /** Stops navigating. */
    int   (*stop)( void );

    /** Closes the interface. */
    void  (*cleanup)( void );

    /** Injects the current time. */
    int   (*inject_time)(GpsUtcTime time, int64_t timeReference,
                         int uncertainty);

    /**
     * Injects current location from another location provider (typically cell
     * ID). Latitude and longitude are measured in degrees expected accuracy is
     * measured in meters
     */
    int  (*inject_location)(double latitude, double longitude, float accuracy);

    /**
     * Specifies that the next call to start will not use the
     * information defined in the flags. GPS_DELETE_ALL is passed for
     * a cold start.
     */
    void  (*delete_aiding_data)(GpsAidingData flags);

    /**
     * min_interval represents the time between fixes in milliseconds.
     * preferred_accuracy represents the requested fix accuracy in meters.
     * preferred_time represents the requested time to first fix in milliseconds.
     *
     * 'mode' parameter should be one of GPS_POSITION_MODE_MS_BASED
     * or GPS_POSITION_MODE_STANDALONE.
     * It is allowed by the platform (and it is recommended) to fallback to
     * GPS_POSITION_MODE_MS_BASED if GPS_POSITION_MODE_MS_ASSISTED is passed in, and
     * GPS_POSITION_MODE_MS_BASED is supported.
     */
     /// v1.0 ///
//    int   (*set_position_mode)(GpsPositionMode mode, GpsPositionRecurrence recurrence,
//            uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time);
    /// v1.1 ///
    int   (*set_position_mode)(GpsPositionMode mode, GpsPositionRecurrence recurrence,
            uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time,
            bool lowPowerMode);


    /** Get a pointer to extension information. */
    const void* (*get_extension)(const char* name);

    /// v1.1 ///
    int  (*inject_fused_location)(double latitude, double longitude, float accuracy);

} GpsInterface_ext;


/**
 * Extended interface for GPS Measurements support.
 */
typedef struct {
    /** Set to sizeof(GpsMeasurementInterface_ext) */
    size_t size;

    /**
     * Initializes the interface and registers the callback routines with the HAL.
     * After a successful call to 'init' the HAL must begin to provide updates at its own phase.
     *
     * Status:
     *    GPS_MEASUREMENT_OPERATION_SUCCESS
     *    GPS_MEASUREMENT_ERROR_ALREADY_INIT - if a callback has already been registered without a
     *              corresponding call to 'close'
     *    GPS_MEASUREMENT_ERROR_GENERIC - if any other error occurred, it is expected that the HAL
     *              will not generate any updates upon returning this error code.
     */
    /// v1.0 ///
//    int (*init) (GpsMeasurementCallbacks* callbacks);
    /// v1.1 ///
    int (*init) (GpsMeasurementCallbacks_ext* callbacks, bool enableFullTracking);

    /**
     * Stops updates from the HAL, and unregisters the callback routines.
     * After a call to stop, the previously registered callbacks must be considered invalid by the
     * HAL.
     * If stop is invoked without a previous 'init', this function should perform no work.
     */
    void (*close) ();

} GpsMeasurementInterface_ext;


/**
 * Interface for passing GNSS configuration contents from platform to HAL.
 */
typedef struct {
    /** Set to sizeof(GnssConfigurationInterface) */
    size_t size;

    /**
     * Deliver GNSS configuration contents to HAL.
     * Parameters:
     *     config_data - a pointer to a char array which holds what usually is expected from
                         file(/etc/gps.conf), i.e., a sequence of UTF8 strings separated by '\n'.
     *     length - total number of UTF8 characters in configuraiton data.
     *
     * IMPORTANT:
     *      GPS HAL should expect this function can be called multiple times. And it may be
     *      called even when GpsLocationProvider is already constructed and enabled. GPS HAL
     *      should maintain the existing requests for various callback regardless the change
     *      in configuration data.
     */
    void (*configuration_update) (const char* config_data, int32_t length);

   //// v1.1 ////
   void (*setBlacklist) (long long* blacklist, int32_t size);
} GnssConfigurationInterface_ext;


#if defined (__ANDROID_OS__)
struct gps_device_t_ext {
    struct hw_device_t common;

    /**
     * Set the provided lights to the provided values.
     *
     * Returns: 0 on succes, error code on failure.
     */
    const GpsInterface_ext* (*get_gps_interface)(struct gps_device_t_ext* dev);
};
#elif defined (__LINUX_OS__)
struct gps_device_t_ext {
    /**
     * Set the provided lights to the provided values.
     *
     * Returns: 0 on succes, error code on failure.
     */
    const GpsInterface_ext* (*get_gps_interface)(struct gps_device_t_ext* dev);
};
#endif

__END_DECLS

#endif /* ANDROID_INCLUDE_HARDWARE_GPS_MTK_H */


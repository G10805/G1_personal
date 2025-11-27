/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/**
 *  @file adr_release_type.h
 *  @brief Defined several data structures for API between framework and library
 *
 *  For ADR API usage, please include "adr_release_api.h" and "adr_release_type.h"
 *
 *  @author Li-Min Lin (mtk07825) <li-min.lin@mediatek.com>
 */
#ifndef INCLUDE_ADR_RELEASE_TYPE_H_
#define INCLUDE_ADR_RELEASE_TYPE_H_

#include <stdbool.h>
#include <stdint.h>

#define ADR_UNUSED(x) (void)(x)

#ifdef __cplusplus
    extern "C" {
#endif

/**
 *  @enum LOG_LEVEL
 *  @brief Refer to Android Log Level
 */
typedef enum {
    L_VERBOSE, L_DEBUG, L_INFO, L_WARN, L_ERROR, L_ASSERT, L_SUPPRESS
} LOG_LEVEL;

/**
 *  @struct adr_sensor_struct
 *  @brief Value in x, y, z, and t
 */
typedef struct {
    double x;
    double y;
    double z;
    double t;
} adr_sensor_struct;

/**
 *  @struct adr_ned_struct
 *  @brief Value in North, East, and Down
 */
typedef struct {
    double n;
    double e;
    double d;
} adr_ned_struct;

/**
 *  @struct adr_llh_struct
 *  @brief Value in Lat, Lon, and Alt
 */
typedef struct {
    double lat;
    double lon;
    double alt;
} adr_llh_struct;

/**
 *  @struct adr_euler_struct
 *  @brief Value in Roll, Pitch, and Yaw
 */
typedef struct {
    double roll;
    double pitch;
    double yaw;
} adr_euler_struct;

typedef struct {
    int pos_valid, vel_valid, heading_valid;
    double ts_sec;
    adr_llh_struct pos;
    adr_llh_struct pos_accuracy;
    adr_ned_struct vel;
    adr_ned_struct vel_accuracy;
    double speed;
    double speed_accuracy;
    double heading;
    double heading_accuracy;
} adr_extra_struct;

/**
 *  @struct adr_ground_truth_struct
 *  @brief Ground truth information
 */
typedef struct {
    double ts_sec;                  ///< UTC time integer part (s)
    int ts_millisec;                ///< UTC time fractional part (ms)
    double lat;                     ///< Latitude (degrees)
    double lon;                     ///< Longitude (degrees)
    float alt;                      ///< Ellipsoid height ref to WGS84 (m)
    adr_ned_struct velocity;        ///< North, East, and Down velocity (m/s)
    double roll;                    ///< Roll (degrees)
    double pitch;                   ///< Pitch (degrees)
    double heading;                 ///< Heading angle (degrees)
} adr_ground_truth_struct;

/**
 *  @struct adr_competitor_struct
 *  @brief Competitor information
 */
typedef struct {
    double ts_sec;                  ///< UTC time integer part (s)
    int ts_millisec;                ///< UTC time fractional part (ms)
    double lat;                     ///< Latitude (degrees)
    double lon;                     ///< Longitude (degrees)
    float alt;                      ///< Ellipsoid height ref to WGS84 (m)
    double h_velocity;              ///< Horizontal velocity (m/s)
    double heading;                 ///< Heading angle (degrees)
    int fix_mode;                   ///< Fix state
} adr_competitor_struct;

/**
 *  @struct adr_mems_msg_struct
 *  @brief MEMS information
 */
typedef struct {
    double ts_sec;                  ///< UTC time integer part (s)
    int ts_millisec;                ///< UTC time fractional part (ms)
    int64_t monotonic_raw;          ///< Raw hardware-based time that is not subject to NTP adjustments
    double ts;                      ///< Calculation time (s)
    adr_sensor_struct acc;          ///< Accelerometer X, Y, and Z directions (m/s)
    adr_sensor_struct gyro;         ///< Gyroscope X, Y, and Z directions (rad/s)
    adr_sensor_struct acc_raw;      ///< Raw accelerometer X, Y, and Z directions (m/s)
    adr_sensor_struct gyro_raw;     ///< Raw gyroscope X, Y, and Z directions (rad/s)
    double can1;                    ///< Vehicle velocity (m/s)
    int can2;                       ///< Vehicle gear
    double can3;                    ///< Rudder angle (degree)
    double wheel_speed[4];          ///< Wheel speed: left-front, right-front, left-rear, right-rear
} adr_mems_msg_struct;

/**
 *  @struct adr_nsv_struct
 *  @brief GNSS raw information
 */
typedef struct {
    int prn;                        ///< Satellite ID (GPS: 1-32, GLONASS: 101-124, BDS: 201-214)
    double ps_range;                ///< Raw pseudo-range measurements (m)
    double carrier_phase;           ///< Raw carrier phase measurement sync to GPS time (cycle)
    int cycle_slip;                 ///< Incremented every time there is a cycle slip on this satellite [0-999]
    double doppler;                 ///< Doppler measurement (Hz)
    int cn0;                        ///< Carrier-to-noise ratio (db-Hz) [0-99]
    int elevation;                  ///< Satellite elevation angle (degrees)
    int azimuth;                    ///< Satellite azimuth angle (degrees)
    int iono;                       ///< Ionosphere Correction value (m)
    bool fixed_flag;                ///< Satellites used for fix (0: not used, 1: used)
} adr_nsv_struct;

/**
 *  @struct adr_gnss_msg_struct
 *  @brief GNSS information
 */
typedef struct {
    double ts_sec;                  ///< UTC time integer part (s)
    int ts_millisec;                ///< UTC time fractional part (ms)
    int64_t monotonic_raw;          ///< Raw hardware-based time that is not subject to NTP adjustments
    double ts;                      ///< Calculation time (s)
    double lat;                     ///< Latitude (degrees)
    double lon;                     ///< Longitude (degrees)
    float alt;                      ///< Ellipsoid height ref to WGS84 (m)
    double baro_hei;                ///< (Extra) Barometer height
    adr_ned_struct velocity;        ///< North, East, and Down velocity (m/s)
    float heading;                  ///< Heading angle (degrees) [0, +2pi]
    int fix_mode;                   ///< Fix state (1: no fix, 2: 2D fix, 3: 3D fix)
    float h_accuracy;               ///< Horizontal estimated position error (m)
    float v_accuracy;               ///< Vertical estimated position error (m)
    adr_ned_struct vel_accuracy;    ///< North, East, and Down velocity accuracy (m/s)
    float heading_accuracy;         ///< Heading angle accuracy (degrees)
    int fix_nsv;                    ///< Number of satellites used for fix
    float pdop;                     ///< Positioning DOP
    float hdop;                     ///< Horizontal DOP
    float vdop;                     ///< Vertical DOP
    float baro;                     ///< Pressure (Pa)
    float baro_t;                   ///< Temperature (degrees Celsius)
    double odom;                    ///< Vehicle velocity (m/s)
    double odom_cal;                ///< (Extra) Calculated vehicle velocity (m/s)
    int nsv;                        ///< Number of satellites
    int max_cn0;                    ///< (Extra) Maximum carrier-to-noise ratio of all satellites in view
    int min_cn0;                    ///< (Extra) Minimum carrier-to-noise ratio of all satellites in view
    int mean_cn0;                   ///< (Extra) Mean carrier-to-noise ratio of all satellites in view
    double odom_change;             ///< (Extra) Vehicle velocity change
    double heading_change;          ///< (Extra) Yaw change
    double feature[112];            ///< (Extra) Learning feature
    double contexture_entropy;      ///< (Extra) Learning prediction entropy
    double contexture_normal_prob;  ///< (Extra) Learning prediction normal probability
    adr_nsv_struct *nsv_ptr;        ///< Satellites measurement
    adr_ground_truth_struct *gt;    ///< (Extra) Ground truth measurement
    adr_competitor_struct *c;       ///< (Extra) Competitor measurement
    adr_extra_struct extra_info;    ///< Extra info: position, velocity, heading
} adr_gnss_msg_struct;

/**
 *  @struct adr_pvt_msg_struct
 *  @brief ADR information
 */
typedef struct {
    double ts_sec;                  ///< UTC time integer part (s), replace RMC and GGA
    int ts_millisec;                ///< UTC time fractional part (ms), replace RMC and GGA
    int64_t monotonic_raw;          ///< Raw hardware-based time that is not subject to NTP adjustments
    double ts;                      ///< Calculation time (s)
    double lat;                     ///< Latitude (degrees and minutes), replace RMC and GGA
    double lon;                     ///< Longitude (degrees and minutes), replace RMC and GGA
    float alt;                      ///< Ellipsoid height ref to WGS84 (m)
    double velocity;                ///< Speed over ground (knots), replace RMC
    double odom;                    ///< Vehicle velocity (m/s)
    double heading;                 ///< Heading angle (degrees) (0, +2pi], replace RMC
    int fix_state;                  ///< Fix state (0: invalid, 1: GNSS only, 2: fusion, 6: DR only), replace GGA

    int pos_init_state;             ///< Position initial state (0: invalid, 1: coarse, 2: fine)
    int vel_init_state;             ///< Velocity initial state (0: invalid, 1: coarse, 2: fine)
    int heading_init_state;         ///< Heading initial state (0: invalid, 1: coarse, 2: fine)
    int mu_state;                   ///< Measurement update state (0: ZUPT, 1: NHC, 2: NHC_GNSS,
                                    ///<                           3: GNSS, 4: GNSSRH, 5: GNSSH, 6: Prediction)
    double epx;                     ///< Longitude error estimate in meters, 95% confidence
    double epy;                     ///< Latitude error estimate in meters, 95% confidence
    double epv;                     ///< Estimated vertical error in meters, 95% confidence
    double climb;                   ///< Climb (positive) or sink (negative) rate, meters per second
    double epd;                     ///< Direction error estimate in degrees, 95% confidence
    double eps;                     ///< Speed error estimate in meters/sec, 95% confidence
    double epc;                     ///< Climb/sink error estimate in meters/sec, 95% confidence
} adr_pvt_msg_struct;

/**
 *  @struct mnl2adr_msg_struct
 *  @brief MNL to ADR information
 */
typedef struct {
    double  latitude[2];            ///< latitude in radian
    double  longitude[2];           ///< longitude in radian
    double  altitude[2];            ///< altitude in meters above the WGS 84 reference ellipsoid
    float   KF_velocity[3];         ///< Kalman Filter velocity in meters per second under (N,E,D) frame
    float   LS_velocity[3];         ///< Least Square velocity in meters per second under (N,E,D) frame
    float   HACC;                   ///< position horizontal accuracy in meters
    float   VACC;                   ///< position vertical accuracy in meters
    float   KF_velocitySigma[3];    ///< Kalman Filter velocity one sigma error in meter per second under (N,E,D) frame
    float   LS_velocitySigma[3];    ///< Least Square velocity one sigma error in meter per second under (N,E,D) frame
    float   HDOP;                   ///< horizontal dilution of precision value in unitless
    float   confidenceIndex[3];     ///< GPS confidence index
    unsigned int   gps_sec;         ///< Timestamp of GPS location
    unsigned int   leap_sec;        ///< correct GPS time with phone kernel time
} mnl2adr_msg_struct;

/**
 *  @struct adr2mnl_msg_struct
 *  @brief ADR to MNL information
 */
typedef struct {
    double  latitude;               ///< latitude in radian
    double  longitude;              ///< longitude in radian
    double  altitude;               ///< altitude in meters above the WGS 84 reference ellipsoid
    float   velocity[3];            ///< SENSOR velocity in meters per second under (N,E,D) frame
    float   acceleration[3];        ///< SENSOR acceleration in meters per second^2 under (N,E,D) frame
    float   HACC;                   ///< position horizontal accuracy in meters
    float   VACC;                   ///< position vertical accuracy in meters
    float   velocitySigma[3];       ///< SENSOR velocity one sigma error in meter per second under (N,E,D) frame
    float   accelerationSigma[3];   ///< SENSOR acceleration one sigma error in meter per second^2 under (N,E,D) frame
    float   bearing;                ///< SENSOR heading in radian UNDER (N,E,D) frame
    float   confidenceIndex[3];     ///< SENSOR confidence index [Note: [0] heading error confidence, others disable]
    float   barometerHeight;        ///< barometer height in meter
    int     valid_flag[4];          ///< SENSOR AGMB hardware valid flag [Note: disable]
    int     staticIndex;            ///< AR status [static, move, uncertain], [0,1,99]
    unsigned long long timestamp;   ///< Timestamp of SENSOR location [Note: disable]
} adr2mnl_msg_struct;

/**
 *  @struct adr_acc_config_struct
 *  @brief Accelerometer configuration
 */
typedef struct {
    int valid;                      ///< Valid or not (0: invalid, 1: valid)
    double a_d_sigma;               ///< Standard deviation of Markov correlated bias (m/s^2)
    double a_w_sigma;               ///< Standard deviation of white noise (m/s^2/sqrt(Hz))
    double tau_a;                   ///< Correlation time or time constant of the Markov correlated bias (s)
    double a_zero_pt_offset;        ///< Zero-point offset (m/s^2)
    double a_offset_sigma;          ///< Offset variation (m/s^2)
    int a_x_axis;                   ///< 1:x, 2:y, 3:z, -1:-x, -2:-y, -3:-z
    int a_y_axis;                   ///< 1:x, 2:y, 3:z, -1:-x, -2:-y, -3:-z
    int a_z_axis;                   ///< 1:x, 2:y, 3:z, -1:-x, -2:-y, -3:-z
} adr_acc_config_struct;

/**
 *  @struct adr_gyro_config_struct
 *  @brief Gyroscope configuration
 */
typedef struct {
    int valid;                      ///< Valid or not (0: invalid, 1: valid)
    double g_d_sigma;               ///< Standard deviation of Markov correlated bias (rad/s)
    double g_w_sigma;               ///< Standard deviation of white noise (rad/s/sqrt(Hz))
    double tau_g;                   ///< Correlation time or time constant of the Markov correlated bias (s)
    double g_zero_pt_offset;        ///< Zero-point offset (rad/s)
    double g_offset_sigma;          ///< Offset variation (rad/s)
    double g_z_neg_sf;              ///< z axis negative scale factor
    double g_z_pos_sf;              ///< z axis positive scale factor
    int g_x_axis;                   ///< 1:x, 2:y, 3:z, -1:-x, -2:-y, -3:-z
    int g_y_axis;                   ///< 1:x, 2:y, 3:z, -1:-x, -2:-y, -3:-z
    int g_z_axis;                   ///< 1:x, 2:y, 3:z, -1:-x, -2:-y, -3:-z
} adr_gyro_config_struct;

/**
 *  @struct adr_odom_config_struct
 *  @brief Odometer configuration
 */
typedef struct {
    int valid;                      ///< Valid or not (0: invalid, 1: valid)
    double scale_factor;            ///< Scale factor
} adr_odom_config_struct;

/**
 *  @struct adr_lever_arm_config_struct
 *  @brief Lever-arm configuration
 */
typedef struct {
    int valid;
    double x;                       ///< vframe gnss_to_mems, m
    double y;                       ///< vframe gnss_to_mems, m
    double z;                       ///< vframe gnss_to_mems, m
} adr_lever_arm_config_struct;

/**
 *  @struct adr_alignment_config_struct
 *  @brief Alignment configuration
 */
typedef struct {
    int valid;
    double roll;                    ///< vframe_to_bframe, degree
    double pitch;                   ///< vframe_to_bframe, degree
    double yaw;                     ///< vframe_to_bframe, degree
} adr_alignment_config_struct;

/**
 *  @struct adr_freq_config_struct
 *  @brief Frequency configuration
 */
typedef struct {
    int valid;
    int gnss_freq;
    int mems_freq;
    int output_freq;
} adr_freq_config_struct;

/**
 *  @struct adr_turn_on_config_struct
 *  @brief Turn-on configuration
 */
typedef struct {
    int valid;
    char turn_on_path[50];
} adr_turn_on_config_struct;

/**
 *  @struct adr_algo_config_struct
 *  @brief Algo configuration
 */
typedef struct {
    int valid;
    int enable_feedback;
} adr_algo_config_struct;

/**
 *  @struct adr_user_param_struct
 *  @brief User defined parameter in configuration file
 */
typedef struct {
    adr_acc_config_struct acc_config;               ///< Accelerometer configuration
    adr_gyro_config_struct gyro_config;             ///< Gyroscope configuration
    adr_odom_config_struct odom_config;             ///< Odometer configuration
    adr_lever_arm_config_struct lever_arm_config;   ///< Lever-arm configuration
    adr_alignment_config_struct alignment_config;   ///< Alignment configuration
    adr_freq_config_struct freq_config;             ///< Frequency configuration
    adr_turn_on_config_struct turn_on_config;       ///< Turn-on configuration
    adr_algo_config_struct algo_config;             ///< Algo configuration
} adr_user_param_struct;

/**
 *  @struct adr_state_data_struct
 *  @brief last state information for TTFF
 */
typedef struct {
    int valid;                     ///< 0: invalid, 1: valid
    double ts_sec;                 ///< UTC time integer part (s)
    int ts_millisec;               ///< UTC time fractional part (ms)
    double state[9];               ///< last state data (position, velocity, attitude)
    double P[9];                   ///< last P
} adr_state_data_struct;

/**
 *  @struct adr_odom_data_struct
 *  @brief last odometer for TTFF
 */
typedef struct {
    int valid;                      ///< Valid or not (0: invalid, 1: valid)
    double scale_factor;            ///< Scale factor
} adr_odom_data_struct;

/**
 *  @struct adr_alignment_data_struct
 *  @brief last state information for TTFF
 */
typedef struct {
    int valid;                      ///< 0: invalid, 1: valid
    double roll;                    ///< deg
    double pitch;                   ///< deg
    double yaw;                     ///< deg
} adr_alignment_data_struct;

typedef struct {
    int valid;                      ///< 0: invalid, 1: valid
    double gyro_bias_x[13];         ///< rad/s, domain [-40 degC ~ 90 degC), inteval 10 degC
    double gyro_bias_y[13];         ///< rad/s, domain [-40 degC ~ 90 degC), inteval 10 degC
    double gyro_bias_z[13];         ///< rad/s, domain [-40 degC ~ 90 degC), inteval 10 degC
    int count[13];                  ///< max number is 100
} adr_gyro_bias_data_struct;

/**
 *  @struct adr_user_archive_struct
 *  @brief User turn-on information
 */
typedef struct {
    adr_state_data_struct state_data;           ///< last state information
    adr_odom_data_struct odom_data;             ///< last odom information
    adr_alignment_data_struct alignment_data;   ///< last alignment information
    adr_gyro_bias_data_struct gyro_bias_data;   ///< last gyro bias information
} adr_user_archive_struct;

#ifdef __cplusplus
    }
#endif

#endif  // INCLUDE_ADR_RELEASE_TYPE_H_

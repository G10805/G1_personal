////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __IS_INTERFACE_H__
#define __IS_INTERFACE_H__

#include <stdint.h>
#include <stdbool.h>
#include "NcLibWarpCommonDef.h"

#define IS_API_VERSION_MAJOR        1   /**< Major API version number */
#define IS_API_VERSION_MINOR        1   /**< Minor API version number */
#define IS_API_VERSION_STEP         0   /**< Step API version number */

#if defined(__GNUC__)
#define EIS_VISIBILITY_PUBLIC __attribute__ ((visibility ("default")))
#define EIS_VISIBILITY_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define EIS_VISIBILITY_PUBLIC __declspec(dllexport)
#define EIS_VISIBILITY_LOCAL
#endif // defined(__GNUC__)


/**  @addtogroup Return_Values
  *  @brief     IS algorithm return values
  *  @{
  */
#define IS_RET_BASE                 0x08000000                  /**< @return_val Base value for all IS related errors and warnings */
#define IS_RET_SUCCESS              (0)                         /**< @return_val Success return value */

#define IS_RET_ERROR_BASE           (IS_RET_BASE)               /**< @return_val Error base value - algorithm cannot recover */

#define IS_RET_FRAME_NOT_PROCESSES  (IS_RET_ERROR_BASE + 1)     /**< @return_val Frame was not processed */
#define IS_RET_GENERAL_ERROR        (IS_RET_ERROR_BASE + 2)     /**< @return_val General error return value */
#define IS_RET_INVALID_INPUT        (IS_RET_ERROR_BASE + 3)     /**< @return_val Invalid input */
#define IS_RET_OUT_OF_MEMORY        (IS_RET_ERROR_BASE + 4)     /**< @return_val Out of memory error */

#define IS_RET_WARN_BASE            (IS_RET_BASE | 0x100)       /**< @return_val Warning base value - algorithm can recover but
                                                                  *  results could be sub-optimal
                                                                  */

#define IS_RET_WARN_INVALID_INPUT   (IS_RET_WARN_BASE + 1)      /**< @return_val Invalid input */
/** @} */

/**  @addtogroup    Errors_Information
  *  @brief         IS algorithm output errors and warnings enumerators and information
  *  @{
  */

/**  @addtogroup    Critical_Errors
 *  @brief          Critical errors. Algorithm cannot recover. API will return with the appropriate error code.
 *                  Values range: 0000-0511
 *  @{
 */

 /** \error ERROR IS0001: Internal assertion failed */
#define IS_ERR_ASSERTION                    "IS0001"

 /** \error ERROR IS0002: General error occurred */
#define IS_ERR_GENERAL_ERROR                "IS0002"

 /** \error ERROR IS0003: Invalid input was detected */
#define IS_ERR_INVALID_INPUT                "IS0003"

/** \error ERROR IS0004: Memory allocation failed */
#define IS_ERR_OUT_OF_MEMORY                "IS0004"

/** \error ERROR IS0005: Invalid filter status was detected */
#define IS_ERR_INVALID_FILTER_STATUS        "IS0005"

/** \error ERROR IS0006: Invalid initial FPS values */
#define IS_ERR_INVALID_INPUT_FPS            "IS0006"

/** \error ERROR IS0007: Out of Order Start for Frame timestamp was detected */
#define IS_ERR_OOO_SOF                      "IS0007"

/** \error ERROR IS0008: Duplicated frame was detected */
#define IS_ERR_DUPLICATED_FRAMES            "IS0008"

/** \error ERROR IS0009: Actual Gyro rate exceeds the requested gyro rate, could cause buffer overflow */
#define IS_ERR_GYRO_RATE_EXCEEDS            "IS0009"

/** \error ERROR IS0010: Invalid IFE input data, struct might be un-initialized */
#define IS_ERR_IFE_INVALID_INIT             "IS0010"

/** \error ERROR IS0011: Invalid IFE input data. Region of interest might be outside of the image */
#define IS_ERR_IFE_INVALID_ROI              "IS0011"

/** \error ERROR IS0012: Out of Order frame ID was detected */
#define IS_ERR_OOO_FRAME_ID                 "IS0012"

/** \error ERROR IS0013: Invalid SAT matrix was detected */
#define IS_ERR_SAT_INVALID                  "IS0013"

/** \error ERROR IS0014: Invalid SAT matrix was detected */
#define IS_ERR_SENSOR_ID_INVALID            "IS0014"

/** \error ERROR IS0015: Invalid input, configuration mismatch. Gyro based MCTF is enabled but output structs are not passed as input */
#define IS_ERR_INVALID_INPUT_MCTF           "IS0015"

/** \error ERROR IS0016: Invalid input, configuration mismatch. Pointer structs are NULL */
#define IS_ERR_INVALID_INPUT_STAB_TRANSFORM "IS0016"

/** \error ERROR IS0017: Invalid input, configuration mismatch. Pointer structs are NULL */
#define IS_ERR_INVALID_INIT_INPUT           "IS0017"

/** \error ERROR IS0018: Gyro missing samples */
#define IS_ERR_GYRO_MISSING_MANY_SAMPLES    "IS0018"

/** @} */

/**  @addtogroup    ISQ_Warnings
*  @brief           Image Stabilization Quality warnings. Algorithm can recover, however degradation in stabilization quality is expected.
*                   Values range: 0512-1023
*  @{
*/

/** \error ERROR IS0512: Invalid rolling shutter skew value, is equal to zero */
#define IS_ERR_RSS_ZERO                     "IS0512"

/** \error ERROR IS0513: Invalid rolling shutter skew value, exceed frame time */
#define IS_ERR_RSS_EXCEEDS                  "IS0513"

/** \error ERROR IS0514: Invalid filter values are detected, sub-optimal stabilization could occur */
#define IS_ERR_INVALID_FILTER_VALUES        "IS0514"

/** \error ERROR IS0515: Invalid SOF value */
#define IS_ERR_SOF_INVALID                  "IS0515"

/** \error ERROR IS0516: Invalid frame time value */
#define IS_ERR_FRAME_TIME_INVALID           "IS0516"

/** \error ERROR IS0517: Invalid exposure time value */
#define IS_ERR_EXPOSURE_TIME_INVALID        "IS0517"

/** \error ERROR IS0518: Estimated gyro rate is not close to mean gyro rate. Might indicate on missing gyro samples or false gyro time-stamps */
#define IS_ERR_GYRO_RATE_EST_VS_AVG         "IS0518"

/** \error ERROR IS0519: Estimated gyro rate is not close to initial value. Might indicate on false HW configuration */
#define IS_ERR_GYRO_RATE_EST_VS_INIT        "IS0519"

/** \error ERROR IS0521: Gyro a missing sample */
#define IS_ERR_GYRO_MISSING_SAMPLE          "IS0521"

/** \error ERROR IS0522: Gyro missing samples from beginning of frame */
#define IS_ERR_GYRO_FIRST_MISSING_SAMPLE    "IS0522"

/** \error ERROR IS0523: Gyro missing samples from beginning of frame */
#define IS_ERR_GYRO_LAST_MISSING_SAMPLE     "IS0523"

/** \error ERROR IS0524: Out of Order gyro sample */
#define IS_ERR_GYRO_OOO                     "IS0524"

/** \error ERROR IS0525: Duplicated gyro sample */
#define IS_ERR_GYRO_DUPLICATED              "IS0525"

/** \error ERROR IS0526: Gyro samples spacing is too big with respect to estimated frequency. Might indicate on missing gyro samples or false gyro time-stamps */
#define IS_ERR_GYRO_SPACING                 "IS0526"

/** \error ERROR IS0527: IFE Region of Interest is not centered */
#define IS_ERR_IFE_ROI_NOT_CENTERED         "IS0527"

/** \error ERROR IS0528: IPE Region of Interest is not centered */
#define IS_ERR_IPE_ROI_NOT_CENTERED         "IS0528"

/** @} */


/**  @addtogroup    Verbose_Information
*  @brief           Additional information that might indicate on use-cases that might affect the stabilized video
*                   Values range: 1024-1535
*  @{
*/

/** \error VERBOSE IS1024: Pause-Resume use-case was detected */
#define IS_VRB_PAUSE_RESUME                 "IS1024"

/** \error VERBOSE IS1025: Invalid input was passed to algorithm destroy function. Could indicate on invalid SW flow */
#define IS_VRB_DESTROY_INVALID_INPUT        "IS1025"

/** \error VERBOSE IS1026: Input has some configurations which are sub-optimal */
#define IS_VRB_INPUT_INFO                   "IS1026"

/** \error VERBOSE IS1027: General debug information */
#define IS_VRB_DEBUG_INFO                   "IS1027"

/** \error VERBOSE IS1028: Frame drop was detected, stabilized video could look not smooth */
#define IS_VRB_FRAME_DROP                   "IS1028"


/** @} */

/** @} */

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/*         Chromatix data structures                                         */
/*****************************************************************************/

/** EIS general configurations */
typedef struct eis_general_s
{
    /**< Minimal total margins ratio for x axis (physical + virtual), w.r.t. input image size and represent sum of both sides.
     *   format: float
     */
    float minimal_total_margin_x;

    /**< Minimal total margins ratio for y axis (physical + virtual), w.r.t. input image size and represent sum of both sides.
    *   format: float
    */
    float minimal_total_margin_y;

    /**< Focal length in pixel units for 1080p. This value is scaled for other resolutions.
     *   format: 32u
     */
    uint32_t focal_length;

    /**< Gyro sampling frequency in Hz
     *   format: 16u
     */
    uint32_t gyro_frequency;

    /**< Gyro noise floor.
     *   format: float
     */
    float gyro_noise_floor;

    /**< Reserved parameter 1
     *   format: float
     */
    float res_param_1;

    /**< Reserved parameter 2
     *   format: float
     */
    float res_param_2;

    /**< Reserved parameter 3
     *   format: float
     */
    float res_param_3;

    /**< Reserved parameter 4
     *   format: float
     */
    float res_param_4;

    /**< Reserved parameter 5
     *   format: float
     */
    float res_param_5;

    /**< Reserved parameter 6
     *   format: float
     */
    float res_param_6;

    /**< Reserved parameter 7
     *   format: float
     */
    float res_param_7;

    /**< Reserved parameter 8
     *   format: float
     */
    float res_param_8;

    /**< Reserved parameter 9
     * format: float
     */
    float res_param_9;

    /**< Reserved parameter 10
     *   format: float
     */
    float res_param_10;

    /**< Reserved LUT parameter 1
     *   format: float
     */
    float res_lut_param_1[32];

    /**< Reserved LUT parameter 2
     *   format: float
     */
    float res_lut_param_2[32];

    /**<  Reserved LUT parameter 3
     *    format: 32u
     */
    uint32_t res_lut_param_3[16];
} eis_general;

/** EIS timing related configurations */
typedef struct eis_timing_s
{
    /**< Offset to adjust the 3D shake gyro interval start/end times to better align with the frame. (Offset between gyro timing and SOF timing in microseconds)
     *   format: 32s
     */
    int32_t s3d_offset_1;

    /**< Offset to adjust the 3D shake gyro interval start/end times to better align with the frame. (Offset between gyro timing and SOF timing in microseconds)
     *   format: 32s
     */
    int32_t s3d_offset_2;

    /**< Offset to adjust the 3D shake gyro interval start/end times to better align with the frame. (Offset between gyro timing and SOF timing in microseconds)
     *   format: 32s
     */
    int32_t s3d_offset_3;

    /**<  Offset to adjust the 3D shake gyro interval start/end times to better align with the frame. (Offset between gyro timing and SOF timing in microseconds)
     *    format: 32s
     */
    int32_t s3d_offset_4;

    /**< Threshold whereby exposure times above and below this threshold results in a different offset getting applied to the 3D shake gyro time interval.
     *   Thresholding for deciding s3d_offset based on integration time in seconds)
     *   format: float
     */
    float s3d_threshold_1;

    /**< Threshold whereby exposure times above and below this threshold results in a different offset getting applied to the 3D shake gyro time interval.
     *   Thresholding for deciding s3d_offset based on integration time in seconds)
     *   format: float
     */
    float s3d_threshold_2;

    /**< Threshold whereby exposure times above and below this threshold results in a different offset getting applied to the 3D shake gyro time interval.
     *   Thresholding for deciding s3d_offset based on integration time in seconds)
     *   format: float
     */
    float s3d_threshold_3;

    /**< Threshold whereby exposure times above and below this threshold results in a different offset getting applied to the 3D shake gyro time interval.
     *   Thresholding for deciding s3d_offset based on integration time in seconds)
     *   format: float
     */
    float s3d_threshold_4_ext;
} eis_timing;

/** EIS blur masking configurations */
typedef struct eis_blur_masking_s
{
    /**< Enable blur masking mechanism
     *   format: 1u
     */
    bool enable;

    /**< Minimum strength EIS can go down to when estimating strong blur
     *   format: float
     */
    float min_strength;

    /**< if (exp_time > exposure_time_th) then blur masking feature is enabled.
     *   Otherwise disabled
     *   Units are seconds
     *   format: float
     */
    float exposure_time_th;

    /**< Blur below this point won't decrease strength. Units are pixels out of 1920 resolutions.
      *  Blur above this point will cause min_strength stabilization (in between start/end interpolate) Units are pixels out of 1920 resolutions.
      *  If (end_decrease_at_blur>start_decrease_at_blur) then feature will be disabled.
      *  format: float
      */
    float start_decrease_at_blur;

    /**< Blur below this point won't decrease strength. Units are pixels out of 1920 resolutions.
      *  Blur above this point will cause min_strength stabilization (in between start/end interpolate) Units are pixels out of 1920 resolutions.
      *  If (end_decrease_at_blur>start_decrease_at_blur) then feature will be disabled.
      *  format: float
      */
    float end_decrease_at_blur;

    /**< blur_masking_res1
     *   format: float
     */
    float blur_masking_res1;

    /**< blur_masking_res2
     *   format: float
     */
    float blur_masking_res2;
} eis_blur_masking;

/** Chromatix configuration data structure */
typedef struct is_chromatix_info_s
{
    eis_general         general;            /**< General Chromatix configuration struct */
    eis_timing          timing;             /**< Timing related Chromatix configuration struct */
    eis_blur_masking    blur_masking;       /**< Blur masking related Chromatix configuration struct */
} is_chromatix_info_t;

/*****************************************************************************/
/*         EIS Algorithm data structures                                     */
/*****************************************************************************/

/** Frame time data structure */
typedef struct frame_times_s
{
    uint64_t sof;             /**< Start of frame in microseconds */
    uint32_t frame_time;      /**< Total frame time in microseconds */
    float exposure_time;      /**< Exposure time in seconds */
} frame_times_t;

/** 3x3 matrix */
typedef double mat3x3[3][3];

/** Location of camera enumerator */
typedef enum CameraPosition_t
{
    CAM_REAR = 0,           /**< Rear camera */
    CAM_FRONT = 1,          /**< front camera */
    CAM_REAR_AUX = 2,       /**< secondary rear camera */
    CAM_FRONT_AUX = 3,      /**< secondary front camera */
    CAM_EXTERNAL = 4        /**< @deprecated */
} CameraPosition;

/** EIS algorithm type */
typedef enum cam_is_type_e
{
    IS_TYPE_NONE,                           /**< @deprecated */
    IS_TYPE_DIS,                            /**< @deprecated */
    IS_TYPE_GA_DIS,                         /**< @deprecated */
    IS_TYPE_EIS_1_0,                        /**< @deprecated */
    IS_TYPE_EIS_2_0,                        /**< EIS 2.x */
    IS_TYPE_EIS_NFL = IS_TYPE_EIS_2_0,      /**< EIS Non-Future looking */
    IS_TYPE_EIS_3_0,                        /**< EIS 3.x */
    IS_TYPE_EIS_FL = IS_TYPE_EIS_3_0        /**< EIS Future looking */
} cam_is_type_t;

/** EIS algorithm initialization type */
typedef enum cam_is_operation_mode_e
{
    IS_OPT_REGULAR                 = 0,        /**< Regular operation mode, Stabilizer works as expected. */
    IS_OPT_CALIBRATION_SYMMETRIC   = 1,        /**< Calibration mode (symmetric Downscale) - Stabilizer won't act but instead downscale
                                                *   Input image to output image size for later tuning or simulations.
                                                *   Downscale is symmetric for both axes. In case of different margins
                                                *   per axis there will be crop on one of the axes.
                                                */
    IS_OPT_CALIBRATION_ASYMMETRIC  = 2,        /**< Calibration mode (asymmetric Downscale) - Stabilizer won't act but instead downscale
                                                *   Input image to output image size for later tuning or simulations.
                                                *   Downscale is asymmetric for both axes. In case of different margins
                                                *   per axis there will be need for different upscale per axis before simulations or tuning.
                                                */
    IS_OPT_UNDISTORTION_ONLY       = 3,        /**< Un-Distortion only mode (no stabilization). Will output grid for un-distortion. */
}cam_is_operation_mode_t;

/** Single gyro sample structure */
typedef struct _gyro_sample_s
{
    double      data[3];    /**< 0==x, 1==y, 2==z , angular velocity [Rad/sec] */
    uint64_t    ts;         /**< time stamp in microseconds */
} gyro_sample_t;

/** Gyro samples array structure */
typedef struct _gyro_data_s
{
    gyro_sample_t*  gyro_data;      /**< gyro data */
    uint32_t        num_elements;   /**< number of elements in the buffer */
} gyro_data_t;

/** Single OIS sample structure */
typedef struct ois_sample_s
{
    double      data[2];    /**< 0==x, 1==y, offset */
    uint64_t    ts;         /**< time stamp in microseconds */
} ois_sample_t;

/** OIS samples array structure */
typedef struct ois_data_s
{
    ois_sample_t*   samples;        /**< array of OIS samples */
    uint32_t        num_elements;   /**< number of elements in the buffer */
} ois_data_t;

/** Gyro interval data structure */
typedef struct gyro_times_s
{
    uint64_t first_gyro_ts; /**< First gyro sample timestamps [microseconds] in a batch of gyro samples */
    uint64_t last_gyro_ts;  /**< Last gyro sample timestamps [microseconds] in a batch of gyro samples */
} gyro_times_t;

/** Per sensor configuration data structure */
typedef struct is_init_data_sensor_s
{
    const NcLibWarpGrid* ldc_in2out;        /**< input to output lens distortion correction grid */
    const NcLibWarpGrid* ldc_out2in;        /**< output to input lens distortion correction grid */

    uint32_t is_input_frame_width;          /**< EIS input frame width */
    uint32_t is_input_frame_height;         /**< EIS input frame height */

    uint32_t sensor_mount_angle;            /**< Sensor mount angle (0, 90, 180, 270) */
    CameraPosition camera_position;         /**< Camera position (front, back, etc.) */

    float optical_center_x;                 /**< Lens optical center shift w.r.t. input image origin defined on image center in x-axis direction [pixels of input image] */
    float optical_center_y;                 /**< Lens optical center shift w.r.t. input image origin defined on image center in y-axis direction [pixels of input image] */

    is_chromatix_info_t is_chromatix_info;  /**< Chromatix configurations */

} is_init_data_sensor;

/** Common configurations for all sensors data structure */
typedef struct is_init_data_common_s
{
    cam_is_type_t           is_type;            /**< EIS algorithm version */
    cam_is_operation_mode_t operation_mode;     /**< EIS operation mode:  */

    uint32_t is_output_frame_width;     /**< EIS output frame width after virtual margin upscale */
    uint32_t is_output_frame_height;    /**< EIS output frame height after virtual margin upscale  */

    uint16_t frame_fps;                 /**< Frame rate */

    bool is_warping_using_ica;          /**< if true then EIS warping matrix is executed in ICA. Otherwise in GPU.
                                         *
                                         *  @warning In case this value is false, IQ and performance degradation and could occur.
                                         */

    bool do_virtual_upscale_in_matrix;  /**< if true then upscale is done by ICA matrix, otherwise by FW chosen up-scaler
                                         *
                                         *  @warning In case this value is true, IQ and performance degradation and could occur.
                                         */


    bool is_calc_transform_alignment_matrix_orig;   /**< undistorted frame alignment matrix using gyro, a matrix between undistorted Cur to undistorted Prev */

    bool is_calc_transform_alignment_matrix_mctf;   /**< stabilized frame alignment matrix using gyro, a final
                                                      *   matrix for MCTF matrix between stabC to stabP
                                                      */

    bool is_sat_applied_in_ica; /**< If true, sensor alignment transform, SAT matrix will be applied in ICA along with
                                 *   EIS transform. EIS output matrices are already combined with SAT matrix on this case.
                                 *   Otherwise SAT is applied in GPU prior to calling EIS 3.x frame stabilization - eis3_process().
                                 *
                                 *  @warning In case this value is false, that is SAT is
                                 *           applied in GPU, IQ and performance degradation and could occur.
                                 */

    bool is_ois_enabled;    /**< if true, then OIS support is enabled. Otherwise disabled */

    /*          EIS 2.x specific configurations             */

    /*          EIS 3.x specific configurations             */
    uint16_t buffer_delay;          /**< For EIS3 future looking only */
    uint16_t num_mesh_y;            /**< Number of matrices used for stabilization */

} is_init_data_common;

/* IS input data structure */
typedef struct is_input_s
{
    uint32_t            frame_id;           /**< Frame ID */
    uint32_t            active_sensor_idx;  /**< Active sensor index */
    uint32_t            sensor_rolling_shutter_skew;   /**< sensor_rolling_shutter_skew in microseconds */

    frame_times_t       frame_times;        /**< Frame time */

    gyro_data_t         gyro_data;          /**< Gyro data - gyro sample array */

    ois_data_t          ois_data;           /**< OIS data -  OIS samples array*/

    WindowRegion        ife_crop;           /**< IFE crop window. Relative to IFE input size, i.e.
                                             *    - fullWidth,   fullHeight   == IFE input size
                                             *    - windowWidth, windowHeight == IFE crop size (actual)
                                             *    - windowLeft,  windowTop    == IFE crop left, top (actual)
                                             */

    WindowRegion        ipe_zoom;           /**< IPE zoom window. Relative to EIS output crop size, i.e.
                                             *    - fullWidth,   fullHeight   == EIS output size
                                             *                                == ICA1 input size - total stabilization margins
                                             *                                      ( physical+virtual )
                                             *    - windowWidth, windowHeight == ICA1 output size
                                             *    - windowLeft,  windowTop    == ICA1 output crop left, top
                                             */

    float               focus_distance;     /**< Focus distance in meters. Distance from object on which focus is locked on */

    const mat3x3*       sat;                /**< Optional - if equal to NULL then EIS disregards this parameter.
                                            *
                                            * - Order of matrices operators from in->out is first SAT and then EIS.
                                            * - In order to align current sensor to previous sensor,
                                            *   use SAT matrix which is OUT2IN, thus the matrix direction is from
                                            *   previous sensor geometry to current sensor geometry.
                                            * - SAT transform shall be defined on resolution of input to EIS.
                                            * - SAT transform shall define center of image as origin (0,0).
                                            */
} is_input_t;

/** IS output data structure */
typedef struct is_output_type_s
{
    uint32_t            frame_id;                   /**< Processed frame index  */
    uint32_t            active_sensor_idx;          /**< Active sensor index, synced with frame_id */

    NcLibWarp           stabilizationTransform;     /**< Stabilization transform, as passed to NcLib. Matrices are
                                                     *   to synced with frame_id
                                                     */

    NcLibWarpMatrices   alignment_matrix_orig;       /**< Original frame alignment matrix using gyro instead of
                                                     *   LRME, a matrix between origC to origP
                                                     */

    NcLibWarpMatrices   alignment_matrix_final;      /**< Stabilized frame alignment matrix using gyro target, a final
                                                     *   matrix for MCTF matrix between stabC to stabP */

    bool                has_output;                 /**< if true, a frame was processed. Otherwise frame was not processed */
} is_output_type;

/** IS get stabilization margin input struct. Used by eis3_get_stabilization_margin_ex() */
typedef struct is_get_stabilization_margin_s
{
    uint32_t common_is_output_frame_height;             /**< Output frame height - Common to all sensors */
    uint32_t common_is_output_frame_width;              /**< Output frame width - Common to all sensors */
    bool     common_do_virtual_upscale_in_matrix;       /**< If true then upscale is done using matrix. Common to all sensors */

    uint32_t sensor_is_input_frame_height;              /**< Sensor input frame height */
    uint32_t sensor_is_input_frame_width;               /**< Sensor input frame width */

    float    sensor_minimal_total_margin_x;             /**< Minimal total margins as supplied to chromatix struct */
    float    sensor_minimal_total_margin_y;             /**< Minimal total margins as supplied to chromatix struct */
} is_get_stabilization_margin;

#ifdef __cplusplus
}
#endif

#endif /* _IS_INTERFACE_H_ */

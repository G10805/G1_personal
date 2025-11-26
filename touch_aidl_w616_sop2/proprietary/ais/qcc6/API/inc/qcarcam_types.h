#ifndef QCARCAM_TYPES_H
#define QCARCAM_TYPES_H

/**************************************************************************************************
@file
    qcarcam_types.h

@brief
    QCarCam API types - QTI Automotive Imaging System Proprietary API

Copyright (c) 2016-2022 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

**************************************************************************************************/

/*=================================================================================================
** INCLUDE FILES FOR MODULE
=================================================================================================*/
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*=================================================================================================
** Constant and Macros
=================================================================================================*/

/** @addtogroup qcarcam_constants
@{ */

/** @brief Maximum number of planes in the buffer. */
#define QCARCAM_MAX_NUM_PLANES 3

/** @brief Maximum number of buffers. */
#define QCARCAM_MAX_NUM_BUFFERS 20

/** @brief Minimum number of buffers. */
#define QCARCAM_MIN_NUM_BUFFERS 3

/** @brief Maximum input name length. */
#define QCARCAM_INPUT_NAME_MAX_LEN 80

/** @brief Macro for a timeout of infinite time. */
#define QCARCAM_TIMEOUT_INIFINITE (-1ULL)

/** @brief Macro for a no wait timeout (i.e., not do wait). */
#define QCARCAM_TIMEOUT_NO_WAIT   (0)

/** @brief Maximum size of gamma table setting.*/
#define QCARCAM_MAX_GAMMA_TABLE 128

/** @brief Maximum payload size in bytes. */
#define QCARCAM_MAX_PAYLOAD_SIZE 1024

/** @brief Maximum vendor defined payload size in bytes.*/
#define QCARCAM_MAX_VENDOR_PAYLOAD_SIZE 1024

/** @brief Maximum number of frames in a batch.*/
#define QCARCAM_MAX_BATCH_FRAMES 4

/** @brief Maximum number of ISP instances.*/
#define QCARCAM_MAX_ISP_INSTANCES 4

/** @brief Maximum number of input modes supported. */
#define QCARCAM_MAX_NUM_MODES   12

/**@brief qcarcam open flags */
#define QCARCAM_OPEN_FLAGS_MASTER   1U << 0  /**< Sets session as master for the input. */
#define QCARCAM_OPEN_FLAGS_RECOVERY 1U << 1  /**< Enables self-recovery for the session. */
#define QCARCAM_OPEN_FLAGS_REQUEST_MODE 1U << 2


/** @brief max number of input streams as part of an input handle. */
#define QCARCAM_MAX_INPUT_STREAMS 4

/** @brief max number of output streams as part of an input handle. */
#define QCARCAM_MAX_OUTPUT_STREAMS 8U

/** @brief max number of sources for an input. */
#define QCARCAM_INPUT_MAX_NUM_SOURCES 16U

/** @brief Input description flags. */
#define QCARCAM_INPUT_FLAG_PAIRED     (1U << 0)   /**< Paired input stream. */


/** @brief Event type: Frame ready event. The frame can be dequeued via QCarCamGetFrame(). */
#define QCARCAM_EVENT_FRAME_READY   (1U << 0)

/** @brief Event type: Error event associated with the #QCarCamErrorInfo_t payload. */
#define QCARCAM_EVENT_ERROR         (1U << 1)

/** @brief Event type: Vendor event associated with the #QCarCamVendorParam_t payload. */
#define QCARCAM_EVENT_VENDOR        (1U << 2)

/** @brief Event type: Start of Frame (SOF) event associated with the #QCarCamHWTimestamp_t
           payload. */
#define QCARCAM_EVENT_FRAME_SOF     (1U << 3)

#define QCARCAM_EVENT_INPUT_SIGNAL    (1U << 4)        /**< Input signal change event with #QCarCamInputSignal_e payload. */
#define QCARCAM_EVENT_PROPERTY_NOTIFY (1U << 5)        /**< Property change notification event. */
#define QCARCAM_EVENT_RECOVERY        (1U << 6)        /**< Recovery event with #QCarCamRecovery_t payload. */

/** @brief Input mode was changed. */
#define QCARCAM_EVENT_INPUT_MODE_CHANGE (1U << 8)
/** @brief Event when a new input is detected. Event only sent as a system wide event. */
#define QCARCAM_EVENT_INPUT_DETECT      (1U << 9)


/** @brief Buffer flag bit if the buffer is cached. */
#define QCARCAM_BUFFER_FLAG_CACHE       (1U << 0)

/** @brief Buffer flag bit if the output buffer (memHndl in #QCarCamPlane_t) refers to an OS memory
           handle. */
#define QCARCAM_BUFFER_FLAG_OS_HNDL     (1U << 1)

/** @brief will be redefined later to extend support for security domains */
#define QCARCAM_BUFFER_FLAG_SECURE      (1U << 2)


/** @brief Invalid QCarCam handle type. */
#define QCARCAM_HNDL_INVALID ((QCarCamHndl_t)0)


/** @brief Request flag bit if inputMetaIdx is used. */
#define QCARCAM_REQUEST_FLAGS_INPUT_META       (1U << 0)

/** @brief Request flag bit if outputMetaIdx is used. */
#define QCARCAM_REQUEST_FLAGS_OUTPUT_META      (1U << 1)


/** @brief Macro for creating the color format.

    @hideinitializer
*/
#define QCARCAM_COLOR_FMT(_pattern_, _bitdepth_, _pack_)    \
    ((((uint32_t)(_pack_) & 0xffU) << 24) |                 \
     (((uint32_t)(_bitdepth_) & 0xffU) << 16) |             \
      ((uint32_t)(_pattern_) & 0xffffU))

/** @brief Macro for getting the color pattern from the color format.

    @hideinitializer
*/
#define QCARCAM_COLOR_GET_PATTERN(_color_)                  \
    ((QCarCamColorPattern_e)((_color_) & 0xffff))

/** @brief Macro for getting the bit depth from the color format.

    @hideinitializer
*/
#define QCARCAM_COLOR_GET_BITDEPTH(_color_)                 \
    ((QCarCamColorBitDepth_e)(((_color_) & 0xff0000) >> 16))

/** @brief Macro for getting the color pack from the color format.

    @hideinitializer
*/
#define QCARCAM_COLOR_GET_PACK(_color_)                     \
    ((QCarCamColorPack_e)(((_color_) & 0xff000000) >> 24))

/** @} */ /* end_addtogroup qcarcam_constants */

/** @addtogroup qcarcam_datatypes
@{ */

/*=================================================================================================
** Typedefs
=================================================================================================*/

/** @brief QCarCam input handle type requested via QCarCamOpen() associated to a unique input ID. */
typedef uint64_t QCarCamHndl_t;

/** @brief Return codes. */
typedef enum
{
    QCARCAM_RET_OK = 0,         /**< API return type OK (success). */
    QCARCAM_RET_FAILED,         /**< API return type failed. */
    QCARCAM_RET_BADPARAM,       /**< API return type bad parameter(s). */
    QCARCAM_RET_INVALID_OP,     /**< API return type invalid operation. */
    QCARCAM_RET_BADSTATE,       /**< API return type bad state. */
    QCARCAM_RET_NOT_PERMITTED,  /**< API return type not permitted. */
    QCARCAM_RET_OUT_OF_BOUND,   /**< API return type out of bound. */
    QCARCAM_RET_TIMEOUT,        /**< API return type time out. */
    QCARCAM_RET_NOMEM,          /**< API return type no memory. */
    QCARCAM_RET_UNSUPPORTED,    /**< API return type not supported. */
    QCARCAM_RET_BUSY,           /**< API return type busy. */
    QCARCAM_RET_NOT_FOUND,      /**< API return type not found. */
    QCARCAM_RET_LAST      = 255 /**< Last or invalid QCARCAM_RET_* type. */
} QCarCamRet_e;

/** @brief Color type definition. */
typedef enum
{
    QCARCAM_RAW = 0U,                       /**< Type for raw color format. */

    /* Memory layout for YUV formats is the color component name from LSB to MSB
     * e.g. YUYV ->  Y:[0-7], U:[8-15], Y[16-23], V[24-31]
     * */
    QCARCAM_YUV_YUYV = 0x100U,              /**< Type for YUV_YUYV color format. */
    QCARCAM_YUV_YVYU,                       /**< Type for YUV_YVYU color format. */
    QCARCAM_YUV_UYVY,                       /**< Type for YUV_UYVY color format. */
    QCARCAM_YUV_VYUY,                       /**< Type for YUV_VYUY color format. */

    QCARCAM_YUV_Y_UV  = 0x104U,             /**< Type for YUV_Y_UV color format. */
    QCARCAM_YUV_Y_VU  = 0x105U,             /**< Type for YUV_Y_VV color format. */
    QCARCAM_YUV_Y_U_V = 0x106U,             /**< Type for YUV_Y_U_V color format. */
    QCARCAM_YUV_Y_V_U = 0x107U,             /**< Type for YUV_Y_V_U color format. */

    QCARCAM_YUV_NV12 = QCARCAM_YUV_Y_UV,    /**< Type for YUV_NV12 color format. */
    QCARCAM_YUV_NV21 = QCARCAM_YUV_Y_VU,    /**< Type for YUV_NV21 color format. */
    QCARCAM_YUV_YU12 = QCARCAM_YUV_Y_U_V,   /**< Type for YUV_YU12 color format. */
    QCARCAM_YUV_YV12 = QCARCAM_YUV_Y_V_U,   /**< Type for YUV_YV12 color format. */

    QCARCAM_BAYER_GBRG = 0x200U,            /**< Type for BAYER_GBRG color format. */
    QCARCAM_BAYER_GRBG,                     /**< Type for BAYER_GRBG color format. */
    QCARCAM_BAYER_RGGB,                     /**< Type for BAYER_RGGB color format. */
    QCARCAM_BAYER_BGGR,                     /**< Type for BAYER_BGGR color format. */

    /* Memory layout for RGB formats is the color component name from MSB to LSB
     * e.g. RGB888 ->  R:[16-23], G:[8-15], B[0-7]
     * */
    QCARCAM_RGB = 0x300U,        /**< Type for RGB color R:[16-23], G:[8-15]: B[0-7] */
    QCARCAM_RGB_RGB888 = QCARCAM_RGB, /**< Type for RGB888 color R:[16-23], G:[8-15]: B[0-7] */
    QCARCAM_RGB_BGR888,         /**< Type for BGR888 color B:[16-23], G:[8-15]: R[0-7] */
    QCARCAM_RGB_RGB565,         /**< Type for RGB565 color R:[11-15], G:[5-10]: B[0-4] */
    QCARCAM_RGB_RGBX8888,       /**< Type for RGBX8888 color R:[24-31] G:[16-23], B:[8-15]: X[0-7] */
    QCARCAM_RGB_BGRX8888,       /**< Type for BGRX8888 color B:[24-31] G:[16-23], R:[8-15]: X[0-7] */
    QCARCAM_RGB_RGBX1010102,    /**< Type for RGBX1010102 color R:[22-31] G:[12-21], B:[2-11]: X[0-1] */
    QCARCAM_RGB_BGRX1010102,    /**< Type for BGRX1010102 color B:[22-31] G:[12-21], R:[2-11]: X[0-1] */

    QCARCAM_COLORTYPE_MAX = 0x7fffffffU     /**< Last or invalid color type. */
} QCarCamColorPattern_e;

/* @brief Bitdepth per color channel
 * For RAW/YUV formats it is the bits per color channel
 * For RGB formats it is the effective bits per pixel */
typedef enum
{
    QCARCAM_BITDEPTH_8 =  8U,           /**< Type for color bit depth of 8. */
    QCARCAM_BITDEPTH_10 = 10U,          /**< Type for color bit depth of 10. */
    QCARCAM_BITDEPTH_12 = 12U,          /**< Type for color bit depth of 12. */
    QCARCAM_BITDEPTH_14 = 14U,          /**< Type for color bit depth of 14. */
    QCARCAM_BITDEPTH_16 = 16U,          /**< Type for color bit depth of 16. */
    QCARCAM_BITDEPTH_20 = 20U,          /**< Type for color bit depth of 20. */
    QCARCAM_BITDEPTH_24 = 24U,          /**< Type for color bit depth of 24. */
    QCARCAM_BITDEPTH_32 = 32U,          /**< Type for color bit depth of 32. */
    QCARCAM_BITDEPTH_MAX = 0x7fffffffU  /**< Last or invalid color bit depth type. */
} QCarCamColorBitDepth_e;

/** @brief Color packing type. */
typedef enum
{
    QCARCAM_PACK_QTI = 0,           /**< Type for Qualcomm Technologies Inc (QTI) color pack. */
    QCARCAM_PACK_MIPI,              /**< Type for Mobile Industry Processor Interface (MIPI)
                                         color pack. */
    QCARCAM_PACK_PLAIN8,            /**< Type for Plain 8 color pack. */
    QCARCAM_PACK_PLAIN16,           /**< Type for Plain 16 color pack. */
    QCARCAM_PACK_PLAIN32,           /**< Type for Plain 32 color pack. */
    QCARCAM_PACK_MAX = 0x7fffffffU  /**< Last or invalid color pack type. */
} QCarCamColorPack_e;

/** @brief Color formats. */
typedef enum
{
    /**< MIPI packed RAW formats. */
    QCARCAM_FMT_MIPIRAW_8 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                              QCARCAM_BITDEPTH_8,
                                              QCARCAM_PACK_MIPI),   /**< MIPI packed RAW 8bit */
    QCARCAM_FMT_MIPIRAW_10 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_10,
                                               QCARCAM_PACK_MIPI),  /**< MIPI packed RAW 10bit */
    QCARCAM_FMT_MIPIRAW_12 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_12,
                                               QCARCAM_PACK_MIPI),  /**< MIPI packed RAW 12bit */
    QCARCAM_FMT_MIPIRAW_14 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_14,
                                               QCARCAM_PACK_MIPI),  /**< MIPI packed RAW 14bit */
    QCARCAM_FMT_MIPIRAW_16 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_16,
                                               QCARCAM_PACK_MIPI),  /**< MIPI packed RAW 16bit */
    QCARCAM_FMT_MIPIRAW_20 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_20,
                                               QCARCAM_PACK_MIPI),  /**< MIPI packed RAW 20bit */

    /**< PLAIN16 packed RAW formats. */
    QCARCAM_FMT_PLAIN16_10 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_10,
                                               QCARCAM_PACK_PLAIN16),/**< Plain16 packed RAW
                                                                          10bit */
    QCARCAM_FMT_PLAIN16_12 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_12,
                                               QCARCAM_PACK_PLAIN16),/**< Plain16 packed RAW
                                                                          12bit */
    QCARCAM_FMT_PLAIN16_14 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_14,
                                               QCARCAM_PACK_PLAIN16),/**< Plain16 packed RAW
                                                                          14bit */
    QCARCAM_FMT_PLAIN16_16 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_16,
                                               QCARCAM_PACK_PLAIN16),/**< Plain16 packed RAW
                                                                          16bit */
    QCARCAM_FMT_PLAIN32_20 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_20,
                                               QCARCAM_PACK_PLAIN32),/**< Plain32 packed RAW
                                                                             20bit */

    /**< YUV Packed Formats. */
    QCARCAM_FMT_UYVY_8 = QCARCAM_COLOR_FMT(QCARCAM_YUV_UYVY,
                                           QCARCAM_BITDEPTH_8,
                                           QCARCAM_PACK_QTI),    /**< QTI packed YUV_UYVY
                                                                         8bit */
    QCARCAM_FMT_UYVY_10 = QCARCAM_COLOR_FMT(QCARCAM_YUV_UYVY,
                                            QCARCAM_BITDEPTH_10,
                                            QCARCAM_PACK_QTI),   /**< QTI packed YUV_UYVY
                                                                         10bit */
    QCARCAM_FMT_UYVY_12 = QCARCAM_COLOR_FMT(QCARCAM_YUV_UYVY,
                                            QCARCAM_BITDEPTH_12,
                                            QCARCAM_PACK_QTI),   /**< QTI packed YUV_UYVY
                                                                         12bit */
    QCARCAM_FMT_YUYV_8 = QCARCAM_COLOR_FMT(QCARCAM_YUV_YUYV,
                                           QCARCAM_BITDEPTH_8,
                                           QCARCAM_PACK_QTI),    /**< QTI packed YUV_YUYV
                                                                         8bit */
    QCARCAM_FMT_YUYV_10 = QCARCAM_COLOR_FMT(QCARCAM_YUV_YUYV,
                                            QCARCAM_BITDEPTH_10,
                                            QCARCAM_PACK_QTI),   /**< QTI packed YUV_YUYV
                                                                         10bit */
    QCARCAM_FMT_YUYV_12 = QCARCAM_COLOR_FMT(QCARCAM_YUV_YUYV,
                                            QCARCAM_BITDEPTH_12,
                                            QCARCAM_PACK_QTI),   /**< QTI packed YUV_YUYV
                                                                         12bit */

    /**< YUV Semi-Planar Formats. */
    QCARCAM_FMT_NV12 = QCARCAM_COLOR_FMT(QCARCAM_YUV_NV12,
                                         QCARCAM_BITDEPTH_8,
                                         QCARCAM_PACK_QTI),      /**< QTI packed YUV_NV12
                                                                         8bit */
    QCARCAM_FMT_NV21 = QCARCAM_COLOR_FMT(QCARCAM_YUV_NV21,
                                         QCARCAM_BITDEPTH_8,
                                         QCARCAM_PACK_QTI),      /**< QTI packed YUV_NV21
                                                                         8bit */
    QCARCAM_FMT_YU12 = QCARCAM_COLOR_FMT(QCARCAM_YUV_YU12,
                                         QCARCAM_BITDEPTH_8,
                                         QCARCAM_PACK_QTI),     /**< QTI packed YUV_YU12
                                                                         8bit */
    QCARCAM_FMT_YV12 = QCARCAM_COLOR_FMT(QCARCAM_YUV_YV12,
                                         QCARCAM_BITDEPTH_8,
                                         QCARCAM_PACK_QTI),     /**< QTI packed YUV_YV12
                                                                         8bit */

    /**< YUV MIPI and PLAIN packed formats. */
    QCARCAM_FMT_MIPIUYVY_10 = QCARCAM_COLOR_FMT(QCARCAM_YUV_UYVY,
                                                QCARCAM_BITDEPTH_10,
                                                QCARCAM_PACK_MIPI), /**< MIPI packed UYVY 10bit */
    QCARCAM_FMT_P010 = QCARCAM_COLOR_FMT(QCARCAM_YUV_Y_UV,
                                         QCARCAM_BITDEPTH_10,
                                         QCARCAM_PACK_PLAIN16),     /**< Plain16 packed Y_UV 10bit */

    /**< RGB Formats. */
    QCARCAM_FMT_RGB_888 = QCARCAM_COLOR_FMT(QCARCAM_RGB,
                                            QCARCAM_BITDEPTH_24,
                                            QCARCAM_PACK_QTI),      /**< QTI packed RGB888 24bit */

    QCARCAM_FMT_BGR_888 = QCARCAM_COLOR_FMT(QCARCAM_RGB_BGR888,
                                            QCARCAM_BITDEPTH_24,
                                            QCARCAM_PACK_QTI),      /**< QTI packed BGR888 24bit */

    QCARCAM_FMT_RGB_565 = QCARCAM_COLOR_FMT(QCARCAM_RGB_RGB565,
                                            QCARCAM_BITDEPTH_16,
                                            QCARCAM_PACK_QTI),      /**< QTI packed RGB565 16bit */

    QCARCAM_FMT_RGBX_8888 = QCARCAM_COLOR_FMT(QCARCAM_RGB_RGBX8888,
                                              QCARCAM_BITDEPTH_32,
                                              QCARCAM_PACK_QTI),    /**< QTI packed RGBX8888 32bit */

    QCARCAM_FMT_BGRX_8888 = QCARCAM_COLOR_FMT(QCARCAM_RGB_BGRX8888,
                                              QCARCAM_BITDEPTH_32,
                                              QCARCAM_PACK_QTI),    /**< QTI packed BGRX8888 32bit */

    QCARCAM_FMT_RGBX_1010102 = QCARCAM_COLOR_FMT(QCARCAM_RGB_RGBX1010102,
                                                 QCARCAM_BITDEPTH_32,
                                                 QCARCAM_PACK_QTI),     /**< QTI packed RGBX1010102 32bit */

    QCARCAM_FMT_BGRX_1010102 = QCARCAM_COLOR_FMT(QCARCAM_RGB_BGRX1010102,
                                                 QCARCAM_BITDEPTH_32,
                                                 QCARCAM_PACK_QTI),     /**< QTI packed BGRX1010102 32bit */

    QCARCAM_FMT_MAX = 0x7FFFFFFF    /**< Last or invalid color format type. */
} QCarCamColorFmt_e;

/** @brief Supported color space formats */
typedef enum
{
    QCARCAM_COLOR_SPACE_UNCORRECTED = 0,
    QCARCAM_COLOR_SPACE_SRGB,
    QCARCAM_COLOR_SPACE_LRGB,
    QCARCAM_COLOR_SPACE_BT601,
    QCARCAM_COLOR_SPACE_BT601_FULL,
    QCARCAM_COLOR_SPACE_BT709,
    QCARCAM_COLOR_SPACE_BT709_FULL,
}QCarCamColorSpace_e;

/** @brief Parameter settings.
 *
 *    Expected *pValue for QCarCamGetParam() and QCarCamSetParam():
 *
 * @verbatim

         PARAMETER                              |       TYPE                |    NOTE
  -------------------------------------------------------------------------------------------------------
  QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK        |  uint32_t                 | bitmask of QCARCAM_EVENT_*
  QCARCAM_STREAM_CONFIG_PARAM_SET_CROP          |  QCarCamRegion_t          |
  QCARCAM_STREAM_CONFIG_PARAM_LATENCY_CONTROL   |  QCarCamLatencyControl_t  |
  QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL|  QCarCamFrameDropConfig_t |
  QCARCAM_STREAM_CONFIG_PARAM_INPUT_MODE        |  uint32_t                 |
  QCARCAM_STREAM_CONFIG_PARAM_MASTER            |  uint32_t                 | 1 to set, 0 to relinquish
  QCARCAM_STREAM_CONFIG_PARAM_CHANGE_SUBSCRIBE  |  uint32_t                 |
  QCARCAM_STREAM_CONFIG_PARAM_CHANGE_UNSUBSCRIBE|  uint32_t                 |
  QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE        |  QCarCamBatchConfig_t     |
  QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE       |  QCarCamIspUsecaseConfig_t|
  ----------------------------------------------|---------------------------|
  QCARCAM_SENSOR_PARAM_MIRROR_H                 |  uint32_t                 |
  QCARCAM_SENSOR_PARAM_MIRROR_V                 |  uint32_t                 |
  QCARCAM_SENSOR_PARAM_VID_STD                  |  uint32_t                 |
  QCARCAM_SENSOR_PARAM_CURRENT_VID_STD          |  uint32_t                 |
  QCARCAM_SENSOR_PARAM_SIGNAL_STATUS            |  QCarCamInputSignal_e     |
  QCARCAM_SENSOR_PARAM_EXPOSURE                 |  QCarCamExposureConfig_t  |
  QCARCAM_SENSOR_PARAM_GAMMA                    |  QCarCamGammaConfig_t     |
  QCARCAM_SENSOR_PARAM_BRIGHTNESS               |  float                    |
  QCARCAM_SENSOR_PARAM_CONTRAST                 |  float                    |
  QCARCAM_SENSOR_PARAM_HUE                      |  float                    |
  QCARCAM_SENSOR_PARAM_SATURATION               |  float                    |
  QCARCAM_SENSOR_PARAM_COLOR_SPACE              |  QCarCamColorSpace_e      |
  ----------------------------------------------|---------------------------|
  QCARCAM_VENDOR_PARAM                          |  QCarCamVendorParam_t     |

   @endverbatim
*/
typedef enum
{
    QCARCAM_STREAM_CONFIG_PARAM_BASE = 0x00000000,  /**< Stream configuration parameter base
                                                         value */
    QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK,         /**< Event mask stream configuration
                                                         parameter value */
    QCARCAM_STREAM_CONFIG_PARAM_SET_CROP,           /**< Set crop at the input */
    QCARCAM_STREAM_CONFIG_PARAM_LATENCY_CONTROL,    /**< Max buffer latency in frame done Q. */
    QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL,   /**< frame drop control. */
    QCARCAM_STREAM_CONFIG_PARAM_INPUT_MODE,           /**< Input device mode.. */
    QCARCAM_STREAM_CONFIG_PARAM_MASTER,               /**< Set the client as master. */
    QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_SUBSCRIBE,      /**< Event subscription. */
    QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_UNSUBSCRIBE,    /**< Event unsubscribe. */
    QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE,           /**< Configures batch mode. */
    QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE,          /**< Configures ISP usecase type. */
    QCARCAM_STREAM_CONFIG_PARAM_METADATA_TAG,         /**< Configures ISP meta data tag. */

    QCARCAM_SENSOR_PARAM_BASE = 0x00000100,    /**< Sensor parameter base value */
    QCARCAM_SENSOR_PARAM_MIRROR_H,             /**< Horizontal mirror. */
    QCARCAM_SENSOR_PARAM_MIRROR_V,             /**< Vertical mirror. */
    QCARCAM_SENSOR_PARAM_VID_STD,              /**< Video standard. */
    QCARCAM_SENSOR_PARAM_CURRENT_VID_STD,      /**< Current Detected Video standard. */
    QCARCAM_SENSOR_PARAM_SIGNAL_STATUS,        /**< Video lock status. */
    QCARCAM_SENSOR_PARAM_EXPOSURE,             /**< exposure setting. */
    QCARCAM_SENSOR_PARAM_GAMMA,                /**< gamma setting. */
    QCARCAM_SENSOR_PARAM_BRIGHTNESS,           /**< brightness setting. */
    QCARCAM_SENSOR_PARAM_CONTRAST,             /**< contrast setting. */
    QCARCAM_SENSOR_PARAM_HUE,                  /**< hue setting. */
    QCARCAM_SENSOR_PARAM_SATURATION,           /**< saturation setting. */
    QCARCAM_SENSOR_PARAM_COLOR_SPACE,          /**< color space setting. */

    QCARCAM_VENDOR_PARAM_BASE = 0x00000300,    /**< Vendor specific sensor parameter value. */
    QCARCAM_VENDOR_PARAM,                      /**< Vendor parameter */

    QCARCAM_PARAM_MAX = 0x7FFFFFFF    /**< Last or invalid parameter type. */
} QCarCamParamType_e;

/** @brief Interlaced field description */
typedef enum
{
    QCARCAM_INTERLACE_FIELD_NONE = 0,   /**< not an interlaced frame */
    QCARCAM_INTERLACE_FIELD_UNKNOWN,    /**< interlaced but unknown field type */
    QCARCAM_INTERLACE_FIELD_ODD,        /**< interlaced odd field */
    QCARCAM_INTERLACE_FIELD_EVEN,       /**< interlaced even field */
    QCARCAM_INTERLACE_FIELD_ODD_EVEN,   /**< interlaced odd field at top, even field on bottom */
    QCARCAM_INTERLACE_FIELD_EVEN_ODD,   /**< interlaced even field at top, odd field on bottom */

    QCARCAM_INTERLACE_FIELD_MAX = 0x7FFFFFFF  /**< Last or invalid interlace type. */
} QCarCamInterlaceField_e;

/** @brief Exposure modes */
typedef enum
{
    QCARCAM_EXPOSURE_AUTO,   /**< Automatic exposure control */
    QCARCAM_EXPOSURE_MANUAL, /**< Manual exposure control */
} QCarCamExposureMode_e;

/** @brief Exposures identification.
 *    T1 exposure is a longest and each successive exposure being shorter (T1 > T2 > T3 > T4)
 */
typedef enum
{
    QCARCAM_HDR_EXPOSURE_T1,
    QCARCAM_HDR_EXPOSURE_T2,
    QCARCAM_HDR_EXPOSURE_T3,
    QCARCAM_HDR_EXPOSURE_T4,
    QCARCAM_HDR_NUM_EXPOSURES
} QCarCamExposure_e;

/** @brief Gamma parameter type. */
typedef enum
{
    QCARCAM_GAMMA_EXPONENT,         /**< set gamma exponent value in a float */
    QCARCAM_GAMMA_KNEEPOINTS,       /**< set table of gamma kneepoint values. */
} QCarCamGammaType_e;


/** @brief Input operation modes.

    The operation mode dictates the overall operating mode of the session.
    If an operation mode has an ISP component, then the QCarCamIspUsecase_e can specify
    the usecase for an ISP component instance.
  */
typedef enum
{
    QCARCAM_OPMODE_RAW_DUMP,        /**< Raw dump of data
                                      For 2 streams, use QCARCAM_OPMODE_RAW_DUMP + QCarCamOpen_t->numInputs=2. */
    QCARCAM_OPMODE_ISP,             /**< Inline ISP. */
    QCARCAM_OPMODE_OFFLINE_ISP,     /**< Injection to ISP. */
    QCARCAM_OPMODE_PAIRED_INPUT,    /**< Paired CSI inputs to single output buffer. */
    QCARCAM_OPMODE_DEINTERLACE,     /**< Deinterlace input. */
    QCARCAM_OPMODE_TRANSFORMER,     /**< Gpu color conversion and scaling. */
    QCARCAM_OPMODE_PREPROCESS_ISP,  /**< Preprocessing then ISP. */
    QCARCAM_OPMODE_RDI_CONVERSION,  /**< yuv422 -> nv12 raw conversion. */
    QCARCAM_OPMODE_ISP_JPEG,        /**< ISP + JPEG. */
    QCARCAM_OPMODE_RAW_DUMP_TRANSFORMER, /**< Raw dump + GPU transform. */
    QCARCAM_OPMODE_FUSED_SHDR,      /**< fused SHDR. */

    QCARCAM_OPMODE_MAX = 0x7FFFFFFF /**< Last or invalid parameter type. */
} QCarCamOpmode_e;

/** @brief ISP use case for operating modes with ISP */
typedef enum
{
    QCARCAM_ISP_USECASE_PREVIEW,                /**< outputs 0-preview */
    QCARCAM_ISP_USECASE_SHDR_PREPROCESS,        /**< outputs 0-preview */
    QCARCAM_ISP_USECASE_CUSTOM_START  = 0x100U, /**< starting offset for custom defined usecases */

    QCARCAM_ISP_USECASE_MAX = 0x7FFFFFFF    /**< Last or invalid parameter type. */
} QCarCamIspUsecase_e;

/** @brief Batch mode types. */
typedef enum
{
    QCARCAM_BATCH_MODE_FILL_ALL_FRAME_INFO = 0, /**< #QCarCamBatchFramesInfo_t filled for each of the batched frames. */

    QCARCAM_BATCH_MODE_MAX = 0x7FFFFFFF         /**< Last or invalid parameter type. */
}QCarCamBatchMode_e;

/** @brief buffer list identification */
typedef enum
{
    QCARCAM_BUFFERLIST_ID_OUTPUT_0       = 0,       /**< output buffer lists start at this index */
    QCARCAM_BUFFERLIST_ID_INPUT_0        = 0x100U,  /**< input buffer list for injection */
    QCARCAM_BUFFERLIST_ID_INPUT_METADATA = 0x200U,  /**< input metadata bufferlist */
    QCARCAM_BUFFERLIST_ID_OUTPUT_METADATA = 0x300U, /**< output metadata bufferlist */

    QCARCAM_BUFFERLIST_ID_MAX = 0x7FFFFFFF          /**< Last or invalid parameter type. */
} QCarCamBufferListId_e;

/** @brief Input Event payload definition */
typedef enum
{
    QCARCAM_INPUT_SIGNAL_VALID = 0,   /**< Signal is locked/valid */
    QCARCAM_INPUT_SIGNAL_LOST,        /**< Signal is not locked or was lost */

    QCARCAM_INPUT_SIGNAL_MAX = 0x7FFFFFFF    /**< Last or invalid parameter type. */
} QCarCamInputSignal_e;

/** @brief Error event type definition. */
typedef enum
{
    QCARCAM_ERROR_FATAL,        /**< Fatal error */
    QCARCAM_ERROR_WARNING,      /**< Non Fatal error */
} QCarCamErrorEvent_e;

/** @brief Recover Message Id */
typedef enum
{
   QCARCAM_RECOVERY_STARTED = 0, /**< Recovery started */
   QCARCAM_RECOVERY_SUCCESS,     /**< Recovery completed successfully */
   QCARCAM_RECOVERY_FAILED       /**< Recovery failed - handle in error state */
} QCarCamEventRecoveryMsg_e;

/*=================================================================================================
** Typedefs
=================================================================================================*/

/** @brief QCarCam API initialization definition. */
typedef struct
{
    uint32_t flags;         /**< QCarCam initialization flags. */
    uint32_t apiVersion;    /**< QCarCam API version. */
} QCarCamInit_t;

/** @brief ISP instance config parameters. */
typedef struct
{
    uint32_t  id;                  /**< ISP instance id. */
    uint32_t  cameraId;            /**< ISP camera id. */
    QCarCamIspUsecase_e usecaseId;    /**< ISP use case. */
    uint32_t  tuningMode;
} QCarCamIspUsecaseConfig_t;

/** @brief An input stream is defined by the source of an input in a specific mode */
typedef struct
{
    uint32_t inputId;   /**< Input identifier */
    uint32_t srcId;     /**< Input source identifier. See #QCarCamInputSrc_t */
    uint32_t inputMode; /**< The input mode id is the index into #QCarCamInputModes_t pModex*/
} QCarCamInputStream_t;

/** @brief QCarCamOpen parameter. */
typedef struct
{
    QCarCamOpmode_e opMode; /**< OPMODE to indicate usecase */
    uint32_t  priority;     /**< priority of the stream */
    uint32_t  flags;        /**< bitmask of QCARCAM_OPEN_FLAGS_* */
    QCarCamInputStream_t  inputs[QCARCAM_MAX_INPUT_STREAMS]; /**< set of input sources */
    uint32_t  numInputs;   /**< number of inputs[] */
} QCarCamOpen_t;

/** @brief Per stream request for now will hold the buffers identifier */
typedef struct
{
    uint32_t bufferlistId; /**< buffer list */
    uint32_t bufferId;     /**< buffer id within the buffer list*/
} QCarCamStreamRequest_t;

/** @brief Capture request definition */
typedef struct
{
    uint32_t requestId;       /**< unique request Id that monotonically increases */
    uint32_t inputBufferIdx;  /**< for injection usecase */
    uint32_t inputMetaIdx;    /**< input metadata index */
    uint32_t outputMetaIdx;   /**< output metadata index */
    QCarCamStreamRequest_t streamRequests[QCARCAM_MAX_OUTPUT_STREAMS]; /**< output buffer indices */
    uint32_t numStreamRequests; /**< number of per streamRequests[] */
    uint32_t  flags;        /**< bitmask of QCARCAM_REQUEST_FLAGS_* */
} QCarCamRequest_t;

/** @brief Rectange region defined by start x/y offset nad width/height */
typedef struct
{
    uint32_t x;  /**< offset from start pixel in x direction */
    uint32_t y;  /**< offset from start line in y direction */
    uint32_t width; /**< width in pixels */
    uint32_t height; /**< height in pixels */
} QCarCamRegion_t;

/** @brief A source is identified by its source id
 * It is defined by resolution (width/height), color format, frame rate, and
 * security domain.
*/
typedef struct
{
    uint32_t srcId;  /**< identifier of the source stream */
    uint32_t width;  /**< Frame width in pixels. */
    uint32_t height; /**< Frame height in pixels.*/
    QCarCamColorFmt_e   colorFmt;   /**< Color format */
    float   fps;        /**< Frame per second frequency (in hertz). */
    uint32_t securityDomain; /**< @NOTE to be defined */
} QCarCamInputSrc_t;

/** @brief Input mode definition.
 * An input mode can comprise of 1 or more sources.
 * For instance, fused HDR mode may have 1 source, but a non-fused mode may have
 * a separate source for each exposure.
*/
typedef struct {
    QCarCamInputSrc_t sources[QCARCAM_INPUT_MAX_NUM_SOURCES];   /**< Definition of available input sources for this mode. */
    uint32_t          numSources;                               /**< Number of sources available for this mode. */
} QCarCamMode_t;

/** @brief Input mode query structure
 *  QCarCamQueryInputModes() would return QCarCamMode_t[numModes]
*/
typedef struct
{
    uint32_t currentMode;  /**< current mode set for an input */
    uint32_t numModes;     /**< number of modes to query (size of pModes[]). */
    QCarCamMode_t *pModes; /**< pointer to memory that will hold up to QCarCamMode_t[numModes] */
} QCarCamInputModes_t;

/** @brief Input definition
 *  QCarCamQueryInputs() would return QCarCamInput_t[]
 *  QCarCamQueryInputModes() would return QCarCamMode_t[numModes]
*/
typedef struct
{
    uint32_t      inputId;                                /**< Input Identifier. */
    uint32_t      devId;                                  /**< Input device Id */
    uint32_t      subdevId;                               /**< Input sub device Id */
    char          inputName[QCARCAM_INPUT_NAME_MAX_LEN];  /**< Input name. */
    uint32_t      numModes;                               /**< Number of input modes
                                                               in the modes list. */
    uint32_t      flags;                                  /**< bitmask of QCARCAM_INPUT_FLAG_ */
} QCarCamInput_t;


/** @brief Buffer plane definition. */
typedef struct
{
    uint32_t    width;      /**< Width in pixels. */
    uint32_t    height;     /**< Height in pixels. */
    uint32_t    stride;     /**< Stride in bytes. */
    uint32_t    size;       /**< Size in bytes. */
    uint64_t    memHndl;    /**< Buffer for the plane. */
    uint32_t    offset;     /**< Offset in bytes from start of buffer to the plane. */
} QCarCamPlane_t;

/** @brief Buffer definition. */
typedef struct
{
    QCarCamPlane_t  planes[QCARCAM_MAX_NUM_PLANES]; /**< List of all planes in a buffer. */
    uint32_t        numPlanes;                      /**< Number of planes in a buffer. */
} QCarCamBuffer_t;

/** @brief Buffer list definition. */
typedef struct
{
    uint32_t            id;         /**< Buffer list ID associated with the output buffer of an
                                         input. */

    QCarCamColorFmt_e   colorFmt;   /**< Color format type. */
    QCarCamBuffer_t     *pBuffers;  /**< List of buffers. */
    uint32_t            nBuffers;   /**< Number of buffers in a list. */
    uint32_t            flags;      /**< Bitmask of the QCARCAM_BUFFER_FLAG_* macro. */
} QCarCamBufferList_t;

/** @brief Hardware timestamp definition. */
typedef struct
{
    uint64_t timestamp;         /**< Hardware timestamp (in nanoseconds). */
    uint64_t timestampGPTP;     /**< Generic Precision Time Protocol (GPTP) timestamp. */
} QCarCamHWTimestamp_t;

/** @brief Information for a batch frame */
typedef struct
{
    uint32_t               seqNo;  /**< frame number */
    QCarCamHWTimestamp_t   timestamp; /**< hw timestamp */
} QCarCamBatchFramesInfo_t;

/** @brief Frame buffer ready payload definition. */
typedef struct
{
    uint32_t                id;             /**< Buffer list ID */
    uint32_t                bufferIndex;    /**< Index of the QCarCamBuffer_t pBuffers list.  */
    uint32_t                seqNo;          /**< Sequence number (i.e., frame ID). */
    uint64_t                timestamp;      /**< Software based timestamp for frame ready */
    QCarCamHWTimestamp_t    sofTimestamp;   /**< Hardware Start of Frame (SoF) timestamp. */
    uint32_t                flags;          /**< For future use. */
    QCarCamInterlaceField_e fieldType;      /**< Interlaced field information */

    uint32_t                requestId;      /**< submit frame requestId */
    uint32_t                inputMetaIdx;   /**< intput metadata consumed for the request */

    QCarCamBatchFramesInfo_t  batchFramesInfo[QCARCAM_MAX_BATCH_FRAMES];  /**< detailed info for each frame in batch */
} QCarCamFrameInfo_t;

/** @brief Controls latency Q for the output */
typedef struct
{
    uint32_t  latencyMax; /**< Max buffer latency in frame done Q. */
    uint32_t  latencyReduceRate; /**< Number of buffers to drop when max latency reached. */
} QCarCamLatencyControl_t;

/** @brief Vendor parameter and vendor event payload definition*/
typedef struct
{
    uint8_t    data[QCARCAM_MAX_VENDOR_PAYLOAD_SIZE];  /**< Vendor data */
} QCarCamVendorParam_t;

/** @brief List of events to subscribe to */
typedef struct
{
    uint32_t numParams; /**< List of entries in params */
    uint32_t* params;   /**< List of #QCarCamParamType_e to subscribe to */
}QCarCamEventSubscribe_t;

/** @brief Error event payload definition. */
typedef struct
{
    QCarCamErrorEvent_e errorId;      /**< Error ID type as defined by #QCarCamErrorEvent_e. */
    uint32_t            errorCode;    /**< Error code from QTI Safety Manual. */
    uint32_t            frameId;      /**< Frame Id associated with error if applicable */
    uint64_t            timestamp;    /**< Timestamp of error detection */
} QCarCamErrorInfo_t;

/** @brief Recovery event payload definition */
typedef struct
{
    QCarCamEventRecoveryMsg_e  msg;   /**< Recovery message id */
    QCarCamErrorInfo_t         error; /**< Error event information */
} QCarCamRecovery_t;

/**
 * @brief Contains possible values for the event payload in QCarCam_EventCallback_t().
 *
 * @verbatim

           EVENT ID                      |       TYPE              |    NOTE
    ---------------------------------------------------------------------------
    QCARCAM_EVENT_FRAME_READY            | QCarCamFrameInfo_t      |
    QCARCAM_EVENT_INPUT_SIGNAL           | QCarCamInputSignal_e    |  u32Data
    QCARCAM_EVENT_ERROR                  | QCarCamEventError_e     |  u32Data
    QCARCAM_EVENT_VENDOR                 | QCarCamVendorParam_t    |
    QCARCAM_EVENT_PROPERTY_NOTIFY        | QCarCamParamType_e      |  u32Data
    QCARCAM_EVENT_FRAME_SOF              | QCarCamHWTimestamp_t    |
    QCARCAM_EVENT_RECOVERY               | QCarCamRecovery_t       |

 * @endverbatim
*/
typedef union
{
    uint32_t             u32Data;           /**< uint32_t type */
    QCarCamErrorInfo_t   errInfo;           /**< Error info type */
    QCarCamHWTimestamp_t hwTimestamp;       /**< HW timestamp */
    QCarCamVendorParam_t vendorData;        /**< vendor data paylaod. */
    QCarCamFrameInfo_t   frameInfo;         /**< Frame info. */
    QCarCamRecovery_t    recovery;          /**< recovery payload. */
    uint8_t array[QCARCAM_MAX_PAYLOAD_SIZE];   /**< max event payload. */
} QCarCamEventPayload_t;

/** @brief Exposure configuration parameter
 *  T1 exposure is the longest and each successive exposure being shorter (T1 > T2 > T3 > T4).
 *  The indices in exposureTime and exposureRatio arrays 0,1,2,3 correspond to T1, T2, T3, T4
*/
typedef struct
{
    QCarCamExposureMode_e mode;   /**< exposure mode. */
    uint32_t hdrMode;             /**< hdr mode. */
    uint32_t numExposures;        /**< number of hdr exposures. */
    float exposureTime[QCARCAM_HDR_NUM_EXPOSURES];  /**< time in ms. */
    float exposureRatio[QCARCAM_HDR_NUM_EXPOSURES]; /**< ratio going for successive exposures (T1/T2, T2/T3, T3/T4). */
    float gain[QCARCAM_HDR_NUM_EXPOSURES];           /**< 1.0 to Max supported in sensor. */
}QCarCamExposureConfig_t;

/** @brief Gamma Configuration parameter */
typedef struct
{
    QCarCamGammaType_e type;  /**< gamma configuration mode. */
    float fpValue;            /**< gamma exponent. */
    uint32_t table[QCARCAM_MAX_GAMMA_TABLE]; /**< gamma table. */
    uint32_t tableSize;       /**< size of gamma table. */
}QCarCamGammaConfig_t;

/** @brief Frame drop config parameters */
typedef struct
{
    uint8_t frameDropPeriod;      /**< number of bits in pattern (min value of 1. max value of 32). */
    uint32_t frameDropPattern;    /**< active bit pattern. value of 1 to keep frame, value of 0 to drop frame. */
}QCarCamFrameDropConfig_t;

/** @brief Batch mode configuration */
typedef struct
{
    QCarCamBatchMode_e mode;   /**< batch mode. */
    uint32_t numBatchFrames;   /**< number of frames in batch. */
    uint32_t frameIncrement;   /**< offset in bytes from frame N's first pixel to frame N+1's first pixel. */
    uint32_t detectFirstPhaseTimer; /**< For special case that requires batching to be started based on specific time difference. */
}QCarCamBatchConfig_t;


/** @} */ /* end_addtogroup qcarcam_datatypes */

/**
 * @cond QCarCamEventCallback_t @endcond
 *
 * @ingroup qcarcam_functions
 *
 * @brief Callback function for handling events.
 *
 * @param[out] hndl          Handle of the input.
 * @param[out] eventId       ID of the event that has resulted from the QCARCAM_EVENT_* macro.
 * @param[out] pPayload      Event payload data associated with the event ID.
 * @param[out] pPrivateData  Handle provided by the client.
 *
 * @detdesc
 * The QCarCam client library calls using this event handler callback whenever any event occurs in
 * the QCarCam system. The client should provide this callback via QCarCamRegisterEventCallback().
 * @par
 * For the pPayload associated with each eventId, check the table provided with
 * #QCarCamEventPayload_t.
 *
 * @return #QCARCAM_RET_OK if successful.
 */
typedef QCarCamRet_e (*QCarCamEventCallback_t)(
    const QCarCamHndl_t hndl,
    const uint32_t eventId,
    const QCarCamEventPayload_t *pPayload,
    void  *pPrivateData);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* QCARCAM_TYPES_H */

#ifndef _AIS_COMM_H_
#define _AIS_COMM_H_

/**
 * @file ais_comm.h
 *
 * @brief defines all structures and definition between client and server.
 *
 * Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "CameraResult.h"

#include "ais.h"

#include <sys/types.h>

#if defined(__ANDROID__)
#include <sys/stat.h>
#include <errno.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/** ais version */
/** ADD SUPPORTED VERSION, CHANGE THE ARRAY OF sg_ais_supported_version FOR BACKWARD COMPATIBILITY */

#define AIS_VERSION_MAJOR QCARCAM_VERSION_MAJOR
#define AIS_VERSION_MINOR QCARCAM_VERSION_MINOR

#define AIS_VERSION_GET_MAJOR(ver) (((ver) >> 8) & 0xFF)
#define AIS_VERSION_GET_MINOR(ver) ((ver) & 0xFF)

#define AIS_VERSION                     QCARCAM_VERSION
#define AIS_VERSION_MINIMUM_COMPATIBLE  QCARCAM_VERSION_MINIMUM_COMPATIBLE

/**
 * ais temporary file path with write/read permission
 */
#if defined(__QNXNTO__)
#define AIS_SOCKET_PATH "/tmp"
#define AIS_TEMP_PATH "/tmp"
#elif defined(__ANDROID__)
#ifndef AIS_EARLYSERVICE
#define AIS_SOCKET_PATH "/dev/socket/camera"
#define AIS_TEMP_PATH "/dev/socket/camera"
#else
#define AIS_SOCKET_PATH "/early_services/dev/socket/camera"
#define AIS_TEMP_PATH "/early_services/dev/socket/camera"
#endif
#elif defined(__AGL__)
#define AIS_SOCKET_PATH "/tmp"
#define AIS_TEMP_PATH "/tmp"
#elif defined(_LINUX) || defined(CAMERA_UNITTEST)
#define AIS_SOCKET_PATH "/tmp/camera"
#define AIS_TEMP_PATH "/tmp/camera"
#endif


/** indicator of one context which has been allocated */
#define AIS_CONTEXT_IN_USE ((QCarCamHndl_t)-1)

/** maximum number of event connections */
#define AIS_CONN_EVENT_MAX_NUM 1

/** maximum number of command connections */
#define AIS_CONN_CMD_MAX_NUM 2

/** index for command connections */
#define AIS_CONN_CMD_IDX_MAIN 0
#define AIS_CONN_CMD_IDX_WORK 1


/** maximum number of all connections */
#define AIS_CONN_MAX_NUM (AIS_CONN_CMD_MAX_NUM + AIS_CONN_EVENT_MAX_NUM)


#define AIS_CONN_EVENT_NUM(cnt) (AIS_CONN_EVENT_MAX_NUM)

/** count of connection for commands */
#define AIS_CONN_CMD_NUM(cnt) ((cnt) - AIS_CONN_EVENT_MAX_NUM)

/** event connection ID */
#define AIS_CONN_ID_EVENT(id) ((id) + 0)

/** command connection ID */
#define AIS_CONN_ID_CMD_MAIN(id) ((id) + 1)
#define AIS_CONN_ID_CMD_WORK(id) ((id) + 2)


/** delay between health checks */
#define HEALTH_CHECK_DELAY_MSEC 200
/** max number of attempts for signal to be set */
#define MAX_HEALTH_SIGNAL_ATTEMPTS 20


/** health message event ID */
#define EVENT_HEALTH_MSG 0xA150C0EE


/**
 * command id
 * IF A NEW COMMAND IS ADDED, PLEASE ADD A NEW COMMAND PARAMETER
 * AND PUT THIS PARAMETER IN COMMAND PARAMETER HOLDER
 */
typedef enum
{
    AIS_CMD_QUERY_INPUTS = 0, /**< query inputs */
    AIS_CMD_QUERY_INPUT_MODES,/**< query input modes */
    AIS_CMD_OPEN,             /**< open */
    AIS_CMD_CLOSE,            /**< close */
    AIS_CMD_G_PARAM,          /**< g_param */
    AIS_CMD_S_PARAM,          /**< s_param */
    AIS_CMD_S_BUFFERS,        /**< s_buffers */
    AIS_CMD_S_BUFFERS_V2,     /**< s_buffers_v2 */
    AIS_CMD_START,            /**< start */
    AIS_CMD_STOP,             /**< stop */
    AIS_CMD_PAUSE,            /**< pause */
    AIS_CMD_RESUME,           /**< resume */
    AIS_CMD_GET_FRAME,        /**< get_frame */
    AIS_CMD_RELEASE_FRAME,    /**< release_frame */

    AIS_CMD_GET_EVENT, // not supported currently

    //INTERNAL USE
    //sub transactions for s_buffers
    AIS_CMD_S_BUFFERS_START,
    AIS_CMD_S_BUFFERS_TRANS,
    AIS_CMD_S_BUFFERS_END,

    AIS_CMD_HEALTH,
    AIS_CMD_QUERY_DIAGNOSTICS,
    AIS_CMD_UNINITIALIZE,      /**< uninitialize */
    AIS_CMD_QUERY_INPUTS_V2,
    AIS_CMD_GET_FRAME_V2,
    AIS_CMD_RELEASE_FRAME_V2,
    AIS_CMD_REGISTER_CB,      /**< register callback */
    AIS_CMD_UNREGISTER_CB,     /**< unregister callback */
    AIS_CMD_SUBMIT_REQUEST,
} e_ais_cmd;

#define AIS_CONN_FLAG_HEALTH_CONN (1 << 0)

/**
 * connection information to be exchanged between client/server
 */
typedef struct
{
    int id;                     /**< connection id */
    int cnt;                    /**< number of connections */
    unsigned int gid;           /**< group id of client*/
    unsigned int pid;           /**< process id of client */
    unsigned int app_version;   /**< QCarCam API version of application */
    unsigned int version;       /**< QCarCam API version of client lib */
    CameraResult result;        /**< result of exchange */
    unsigned int flags;         /**< flags for new connection */
} s_ais_conn_info;


/**
 * event structures
 */
typedef struct
{
    uint32_t event_id;         /**< event id */
    QCarCamEventPayload_t payload;  /**< event payload */

    unsigned int event_cnt;           /**< event counter, debug purpuse */
} s_ais_event;


/**
 * command parameter header
 * IT MUST BE IN THE FIRST PLACE FOR ALL COMMANDS' PARAMETERS
 */
typedef struct
{
    int          cmd_id;   /**< command id */
    CameraResult result;   /**< result of current command */

} s_ais_cmd_header;

/**
 * command parameter for query_inputs
 */
typedef struct
{
    int          cmd_id;
    CameraResult result;

    int is_p_inputs_set;
    unsigned int size;
    QCarCamInput_t inputs[MAX_NUM_AIS_INPUTS];
    unsigned int ret_size;

} s_ais_cmd_query_inputs;

/**
 * command parameter for query_inputs
 */
typedef struct
{
    int          cmd_id;
    CameraResult result;

    unsigned int input_id;
    unsigned int current_mode;
    unsigned int num_modes;
    QCarCamMode_t modes[MAX_NUM_AIS_INPUT_MODES];
} s_ais_cmd_query_input_modes;


/**
 * command parameter for query_diagnostics
 */
typedef struct
{
    int cmd_id;
    CameraResult result;

    void* ais_diag_info;
    unsigned int diag_size;
}s_ais_cmd_query_diagnostics;

/**
 * command parameter for open
 */
typedef struct
{
    int          cmd_id;
    CameraResult result;

    QCarCamOpen_t openParams;

    QCarCamHndl_t handle;
} s_ais_cmd_open;

/**
 * command parameter for close
 */
typedef struct
{
    int          cmd_id;
    CameraResult result;

    QCarCamHndl_t handle;
} s_ais_cmd_close;

#if 0
/// @brief Union to hold possible values to p_value in qcarcam_s_param and qcarcam_g_param
typedef union
{
    uint32_t u32Data;                         ///< unsigned int type
    uint64_t u64Data;                 ///< unsigned uint64 value
    float fpData;                               ///< float type
    QCarCamExposureConfig_t exposureConfig;       ///< Exposure settings
    QCarCamExposureConfig_t hdrExposureConfig; ///< HDR Exposure settings
    QCarCamGammaConfig_t gammaConfig;             ///< Gamma settings
    QCarCamFrameDropConfig_t frame_rate_config;          ///< Frame rate settings
    QCarCamIspCtrlsConfig_t ispCtrls;             ///< Used to control isp sensor settings
    QCarCamVendorParam_t vendorParam;             ///< vendor param
    int reserved[QCARCAM_MAX_PAYLOAD_SIZE];       ///< Used to ensure union size won't change
}QCarCamParamValue_t;
#endif

#define PARAM_DATA_MAX_SIZE QCARCAM_MAX_PAYLOAD_SIZE

/**
 * command parameter for g_param
 */
typedef struct
{
    int          cmd_id;
    CameraResult result;

    QCarCamHndl_t handle;
    QCarCamParamType_e param;

    unsigned int size;
    unsigned char data[PARAM_DATA_MAX_SIZE];
} s_ais_cmd_g_param;

/**
 * command parameter for s_param
 */
typedef struct
{
    int          cmd_id;
    CameraResult result;

    QCarCamHndl_t handle;
    QCarCamParamType_e param;

    unsigned int size;
    unsigned char data[PARAM_DATA_MAX_SIZE];
} s_ais_cmd_s_param;

/**
 * command parameter for s_buffers
 */
typedef struct
{
    int          cmd_id;
    CameraResult result;

    QCarCamHndl_t handle;
    QCarCamBufferList_t buffers;
    QCarCamBuffer_t buffer[QCARCAM_MAX_NUM_BUFFERS];
} s_ais_cmd_s_buffers;

/**
 * command parameter for start
 */
typedef struct
{
    int          cmd_id;
    CameraResult result;

    QCarCamHndl_t handle;
} s_ais_cmd_start;

/**
 * command parameter for stop
 */
typedef struct
{
    int          cmd_id;
    CameraResult result;

    QCarCamHndl_t handle;
} s_ais_cmd_stop;

/**
 * command parameter for pause
 */
typedef struct
{
    int          cmd_id;
    CameraResult result;

    QCarCamHndl_t handle;
} s_ais_cmd_pause;

/**
 * command parameter for resume
 */
typedef struct
{
    int          cmd_id;
    CameraResult result;

    QCarCamHndl_t handle;
} s_ais_cmd_resume;

/**
 * command parameter for get_frame
 */
typedef struct
{
    int          cmd_id;
    CameraResult result;

    QCarCamHndl_t handle;
    unsigned long long int timeout;
    unsigned int flags;

    QCarCamFrameInfo_t frame_info;
} s_ais_cmd_get_frame;

/**
 * command parameter for release_frame
 */
typedef struct
{
    int          cmd_id;
    CameraResult result;

    QCarCamHndl_t handle;
    unsigned int idx;
} s_ais_cmd_release_frame;

/**
 * command parameter for release_frame_v2
 */
typedef struct
{
    int          cmd_id;
    CameraResult result;

    QCarCamHndl_t handle;
    unsigned int id;
    unsigned int idx;
} s_ais_cmd_release_frame_v2;

/**
 * command parameter for submit_request
 */
typedef struct
{
    int          cmd_id;
    CameraResult result;

    QCarCamHndl_t handle;
    QCarCamRequest_t request;
} s_ais_cmd_submit_request;

/**
 * command parameter for get_event
 */
typedef struct
{
    int          cmd_id;
    CameraResult result;

    QCarCamHndl_t handle;

    s_ais_event event;
} s_ais_cmd_get_event;


/**
 * command parameter for register callback
 */
typedef struct
{
    int          cmd_id;
    CameraResult result;

    QCarCamHndl_t handle;
} s_ais_cmd_register_cb;

/**
 * command parameter holder for all commands
 */
typedef union
{

    s_ais_cmd_header           header;

    s_ais_cmd_query_inputs     query_inputs;
    s_ais_cmd_query_input_modes query_input_modes;

    s_ais_cmd_open             open;
    s_ais_cmd_close            close;

    s_ais_cmd_g_param          g_param;
    s_ais_cmd_s_param          s_param;
    s_ais_cmd_s_buffers        s_buffers;

    s_ais_cmd_start            start;
    s_ais_cmd_stop             stop;
    s_ais_cmd_pause            pause;
    s_ais_cmd_resume           resume;

    s_ais_cmd_get_frame        get_frame;
    s_ais_cmd_release_frame    release_frame;
    s_ais_cmd_release_frame_v2 release_frame_v2;

    s_ais_cmd_get_event        get_event;

    s_ais_cmd_query_diagnostics query_diagnostics;

    s_ais_cmd_register_cb      register_cb;

    s_ais_cmd_submit_request   submit_req;

} u_ais_cmd;

#ifdef __cplusplus
}
#endif


#endif //_AIS_COMM_H_

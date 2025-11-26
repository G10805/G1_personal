#ifndef CAPI_V2_EXTN_H
#define CAPI_V2_EXTN_H

/* ========================================================================*/
/*
 * Copyright (c) 2018-2019, 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
/* ========================================================================*/
/*
 @file capi_v2_extn.h
 Common Audio Processing Interface v2 header file

 This file defines the types and structures that are used by the capi wrappers
 using the CAPI_V2 interface.
 */
/* ========================================================================*/


typedef enum PDK_STAGE2_STRUCT_ID {
    SVA_ID_CONFIG,
    SVA_ID_VERSION,
    SVA_ID_MEM_CONFIG,
    SVA_ID_SCRATCH_PARAM,
    SVA_ID_THRESHOLD_CONFIG,  /** threshold configuration */
    SVA_ID_REINIT_ALL,        /** initialize all internal variables */
    SVA_ID_REINIT_NETWORK,    /** initialize network layers */
    SVA_ID_RESULT,            /** retrieve detection result */
    SVA_ID_RMS_THRESHOLD_CONFIG,  /** RMS threshold configuration */
    SVA_ID_END_DETECTION_CONFIG,  /** keyword end detection configuration */
    SVA_ID_THRESHOLD_TABLE_CONFIG, /** threshold table */
} SVA_STRUCT_ID;

typedef struct pdk_stage2_threshold_config {
    uint32_t struct_size;
    int32_t smm_threshold;
} sva_threshold_config_t;


typedef struct pdk_stage2_rms_threshold_config {
    uint32_t struct_size;
    int32_t rms_threshold;
} sva_rms_threshold_config_t;

typedef struct pdk_stage2_result {
    uint32_t struct_size;
    uint32_t reserved;
    int32_t is_detected;
    int32_t best_confidence;
    int32_t start_position;
    int32_t end_position;  // relative position from current frame
    int32_t current_confidence;
} sva_result_t;

typedef struct pdk_stage2_config {
    uint32_t struct_size;
    uint32_t sample_rate;
    uint32_t frame_samples;
} sva_config_t;

typedef struct pdk_stage2_memory_config {
    uint32_t struct_size;
    uint32_t lib_static_mem_size;
    uint32_t lib_scratch_mem_size;
    uint32_t lib_stack_mem_size;
} sva_memory_config_t;

typedef struct pdk_stage2_end_detection_frames_config {
    uint32_t struct_size;
    int32_t num_ending_frames;
} sva_end_detection_frames_config_t;


typedef enum VOICEPRINT2_STRUCT_IDS {
    VOICEPRINT2_ID_VERSION = 0, // refers to voiceprint2_ver_param_t in voiceprint2_calibration_api.h
    VOICEPRINT2_ID_MEMORY,
    VOICEPRINT2_ID_ENGINE_CONFIG,    // refer to voiceprint2_config_t
    VOICEPRINT2_ID_REINIT,
    VOICEPRINT2_ID_INPUT_DATA,    // refer to voiceprint2_input_data_t
    VOICEPRINT2_ID_RESULT,    // refers to voiceprint2_result_t in voiceprint2_calibration_api.h
    VOICEPRINT2_ID_SVA_UV_SCORE,    // refer to voiceprint2_sva_uv_score_t
    VOICEPRINT2_ID_THRESHOLD_CONFIG,
    VOICEPRINT2_STRUCT_IDS_END,
}VOICEPRINT2_STRUCT_IDS;

/*
voiceprint2_result_t

voiceprint2_result_t is used to get result of voiceprint processing.

is_detected:            [OUT]: detection result

vop2_user_score:        [OUT]: User verification vop_score

combined_user_score:    [OUT]: combined user score with SVA UV score

antispoofing_score:     [OUT]: Antispoofing score

is_antispoofing_passed: [OUT]: decision for antispoofing for debugging purposes

is_user_verification_passed: [OUT]: decision for user verification for debugging purposes
*/
typedef struct voiceprint2_result {
    int32_t is_detected;
    float vop2_user_score;
    float combined_user_score;
    int32_t is_user_verification_passed;
} voiceprint2_result_t;

typedef int8_t voiceprint2_t;


/* voiceprint2_ver_param_t

This structure defines the parameters associated with VOICEPRINT2_ID_VERSION in enum VOICEPRINT2_STRUCT_IDS

Supported only for get_param(...) api

size: [IN/OUT]: IN : Caller fills size  for the size of the payload structure, expects size >= sizeof(voiceprint2_ver_param_t)
                OUT: Callee fills the appropriate size of the buffer filled which is sizeof(voiceprint2_ver_param_t)

   */
typedef struct voiceprint2_ver_param {
    uint32_t version_msb;
    uint32_t version_lsb;
} voiceprint2_ver_param_t;


/* voiceprint2_sva_uv_score_t

This structure defines the parameters associated with VOICEPRINT2_SVA_UV_SCORE in enum VOICEPRINT2_STRUCT_IDS

Supported only for set_param(...) api

sva_uv_confidence_score: [IN]: IN : Caller fills the value of sva_uv_confidence_score

   */
typedef struct voiceprint2_sva_uv_score {
    float sva_uv_confidence_score;
} voiceprint2_sva_uv_score_t;


/* voiceprint2_threshold_config_t

This structure defines the parameters associated with VOICEPRINT2_THRESHOLD_CONFIG in enum VOICEPRINT2_STRUCT_IDS

Supported only for set_param(...) api

user_verification_enabled:      [IN]: Caller fills whether user_verification enable / disable (1/0)
anti_spoofing_enabled:          [IN]: Caller fills whether anti_spoofing enable / disable (1/0)
user_verification_threshold:    [IN]: Caller fills user verification threshold
anti_spoofing_threshold:        [IN]: Caller fills anti spoofing threshold
 */
typedef struct voiceprint2_threshold_config {
    float user_verification_threshold;
} voiceprint2_threshold_config_t;


/*
voiceprint2_memory_t

voiceprint2_memory_t is the library structure encapsulating memory details of the library.
Memory for this structure has to be allocated by Callee before calling any of the library apis

Note: each structure definition should have a specific id in VOICEPRINT2_STRUCT_IDS

struct_size:               [IN]: Filled by the caller, shows the size of the structure

lib_static_mem_size:   [IN/OUT]: Size of library static memory required in bytes, this memory
                        --[OUT]: Calle fills for api voiceprint2_api_get_req, [IN]: Caller fills for api voiceprint2_api_init

lib_scratch_mem_size:  [IN/OUT]: Size of library Scratch memory required in bytes, this memory
                        --[OUT]: Calle fills for api voiceprint2_api_get_req, [IN]: Caller fills for api voiceprint2_api_init

lib_stack_size:           [OUT]: Size of the stack needed by library. [out]: for api voiceprint2_api_get_req, Don't care for api voiceprint2_api_init

   */
typedef struct voiceprint2_memory {
    uint32_t lib_static_mem_size;
    uint32_t lib_scratch_mem_size;
    uint32_t lib_mem_size;
    uint32_t lib_stack_mem_size;
} voiceprint2_memory_t;


/*
voiceprint2_config_t

voiceprint2_config_t is the library structure encapsulating library engine configuration

struct_size:     [IN]: size of the structure, filled by the caller

sample_rate:     [IN]: sampling rate of input audio signal, filled by the caller

user_model_size: [IN]: size of user model size, filled by the caller

p_user_model:    [IN]: pointer to user model buffer, filled by the caller
   */
typedef struct voiceprint2_config {
    uint32_t sample_rate;
    uint32_t user_model_size;
    uint8_t *p_user_model;
} voiceprint2_config_t;

typedef enum _STAGE2_UV_WRAPPER_STRUCT_ID {
    STAGE2_UV_WRAPPER_ID_VERSION = 0,
    STAGE2_UV_WRAPPER_ID_CONFIG,
    STAGE2_UV_WRAPPER_ID_MEM_INFO,
    STAGE2_UV_WRAPPER_ID_SCRATCH_PARAM,
    STAGE2_UV_WRAPPER_ID_INMODEL_BUFFER_SIZE,
    STAGE2_UV_WRAPPER_ID_MODEL_TYPE,
    STAGE2_UV_WRAPPER_ID_SCORE_PARAM,
    STAGE2_UV_WRAPPER_ID_THRESHOLD,
    STAGE2_UV_WRAPPER_ID_THRESHOLD_TYPE,
    STAGE2_UV_WRAPPER_ID_THRESHOLD_TABLE,
    STAGE2_UV_WRAPPER_ID_SVA_UV_SCORE,
    STAGE2_UV_WRAPPER_ID_REINIT,
    STAGE2_UV_WRAPPER_ID_RESULT,
    STAGE2_UV_WRAPPER_ID_RELEASE,
    STAGE2_UV_WRAPPER_ID_ENROLLMENT,
    STAGE2_UV_WRAPPER_ID_MATCHING,
    STAGE2_UV_WRAPPER_ID_INFO_ALL,
}STAGE2_UV_WRAPPER_STRUCT_ID;

/*
stage2_uv_wrapper_result

stage2_uv_wrapper_result is used to get result of user-verification processing.

is_detected:            [OUT]: detection result

is_antispoofing_passed: [OUT]: decision for antispoofing for debugging purposes

final_user_score:       [OUT]: combined user score with SVA UV score

vop2_user_score:        [OUT]: User verification vop_score

gmmsva_user_score:      [OUT]: User verification gmm_score

antispoofing_score:     [OUT]: Antispoofing score

*/
typedef struct stage2_uv_wrapper_result {
    uint32_t reserved[8];
    int32_t is_detected;
    int32_t is_anti_spoofing_passed;
    int32_t final_user_score;
    float vop2_user_score;
    float gmmsva_user_score;
    float antispoofing_score;
} stage2_uv_wrapper_result_t;

/*
stage2_uv_wrapper_stage1_uv_score_t

This structure defines the parameters associated with STAGE2_UV_WRAPPER_ID_SVA_UV_SCORE in enum STAGE2_UV_WRAPPER_STRUCT_ID

Supported only for set_param(...) api

stage1_uv_score: [IN]: IN : Caller fills the value of stage1_uv_score

*/

typedef struct stage2_uv_wrapper_stage1_uv_score {
    int32_t stage1_uv_score;
} stage2_uv_wrapper_stage1_uv_score_t;

/*
stage2_uv_wrapper_scratch_param_t
This structure used to allocate scratch memory after UV model initializaion.
*/

typedef struct stage2_uv_wrapper_scratch_param {
    uint32_t reserved;
    uint32_t scratch_size;
    int8_t *scratch_ptr;
} stage2_uv_wrapper_scratch_param_t;

/*
stage2_uv_wrapper_threshold_config_t

This structure defines the parameters associated with STAGE2_UV_WRAPPER_THRESHOLD_CONFIG
in enum STAGE2_UV_WRAPPER_STRUCT_ID

Supported only for set_param(...) api

threshold:                      [IN]: Caller fills user verification threshold
anti_spoofing_enabled:          [IN]: Caller fills whether anti_spoofing enable / disable (1/0)
anti_spoofing_threshold:        [IN]: Caller fills anti spoofing threshold
*/

typedef struct stage2_uv_wrapper_threshold_config {
    uint32_t reserved[2];
    int32_t threshold;
    int32_t anti_spoofing_enabled;
    float anti_spoofing_threshold;
} stage2_uv_wrapper_threshold_config_t;

/*
stage2_uv_wrapper_config_t

stage2_uv_wrapper_config_t is the library structure encapsulating library engine configuration

sample_rate:     [IN]: sampling rate of input audio signal, filled by the caller

user_model_size: [IN]: size of user model size, filled by the caller

p_user_model:    [IN]: pointer to user model buffer, filled by the caller
*/
typedef struct stage2_uv_wrapper_config {
    uint32_t sample_rate;
    uint32_t user_model_size;
    uint8_t *p_user_model;
} stage2_uv_wrapper_config_t;


#endif /* #ifndef CAPI_V2_EXTN_H */

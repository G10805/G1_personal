#ifndef _AIS_H_
#define _AIS_H_

/* ===========================================================================
 * Copyright (c) 2016-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @file ais.h
 * @brief AIS API - QTI Automotive Imaging System Proprietary API
 *
=========================================================================== */

#include "qcarcam.h"
#include "CameraResult.h"

/*need at least 3 buffers (2 at VFE and 1 empty)*/
#define MIN_USR_NUMBER_OF_BUFFERS 3
#define MIN_USR_NUMBER_OF_INPUT_BUFFERS 1

#define MAX_NUM_AIS_INPUTS 16

/** maximum number of clients supported */
#define AIS_MAX_USR_CONTEXTS 64

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
/// ais_initialize flag bits
///////////////////////////////////////////////////////////////////////////////
/* Flag to defer the detection of input devices to event thread. As a result,
 * the engine will be in ready state earlier while detection is proceeding in
 * parallel. The engine will return BUSY if ais_query_inputs is called during
 * detection.
 * */
#define AIS_INITIALIZE_DEFER_INPUT_DETECTION 1 << 0

/* Flag to control power suspend/resume bridge in granular */
#define AIS_INITIALIZE_POWER_INPUT_GRANULAR  1 << 1

///////////////////////////////////////////////////////////////////////////////
/// ais_initialize
///
/// @brief Initialize AIS engine
///
/// @param initialization parameters
///
/// @return CAMERA_SUCCESS if successful;
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_initialize(qcarcam_init_t* p_init_params);

///////////////////////////////////////////////////////////////////////////////
/// ais_uninitialize
///
/// @brief Uninitialize AIS engine
///
/// @return CAMERA_SUCCESS if successful;
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_uninitialize(void);


///////////////////////////////////////////////////////////////////////////////
/// ais_query_inputs / ais_query_inputs_v2
///
/// @brief Queries available inputs. To get the number of available inputs to query, call with p_inputs set to NULL.
///
/// @param p_inputs   Pointer to array inputs. If NULL, then ret_size returns number of available inputs to query
/// @param size       Number of elements in array
/// @param ret_size   If p_inputs is set, number of elements in array that were filled
///                   If p_inputs is NULL, number of available inputs to query
///
/// @return CAMERA_SUCCESS if successful.
///         CAMERA_EITEMBUSY if engine is in detection.
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_query_inputs(qcarcam_input_t* p_inputs, unsigned int size, unsigned int* ret_size);
CameraResult ais_query_inputs_v2(qcarcam_input_v2_t* p_inputs, unsigned int size, unsigned int* ret_size);

///////////////////////////////////////////////////////////////////////////////
/// ais_query_diagnsotics
///
/// @brief Queries the system diagnostic Info.
///
/// @param p_diag_info   Pointer to diagnostic info.
/// @param diag_size     Size of user allocated memory
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_query_diagnostics(void *p_diag_info, unsigned int diag_size);

///////////////////////////////////////////////////////////////////////////////
/// ais_open
///
/// @brief Opens handle to input
///
/// @param desc   Unique identifier of input to be opened
///
/// @return NOT NULL if successful; NULL on failure
///////////////////////////////////////////////////////////////////////////////
qcarcam_hndl_t ais_open(qcarcam_input_desc_t desc);

///////////////////////////////////////////////////////////////////////////////
/// ais_close
///
/// @brief Closes handle to input
///
/// @param hndl   Handle of input that was opened
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_close(qcarcam_hndl_t hndl);

///////////////////////////////////////////////////////////////////////////////
/// ais_g_param
///
/// @brief Get parameter value
///
/// @param hndl     Handle of input
/// @param param    Parameter to get
/// @param p_value  Pointer to structure of value that will be retrieved
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_g_param(qcarcam_hndl_t hndl, qcarcam_param_t param, qcarcam_param_value_t* p_value);

///////////////////////////////////////////////////////////////////////////////
/// ais_s_param
///
/// @brief Set parameter
///
/// @param hndl     Handle of input
/// @param param    Parameter to set
/// @param p_value  Pointer to structure of value that will be set
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_s_param(qcarcam_hndl_t hndl, qcarcam_param_t param, const qcarcam_param_value_t* p_value);

///////////////////////////////////////////////////////////////////////////////
/// ais_s_buffers
///
/// @brief Set buffers
///
/// @param hndl       Handle of input
/// @param p_buffers  Pointer to set buffers structure
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_s_buffers(qcarcam_hndl_t hndl, qcarcam_buffers_t* p_buffers);

///////////////////////////////////////////////////////////////////////////////
/// ais_s_buffers_v2
///
/// @brief Set buffers for specifc buffer list
///
/// @param hndl       Handle of input
/// @param p_bufferlist  Pointer to bufferlist
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_s_buffers_v2(qcarcam_hndl_t hndl, const qcarcam_bufferlist_t* p_bufferlist);

///////////////////////////////////////////////////////////////////////////////
/// ais_start
///
/// @brief Start input
///
/// @param hndl       Handle of input
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_start(qcarcam_hndl_t hndl);

///////////////////////////////////////////////////////////////////////////////
/// ais_stop
///
/// @brief Stop input that was started
///
/// @param hndl       Handle of input
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_stop(qcarcam_hndl_t hndl);

///////////////////////////////////////////////////////////////////////////////
/// ais_pause
///
/// @brief Pause input that was started. Does not relinquish resource
///
/// @param hndl       Handle of input
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_pause(qcarcam_hndl_t hndl);

///////////////////////////////////////////////////////////////////////////////
/// ais_resume
///
/// @brief Resumes input that was paused
///
/// @param hndl       Handle of input
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_resume(qcarcam_hndl_t hndl);

///////////////////////////////////////////////////////////////////////////////
/// ais_get_frame
///
/// @brief Get available frame
///
/// @param hndl          Handle of input
/// @param p_frame_info  Pointer to frame information that will be filled
/// @param timeout       Max wait time in ns for frame to be available before timeout
/// @param flags         Flags
///
/// @return CAMERA_SUCCESS if successful; CAMERA_EEXPIRED if timeout
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_get_frame(qcarcam_hndl_t hndl, qcarcam_frame_info_t* p_frame_info,
        unsigned long long int timeout, unsigned int flags);

///////////////////////////////////////////////////////////////////////////////
/// ais_get_frame_v2
///
/// @brief Get available frame
///
/// @param hndl          Handle of input
/// @param p_frame_info  Pointer to frame information that will be filled
/// @param timeout       Max wait time in ns for frame to be available before timeout
/// @param flags         Flags
///
/// @return CAMERA_SUCCESS if successful; CAMERA_EEXPIRED if timeout
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_get_frame_v2(qcarcam_hndl_t hndl, qcarcam_frame_info_v2_t* p_frame_info,
        unsigned long long int timeout, unsigned int flags);

///////////////////////////////////////////////////////////////////////////////
/// ais_release_frame
///
/// @brief Re-enqueue frame buffers
///
/// @param hndl       Handle of input
/// @param idx        Index into the qcarcam_buffers_t buffers table to reenqueue
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_release_frame(qcarcam_hndl_t hndl, unsigned int idx);

///////////////////////////////////////////////////////////////////////////////
/// ais_release_frame_v2
///
/// @brief Re-enqueue frame buffers
///
/// @param hndl       Handle of input
/// @param id         bufferlist id
/// @param idx        Index into the qcarcam_buffers_t buffers table to reenqueue
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_release_frame_v2(qcarcam_hndl_t hndl, unsigned int id, unsigned int idx);

#ifdef __cplusplus
};
#endif


#ifdef __cplusplus

class AisEngineBase
{
protected:
    virtual ~AisEngineBase(){};

public:
    ///////////////////////////////////////////////////////////////////////////////
    /// ais_initialize
    ///
    /// @brief Initialize AIS engine
    ///
    /// @param initialization parameters
    ///
    /// @return CAMERA_SUCCESS if successful;
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_initialize(qcarcam_init_t* p_init_params) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_uninitialize
    ///
    /// @brief Uninitialize AIS engine
    ///
    /// @return CAMERA_SUCCESS if successful;
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_uninitialize(void) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_query_inputs / ais_query_inputs_v2
    ///
    /// @brief Queries available inputs. To get the number of available inputs to query, call with p_inputs set to NULL.
    ///
    /// @param p_inputs   Pointer to array inputs. If NULL, then ret_size returns number of available inputs to query
    /// @param size       Number of elements in array
    /// @param ret_size   If p_inputs is set, number of elements in array that were filled
    ///                   If p_inputs is NULL, number of available inputs to query
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_query_inputs(qcarcam_input_t* p_inputs, unsigned int size, unsigned int* ret_size) = 0;
    virtual CameraResult ais_query_inputs_v2(qcarcam_input_v2_t* p_inputs, unsigned int size, unsigned int* ret_size) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_query_diagnsotics
    ///
    /// @brief Queries the system diagnostic Info.
    ///
    /// @param p_diag_info  Pointer to diagnostic info.
    /// @param diag_size    Size of user allocated memory
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_query_diagnostics(void *p_diag_info, unsigned int diag_size) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_open
    ///
    /// @brief Opens handle to input
    ///
    /// @param desc   Unique identifier of input to be opened
    ///
    /// @return NOT NULL if successful; NULL on failure
    ///////////////////////////////////////////////////////////////////////////////
    virtual qcarcam_hndl_t ais_open(qcarcam_input_desc_t desc) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_close
    ///
    /// @brief Closes handle to input
    ///
    /// @param hndl   Handle of input that was opened
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_close(qcarcam_hndl_t hndl) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_g_param
    ///
    /// @brief Get parameter value
    ///
    /// @param hndl     Handle of input
    /// @param param    Parameter to get
    /// @param p_value  Pointer to structure of value that will be retrieved
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_g_param(qcarcam_hndl_t hndl, qcarcam_param_t param,
            qcarcam_param_value_t* p_value) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_s_param
    ///
    /// @brief Set parameter
    ///
    /// @param hndl     Handle of input
    /// @param param    Parameter to set
    /// @param p_value  Pointer to structure of value that will be set
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_s_param(qcarcam_hndl_t hndl, qcarcam_param_t param,
            const qcarcam_param_value_t* p_value) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_s_buffers
    ///
    /// @brief Set buffers
    ///
    /// @param hndl       Handle of input
    /// @param p_buffers  Pointer to set buffers structure
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_s_buffers(qcarcam_hndl_t hndl, qcarcam_buffers_t* p_buffers) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_s_buffers_v2
    ///
    /// @brief Set buffers for particular buffer list
    ///
    /// @param hndl       Handle of input
    /// @param p_bufferlist  Pointer to bufferlist
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_s_buffers_v2(qcarcam_hndl_t hndl, const qcarcam_bufferlist_t* p_bufferlist) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_start
    ///
    /// @brief Start input
    ///
    /// @param hndl       Handle of input
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_start(qcarcam_hndl_t hndl) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_stop
    ///
    /// @brief Stop input that was started
    ///
    /// @param hndl       Handle of input
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_stop(qcarcam_hndl_t hndl) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_pause
    ///
    /// @brief Pause input that was started. Does not relinquish resource
    ///
    /// @param hndl       Handle of input
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_pause(qcarcam_hndl_t hndl) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_resume
    ///
    /// @brief Resumes input that was paused
    ///
    /// @param hndl       Handle of input
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_resume(qcarcam_hndl_t hndl) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_get_frame
    ///
    /// @brief Get available frame
    ///
    /// @param hndl          Handle of input
    /// @param p_frame_info  Pointer to frame information that will be filled
    /// @param timeout       Max wait time in ns for frame to be available before timeout
    /// @param flags         Flags
    ///
    /// @return CAMERA_SUCCESS if successful; CAMERA_EEXPIRED if timeout
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_get_frame(qcarcam_hndl_t hndl, qcarcam_frame_info_t* p_frame_info,
            unsigned long long int timeout, unsigned int flags) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_get_frame_v2
    ///
    /// @brief Get available frame
    ///
    /// @param hndl          Handle of input
    /// @param p_frame_info  Pointer to frame information that will be filled
    /// @param timeout       Max wait time in ns for frame to be available before timeout
    /// @param flags         Flags
    ///
    /// @return CAMERA_SUCCESS if successful; CAMERA_EEXPIRED if timeout
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_get_frame_v2(qcarcam_hndl_t hndl, qcarcam_frame_info_v2_t* p_frame_info,
            unsigned long long int timeout, unsigned int flags) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_release_frame
    ///
    /// @brief Re-enqueue frame buffers
    ///
    /// @param hndl       Handle of input
    /// @param idx        Index into the qcarcam_buffers_t buffers table to reenqueue
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_release_frame(qcarcam_hndl_t hndl, unsigned int idx) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_release_frame_v2
    ///
    /// @brief Re-enqueue frame buffers
    ///
    /// @param hndl       Handle of input
    /// @param id         bufferlist id
    /// @param idx        Index into the qcarcam_buffers_t buffers table to reenqueue
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_release_frame_v2(qcarcam_hndl_t hndl, unsigned int id, unsigned int idx) = 0;
};

#endif /* __cplusplus */

#endif


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
#define MAX_NUM_AIS_INPUT_MODES 16

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
CameraResult ais_initialize(const QCarCamInit_t* p_init_params);

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
CameraResult ais_query_inputs(QCarCamInput_t* p_inputs, uint32_t size, uint32_t* ret_size);

///////////////////////////////////////////////////////////////////////////////
/// AisQueryInputModes
///
/// @brief Queries available input modes
///
/// @param inputId   input Id queried
/// @param pQueryModes input modes to be filled
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult AisQueryInputModes(uint32 inputId, QCarCamInputModes_t* pQueryModes);

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
CameraResult ais_query_diagnostics(QCarCamDiagInfo *pDiagInfo);

///////////////////////////////////////////////////////////////////////////////
/// ais_open
///
/// @brief Opens handle to input
///
/// @param desc   Unique identifier of input to be opened
///
/// @return NOT NULL if successful; NULL on failure
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_open(const QCarCamOpen_t* pOpenParams, QCarCamHndl_t* pHndl);

///////////////////////////////////////////////////////////////////////////////
/// ais_close
///
/// @brief Closes handle to input
///
/// @param hndl   Handle of input that was opened
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_close(QCarCamHndl_t hndl);

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
CameraResult ais_g_param(QCarCamHndl_t hndl, QCarCamParamType_e param, void* pValue, uint32_t size);

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
CameraResult ais_s_param(QCarCamHndl_t hndl, QCarCamParamType_e param, const void* pValue, uint32_t size);

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
CameraResult ais_s_buffers(QCarCamHndl_t hndl, const QCarCamBufferList_t* p_buffers);


///////////////////////////////////////////////////////////////////////////////
/// ais_start
///
/// @brief Start input
///
/// @param hndl       Handle of input
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_start(QCarCamHndl_t hndl);

///////////////////////////////////////////////////////////////////////////////
/// ais_stop
///
/// @brief Stop input that was started
///
/// @param hndl       Handle of input
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_stop(QCarCamHndl_t hndl);

///////////////////////////////////////////////////////////////////////////////
/// ais_pause
///
/// @brief Pause input that was started. Does not relinquish resource
///
/// @param hndl       Handle of input
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_pause(QCarCamHndl_t hndl);

///////////////////////////////////////////////////////////////////////////////
/// ais_resume
///
/// @brief Resumes input that was paused
///
/// @param hndl       Handle of input
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_resume(QCarCamHndl_t hndl);

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
CameraResult ais_get_frame(QCarCamHndl_t hndl, QCarCamFrameInfo_t* p_frame_info,
        uint64_t timeout, uint32_t flags);

///////////////////////////////////////////////////////////////////////////////
/// ais_release_frame
///
/// @brief Re-enqueue frame buffers
///
/// @param hndl       Handle of input
/// @param idx        Index into the QCarCamBufferList_t buffers table to reenqueue
///
/// @return CAMERA_SUCCESS if successful
///////////////////////////////////////////////////////////////////////////////
CameraResult ais_release_frame(QCarCamHndl_t hndl, uint32_t idx);


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
CameraResult ais_release_frame_v2(QCarCamHndl_t hndl, unsigned int id, unsigned int idx);

CameraResult AisRegisterEventCallback(
    const QCarCamHndl_t hndl,
    const QCarCamEventCallback_t callbackFunc,
    void  *pPrivateData);

CameraResult AisUnRegisterEventCallback(const QCarCamHndl_t hndl);

CameraResult AisSubmitRequest(const QCarCamHndl_t hndl, const QCarCamRequest_t* pRequest);

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
    virtual CameraResult ais_initialize(const QCarCamInit_t* pInitParams) = 0;

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
    virtual CameraResult ais_query_inputs(QCarCamInput_t* p_inputs, uint32_t size, uint32_t* ret_size) = 0;

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
    virtual CameraResult ais_query_diagnostics(QCarCamDiagInfo *pDiagInfo) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_open
    ///
    /// @brief Opens handle to input
    ///
    /// @param desc   Unique identifier of input to be opened
    ///
    /// @return NOT NULL if successful; NULL on failure
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_open(const QCarCamOpen_t* pOpenParams, QCarCamHndl_t* pHndl) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_close
    ///
    /// @brief Closes handle to input
    ///
    /// @param hndl   Handle of input that was opened
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_close(QCarCamHndl_t hndl) = 0;

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
    virtual CameraResult ais_g_param(QCarCamHndl_t hndl, QCarCamParamType_e param,
            void* pValue, uint32_t size) = 0;

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
    virtual CameraResult ais_s_param(QCarCamHndl_t hndl, QCarCamParamType_e param,
            const void* pValue, uint32_t size) = 0;

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
    virtual CameraResult ais_s_buffers(QCarCamHndl_t hndl, const QCarCamBufferList_t* p_buffers) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_start
    ///
    /// @brief Start input
    ///
    /// @param hndl       Handle of input
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_start(QCarCamHndl_t hndl) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_stop
    ///
    /// @brief Stop input that was started
    ///
    /// @param hndl       Handle of input
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_stop(QCarCamHndl_t hndl) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_pause
    ///
    /// @brief Pause input that was started. Does not relinquish resource
    ///
    /// @param hndl       Handle of input
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_pause(QCarCamHndl_t hndl) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_resume
    ///
    /// @brief Resumes input that was paused
    ///
    /// @param hndl       Handle of input
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_resume(QCarCamHndl_t hndl) = 0;

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
    virtual CameraResult ais_get_frame(QCarCamHndl_t hndl, QCarCamFrameInfo_t* pFrameInfo,
            uint64_t timeout, uint32_t flags) = 0;

    ///////////////////////////////////////////////////////////////////////////////
    /// ais_release_frame
    ///
    /// @brief Re-enqueue frame buffers
    ///
    /// @param hndl       Handle of input
    /// @param idx        Index into the QCarCamBufferList_t buffers table to reenqueue
    ///
    /// @return CAMERA_SUCCESS if successful
    ///////////////////////////////////////////////////////////////////////////////
    virtual CameraResult ais_release_frame(QCarCamHndl_t hndl, uint32_t idx) = 0;
    virtual CameraResult ais_release_frame_v2(QCarCamHndl_t hndl, unsigned int id, unsigned int idx) = 0;

    virtual CameraResult AisQueryInputModes(uint32 inputId, QCarCamInputModes_t* pQueryModes) = 0;

    virtual CameraResult AisRegisterEventCallback(
        const QCarCamHndl_t hndl,
        const QCarCamEventCallback_t callbackFunc,
        void  *pPrivateData) = 0;

    virtual CameraResult AisUnRegisterEventCallback(const QCarCamHndl_t hndl) = 0;

    virtual CameraResult AisSubmitRequest(const QCarCamHndl_t hndl, const QCarCamRequest_t* pRequest) = 0;
};

#endif /* __cplusplus */

#endif


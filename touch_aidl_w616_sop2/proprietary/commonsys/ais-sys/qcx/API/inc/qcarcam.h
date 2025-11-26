#ifndef QCARCAM_H
#define QCARCAM_H

/**************************************************************************************************
@file
    qcarcam.h

@brief
    QCarCam API - QTI Automotive Imaging System Proprietary API.

Copyright (c) 2016-2022 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

**************************************************************************************************/

/*=================================================================================================
** INCLUDE FILES FOR MODULE
=================================================================================================*/

#include "qcarcam_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*=================================================================================================
** Constant and Macros
=================================================================================================*/

/** @addtogroup qcarcam_constants
@{ */
	
/** @brief QCarCam major version type. */
#define QCARCAM_VERSION_MAJOR              6

/** @brief QCarCam minor version type. */
#define QCARCAM_VERSION_MINOR              0
	
/** @brief QCarCam minor compatible version type. */
#define QCARCAM_VERSION_MINOR_COMPATIBLE   0

/** @brief QCarCam version type.  

    @hideinitializer
*/
#define QCARCAM_VERSION \
    ((QCARCAM_VERSION_MAJOR << 8) | QCARCAM_VERSION_MINOR)

/** @brief QCarCam minimum compatible version type.

    @hideinitializer
*/
#define QCARCAM_VERSION_MINIMUM_COMPATIBLE \
    ((QCARCAM_VERSION_MAJOR << 8) | QCARCAM_VERSION_MINOR_COMPATIBLE)

/** @} */ /* end_addtogroup qcarcam_constants */

/*=================================================================================================
** API Functions
=================================================================================================*/

/** @addtogroup qcarcam_functions
@{ */


/**
 * @cond QCarCamInitialize @endcond
 *
 * @brief Initializes the QCarCam interface. Must be the first call to the library.
 *
 * @param[in] pInitParams   Structure required for initialization.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_UNSUPPORTED If QCarCam API version mismatch is detected; \n
 * #QCARCAM_RET_NOMEM       If not enough system memory available; \n
 * #QCARCAM_RET_NOT_FOUND   If communication to server cannot be established.
 *
 * @note If communication fails to be established, this typically suggests server is either not
 *       running or not fully initialized to accept clients.
 *
 * @see QCarCamUninitialize()
 */
QCarCamRet_e QCarCamInitialize(const QCarCamInit_t *pInitParams);

/**
 * @cond QCarCamUninitialize @endcond
 *
 * @brief Uninitializes the QCarCam interface. This is the last call to the library.
 *
 * @pre QCarCamInitialize() was called.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet; \n
 * #QCARCAM_RET_TIMEOUT     If server does not respond within a certain time.
 *
 * @see QCarCamInitialize()
 */
QCarCamRet_e QCarCamUninitialize(void);

/**
 * @cond QCarCamQueryInputs @endcond
 *
 * @brief Queries the available input information.
 *
 * @pre QCarCamInitialize() was called.
 *
 * @param[out] pInputs  Pointer to the array inputs. If NULL (along with size = 0), *pRetSize
 *                      returns the number of available inputs.
 * @param[in]  size     Number of elements pointed to by pInputs.
 * @param[out] pRetSize If pInputs is set, it is the number of elements in an array that were
 *                      filled.
 *                      If pInputs is NULL, it is the number of available inputs to query.
 *
 * @detdesc
 * It is recommended to first query the number of inputs available, followed by their actual
 * information query. E.g.,:
 * @par
 * -# Upon successful return of QCarCamQueryInputs() (pInputs = NULL, size = 0, &retSize),
 *    retSize shall contain the number of available inputs.
 * -# Call QCarCamQueryInputs() again with the pInput/size at least large enough to hold 
 *    the number of inputs returned in Step 1. Upon a successful return, pInput will be
 *    filled up to the *pRetSize entries.
 *
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet; \n
 * #QCARCAM_RET_FAILED      If memory operations failed when populating input information. \n
 * #QCARCAM_RET_BUSY        If detection of all available inputs is not yet complete. Will only return available \n
 *                          inputs up to this point in time.
 * @see QCarCamInitialize()
 */
QCarCamRet_e QCarCamQueryInputs(
    QCarCamInput_t *pInputs,
    const uint32_t size,
    uint32_t *pRetSize);

/**
 * @cond QCarCamOpen @endcond
 *
 * @brief Creates a #QCarCamHndl_t session.
 *
 * @pre QCarCamInitialize() was called.
 *
 * @param[in]  pOpenParams  Open parameters for the session.
 * @param[out]  pHndl  Handle to opened session.
 *
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK on success with #QCarCamHndl_t handle filled with valid handle. \n
 * #QCARCAM_RET_BADPARAM  If invalid open parameters are provided \n
 * #QCARCAM_RET_BADSTATE  If QCarCam interface is not initialized. \n
 * #QCARCAM_RET_FAILED    If failure due to other errors such as out of memory
 *
 * @see QCarCamInitialize()
 * @see QCarCamQueryInputs()
 * @see QCarCamClose()
 */
QCarCamRet_e QCarCamOpen(const QCarCamOpen_t* pOpenParams, QCarCamHndl_t* pHndl);

/**
 * @cond QCarCamClose @endcond
 *
 * @brief Closes the #QCarCamHndl_t associated to an input ID.
 *
 * @pre Successful QCarCamOpen() call returning the same handle.
 *
 * @param[in]  hndl    QCarCam handle type to close.
 *
 * @detdesc
 * The call releases resources associated with the handle returned by a successful QCarCamOpen()
 * call.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If input is not opened.
 *
 * @see QCarCamOpen()
 */
QCarCamRet_e QCarCamClose(const QCarCamHndl_t hndl);

/**
 * @cond QCarCamGetParam @endcond
 *
 * @brief Gets the parameter value from an input represented by the #QCarCamHndl_t.
 *
 * @pre Successful QCarCamOpen() call returning the same handle.
 *
 * @param[in]  hndl    Input handle from which to query the parameter.
 * @param[in]  param   Specifies the parameter to get.
 * @param[out] pValue  Points to the data that will be retrieved.
 * @param[in]  size    Memory byte size pointed to by the *pValue; value no bigger than the size of
 *                     the largest data structure documented in #QCarCamParamType_e.
 *
 * @detdesc
 * For the pValue and size parameters, check the table provided with #QCarCamParamType_e.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet; \n
 * #QCARCAM_RET_INVALID_OP  If set parameter request type is invalid; \n
 * #QCARCAM_RET_UNSUPPORTED If size of parameter exceeds maximum supported value; \n
 * #QCARCAM_RET_FAILED      If memory operations failed when populating parameter information.
 *
 * @see QCarCamOpen()
 * @see QCarCamSetParam()
 */
QCarCamRet_e QCarCamGetParam(
    const QCarCamHndl_t hndl,
    const QCarCamParamType_e param,
    void *pValue,
    const uint32_t size);

/**
 * @cond QCarCamSetParam @endcond
 *
 * @brief Sets the parameter value to an input represented by the #QCarCamHndl_t.
 *
 * @pre Successful QCarCamOpen() call returning the same handle.
 *
 * @param[in] hndl    Input handle to which to apply the parameter.
 * @param[in] param   Specifies the parameter to set.
 * @param[in] pValue  Points to the data that will be set.
 * @param[in] size    Memory byte size pointed to by the *pValue; value no bigger than the size of
 *                    the largest data structure documented in #QCarCamParamType_e.
 *
 * @detdesc
 * For the pValue and size parameters, check the table provided with #QCarCamParamType_e.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet; \n
 * #QCARCAM_RET_INVALID_OP  If set parameter request type is invalid.
 * #QCARCAM_RET_UNSUPPORTED If size of parameter exceeds maximum supported value; \n
 * #QCARCAM_RET_FAILED      If memory operations failed when reading parameter information.
 *
 * @see QCarCamOpen()
 * @see QCarCamGetParam()
 */
QCarCamRet_e QCarCamSetParam(
    const QCarCamHndl_t hndl,
    const QCarCamParamType_e param,
    const void *pValue,
    const uint32_t size);

/**
 * @cond QCarCamSetBuffers @endcond
 *
 * @brief Sets the buffers for the input.
 *
 * @pre Successful QCarCamOpen() call returning the same handle.
 *
 * @param[in] hndl         Handle of the input.
 * @param[in] pBuffers     Pointer to set the buffer structure.
 *
 * @detdesc
 * If an input has more than one output buffer based on its input operating mode, individual output
 * buffers will be required to be set for each of the input's selected mode to work.
 * @par
 * This requires multiple calls to QCarCamSetBuffers() per input.
 * @par
 * The maximum number of buffers per input per output mode is defined by #QCARCAM_MAX_NUM_BUFFERS.
 * For ASIL-B operations, the minimum number of buffers per input per output mode is defined
 * by #QCARCAM_MIN_NUM_BUFFERS.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet; \n
 * #QCARCAM_RET_NOMEM       If not enough system memory available; \n
 * #QCARCAM_RET_FAILED      If memory operations failed when reading buffer list provided.
 *
 * @see QCarCamOpen()
 */
QCarCamRet_e QCarCamSetBuffers(const QCarCamHndl_t hndl, const QCarCamBufferList_t *pBuffers);

/**
 * @cond QCarCamStart @endcond
 *
 * @brief Starts the input streaming on the input represented by the #QCarCamHndl_t.
 *
 * @pre Populate necessary buffers for the input using QCarCamSetBuffers().
 *
 * @param[in]  hndl    Handle of the input.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If start was requested in the wrong state or QCarCam interface is not
 *                          initialized yet.; \n
 * #QCARCAM_RET_NOMEM       If no buffers set for the specified input.
 *
 * @see QCarCamSetBuffers()
 */
QCarCamRet_e QCarCamStart(const QCarCamHndl_t hndl);

/**
 * @cond QCarCamStop @endcond
 *
 * @brief Stops the input streaming on a streaming input represented by the #QCarCamHndl_t
 *
 * @pre Successful QCarCamStart() call on the same handle.
 *
 * @param[in]  hndl    Handle of the input.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If input is not started or paused or QCarCam interface is not
 *                          initialized yet.
 *
 * @see QCarCamStart()
 */
QCarCamRet_e QCarCamStop(const QCarCamHndl_t hndl);

/**
 * @cond QCarCamPause @endcond
 *
 * @brief Pauses the input that was started without relinquishing resources.
 *
 * @pre Successful QCarCamStart() call on the same handle.
 *
 * @param[in]  hndl    Handle of the input.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If input is not started or QCarCam interface is not initialized yet.
 *
 * @see QCarCamStart()
 * @see QCarCamResume()
 */
QCarCamRet_e QCarCamPause(const QCarCamHndl_t hndl);

/**
 * @cond QCarCamResume @endcond
 *
 * @brief Resumes the input that was paused.
 *
 * @pre Successful QCarCamPause() call on the same handle.
 *
 * @param[in]  hndl    Handle of the input.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If input is not paused or QCarCam interface is not initialized yet.
 *
 * @see QCarCamPause()
 */
QCarCamRet_e QCarCamResume(const QCarCamHndl_t hndl);

/**
 * @cond QCarCamGetFrame @endcond
 *
 * @brief Gets the available frame for input represented by the #QCarCamHndl_t.
 *
 * @pre Successful QCarCamStart() call on the same handle.
 *
 * @param[in]  hndl        Handle of the input.
 * @param[in/out] pFrameInfo  Pointer to the frame information that will be filled. \n
 *                           The bufferlist id will indicate which bufferlist will be dequeued from.
 * @param[in]  timeout     Maximum wait time in nanoseconds for the frame to be available 
 *                         before timeout.
 * @param[in]  flags       Flags for getting the frame (reserved for future use).
 *
 * @detdesc
 * Input frames can only be fetched after a successful QCarCamStart() call - required after either 
 * QCarCamOpen() or QCarCamStop() of the same input. Similarly, call to QCarCamResume() is required
 * to fetch frames if input was paused via QCarCamPause().
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If input is not started or QCarCam interface is not initialized yet; \n
 * #QCARCAM_RET_TIMEOUT     If next frame not available within the specified timeout requested.
 *
 * @see QCarCamStart()
 * @see QCarCamReleaseFrame()
 */
QCarCamRet_e QCarCamGetFrame(
    const QCarCamHndl_t hndl,
    QCarCamFrameInfo_t *pFrameInfo,
    const uint64_t timeout,
    const uint32_t flags);

/**
 * @cond QCarCamReleaseFrame @endcond
 *
 * @brief Signals that the released bufferIndex can be reused for any future frames for input
 *        represented by the #QCarCamHndl_t.
 *
 * @pre Successful QCarCamGetFrame() call on the same handle.
 *
 * @param[in] hndl         Handle of the input.
 * @param[in] id           Buffer list ID associated with the input handle obtained from
 *                         QCarCamGetFrame() via the #QCarCamFrameInfo_t structure.
 * @param[in] bufferIndex  Index into the #QCarCamBufferList_t buffers list to re-enqueue buffer
 *                         for the specified buffer list ID.
 *
 * @detdesc
 * Upon a successful return of QCarCamGetFrame(), and the buffer identified by <id, bufferIndex>
 * has been processed, the buffer should be released back to the QCarcam library using
 * QCarCamReleaseFrame() as soon as possible so that new input frames can be fetched.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If requested buffer is not expected to be released yet.
 *
 * @see QCarCamGetFrame()
 */
QCarCamRet_e QCarCamReleaseFrame(
    const QCarCamHndl_t hndl,
    const uint32_t id,
    const uint32_t bufferIndex);
    

/**
 * @cond QCarCamRegisterEventCallback @endcond
 *
 * @brief Register a QCarCam event callback for system wide or per input events.
 *
 * @pre Successful QCarCamOpen() call returning the same handle.
 *
 * @param[in]  hndl            Handle of the input. Set to NULL if registering a system wide event
 *                             callback.
 * @param[in]  callbackFunc    Callback function provided by the client.
 * @param[in]  pPrivateData    Pointer to the private data provided by the client. Will be returned
 *                             with each system or input event.
 *
 * @detdesc
 * The QCarCam API supports one system wide event callback to be registered after initializing the
 * QCarCam API using QCarCamInitialize(). The QCarCam API uninitializing using
 * QCarCamUninitialize() will automatically unregister all event callbacks registered by the
 * client.
 * @par
 * The QCarCam API supports one input event callback to be registered per input after an input has
 * been opened successfully using QCarCamOpen().
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet;
 *
 * @see QCarCamUnregisterEventCallback()
 */
QCarCamRet_e QCarCamRegisterEventCallback(
    const QCarCamHndl_t hndl,
    const QCarCamEventCallback_t callbackFunc,
    void  *pPrivateData);

/**
 * @cond QCarCamUnregisterEventCallback @endcond
 *
 * @brief Unregister a QCarCam event callback for system wide or per input events.
 *
 * @pre Successful QCarCamRegisterEventCallback() call on the same handle.
 *
 * @param[in]  hndl    Handle of the input. Set to NULL if unregistering a system wide event
 *                     callback.
 *
 * @detdesc
 * A system wide event callback can be unregistered only after all inputs have been closed using
 * QCarCamClose().
 * @par
 * An input event callback can be unregistered only after that input has been closed using
 * QCarCamClose().
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet.
 *
 * @see QCarCamRegisterEventCallback()
 */
QCarCamRet_e QCarCamUnregisterEventCallback(const QCarCamHndl_t hndl);

/*=================================================================================================
** NEW APIs
** The Following APIs are defined but are UNSUPPORTED in this draft.
** Their definitions may slightly change in the final revision of this API
=================================================================================================*/
/**
 * @cond QCarCamReserve @endcond
 *
 * @brief Reserves resources required for the usecase for the input represented by the #QCarCamHndl_t.
 *
 * @pre Populate necessary buffers for the input using QCarCamSetBuffers() and populated all other parameters
 *      required to define the usecase.
 *
 * @param[in] hndl         Handle of the input.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If in invalid state when calling this API
 *
 * @see QCarCamRelease()
 */
QCarCamRet_e QCarCamReserve(const QCarCamHndl_t hndl);

/**
 * @cond QCarCamRelease @endcond
 *
 * @brief Releases resources previously reserved for the input represented by the #QCarCamHndl_t.
 *
 * @pre Successful QCarCamReserve() on the same handle.
 *
 * @param[in] hndl         Handle of the input.
 *
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If requested buffer is not expected to be released yet.
 *
 * @see QCarCamReserve()
 */
QCarCamRet_e QCarCamRelease(const QCarCamHndl_t hndl);

/**
 * @cond QCarCamQueryInputModes @endcond
 *
 * @brief Queries available modes for the input represented by inputId.
 *
 * @pre Successful QCarCamInitialize()
 *
 * @param[in] inputId         Input identifier for which to enumerate the available modes.
                              Available inputs can be queried using QCarCamQueryInputs().
 * @param[out] pInputModes    Pointer to structure that will hold the enumerated modes. Number of modes
                              available can be queried using QCarCamQueryInputs().
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 *
 * @see QCarCamInitialize() and QCarCamQueryInputs()
 */
QCarCamRet_e QCarCamQueryInputModes(
    const uint32_t inputId,
    QCarCamInputModes_t* pInputModes);

/**
 * @cond QCarCamSubmitRequest @endcond
 *
 * @brief Submits a frame request for the input represented by the #QCarCamHndl_t. The frame request
 *         includes metadata settings to be applied for the frame as well as list of input/output buffers
 *
 * @pre Successful QCarCamStart()
 *
 * @param[in] hndl         Handle of the input.
 * @param[in] pRequest     Pointer to request structure
 *
 * @detdesc
 * The request is identified by a unique requestId. The frame will be delivered asynchronously through the QCARCAM_EVENT_FRAME_READY
 * and the requestId will be set in the QCarCamFrameInfo_t
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If handle is not in correct state
 *
 * @see 
 */
QCarCamRet_e QCarCamSubmitRequest(const QCarCamHndl_t hndl, const QCarCamRequest_t* pRequest);

/**
 * @brief Initiates recovery of streams/resources in error/degraded state.
 *
 * @note Yet to be fully defined
 */
QCarCamRet_e QCarCamInitiateRecovery();


/*=================================================================================================
** DIAGNOSTIC API
** For now, this is kept for backward compatibility. However, it will be replaced with a newer API
=================================================================================================*/
#include "qcarcam_diag_types.h"

/**************************************************************************************************
QCarCamQueryDiagnostics

@brief Queries System Status and Diagnostics

Able to query system status anytime after initializing QCarCam API using QCarCamInitialize().

@param      pDiag  Pointer to user allocated memory buffer to store system status to be
                        retrieved.
                        Note: Size of memory buffer will need to be size of
                        structure: QCarCamSystemStatusInfo_t

@return QCARCAM_RET_OK only if successful; check QCarCamRet_e otherwise.
**************************************************************************************************/
QCarCamRet_e QCarCamQueryDiagnostics(QCarCamDiagInfo* pDiag);


/** @} */ /* end_addtogroup qcarcam_functions */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* QCARCAM_H */

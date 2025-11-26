#ifndef QCX_UTILS_H
#define QCX_UTILS_H

/**================================================================================================

 @file
 qcxutils.h

 @brief
 This file defines misc. utility functions for overall OSAL

 Copyright (c) 2022-2024 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.

 ================================================================================================**/

/**================================================================================================

 INCLUDE FILES FOR MODULE

 ================================================================================================**/
#include "qcxstatus.h"

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/**================================================================================================
 DEFINITIONS AND DECLARATIONS
 ================================================================================================**/

/**================================================================================================
 ** Constant and Macros
 ================================================================================================**/

/** Maximum value of 32 bit unsigned int */
#define MAX_UINT32_VALUE        0xFFFFFFFFU

/** @TODO: REMOVE THESE FROM HERE AND MOVE THEM TO WHERE USED **/
/** Poll period in milliseconds checks for npa,pmem_server,smmu device path to become valid */
#define POLL_PERIOD             5

/** Wait period in milliseconds for npa,pmem_server,smmu device path to become valid */
#define MAX_POLL                200

#ifdef FEAT_ENABLE_GET_DAUGHTERCARD_INFO
/** Wait period in milliseconds for daughter card utility */
#define MAX_POLL_CAMERA_CARD    50
#endif /* FEAT_ENABLE_GET_DAUGHTERCARD_INFO */

/**================================================================================================
 ** Variables
 ================================================================================================**/

/**================================================================================================
 ** Typedefs
 ================================================================================================**/

/**================================================================================================
 FUNCTION DEFINITIONS
 ================================================================================================**/

/**************************************************************************************************
 @brief
 Maps DomainOS Error to CamStatus_e

 Generally, when there is no corresponding CamStatus_e to match, AOErrorCode_e in the domain
 of OS, data structures, and resource limits are mapped to CAMERA_EFAILED.
 Furthermore, only unhandled fromDomainOSError will map to CAMERA_EFAILED.

 @param fromDomainOSError
 The error code to map from OS Domain error, in QNX it is AOErrorCode_e

 @return
 The closest matching CamStatus_e of the DomainOSError

 @note
 The order the of switch cases match the order of enum inside AOErrorCode_e's definition for
 cross-checking/reference.  Do not reorder them.
 **************************************************************************************************/
CamStatus_e OSAL_MapError(
    const int fromDomainOSError);


/**************************************************************************************************
 @brief
 Maps errno to CamStatus_e

  @param err
 The error code to map from

 @return
 The closest matching CamStatus_e of the errno error code

 **************************************************************************************************/
CamStatus_e OSAL_MapErrno(
    const int err);


/**************************************************************************************************
 * @brief
 * Converts a string into an 32-bit unsigned integer
 *
 * @param pValueStr
 * Pointer to the string from which to extract the value
 *
 * @param pValue
 * Pointer to the buffer that will contain the extracted value
 *
 * @return
 * Error code as defined in CamStatus_e
 **************************************************************************************************/
CamStatus_e OSAL_GetUINT32Value (
    const char *const pValueStr,
    uint32_t *pValue);


/**************************************************************************************************
 * @brief
 * Converts a string into a float value
 *
 * @param pValueStr
 * Pointer to the string from which to extract the value
 *
 * @param pValue
 * Pointer to the buffer that will contain the extracted value
 *
 * @return
 * Error code as defined in CamStatus_e
 **************************************************************************************************/
CamStatus_e OSAL_GetFloatValue (
    const char *const pValueStr,
    float *pValue);


#ifdef LINUX_LRH
/**********************************************************************************************//**
 * @brief
 * Duplicates file descriptor (fd)
 *
 * @param targetPid
 * Process ID of process that owns the fd (i.e. target_fd)
 *
 * @param targetFd
 * fd that needs to be copied
 *
 * @param copiedFd
 * Pointer to newly created fd that's returned to the caller
 *
 * @return
 * Error code as defined in CamStatus_e
 **************************************************************************************************/
CamStatus_e OSAL_CopyFd (
    int targetPid,
    int targetFd, int *copiedFd);
#endif

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif // QCX_UTILS_H


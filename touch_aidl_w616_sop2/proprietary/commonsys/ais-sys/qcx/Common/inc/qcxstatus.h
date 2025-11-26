#ifndef QCX_STATUS_H
#define QCX_STATUS_H

/* ===========================================================================
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @file qcxstatus.h
 * @brief Camera Driver Return Codes
 *
=========================================================================== */


#include "qcxtypes.h"

#ifdef __cplusplus
extern "C" {
#endif


/**================================================================================================
                      DEFINITIONS AND DECLARATIONS
================================================================================================**/

/**================================================================================================
** Constant and Macros
================================================================================================**/


/**================================================================================================
** Typedefs
================================================================================================**/

/**************************************************************************************************
@brief
    List of QCX return codes
**************************************************************************************************/
typedef enum
{
    CAMERA_SUCCESS  = 0,        /// Success
    CAMERA_EFAILED,             /// Error that does not fit in any of the below categories ///
    CAMERA_EBADPARAM,           /// One or more bad parameters were passed to the function ///
    CAMERA_EINVALIDOP,          /// Invalid operation requested ///
    CAMERA_EINVALIDSTATE,       /// Operation requested in an invalid state ///
    CAMERA_ENOTPERMITTED,       /// The operation is not permitted
    CAMERA_EOUTOFBOUND,         /// One or more parameters are out of bound
    CAMERA_ETIMEOUT,            /// A timeout has occurred
    CAMERA_ENOMEMORY,           /// There is not enough memory to perform this operation
    CAMERA_ENOTSUPPORTED,       /// The requested operation is not supported
    CAMERA_EBUSY,               /// Resources are not available to perform this operation
    CAMERA_ENOTFOUND,           /// Resource was not found
    CAMERA_EUNABLETOLOAD,       /// Error loading object/library

    CAMERA_ENODATA,             /// There was no data returned
    CAMERA_NEWCONNECTION,       /// A new client has connected to the IPC server
    CAMERA_ERETRY,              /// Caller should retry this operation
    CAMERA_EINVALIDSIZE,        /// Passed object has invalid size
    CAMERA_EFILENOTFOUND,       /// File was not found
    CAMERA_EBADHANDLE,          /// An invalid handle was passed
    CAMERA_EIPCRXFAILURE,       /// IPC Receive operation failed
    CAMERA_EIPCTXFAILURE,       /// IPC Transmit operation failed

    CAMERA_ECAMCLOCKFAILURE,    /// A failure in camera clock has occurred
    CAMERA_EHWINITFAILURE,      /// Driver failed to initialize HW
    CAMERA_EMEMHWFAILURE,       /// An error occurred in the memory HW
    CAMERA_EI2CINTEGRITY,       /// I2C Integrity check failed
    CAMERA_ENOCONNECTION,       /// There is no active IPC connection on the passed handle
    CAMERA_EINVALIDCONFIG,      /// Invalid configurations provided
    CAMERA_ECCINAK,             /// A CCI NACK was received
    CAMERA_ECCIHWFAILURE,       /// A failure has occurred in CCI HW
    CAMERA_ESOCUNRESPONSIVE,    /// SoC Liveliness error status flag
    CAMERA_BISTFAILURE,         /// BIST Error status flag

    CAMERA_ENOMORE              /// in some functions where we repeatedly call a functions to get multiple
                                /// results, they will return this error code to indicate there is no more
                                /// available items.


} CamStatus_e;


#ifdef __cplusplus
}
#endif


#endif /* QCX_STATUS_H */

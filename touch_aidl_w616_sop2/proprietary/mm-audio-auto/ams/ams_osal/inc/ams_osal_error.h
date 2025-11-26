#ifndef AMS_OSAL_ERROR_H
#define AMS_OSAL_ERROR_H

/**
*
* \file ams_osal_error.h
*
* \brief
*     This file contains error codes used by AudioReach.
* \copyright
* Copyright (c) 2020, 2022 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/


/* -----------------------------------------------------------------------
** ERROR CODES
** ----------------------------------------------------------------------- */
/** Success. The operation completed with no errors. */
#define AMS_EOK                                     (0)
/** General failure. */
#define AMS_EFAILED                                 (1)
/** Bad operation parameter. */
#define AMS_EBADPARAM                               (2)
/** Unsupported routine or operation. */
#define AMS_EUNSUPPORTED                            (3)
/** Unsupported version. */
#define AMS_EVERSION                                (4)
/** Unexpected problem encountered. */
#define AMS_EUNEXPECTED                             (5)
/** Unhandled problem occurred. */
#define AMS_EPANIC                                  (6)
/** Unable to allocate resource. */
#define AMS_ENORESOURCE                             (7)
/** Invalid handle. */
#define AMS_EHANDLE                                 (8)
/** Operation is already processed. */
#define AMS_EALREADY                                (9)
/** Operation is not ready to be processed. */
#define AMS_ENOTREADY                               (10)
/** Operation is pending completion. */
#define AMS_EPENDING                                (11)
/** Operation cannot be accepted or processed. */
#define AMS_EBUSY                                   (12)
/** Operation was aborted due to an error. */
#define AMS_EABORTED                                (13)
/** Operation requests an intervention to complete. */
#define AMS_ECONTINUE                               (14)
/** Operation requests an immediate intervention to complete. */
#define AMS_EIMMEDIATE                              (15)
/** Operation was not implemented. */
#define AMS_ENOTIMPL                                (16)
/** Operation needs more data or resources. */
#define AMS_ENEEDMORE                               (17)
/** Operation does not have memory. */
#define AMS_ENOMEMORY                               (18)
/** Item does not exist. */
#define AMS_ENOTEXIST                               (19)
/** Operation is finished. */
#define AMS_ETERMINATED                             (20)
/** Operation timeout. */
#define AMS_ETIMEOUT                                (21)
/** Data read/write failed. */
#define AMS_EIODATA                                 (22)
/** Sub system reset occured. */
#define AMS_ESUBSYSRESET                            (23)
/** @} */  /* end_addtogroup AMS_error_codes */

/** Checks if a result is a success */
#define AMS_SUCCEEDED(x)   (AMS_EOK == (x) )

/** Checks if a result is failure. */
#define AMS_FAILED(x)   (AMS_EOK != (x) )

#endif /* #ifndef AMS_OSAL_ERROR_H */

#ifndef DIAG_LSM_H
#define DIAG_LSM_H
/**
*=============================================================================
*  \file diag_lsm.h
*  \brief
*                   Diag Mapping Layer DLL declarations
*
*           This file is a stub version of the Diag Mapping Layer Interface
*           header. This file is used for compilation with the
*           Off-target Test Platform(OTTP).
*
*  \copyright
*      Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
*      All Rights Reserved.
*      Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*=============================================================================
*/

#include "ar_osal_types.h"

/**
* \brief
* Initializes the Diag Legacy Mapping Layer. This should be called only once
* per process. This is a stub used for compilation on OTTP
*
* \depends
* Successful initialization requires Diag CS component files to be present
* and accessible in the file system.
*
* \return FALSE = failure, else TRUE
*/
bool_t Diag_LSM_Init(uint8_t* pIEnv) { __UNREFERENCED_PARAM(pIEnv); return TRUE; }

bool_t Diag_LSM_DeInit(void) { return TRUE; }

#endif /* DIAG_LSM_H */


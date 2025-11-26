/**================================================================================================

 @file
 qcxtypes.h

 @brief
 QCX Types

 Copyright (c) 2022, 2024 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.

 ================================================================================================**/
#include <stddef.h>
#include <stdint.h>

#if !defined(__ANDROID__)
typedef uint8_t boolean;
#else
#define boolean uint8_t
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

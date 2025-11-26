////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __IS_COMMOM_INTF_H__
#define __IS_COMMOM_INTF_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//TODO (etsairi): remove (use bool instead)
typedef bool boolean;

typedef struct
{
    double w;
    double x;
    double y;
    double z;
} quaternion_type;

#ifdef __cplusplus
}
#endif

#endif /* __IS_COMMOM_INTF_H__ */

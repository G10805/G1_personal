#ifndef __CAMERAPLATFORMLINUX_H_
#define __CAMERAPLATFORMLINUX_H_

/**
 * @file CameraPlatformLinux.h
 *
 * @brief Camera platform APIs for Linux platform
 *
 * Copyright (c) 2017-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

/* ===========================================================================
                        INCLUDE FILES
=========================================================================== */
#include "CameraPlatform.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* ===========================================================================
                        DATA DECLARATIONS
=========================================================================== */

/* ---------------------------------------------------------------------------
** Constant / Define Declarations
** ------------------------------------------------------------------------ */
/** Used as handle_type for cam_control. Must match KMD value */
#define AIS_IFE_CMD_TYPE 0xA151FE00

/* ---------------------------------------------------------------------------
** Type Declarations
** ------------------------------------------------------------------------ */
typedef enum {
    AIS_SUBDEV_AISMGR,
    AIS_SUBDEV_IFE,
    AIS_SUBDEV_CSIPHY,
    AIS_SUBDEV_SENSOR,
    AIS_SUBDEV_REQMGR_NODE,
    AIS_SUBDEV_REQMGR,
    AIS_SUBDEV_SYNCMGR_NODE,
    AIS_SUBDEV_SYNCMGR,
    AIS_SUBDEV_CCI,
    AIS_SUBDEV_CPAS,
    AIS_SUBDEV_MAX,
}AisSubdevIdType;

typedef enum {
    AIS_HANDLE_TYPE_SESSION,
    AIS_HANDLE_TYPE_DEVICE,
    AIS_HANDLE_TYPE_MAX,
}AisKmdHandletype;

/* ===========================================================================
                        EXTERNAL API DECLARATIONS
=========================================================================== */
const char* CameraPlatformGetDevPath(AisSubdevIdType id, uint32 subdevIdx);
int CameraPlatformGetFd(AisSubdevIdType id, uint32 subdevIdx);
void CameraPlatformClearFd(AisSubdevIdType id, uint32 subdevIdx);
uint32 CameraPlatformGetKMDHandle(AisKmdHandletype type, AisSubdevIdType id, uint32 subdevIdx);
void CameraPlatformSetKMDHandle(AisKmdHandletype type, AisSubdevIdType id, uint32 subdevIdx, uint32 handle);

int CameraPlatform_GetCsiCore(CameraSensorIndex idx);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // __CAMERAPLATFORMLINUX_H_

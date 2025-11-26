/*
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "xf86drm.h"
#include "xf86drmMode.h"
#include <poll.h>
#include <utils/Log.h>
#include <errno.h>
#include <string.h>

#define DRM_EVENT_SDE_UNDERRUN		0x80000008
#define DRM_EVENT_SDE_SMMUFAULT 	0x80000009
#define DRM_EVENT_SDE_VSYNC_MISS	0x80000010
#define DRM_EVENT_RECOVERY_SUCCESS	0x80000011
#define DRM_EVENT_RECOVERY_FAILURE	0x80000012
#define DRM_EVENT_BRIDGE_ERROR		0x80000013

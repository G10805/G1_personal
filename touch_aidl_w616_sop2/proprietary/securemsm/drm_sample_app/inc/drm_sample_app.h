/**
 * Copyright (c) 2023 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef DRM_SAMPLE_APP_H_
#define DRM_SAMPLE_APP_H_

#include <stdint.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "DRMSAMPLEAPP"

/* Error Codes */
#define DRM_SUCCESS 0
#define DRM_FAILURE 1

#define C_LENGTHOF(array) \
    ((void *)&(array) == (void *)(array) ? sizeof(array) / sizeof *(array) : 0)

#define BAIL_OUT()                                         \
    {                                                      \
        ALOGE("Error at %s : %d", __FUNCTION__, __LINE__); \
        goto exit;                                         \
    }

#define CHECK_COND(cond) \
    if (!(cond)) {       \
        BAIL_OUT();      \
    }

#define CHECK_COND_ERR(cond, err_code) \
    if (!(cond)) {                     \
        ret = err_code;                \
        BAIL_OUT();                    \
    }

#define GUARD(func) CHECK_COND_ERR(DRM_SUCCESS == (func), DRM_FAILURE);

#define ERROR(err_code) \
    ret = err_code;     \
    BAIL_OUT();

/* Enum to handle the command id */
typedef enum {
    DRM_SAMPLE_ALL = 1,
    DRM_SAMPLE_SMMU_VA = 2,
    DRM_SAMPLE_TZ_VA = 3,
    DRM_SAMPLE_SFS = 4,
    DRM_SAMPLE_RPMB = 5,
    DRM_SAMPLE_HDCP = 6,
    DRM_SAMPLE_ACCESS_WITH_CPZ_LICENSE = 7,
    DRM_SAMPLE_ACCESS_WITHOUT_CPZ_LICENSE = 8,
} drm_sample_cmd_t;

#endif /* DRM_SAMPLE_APP_H_ */

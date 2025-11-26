/*========================================================================
  Copyright (c) 2024 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  =========================================================================*/

#include <map>

#define SIZE_2MB 0x200000

#define T_ERROR(error_code)                                                      \
    ret = (error_code);                                                          \
    ALOGE("%s::%d err=0x%x errno:%d(%s)", __func__, __LINE__, error_code, errno, \
          strerror(errno));                                                      \
    goto exit;

#define T_CHECK_ERR(cond, error_code) \
    if (!(cond)) {                    \
        T_ERROR(error_code)           \
    }

#define T_CHECK(cond)                        \
    if (!(cond)) {                           \
        ALOGE("%s::%d", __func__, __LINE__); \
        goto exit;                           \
    }

#define LOGD_PRINT(...)      \
    do {                     \
        ALOGD(__VA_ARGS__);  \
        printf(__VA_ARGS__); \
        printf("\n");        \
    } while (0)

#define LOGE_PRINT(...)      \
    do {                     \
        ALOGE(__VA_ARGS__);  \
        printf(__VA_ARGS__); \
        printf("\n");        \
    } while (0)

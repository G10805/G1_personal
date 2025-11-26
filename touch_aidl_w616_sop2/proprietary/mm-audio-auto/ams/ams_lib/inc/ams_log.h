#ifndef AMS_LOG_H
#define AMS_LOG_H
/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#include <stdio.h>
#include <string.h>

extern unsigned int g_ams_test_log_enable;
extern unsigned int g_ams_test_log_dbg_lvl;
extern unsigned int g_ams_test_log_show_tid;

#define AMS_LIB_LOG_SKIP_THREAD_ID (1)

#define AMS_LIB_LOGE(format, ...)                                                                                   \
    {                                                                                                               \
        if (g_ams_test_log_enable)                                                                                  \
        {                                                                                                           \
            if (g_ams_test_log_show_tid)                                                                            \
            {                                                                                                       \
                uint32_t tid = 0;                                                                                   \
                printf("E:tid=%u:%s:%s,line %d : " format "\n", tid, "ams_lib", __func__, __LINE__, ##__VA_ARGS__); \
            }                                                                                                       \
            else                                                                                                    \
            {                                                                                                       \
                printf("E:%s:%s,line %d : " format "\n", "ams_lib", __func__, __LINE__, ##__VA_ARGS__);             \
            }                                                                                                       \
        }                                                                                                           \
    }
#define AMS_LIB_LOGI(format, ...)                                                                                  \
    {                                                                                                              \
        if (g_ams_test_log_enable && g_ams_test_log_dbg_lvl >= 1)                                                  \
        {                                                                                                          \
            if (g_ams_test_log_show_tid)                                                                           \
            {                                                                                                      \
                uint32_t tid = 0;                                                                                  \
                printf("I:tid=%u:%s:%s,line %d: " format "\n", tid, "ams_lib", __func__, __LINE__, ##__VA_ARGS__); \
            }                                                                                                      \
            else                                                                                                   \
            {                                                                                                      \
                printf("I:%s:%s,line %d: " format "\n", "ams_lib", __func__, __LINE__, ##__VA_ARGS__);             \
            }                                                                                                      \
        }                                                                                                          \
    }

#define AMS_LIB_LOGD(format, ...)                                                                                 \
    {                                                                                                             \
        if (g_ams_test_log_enable && g_ams_test_log_dbg_lvl > 1)                                                  \
        {                                                                                                         \
            if (g_ams_test_log_show_tid)                                                                          \
            {                                                                                                     \
                uint32_t tid = 0;                                                                                 \
                printf("D:tid=%u:%s:%s,line %d " format "\n", tid, "ams_lib", __func__, __LINE__, ##__VA_ARGS__); \
            }                                                                                                     \
            else                                                                                                  \
            {                                                                                                     \
                printf("D:%s:%s,line %d " format "\n", "ams_lib", __func__, __LINE__, ##__VA_ARGS__);             \
            }                                                                                                     \
        }                                                                                                         \
    }

#define AMS_LIB_TEST_ASSERT(cond)                  \
    {                                              \
        if (!(cond))                               \
        {                                          \
            printf("%s:TEST FAILED \n", __func__); \
        }                                          \
        else                                       \
        {                                          \
            printf("%s:TEST OK \n", __func__);     \
        }                                          \
    }

#endif

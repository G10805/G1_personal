////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef IS_DBG_UTIL_H
#define IS_DBG_UTIL_H

#include <stdio.h>
#include <inttypes.h>

#include "is_interface.h"

#define IS_LOG_LEVEL_INFO      (5)
#define IS_LOG_LEVEL_LOW       (4)
#define IS_LOG_LEVEL_HIGH      (3)
#define IS_LOG_LEVEL_ERROR     (2)
#define IS_LOG_LEVEL_NONE      (1)


#ifndef IS_LOG_LEVEL
#define IS_LOG_LEVEL IS_LOG_LEVEL_HIGH
#endif /*IS_LOG_LEVEL */

#define LOG_LEVEL_INFO_LABLE      "[INFO]"
#define LOG_LEVEL_LOW_LABLE       "[LOW] "
#define LOG_LEVEL_HIGH_LABLE      "[HIGH]"
#define LOG_LEVEL_ERROR_LABLE     "[ERR] "

#if defined(ANDROID)

#undef LOG_TAG
#define LOG_TAG "IS_ALGO"

#include <android/log.h>

/** ERROR debug macro */
#if (IS_LOG_LEVEL >= IS_LOG_LEVEL_ERROR)
#define IS_ERR(_msg_idx, fmt, ...)      __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, _msg_idx": %s(%d): " fmt , __FUNCTION__, __LINE__ ,##__VA_ARGS__)
#else
#define IS_ERR(...)                     ((void)(0))
#endif /* (IS_LOG_LEVEL >= IS_LOG_LEVEL_ERROR) */

/** HIGH debug macro */
#if (IS_LOG_LEVEL >= IS_LOG_LEVEL_HIGH)
#define IS_HIGH(_msg_idx, fmt, ...)     __android_log_print(ANDROID_LOG_WARN, LOG_TAG, _msg_idx": %s(%d): " fmt, __FUNCTION__, __LINE__ , ##__VA_ARGS__)
#else
#define IS_HIGH(...)                    ((void)(0))
#endif /* (IS_LOG_LEVEL >= IS_LOG_LEVEL_HIGH) */

/** LOW debug macro */
#if (IS_LOG_LEVEL >= IS_LOG_LEVEL_LOW)
#define IS_LOW(_msg_idx, fmt, ...)      __android_log_print(ANDROID_LOG_INFO, LOG_TAG, _msg_idx": %s(%d): " fmt, __FUNCTION__, __LINE__ , ##__VA_ARGS__)
#else
#define IS_LOW(...)                     ((void)(0))
#endif /* (IS_LOG_LEVEL >= IS_LOG_LEVEL_LOW) */

/** INFO debug macro */
#if (IS_LOG_LEVEL >= IS_LOG_LEVEL_INFO)
#define IS_INFO(_msg_idx, fmt, ...)     __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, _msg_idx": %s(%d): " fmt, __FUNCTION__, __LINE__ , ##__VA_ARGS__)
#else
#define IS_INFO(...)                    ((void)(0))
#endif /* (IS_LOG_LEVEL >= IS_LOG_LEVEL_INFO) */

#else

#if !defined(IS_FPRINTF_MSG)
#if defined(__IS_FPRINTF_MSG_NO_INFO__)
/** printf format label */
#define IS_FPRINTF_MSG(_file, _msg_idx, _label,_fmt,...)    \
do {                                                        \
    fprintf(_file,"%s: ",_msg_idx);                         \
    fprintf(_file,_fmt, ##__VA_ARGS__);                     \
    fprintf(_file,"\n");                                    \
} while(0,0)
#else
/** printf format label */
#define IS_FPRINTF_MSG(_file, _msg_idx, _label,_fmt,...)    \
do {                                                        \
    fprintf(_file,"%s ",_label);                            \
    fprintf(_file,"%s: ",_msg_idx);                         \
    fprintf(_file,"%s(%d): ", __FUNCTION__, __LINE__);      \
    fprintf(_file,_fmt, ##__VA_ARGS__);                     \
    fprintf(_file,"\n");                                    \
} while(0,0)
#endif
#endif /* IS_FPRINTF_MSG */

/** ERROR debug macro */
#if (IS_LOG_LEVEL >= IS_LOG_LEVEL_ERROR)
#define IS_ERR(_msg_idx, fmt, ...)      IS_FPRINTF_MSG(stderr, _msg_idx, LOG_LEVEL_ERROR_LABLE, fmt, ##__VA_ARGS__)
#else
#define IS_ERR(...)                     ((void)(0))
#endif /* (IS_LOG_LEVEL >= IS_LOG_LEVEL_ERROR) */

/** HIGH debug macro */
#if (IS_LOG_LEVEL >= IS_LOG_LEVEL_HIGH)
#define IS_HIGH(_msg_idx, fmt, ...)     IS_FPRINTF_MSG(stderr, _msg_idx, LOG_LEVEL_HIGH_LABLE, fmt, ##__VA_ARGS__)
#else
#define IS_HIGH(...)                    ((void)(0))
#endif /* (IS_LOG_LEVEL >= IS_LOG_LEVEL_HIGH) */

/** LOW debug macro */
#if (IS_LOG_LEVEL >= IS_LOG_LEVEL_LOW)
#define IS_LOW(_msg_idx, fmt, ...)      IS_FPRINTF_MSG(stderr, _msg_idx, LOG_LEVEL_LOW_LABLE, fmt, ##__VA_ARGS__)
#else
#define IS_LOW(...)                     ((void)(0))
#endif /* (IS_LOG_LEVEL >= IS_LOG_LEVEL_LOW) */

/** INFO debug macro */
#if (IS_LOG_LEVEL >= IS_LOG_LEVEL_INFO)
#define IS_INFO(_msg_idx, fmt, ...)     IS_FPRINTF_MSG(stderr, _msg_idx, LOG_LEVEL_INFO_LABLE, fmt, ##__VA_ARGS__)
#else
#define IS_INFO(...)                    ((void)(0))
#endif /* (IS_LOG_LEVEL >= IS_LOG_LEVEL_INFO) */

#endif /* ANDROID */

#ifndef  IS_COMPILE_TIME_ASSERT
#define _IS_PASTE_STR(a,b) a##b
#define _IS_COMPILE_TIME_ASSERT(expr, id)   typedef char _IS_PASTE_STR(CASSERT_FAILED_, id)[(expr)?1:-1]
#define IS_COMPILE_TIME_ASSERT(expr) _IS_COMPILE_TIME_ASSERT(expr, __LINE__)
#endif /* !IS_COMPILE_TIME_ASSERT */

#ifndef IS_ASSERT
#include  <assert.h>
#if defined(ANDROID)
#ifndef NDEBUG
#define IS_ASSERT(c)    { if (!(c)) {IS_ERR(IS_ERR_ASSERTION, "Assertion failed: %s, file %s, line %d\n", #c, __FILE__, __LINE__ ); assert(false);} }
#else
#define IS_ASSERT(c)    { assert(c); }
#endif /* NDEBUG */
#else
#if defined(__IS_FPRINTF_MSG_NO_INFO__)
#define IS_ASSERT(c)    { if (!(c)) {IS_ERR(IS_ERR_ASSERTION, "Assertion failed\n"); __debugbreak();  } }
#else
#define IS_ASSERT(c)    { if (!(c)) {IS_ERR(IS_ERR_ASSERTION, "Assertion failed: %s, file %s, line %d\n", #c, __FILE__, __LINE__ ); __debugbreak();  } }
#endif /* __IS_FPRINTF_MSG_NO_INFO__ */
#endif /* ANDROID */
#endif /* IS_ASSERT */

/* If not defined explicit default directory to save the output files, use default values according to the OS */
#if !defined(IS_DBG_UTILS_DEFAULT_DIR)
#if defined(_WIN32) || defined(_WIN64)
#define IS_DBG_UTILS_DEFAULT_DIR        "./"
#elif defined (ANDROID)
#define IS_DBG_UTILS_DEFAULT_DIR        "/data/vendor/camera/"
#endif
#endif /* !IS_DBG_SAVE_LOG_DEFAULT_DIR */

#if !defined(IS_DBG_UTILS_DEFAULT_DIR)
#error "IS_DBG_UTILS_DEFAULT_DIR is not defined"
#endif

#ifndef IS_FOPEN
#if defined(ANDROID)
#ifndef ANDROID_IS_FOPEN_S
#define ANDROID_IS_FOPEN_S
typedef int errno_t;
static errno_t fopen_s(FILE** file, char const* name, char const* mode)
{
    *file = fopen(name, mode);
    return (NULL == *file) ? -1 : 0;
}
#endif /* ANDROID_IS_FOPEN_S */
#endif /* ANDORID */

#define IS_FOPEN(_f, _fname, _attr)     fopen_s(_f, _fname, _attr)
#endif /* IS_ASSERT */

#ifndef IS_FCLOSE
#define IS_FCLOSE(x)       if (NULL != (x)) {fclose(x); x = NULL;}
#endif  /* IS_FCLOSE */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//          Debug API
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void eis_debug_print_process_input(
    const char* file_name,
    const void* is_input);

void eis_debug_print_init_info(
    const char* file_name,
    const void* init_common,
    const void* init_sensors,
    uint32_t num_sensors);

#ifdef __cplusplus
}
#endif

#endif /* !IS_DBG_UTIL_H */


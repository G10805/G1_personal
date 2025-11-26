/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _GEN_UTIL_PROPERTIES_H_
#define _GEN_UTIL_PROPERTIES_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PRINT(a) {printf a ;printf("\n");fflush(stdout);}

#ifdef ENABLE_PROP_DEBUG
#define PROP_DEBUG(a)  PRINT(a)
#else
#define PROP_DEBUG(a)
#endif
#define PROP_PRINT(a)  PRINT(a)

#define MAX_PROPERTY_KEY_SIZE     (128)
#define MAX_PROPERTY_VALUE_SIZE   (128)
#define PROPERTY_VALUE_MAX        (MAX_PROPERTY_VALUE_SIZE)
#define PROPERTY_FILE_PATH        "/etc/prop/"
#define PROPERTY_FILE_NAME        ".property.conf"

bool property_get(const char * key, char *value, const char * default_value);
bool property_set(const char * key, const char * value);
int32_t property_get_int32(const char * key, int32_t default_value);
bool property_get_bool(const char * key, bool default_value);
void property_init(void);
void property_exit(void);

#ifdef __cplusplus
}
#endif

#endif  /* _GEN_UTIL_PROPERTIES_H_ */

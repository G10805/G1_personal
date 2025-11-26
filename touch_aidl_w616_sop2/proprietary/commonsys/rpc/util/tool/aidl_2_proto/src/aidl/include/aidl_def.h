/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <cstdint>

/*******************************************************************************
 * AIDL keyword
 *******************************************************************************/
#define AIDL_AT                 "@"

#define AIDL_AT_NULLABLE        (AIDL_AT "nullable")

#define AIDL_ENUM               "enum"

#define AIDL_IMPORT             "import"

#define AIDL_IN                 "in"

#define AIDL_INTERFACE          "interface"

#define AIDL_ONEWAY             "oneway"

#define AIDL_OUT                "out"

#define AIDL_PACKAGE            "package"

#define AIDL_PARCELABLE         "parcelable"

#define AIDL_UNION              "union"

/*******************************************************************************
 * AIDL data type
 *******************************************************************************/
#define AIDL_TYPE_ARRAY         "[]"

#define AIDL_TYPE_INT_ARRAY     "int[]"

#define AIDL_TYPE_BOOLEAN       "boolean"

#define AIDL_TYPE_BYTE          "byte"

#define AIDL_TYPE_BYTE_ARRAY    "byte[]"

#define AIDL_TYPE_CHAR          "char"

#define AIDL_TYPE_INT           "int"

#define AIDL_TYPE_LONG          "long"

#define AIDL_TYPE_STRING        "String"

#define AIDL_TYPE_VOID          "void"

/*******************************************************************************
 * AIDL misc
 *******************************************************************************/
#define AIDL_FILE_SUFFIX        ".aidl"

#define AIDL_HAL_PACKAGE_PREFIX "android.hardware."

#define AIDL_CALLBACK_SUFFIX_CALLBACK   "Callback"
#define AIDL_CALLBACK_SUFFIX_CALLBACKS  "Callbacks"
#define AIDL_CALLBACK_SUFFIX_LISTENER   "Listener"

/* AIDL file type */
typedef uint32_t    AidlType;

#define AIDL_TYPE_INTERFACE     ((AidlType) 0x0)
#define AIDL_TYPE_CALLBACK      ((AidlType) 0x1)
#define AIDL_TYPE_PARCELABLE    ((AidlType) 0x2)
#define AIDL_TYPE_UNION         ((AidlType) 0x3)
#define AIDL_TYPE_ENUM          ((AidlType) 0x4)

/* AIDL param direction */
typedef uint16_t    AidlParamDir;

#define AIDL_PARAM_IN           ((AidlParamDir) 0x1)
#define AIDL_PARAM_OUT          ((AidlParamDir) 0x2)
#define AIDL_PARAM_IN_OUT       ((AidlParamDir) 0x3)
#define AIDL_PARAM_DIR_MASK     ((AidlParamDir) 0x3)

#define AIDL_NAME               "aidl"

#define AIDL_HAL_HEADER_PREFIX  "aidl/android/hardware"

#define AIDL_HAL_USING_PREFIX   "aidl::android::hardware"
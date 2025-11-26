#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
"""

from enum import Enum, unique

class CommonDef:
    # Generation type
    @unique
    class G_TYPE(Enum):
        H = 0
        CPP = 1

    # Message processing type
    @unique
    class M_TYPE(Enum):
        CLASS = 0
        INTERFACE = 1

    # Handling type
    @unique
    class H_TYPE(Enum):
        SERIALIZE = 0
        DE_SERIALIZE = 1

    # Repeated field type
    @unique
    class R_TYPE(Enum):
        BYTES = 0
        CLASS = 1

    HEADER_PRIMITIVE_TYPES = ["bool", "int32", "int64", "uint32", "float", "double", "char"]
    # AIDL: bool, int, long, byte, double, float
    PRIMITIVE_TYPES = HEADER_PRIMITIVE_TYPES.copy()
    PRIMITIVE_TYPES.append("string")
    PRIMITIVE_TYPES_WITH_BYTES = PRIMITIVE_TYPES.copy()
    PRIMITIVE_TYPES_WITH_BYTES.append("bytes")

    TYPE_MAPPING_CPP = {
        "bool": "bool",
        "bytes": "int8_t",
        "int32": "int32_t",
        "int64": "int64_t",     # long
        "uint32": "int8_t",     # byte
        "float": "float",
        "double": "double",
        "string": "string",
        "char": "char16_t"      # constructed type, not in Proto
    }

    INT32_TYPE = "int32"

    # Type "char" in AIDL is also mapped as "int32" in proto
    # This needs to be handled separately
    TYPE_MAPPING_AIDL = {
        "bool": "boolean",
        "bytes": "byte",
        "int32": "int",
        "int64": "long",
        "uint32": "byte",
        "float": "float",
        "double": "double",
        "string": "String",
        "char": "char"          # constructed type, not in Proto
    }

    PROMPT = \
"""/* This is an auto-generated source file. */

"""

    MACRO = \
"""#pragma once

"""

    TAB = ""

    ENUM = "enum"
    CLASS = "class"
    MESSAGE = "message"
    UNION = "union"
    ONEOF = "oneof"
    ARRAY = "array"
    VECTOR = "vector"

    ID_NAME_MAX_LENGTH = 76
    SANITIZE_MAX_ITERATION = 10

    package_prefix = ""
    aidl_using_prefix = ""
    proto_using_prefix = ""
    aidl_include_prefix = ""
    additional_include_header = ""
    base_intf = ""

    msg_common_interface_name = ""
    common_status_full_name = ""

    special_interface_name = "ISatRadio"
    special_interface = False
    special_method_name = "initialize"
    special_method = False
    special_param_name = "halOption"
    special_param = False

    callback_keywords = set()
    interface_set = set()
    id_definition_offset = 0
    id_definition_callback_offset = 0x8000

    # Default ndk name
    bp_ndk_name = "android.hardware.package.module-V1-ndk"
    provided_package_name = ""
    BLUEPRINT = \
"""cc_library_shared {

    name: "libqti-__PACKAGE_BASE__-message",

    proprietary: true,

    local_include_dirs: [
        "include/",
        "message/include/"
    ],

    srcs: [
        "message/src/*.cpp"
    ],

    shared_libs: [
        __USED_SHARED_LIBS__
    ],

    static_libs: [
        __USED_STATIC_LIBS__
    ],

    export_include_dirs: [
        "include",
        "message/include"
    ],
}"""

    MAKEFILE = \
"""__CONDITIONAL_CONFIG_DEFINE__

LIBRARY=libqti-__PACKAGE_NAME__-message.so.0

CUR_DIR = $(shell pwd)
SRC_DIR = $(CUR_DIR)/message/src
INC_DIR = $(CUR_DIR)/message/include

CXXFLAGS += -c -std=c++17 -fPIC
CPPFLAGS += -Wall -Werror -Wno-unused-function -Wno-ignored-attributes
CPPFLAGS += -I$(INC_DIR)
LDFLAGS += -lstdc++
LDFLAGS += __ADDITIONAL_LDFLAGS__
LDFLAGS += -shared -Wl,-soname,$(LIBRARY)
__CONDITIONAL_CONFIG_LOGIC__
CXXOBJECTS := $(SRCS:.cpp=.o)

$(LIBRARY): $(CXXOBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

$(CXXOBJECTS): %.o: %.cpp
	$(CXX) $< -o $@  $(CXXFLAGS) $(CPPFLAGS)

clean:
	rm -rf *.o *.so*"""

    CONDITIONAL_LOGIC = \
"""
ifeq ($(__CONDITIONAL_DEF__), y)
SRCS += $(SRC_DIR)/__SRC_FILE_NAME__
endif"""

# Definitions for test code

    TEST_VECTOR_LENGTH = 4
    TEST_RANDOM_BIT_LENGTH = 7
    TEST_RANDOM_STRING_MAX_LENGTH = 16

    INT_LIMIT = (-2147483648, 2147483647)
    LONG_LIMIT = (-9223372036854775808, 9223372036854775807)

    TEST_VERIFY = \
"""
#define VERIFY(f)   { \\
                        if (!(f)) \\
                            { \\
                                ALOGE("%s: %s fail, return false", __func__, #f); \\
                                return false; \\
                            } \\
                    }
"""

    COMPARE_STR = "COMPARE_STR"
    TEST_COMPARE_STR = \
"""
static bool COMPARE_STR(string& str1, string& str2, const char *output)
{
    ALOGD("%s: %s: str1 size=%d, str2 size=%d", __func__, output, str1.size(), str2.size());
    if (!str1.compare(str2))
    {
        ALOGD("%s: same %s", __func__, output);
        return true;
    }
    else
    {
        ALOGE("%s: mis-matched %s", __func__, output);
        return false;
    }
}
"""

    TEST_COMPARE_VEC = \
"""
template<typename VectorType>
static bool COMPARE_VEC(vector<VectorType>& vec1, vector<VectorType>& vec2, const char *output)
{
    ALOGD("%s: %s: vec1 size=%d, vec2 size=%d", __func__, output, vec1.size(), vec2.size());
    if (vec1 == vec2)
    {
        ALOGD("%s: same %s", __func__, output);
        return true;
    }
    else
    {
        ALOGE("%s: mis-matched %s", __func__, output);
        return false;
    }
}
"""

    TEST_COMPARE_ARY = \
"""
template<typename ArrayType, size_t N>
static bool COMPARE_ARY(array<ArrayType, N>& array1, array<ArrayType, N>& array2, const char *output)
{
    ALOGD("%s: %s: array1 size=%d, array2 size=%d", __func__, output, array1.size(), array2.size());
    if (array1 == array2)
    {
        ALOGD("%s: same %s", __func__, output);
        return true;
    }
    else
    {
        ALOGE("%s: mis-matched %s", __func__, output);
        return false;
    }
}
"""

    COMPARE_BUF = "COMPARE_BUF"
    TEST_COMPARE_BUF = \
"""
static bool COMPARE_BUF(void *buf1, void *buf2, size_t length, const char *output)
{
    ALOGD("%s: size: %d: %s", __func__, length, output);
    if (!memcmp(buf1, buf2, length))
    {
        ALOGD("%s: same %s", __func__, output);
        return true;
    }
    else
    {
        ALOGE("%s: mis-matched %s", __func__, output);
        return false;
    }
}
"""

    COMPARE_OBJ = "COMPARE_OBJ"
    TEST_COMPARE_OBJ = \
"""
template<typename ObjType>
static bool COMPARE_OBJ(ObjType &obj1, ObjType &obj2, const char *output)
{
    ALOGD("%s: %s: obj1 size=%d, obj2 size=%d", __func__, output, sizeof(obj1), sizeof(obj2));
    if (obj1 == obj2)
    {
        ALOGD("%s: same %s", __func__, output);
        return true;
    }
    else
    {
        ALOGE("%s: mis-matched %s", __func__, output);
        return false;
    }
}
"""

    TEST_MK = \
"""LOCAL_DIR_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PATH := $(LOCAL_DIR_PATH)
LOCAL_MODULE := __TEST_MODULE_NAME__
LOCAL_PROPRIETARY_MODULE := true

LOCAL_C_INCLUDES := \\
    $(LOCAL_PATH)/../include

LOCAL_SRC_FILES := \\
    __TEST_CODE_SOURCE_FILE__

LOCAL_SHARED_LIBRARIES := \\
    __USED_SHARED_LIBS__

LOCAL_STATIC_LIBRARIES := \\
    __USED_STATIC_LIBS__

include $(BUILD_EXECUTABLE)"""
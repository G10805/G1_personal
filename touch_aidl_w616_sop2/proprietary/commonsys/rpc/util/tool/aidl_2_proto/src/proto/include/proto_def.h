/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

/*******************************************************************************
 * Proto keyword
 *******************************************************************************/
#define PROTO_IMPORT            "import"

#define PROTO_MESSAGE           "message"

#define PROTO_OPTION            "option"

#define PROTO_PACKAGE           "package"

#define PROTO_REPEATED          "repeated"

#define PROTO_SYNTAX            "syntax"

#define PROTO_ENUM              "enum"

#define PROTO_OPTIONAL          "optional"

#define PROTO_ONEOF             "oneof"

/*******************************************************************************
 * Proto data type
 *******************************************************************************/
#define PROTO_TYPE_ARRAY        "repeated"

#define PROTO_TYPE_BOOL         "bool"

#define PROTO_TYPE_BYTES        "bytes"

#define PROTO_TYPE_INT32        "int32"

#define PROTO_TYPE_INT64        "int64"

#define PROTO_TYPE_UINT32       "uint32"

#define PROTO_TYPE_STRING       "string"

#define PROTO_TYPE_GENERIC      "generic"

/*******************************************************************************
 * Misc
 *******************************************************************************/
#define PROTO_MESSAGE_REQ       "Req"
#define PROTO_MESSAGE_IND       "Ind"
#define PROTO_MESSAGE_CFM       "Cfm"

#define LINE_SUFFIX             ";"

#define PROTO_FILE_SUFFIX       ".proto"

#define PROTO_CALLBACK_SUFFIX   "Callback"

#define HAL_STATUS_NAME         "HalStatus"
#define HAL_STATUS_PROTO_FILE   (HAL_STATUS_NAME PROTO_FILE_SUFFIX)

#define PROTO_DEFAULT_SYNTAX    "syntax = \"proto3\";"

#define PROTO_DEFAULT_OPTION    "option optimize_for = LITE_RUNTIME;"

#define PROTO_ENUM_RESERVED_SUFFIX  "_RESERVED = 0;"

#define PROTO_ENUM_RESERVED_PLACE_HOLDER  "ENUM_RESERVED_PLACE_HOLDER"

#define PROTO_ENUM_ALLOW_ALIAS  "option allow_alias = true;"

#define PROTO_ENUM_ALLOW_ALIAS_PLACE_HOLDER  "ENUM_ALLOW_ALIAS_PLACE_HOLDER"

#define PROTO_ONEOF_SUFFIX      "OneOf"

#define PROTO_PACKAGE_HAL       "package hal;"

#define PROTO_HAL_STATUS_MESSAGE    "\
message HalStatus\n\
{\n\
    int32 status = 1;\n\
    string info = 2;\n\
}"

#define PROTO_HAL_STATUS    \
PROTO_DEFAULT_SYNTAX \
"\n" \
"\n" \
PROTO_PACKAGE_HAL \
"\n" \
"\n" \
PROTO_DEFAULT_OPTION \
"\n" \
"\n" \
PROTO_HAL_STATUS_MESSAGE

#define PROTO_ANDROID_BP        "\
cc_library_static {\n\
    name: \"libqti-hal-proto\",\n\
    vendor: true,\n\
\n\
    proto: {\n\
        canonical_path_from_root: false,\n\
        export_proto_headers: true,\n\
        type: \"lite\",\n\
    },\n\
\n\
    local_include_dirs: [\n\
        \"./\",\n\
    ],\n\
\n\
    srcs: [\n\
        \"*.proto\",\n\
    ],\n\
}"

#define PROTO_LINUX_MAKEFILE        "\
LIBRARY=libqti-hal-proto.so.0\n\
\n\
CUR_DIR = $(shell pwd)\n\
SRC_DIR = $(CUR_DIR)/gen\n\
\n\
CXXFLAGS += -c -std=c++17 -fPIC\n\
CXXFLAGS += -Wall -Werror -Wno-unused-function -Wno-ignored-attributes\n\
CPPFLAGS += -I$(SRC_DIR)\n\
LDFLAGS += -lstdc++ -lprotobuf-lite\n\
LDFLAGS += -shared -Wl,-soname,$(LIBRARY)\n\
\n\
PBCCSRCS := $(wildcard $(SRC_DIR)/*.pb.cc)\n\
PBCCOBJECTS := $(PBCCSRCS:.pb.cc=.o)\n\
\n\
$(LIBRARY): $(PBCCOBJECTS)\n\
	$(CC) $^ -o $@ $(LDFLAGS)\n\
\n\
$(PBCCOBJECTS): %.o: %.pb.cc\n\
	$(CXX) $< -o $@  $(CXXFLAGS) $(CPPFLAGS)\n\
\n\
clean:\n\
	rm -rf *.o *.so*\n\
"

/* Proto file type */
typedef uint32_t    ProtoType;

#define PROTO_TYPE_REQ     ((ProtoType) 0x0)
#define PROTO_TYPE_IND     ((ProtoType) 0x1)
#define PROTO_TYPE_COMMON  ((ProtoType) 0x2)
#define PROTO_TYPE_ONEOF   ((ProtoType) 0x3)
#define PROTO_TYPE_ENUM    ((ProtoType) 0x4)
#define PROTO_TYPE_CFM     ((ProtoType) 0x5)
#define PROTO_TYPE_UNKNOW  ((ProtoType) 0xFF)

/* Proto node type */
typedef uint32_t    ProtoNodeType;

#define PROTO_NODE_UNKNOWN  ((ProtoNodeType) 0x0)
#define PROTO_NODE_MESSAGE  ((ProtoNodeType) 0x1)
#define PROTO_NODE_ENUM     ((ProtoNodeType) 0x2)
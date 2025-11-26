/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include "apcc_config.h"

/* APCC file type */
typedef uint32_t    ApccType;

#define APCC_TYPE_BASE                              ((ApccType) 0)

/* 0. folder: . */
#define APCC_TYPE_ANDROID_BP                        ((ApccType) (APCC_TYPE_BASE + 0))

/* 1. folder: client */
#define APCC_TYPE_HAL_API_HEADER                    ((ApccType) (APCC_TYPE_BASE + 1))

#define APCC_TYPE_HAL_RPC_HEADER                    ((ApccType) (APCC_TYPE_BASE + 2))
#define APCC_TYPE_HAL_RPC_SOURCE                    ((ApccType) (APCC_TYPE_BASE + 3))

#define APCC_TYPE_HAL_STATUS_UTIL_HEADER            ((ApccType) (APCC_TYPE_BASE + 4))
#define APCC_TYPE_HAL_STATUS_UTIL_SOURCE            ((ApccType) (APCC_TYPE_BASE + 5))

#define APCC_TYPE_CLIENT_ANDROID_BP                 ((ApccType) (APCC_TYPE_BASE + 6))

/* 2. folder: include */
#define APCC_TYPE_HAL_MESSAGE_DEF_HEADER            ((ApccType) (APCC_TYPE_BASE + 7))
#define APCC_TYPE_HAL_SOMEIP_DEF_HEADER             ((ApccType) (APCC_TYPE_BASE + 8))

/* 3. folder: message */
#define APCC_TYPE_HAL_MSG_HEADER                    ((ApccType) (APCC_TYPE_BASE + 9))
#define APCC_TYPE_HAL_MSG_SOURCE                    ((ApccType) (APCC_TYPE_BASE + 10))

/* 4. folder: server */
#define APCC_TYPE_SERVICE_SOURCE                    ((ApccType) (APCC_TYPE_BASE + 11))

#define APCC_TYPE_HAL_SERVICE_HEADER                ((ApccType) (APCC_TYPE_BASE + 12))
#define APCC_TYPE_HAL_SERVICE_SOURCE                ((ApccType) (APCC_TYPE_BASE + 13))

#define APCC_TYPE_HAL_RPC_SERVER_HEADER             ((ApccType) (APCC_TYPE_BASE + 14))
#define APCC_TYPE_HAL_RPC_SERVER_SOURCE             ((ApccType) (APCC_TYPE_BASE + 15))

#define APCC_TYPE_HAL_IMPL_HEADER                   ((ApccType) (APCC_TYPE_BASE + 16))
#define APCC_TYPE_HAL_IMPL_SOURCE                   ((ApccType) (APCC_TYPE_BASE + 17))

#define APCC_TYPE_HAL_SERVER_API_HEADER             ((ApccType) (APCC_TYPE_BASE + 18))

#define APCC_TYPE_SERVER_HAL_STATUS_UTIL_HEADER     ((ApccType) (APCC_TYPE_BASE + 19))
#define APCC_TYPE_SERVER_HAL_STATUS_UTIL_SOURCE     ((ApccType) (APCC_TYPE_BASE + 20))

#define APCC_TYPE_SERVER_ANDROID_BP                 ((ApccType) (APCC_TYPE_BASE + 21))
#define APCC_TYPE_SERVER_LIB_ANDROID_BP             ((ApccType) (APCC_TYPE_BASE + 22))
#define APCC_TYPE_SERVER_MAIN_ANDROID_BP            ((ApccType) (APCC_TYPE_BASE + 23))

#define APCC_TYPE_MAX                               APCC_TYPE_SERVER_MAIN_ANDROID_BP

#define APCC_TYPE_COUNT                             (APCC_TYPE_MAX - APCC_TYPE_BASE + 1)

/* generic comment added in source file generated */
#define APCC_GENERIC_COMMENT    "/* auto-generated source file */\n"

/* C++ keyword */
#define CPP_PRAGMA                                  "#pragma once"

#define CPP_INCLUDE                                 "#include"

#define CPP_USING                                   "using"

#define CPP_DEFINE                                  "#define"

#define CPP_NAMESPACE                               "namespace"

#define CPP_STRUCT                                  "struct"

#define CPP_TYPEDEF                                 "typedef"

#define CPP_TYPEDEF_STRUCT                          "typedef struct"

#define CPP_STATIC                                  "static"

#define CPP_VOID                                    "void"

#define CPP_RETURN                                  "return"

#define CPP_IF                                      "if"
#define CPP_ELSE                                    "else"

#define CPP_DO                                      "do"
#define CPP_WHILE                                   "while"

#define CPP_SWITCH                                  "switch"
#define CPP_CASE                                    "case"
#define CPP_BREAK                                   "break"
#define CPP_DEFAULT                                 "default"

#define CPP_CONST                                   "const"

#define CPP_CLASS                                   "class"

#define CPP_PUBLIC                                  "public"

#define CPP_PRIVATE                                 "private"

#define CPP_FOR                                     "for"

#define CPP_STATIC_CAST                             "static_cast"
#define CPP_REINTERPRET_CAST                        "reinterpret_cast"

#define CPP_NEW                                     "new"
#define CPP_DELETE                                  "delete"

#define CPP_OVERRIDE                                "override"

#define CPP_NULL                                    "NULL"
#define CPP_NULLPTR                                 "nullptr"

#define CPP_MAIN                                    "main"

/* basic data type */
#define CPP_UINT8                                   "uint8_t"
#define CPP_UINT16                                  "uint16_t"
#define CPP_UINT32                                  "uint32_t"
#define CPP_UINT64                                  "uint64_t"
#define CPP_CHAR                                    "char"
#define CPP_CHAR16                                  "char16_t"
#define CPP_INT8                                    "int8_t"
#define CPP_INT16                                   "int16_t"
#define CPP_INT32                                   "int32_t"
#define CPP_INT64                                   "int64_t"
#define CPP_INT                                     "int"
#define CPP_LONG                                    "long"
#define CPP_BOOL                                    "bool"

#define CPP_SIZE_T                                  "size_t"

#define CPP_TRUE                                    "true"
#define CPP_FALSE                                   "false"

#define CPP_AUTO                                    "auto"

/* STL */
#define CPP_ARRAY                                   "array"
#define CPP_STRING                                  "string"
#define CPP_VECTOR                                  "vector"
#define CPP_MAP                                     "map"
#define CPP_OPTOINAL                                "optional"
#define CPP_PAIR                                    "pair"
#define CPP_SHARED_PTR                              "shared_ptr"
#define CPP_THREAD                                  "thread"
#define CPP_TUPLE                                   "tuple"
#define CPP_CHRONO                                  "chrono"

#define CPP_INT32_VECTOR                            "vector<int32_t>"
#define CPP_BYTE_VECTOR                             "vector<uint8_t>"
#define CPP_BYTE_VECTOR_REF                         "vector<uint8_t>&"

#define CPP_POINTER                                 "*"
#define CPP_CHAR_POINTER                            "char*"
#define CPP_CHAR_POINTER_POINTER                    "char**"
#define CPP_UINT8_POINTER                           "uint8_t*"
#define CPP_VOID_POINTER                            "void*"
#define CPP_VOID_POINTER_POINTER                    "void**"

#define CPP_REF                                     "&"

#define HEADER_FILE_SUFFIX                          ".h"

#define STD_NAMESPACE                               "std"

#define STD_GET_FUNC                                "get"
#define STD_MOVE_FUNC                               "move"
#define STD_MAKE_SHARED_FUNC                        "make_shared"
#define STD_ENABLE_SHARED_FROM_THIS                 "enable_shared_from_this"
#define STD_SHARED_FROM_THIS_FUNC                   "shared_from_this()"

#define CSTDINT_HEADER_FILE                         "cstdint"

#define OPEN_SOMEIP_NAME                            "openSomeip"
#define CLOSE_SOMEIP_NAME                           "closeSomeip"
#define SEND_NAME                                   "send"
#define SEND_RESPONSE_NAME                          "sendResponse"
#define MAP_2_SOMEIP_INSTANCE_ID_NAME               "map2SomeipInstanceId"
#define GET_INSTANCE_ID_AVAILABLE_NAME              "getInstanceIdAvailable"
#define EXIST_INSTANCE_NAME                         "existInstance"
#define REMOVE_INSTANCE_NAME                        "removeInstance"
#define GET_INTERFACE_NAME                          "getInterface"
#define SET_PROXY_NAME                              "setProxy"
#define SET_ADD_HEADER_FLAG_NAME                    "setAddHeaderFlag"

#define HAL_RPC_CLASS                               "HalRpc"
#define HAL_RPC_CLIENT_CLASS                        "HalRpcClient"
#define HAL_RPC_SERVER_CLASS                        "HalRpcServer"
#define HAL_RPC_INIT_FUNC                           (HAL_RPC_CLASS "::" "init")
#define HAL_RPC_OPEN_SOMEIP_FUNC                    (HAL_RPC_CLASS "::" OPEN_SOMEIP_NAME)
#define HAL_RPC_CLOSE_SOMEIP_FUNC                   (HAL_RPC_CLASS "::" CLOSE_SOMEIP_NAME)
#define HAL_RPC_SEND_FUNC                           (HAL_RPC_CLASS "::" SEND_NAME)
#define HAL_RPC_SEND_RESPONSE_FUNC                  (HAL_RPC_CLASS "::" SEND_RESPONSE_NAME)
#define HAL_RPC_MAP_2_SOMEIP_INSTANCE_ID_FUNC       (HAL_RPC_CLASS "::" MAP_2_SOMEIP_INSTANCE_ID_NAME)
#define HAL_RPC_GET_INSTANCE_ID_AVAILABLE_FUNC      (HAL_RPC_CLASS "::" GET_INSTANCE_ID_AVAILABLE_NAME)
#define HAL_RPC_EXIST_INSTANCE_FUNC                 (HAL_RPC_CLASS "::" EXIST_INSTANCE_NAME)
#define HAL_RPC_REMOVE_INSTANCE_FUNC                (HAL_RPC_CLASS "::" REMOVE_INSTANCE_NAME)
#define HAL_RPC_GET_INTERFACE_FUNC                  (HAL_RPC_CLASS "::" GET_INTERFACE_NAME)
#define HAL_RPC_SET_PROXY_FUNC                      (HAL_RPC_CLASS "::" SET_PROXY_NAME)
#define HAL_RPC_SET_ADD_HEADER_FLAG_FUNC            (HAL_RPC_CLASS "::" SET_ADD_HEADER_FLAG_NAME)
#define HAL_RPC_CLIENT_SEND_FUNC                    (HAL_RPC_CLIENT_CLASS "::" SEND_NAME)
#define HAL_RPC_SERVER_SEND_FUNC                    (HAL_RPC_SERVER_CLASS "::" SEND_NAME)

#define QTI_HAL_RPC_NAMESPACE                       "::qti::hal::rpc"
#define QTI_HAL_RPC                                 (QTI_HAL_RPC_NAMESPACE "::" HAL_RPC_CLASS)
#define QTI_HAL_RPC_CLIENT                          (QTI_HAL_RPC_NAMESPACE "::" HAL_RPC_CLIENT_CLASS)
#define QTI_HAL_RPC_SERVER                          (QTI_HAL_RPC_NAMESPACE "::" HAL_RPC_SERVER_CLASS)
#define QTI_SOMEIP_MAP                              (QTI_HAL_RPC_NAMESPACE "::" SOMEIP_MAP_NAME)

#define HAL_DEFAULT_INSTANCE                        "default"
#define HAL_DEFAULT_INSTANCE_ID                     0

#define PROXY_VAR                                   "proxy_"
#define STATIC_PROXY_VAR                            "sProxy"
#define GET_STATIC_PROXY_NAME                       "getStaticProxy"

#define NDK_SCOPED_ASTATUS                          "ndk::ScopedAStatus"
#define SCOPED_ASTATUS                              "ScopedAStatus"
#define SCOPED_ASTATUS_OK                           "ScopedAStatus::ok()"
#define SCOPED_ASTATUS_IS_OK_FUNC                   "isOk()"
#define SCOPED_ASTATUS_VAR                          "_scoped_astatus"

#define AIDL_BN_NAME                                "Bn"

#define ANDROID_BASE_MACROS_HEADER                  "android-base/macros.h"

#define HAL_RPC_HEADER                              "hal_rpc.h"

#define DISALLOW_COPY_AND_ASSIGN_MACRO              "DISALLOW_COPY_AND_ASSIGN"

#define DEFAULT_SPACE                               "    "

#define IS_VALID_FUNC                               "isValid()"

#define PARSE_HAL_STATUS_FUNC                       "ParseHalStatus"

#define RPC_NAME_LOWER                              "rpc"

#define HAL_MODE_PROPERTY_VALUE_RPC                 RPC_NAME_LOWER

#define VALIDATE_AND_CALL_FUNC                      "validateAndCall"
#define VALIDATE_AND_CALL_WITH_LOCK_FUNC            "validateAndCallWithLock"
#define VALIDATE_AND_CALL_WITH_LOCK_TYPE            "std::unique_lock<std::recursive_mutex>*"
#define ACQUIRE_GLOBAL_LOCK_FUNC                    "acquireGlobalLock"

#define LOCK_NAME                                   "lock"

#define STATUS_UNKNOWN_SUFFIX                       "_UNKNOWN"
#define STATUS_FAILURE_UNKNOWN_NAME                 "FAILURE_UNKNOWN"
#define STATUS_ERROR_UNKNOWN_NAME                   "ERROR_UNKNOWN"
#define STATUS_ERROR_UNKNOWN_CODE                   "HAL_ERROR_UNKNOWN"

#define CREATE_SCOPED_ASTATUS_FUNC                  "CreateScopedAStatus"

#define CREATE_SCOPED_ASTATUS_ERROR_FUNC            "CreateScopedAStatusError"

#define FROM_SERVICE_SPECIFIC_ERROR_FUNC            "fromServiceSpecificError"
#define FROM_SERVICE_SPECIFIC_ERROR_WITH_MESSAGE_FUNC   "fromServiceSpecificErrorWithMessage"

#define DELIM_LINE  "/* ------------------------------------------------------------------------------------ */"

#define LOG_FAIL_TO_OPEN_SOMEIP     "IFDBG(DebugOut(DEBUG_ERROR, TEXT(\"%s: fail to open someip\"), __func__));"
#define LOG_FAIL_TO_START_SOMEIP    "IFDBG(DebugOut(DEBUG_ERROR, TEXT(\"%s: fail to start someip in %d sec\"), __func__, total_timeout));"

#define HAL_MESSAGE_DEF_HEADER                      "hal_message_def.h"

#define HAL_REQ_BASE_NAME                           "HAL_REQ_BASE"
#define HAL_CFM_BASE_NAME                           "HAL_CFM_BASE"
#define HAL_IND_BASE_NAME                           "HAL_IND_BASE"
#define HAL_UPSTREAM_REQ_BASE_NAME                  "HAL_UPSTREAM_REQ_BASE"
#define HAL_DOWNSTREAM_CFM_BASE_NAME                "HAL_DOWNSTREAM_CFM_BASE"

#define SOMEIP_SERVICE_ID_TYPE                      "SomeipServiceId"
#define HAL_MSG_TYPE                                "HalMsg"
#define MSG_TYPE_NAME                               "msgType"

/* HalRpc override func */
#define INIT_SOMEIP_CONTEXT_FUNC                    "initSomeipContext"
#define INIT_HAL_STATUS_FUNC                        "initHalStatus"
#define DELETE_HAL_STATUS_FUNC                      "deleteHalStatus"
#define ADD_HAL_STATUS_FUNC                         "addHalStatus"
#define IS_HAL_REQ_FUNC                             "isHalReq"
#define IS_HAL_CFM_FUNC                             "isHalCfm"
#define IS_HAL_IND_FUNC                             "isHalInd"
#define IS_HAL_UPSTREAM_REQ_FUNC                    "isHalUpstreamReq"
#define HANDLE_HAL_REQ_FUNC                         "handleHalReq"
#define HANDLE_HAL_CFM_FUNC                         "handleHalCfm"
#define HANDLE_HAL_IND_FUNC                         "handleHalInd"
#define HANDLE_HAL_UPSTREAM_REQ_FUNC                "handleHalUpstreamReq"
#define INIT_FUNC                                   "init"
#define DEINIT_FUNC                                 "deinit"

#define ADD_HAL_CALLBACK_FUNC                       "addCallback"
#define ADD_HAL_CALLBACK2_FUNC                      "addCallback2"
#define ADD_HAL_CALLBACK_RESULT_FUNC                "addCallbackResult"
#define CLEAR_HAL_CALLBACK_FUNC                     "clearCallback"

#define GET_HAL_STATUS_FUNC                         "getHalStatus"

#define GET_HAL_STATUS_CODE_FUNC                    "GetHalStatusCode"

#define WAIT_HAL_STATUS_FUNC                        "waitHalStatus"
#define REMOVE_HAL_STATUS_FUNC                      "removeHalStatus"

#define ATOI_FUNC                                   "atoi"

#define MOD_BEGIN_STR                               " {"
#define MOD_END_STR                                 "}"

#define HAL_STATUS_CODE_NAME                        "HalStatusCode"

#define HAL_STATUS_TYPE                             "HalStatus"
#define HAL_STATUS_VAR                              "halStatus"
#define HAL_STATUS_INT_VAR                          "hal_status_"
#define HAL_STATUS_POINTER                          "HalStatus*"
#define HAL_STATUS_POINTER_POINTER                  "HalStatus**"
#define HAL_STATUS                                  (QTI_HAL_RPC_NAMESPACE "::" HAL_STATUS_TYPE)

#define HAL_STATUS_TUPLE_TYPE                       "HalStatusT"
#define HAL_STATUS_TUPLE_SUFFIX                     "Result"
#define HAL_STATUS_TUPLE                            (QTI_HAL_RPC_NAMESPACE "::" HAL_STATUS_TUPLE_TYPE)

#define HAL_STATUS_PARAM_TYPE                       "HalStatusParam"
#define HAL_STATUS_PARAM_VAR                        "_hal_status_param"

#define INSTANCE_ID_TYPE                            CPP_UINT16
#define INSTANCE_ID_NAME                            "instanceId"
#define INSTANCE_ID_VAR_NAME                        "instance_id_"
#define INVALID_HAL_INSTANCE_ID_NAME                "INVALID_HAL_INSTANCE_ID"

#define INTERFACE_NAME                              "interface"
#define INTERFACE_VAR                               "_intf"

#define SERVICE_NAME                                "Service"

#define SERVICE_SUFFIX                              "_service"

#define INIT_SUFFIX                                 "_init"

#define RPC_SERVER_NAME                             "RpcServer"

#define RPC_SERVER_SUFFIX                           "_rpc_server"

/* + hal rpc util */
/* header name */
#define AIDL_RETURN_UTIL_NAME                       "aidl_return_util"
#define AIDL_SYNC_UTIL_NAME                         "aidl_sync_util"
/* include */
#define HAL_AIDL_RETURN_UTIL_HEADER                 (AIDL_RETURN_UTIL_NAME HEADER_FILE_SUFFIX)
#define HAL_AIDL_SYNC_UTIL_HEADER                   (AIDL_SYNC_UTIL_NAME HEADER_FILE_SUFFIX)
/* using */
#define HAL_USING_VALIDATE_AND_CALL                 (AIDL_RETURN_UTIL_NAME "::" VALIDATE_AND_CALL_FUNC)
#define HAL_USING_VALIDATE_AND_CALL_WITH_LOCK       (AIDL_RETURN_UTIL_NAME "::" VALIDATE_AND_CALL_WITH_LOCK_FUNC)
#define HAL_USING_ACQUIRE_GLOBAL_LOCK               (AIDL_SYNC_UTIL_NAME "::" ACQUIRE_GLOBAL_LOCK_FUNC)
/* - hal rpc util */

#define UPSTREAM_NAME                               "Upstream"
#define DOWNSTREAM_NAME                             "Downstream"

#define HAL_NDK_LIB                                 "\"android.hardware.hal-V1-ndk\""

#define QTI_HAL_MESSAGE_LIB                         "\"libqti-hal-message\""
#define QTI_HAL_RPC_LIB                             "\"libqti-hal-rpc\""
#define QTI_HAL_PROTO_LIB                           "\"libqti-hal-proto\""
#define QTI_HAL_IMPL_LIB                            "\"libqti-hal-impl\""
#define QTI_HAL_SERVICE_LIB                         "\"libqti-hal-service\""
#define QTI_HAL_SERVICE_BIN                         "\"qti-hal-service\""
#define QTI_HAL_AIDL_UTIL_SOURCE                    "        \"aidl/util/*.cpp\",\n"
#define QTI_HAL_AIDL_UTIL_HEADER                    "        \"aidl/util\",\n"
#define QTI_HAL_SERVER_UTIL_SOURCE                  "        \"util/*.cpp\",\n"
#define QTI_HAL_SERVER_UTIL_HEADER                  "        \"util\",\n"

typedef uint16_t    CppTypeT;
#define CPP_TYPE_BASIC      ((CppTypeT) 0x0)
#define CPP_TYPE_STL        ((CppTypeT) 0x1)
#define CPP_TYPE_COMMON     ((CppTypeT) 0x2)

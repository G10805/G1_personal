/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <cctype>
#include <cstring>
#ifndef EXCLUDE_EXPERIMENTAL_FILESYSTEM
#include <experimental/filesystem>
#else
#include <filesystem>
#endif
#include <functional>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "apcc_config.h"
#include "apcc_def.h"

using std::fstream;
using std::ifstream;
using std::istringstream;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::stringstream;
using std::vector;

using std::endl;

using GenFunc = std::function<void(stringstream&)>;


struct HalBaseInstance
{
    vector<string> includeHeader;
    vector<string> systemHeader;
    vector<string> utilHeader;
    vector<string> halHeader;
    vector<string> usingType;
};

struct CppType
{
    string name;
    CppTypeT type;
};

struct CppTypeInfo: CppType
{
    string includeHeader;
    string usingType;
    string typeRef;
};

typedef struct
{
    string itemType;
    string variableName;
    string arraySize;
    vector<string> item;
} ArrayStruct;

typedef struct
{
    string type;
    string name;
} FuncParameter;

typedef struct
{
    string returnType;
    string funcName;
    vector<FuncParameter> param;
} FuncType;

typedef struct
{
    string name;
    vector<FuncParameter> field;
} StructType;

typedef struct
{
    string halCfm;
    string halResultName;
    string halResultType;
    vector<FuncParameter> halResultParam;
    FuncType halResultParseFunc;
} HalResultInfo;

typedef struct
{
    string typeName;
    string payloadName;
    StructType payloadStruct;
    string msgTransportName;
    string reqBase;
    string reqCount;
    string indBase;
    string indCount;
    string cfmBase;
    string cfmCount;
    string upReqBase;
    string upReqCount;
    string downCfmBase;
    string downCfmCount;
    string isHalReq;
    string isHalCfm;
    string isHalInd;
    string isHalUpReq;
    string isHalDownCfm;
    string clientAppName;
    string serverAppName;
    string serviceId;
    string eventGroupId;
} HalMsgInfo;

typedef struct
{
    string intfName;
    string className;
    string headerFile;
    string usingType;
} AidlIntfInfo;

typedef struct
{
    string className;
    string classVar;
    bool existCallback;
    bool existCallback2;
    bool existCallbackResult;
    FuncParameter callbackVar;  /* e.g. shared_ptr<IHalCallback> callback_; */
    FuncParameter callbackVar2; /* e.g. shared_ptr<IHalCallback2> callback2_; */
    FuncParameter callbackVarResult; /* e.g. shared_ptr<IHalCallbackResult> callback_result_; */
    FuncParameter staticInst;   /* e.g. static std::shared_ptr<BnHal> sHalRpc;*/
    FuncType createFunc;
    vector<FuncType> callbackFunc;
    vector<FuncType> callback2Func;
} HalClassInfo;

typedef struct
{
    string className;
    string classVar;
    bool existCallback;
    bool existCallback2;
    bool existCallbackResult;
    bool isClearCallbackRequired;
    FuncParameter callbackVar;  /* e.g. vector<shared_ptr<IHalCallback>> callback_; */
    FuncParameter callbackVar2; /* e.g. vector<shared_ptr<IHalCallback2>> callback2_; */
    FuncParameter callbackVarResult; /* e.g. vector<shared_ptr<IHalCallbackResult>> callback_result_; */
    FuncParameter staticInst;   /* e.g. static std::shared_ptr<BnHal> sHalRpc;*/
    FuncParameter staticProxy;  /* e.g. static std::shared_ptr<HalRpc> sProxy;*/
    string halModePropertyName; /* e.g. persist.vendor.hal.hal_mode */
} HalRpcClassInfo;

typedef struct
{
    bool gen;
    string className;
    vector<FuncType> func;
    string halRpcBaseClassName;
    vector<string> halRpcClassName;
    vector<string> halRpcInclude;
} HalRpcServiceInfo;

typedef struct
{
    bool gen;       /* true: gen hal status util header and source */
    string aidlHalStatusType;
    bool existHalStatusType;
    string defaultHalStatusCode;

    string halStatusHeader;
    string halStatusUsing;  /* e.g. aidl::android::hardware::hal::HalStatusCode */

    bool existCreateHalStatusFunc;
    vector<FuncType> createHalStatusFunc;   /* e.g. createHalStatus */
} HalStatusInfo;

typedef struct
{
    string name;
    string nameUpper;
    string nameLower;
} AidlHalName;

typedef struct
{
    string intfName;
    AidlHalName halName;
    string headerFile;
    string halRpcHeader;
} AidlHalInfo;

typedef struct
{
    string intfName;
    FuncParameter param;
} AidlCallbackInfo;

typedef struct
{
    string intfName;
    string includeHeader;
    FuncParameter param;
    FuncParameter halRpcInstListParam;
    FuncParameter halRpcServerClassParam;
    FuncParameter halRpcServerInstListParam;
    vector<FuncType> halRpcServerApiFunc;
    vector<FuncType> createIntfFunc;
    vector<FuncType> getIntfFunc;
    vector<FuncType> removeIntfFunc;
    vector<FuncType> getIntfKeyFunc;
    vector<FuncParameter> intfKeyParam;
} AidlIntfReturnInfo;

typedef struct
{
    string serviceName;
    string serviceHeader;
    FuncType serviceInitFunc;
    string halRpcServerName;
    string halRpcServerHeader;
    string halRpcServerUsingType;
    vector<FuncType> halRpcServerApiFunc;
    string halApiHeader;
} HalServerInfo;

typedef struct
{
    string aidlCallbackName;
    string aidlCallbackVar;
    FuncType funcType;
} HalRpcCallbackUtilInfo;

typedef struct
{
    string className;
    string classType;
    string classVar;
    string classVarInt;
    FuncParameter intfParam;        /* e.g. std::shared_ptr<IHal> hal_; */
    FuncParameter callbackParam;    /* e.g. std::shared_ptr<IHalCallback> hal_callback_; */
    FuncParameter callback2Param;   /* e.g. std::shared_ptr<IHalCallback2> hal_callback2_; */
    FuncParameter staticInst;       /* e.g. static std::shared_ptr<BnHalRpcServer> sHalRpcServer;*/
    FuncParameter aidlIntfParam;
    vector<FuncParameter> createFuncParam;
    FuncType constructorDefault;
    FuncType constructor;
    FuncType deconstructor;
} HalRpcServerClassInfo;

typedef struct
{
    bool existCallback;
    string className;
    string classVar;
} HalCallbackClassInfo;

typedef struct
{
    string halBnName;           /* e.g. BnHal */
    string halBnHeaderFile;     /* e.g. aidl/android/hardware/hal/BnHal.h */
    string halHeaderFile;       /* e.g. aidl/android/hardware/hal/IHal.h */
} HalIntfFileInfo;

typedef struct
{
    ApccType apccType;
    string aidlFileName;            /* e.g. IHalMod.aidl */
    string protoFileName;           /* e.g. IHalMod.proto */
    string aidlIntfName;            /* e.g. IHalMod */
    string aidlCallbackName;        /* e.g. IHalModCallback */
    string aidlCallback2Name;       /* e.g. IHalModCallback2 */
    string aidlCallbackResultName;  /* e.g. IHalModResultCallback */
    string aidlPackageName;         /* e.g. android.hardware.hal */
    bool isMultiAidlIntf;
    bool isCfmDefined;
    bool isSingleSomeip;
    bool isAidlIntfRoot;
    AidlHalInfo aidlHalRoot;
    vector<string> aidlEnumType;
    vector<string> aidlIntfIncludeHeader;
    vector<string> aidlCallbackIncludeHeader;
    vector<string> aidlCallback2IncludeHeader;
    vector<string> aidlCallbackResultIncludeHeader;
    vector<string> aidlIntfUsingType;
    vector<string> aidlCallbackUsingType;
    vector<string> aidlCallback2UsingType;
    vector<string> aidlCallbackResultUsingType;
    string halName;                 /* e.g. HalMod */
    string halNameUpper;            /* e.g. HAL_MOD */
    string halNameLower;            /* e.g. hal_mod */
    string halCallbackName;         /* e.g. HalCallback */
    string halCallback2Name;        /* e.g. HalCallback2 */
    AidlHalName aidlCallbackHalName;
    AidlHalName aidlCallback2HalName;
    AidlHalName aidlCallbackResultHalName;  /* e.g. HalModResult */
    vector<string> halNamespace;    /* e.g. aidl::android::hardware::hal */
    vector<string> halRpcNamespace; /* e.g. aidl::android::hardware::hal::rpc */
    HalIntfFileInfo halIntfFileInfo;
    HalIntfFileInfo halCallbackFileInfo;
    HalIntfFileInfo halCallback2FileInfo;
    HalRpcClassInfo halRpcClassInfo;
    HalMsgInfo halMsgInfo;
    HalStatusInfo halStatusInfo;

    string halPayload;              /* e.g. HalPayload */
    string halCfmParam;             /* e.g. HalCfmParam */
    vector<string> halReq;          /* e.g. SampleReq */
    vector<string> halInd;          /* e.g. SampleInd */
    vector<string> halInd2;         /* e.g. SampleInd2 */
    vector<string> halCfm;          /* e.g. SampleCfm */
    vector<string> halUpReq;        /* e.g. SampleUpReq */
    vector<string> halDownCfm;      /* e.g. SampleDownCfm */
    vector<string> halReqDef;       /* e.g. HAL_MOD_SAMPLE_REQ */
    vector<string> halIndDef;       /* e.g. HAL_MOD_SAMPLE_IND */
    vector<string> halInd2Def;      /* e.g. HAL_MOD_SAMPLE_IND2 */
    vector<string> halCfmDef;       /* e.g. HAL_MOD_SAMPLE_CFM */
    vector<string> halUpReqDef;     /* e.g. HAL_MOD_SAMPLE_UP_REQ */
    vector<string> halDownCfmDef;   /* e.g. HAL_MOD_SAMPLE_DOWN_CFM */
    vector<string> halCommonMsg;
    vector<string> halEnumType;
    vector<string> halCallbackCommonMsg;
    vector<string> halCallbackEnumType;
    vector<string> halCallbackResultCommonMsg;
    vector<string> halCallbackResultEnumType;
    vector<StructType> halIndParamStruct;
    vector<StructType> halInd2ParamStruct;
    vector<StructType> halUpstreamReqParamStruct;
    vector<CppTypeInfo> aidlIntfParamType;
    vector<CppTypeInfo> aidlIntfReturnType;
    vector<CppTypeInfo> aidlCallbackParamType;
    vector<CppTypeInfo> aidlCallbackReturnType;
    vector<CppTypeInfo> aidlCallback2ParamType;
    vector<CppTypeInfo> aidlCallback2ReturnType;
    vector<CppTypeInfo> aidlCallbackResultParamType;
    vector<CppTypeInfo> aidlCallbackResultReturnType;
    vector<FuncType> halCommonFunc;
    vector<FuncType> halCommonCallback;
    vector<FuncType> halCommonCallback2;
    vector<FuncType> halCommonCallbackResult;
    bool handleHalCfm;

    vector<FuncType> halReqFunc;
    vector<FuncType> halIndFunc;
    vector<FuncType> halInd2Func;
    vector<FuncType> halCfmFunc;
    vector<FuncType> halUpReqFunc;
    vector<FuncType> halDownCfmFunc;
    vector<StructType> halCfmStruct;
    bool isHalImplEnabled;
    bool isAidlSyncApi;
    bool isValidateFuncSupported;
    vector<FuncType> halRpcFunc;
    vector<FuncType> halRpcInternalFunc;
    vector<FuncType> halRpcOverrideFunc;
    vector<FuncType> halRpcStaticFunc;
    vector<FuncType> halRpcHandleIndFunc;
    vector<FuncType> halRpcHandleInd2Func;
    vector<FuncType> halRpcHandleUpReqFunc;
    vector<HalRpcCallbackUtilInfo> halRpcCallbackUtilInfo;
    vector<FuncType> halApiFunc;
    vector<FuncType> halMsgTransportFunc;

    vector<FuncType> aidlCallbackFunc;
    vector<FuncType> aidlCallback2Func;
    vector<FuncType> aidlCallbackResultFunc;
    vector<AidlIntfInfo> halAidlIntfInfo;
    vector<HalResultInfo> halResultInfo;
    vector<HalResultInfo> halDownResultInfo;

    HalRpcServiceInfo halRpcServiceInfo;

    string intfProtoHeader; /* e.g. IHal.pb.h */
    string callbackProtoHeader; /* e.g. IHalCallback.pb.h */
    string callbackResultProtoHeader; /* e.g. IHalCallback.pb.h */

    HalRpcServerClassInfo halRpcServerClassInfo;
    HalCallbackClassInfo halCallbackClassInfo;
    HalCallbackClassInfo halCallback2ClassInfo;
    HalServerInfo halServerInfo;
    vector<FuncType> halCallbackFunc;
    vector<FuncType> halCallback2Func;
    vector<FuncType> halRpcServerFunc;
    vector<FuncType> halRpcServerReqFunc;
    vector<FuncType> halRpcServerCfmFunc;
    vector<FuncType> halRpcServerIndFunc;
    vector<FuncType> halRpcServerInd2Func;
    vector<FuncType> halRpcServerHandleReqFunc;
    vector<FuncType> halRpcServerOverrideFunc;
    vector<FuncType> halRpcServerPublicFunc;
    vector<FuncType> halRpcServerStaticFunc;
    vector<StructType> halRpcServerReqParamStruct;

    map<string, AidlCallbackInfo> aidlCallbackInfo;
    map<string, AidlIntfReturnInfo> aidlIntfReturnInfo;

    HalClassInfo halClassInfo;
} ApccGenInfo;

typedef struct
{
    string aidlPath;
    string protoPath;
    string genPath;
    string ndkLib;      /* HAL NDK lib */
    bool aidlSyncApi;   /* true: aidl sync api, false: aidl async api */
    bool multiCallback; /* true: multi callback, false: single callback */
    bool validateFunc;  /* true: add validate func when to call HalRpc internal func, false: not */
    bool removeFile;    /* true: remove existing source file before creating new file, false: not */
    bool defineCfm;     /* true: define cfm message, false: not */
    bool server;        /* true: gen source code for hal rpc server, false: not */
    bool halImplStub;   /* true: gen source code for hal impl stub, false: not */
    bool singleSomeip;  /* true: use single someip in multiple AIDL interfaces, false: not */
    bool removeIntf;    /* true: remove hal rpc server instance when aidl intf is removed, false: not */
    bool test;          /* true: enable APCC test mode (i.e. generate hal message empty function, false: not */
} ApccParam;

typedef struct
{
    string name;
    string folder;
} HalFile;

typedef struct
{
    ApccType type;
    string name;
    string path;
    bool gen;   /* true: generate source file, false: not */
} ApccFile;

typedef struct
{
    ApccParam param;
    uint32_t aidlIntfNumber;
    vector<string> aidlIntfName;
    vector<vector<ApccFile>> srcFile;
} ApccInfo;

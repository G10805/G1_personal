/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "aidl.h"
#include "aidl_util.h"

#include "apcc_gen.h"

#include "proto.h"
#include "proto_util.h"

#include "log.h"
#include "util.h"

static void hms_add_comment_todo(stringstream& ss)
{
    ss << SPACES << COMMENT_TODO << endl;
}

static void hms_add_return(const string& result, stringstream& ss)
{
    hms_add_comment_todo(ss);
    apcc_add_return_line(SPACES, result, ss);
}

static void hms_add_return(const FuncType& funcType, stringstream& ss)
{
    const string& returnType = funcType.returnType;

    if (apcc_is_void_type(returnType))
    {
        hms_add_comment_todo(ss);
    }
    else if (apcc_is_cpp_type(returnType, CPP_VOID_POINTER))
    {
        hms_add_return(CPP_NULL, ss);
    }
    else if (apcc_is_cpp_type(returnType, CPP_BOOL))
    {
        hms_add_return(CPP_FALSE, ss);
    }
    else
    {
        loge("%s: unknown return type: %s, func name: %s", __func__,
            returnType.c_str(), funcType.funcName.c_str());
    }
}

static void hms_add_func_impl(const vector<FuncType>& funcType, stringstream& ss)
{
    for (auto it: funcType)
    {
        ss << endl;
        apcc_add_func_impl(it, "", "", ss);
        add_module_begin("", ss);
        /* FIXME */
        hms_add_return(it, ss);
        add_module_end(ss);
    }
}

static string hms_get_proto_msg(const string& protoNamespace, const string& aidlIntfName, const string& halMsg)
{
    stringstream ss;
    ss << "::" << protoNamespace << "::" << aidlIntfName << '_' << halMsg;
    return ss.str();
}

static void hms_add_using(const ApccGenInfo& genInfo, const string& aidlIntfName,
    const vector<string>& halMsg, stringstream& ss)
{
    string protoNamespace;
    if (!halMsg.empty() &&
        valid_str(aidlIntfName) &&
        proto_get_namespace(aidlIntfName, protoNamespace))
    {
        for (auto it: halMsg)
        {
            apcc_add_using(it, hms_get_proto_msg(protoNamespace, aidlIntfName, it), ss);
        }
        ss << endl;
    }
}

/* ------------------------------------------------------------------------------------ */

static void hms_add_using(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* hal req */
    hms_add_using(genInfo, genInfo.aidlIntfName, genInfo.halReq, ss);

    /* hal cfm */
    hms_add_using(genInfo, genInfo.aidlIntfName, genInfo.halCfm, ss);

    /* hal ind */
    hms_add_using(genInfo, genInfo.aidlCallbackName, genInfo.halInd, ss);

    /* hal req (upstream) */
    hms_add_using(genInfo, genInfo.aidlCallbackResultName, genInfo.halUpReq, ss);

    /* hal cfm (downstream) */
    hms_add_using(genInfo, genInfo.aidlCallbackResultName, genInfo.halDownCfm, ss);
}

static void hms_add_include(const ApccGenInfo& genInfo, stringstream& ss)
{
    apcc_add_include_header(apcc_get_gen_file_name(genInfo.aidlIntfName, APCC_TYPE_HAL_MSG_HEADER), ss);
    apcc_add_include_header(PROTO_COMMON_HEADER, ss);
    ss << endl;

    apcc_add_include_header(genInfo.intfProtoHeader, ss);
    if (valid_str(genInfo.callbackProtoHeader))
    {
        apcc_add_include_header(genInfo.callbackProtoHeader, ss);
    }
    if (valid_str(genInfo.callbackResultProtoHeader))
    {
        apcc_add_include_header(genInfo.callbackResultProtoHeader, ss);
    }
    ss << endl;

    apcc_add_include_header(apcc_get_gen_file_name(genInfo.aidlHalRoot.intfName, APCC_TYPE_HAL_MESSAGE_DEF_HEADER), ss);
    ss << endl;
}

/* ------------------------------------------------------------------------------------ */

/* hal req */
static void gen_hal_req_serialize_source(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* 1. comment */
    apcc_add_comment_hal_msg(genInfo.halNameLower, COMMENT_HAL_REQ_SERIALIZE, ss);

    /* 2. func */
    hms_add_func_impl(genInfo.halReqFunc, ss);
    ss << endl;
}

static void gen_hal_req_parse_source(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* 1. comment */
    apcc_add_comment_hal_msg(genInfo.halNameLower, COMMENT_HAL_REQ_PARSE, ss);

    /* 2. func */
    hms_add_func_impl(genInfo.halRpcServerReqFunc, ss);
    ss << endl;
}

static void gen_hal_req_source(const ApccGenInfo& genInfo, stringstream& ss)
{
    gen_hal_req_serialize_source(genInfo, ss);

    gen_hal_req_parse_source(genInfo, ss);
}

/* ------------------------------------------------------------------------------------ */

/* hal cfm */
static void gen_hal_cfm_parse_source(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* 1. comment */
    apcc_add_comment_hal_msg(genInfo.halNameLower, COMMENT_HAL_CFM_PARSE, ss);

    /* 2. func */
    hms_add_func_impl(genInfo.halCfmFunc, ss);
    ss << endl;
}

static void gen_hal_cfm_serialize_source(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* 1. comment */
    apcc_add_comment_hal_msg(genInfo.halNameLower, COMMENT_HAL_CFM_SERIALIZE, ss);

    /* 2. func */
    hms_add_func_impl(genInfo.halRpcServerCfmFunc, ss);
    ss << endl;
}

static void gen_hal_cfm_source(const ApccGenInfo& genInfo, stringstream& ss)
{
    gen_hal_cfm_parse_source(genInfo, ss);

    gen_hal_cfm_serialize_source(genInfo, ss);
}

/* ------------------------------------------------------------------------------------ */

/* hal ind */
static void gen_hal_ind_parse_source(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* 1. comment */
    apcc_add_comment_hal_msg(genInfo.halNameLower, COMMENT_HAL_IND_PARSE, ss);

    /* 2. func */
    hms_add_func_impl(genInfo.halIndFunc, ss);
    ss << endl;
}

static void gen_hal_ind_serialize_source(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* 1. comment */
    apcc_add_comment_hal_msg(genInfo.halNameLower, COMMENT_HAL_IND_SERIALIZE, ss);

    /* 2. func */
    hms_add_func_impl(genInfo.halRpcServerIndFunc, ss);
}

static void gen_hal_ind_source(const ApccGenInfo& genInfo, stringstream& ss)
{
    gen_hal_ind_parse_source(genInfo, ss);

    gen_hal_ind_serialize_source(genInfo, ss);
}

/* ------------------------------------------------------------------------------------ */

/* hal req (upstream) */
static void gen_hal_up_req_source(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (valid_str(genInfo.aidlCallbackResultName))
    {
        /* 1. comment */
        apcc_add_comment_hal_msg(genInfo.halNameLower, COMMENT_HAL_UP_REQ, ss);

        /* 2. func */
        hms_add_func_impl(genInfo.halUpReqFunc, ss);
        ss << endl;
    }
}

/* ------------------------------------------------------------------------------------ */

/* hal cfm (downstream) */
static void gen_hal_down_cfm_source(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (valid_str(genInfo.aidlCallbackResultName))
    {
        /* 1. comment */
        apcc_add_comment_hal_msg(genInfo.halNameLower, COMMENT_HAL_DOWN_CFM, ss);

        /* 2. func */
        hms_add_func_impl(genInfo.halDownCfmFunc, ss);
        ss << endl;
    }
}

/* ------------------------------------------------------------------------------------ */

void gen_hal_msg_source(stringstream& ss)
{
    const ApccGenInfo& genInfo = apcc_get_gen_info();

    FUNC_ENTER();
    /* 1. comment */
    apcc_add_comment(ss);

    /* 2. include */
    hms_add_include(genInfo, ss);

    /* 3. using */
    hms_add_using(genInfo, ss);

    /* 4. hal req */
    gen_hal_req_source(genInfo, ss);

    if (genInfo.handleHalCfm)
    {
        /* 5. hal cfm */
        gen_hal_cfm_source(genInfo, ss);
    }

    /* 6. hal ind */
    if (valid_str(genInfo.aidlCallbackName))
    {
        gen_hal_ind_source(genInfo, ss);
    }

    /* 7. hal req (upstream) */
    gen_hal_up_req_source(genInfo, ss);

    /* 8. hal cfm (downstream) */
    gen_hal_down_cfm_source(genInfo, ss);
    FUNC_LEAVE();
}

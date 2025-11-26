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

static void add_hal_msg_func(const vector<FuncType>& func, stringstream& ss)
{
    for (vector<FuncType>::size_type index = 0; index < func.size(); index++)
    {
        ss << endl;
        apcc_add_func_def(func[index], "", ";", ss);
        ss << endl;
    }
}

static void add_hal_status_header(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (genInfo.halStatusInfo.existHalStatusType)
    {
        apcc_add_sys_include_header(genInfo.halStatusInfo.halStatusHeader, ss);
    }
}

static void add_hal_status_using(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (genInfo.halStatusInfo.existHalStatusType)
    {
        apcc_add_using(genInfo.halStatusInfo.halStatusUsing, ss);
    }
}

static void add_hal_result_using(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (!genInfo.halResultInfo.empty())
    {
        apcc_add_using(HAL_STATUS_TUPLE, ss);
    }
}

/* ------------------------------------------------------------------------------------ */

/* hal req */
static void gen_hal_req_serialize_header(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* 1. comment */
    apcc_add_comment_hal_msg(genInfo.halNameLower, COMMENT_HAL_REQ_SERIALIZE, ss);

    /* 2. func */
    add_hal_msg_func(genInfo.halReqFunc, ss);
    ss << endl;
}

static void gen_hal_req_parse_header(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* 1. comment */
    apcc_add_comment_hal_msg(genInfo.halNameLower, COMMENT_HAL_REQ_PARSE, ss);

    /* 2. func */
    add_hal_msg_func(genInfo.halRpcServerReqFunc, ss);
    ss << endl;
}

static void gen_hal_req_header(const ApccGenInfo& genInfo, stringstream& ss)
{
    gen_hal_req_serialize_header(genInfo, ss);

    gen_hal_req_parse_header(genInfo, ss);
}

/* ------------------------------------------------------------------------------------ */

/* hal cfm */
static void gen_hal_cfm_parse_header(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* 1. comment */
    apcc_add_comment_hal_msg(genInfo.halNameLower, COMMENT_HAL_CFM_PARSE, ss);

    /* 2. function */
    add_hal_msg_func(genInfo.halCfmFunc, ss);
    ss << endl;
}

static void gen_hal_cfm_serialize_header(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* 1. comment */
    apcc_add_comment_hal_msg(genInfo.halNameLower, COMMENT_HAL_CFM_SERIALIZE, ss);

    /* 2. function */
    add_hal_msg_func(genInfo.halRpcServerCfmFunc, ss);
    ss << endl;
}

static void gen_hal_cfm_header(const ApccGenInfo& genInfo, stringstream& ss)
{
    gen_hal_cfm_parse_header(genInfo, ss);

    gen_hal_cfm_serialize_header(genInfo, ss);
}

/* ------------------------------------------------------------------------------------ */

/* hal ind */
static void gen_hal_ind_parse_header(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (valid_str(genInfo.aidlCallbackName))
    {
        /* 1. comment */
        apcc_add_comment_hal_msg(genInfo.halNameLower, COMMENT_HAL_IND_PARSE, ss);

        /* 2. function */
        add_hal_msg_func(genInfo.halIndFunc, ss);
        ss << endl;
    }
}

static void gen_hal_ind_serialize_header(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (valid_str(genInfo.aidlCallbackName))
    {
        /* 1. comment */
        apcc_add_comment_hal_msg(genInfo.halNameLower, COMMENT_HAL_IND_SERIALIZE, ss);

        /* 2. function */
        add_hal_msg_func(genInfo.halRpcServerIndFunc, ss);
    }
}

static void gen_hal_ind_header(const ApccGenInfo& genInfo, stringstream& ss)
{
    gen_hal_ind_parse_header(genInfo, ss);

    gen_hal_ind_serialize_header(genInfo, ss);
}

/* ------------------------------------------------------------------------------------ */

/* hal req (upstream) */
static void hru_add_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    add_hal_msg_func(genInfo.halUpReqFunc, ss);
}

static void gen_hal_up_req_header(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (valid_str(genInfo.aidlCallbackResultName))
    {
        ss << endl;
        /* 1. comment */
        apcc_add_comment_hal_msg(genInfo.halNameLower, COMMENT_HAL_UP_REQ, ss);

        /* 2. func */
        hru_add_func(genInfo, ss);
        ss << endl;
    }
}

/* ------------------------------------------------------------------------------------ */

/* hal cfm (downstream) */
static void hcd_add_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    add_hal_msg_func(genInfo.halDownCfmFunc, ss);
}

static void gen_hal_down_cfm_header(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (valid_str(genInfo.aidlCallbackResultName))
    {
        /* 1. comment */
        apcc_add_comment_hal_msg(genInfo.halNameLower, COMMENT_HAL_DOWN_CFM, ss);

        /* 2. function */
        hcd_add_func(genInfo, ss);
    }
}

/* ------------------------------------------------------------------------------------ */

static void hmh_add_using(const ApccGenInfo& genInfo, stringstream& ss)
{
    vector<string> usingType;

    apcc_add_intf_internal_using(genInfo, ss);
    apcc_add_callback_internal_using(genInfo, ss);

    apcc_get_all_using(genInfo, usingType);
    // dump_string("hal all using", usingType);
    // logd("%s: halStatusUsing: %s", __func__, genInfo.halStatusInfo.halStatusUsing.c_str());
    if (!contain_string(usingType, genInfo.halStatusInfo.halStatusUsing))
    {
        add_hal_status_using(genInfo, ss);
    }
    apcc_add_using(usingType, ss);

    add_hal_result_using(genInfo, ss);
}

static void hmh_add_include(const ApccGenInfo& genInfo, stringstream& ss)
{
    vector<string> includeHeader;

    apcc_get_all_include(genInfo, includeHeader);
    // dump_string("hal all header", includeHeader);
    // logd("%s: halStatusHeader: %s", __func__, genInfo.halStatusInfo.halStatusHeader.c_str());
    if (!contain_string(includeHeader, genInfo.halStatusInfo.halStatusHeader))
    {
        add_hal_status_header(genInfo, ss);
    }
    apcc_add_sys_include_header(includeHeader, ss);
    ss << endl;

    apcc_add_include_header(HAL_UTIL_HEADER, ss);
}

/* ------------------------------------------------------------------------------------ */

void gen_hal_msg_header(stringstream& ss)
{
    const ApccGenInfo& genInfo = apcc_get_gen_info();

    FUNC_ENTER();
    /* 1. comment */
    apcc_add_comment(ss);

    /* 2. pragma */
    apcc_add_pragma(ss);

    /* 3. include */
    hmh_add_include(genInfo, ss);

    /* 4. using */
    hmh_add_using(genInfo, ss);

    /* 5. hal req header */
    gen_hal_req_header(genInfo, ss);

    if (genInfo.handleHalCfm)
    {
        /* 6. hal cfm header */
        gen_hal_cfm_header(genInfo, ss);
    }

    /* 7. hal ind header */
    gen_hal_ind_header(genInfo, ss);

    /* 8. hal req (upstream) header */
    gen_hal_up_req_header(genInfo, ss);

    /* 9. hal cfm (downstream) header */
    gen_hal_down_cfm_header(genInfo, ss);
    FUNC_LEAVE();
}

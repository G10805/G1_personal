/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <map>
#include <vector>

#include "aidl.h"
#include "aidl_util.h"

#include "apcc_gen.h"

#include "proto.h"
#include "proto_util.h"

#include "log.h"
#include "util.h"

using std::map;
using std::vector;

#define MDH_DEF_ALIGN_WIDTH             36

#define MDH_DEF_ALIGN_PAD_WIDTH         4

#define COMMENT_PRIMITIVE_DEFINITIONS   "\
/*******************************************************************************\n\
 * Primitive definitions\n\
 *******************************************************************************/\n\
"

#define COMMENT_HAL_REQUEST                 "/* Hal request (downstream primitive) */"
#define COMMENT_HAL_INDICATOR               "/* Hal indicator (upstream primitive) */"
#define COMMENT_HAL_CONFIRMATION            "/* Hal confirmation (upstream primitive) */"
#define COMMENT_HAL_UPSTREAM_REQUEST        "/* Hal request (upstream primitive) */"
#define COMMENT_HAL_DOWNSTREAM_CONFIRMATION "/* Hal confirmation (downstream primitive) */"

static uint32_t sMdhAlignWidth = MDH_DEF_ALIGN_WIDTH;

static void mdh_define_hal_msg_base(const string& halNameUpper, const string& val, stringstream& ss)
{
    string str = val;
    replace_string(str, "HAL", halNameUpper);
    apcc_add_define(str, val, sMdhAlignWidth, ss);
    ss << endl;
}

static string mdh_get_hal_msg_count_val(const vector<string>& msgDef)
{
    stringstream ss;
    vector<string>::size_type size = msgDef.size();

    if (size == 1)
    {
        ss << "(1)";
    }
    else if (size > 1)
    {
        const string first_msg = msgDef[0];
        const string last_msg = msgDef[size-1];
        ss << '(' << last_msg << " - " << first_msg << " + 1" << ')';
    }
    return ss.str();
}

static void mdh_add_hal_msg_count(const string& key, const vector<string>& msgDef, stringstream& ss)
{
    ss << endl;
    apcc_add_define(key, mdh_get_hal_msg_count_val(msgDef), sMdhAlignWidth, ss);
    ss << endl;
}

static string mdh_get_is_hal_msg_key(const string& key, const string& type_name)
{
    stringstream ss;
    ss << key << '(' << type_name << ')';
    return ss.str();
}

static string mdh_get_is_hal_msg_val(const string& type_name, const vector<string>& msgDef)
{
    stringstream ss;
    vector<string>::size_type size = msgDef.size();
    const string first_msg = msgDef[0];

    if (size == 1)
    {
        ss << "((" << type_name << ')' << " == " << first_msg << ')';
    }
    else if (size > 1)
    {
        const string last_msg = msgDef[size-1];
        ss << "(((" << type_name << ')' << " >= " << first_msg << ')';
        ss << " && ((" << type_name << ')' << " <= " << last_msg << "))";
    }

    return ss.str();
}


static void mdh_add_is_hal_msg(const string& key, const vector<string>& msgDef, stringstream& ss)
{
    const string& type_name = "t";
    apcc_add_define(mdh_get_is_hal_msg_key(key, type_name),
        mdh_get_is_hal_msg_val(type_name, msgDef), sMdhAlignWidth, ss);
    ss << endl;
}

static string mdh_get_hal_msg_val(const string& msg_type_name, const string& val, uint16_t index)
{
    stringstream ss;
    ss << "((" << msg_type_name << ") (" << val << " + " << get_hex_string(index) << "))";
    return ss.str();
}

static void mdh_add_hal_msg(const string& msg_type_name, const string& msgDef, const string& val, uint16_t index, stringstream& ss)
{
    /* e.g. #define HAL_SAMPLE_REQ            ((HalMsg) (HAL_REQ_BASE + 0x0001)) */
    apcc_add_define(msgDef, mdh_get_hal_msg_val(msg_type_name, val, index), sMdhAlignWidth, ss);
}

static void mdh_add_comment(const char *comment, const string& halName, stringstream& ss)
{
    string str = comment;
    replace_string(str, "Hal", halName);
    ss << str << endl;
}

static void mdh_add_define(const ApccGenInfo& genInfo, const string& msg_type_name, stringstream& ss)
{
    const vector<string>& req = genInfo.halReqDef;
    const vector<string>& ind = genInfo.halIndDef;
    const vector<string>& cfm = genInfo.halCfmDef;
    const vector<string>& upReq = genInfo.halUpReqDef;
    const vector<string>& downCfm = genInfo.halDownCfmDef;
    vector<string>::size_type index;
    string first_msg, last_msg;

    /* comment: Primitive definitions */
    ss << COMMENT_PRIMITIVE_DEFINITIONS << endl;

    if (req.empty())
        return;

    /* comment: Hal request */
    mdh_add_comment(COMMENT_HAL_REQUEST, genInfo.halName, ss);
    mdh_define_hal_msg_base(genInfo.halNameUpper, HAL_REQ_BASE_NAME, ss);
    for (index = 0; index < req.size(); index++)
    {
        mdh_add_hal_msg(msg_type_name, req[index], genInfo.halMsgInfo.reqBase, index, ss);
    }
    mdh_add_hal_msg_count(genInfo.halMsgInfo.reqCount, req, ss);

    mdh_add_is_hal_msg(genInfo.halMsgInfo.isHalReq, req, ss);

    if (genInfo.isCfmDefined && !cfm.empty())
    {
        /* comment: Hal confirmation */
        mdh_add_comment(COMMENT_HAL_CONFIRMATION, genInfo.halName, ss);
        mdh_define_hal_msg_base(genInfo.halNameUpper, HAL_CFM_BASE_NAME, ss);
        for (index = 0; index < cfm.size(); index++)
        {
            mdh_add_hal_msg(msg_type_name, cfm[index], genInfo.halMsgInfo.cfmBase, index, ss);
        }
        mdh_add_hal_msg_count(genInfo.halMsgInfo.cfmCount, cfm, ss);

        mdh_add_is_hal_msg(genInfo.halMsgInfo.isHalCfm, cfm, ss);
    }

    if (!ind.empty())
    {
        /* comment: Hal indicator */
        mdh_add_comment(COMMENT_HAL_INDICATOR, genInfo.halName, ss);
        mdh_define_hal_msg_base(genInfo.halNameUpper, HAL_IND_BASE_NAME, ss);
        for (index = 0; index < ind.size(); index++)
        {
            mdh_add_hal_msg(msg_type_name, ind[index], genInfo.halMsgInfo.indBase, index, ss);
        }
        mdh_add_hal_msg_count(genInfo.halMsgInfo.indCount, ind, ss);

        mdh_add_is_hal_msg(genInfo.halMsgInfo.isHalInd, ind, ss);
    }

    if (!upReq.empty())
    {
        /* comment: Hal request (upstream) */
        mdh_add_comment(COMMENT_HAL_UPSTREAM_REQUEST, genInfo.halName, ss);
        mdh_define_hal_msg_base(genInfo.halNameUpper, HAL_UPSTREAM_REQ_BASE_NAME, ss);
        for (index = 0; index < upReq.size(); index++)
        {
            mdh_add_hal_msg(msg_type_name, upReq[index], genInfo.halMsgInfo.upReqBase, index, ss);
        }
        mdh_add_hal_msg_count(genInfo.halMsgInfo.upReqCount, upReq, ss);

        mdh_add_is_hal_msg(genInfo.halMsgInfo.isHalUpReq, upReq, ss);
    }

    if (!downCfm.empty())
    {
        /* comment: Hal request (upstream) */
        mdh_add_comment(COMMENT_HAL_DOWNSTREAM_CONFIRMATION, genInfo.halName, ss);
        mdh_define_hal_msg_base(genInfo.halNameUpper, HAL_DOWNSTREAM_CFM_BASE_NAME, ss);
        for (index = 0; index < downCfm.size(); index++)
        {
            mdh_add_hal_msg(msg_type_name, downCfm[index], genInfo.halMsgInfo.downCfmBase, index, ss);
        }
        mdh_add_hal_msg_count(genInfo.halMsgInfo.downCfmCount, downCfm, ss);

        mdh_add_is_hal_msg(genInfo.halMsgInfo.isHalDownCfm, downCfm, ss);
    }
}

static void mdh_init_gen(const ApccGenInfo& genInfo)
{
    uint32_t maxLen = 0;

    get_max_str_len(genInfo.halReqDef, maxLen);
    get_max_str_len(genInfo.halIndDef, maxLen);
    get_max_str_len(genInfo.halCfmDef, maxLen);

    sMdhAlignWidth = maxLen + MDH_DEF_ALIGN_PAD_WIDTH;

    // logd("%s: sMdhAlignWidth: %d, maxLen: %d", __func__, sMdhAlignWidth, maxLen);
}

/* ------------------------------------------------------------------------------------ */

void gen_hal_message_def_header(stringstream& ss)
{
    const ApccGenInfo& genInfo = apcc_get_gen_info();

    mdh_init_gen(genInfo);

    FUNC_ENTER();
    /* 1. comment */
    apcc_add_comment(ss);

    /* 2. pragma */
    apcc_add_pragma(ss);

    /* 3. include */
    ss << "#include" << " " << "\"" << HAL_MESSAGE_DEF_HEADER << "\"" << endl;
    ss << endl;

    /* 4. typedef uint16_t HalMsg; */
    ss << "typedef uint16_t " << genInfo.halMsgInfo.typeName << ";" << endl;
    ss << endl;

    /* 5. define */
    mdh_add_define(genInfo, genInfo.halMsgInfo.typeName, ss);

    FUNC_LEAVE();
}

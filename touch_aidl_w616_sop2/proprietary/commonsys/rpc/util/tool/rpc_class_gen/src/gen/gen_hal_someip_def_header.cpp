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

#define SDH_DEF_ALIGN_WIDTH             36

#define SDH_DEF_ALIGN_PAD_WIDTH         4

#define COMMENT_HAL_COMMON_DEFINITION   "\
/*******************************************************************************\n\
 * Hal common definition\n\
 *******************************************************************************/\n\
"

#define COMMENT_HAL_CLIENT_DEFINITION   "\
/*******************************************************************************\n\
 * Hal client definition\n\
 *******************************************************************************/\n\
"

#define COMMENT_HAL_SERVER_DEFINITION   "\
/*******************************************************************************\n\
 * Hal server definition\n\
 *******************************************************************************/\n\
"

typedef struct
{
    pair<string, string> halEventGroupIdBase;
    pair<string, string> halEventGroupId;
    pair<string, string> halEventIdNumber;
    pair<string, string> halClientAppName;
    pair<string, string> halServerAppName;
    uint32_t macroAlignWidth;
    bool existEvent;
} HalSomeipDefInstance;

static HalSomeipDefInstance sHalSomeipDefInstance;

static string get_hal_event_group_id_val(const string& halEventGroupIdBase, uint16_t index = 0)
{
    stringstream ss;
    ss << "((uint16_t) (" << halEventGroupIdBase << " + " << get_hex_string(index) << "))";
    return ss.str();
}

static void sdh_add_hal_def_comment(const string& halName, const string& comment, stringstream& ss)
{
    string str = comment;
    replace_string(str, "Hal", halName);
    ss << str << endl;
}

static void sdh_add_define(const string& key, const string& value, stringstream& ss)
{
    HalSomeipDefInstance *inst = &sHalSomeipDefInstance;
    apcc_add_define(key, value, inst->macroAlignWidth, ss);
}

static void sdh_add_define(const ApccGenInfo& genInfo, stringstream& ss)
{
    HalSomeipDefInstance *inst = &sHalSomeipDefInstance;

    if (inst->existEvent)
    {
        sdh_add_define(inst->halEventGroupIdBase.first, inst->halEventGroupIdBase.second, ss);
        ss << endl;

        sdh_add_hal_def_comment(genInfo.halName, COMMENT_HAL_COMMON_DEFINITION, ss);
        sdh_add_define(inst->halEventGroupId.first, inst->halEventGroupId.second, ss);
        ss << endl;
        sdh_add_define(inst->halEventIdNumber.first, inst->halEventIdNumber.second, ss);
        ss << endl;
    }

    sdh_add_hal_def_comment(genInfo.halName, COMMENT_HAL_CLIENT_DEFINITION, ss);
    sdh_add_define(inst->halClientAppName.first, inst->halClientAppName.second, ss);
    ss << endl;

    sdh_add_hal_def_comment(genInfo.halName, COMMENT_HAL_SERVER_DEFINITION, ss);
    sdh_add_define(inst->halServerAppName.first, inst->halServerAppName.second, ss);
}

static void sdh_add_include(const ApccGenInfo& genInfo, stringstream& ss)
{
    apcc_add_include_header(apcc_get_gen_file_name(genInfo.aidlHalRoot.intfName, APCC_TYPE_HAL_MESSAGE_DEF_HEADER), ss);
    ss << endl;

    apcc_add_include_header(SOMEIP_COMMON_DEF_HEADER, ss);
    ss << endl;
}

static void sdh_init_macro_align_width(HalSomeipDefInstance *inst)
{
    uint32_t maxLen = 0;
    vector<string> str;

    str.push_back(inst->halEventGroupIdBase.first);
    str.push_back(inst->halEventGroupId.first);
    str.push_back(inst->halEventIdNumber.first);
    str.push_back(inst->halClientAppName.first);
    str.push_back(inst->halServerAppName.first);

    get_max_str_len(str, maxLen);

    inst->macroAlignWidth = maxLen + SDH_DEF_ALIGN_PAD_WIDTH;
    // logd("%s: macroAlignWidth: %d, maxLen: %d", __func__, inst->macroAlignWidth, maxLen);
}

static void sdh_init_gen(const ApccGenInfo& genInfo)
{
    HalSomeipDefInstance *inst = &sHalSomeipDefInstance;
    const string& halNameUpper = genInfo.halNameUpper;
    string halEventGroupIdBase = concat_string(halNameUpper, HAL_EVENTGROUP_ID_BASE_SUFFIX);
    string halEventGroupIdBaseVal = HAL_EVENTGROUP_ID_BASE_VAL;
    string halEventGroupId = concat_string(halNameUpper, HAL_EVENTGROUP_ID_SUFFIX);
    string halEventGroupIdVal = get_hal_event_group_id_val(halEventGroupIdBase);
    string halEventIdNumber = concat_string(halNameUpper, HAL_EVENT_ID_NUMBER_SUFFIX);
    string halEventIdNumberVal = concat_string(halNameUpper, HAL_IND_COUNT_SUFFIX);
    string halClientAppName = concat_string(halNameUpper, HAL_CLIENT_APP_NAME_SUFFIX);
    string halClientAppNameVal = get_string('"', genInfo.halNameLower, HAL_CLIENT_APP_NAME_VAL_SUFFIX);
    string halServerAppName = concat_string(halNameUpper, HAL_SERVER_APP_NAME_SUFFIX);
    string halServerAppNameVal = get_string('"', genInfo.halNameLower, HAL_SERVER_APP_NAME_VAL_SUFFIX);

    inst->halEventGroupIdBase = pair<string, string>(halEventGroupIdBase, halEventGroupIdBaseVal);
    inst->halEventGroupId     = pair<string, string>(halEventGroupId,     halEventGroupIdVal);
    inst->halEventIdNumber    = pair<string, string>(halEventIdNumber,    halEventIdNumberVal);
    inst->halClientAppName    = pair<string, string>(halClientAppName,    halClientAppNameVal);
    inst->halServerAppName    = pair<string, string>(halServerAppName,    halServerAppNameVal);

    sdh_init_macro_align_width(inst);
    inst->existEvent = valid_str(genInfo.aidlCallbackName);
    // logd("%s: existEvent %x", __func__, inst->existEvent);
}

/* ------------------------------------------------------------------------------------ */

void gen_hal_someip_def_header(stringstream& ss)
{
    const ApccGenInfo& genInfo = apcc_get_gen_info();

    FUNC_ENTER();
    sdh_init_gen(genInfo);

    /* 1. comment */
    apcc_add_comment(ss);

    /* 2. pragma */
    apcc_add_pragma(ss);

    /* 3. include */
    sdh_add_include(genInfo, ss);

    /* 4. define */
    sdh_add_define(genInfo, ss);
    FUNC_LEAVE();
}

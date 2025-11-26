/* ===========================================================================
 * Copyright (c) 2019-2020, 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#include "ais_hidl_client.h"
#include <hidl/LegacySupport.h>

#if (defined (__GNUC__) && !defined(__INTEGRITY))
#define PUBLIC_API __attribute__ ((visibility ("default")))
#else
#define PUBLIC_API
#endif

// Application specific global variables
android::sp<vendor::qti::automotive::qcarcam::V1_1::IQcarCamera> pService;
qcarcam_input_t *pInputLists;
qcarcam_input_v2_t *pInputListsv2;
uint32_t queryNumInputs;

using namespace android;
using android::hardware::configureRpcThreadpool;

#define QCARCAM_MAX_STREAMS 8

PUBLIC_API qcarcam_ret_t qcarcam_initialize(qcarcam_init_t* p_init_params)
{
    char* p_debug_tag = NULL;
    if (p_init_params)
        p_debug_tag = (char*)p_init_params->debug_tag;
    ais_log_init(NULL, p_debug_tag);
    AIS_HIDL_DBGMSG("Acquiring ais-hidl Enumerator");
    pService = IQcarCamera::getService();
    if (pService.get() == nullptr) {
        AIS_HIDL_ERRORMSG("getService returned NULL");
        return QCARCAM_RET_FAILED;
    }
    configureRpcThreadpool(QCARCAM_MAX_STREAMS, false /* callerWillJoin */);
    return QCARCAM_RET_OK;
}

PUBLIC_API qcarcam_ret_t qcarcam_uninitialize(void)
{
    pService = nullptr;
    return QCARCAM_RET_OK;
}

PUBLIC_API qcarcam_ret_t qcarcam_query_inputs(qcarcam_input_t* p_inputs, unsigned int size, unsigned int* ret_size)
{
    uint32_t i = 0;
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    vendor::qti::automotive::qcarcam::V1_0::QcarcamError Error;

    if (NULL == pInputLists) {
        auto result = pService->getInputStreamList([&](const auto& tmpError, hidl_vec<vendor::qti::automotive::qcarcam::V1_0::QcarcamInputInfo> InputLists) {
                Error = tmpError;
                if (vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_OK == Error) {
                AIS_HIDL_DBGMSG("Found %d input streams",InputLists.size());
                queryNumInputs = InputLists.size();
                pInputLists = (qcarcam_input_t *)calloc(queryNumInputs, sizeof(*pInputLists));
                if (pInputLists) {
                for (auto&& cam: InputLists) {
                AIS_HIDL_DBGMSG("Found camera %d", static_cast<uint32_t>(cam.desc));
                // Copy the result into qcarcam structure from hidl cb
                pInputLists[i].desc = (qcarcam_input_desc_t)cam.desc;
                strlcpy(pInputLists[i].name, InputLists[i].name.c_str(), sizeof(pInputLists[i].name));
                strlcpy(pInputLists[i].parent_name, InputLists[i].parent_name.c_str(), sizeof(pInputLists[i].parent_name));
                pInputLists[i].num_res = InputLists[i].num_res;
                for (int j = 0; j<InputLists[i].num_res; j++) {
                pInputLists[i].res[j].width = InputLists[i].res[j].width;
                pInputLists[i].res[j].height = InputLists[i].res[j].height;
                pInputLists[i].res[j].fps = InputLists[i].res[j].fps;
                }
                pInputLists[i].num_color_fmt = InputLists[i].num_color_fmt;
                for (int j = 0; j<InputLists[i].num_color_fmt; j++)
                    pInputLists[i].color_fmt[j] = (qcarcam_color_fmt_t)InputLists[i].color_fmt[j];
                pInputLists[i].flags = InputLists[i].flags;
                i++;
                }
                }
                } else {
                    AIS_HIDL_ERRORMSG("getInputStreamList Failed with Error %d",static_cast<int32_t>(Error));
                    ret = QCARCAM_RET_FAILED;
                }
        } );
        if (!result.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport error; getInputStreamList() failed");
            ret = QCARCAM_RET_FAILED;
        }
    }
    *ret_size = queryNumInputs;

    if (NULL != p_inputs && NULL != pInputLists)
        memcpy(p_inputs, pInputLists, size*sizeof(qcarcam_input_t));

    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_query_inputs_v2(qcarcam_input_v2_t* p_inputs, unsigned int size, unsigned int* ret_size)
{
    uint32_t i = 0;
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    vendor::qti::automotive::qcarcam::V1_0::QcarcamError Error;

    if (NULL == pInputListsv2) {
        pService->getInputStreamList_1_1([&](const auto& tmpError, hidl_vec<vendor::qti::automotive::qcarcam::V1_1::QcarcamInputInfov2> InputLists) {
                Error = tmpError;
                if (vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_OK == Error) {
                AIS_HIDL_DBGMSG("Found %d input streams",InputLists.size());
                queryNumInputs = InputLists.size();
                pInputListsv2 = (qcarcam_input_v2_t *)calloc(queryNumInputs, sizeof(*pInputListsv2));
                if (pInputListsv2) {
                for (auto&& cam: InputLists) {
                AIS_HIDL_DBGMSG("Found camera %d", static_cast<uint32_t>(cam.desc));
                // Copy the result into qcarcam structure from hidl cb
                pInputListsv2[i].desc = (qcarcam_input_desc_t)cam.desc;
                strlcpy(pInputListsv2[i].name, InputLists[i].name.c_str(), sizeof(pInputLists[i].name));
                //strlcpy(pInputListsv2[i].parent_name, InputLists[i].parent_name.c_str(), sizeof(pInputLists[i].parent_name));
                pInputListsv2[i].num_modes = InputLists[i].num_modes;
               for (int j = 0; j<InputLists[i].num_modes; j++) {
                    pInputListsv2[i].modes[j].res.width = InputLists[i].modes[j].res.width;
                    pInputListsv2[i].modes[j].res.height = InputLists[i].modes[j].res.height;
                    pInputListsv2[i].modes[j].res.fps = InputLists[i].modes[j].res.fps;
                    pInputListsv2[i].modes[j].fmt = (qcarcam_color_fmt_t)InputLists[i].modes[j].color_fmt;
                }
                 pInputListsv2[i].flags = InputLists[i].flags;
                i++;
                }
                }
                } else {
                    AIS_HIDL_ERRORMSG("getInputStreamList Failed with Error %d",static_cast<int32_t>(Error));
                    ret = QCARCAM_RET_FAILED;
                }
        } );
    }
    *ret_size = queryNumInputs;

    if (NULL != p_inputs && NULL != pInputLists)
        memcpy(p_inputs, pInputLists, size*sizeof(qcarcam_input_t));

    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_query_diagnostics(void* p_diag_info, unsigned int diag_size)
{
    (void)p_diag_info;
    (void)diag_size;

    return QCARCAM_RET_UNSUPPORTED;
}

PUBLIC_API qcarcam_hndl_t qcarcam_open(qcarcam_input_desc_t desc)
{
    // Create and return stream obj
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    ais_hidl_client *clientObj = new ais_hidl_client(pService);
    ret = clientObj->ais_client_open_stream(desc);
    if (QCARCAM_RET_OK != ret) {
        AIS_HIDL_ERRORMSG("ais_client_open_stream failed with ret = %d", ret);
        delete(clientObj);
        clientObj = NULL;
    } else {
        clientObj->mInitialized = true;
    }
    return (static_cast<qcarcam_hndl_t>(clientObj));
}

PUBLIC_API qcarcam_ret_t qcarcam_close(qcarcam_hndl_t hndl)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    ais_hidl_client *clientObj;
    clientObj = static_cast<ais_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized) {
        ret = clientObj->ais_client_close_stream();
        // Delete stream obj
        clientObj->mInitialized = false;
        // TODO::Remove this and solve crash
        //delete(clientObj);
        //clientObj = nullptr;
    } else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_g_param(qcarcam_hndl_t hndl, qcarcam_param_t param, qcarcam_param_value_t* p_value)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    ais_hidl_client *clientObj;
    clientObj = static_cast<ais_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->ais_client_g_param(param, p_value);
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_s_param(qcarcam_hndl_t hndl, qcarcam_param_t param, const qcarcam_param_value_t* p_value)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    ais_hidl_client *clientObj;
    clientObj = static_cast<ais_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized) {
        if (QCARCAM_PARAM_EVENT_CB == param)
            ret = clientObj->ais_client_s_cb(p_value->ptr_value);
        else
            ret = clientObj->ais_client_s_param(param, p_value);
    }
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_s_buffers(qcarcam_hndl_t hndl, qcarcam_buffers_t* p_buffers)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    ais_hidl_client *clientObj;
    clientObj = static_cast<ais_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->ais_client_s_buffer(p_buffers);
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_s_buffers_v2(qcarcam_hndl_t hndl, qcarcam_bufferlist_t* p_buffers)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    ais_hidl_client *clientObj;
    clientObj = static_cast<ais_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->ais_client_s_buffer_v2(p_buffers);
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_start(qcarcam_hndl_t hndl)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    ais_hidl_client *clientObj;
    clientObj = static_cast<ais_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->ais_client_start_stream(hndl);
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_stop(qcarcam_hndl_t hndl)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    ais_hidl_client *clientObj;
    clientObj = static_cast<ais_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->ais_client_stop_stream();
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_pause(qcarcam_hndl_t hndl)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    ais_hidl_client *clientObj;
    clientObj = static_cast<ais_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->ais_client_pause_stream();
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_resume(qcarcam_hndl_t hndl)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    ais_hidl_client *clientObj;
    clientObj = static_cast<ais_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->ais_client_resume_stream();
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_get_frame(qcarcam_hndl_t hndl, qcarcam_frame_info_t* p_frame_info,
        unsigned long long int timeout, unsigned int flags)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    ais_hidl_client *clientObj;
    clientObj = static_cast<ais_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->ais_client_get_frame(p_frame_info, timeout, flags);
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_get_frame_v2(qcarcam_hndl_t hndl, qcarcam_frame_info_v2_t* p_frame_info,
        unsigned long long int timeout, unsigned int flags)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    ais_hidl_client *clientObj;
    clientObj = static_cast<ais_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->ais_client_get_frame_v2(p_frame_info, timeout, flags);
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_release_frame(qcarcam_hndl_t hndl, unsigned int idx)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    ais_hidl_client *clientObj;
    clientObj = static_cast<ais_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->ais_client_release_frame(idx);
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_release_frame_v2(qcarcam_hndl_t hndl,unsigned int id, unsigned int idx)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    ais_hidl_client *clientObj;
    clientObj = static_cast<ais_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->ais_client_release_frame_v2(id,idx);
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}


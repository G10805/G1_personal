/* ===========================================================================
 * Copyright (c) 2019-2020, 2022, 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <binder/ProcessState.h>
#include "ais_aidl_client.h"
#include <map>


#if (defined (__GNUC__) && !defined(__INTEGRITY))
#define PUBLIC_API __attribute__ ((visibility ("default")))
#else
#define PUBLIC_API
#endif

// Application specific global variables
std::shared_ptr<aidl::vendor::qti::automotive::qcarcam::IQcarCamera> pService;
qcarcam_input_v2_t *pInputListsv2;
uint32_t queryNumInputs;

static std::map<qcarcam_hndl_t, std::shared_ptr<ais_aidl_client>> gMap;


using namespace android;

#define QCARCAM_MAX_STREAMS 8

PUBLIC_API qcarcam_ret_t qcarcam_initialize(qcarcam_init_t* p_init_params)
{
    char* p_debug_tag = NULL;
    if (p_init_params)
        p_debug_tag = (char*)p_init_params->debug_tag;
    ais_log_init(NULL, p_debug_tag);
    AIS_AIDL_ERRORMSG("Acquiring ais-aidl Enumerator");
    const std::string serviceName = std::string() + IQcarCamera::descriptor + "/default";
    pService = IQcarCamera::fromBinder(ndk::SpAIBinder(AServiceManager_checkService(serviceName.c_str())));
    if (!pService) {
        AIS_AIDL_ERRORMSG("getService returned NULL");
        return QCARCAM_RET_FAILED;
    }
    bool success = ABinderProcess_setThreadPoolMaxThreadCount(QCARCAM_MAX_STREAMS);
    if (success == false)
    {
        AIS_AIDL_ERRORMSG("ABinderProcess_setThreadPoolMaxThreadCount returned false");
    }
    ABinderProcess_startThreadPool();

    return QCARCAM_RET_OK;
}

PUBLIC_API qcarcam_ret_t qcarcam_uninitialize(void)
{
    pService = nullptr;
    return QCARCAM_RET_OK;
}

PUBLIC_API qcarcam_ret_t qcarcam_query_inputs_v2(qcarcam_input_v2_t* p_inputs, unsigned int size, unsigned int* ret_size)
{
    uint32_t i = 0;
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    QcarcamError Error;

    if (NULL == pInputListsv2) {
        std::vector<QcarcamInputInfov2> InputLists;
        auto retVal = pService->getInputStreamList(&InputLists, &Error);
        if (!retVal.isOk()) {
            AIS_AIDL_ERRORMSG("aidl transport layer error, queryInputs() failed");
        } else {
            if (QcarcamError::QCARCAM_OK == Error) {
                AIS_AIDL_DBGMSG("Found %d input streams",InputLists.size());
                queryNumInputs = InputLists.size();
                pInputListsv2 = (qcarcam_input_v2_t *)calloc(queryNumInputs, sizeof(*pInputListsv2));
                if (pInputListsv2) {
                    for (auto&& cam: InputLists) {
                        AIS_AIDL_DBGMSG("Found camera %d", static_cast<uint32_t>(cam.desc));
                        // Copy the result into qcarcam structure from hidl cb
                        pInputListsv2[i].desc = (qcarcam_input_desc_t)cam.desc;
                        strlcpy(pInputListsv2[i].name, InputLists[i].name.c_str(), sizeof(pInputListsv2[i].name));
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
                AIS_AIDL_ERRORMSG("getInputStreamList Failed with Error %d",static_cast<int32_t>(Error));
                ret = QCARCAM_RET_FAILED;
            }
        }
    }
    *ret_size = queryNumInputs;

    if (NULL != p_inputs && NULL != pInputListsv2)
        memcpy(p_inputs, pInputListsv2, size*sizeof(qcarcam_input_t));

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
    std::shared_ptr<ais_aidl_client> clientObj = ndk::SharedRefBase::make<ais_aidl_client>(pService);
    ret = clientObj->ais_client_open_stream(desc);
    if (QCARCAM_RET_OK != ret) {
        AIS_AIDL_ERRORMSG("ais_client_open_stream failed with ret = %d", ret);
        clientObj.reset();
        return nullptr;
    } else {
        clientObj->mInitialized = true;
    }
    qcarcam_hndl_t pHndl = (qcarcam_hndl_t)clientObj.get();
    gMap[pHndl] = clientObj;
    AIS_AIDL_DBGMSG("ais_client_open_stream pHndl = %#x", pHndl);

    return (static_cast<qcarcam_hndl_t>(clientObj.get()));
}

PUBLIC_API qcarcam_ret_t qcarcam_close(qcarcam_hndl_t hndl)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    ais_aidl_client *clientObj;
    if (gMap[hndl] && gMap[hndl]->mInitialized) {
        ret = gMap[hndl]->ais_client_close_stream();
        gMap.erase(hndl);
    } else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_g_param(qcarcam_hndl_t hndl, qcarcam_param_t param, qcarcam_param_value_t* p_value)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    if (gMap[hndl] && gMap[hndl]->mInitialized)
        ret = gMap[hndl]->ais_client_g_param(param, p_value);
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_s_param(qcarcam_hndl_t hndl, qcarcam_param_t param, const qcarcam_param_value_t* p_value)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    if (gMap[hndl] && gMap[hndl]->mInitialized) {
        if (QCARCAM_PARAM_EVENT_CB == param)
            ret = gMap[hndl]->ais_client_s_cb(p_value->ptr_value);
        else
            ret = gMap[hndl]->ais_client_s_param(param, p_value);
    }
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_s_buffers_v2(qcarcam_hndl_t hndl, qcarcam_bufferlist_t* p_buffers)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    if (gMap[hndl] && gMap[hndl]->mInitialized)
        ret = gMap[hndl]->ais_client_s_buffer_v2(p_buffers);
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_start(qcarcam_hndl_t hndl)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    if (gMap[hndl] && gMap[hndl]->mInitialized)
        ret = gMap[hndl]->ais_client_start_stream(hndl);
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_stop(qcarcam_hndl_t hndl)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    if (gMap[hndl] && gMap[hndl]->mInitialized)
        ret = gMap[hndl]->ais_client_stop_stream();
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_pause(qcarcam_hndl_t hndl)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    if (gMap[hndl] && gMap[hndl]->mInitialized)
        ret = gMap[hndl]->ais_client_pause_stream();
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_resume(qcarcam_hndl_t hndl)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    if (gMap[hndl] && gMap[hndl]->mInitialized)
        ret = gMap[hndl]->ais_client_resume_stream();
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_get_frame_v2(qcarcam_hndl_t hndl, qcarcam_frame_info_v2_t* p_frame_info,
        unsigned long long int timeout, unsigned int flags)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    if (gMap[hndl] && gMap[hndl]->mInitialized)
        ret = gMap[hndl]->ais_client_get_frame_v2(p_frame_info, timeout, flags);
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API qcarcam_ret_t qcarcam_release_frame_v2(qcarcam_hndl_t hndl,unsigned int id, unsigned int idx)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    if (gMap[hndl] && gMap[hndl]->mInitialized)
        ret = gMap[hndl]->ais_client_release_frame_v2(id,idx);
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

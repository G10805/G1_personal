/*
 * Copyright (c) 2016-2021, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#define LOG_TAG "generic_effect_ar"
#define LOG_NDDEBUG 0
/*#define LOG_NDEBUG 0*/

#include <qti_gef_api.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <cutils/log.h>
#include <cutils/trace.h>
#include <cutils/str_parms.h>
#include <cutils/properties.h>
#include <cutils/list.h>

#include "PalDefs.h"
#include "PalApi.h"

#define XSTR(x) STR(x)
#define STR(x) #x

//ACDB fn pointers
typedef int (*audio_extn_gef_get_pal_info)(void*, const audio_devices_t hal_device_id,
                                 pal_device_id_t* pal_device_id,
                                 audio_output_flags_t hal_stream_flag,
                                 pal_stream_type_t *pal_stream_type,
                                 char *address);

static uint64_t TRACE_TAG = ATRACE_TAG_NEVER;


typedef struct effect_private_data {
    struct listnode list;
    effect_uuid_t   uuid;
    bool            enable;
    gef_func_ptr    cb;
    void*           data;             //data given by effect
    void*           gef_private_data; //pointer to gef handle
} effect_private_data;

typedef struct gef_private_data {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int             num_effects;
    audio_extn_gef_get_pal_info get_pal_info;

    //list of all effects
    struct listnode effect_list;
    void*           hal_data;
    int             debug_enable;
    bool            debug_func_entry;
    bool            is_gef_initialized;
} gef_private_data;


static gef_private_data gef_global_handle;

static pthread_once_t gef_lib_once = PTHREAD_ONCE_INIT;

static void check_and_enable_debug_logs(struct gef_private_data *my_data)
{
    if (!my_data)
        return;

    char val[PROPERTY_VALUE_MAX] = {0};
    if (property_get("vendor.audio.gef.debug.flags", val, "") &&
        !strncmp(val, "1", 1)) {
        my_data->debug_enable = 1;
        my_data->debug_func_entry = true;
    }
}

static void check_and_enable_traces()
{
    if (property_get_bool("vendor.audio.gef.enable.traces", false)) {
        TRACE_TAG = ATRACE_TAG_AUDIO;
    }
}

static void gef_lib_init()
{
    gef_private_data* my_data = &gef_global_handle;
    list_init(&my_data->effect_list);
    //initialize debug and tracing
    check_and_enable_debug_logs(my_data);
    check_and_enable_traces();

    pthread_mutex_init(&my_data->mutex, (const pthread_mutexattr_t *) NULL);
    pthread_cond_init(&my_data->cond, (const pthread_condattr_t *) NULL);

    my_data->num_effects = 0;
    my_data->is_gef_initialized = false;
    my_data->hal_data = NULL;
}

static void gef_lib_init_once()
{
    pthread_once(&gef_lib_once, gef_lib_init);
}

static char* gef_lib_get_bus_address_name(uint32_t addr)
{
    char* addr_name= "BUS00_MEDIA";
    switch (addr)
    {
        case 0:
            addr_name = "BUS00_MEDIA";
            break;
        case 1:
            addr_name = "BUS01_SYS_NOTIFICATION";
            break;
        case 2:
            addr_name = "BUS02_NAV_GUIDANCE";
            break;
        case 3:
            addr_name = "BUS03_PHONE";
            break;
        case 4:
            addr_name = "BUS04_TTS";
            break;
        case 5:
            addr_name = "BUS05_Mirroring";
            break;
        case 8:
            addr_name = "BUS08_FRONT_PASSENGER";
            break;
        case 16:
            addr_name = "BUS16_REAR_SEAT";
        default:
            break;

    }
    return addr_name;
}

__attribute__ ((visibility ("default")))
gef_handle_t* gef_register_session(effect_uuid_t effectId)
{
    gef_private_data* my_data = &gef_global_handle;
    effect_private_data* effect_data = (effect_private_data*) NULL;

    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: enter with (%d, %d, %d)",
        __func__, effectId.timeLow, effectId.timeMid,
        effectId.timeHiAndVersion);

    gef_lib_init_once();
    pthread_mutex_lock(&my_data->mutex);

    //store effect data
    //have to create dynamic memory and store in a list
    effect_data = (effect_private_data*)calloc(1, sizeof(effect_private_data));
    if (!effect_data) {
        goto ERROR_RETURN;
    }

    effect_data->uuid = effectId;
    effect_data->enable = false;
    effect_data->cb = (gef_func_ptr) NULL;
    effect_data->data = (void*) NULL;
    effect_data->gef_private_data = (void*)my_data;

    my_data->num_effects++;

    ALOGD_IF(gef_global_handle.debug_func_entry,
        "number of registered effects %d", my_data->num_effects);

    //add this to the list of nodes
    list_add_tail(&gef_global_handle.effect_list, &effect_data->list);

ERROR_RETURN:
    pthread_cond_signal(&my_data->cond);
    pthread_mutex_unlock(&my_data->mutex);

    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: Exit with effect_handle %p", __func__, effect_data);
    return (gef_handle_t*)effect_data;
}

__attribute__ ((visibility ("default")))
int gef_deregister_session(gef_handle_t* handle)

{
    int ret = 0;
    effect_private_data* effect_data = (effect_private_data*)handle;
    gef_private_data* gp_data = (gef_private_data*) NULL;

    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: Enter", __func__);

    if (!effect_data|| !effect_data->gef_private_data) {
        ret = -EINVAL;
        goto ERROR_RETURN;
    }

    gp_data = (gef_private_data*)effect_data->gef_private_data;
    if (!gp_data || gp_data != &gef_global_handle) {
        ret = -EINVAL;
        goto ERROR_RETURN;
    }

    pthread_mutex_lock(&gp_data->mutex);

    //remove the node from the list
    gp_data->num_effects--;

    ALOGD_IF(gef_global_handle.debug_func_entry,
        "number of registered effects %d", gp_data->num_effects);
    list_remove(&effect_data->list);
    free(effect_data);

    pthread_cond_signal(&gp_data->cond);
    pthread_mutex_unlock(&gp_data->mutex);

ERROR_RETURN:
    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: Exit with error %d", __func__, ret);
    return ret;
}

__attribute__ ((visibility ("default")))
int gef_enable_effect(gef_handle_t* handle)
{
    effect_private_data* effect_data = (effect_private_data*)handle;
    effect_private_data* fx_ctxt;
    event_type type = EFFECT_ENABLED;
    event_value value = {0, EFFECT_UUID_NULL_, 0, 0, 0, 0};
    struct listnode *node;
    int ret = 0;

    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: Enter", __func__);

    if(!effect_data || !effect_data->gef_private_data ||
       effect_data->gef_private_data != &gef_global_handle) {
           ret = -EINVAL;
           goto ERROR_RETURN;
    }

    effect_data->enable = true;

    // notify enablement event to effect clients
    value.value = effect_data->enable;
    memcpy(&value.effect_uuid, &effect_data->uuid, sizeof(effect_uuid_t));

    list_for_each(node, &gef_global_handle.effect_list) {
        fx_ctxt = node_to_item(node,
                effect_private_data, list);
        if((fx_ctxt != effect_data) && fx_ctxt->cb) {
            fx_ctxt->cb(fx_ctxt, type, value);
        }
    }

ERROR_RETURN:
    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: Exit with error %d", __func__, ret);
    return ret;
}

__attribute__ ((visibility ("default")))
int gef_disable_effect(gef_handle_t* handle)
{
    effect_private_data* effect_data = (effect_private_data*)handle;
    effect_private_data* fx_ctxt;
    event_type type = EFFECT_ENABLED;
    event_value value = {0, EFFECT_UUID_NULL_, 0, 0, 0, 0};
    struct listnode *node;
    int ret = 0;

    ALOGD_IF(gef_global_handle.debug_func_entry,
         "%s: Enter", __func__);

    if(!effect_data || !effect_data->gef_private_data ||
       effect_data->gef_private_data != &gef_global_handle) {
        ret = -EINVAL;
        goto ERROR_RETURN;
    }

    effect_data->enable = false;

    // notify enablement event to effect clients
    value.value = effect_data->enable;
    memcpy(&value.effect_uuid, &effect_data->uuid, sizeof(effect_uuid_t));

    list_for_each(node, &gef_global_handle.effect_list) {
        fx_ctxt = node_to_item(node,
                effect_private_data, list);
        if((fx_ctxt != effect_data) && fx_ctxt->cb) {
            fx_ctxt->cb(fx_ctxt, type, value);
        }
    }

ERROR_RETURN:
    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: Exit with error %d", __func__, ret);
    return ret;
}

__attribute__ ((visibility ("default")))
int gef_query_version(int *majorVersion, int *minorVersion)
{

    int ret = 0;

    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: Enter", __func__);

    if (!majorVersion || !minorVersion) {
        ret = -EINVAL;
        goto ERROR_RETURN;
    }

    *majorVersion = 1;
    *minorVersion = 1;

ERROR_RETURN:

    ALOGD_IF(gef_global_handle.debug_func_entry,
            "%s: Exit with error %d", __func__, ret);
    return ret;
}

/*
 * This function is to be called by vendor abstraction layer to GEF to
 * register a callback. This callback will be invoked by GEF
 * under these circumstances
 *
 * 1. Error when sending a calibration
 * 2. Another effect which was also registered through GEF got enabled
 * 3. A device is connected to notify which device is connected
 *   and the channel map associated
 *
 * handle: - Handle to GEF
 * cb:- pointer to callback
 * data:- data that will be sent in the callback
 *
 * returns:- 0: Operation is successful
 *      EINVAL: When the session is invalid/handle is null etc
 */
__attribute__ ((visibility ("default")))
int gef_register_callback(gef_handle_t* handle, gef_func_ptr cb, void* data)
{

    int ret = 0;
    effect_private_data* effect_data = (effect_private_data*)handle;
    gef_private_data* gp_data;
    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: Enter", __func__);

    if(!effect_data|| !effect_data->gef_private_data) {
        ret = -EINVAL;
        goto ERROR_RETURN;
    }

    gp_data = (gef_private_data*)effect_data->gef_private_data;

    if (!gp_data || gp_data != &gef_global_handle) {
        ret = -EINVAL;
        goto ERROR_RETURN;
    }

    pthread_mutex_lock(&gp_data->mutex);

    effect_data->cb = cb;
    effect_data->data = data;

    pthread_cond_signal(&gp_data->cond);
    pthread_mutex_unlock(&gp_data->mutex);

ERROR_RETURN:
    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: Exit with error %d", __func__, ret);
    return ret;
}

/*
 * This method sends the parameters that will be sent to the DSP
 * The parameters are automatically stored in ACDB cache.
 *
 * handle: - Handle to GEF
 * data:- data corresponding to the effect
 *
 * returns:- 0: Operation is successful
 *       EINVAL: When the session is invalid/handle is null etc
 *       ENODEV: When GEF has not been initialized
 *       ENOSYS: Sending parameters is not supported
 */
int gef_set_ar_param(gef_handle_t* handle,
        effect_config_params* params, effect_data_in* data)
{
    int ret = 0;
    effect_private_data* effect_data = (effect_private_data*)handle;
    gef_private_data* gp_data = (gef_private_data*)NULL;
    int pal_device_count = 0;
    int device_count = 0;
    int i = 0;
    pal_device_id_t *pal_device_ids = NULL;
    pal_stream_type_t pal_stream_type;
    char *address=NULL;

    if (!params)
        return -EINVAL;

    if (params->device_id == AUDIO_DEVICE_NONE) {
        ALOGI("%s: This is stream param, as device id is AUDIO_DEVICE_NONE",
                __func__);
        device_count = 1;
    } else {
        device_count = popcount(params->device_id);
    }

    if (device_count) {
        pal_device_ids = (pal_device_id_t *)calloc(device_count,
                                                    sizeof(pal_device_id_t));
        if (!pal_device_ids)
            return -ENOMEM;
    } else {
            return -EINVAL;
    }

    ALOGD_IF(gef_global_handle.debug_func_entry, "%s: Enter", __func__);

    if((!effect_data)||(!data)||(!(data->data))) {
        ret = -EINVAL;
        goto ERROR_RETURN;
    }

    gp_data = (gef_private_data*)effect_data->gef_private_data;
    if (!gp_data || gp_data != &gef_global_handle) {
        ret = -EINVAL;
        goto ERROR_RETURN;
    }
    if (params->device_id == AUDIO_DEVICE_OUT_BUS)
    {
        //use stream_type for an bus address
        address = gef_lib_get_bus_address_name(params->stream_type);
        // reset flag
        params->stream_type = AUDIO_OUTPUT_FLAG_NONE;
    }
    ret = -ENOSYS;

    if (gef_global_handle.get_pal_info != NULL) {
        pal_device_count = gef_global_handle.get_pal_info(gef_global_handle.hal_data,
        params->device_id, pal_device_ids, params->stream_type, &pal_stream_type, address);
    } else {
        ALOGE("%s: address = 0x%x", __func__, (unsigned int)address);
        goto ERROR_RETURN;
    }
    /*
     * handle stream-only or device-only param
     */
    if (pal_device_count == 0 && params->device_id == AUDIO_DEVICE_NONE)
        pal_device_count = 1;

    if (pal_device_count <= 0) {
        ALOGE("%s: failed to get pal information from HAL.\n", __func__);
        goto ERROR_RETURN;
    }

    // if (params->stream_type == AUDIO_OUTPUT_FLAG_NONE)
    //     pal_stream_type = PAL_STREAM_GENERIC;

    for (i = 0; i < pal_device_count; i++) {
        ret = pal_gef_rw_param(PAL_PARAM_ID_UIEFFECT, data->data, data->length,
            pal_device_ids[i],
            pal_stream_type, GEF_PARAM_WRITE, address);
    }

    if (params->persist) {
        ALOGE("%s: please use gef_store_ar_cacheparam.\n", __func__);
    }

ERROR_RETURN:
    free(pal_device_ids);

    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: Exit with error %d", __func__, ret);
    return ret;
}

/*
 * This api will retrieve the entire data binary from the DSP
 * for the module and parameter ids mentioned.
 *
 * Vendor abstraction layer is expected to allocate memory for
 * the buffer into which the data corresponding to the parameters
 * will be stored
 * only only one device is allowed!
 * handle: - Handle to GEF
 * data:- data corresponding to the effect
 *
 * returns:- ENOSYS: Retrieving parameters is not supported
 */
int gef_get_ar_param(const gef_handle_t* handle,
    effect_config_params* params, effect_data_out* data)
{
    int ret = 0;
    effect_private_data* effect_data = (effect_private_data*)handle;
    gef_private_data* gp_data = (gef_private_data*)NULL;
    int pal_device_count = 0;
    int device_count = 0;
    pal_device_id_t pal_device_id;
    pal_stream_type_t pal_stream_type;
    char *address=NULL;

    if (!params)
        return -EINVAL;

    device_count = popcount(params->device_id);
    if (device_count != 1) {
        ALOGE("%s: only one device is supported for param read.", __func__);
        ret = -EINVAL;
        goto ERROR_RETURN;
    }

    ALOGD("%s: device id=%d stream type = %d\n", __func__,
            params->device_id, params->stream_type);

    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: Enter", __func__);

    if((!effect_data)||(!data)||(!(data->data))) {
        ret = -EINVAL;
        goto ERROR_RETURN;
    }

    gp_data = (gef_private_data*)effect_data->gef_private_data;
    if (!gp_data || gp_data != &gef_global_handle) {
        ret = -EINVAL;
        goto ERROR_RETURN;
    }
    if (params->device_id == AUDIO_DEVICE_OUT_BUS)
    {
        //use stream_type for an bus address
        address = gef_lib_get_bus_address_name(params->stream_type);
        // reset flag
        params->stream_type = AUDIO_OUTPUT_FLAG_NONE;
    }
    if (gef_global_handle.get_pal_info != NULL) {
        pal_device_count = gef_global_handle.get_pal_info(gef_global_handle.hal_data,
        params->device_id, &pal_device_id, params->stream_type, &pal_stream_type, address);
    } else {
        ALOGE("%s: address = 0x%x", __func__, (unsigned int)address);
        goto ERROR_RETURN;
    }

    if (pal_device_count != device_count) {
        ALOGE("%s: failed to get pal information from HAL.\n", __func__);
        goto ERROR_RETURN;
    }

    ret = pal_gef_rw_param(PAL_PARAM_ID_UIEFFECT, data->data, data->length,
                                pal_device_id, pal_stream_type, GEF_PARAM_READ, address);
ERROR_RETURN:
    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: Exit with error %d", __func__, ret);
    return ret;
}

/*
 * This method sends the parameters that are to be stored in ACDB.
 *
 * handle: - Handle to GEF
 * params:- Parameters of the effect
 * data:- data corresponding to the effect
 *
 * returns:- 0: Operation is successful
 *      EINVAL: When the session is invalid/handle is null etc
 *      ENODEV: When GEF has not been initialized
 *      ENOSYS: Sending parameters is not supported
 */
int gef_store_ar_cacheparam(gef_handle_t* handle,
    effect_config_params* params, effect_data_in* data)
{
    int ret = 0;
    effect_private_data* effect_data = (effect_private_data*)handle;
    gef_private_data* gp_data = (gef_private_data*)NULL;
    int pal_device_count = 0;
    int device_count = 0;
    int i = 0;
    pal_device_id_t    *pal_device_ids = NULL;
    pal_stream_type_t pal_stream_type;
    char *address=NULL;

    if (!params)
        return -EINVAL;

    if (params->device_id == AUDIO_DEVICE_NONE) {
        ALOGI("%s: This is stream param, as device id is AUDIO_DEVICE_NONE",
                __func__);
        device_count = 1;
    } else {
        device_count = popcount(params->device_id);
    }

    if (device_count) {
        pal_device_ids = (pal_device_id_t *)calloc(device_count,
                                                   sizeof(pal_device_id_t));
        if (!pal_device_ids)
            return -ENOMEM;
    } else {
            return -EINVAL;
    }

    ALOGD_IF(gef_global_handle.debug_func_entry, "%s: Enter", __func__);

    if((!effect_data)||(!data)||(!(data->data))) {
        ret = -EINVAL;
        goto ERROR_RETURN;
    }

    gp_data = (gef_private_data*)effect_data->gef_private_data;
    if (!gp_data || gp_data != &gef_global_handle) {
        ret = -EINVAL;
        goto ERROR_RETURN;
    }
    if (params->device_id == AUDIO_DEVICE_OUT_BUS)
    {
        //use stream_type for an bus address
        address = gef_lib_get_bus_address_name(params->stream_type);
        // reset flag
        params->stream_type = AUDIO_OUTPUT_FLAG_NONE;
    }
    ret = -ENOSYS;
    pal_device_count = gef_global_handle.get_pal_info(gef_global_handle.hal_data,
        params->device_id, pal_device_ids, params->stream_type, &pal_stream_type, address);

    /*
     * handle stream-only or device-only param
     */
    if (pal_device_count == 0 && params->device_id == AUDIO_DEVICE_NONE)
        pal_device_count = 1;

    if (pal_device_count <= 0) {
        ALOGE("%s: failed to get pal information from HAL.\n", __func__);
        goto ERROR_RETURN;
    }

    // if (params->stream_type == AUDIO_OUTPUT_FLAG_NONE)
    //     pal_stream_type = PAL_STREAM_GENERIC;

    for (i = 0; i < pal_device_count; i++) {
        ret = pal_gef_rw_param_acdb(PAL_PARAM_ID_UIEFFECT, data->data,
                      data->length, pal_device_ids[i],
                      pal_stream_type, params->sample_rate,
                      1, GEF_PARAM_WRITE, true);
    }

ERROR_RETURN:
    free(pal_device_ids);
    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: Exit with error %d", __func__, ret);
    return ret;
}

/*
 * This method retrieves the parameters that are stored in cache.
 * Vendor abstraction layer is expected to allocate memory for
 * the buffer into which the data corresponding to the parameters
 * will be stored
 *
 * THIS API IS NOT SUPPORTED FOR NOW
 * API will always return ENOSYS
 *
 * handle: - Handle to GEF
 * data:- data corresponding to the effect
 *
 * returns:- ENOSYS: Retrieving parameters is not supported
 */
int gef_retrieve_ar_cacheparam(const gef_handle_t* handle,
    effect_config_params* params, effect_data_out* data)
{
    int ret = 0;
    effect_private_data* effect_data = (effect_private_data*)handle;
    gef_private_data* gp_data = (gef_private_data*)NULL;
    int pal_device_count = 0;
    int device_count = 0;
    pal_device_id_t pal_device_id;
    pal_stream_type_t pal_stream_type;
    char *address=NULL;

    if (!params)
        return -EINVAL;

    device_count = popcount(params->device_id);
    if (device_count != 1) {
        ALOGE("%s: only one device is supported for param read.", __func__);
        ret = -EINVAL;
        goto ERROR_RETURN;
    }

    ALOGD("%s: device id=%d stream type = %d\n", __func__,
            params->device_id, params->stream_type);

    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: Enter", __func__);

    if((!effect_data)||(!data)||(!(data->data))) {
        ret = -EINVAL;
        goto ERROR_RETURN;
    }

    gp_data = (gef_private_data*)effect_data->gef_private_data;
    if (!gp_data || gp_data != &gef_global_handle) {
        ret = -EINVAL;
        goto ERROR_RETURN;
    }
    if (params->device_id == AUDIO_DEVICE_OUT_BUS)
    {
        //use stream_type for an bus address
        address = gef_lib_get_bus_address_name(params->stream_type);
        // reset flag
        params->stream_type = AUDIO_OUTPUT_FLAG_NONE;
    }
    pal_device_count = gef_global_handle.get_pal_info(gef_global_handle.hal_data,
        params->device_id, &pal_device_id, params->stream_type, &pal_stream_type, address);

    if (pal_device_count != device_count) {
        ALOGE("%s: failed to get pal information from HAL.\n", __func__);
        goto ERROR_RETURN;
    }

    ret = pal_gef_rw_param_acdb(PAL_PARAM_ID_UIEFFECT, data->data,
                      data->length, pal_device_id,
                      pal_stream_type, params->sample_rate,
                      1, GEF_PARAM_READ, true);

ERROR_RETURN:
    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: Exit with error %d", __func__, ret);
    return ret;
}


void* gef_init(void *data, audio_extn_gef_get_pal_info fp)
{
    gef_lib_init_once();
    ALOGD_IF(gef_global_handle.debug_func_entry, "%s: Enter", __func__);
    gef_private_data* my_data = &gef_global_handle;
    pthread_mutex_lock(&my_data->mutex);
    my_data->hal_data = data;
    my_data->get_pal_info = fp;
    pthread_mutex_unlock(&my_data->mutex);

    return ((void*)my_data);
}

void gef_deinit(void *data)
{
    gef_private_data* handle = (gef_private_data*) data;
    ALOGD_IF(gef_global_handle.debug_func_entry, "%s: Enter", __func__);

    if (handle != NULL && handle == &gef_global_handle) {
        pthread_mutex_lock(&handle->mutex);
        handle->hal_data = NULL;
        pthread_mutex_unlock(&handle->mutex);
    }
}

void gef_device_config_cb(void* handle, audio_devices_t device,
        audio_channel_mask_t cmask, int sample_rate, int stream_type)
{
    struct listnode *node;
    gef_private_data* my_data = handle;
    event_type type = DEVICE_CONNECTED;
    event_value value =
    {
        .value        = 0,
        .effect_uuid  = EFFECT_UUID_NULL_,
        .device       = device,
        .channel_mask = cmask,
        .sample_rate  = sample_rate,
        .stream_type  = stream_type,
    };

    ALOGD("%s: stream type=%d sample_rate=%d device=%d", __func__,
            stream_type, sample_rate, device);

    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: Enter with (device:%d, mask:%d)", __func__,
        device, cmask);
    if (!my_data || my_data != &gef_global_handle)
    {
        return;
    }

    pthread_mutex_lock(&my_data->mutex);

    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: Number of callbacks: %d)", __func__, my_data->num_effects);

    if(!my_data->num_effects)
    {
        //no enabled effects, just return
        goto DONE;
    }

    //iterate through effect in list and post callback
    list_for_each(node, &gef_global_handle.effect_list) {
        effect_private_data *effect_data = node_to_item(node,
                effect_private_data, list);
        if(effect_data->cb) {
            memcpy(&value.effect_uuid, &effect_data->uuid, sizeof(effect_uuid_t));
            effect_data->cb(effect_data, type, value);
        }
    }

DONE:
    pthread_cond_signal(&my_data->cond);
    pthread_mutex_unlock(&my_data->mutex);

    ALOGD_IF(gef_global_handle.debug_func_entry,
        "%s: Exit", __func__);
}

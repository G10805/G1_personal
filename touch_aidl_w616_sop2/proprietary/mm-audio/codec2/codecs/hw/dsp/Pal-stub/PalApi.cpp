/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/

#include "PalApi.h"


#undef LOG_TAG
#define LOG_TAG "PalApi"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC


char* pal_get_version( ) {
    char* pal_version = (char*)"1.0";
    return pal_version;
}

int32_t pal_init( ) {
    return 0;
}

void pal_deinit() {

}

int32_t pal_stream_open(struct pal_stream_attributes *attributes,
                        uint32_t no_of_devices, struct pal_device *devices,
                        uint32_t no_of_modifiers, struct modifier_kv *modifiers,
                        pal_stream_callback cb, uint64_t cookie,
                        pal_stream_handle_t **stream_handle) {
    (void)attributes;
    (void)no_of_devices;
    (void)devices;
    (void)no_of_modifiers;
    (void)modifiers;
    (void)cb;
    (void)cookie;
    (void)stream_handle;
    return 0;
}

int32_t pal_stream_close(pal_stream_handle_t *stream_handle) {
    (void)stream_handle;
    return 0;
}

int32_t pal_stream_prepare(pal_stream_handle_t *stream_handle) {
    (void)stream_handle;
    return 0;
}

int32_t pal_stream_start(pal_stream_handle_t *stream_handle) {
    (void)stream_handle;
    return 0;
}

int32_t pal_stream_stop(pal_stream_handle_t *stream_handle) {
    (void)stream_handle;
    return 0;
}

int32_t pal_stream_pause(pal_stream_handle_t *stream_handle) {
    (void)stream_handle;
    return 0;
}

int32_t pal_stream_resume(pal_stream_handle_t *stream_handle) {
    (void)stream_handle;
    return 0;
}

int32_t pal_stream_flush(pal_stream_handle_t *stream_handle) {
    (void)stream_handle;
    return 0;
}

int32_t pal_stream_drain(pal_stream_handle_t *stream_handle, pal_drain_type_t type) {
    (void)stream_handle;
    (void)type;
    return 0;
}

int32_t pal_stream_get_buffer_size(pal_stream_handle_t *stream_handle,
                                   size_t *in_buffer, size_t *out_buffer) {
    (void)stream_handle;
    (void)in_buffer;
    (void)out_buffer;
    return 0;
}

int32_t pal_stream_get_tags_with_module_info(pal_stream_handle_t *stream_handle,
                                   size_t *size, uint8_t *payload) {
    (void)stream_handle;
    (void)payload;
    (void)size;
    return 0;
}

int32_t pal_stream_set_buffer_size(pal_stream_handle_t *stream_handle,
                                    pal_buffer_config_t *in_buff_cfg,
                                    pal_buffer_config_t *out_buff_cfg) {
    (void)stream_handle;
    (void)in_buff_cfg;
    (void)out_buff_cfg;
    return 0;
}

ssize_t pal_stream_read(pal_stream_handle_t *stream_handle, struct pal_buffer *buf) {
    (void)stream_handle;
    return buf->size;
}

ssize_t pal_stream_write(pal_stream_handle_t *stream_handle, struct pal_buffer *buf) {
    (void)stream_handle;
    return buf->size;
}

int32_t pal_stream_get_device(pal_stream_handle_t *stream_handle,
                            uint32_t no_of_devices, struct pal_device *devices) {
    (void)stream_handle;
    (void)no_of_devices;
    (void)devices;
    return 0;
}

int32_t pal_stream_set_device(pal_stream_handle_t *stream_handle,
                           uint32_t no_of_devices, struct pal_device *devices) {
    (void)stream_handle;
    (void)no_of_devices;
    (void)devices;
    return 0;
}

int32_t pal_stream_get_param(pal_stream_handle_t *stream_handle,
                           uint32_t param_id, pal_param_payload **param_payload) {
    (void)stream_handle;
    (void)param_id;
    (void)param_payload;
    return 0;
}

int32_t pal_stream_set_param(pal_stream_handle_t *stream_handle,
                           uint32_t param_id, pal_param_payload *param_payload) {
    (void)stream_handle;
    (void)param_id;
    (void)param_payload;
    return 0;
}

int32_t pal_stream_get_volume(pal_stream_handle_t *stream_handle,
                              struct pal_volume_data *volume) {
    (void)stream_handle;
    (void)volume;
    return 0;
}

int32_t pal_stream_set_volume(pal_stream_handle_t *stream_handle,
                              struct pal_volume_data *volume) {
    (void)stream_handle;
    (void)volume;
    return 0;
}

int32_t pal_stream_get_mute(pal_stream_handle_t *stream_handle, bool *state) {
    (void)stream_handle;
    (void)state;
    return 0;
}

int32_t pal_stream_set_mute(pal_stream_handle_t *stream, bool state) {
    (void)stream;
    (void)state;
    return 0;
}

int32_t pal_get_mic_mute(bool *state) {
    (void)state;
    return 0;
}

int32_t pal_set_mic_mute(bool state) {
    (void)state;
    return 0;
}

int32_t pal_get_timestamp(pal_stream_handle_t *stream_handle, struct pal_session_time *stime) {
    (void)stream_handle;
    (void)stime;
    return 0;
}

int32_t pal_add_remove_effect(pal_stream_handle_t *stream_handle, pal_audio_effect_t effect, bool enable) {
    (void)stream_handle;
    (void)effect;
    (void)enable;
    return 0;
}

int32_t pal_set_param(uint32_t param_id, uint8_t *param_payload,
                      size_t payload_size) {
    (void)param_id;
    (void)param_payload;
    (void)payload_size;
    return 0;
}

int32_t pal_get_param(uint32_t param_id, uint8_t **param_payload,
                      size_t *payload_size, uint8_t *query) {
    (void)param_id;
    (void)param_payload;
    (void)payload_size;
    (void)query;
    return 0;
}

int32_t pal_stream_create_mmap_buffer(pal_stream_handle_t *stream_handle,
                              int32_t min_size_frames,
                              struct pal_mmap_buffer *info) {
    (void)stream_handle;
    (void)min_size_frames;
    (void)info;
    return 0;
}

int32_t pal_stream_get_mmap_position(pal_stream_handle_t *stream_handle,
                              struct pal_mmap_position *position) {
    (void)stream_handle;
    (void)position;
    return 0;
}
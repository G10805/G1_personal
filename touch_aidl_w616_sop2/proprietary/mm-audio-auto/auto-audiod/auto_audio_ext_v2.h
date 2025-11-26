/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

__BEGIN_DECLS

#ifdef AGM_HOSTLESS

enum session_setting_type_t {
    SESSION_SET_PINCTRL,
    SESSION_SET_CLOCK,
    SESSION_SET_MAX
};

typedef struct device_hostless_ins_t {
    uint32_t aif_id;
    mutable uint32_t session_id;
    mutable uint64_t session_handle;
    mutable session_setting_type_t session_set_type;
    struct agm_session_config *session_config;
    struct agm_media_config *media_config;
    struct agm_buffer_config *buffer_config;
    uint64_t meta_size;
    uint32_t *metadata;
} device_hostless_ins;

void auto_audio_ar_ext_init(void);
void auto_audio_ar_ext_deinit(void);
int32_t auto_audio_ext_ar_enable_hostless(int snd_card);
void auto_audio_ext_ar_disable_hostless(int snd_card);
int32_t auto_audio_ext_ar_enable_hostless_all(void);
void auto_audio_ext_ar_disable_hostless_all(void);

#else //dummy function
static inline void auto_audio_ar_ext_init(void)
{
}

static inline void auto_audio_ar_ext_deinit(void)
{
}

static inline int32_t auto_audio_ext_ar_enable_hostless(int snd_card)
{
    return 0;
}

static inline void auto_audio_ext_ar_disable_hostless(int snd_card)
{
}

static inline int32_t auto_audio_ext_ar_enable_hostless_all(void)
{
    return 0;
}

static inline void auto_audio_ext_ar_disable_hostless_all(void)
{
}

#endif //AGM_HOSTLESS

__END_DECLS

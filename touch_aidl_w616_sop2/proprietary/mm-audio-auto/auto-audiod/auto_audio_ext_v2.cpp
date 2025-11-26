/*
 * Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "auto_audio_ext_v2"
#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <agm/agm_api.h>
#include <cutils/properties.h>
#include "AutoAudioDaemon.h"
#include "auto_audio_ext_v2.h"
#include "hostless_cfg.h"
#ifdef AUDIOD_AMS_CORE_INIT
#include "ams_client_proxy.h"
#endif

#define AR_SND_MAX 1
#define AGN_INIT_SLEEP_WAIT 10 /* 10 ms */
#define HOSTLESS_CFG_WAIT_CNT 10

extern "C" {

#include <snd-card-def.h>

#define DEVICE_CLK_MAX 3
#ifndef AGM_DEFAULT_STATE
#define AGM_DEFAULT_STATE "\0"
#endif
#ifdef AUDIOD_AMS_CORE_INIT
#ifndef AMS_DEFAULT_STATE
#define AMS_DEFAULT_STATE "\0"
#define AMS_MAX_WAIT_CNT 2500
#endif
#endif
/* default session handle defined for a valid session, will be updated when session open */
#define SESSION_HANDLE_DEFAULT 0
/* session handle defined for a invalid session */
#define SESSION_HANDLE_INVALID 0xFFFF
/* predefined session_id for clk graphs, should avoid conflict with existing usecase*/
#define SESSION_ID_BASE 500

struct agm_session_config stream_config = {
    .dir = RX,
    .sess_mode = AGM_SESSION_NO_HOST,
};

struct agm_buffer_config buffer_config = { 0, 0, 0};

static pthread_mutex_t ar_hostless_ins_lock;

#ifdef PLATFORM_MSMNILE_AU
#define TDM_LPAIF_RX_TERTIARY 4
#define TDM_LPAIF_RX_QUATERNARY 26
#define TDM_LPAIF_RX_QUINARY 28

#define TDM_LPAIF_RX_TERTIARY_STR "TERT"
#define TDM_LPAIF_RX_QUATERNARY_STR "QUAT"
#define TDM_LPAIF_RX_QUINARY_STR "QUIN"

int map_aif_from_tdm(const char* tdm_name) {
    if (!strcmp(TDM_LPAIF_RX_TERTIARY_STR, tdm_name)) {
        return TDM_LPAIF_RX_TERTIARY;
    } else if (!strcmp(TDM_LPAIF_RX_QUATERNARY_STR, tdm_name)) {
        return TDM_LPAIF_RX_QUATERNARY;
    } else if (!strcmp(TDM_LPAIF_RX_QUINARY_STR, tdm_name)) {
        return TDM_LPAIF_RX_QUINARY;
    }
    return 0;
}

struct agm_media_config tert_media_config = { 48000, 8, AGM_FORMAT_PCM_S16_LE, 1};
struct agm_media_config quat_media_config = { 48000, 32, AGM_FORMAT_PCM_S16_LE, 1};
struct agm_media_config quin_media_config = { 48000, 16, AGM_FORMAT_PCM_S16_LE, 1};

uint32_t dev_tert_clk_metadata[] = {
		1, /* No of GKVS*/
		0xA2000000, 0xA200F002, /*GKVS*/
};
uint32_t dev_quat_clk_metadata[] = {
		1, /* No of GKVS*/
		0xA2000000, 0xA200F003, /*GKVS*/
};
uint32_t dev_quin_clk_metadata[] = {
		1, /* No of GKVS*/
		0xA2000000, 0xA200F004, /*GKVS*/
};

const static device_hostless_ins hostless_instances[AR_SND_MAX][DEVICE_CLK_MAX] = {
    {
        {
            .aif_id = TDM_LPAIF_RX_TERTIARY,
            .session_handle = SESSION_HANDLE_DEFAULT,
            .session_set_type = SESSION_SET_CLOCK,
            .session_config = &stream_config,
            .media_config = &tert_media_config,
            .buffer_config = &buffer_config,
            .meta_size = sizeof(dev_tert_clk_metadata),
            .metadata = dev_tert_clk_metadata
        },
        {
            .aif_id = TDM_LPAIF_RX_QUATERNARY,
            .session_handle = SESSION_HANDLE_DEFAULT,
            .session_set_type = SESSION_SET_CLOCK,
            .session_config = &stream_config,
            .media_config = &quat_media_config,
            .buffer_config = &buffer_config,
            .meta_size = sizeof(dev_quat_clk_metadata),
            .metadata = dev_quat_clk_metadata
        },
        {
            .aif_id = TDM_LPAIF_RX_QUINARY,
            .session_handle = SESSION_HANDLE_DEFAULT,
            .session_set_type = SESSION_SET_CLOCK,
            .session_config = &stream_config,
            .media_config = &quin_media_config,
            .buffer_config = &buffer_config,
            .meta_size = sizeof(dev_quin_clk_metadata),
            .metadata = dev_quin_clk_metadata
        }
    }
};

#elif PLATFORM_LEMANS
#define TDM_LPAIF_RXTX_RX_PRIMARY 6 //QUAT
#define TDM_LPAIF_VA_RX_PRIMARY 8 //QUIN
#define TDM_LPAIF_SDR_RX_TERTIARY 20 //TERT

#define TDM_LPAIF_RXTX_RX_PRIMARY_STR "TDM-LPAIF-RXTX-RX-PRIMARY"
#define TDM_LPAIF_VA_RX_PRIMARY_STR "TDM-LPAIF-VA-RX-PRIMARY"
#define TDM_LPAIF_SDR_RX_TERTIARY_STR "TDM-LPAIF-SDR-RX-TERTIARY"

int map_aif_from_tdm(const char* tdm_name) {
    if (!strcmp(TDM_LPAIF_RXTX_RX_PRIMARY_STR, tdm_name)) {
        return TDM_LPAIF_RXTX_RX_PRIMARY;
    } else if (!strcmp(TDM_LPAIF_VA_RX_PRIMARY_STR, tdm_name)) {
        return TDM_LPAIF_VA_RX_PRIMARY;
    } else if (!strcmp(TDM_LPAIF_SDR_RX_TERTIARY_STR, tdm_name)) {
        return TDM_LPAIF_SDR_RX_TERTIARY;
    }
    return 0;
}

struct agm_media_config quat_media_config = { 48000, 32, AGM_FORMAT_PCM_S16_LE, 1};
struct agm_media_config quin_media_config = { 48000, 16, AGM_FORMAT_PCM_S16_LE, 1};
struct agm_media_config hs_if2_media_config = { 48000, 8, AGM_FORMAT_PCM_S16_LE, 1};

uint32_t dev_hs_if2_clk_metadata[] = {
		1, /* No of GKVS*/
		0xA2000000, 0xA2000008, /*GKVS*/
};
uint32_t dev_quat_clk_metadata[] = {
		1, /* No of GKVS*/
		0xA2000000, 0xA2000009, /*GKVS*/
};
uint32_t dev_quin_clk_metadata[] = {
		1, /* No of GKVS*/
		0xA2000000, 0xA200000A, /*GKVS*/
};

const static device_hostless_ins hostless_instances[AR_SND_MAX][DEVICE_CLK_MAX] = {
    {
        {
            .aif_id = TDM_LPAIF_SDR_RX_TERTIARY,
            .session_handle = SESSION_HANDLE_DEFAULT,
            .session_set_type = SESSION_SET_CLOCK,
            .session_config = &stream_config,
            .media_config = &hs_if2_media_config,
            .buffer_config = &buffer_config,
            .meta_size = sizeof(dev_hs_if2_clk_metadata),
            .metadata = dev_hs_if2_clk_metadata
        },
        {
            .aif_id = TDM_LPAIF_RXTX_RX_PRIMARY,
            .session_handle = SESSION_HANDLE_DEFAULT,
            .session_set_type = SESSION_SET_CLOCK,
            .session_config = &stream_config,
            .media_config = &quat_media_config,
            .buffer_config = &buffer_config,
            .meta_size = sizeof(dev_quat_clk_metadata),
            .metadata = dev_quat_clk_metadata
        },
        {
            .aif_id = TDM_LPAIF_VA_RX_PRIMARY,
            .session_handle = SESSION_HANDLE_DEFAULT,
            .session_set_type = SESSION_SET_CLOCK,
            .session_config = &stream_config,
            .media_config = &quin_media_config,
            .buffer_config = &buffer_config,
            .meta_size = sizeof(dev_quin_clk_metadata),
            .metadata = dev_quin_clk_metadata
        }
    }
};

#else
const static device_hostless_ins hostless_instances[AR_SND_MAX][DEVICE_CLK_MAX] = {
    {
        {.session_handle = SESSION_HANDLE_INVALID},
        {.session_handle = SESSION_HANDLE_INVALID},
        {.session_handle = SESSION_HANDLE_INVALID}
    }
};

int map_aif_from_tdm(const char* tdm_name) {
    return 0;
}
#endif

static void set_oem_disable_cfg(int snd_card) {
    uint32_t aif_id = 0;
    int ret = 0;
    int count = 0;
    int head_i = 0;
    int tail_i = 0;
    char oem_disable_tdm[PROPERTY_VALUE_MAX] = {0};
    int size = 0;

#ifdef PLATFORM_MSMNILE_AU
    while (ret = audiod_hostless_disbaled_cfg(oem_disable_tdm)) {
        ALOGW("wait cfg for 10 ms");
        usleep(AGN_INIT_SLEEP_WAIT*1000);
        count++;
        if (count > HOSTLESS_CFG_WAIT_CNT)
            break;
    }
#endif

    size = strlen(oem_disable_tdm);
    if (size > 0) {
        /* delimiter is ',' by default */
        do {
            if (oem_disable_tdm[tail_i] == ',' || oem_disable_tdm[tail_i] == '\0') {
                oem_disable_tdm[tail_i] = '\0';
                aif_id = map_aif_from_tdm(&oem_disable_tdm[head_i]);
                for (const auto& ins: hostless_instances[snd_card]) {
                    if (ins.aif_id == aif_id) {
                        ALOGW("%s: disable %d", __func__, aif_id);
                        ins.session_set_type = SESSION_SET_PINCTRL;
                    }
                }
                head_i = tail_i + 1;
            }
            tail_i++;
        } while (tail_i <= size);
    } else {
        ALOGW("%s: No config property", __func__);
    }
}

static int set_hostless_session_id(int snd_card) {
    uint32_t max_id = 0;
    int node_count = 0;
    void **node_list = NULL;
    int ret = 0;
    int device_id;

/* FIXME: how to get agm card id? */
#ifdef LINUX_ENABLED
    int agm_snd_card = snd_card + 1;
#else
    int agm_snd_card = snd_card;
#endif
    void* card_def = snd_card_def_get_card(agm_snd_card);

    if (!card_def) {
        ALOGE("%s: fail to parser %d", __func__, agm_snd_card);
        return -EINVAL;
    }
    for (int type = 0; type < SND_NODE_TYPE_MAX; type++) {

        node_count = snd_card_def_get_num_node(card_def, type);
        if (node_count <= 0) {
            continue;
        }

        node_list = (void **)calloc(node_count, sizeof(*node_list));
        if (!node_list) {
            ALOGE("%s: alloc for node_list failed", __func__);
            ret = -EINVAL;
            goto end;
        }

        ret = snd_card_def_get_nodes_for_type(card_def, type, node_list, node_count);
        if (ret) {
            ALOGE("%s: failed to get node list, err %d\n", __func__, ret);
            goto end;
        }

        for (int i = 0; i < node_count; i++) {
            ret = snd_card_def_get_int(node_list[i], "id", &device_id);
            if (device_id > max_id) {
                max_id = device_id;
            }
        }
    }

    for (auto& ins: hostless_instances[snd_card]) {
        ins.session_id = ++max_id;
    }
end:
    snd_card_def_put_card(card_def);
    return ret;
}

static int32_t auto_audio_ext_ar_set_dev_metadata(const device_hostless_ins& ins)
{
    int ret = 0;

    ret = agm_aif_set_media_config(ins.aif_id, ins.media_config);
    if (ret) {
        ALOGE("%s: set media config failed", __func__);
        return -EINVAL;
    }

    ret = agm_aif_set_metadata(ins.aif_id, ins.meta_size, (uint8_t*)ins.metadata);
    if (ret) {
        ALOGE("%s: set metadata failed", __func__);
        return -EINVAL;
    }

    ret = agm_session_aif_set_metadata(ins.session_id,
                ins.aif_id,
                ins.meta_size,
                (uint8_t*)ins.metadata);
    if (ret) {
        ALOGE("%s: set session metadata failed", __func__);
       return -EINVAL;
    }

    return ret;
}

static int32_t auto_audio_ext_ar_open_start_pcm(const device_hostless_ins& ins)
{
    int ret = 0;
    ret = agm_session_aif_connect(ins.session_id, ins.aif_id, true);
    if (ret) {
        ALOGE("%s: session connect failed", __func__);
        return -EINVAL;
    }

    ret = agm_session_open(ins.session_id, AGM_SESSION_NO_HOST, &ins.session_handle);
    if (ret) {
        ALOGE("%s: session open failed", __func__);
        return -EINVAL;
    }

    if(ins.session_set_type == SESSION_SET_CLOCK) {
        ret = agm_session_set_config(ins.session_handle, ins.session_config, ins.media_config,
                    ins.buffer_config);
        if (ret) {
            ALOGE("%s: session set config failed", __func__);
            return -EINVAL;
        }

        ret = agm_session_prepare(ins.session_handle);
        if (ret) {
            ALOGE("%s: session prepare failed", __func__);
            return -EINVAL;
        }

        ret = agm_session_start(ins.session_handle);
        if (ret) {
            ALOGE("%s: session start failed", __func__);
            return -EINVAL;
        }
    }

    return ret;
}

static int32_t auto_audio_ext_ar_stop_close_pcm(const device_hostless_ins& ins)
{
    int ret = 0;
    if (ins.session_set_type == SESSION_SET_CLOCK) {
        ret = agm_session_stop(ins.session_handle);
        if (ret) {
            ALOGE("%s: session stop failed", __func__);
        }
    }
    ret = agm_session_close(ins.session_handle);
    if (ret) {
        ALOGE("%s: session close failed", __func__);
    }

    ins.session_handle = SESSION_HANDLE_DEFAULT;
    return ret;
}

/* Note: Due to ADP H/W design, SoC TERT/SEC TDM CLK and FSYNC lines are
 * both connected with CODEC and a single master is needed to provide
 * consistent CLK and FSYNC to slaves, hence configuring SoC TERT TDM as
 * single master and bring up a dummy hostless from TERT to SEC to ensure
 * both slave SoC SEC TDM and CODEC are driven upon system boot.
 */
int32_t auto_audio_ext_ar_enable_hostless(int snd_card)
{
    int32_t ret = 0;
    ALOGI("%s: ENTER", __func__);
    pthread_mutex_lock(&ar_hostless_ins_lock);
    if (snd_card < AR_SND_MAX) {
        set_oem_disable_cfg(snd_card);
        for (const auto& ins: hostless_instances[snd_card]) {
            if(ins.session_handle == SESSION_HANDLE_INVALID)
                continue;

            if (ins.session_handle != SESSION_HANDLE_DEFAULT) {
                ALOGE("%s: hostless for aif_id %d already opened/started", __func__, ins.aif_id);
                continue;
            }
            ALOGI("%s: session_id = %d, aif_id = %d", __func__, ins.session_id, ins.aif_id);
            if (!auto_audio_ext_ar_set_dev_metadata(ins))
                auto_audio_ext_ar_open_start_pcm(ins);
        }
    } else {
        ALOGE("%s, audioreach audiod snd cards num %d", __func__, AR_SND_MAX);
    }
    pthread_mutex_unlock(&ar_hostless_ins_lock);

#ifdef PLATFORM_MSMNILE_AU
    property_set("vendor.audio.feature.hostless.enable", "running");
#endif

    ALOGI("%s: EXIT", __func__);
    return ret;
}

void auto_audio_ext_ar_disable_hostless(int snd_card)
{

    ALOGI("%s: ENTER", __func__);
    pthread_mutex_lock(&ar_hostless_ins_lock);

    if (snd_card < AR_SND_MAX) {
        for (const auto& ins: hostless_instances[snd_card]) {
            if(ins.session_handle == SESSION_HANDLE_INVALID)
                continue;
            auto_audio_ext_ar_stop_close_pcm(ins);
        }
    } else {
        ALOGE("%s, audioreach audiod snd cards num %d", __func__, AR_SND_MAX);
    }

    pthread_mutex_unlock(&ar_hostless_ins_lock);
    ALOGI("%s: EXIT", __func__);
    return;
}

int32_t auto_audio_ext_ar_enable_hostless_all(void)
{
    int32_t ret = 0;
    for (int i = 0; i < AR_SND_MAX; i++) {
        auto_audio_ext_ar_enable_hostless(i);
    }
    return ret;
}

void auto_audio_ext_ar_disable_hostless_all(void)
{
    for (int i = 0; i < AR_SND_MAX; i++) {
        auto_audio_ext_ar_disable_hostless(i);
    }
    return;
}

static int32_t auto_audio_ar_ext_wait_agm(void)
{
    char agm_state[PROPERTY_VALUE_MAX];
    while (1) {
        // audiod starts before agm
#ifdef PLATFORM_MSMNILE_AU
        property_get("vendor.audio.feature.agm.enable", agm_state, AGM_DEFAULT_STATE);
#else
        property_get("init.svc.vendor.agm-1-0", agm_state, AGM_DEFAULT_STATE);
#endif
        if (!strcmp(agm_state, "running")) {
            break;
        } else {
            ALOGW("wait agm for 10 ms");
            usleep(AGN_INIT_SLEEP_WAIT*1000);
        }
    }
    // actually nothing done from agm client
    agm_init();
    return 0;
}
#ifdef AUDIOD_AMS_CORE_INIT
static int32_t auto_audio_ar_ext_wait_ams(void)
{
    int ret = 0;
    int wait_cnt = 0;
    char ams_state[PROPERTY_VALUE_MAX];
    while (wait_cnt++ < AMS_MAX_WAIT_CNT) {
        // audiod starts before ams
        property_get("init.svc.vendor.ams-1-0", ams_state, AMS_DEFAULT_STATE);
        if (!strcmp(ams_state, "running")) {
            ret = ams_core_init();
            ALOGD("AMS init ret %d", ret);
            break;
        } else {
            ALOGW("wait ams for 50 ms");
            usleep(AGN_INIT_SLEEP_WAIT*1000);
        }
    }
    if (wait_cnt >= AMS_MAX_WAIT_CNT){
        ALOGE("AMS init skipped as service was not started!");
    }
    return 0;
}
#endif
void auto_audio_ar_ext_init(void)
{
    auto_audio_ar_ext_wait_agm();
    pthread_mutex_init(&ar_hostless_ins_lock, NULL);
    for (int i = 0; i < AR_SND_MAX; i++) {
        /* try to get the max session id from card-defs.xml, if failed, SESSION_ID_BASE is used by default */
        if (set_hostless_session_id(i)) {
            for (int j = 0; j < DEVICE_CLK_MAX; j++) {
                hostless_instances[i][j].session_id = SESSION_ID_BASE + j;
            }
        }
    }
#ifdef AUDIOD_AMS_CORE_INIT
    auto_audio_ar_ext_wait_ams();
#endif
    return;
}

void auto_audio_ar_ext_deinit(void)
{
    auto_audio_ext_ar_disable_hostless_all();
    pthread_mutex_destroy(&ar_hostless_ins_lock);
    return;
}

}

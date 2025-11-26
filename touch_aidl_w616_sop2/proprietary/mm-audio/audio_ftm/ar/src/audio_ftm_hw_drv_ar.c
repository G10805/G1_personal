/*

Copyright (c) 2011-2013, 2015, 2017, 2018, 2020-2021, Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

Not a Contribution.

Copyright (c) 2011-2012, The Linux Foundation. All rights reserved.

Copyright 2010, The Android Open-Source Project
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <math.h>
#include "audio_ftm_hw_drv.h"
#include "audio_ftm_dtmf_gen.h"
#include "fft.h"
#include <tinyalsa/asoundlib.h>
#include <sound/asound.h>
#include <kvh2xml.h>
#include <agm/agm_api.h>
//struct agm_key_value {
//    uint32_t key; /**< key */
//    uint32_t value; /**< value */
//};
/*
struct prop_data {
    uint32_t prop_id;
    uint32_t num_values;
uint32_t values[];
};
*/
enum stream_type {
    STREAM_PCM,
    STREAM_COMPRESS,
};

static DALBOOL    g_bDriverInitialized=FALSE;   //  whether it has been inited or not
#define SOUND_CARD_NUM 100  // virtual sound card
#define DEEP_BUFFER_OUTPUT_PERIOD_SIZE 960
#define DEEP_BUFFER_OUTPUT_PERIOD_COUNT 8
#define DEFAULT_OUTPUT_SAMPLING_RATE 48000
#define LOW_LATENCY_OUTPUT_PERIOD_SIZE 240
#define LOW_LATENCY_OUTPUT_PERIOD_COUNT 2

#define PADDING_8BYTE_ALIGN(x)  ((((x) + 7) & 7) ^ 7)
#define METADATA_SIZE 16
enum dir{
    PLAYBACK,
    CAPTURE,
    LOOPBACK,
};

static struct pcm_config pb_config = {
    .channels = 2,
    .rate = DEFAULT_OUTPUT_SAMPLING_RATE,
    .period_size = DEEP_BUFFER_OUTPUT_PERIOD_SIZE,
    .period_count = DEEP_BUFFER_OUTPUT_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
    .start_threshold = DEEP_BUFFER_OUTPUT_PERIOD_SIZE / 4,
    .stop_threshold = INT_MAX,
    .avail_min = DEEP_BUFFER_OUTPUT_PERIOD_SIZE / 4,
};

static struct pcm_config pcm_config_low_latency = {
    .channels = 2,
    .rate = DEFAULT_OUTPUT_SAMPLING_RATE,
    .period_size = LOW_LATENCY_OUTPUT_PERIOD_SIZE,
    .period_count = LOW_LATENCY_OUTPUT_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
    .start_threshold = LOW_LATENCY_OUTPUT_PERIOD_SIZE / 4,
    .stop_threshold = INT_MAX,
    .avail_min = LOW_LATENCY_OUTPUT_PERIOD_SIZE / 4,
};

static struct pcm_config cp_config = {
    .channels = 2,
    .period_count = 2,
    .format = PCM_FORMAT_S16_LE,
};
static struct pcm_config pcm_config_fm = {
    .channels = 2,
    .rate = 48000,
    .period_size = 256,
    .period_count = 4,
    .format = PCM_FORMAT_S16_LE,
    .start_threshold = 0,
    .stop_threshold = INT_MAX,
    .avail_min = 0,
};
volatile int g_loopback_run = 0;
volatile int g_fm_run = 0;

int g_curr_rx_device = -1;
int g_curr_tx_device = -1;
int g_curr_afe_lb_device = -1;
int g_curr_adie_lb_device = -1;
int g_curr_fm_device = -1;
int g_curr_ext_lb_device = -1;
int g_curr_device = -1;
static int g_playback_device = -1, g_capture_device = -1;
static int fm_capture = -1, fm_playback = -1;
const char *g_curr_alsa_device_name = AUDIO_FTM_ALSA_DEVICE_MM1;

static struct pcm *g_pcm = 0;
#define AUDIO_CAPTURE_PERIOD_DURATION_MSEC 20

enum {
    FFT_IDLE = 0,
    FFT_START = 9,
    FFT_COMPLETE = 10
}g_enum_status;

typedef struct {
    int freq;
    const char *rec_filename;
    int tone_analysis;
    int freq_tolerance;
}ext_loop_param;

unsigned int g_analyze_status = 0;
unsigned int g_freq_tested = 0;
unsigned int g_total_freq_to_test = NUM_OF_FREQ;
unsigned int g_cap_freq;
ext_loop_param ext_lp_param;

/* unit is %. 5 means 5% tolerance */
#define TOLERANCE 5
#define TONE_VOLUME 0x1000

#define EVENT_ID_DETECTION_ENGINE_GENERIC_INFO 0x0800104F
#define PARAM_ID_DETECTION_ENGINE_GENERIC_EVENT_CFG 0x0800104E
#define PARAM_ID_MFC_OUTPUT_MEDIA_FORMAT            0x08001024

enum pcm_channel_map
{
   PCM_CHANNEL_L = 1,
   PCM_CHANNEL_R = 2,
   PCM_CHANNEL_C = 3,
   PCM_CHANNEL_LS = 4,
   PCM_CHANNEL_RS = 5,
   PCM_CHANNEL_LFE = 6,
   PCM_CHANNEL_CS = 7,
   PCM_CHANNEL_CB = PCM_CHANNEL_CS,
   PCM_CHANNEL_LB = 8,
   PCM_CHANNEL_RB = 9,
};

/* Payload of the PARAM_ID_MFC_OUTPUT_MEDIA_FORMAT parameter in the
 Media Format Converter Module. Following this will be the variable payload for channel_map. */
struct param_id_mfc_output_media_fmt_t
{
   int32_t sampling_rate;
   int16_t bit_width;
   int16_t num_channels;
   uint16_t channel_type[0];
}__attribute__((packed));

struct apm_module_param_data_t
{
   uint32_t module_instance_id;
   uint32_t param_id;
   uint32_t param_size;
   uint32_t error_code;
};

struct detection_engine_generic_event_cfg {
    uint32_t event_mode;
};


struct gsl_module_id_info_entry {
	uint32_t module_id; /**< module id */
	uint32_t module_iid; /**< globally unique module instance id */
};

/**
 * Structure mapping the tag_id to module info (mid and miid)
 */
struct gsl_tag_module_info_entry {
	uint32_t tag_id; /**< tag id of the module */
	uint32_t num_modules; /**< number of modules matching the tag_id */
	struct gsl_module_id_info_entry module_entry[0]; /**< module list */
};

struct gsl_tag_module_info {
    uint32_t num_tags; /**< number of tags */
    struct gsl_tag_module_info_entry tag_module_entry[0];
    /**< variable payload of type struct gsl_tag_module_info_entry */
};

static int rec_loopback();

static unsigned int  bits_to_alsa_format(unsigned int bits)
{
    switch (bits) {
    case 32:
        return SNDRV_PCM_FORMAT_S32_LE;
    case 8:
        return SNDRV_PCM_FORMAT_S8;
    case 24:
        return SNDRV_PCM_FORMAT_S24_3LE;
    default:
    case 16:
        return SNDRV_PCM_FORMAT_S16_LE;
    };
}

int set_device_media_config(struct mixer *mixer, unsigned int channels,
                                unsigned int rate, unsigned int bits, char *intf_name)
{
    char *control = "rate ch fmt";
    char *mixer_str;
    struct mixer_ctl *ctl;
    long media_config[4];
    int ctl_len = 0;
    int ret = 0;

    ctl_len = strlen(intf_name) + 1 + strlen(control) + 1;
    mixer_str = calloc(1, ctl_len);
    if (!mixer_str) {
        DALSYS_Log_Err(LOG_TAG, "%s: memory allocation failed.", __func__);
        return -ENOMEM;
    }
    snprintf(mixer_str, ctl_len, "%s %s", intf_name, control);

    DALSYS_Log_Info("%s - mixer -%s-\n", __func__, mixer_str);
    ctl = mixer_get_ctl_by_name(mixer, mixer_str);
    if (!ctl) {
        DALSYS_Log_Err("Invalid mixer control: %s\n", mixer_str);
        free(mixer_str);
        return ENOENT;
    }

    media_config[0] = rate;
    media_config[1] = channels;
    media_config[2] = bits_to_alsa_format(bits);
    media_config[3] = AGM_DATA_FORMAT_FIXED_POINT;

    DALSYS_Log_Info("%s - %d - %d - %d - %d\n", __func__, media_config[0],  media_config[1], media_config[2], media_config[3]);
    ret = mixer_ctl_set_array(ctl, &media_config, sizeof(media_config)/sizeof(media_config[0]));
    free(mixer_str);
    return ret;
}

int set_audio_intf_metadata(struct mixer *mixer, char *intf_name, enum dir d, int rate, int bitwidth, char *gkv_value)
{
    char *control = "metadata";
    struct mixer_ctl *ctl;
    char *mixer_str;
    struct agm_key_value *gkv = NULL, *ckv = NULL;
    struct prop_data *prop = NULL;
    uint8_t *metadata = NULL;
    uint32_t num_gkv = 1, num_ckv = 2, num_props = 0, val = 0;
    uint32_t gkv_size, ckv_size, prop_size, ckv_index = 0;
    int ctl_len = 0, offset = 0, gkv_count = 0;
    int ret = 0;
    char *split_gkv[30];
    char *temp, *rest = NULL;
    const char s[2] = "-";

    char *ptr = strtok_r(gkv_value, s, &rest);
    split_gkv[0] = ptr;
    while(ptr != NULL) {
          ptr = strtok_r(NULL, s, &rest);
          gkv_count++;
          split_gkv[gkv_count] = ptr;
    }

    if(!strncmp(split_gkv[1], "SPEAKER", strlen(split_gkv[1])+1))
       val = SPEAKER;
    else if (!strncmp(split_gkv[1], "HEADPHONES", strlen(split_gkv[1])+1))
       val = HEADPHONES;
    else if (!strncmp(split_gkv[1], "BT_RX", strlen(split_gkv[1])+1))
       val = BT_RX;
    else if (!strncmp(split_gkv[1], "HANDSET", strlen(split_gkv[1])+1))
       val = HANDSET;
    else if (!strncmp(split_gkv[1], "USB_RX", strlen(split_gkv[1])+1))
       val = USB_RX;
    else if (!strncmp(split_gkv[1], "HDMI_RX", strlen(split_gkv[1])+1))
       val = HDMI_RX;
    else if (!strncmp(split_gkv[1], "SPEAKER_MIC", strlen(split_gkv[1])+1))
       val = SPEAKER_MIC;
    else if (!strncmp(split_gkv[1], "BT_TX", strlen(split_gkv[1])+1))
       val = BT_TX;
    else if (!strncmp(split_gkv[1], "HEADPHONE_MIC", strlen(split_gkv[1])+1))
       val = HEADPHONE_MIC;
    else if (!strncmp(split_gkv[1], "HANDSETMIC", strlen(split_gkv[1])+1))
       val = HANDSETMIC;
    else if (!strncmp(split_gkv[1], "USB_TX", strlen(split_gkv[1])+1))
       val = USB_TX;
    else if (!strncmp(split_gkv[1], "FM_TX", strlen(split_gkv[1])+1))
       val = FM_TX;

    gkv_size = num_gkv * sizeof(struct agm_key_value);
    ckv_size = num_ckv * sizeof(struct agm_key_value);
    prop_size = sizeof(struct prop_data) + (num_props * sizeof(uint32_t));

    metadata = calloc(1, sizeof(num_gkv) + sizeof(num_ckv) + gkv_size + ckv_size + prop_size);
    if (!metadata)
        return -ENOMEM;

    gkv = calloc(num_gkv, sizeof(struct agm_key_value));
    ckv = calloc(num_ckv, sizeof(struct agm_key_value));
    prop = calloc(1, prop_size);
    if (!gkv || !ckv || !prop) {
        if (ckv)
            free(ckv);
        if (gkv)
            free(gkv);
        free(metadata);
        return -ENOMEM;
    }

    if (d == PLAYBACK) {
        gkv[0].key = DEVICERX;
        gkv[0].value = val;
    } else if (d == CAPTURE){
        gkv[0].key = DEVICETX;
        gkv[0].value = val;
    } else {
        gkv[0].key = DEVICERX;
        gkv[0].value = val;
    }

    ckv[ckv_index].key = SAMPLINGRATE;
    ckv[ckv_index].value = rate;

    ckv_index++;
    ckv[ckv_index].key = BITWIDTH;
    ckv[ckv_index].value = bitwidth;

    prop->prop_id = 0;  //Update prop_id here
    prop->num_values = num_props;

    DALSYS_memcpy(metadata, &num_gkv, sizeof(num_gkv));
    offset += sizeof(num_gkv);
    DALSYS_memcpy(metadata + offset, gkv, gkv_size);
    offset += gkv_size;
    DALSYS_memcpy(metadata + offset, &num_ckv, sizeof(num_ckv));
    offset += sizeof(num_ckv);
    DALSYS_memcpy(metadata + offset, ckv, ckv_size);
    offset += ckv_size;
    DALSYS_memcpy(metadata + offset, prop, prop_size);

    ctl_len = strlen(intf_name) + 1 + strlen(control) + 1;
    mixer_str = calloc(1, ctl_len);
    if (!mixer_str) {
        free(metadata);
        return -ENOMEM;
    }
    snprintf(mixer_str, ctl_len, "%s %s", intf_name, control);

    DALSYS_Log_Info("%s - mixer -%s-\n", __func__, mixer_str);
    ctl = mixer_get_ctl_by_name(mixer, mixer_str);
    if (!ctl) {
        DALSYS_Log_Err("Invalid mixer control: %s\n", mixer_str);
        free(gkv);
        free(ckv);
        free(prop);
        free(metadata);
        free(mixer_str);
        return ENOENT;
    }

    ret = mixer_ctl_set_array(ctl, metadata, sizeof(num_gkv) + sizeof(num_ckv) + gkv_size + ckv_size + prop_size);

    free(gkv);
    free(ckv);
    free(prop);
    free(metadata);
    free(mixer_str);
    return ret;
}

int clear_be_metadata(struct mixer *mixer, char *intf_name)
{
    uint32_t md[METADATA_SIZE];
    char *control = "metadata";
    uint32_t ctl_len = 0;
    char *mixer_str = NULL;
    struct mixer_ctl *ctl;

    if (!intf_name || !mixer)
        return 0;

    memset(md, 0, METADATA_SIZE * sizeof(uint32_t));

    ctl_len = strlen(intf_name) + 4 + strlen(control) + 1;
    mixer_str = calloc(1, ctl_len);
    if (!mixer_str) {
        return -ENOMEM;
    }
    snprintf(mixer_str, ctl_len, "%s %s", intf_name, control);
    ctl = mixer_get_ctl_by_name(mixer, mixer_str);
    if (!ctl) {
        DALSYS_Log_Err("Invalid mixer control: %s\n", mixer_str);
        free(mixer_str);
        return -EINVAL;
    }
    mixer_ctl_set_array(ctl, (uint8_t *)md, METADATA_SIZE * sizeof(uint32_t));

    free(mixer_str);

    return 0;
}

int clear_fe_metadata(struct mixer *mixer, int device)
{
    uint32_t md[METADATA_SIZE];
    char *stream = "PCM";
    char *control = "metadata";
    uint32_t ctl_len = 0;
    char *mixer_str = NULL;
    struct mixer_ctl *ctl;

    if (!mixer)
        return 0;

    memset(md, 0, METADATA_SIZE * sizeof(uint32_t));

    ctl_len = strlen(stream) + 4 + strlen(control) + 1;
    mixer_str = calloc(1, ctl_len);
    if (!mixer_str) {
        return -ENOMEM;
    }
    snprintf(mixer_str, ctl_len, "%s%d %s", stream, device, control);
    ctl = mixer_get_ctl_by_name(mixer, mixer_str);
    if (!ctl) {
        DALSYS_Log_Err("Invalid mixer control: %s\n", mixer_str);
        free(mixer_str);
        return -EINVAL;
    }
    mixer_ctl_set_array(ctl, (uint8_t *)md, METADATA_SIZE * sizeof(uint32_t));
    free(mixer_str);
    return 0;
}

int set_stream_metadata_type(struct mixer *mixer, int device, char *val, enum stream_type stype)
{
    char *stream = "PCM";
    char *control = "control";
    char *mixer_str;
    struct mixer_ctl *ctl;
    int ctl_len = 0,ret = 0;

    if (stype == STREAM_COMPRESS)
        stream = "COMPRESS";

    ctl_len = strlen(stream) + 4 + strlen(control) + 1;
    /* ctr_len is steram length + 4 (device id length) + control string length +1
       device_id is 100 for playback and 101 for capture.device id length is 3 digits + null char.*/

    mixer_str = calloc(1, ctl_len);
    if (!mixer_str) {
        DALSYS_Log_Err(LOG_TAG, "%s: memory allocation failed.", __func__);
        return -ENOMEM;
    }
    snprintf(mixer_str, ctl_len, "%s%d %s", stream, device, control);

    DALSYS_Log_Info("%s - mixer -%s-\n", __func__, mixer_str);
    ctl = mixer_get_ctl_by_name(mixer, mixer_str);
    if (!ctl) {
        DALSYS_Log_Err("Invalid mixer control: %s\n", mixer_str);
        free(mixer_str);
        return ENOENT;
    }

    ret = mixer_ctl_set_enum_by_string(ctl, val);
    free(mixer_str);
    return ret;
}

int set_stream_metadata(struct mixer *mixer, int device, uint32_t val, enum stream_type stype, char *intf_name, char *gkv_value)
{
    char *stream = "PCM";
    char *control = "metadata";
    char *mixer_str;
    struct mixer_ctl *ctl;
    uint8_t *metadata = NULL;
    struct agm_key_value *gkv = NULL, *ckv = NULL;
    struct prop_data *prop = NULL;
    uint32_t num_gkv = 1, num_ckv = 1, num_props = 0;
    uint32_t gkv_size, ckv_size, prop_size, index = 0, val_gkv = 0, val_gkv_sink = 0, val_gkv_source = 0;
    int ctl_len = 0, ret = 0, offset = 0, gkv_count = 0;
    char *type = "ZERO";
    char *split_gkv[30];
    char *temp, *rest;
    const char s[2] = "-";

    char *ptr = strtok_r(gkv_value, s, &rest);
    split_gkv[0] = ptr;
    while(ptr != NULL) {
          ptr = strtok_r(NULL, s, &rest);
          gkv_count++;
          split_gkv[gkv_count] = ptr;
    }

    if (!strncmp(split_gkv[0], "PCM_LL_PLAYBACK", strlen(split_gkv[0])+1))
       val = PCM_LL_PLAYBACK;
    else if (!strncmp(split_gkv[0], "COMPRESSED_OFFLOAD_PLAYBACK", strlen(split_gkv[0])+1))
       val = COMPRESSED_OFFLOAD_PLAYBACK;
    else if (!strncmp(split_gkv[0], "HFP_RX_PLAYBACK", strlen(split_gkv[0])+1))
       val = HFP_RX_PLAYBACK;
    else if (!strncmp(split_gkv[0], "HFP_TX_PLAYBACK", strlen(split_gkv[0])+1))
       val = HFP_TX_PLAYBACK;
    else if (!strncmp(split_gkv[0], "PCM_OFFLOAD", strlen(split_gkv[0])+1))
       val = PCM_OFFLOAD_PLAYBACK;
    else if (!strncmp(split_gkv[0], "PCM_RX_LOOPBACK", strlen(split_gkv[0])+1))
       val = PCM_RX_LOOPBACK;
    else if (!strncmp(split_gkv[0], "PCM_RECORD", strlen(split_gkv[0])+1))
       val = PCM_RECORD;
    else
       DALSYS_Log_Err("%s:%d Invalid gkv from test_config file\n", __func__, __LINE__);


    if ( val == PCM_LL_PLAYBACK )
    {
        if (!strncmp(split_gkv[2], "INSTANCE1", strlen(split_gkv[2])+1))
            val_gkv = INSTANCE_1;
         else if (!strncmp(split_gkv[2], "INSTANCE2", strlen(split_gkv[2])+1))
            val_gkv = INSTANCE_2;
         else if (!strncmp(split_gkv[2], "INSTANCE3", strlen(split_gkv[2])+1))
            val_gkv = INSTANCE_3;
         else if (!strncmp(split_gkv[2], "INSTANCE4", strlen(split_gkv[2])+1))
            val_gkv = INSTANCE_4;
         else if (!strncmp(split_gkv[2], "INSTANCE5", strlen(split_gkv[2])+1))
            val_gkv = INSTANCE_5;
         else if (!strncmp(split_gkv[2], "INSTANCE6", strlen(split_gkv[2])+1))
            val_gkv = INSTANCE_6;
         else if (!strncmp(split_gkv[2], "INSTANCE7", strlen(split_gkv[2])+1))
            val_gkv = INSTANCE_7;
         else if (!strncmp(split_gkv[2], "INSTANCE8", strlen(split_gkv[2])+1))
            val_gkv = INSTANCE_8;
        else
            DALSYS_Log_Err("%s:%d Invalid gkv from test_config file\n", __func__, __LINE__);
    }

    if( val == PCM_LL_PLAYBACK )
    {
        if (!strncmp(split_gkv[3], "DEVICEPP_RX_DEFAULT", strlen(split_gkv[3])+1))
            val_gkv_sink = DEVICEPP_RX_DEFAULT;
        else if (!strncmp(split_gkv[3], "AUDIO_MBDRC", strlen(split_gkv[3])+1))
            val_gkv_sink = DEVICEPP_RX_AUDIO_MBDRC;
        else if (!strncmp(split_gkv[3], "VOIP_MBDRC", strlen(split_gkv[3])+1))
            val_gkv_sink = DEVICEPP_RX_VOIP_MBDRC;
        else if (!strncmp(split_gkv[3], "HFP_SINK", strlen(split_gkv[3])+1))
            val_gkv_sink = DEVICEPP_RX_HFPSINK;
        else
            DALSYS_Log_Err("%s:%d Invalid gkv from test_config file\n", __func__, __LINE__);
    }

    if( val == PCM_RECORD)
    {
        if (!strncmp(split_gkv[2],"FLUENCE_FFECNS", strlen(split_gkv[2])+1))
            val_gkv_source = DEVICEPP_TX_FLUENCE_FFECNS;
        else if (!strncmp(split_gkv[2],"AUDIO_FLUENCE_SMECNS", strlen(split_gkv[2])+1))
            val_gkv_source = DEVICEPP_TX_AUDIO_FLUENCE_SMECNS;
        else if (!strncmp(split_gkv[2],"AUDIO_FLUENCE_ENDFIRE", strlen(split_gkv[2])+1))
            val_gkv_source = DEVICEPP_TX_AUDIO_FLUENCE_ENDFIRE;
        else if (!strncmp(split_gkv[2],"AUDIO_FLUENCE_PRO", strlen(split_gkv[2])+1))
            val_gkv_source = DEVICEPP_TX_AUDIO_FLUENCE_PRO;
        else if (!strncmp(split_gkv[2],"VOIP_FLUENCE_PRO", strlen(split_gkv[2])+1))
            val_gkv_source = DEVICEPP_TX_VOIP_FLUENCE_PRO;
        else if (!strncmp(split_gkv[2],"HFP_SINK_FLUENCE_SMECNS", strlen(split_gkv[2])+1))
            val_gkv_source = DEVICEPP_TX_HFP_SINK_FLUENCE_SMECNS;
        else if (!strncmp(split_gkv[2],"VOIP_FLUENCE_SMECNS", strlen(split_gkv[2])+1))
            val_gkv_source = DEVICEPP_TX_VOIP_FLUENCE_SMECNS;
        else
            DALSYS_Log_Err("%s:%d Invalid gkv from test_config file\n", __func__, __LINE__);

        if (!(split_gkv[3] == '\0')) {
            if (!strncmp(split_gkv[3], "INSTANCE1", strlen(split_gkv[3]) + 1))
                val_gkv = INSTANCE_1;
            else if (!strncmp(split_gkv[3], "INSTANCE2", strlen(split_gkv[3]) + 1))
                val_gkv = INSTANCE_2;
            else if (!strncmp(split_gkv[3], "INSTANCE3", strlen(split_gkv[3]) + 1))
                val_gkv = INSTANCE_3;
            else if (!strncmp(split_gkv[3], "INSTANCE4", strlen(split_gkv[3]) + 1))
                val_gkv = INSTANCE_4;
            else if (!strncmp(split_gkv[3], "INSTANCE5", strlen(split_gkv[3]) + 1))
                val_gkv = INSTANCE_5;
            else if (!strncmp(split_gkv[3], "INSTANCE6", strlen(split_gkv[3]) + 1))
                val_gkv = INSTANCE_6;
            else if (!strncmp(split_gkv[3], "INSTANCE7", strlen(split_gkv[3]) + 1))
                val_gkv = INSTANCE_7;
            else if (!strncmp(split_gkv[3], "INSTANCE8", strlen(split_gkv[3]) + 1))
                val_gkv = INSTANCE_8;
            else
                DALSYS_Log_Err("%s:%d Invalid gkv from test_config file\n", __func__, __LINE__);
       }
    }
    if (intf_name)
        type = intf_name;

    ret = set_stream_metadata_type(mixer, device, type, stype);
    if (ret)
        return ret;

    if (stype == STREAM_COMPRESS)
        stream = "COMPRESS";

    if (val == PCM_LL_PLAYBACK || val == COMPRESSED_OFFLOAD_PLAYBACK)
        num_gkv = 3;

    if (val == VOICE_UI && intf_name)
        num_gkv = 2;

    if (val == PCM_RECORD) {
        if (!(split_gkv[3] == '\0')) {
            num_gkv = 3;
        } else {
            num_gkv = 2;
        }
    }

    if (val == PCM_RX_LOOPBACK)
        num_gkv = 1;

    gkv_size = num_gkv * sizeof(struct agm_key_value);
    ckv_size = num_ckv * sizeof(struct agm_key_value);
    prop_size = sizeof(struct prop_data) + (num_props * sizeof(uint32_t));

    metadata = calloc(1, sizeof(num_gkv) + sizeof(num_ckv) + gkv_size + ckv_size + prop_size);
    if (!metadata)
        return -ENOMEM;

    gkv = calloc(num_gkv, sizeof(struct agm_key_value));
    ckv = calloc(num_ckv, sizeof(struct agm_key_value));
    prop = calloc(1, prop_size);
    if (!gkv || !ckv || !prop) {
        if (ckv)
            free(ckv);
        if (gkv)
            free(gkv);
        free(metadata);
        return -ENOMEM;
    }

    if (val == PCM_LL_PLAYBACK)
    {
        gkv[index].key = STREAMRX;
        gkv[index].value = val;
    }
    else if (val == PCM_RECORD)
    {
        gkv[index].key = DEVICEPP_TX;
        gkv[index].value = val_gkv_source;
        index++;
        gkv[index].key = STREAMTX;
        gkv[index].value = val;
        if (!(split_gkv[3] == '\0')) {
            index++;
            gkv[index].key = INSTANCE;
            gkv[index].value = val_gkv;
        }
    }
    if (val == PCM_RX_LOOPBACK)
    {
        gkv[index].key = STREAMRX;
        gkv[index].value = val;
    }

    index++;
    if (val == PCM_LL_PLAYBACK || val == COMPRESSED_OFFLOAD_PLAYBACK) {
        gkv[index].key = INSTANCE;
        gkv[index].value = val_gkv;
        index++;
        gkv[index].key = DEVICEPP_RX;
        gkv[index].value = val_gkv_sink;
    }

    if (val == VOICE_UI && intf_name) {
        gkv[index].key = DEVICEPP_TX;
        gkv[index].value = DEVICEPP_TX_FLUENCE_FFECNS;
    }

    index = 0;
    if (val == PCM_LL_PLAYBACK)
    {
        ckv[index].key = STREAMRX;
        ckv[index].value = val;
    }
    else if (val == PCM_RECORD)
    {
        ckv[index].key = STREAMTX;
        ckv[index].value = val;
    }
    else if (val == PCM_RX_LOOPBACK)
    {
        ckv[index].key = VOLUME;
        ckv[index].value = LEVEL_3;
    }

    prop->prop_id = 0;  //Update prop_id here
    prop->num_values = num_props;

    DALSYS_memcpy(metadata, &num_gkv, sizeof(num_gkv));
    offset += sizeof(num_gkv);
    DALSYS_memcpy(metadata + offset, gkv, gkv_size);
    offset += gkv_size;
    DALSYS_memcpy(metadata + offset, &num_ckv, sizeof(num_ckv));

    offset += sizeof(num_ckv);
    DALSYS_memcpy(metadata + offset, ckv, ckv_size);
    offset += ckv_size;
    DALSYS_memcpy(metadata + offset, prop, prop_size);

    ctl_len = strlen(stream) + 4 + strlen(control) + 1;
    mixer_str = calloc(1, ctl_len);
    if (!mixer_str) {
        free(metadata);
        return -ENOMEM;
    }
    snprintf(mixer_str, ctl_len, "%s%d %s", stream, device, control);

    DALSYS_Log_Info("%s - mixer -%s-\n", __func__, mixer_str);
    ctl = mixer_get_ctl_by_name(mixer, mixer_str);
    if (!ctl) {
        DALSYS_Log_Err("Invalid mixer control: %s\n", mixer_str);
        free(gkv);
        free(ckv);
        free(prop);
        free(metadata);
        free(mixer_str);
        return ENOENT;
    }

    ret = mixer_ctl_set_array(ctl, metadata, sizeof(num_gkv) + sizeof(num_ckv) + gkv_size + ckv_size + prop_size);

    free(gkv);
    free(ckv);
    free(prop);
    free(metadata);
    free(mixer_str);
    return ret;
}

int connect_play_pcm_to_cap_pcm(struct mixer *mixer, signed int p_device, unsigned int c_device)
{
    char *pcm = "PCM";
    char *control = "loopback";
    char *mixer_str;
    struct mixer_ctl *ctl;
    int ctl_len = 0;
    char *val;
    int val_len = 0;
    int ret = 0;

    ctl_len = strlen(pcm) + 4 + strlen(control) + 1;
    mixer_str = calloc(1, ctl_len);
    if (!mixer_str) {
        DALSYS_Log_Err(LOG_TAG, "%s: memory allocation failed.", __func__);
        return -ENOMEM;
    }
    snprintf(mixer_str, ctl_len, "%s%d %s", pcm, c_device, control);

    printf("%s - mixer -%s-\n", __func__, mixer_str);
    ctl = mixer_get_ctl_by_name(mixer, mixer_str);
    if (!ctl) {
        DALSYS_Log_Err("Invalid mixer control: %s\n", mixer_str);
        free(mixer_str);
        return ENOENT;
    }

    if (p_device < 0) {
        val = "ZERO";
    } else {
        val_len = strlen(pcm) + 4;
        val = calloc(1, val_len);

        if (!val) {
            DALSYS_Log_Err(LOG_TAG, "%s: memory allocation failed.", __func__);
            free(mixer_str);
            return -ENOMEM;
        }

        snprintf(val, val_len, "%s%d", pcm, p_device);
    }

    ret = mixer_ctl_set_enum_by_string(ctl, val);
    free(mixer_str);
    if (p_device >= 0)
        free(val);

    return ret;
}

int connect_audio_intf_to_stream(struct mixer *mixer, unsigned int device, char *intf_name, enum stream_type stype, bool connect)
{
    char *stream = "PCM";
    char *control;
    char *mixer_str;
    struct mixer_ctl *ctl;
    int ctl_len = 0;
    int ret = 0;

    if (connect)
        control = "connect";
    else
        control = "disconnect";

    if (stype == STREAM_COMPRESS)
        stream = "COMPRESS";

    ctl_len = strlen(stream) + 4 + strlen(control) + 1;
    /* ctr_len is steram length + 4 (device id length) + control string length +1
       device_id is 100 for playback and 101 for capture.device id length is 3 digits + null char.*/
    mixer_str = calloc(1, ctl_len);
    if (!mixer_str) {
        DALSYS_Log_Err(LOG_TAG, "%s: memory allocation failed.", __func__);
        return -ENOMEM;
    }
    snprintf(mixer_str, ctl_len, "%s%d %s", stream, device, control);

    DALSYS_Log_Info("%s - mixer -%s-\n", __func__, mixer_str);
    ctl = mixer_get_ctl_by_name(mixer, mixer_str);
    if (!ctl) {
        DALSYS_Log_Err("Invalid mixer control: %s\n", mixer_str);
        free(mixer_str);
        return ENOENT;
    }

    ret = mixer_ctl_set_enum_by_string(ctl, intf_name);
    free(mixer_str);
    return ret;
}

size_t get_input_buffer_size(uint32_t sample_rate,
                                    int channel_count)
{
    size_t size = 0;

    size = (sample_rate * AUDIO_CAPTURE_PERIOD_DURATION_MSEC) / 1000;
    /* ToDo: should use frame_size computed based on the format and
       channel_count here. */
    size *= sizeof(short) * channel_count;

    /* make sure the size is multiple of 64 */
    size += 0x3f;
    size &= ~0x3f;

    return size;
}

static int play_loopback()
{
	struct pcm *pcm, *pcm_cap;
	unsigned flags;
	int cap_device_num = 0;
	size_t bufsize;

    flags = PCM_OUT;
    g_loopback_run = 1;
	// We should get the sub-device dynamically from /proc/asound/pcm.
#ifndef ANDROID
#ifndef TINYALSA_LIB
	pcm = pcm_open(flags, "hw:0,4");
#else
	int device_num = 0;
	if (g_playback_device >= 0)
		device_num = g_playback_device;
	pcm = pcm_open(SOUND_CARD_NUM, device_num, flags, &pcm_config_low_latency);
#endif
#else
	int device_num = 0;
	if (g_playback_device >= 0)
	device_num = 103;

	pcm = pcm_open(SOUND_CARD_NUM, device_num, flags, &pcm_config_low_latency);
#endif

	if (!pcm_is_ready(pcm)) {
		DALSYS_Log_Err("pcm_ready() failed\n");
		return -EBADFD;
	}
	cap_device_num = 104;
	cp_config.rate = 48000;
	cp_config.channels = 2;
	bufsize = get_input_buffer_size(cp_config.rate,cp_config.channels);
	cp_config.period_size = bufsize/(cp_config.channels * 2);
	pcm_cap = pcm_open(SOUND_CARD_NUM, cap_device_num, PCM_IN, &cp_config);
	if (!pcm_is_ready(pcm_cap)) {
		DALSYS_Log_Err("pcm_ready() failed\n");
		return -EINVAL;
	}
	if (pcm_start(pcm)) {
		DALSYS_Log_Err("pcm_start failed\n");
        pcm_close(pcm_cap);
        pcm_close(pcm);
		return -EINVAL;
	}
	printf("%d, \n", g_loopback_run);
	if (pcm_start(pcm_cap)) {
		DALSYS_Log_Err("pcm_prepare(cap) failed\n");
		pcm_close(pcm_cap);
		pcm_close(pcm);
		return -EINVAL;
	}
	while(g_loopback_run);
	pcm_close(pcm_cap);
	pcm_close(pcm);

	return 0;
}

static int play_ext_loopback(void* ctxt)
{
    struct pcm *pcm;
    unsigned flags;
    ext_loop_param *play_param = (ext_loop_param *)ctxt;
    int tone_freq = play_param->freq;
    flags = PCM_OUT;

    pcm_config_low_latency.rate = 8000;
    pcm_config_low_latency.channels = 1;

    int device_num = 100;
    pcm = pcm_open(SOUND_CARD_NUM, device_num, flags, &pcm_config_low_latency);

    if (!pcm_is_ready(pcm)) {
        DALSYS_Log_Err("pcm_ready() failed\n");
        return -EBADFD;
    }
    if (pcm_start(pcm)) {
        DALSYS_Log_Err("pcm_start failed\n");
        pcm_close(pcm);
        return -EINVAL;
    }

    unsigned int size;
    char *buffer;
    size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));

    if(DAL_SUCCESS != DALSYS_Malloc(size *sizeof(char), (void **)&buffer))
    {
        DALSYS_Log_Err("%s: failed %d\n",__func__,__LINE__);
        DALSYS_Free(buffer);
        pcm_close(pcm);
        return AUDIO_FTM_ERR_MEM_ALLOC_FAIL;
    }

    static DTMFGenStruct dtmfGen;
    unsigned int i = 0;
    int16 sample = 0;
    audio_ftm_dtmf_tone_init(&dtmfGen, tone_freq, tone_freq, pcm_config_low_latency.rate);

    while(g_loopback_run)
    {
        if(FFT_COMPLETE == g_analyze_status)
        {
            /* For future multiple-frequency test */
            g_freq_tested++;

            /* stop test if all test cases are done */
            if(g_freq_tested == g_total_freq_to_test)
                break;
        }
        else
        {
            for(i = 0; i < size; i=i+2)
            {
                sample = audio_ftm_dtmf_tone_sample_gen(&dtmfGen, TONE_VOLUME);
                buffer[i] = sample&0xff;
                buffer[i+1] = sample>>8;
            }

            if (pcm_write(pcm, buffer, size))
            {
                DALSYS_Log_Err("Error playing sample\n");
                break;
            }
        }
    }

    DALSYS_Free(buffer);
    pcm_close(pcm);
    return 0;
}

static int rec_loopback()
{
	struct pcm *pcm;
	unsigned flags;
	size_t bufsize;

	g_loopback_run = 1;
	flags = PCM_IN;

	// We should get the sub-device dynamically from /proc/asound/pcm.
#ifndef ANDROID
#ifndef TINYALSA_LIB
	pcm = pcm_open(flags, "hw:0,4");
#else
	int device_num = 0;
	if (g_capture_device >= 0)
		device_num = g_capture_device;

    bufsize = get_input_buffer_size(cp_config.rate,cp_config.channels);
    memset(&cp_config, 0, sizeof(cp_config));
    cp_config.channels = 2;
    cp_config.rate = 48000;
    cp_config.period_size = 1024;
    cp_config.period_count = 4;
    cp_config.format = PCM_FORMAT_S16_LE;
    cp_config.start_threshold = 0;
    cp_config.stop_threshold = INT_MAX;
    cp_config.silence_threshold = 0;


	pcm = pcm_open(SOUND_CARD_NUM, device_num, flags, &cp_config);
#endif
#else
	int device_num = 0;
	if (g_capture_device >= 0)
            device_num = 104;
	cp_config.rate = 48000;
	cp_config.channels = 2;
	bufsize = get_input_buffer_size(cp_config.rate,cp_config.channels);
	cp_config.period_size = bufsize/(cp_config.channels * 2);

	pcm = pcm_open(SOUND_CARD_NUM, device_num, flags, &cp_config);
#endif

	if (!pcm_is_ready(pcm)) {
		DALSYS_Log_Err("pcm_ready() failed\n");
		return -EBADFD;
	}

	if (pcm_start(pcm)) {
		DALSYS_Log_Err("pcm_prepare failed\n");
		pcm_close(pcm);
		return -EINVAL;
	}
	while(g_loopback_run);

	pcm_close(pcm);

	return 0;
}

static int rec_ext_loopback(void* ctxt)
{
    struct pcm *pcm;
    unsigned flags;
    size_t bufsize;
    ext_loop_param *rec_param = (ext_loop_param *)ctxt;
    const char *rec_filename = rec_param->rec_filename;
    int enable_tone_analysis = rec_param->tone_analysis;
    int freq_tolerance = rec_param->freq_tolerance;
    flags = PCM_IN;

    /* We should get the sub-device dynamically from /proc/asound/pcm.*/
#ifndef ANDROID
#ifndef TINYALSA_LIB
    pcm_capture_file = pcm_open(flags, "hw:0,4");
    DALSYS_Log_Err("Recording does not support non-Android device currently.\n");
    pcm_close(pcm);
#else
    int device_num = 0;
    if (g_capture_device >= 0)
        device_num = g_capture_device;
    cp_config.rate = 48000;
    cp_config.channels = 1;

    if( 0 == enable_tone_analysis)
    {
        /* tone analysis is disabled. record signal to file */
        audio_ftm_hw_rec_wav(device_num, 48000, 1, rec_filename, &g_loopback_run);
        return 0;
    }

    bufsize = get_input_buffer_size(cp_config.rate,cp_config.channels);
    cp_config.period_size = bufsize/(cp_config.channels * 2);

    pcm = pcm_open(SOUND_CARD_NUM, device_num, flags, &cp_config);
#endif
#else
    int device_num = 101;

    memset(&cp_config, 0, sizeof(cp_config));
    cp_config.channels = 1;
    cp_config.rate = 48000;
    cp_config.period_size = 1024;
    cp_config.period_count = 4;
    cp_config.format = 0;
    cp_config.start_threshold = 0;
    cp_config.stop_threshold = INT_MAX;
    cp_config.silence_threshold = 0;


    if( 0 == enable_tone_analysis)
    {
        /* tone analysis is disabled. record signal to file */
        audio_ftm_hw_rec_wav(device_num, 48000, 1,
            rec_filename, (uint32 *)&g_loopback_run);
        return 0;
    }

    bufsize = get_input_buffer_size(cp_config.rate,cp_config.channels);
    cp_config.period_size = bufsize/(cp_config.channels * 2);

    pcm = pcm_open(SOUND_CARD_NUM, device_num, flags, &cp_config);
#endif

    if (!pcm_is_ready(pcm)) {
        DALSYS_Log_Err("pcm_ready() failed\n");
        return -EBADFD;
    }
    if (pcm_start(pcm)) {
        DALSYS_Log_Err("pcm_prepare failed\n");
        pcm_close(pcm);
        return -EINVAL;
    }

    unsigned int size = 0;
    char *buffer;
    size = bufsize;
    unsigned int frames = pcm_bytes_to_frames(pcm, size);
    unsigned int count = 0;

    if(DAL_SUCCESS != DALSYS_Malloc(size *sizeof(char), (void **)&buffer))
    {
        DALSYS_Log_Err("%s: failed %d\n",__func__,__LINE__);
        pcm_close(pcm);
        return AUDIO_FTM_ERR_MEM_ALLOC_FAIL;
    }

    short int curValue = 0;
    unsigned int pos = 0, freq = 0;
    unsigned int power = 0;
    uint64_t tmp_power = 0;
    /* measure frequency by FFT for last block */
    complex_for_fft *buffer_fft;
    /* ftm pcm_read has 960 frames. for fft, we do 512-point FFT. */
    frames = N_FFT;
    /* For 16-bit mono, 2 Left frames have 2-byte gap. */
    unsigned int gap = 2;
    unsigned int num_of_capture = 0;
    unsigned int capture_power = 0;
    float dbfs_power = 0;

    if(DAL_SUCCESS != DALSYS_Malloc(frames *sizeof(complex_for_fft), (void **)&buffer_fft))
    {
        DALSYS_Log_Err("%s: failed %d\n",__func__,__LINE__);
        DALSYS_Free(buffer);
        pcm_close(pcm);
        return AUDIO_FTM_ERR_MEM_ALLOC_FAIL;
    }

    /* ensure stability */
    //sleep(1);
    while (g_loopback_run&& !pcm_read(pcm, buffer, size)
            && num_of_capture < g_total_freq_to_test)
    {
        if(g_analyze_status >= FFT_IDLE && g_analyze_status < FFT_START)
        {
            g_analyze_status++;
        }
        else if(FFT_START == g_analyze_status)
        {
            for(count = 0; count < frames; count++)
            {
                curValue= 0;
                curValue|= buffer[count*gap + 1]<<8;
                curValue|= buffer[count*gap];
                buffer_fft[count].real = (float)curValue;
                buffer_fft[count].imag = 0;
                tmp_power += curValue * curValue;
            }

            capture_power = (uint32_t) tmp_power;
            float_fft(buffer_fft, frames, FFT_STAGE);
            pos = 0;
            int ignore_DC_value = 1;
            power = 0;
            pos = ignore_DC_value +
                  get_max_pos(buffer_fft+ignore_DC_value, frames/2 - 1);
            /*
            * calculate rms^2 value
            *  dBFS = 20*log10(rms/32768)
            * = 10*log10(rms^2 / 32768^2)
            */
            capture_power = capture_power / frames;
            dbfs_power = (float)capture_power/32768/32768;
            freq = (int)pos * cp_config.rate / frames;
            g_cap_freq = freq;
            g_analyze_status = FFT_COMPLETE;
            num_of_capture++;
        }
    }

    DALSYS_Free(buffer);
    DALSYS_Free(buffer_fft);
    pcm_close(pcm);

    if( freq_tolerance <= 0)
    {
        DALSYS_Log_Err("\n Invalid Frequency Tolerance %d. It must be > 0.", freq_tolerance);
        freq_tolerance = TOLERANCE;
        DALSYS_Log_Err("Use default value %d\n", freq_tolerance);
    }

    float freq_diff;

    DALSYS_Log_Info("\nPlay:\t%d\tHz", rec_param->freq);
    DALSYS_Log_Info("\nCap:\t%d\tHz", g_cap_freq);
    DALSYS_Log_Info("\nPower:\t");
    if(0 == dbfs_power)
        DALSYS_Log_Err("power is 0. dBFS power cannot be calculated.\n");
    else
        DALSYS_Log_Err("%.2f\tdBFS", 10*log10(dbfs_power));

    freq_diff = fabs(1 - (float)g_cap_freq/(rec_param->freq));
    DALSYS_Log_Err("\nDiff:\t%.1f%%\t", freq_diff*100);

    if(freq_diff*100 < freq_tolerance && freq_diff*100 >-freq_tolerance)
    {
        DALSYS_Log_Err("\nExternal Loopback Test Pass\n\n");
        return 0;
    }

    DALSYS_Log_Err("\nExternal Loopback Test Fail\n\n");
    return -1;
}

static int play_fm()
{
	struct pcm *pcm;
	unsigned flags;
	int device_num = 5;

	if (fm_playback >= 0)
		device_num = fm_playback;
	flags = PCM_OUT;

	// We should get the sub-device dynamically from /proc/asound/pcm.
	pcm = pcm_open(SOUND_CARD_NUM, device_num, flags, &pcm_config_fm);

	if (!pcm_is_ready(pcm)) {
		DALSYS_Log_Err("pcm_ready() failed\n");
		return -EBADFD;
	}

	if (pcm_start(pcm)) {
		DALSYS_Log_Err("pcm_prepare failed\n");
		pcm_close(pcm);
		return -EINVAL;
	}

	while(g_fm_run);

	pcm_close(pcm);

	return 0;
}

static int rec_fm()
{
	struct pcm *pcm;
	unsigned flags;
	int device_num = 6;
	size_t bufsize;
	if (fm_capture >= 0)
		device_num = fm_capture;
	g_fm_run = 1;
	flags = PCM_IN;

	// We should get the sub-device dynamically from /proc/asound/pcm.
	pcm = pcm_open(SOUND_CARD_NUM, device_num, flags, &pcm_config_fm);

	if (!pcm_is_ready(pcm)) {
		DALSYS_Log_Err("pcm_ready() failed\n");
		return -EBADFD;
	}

	if (pcm_start(pcm)) {
		DALSYS_Log_Err("pcm_prepare failed\n");
		pcm_close(pcm);
		return -EINVAL;
	}

	while(g_fm_run);

	pcm_close(pcm);

	return 0;
}

int audio_ftm_hw_pcm_in_out_control_disable(struct mixer *mxr, Aud_FTM_DevCtxt_T  *pDevCtxt)
{
    if (!mxr)
        return 0;

    if (pDevCtxt->read_write_flag == PCM_OUT) {
        printf("%s: rx devid: %d, rx be_name: %s\n",
                __func__, pDevCtxt->device_id, pDevCtxt->be_name);
        set_stream_metadata_type(mxr, pDevCtxt->device_id, "ZERO", STREAM_PCM);
        clear_fe_metadata(mxr, pDevCtxt->device_id);
        clear_be_metadata(mxr,  pDevCtxt->be_name);
        set_stream_metadata_type(mxr, pDevCtxt->device_id, pDevCtxt->be_name, STREAM_PCM);
        clear_fe_metadata(mxr, pDevCtxt->device_id);
    } else if (pDevCtxt->read_write_flag == PCM_IN) {
        printf("%s: tx_dev_id: %d tx_be_name: %s\n",
                __func__, pDevCtxt->device_id, pDevCtxt->be_name);
        set_stream_metadata_type(mxr, pDevCtxt->device_id, "ZERO", STREAM_PCM);
        clear_fe_metadata(mxr, pDevCtxt->device_id);
        clear_be_metadata(mxr,  pDevCtxt->be_name);
        set_stream_metadata_type(mxr, pDevCtxt->device_id, pDevCtxt->be_name, STREAM_PCM);
        clear_fe_metadata(mxr, pDevCtxt->device_id);
    }

    return 0;
}

int audio_ftm_hw_loopback_control_disable(struct mixer *mxr, Aud_FTM_DevCtxt_T  *pDevCtxt)
{
    if (!mxr)
        return 0;

    printf("%s: rx devid: %d, rx be_name: %s tx_dev_id: %d tx_be_name: %s\n",
            __func__, pDevCtxt->device_id_rx, pDevCtxt->be_name_rx,
            pDevCtxt->device_id_tx, pDevCtxt->be_name_tx);

    if (pDevCtxt->m_loopback_type == AUDIO_FTM_AFE_LOOPBACK) {
        /* Set loopback to ZERO */
        connect_play_pcm_to_cap_pcm(mxr, -1, pDevCtxt->device_id_tx);
    }
    /* TX Metadata clear and Disconnect FE to BE */
    set_stream_metadata_type(mxr, pDevCtxt->device_id_tx, "ZERO", STREAM_PCM);
    clear_fe_metadata(mxr, pDevCtxt->device_id_tx);
    clear_be_metadata(mxr,  pDevCtxt->be_name_tx);
    set_stream_metadata_type(mxr, pDevCtxt->device_id_tx, pDevCtxt->be_name_tx, STREAM_PCM);
    clear_fe_metadata(mxr, pDevCtxt->device_id_tx);
    /* RX Metadata clear and Disconnect FE to BE */
    set_stream_metadata_type(mxr, pDevCtxt->device_id_rx, "ZERO", STREAM_PCM);
    clear_fe_metadata(mxr, pDevCtxt->device_id_rx);
    clear_be_metadata(mxr,  pDevCtxt->be_name_rx);
    set_stream_metadata_type(mxr, pDevCtxt->device_id_rx, pDevCtxt->be_name_rx, STREAM_PCM);
    clear_fe_metadata(mxr, pDevCtxt->device_id_rx);

    return 0;
}

int audio_ftm_hw_loopback_en(int enable, int afe)
{
	struct mixer *m = 0;
	struct mixer_ctl *ctl = 0;
	int ret = 0;
	pthread_t lb_play_thread;
	pthread_t lb_rec_thread;

	DALSYS_Log_Info("%s(%d.%d)\n",__func__,enable,afe);

	if (enable && afe) {
		g_loopback_run = 1;
		ret = pthread_create(&lb_play_thread, NULL,
            (void *(* _Nonnull)(void *))play_loopback, NULL);
		if (ret != 0)
		{
			DALSYS_Log_Err("Failed to create lb play thread\n");
			return AUDIO_FTM_ERROR;
		}
	}

	if (!enable)
	 g_loopback_run = 0;

	return ret;
}

int audio_ftm_ext_loopback_en(int enable, int afe, int freq, const char *rec_filename,
                                        int tone_analysis, int freq_tolerance)
{
    struct mixer *m = 0;
    struct mixer_ctl *ctl = 0;
    int ret = 0;
    pthread_t lb_play_thread;
    pthread_t lb_rec_thread;
    ext_lp_param.freq = freq;
    ext_lp_param.rec_filename = rec_filename;
    ext_lp_param.tone_analysis = tone_analysis;
    ext_lp_param.freq_tolerance = freq_tolerance;

    DALSYS_Log_Info("%s(%d.%d)\n",__func__,enable,afe);

    if (enable && afe) {
        g_loopback_run = 1;

        ret = pthread_create(&lb_play_thread, NULL,
            (void *(* _Nonnull)(void *))play_ext_loopback, (void *)&ext_lp_param);
        if (ret != 0)
        {
            DALSYS_Log_Err("Failed to create lb play thread\n");
            return AUDIO_FTM_ERROR;
        }
        ret = pthread_create(&lb_rec_thread, NULL,
            (void *(* _Nonnull)(void *))rec_ext_loopback, (void *)&ext_lp_param);
        if (ret != 0)
        {
            DALSYS_Log_Err("Failed to create lb rec thread\n");
            return AUDIO_FTM_ERROR;
        }
    }

    if (!enable)
        g_loopback_run = 0;

        return ret;
}

void audio_ftm_fm_device_set(int capture , int playback)
{
	fm_capture = capture;
	fm_playback = playback;
}
int audio_ftm_fm_hostless_en(int enable)
{
	struct mixer *m = 0;
	struct mixer_ctl *ctl = 0;
	int ret = 0;
	pthread_t fm_play_thread;
	pthread_t fm_rec_thread;

	DALSYS_Log_Info("%s(%d)\n",__func__,enable);

	if (enable) {
		g_fm_run = 1;

		ret = pthread_create(&fm_play_thread, NULL,
            (void *(* _Nonnull)(void *))play_fm, NULL);
		if (ret != 0)
		{
			DALSYS_Log_Err("Failed to create FM play thread\n");
			return AUDIO_FTM_ERROR;
		}
		ret = pthread_create(&fm_rec_thread, NULL,
            (void *(* _Nonnull)(void *))rec_fm, NULL);
		if (ret != 0)
		{
			DALSYS_Log_Err("Failed to create FM rec thread\n");
			return AUDIO_FTM_ERROR;
		}
	}

	if (!enable)
		g_fm_run = 0;

	return ret;
}

AUDIO_FTM_STS_T aud_ftm_hw_init
(
    Aud_FTM_DevCtxt_T  *pCtxt,
    Aud_FTM_HW_INIT_PARAM_T *pInitParam
)
{
	AUDIO_FTM_STS_T res;
	static DALBOOL bIsDBInitialized;
	char *pDevName;

	bIsDBInitialized = FALSE;
	res= AUDIO_FTM_SUCCESS;

	g_bDriverInitialized=TRUE;

	pCtxt->sampleRate=pInitParam->rate;
	pCtxt->bitWidth=pInitParam->width;
	pCtxt->numChannels = 1;
	if (pInitParam->channel == 2) {
		pCtxt->numChannels=pInitParam->channel;
	}
	pCtxt->period_size=0;
	pCtxt->gain=pInitParam->gain;
	pCtxt->duration=pInitParam->duration;
	pCtxt->rx_buf_size=0;
	pCtxt->tx_buf_size=0;
	pCtxt->bLoopbackCase=pInitParam->bLoopbackCase;
	pCtxt->m_loopback_type=pInitParam->m_loopback_type;
	if(pCtxt->bLoopbackCase == TRUE)
	{
		if( (pCtxt->rx_dev_id < 0) || (pCtxt->tx_dev_id < 0) )
			return AUDIO_FTM_ERROR;
	}

	if(pCtxt->tx_dev_id < 0)
		return AUDIO_FTM_ERROR;

	if(pCtxt->rx_dev_id < 0)
		return AUDIO_FTM_ERROR;

	pCtxt->m_state=AUDIO_FTM_HW_DRV_INITIALIZED;

	return AUDIO_FTM_SUCCESS;
}

struct wav_header {
	uint32 riff_id;
	uint32 riff_sz;
	uint32 riff_fmt;
	uint32 fmt_id;
	uint32 fmt_sz;
	uint16 audio_format;
	uint16 num_channels;
	uint32 sample_rate;
	uint32 byte_rate;	  /* sample_rate * num_channels * bps / 8 */
	uint16 block_align;	  /* num_channels * bps / 8 */
	uint16 bits_per_sample;
	uint32 data_id;
	uint32 data_sz;
};

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164
static struct wav_header hdr;
static int fd;

int record_file(unsigned rate, unsigned channels, int fd, unsigned flags,
		int device, unsigned *running)
{
	unsigned avail, xfer, bufsize;
	int r;
	int nfds = 1;
	static int start = 0;
	struct snd_xferi x;
	long frames;
	unsigned offset = 0;
	struct pcm *pcm;
	char *data;
        int device_cap = 101;
        bufsize = 3840;

        memset(&cp_config, 0, sizeof(cp_config));
        cp_config.channels = channels;
        cp_config.rate = rate;
        cp_config.period_size = 1920;
        cp_config.period_count = 4;
        cp_config.format = 0;
        cp_config.start_threshold = 0;
        cp_config.stop_threshold = INT_MAX;
        cp_config.silence_threshold = 0;

     pcm = pcm_open(SOUND_CARD_NUM, device_cap, PCM_IN, &cp_config);

    if (!pcm) {
        DALSYS_Log_Err("Unable to open PCM device (%s)\n",
                pcm_get_error(pcm));
        goto fail;
    }

        if (!pcm_is_ready(pcm)) {
		goto fail;
	}

	if (pcm_start(pcm)) {
		fprintf(stderr, "failed in pcm_prepare\n");
		pcm_close(pcm);
		return -1;
	}
	data = calloc(1, bufsize);
	if (!data) {
		fprintf(stderr, "could not allocate %d bytes\n", bufsize);
		pcm_close(pcm);
		return -ENOMEM;
	}

	while (!pcm_read(pcm, data, bufsize)) {
		if (write(fd, data, bufsize) !=  (ssize_t)bufsize) {
			fprintf(stderr, "could not write %d bytes\n", bufsize);
			close(fd);
			free(data);
			pcm_close(pcm);
			return -1;
		}
		hdr.data_sz += bufsize;
		//printf("*running = %d\n", *running);
		if (*running == 0) {
			DALSYS_Log_Info("done\n");
			break;
		}
	}

	sleep(1);
	hdr.riff_sz = hdr.data_sz + hdr.riff_sz;
	lseek(fd, 0, SEEK_SET);
	write(fd, &hdr, sizeof(hdr));
	close(fd);
	free(data);
	pcm_close(pcm);
	return hdr.data_sz;
fail:
	fprintf(stderr, "pcm error: %s\n", pcm_get_error(pcm));
	return -1;
}

int audio_ftm_hw_rec_wav(int device, int rate, int ch, const char *fn, unsigned *running)
{
	unsigned flag = 0;
	int i = 0;
	fd = open(fn, O_WRONLY | O_CREAT | O_TRUNC, 0664);
	if (fd < 0) {
		fprintf(stderr, "cannot open '%s'\n", fn);
		return -EBADFD;
	}
	memset(&hdr, 0, sizeof(struct wav_header));
	hdr.riff_id = ID_RIFF;
	hdr.riff_fmt = ID_WAVE;
	hdr.fmt_id = ID_FMT;
	hdr.fmt_sz = 16;
	hdr.audio_format = 1;
	hdr.num_channels = ch;
	hdr.sample_rate = rate;
	hdr.bits_per_sample = 16;
	hdr.byte_rate = (rate * ch * hdr.bits_per_sample) / 8;
	hdr.block_align = ( hdr.bits_per_sample * ch ) / 8;
	hdr.data_id = ID_DATA;
	hdr.data_sz = 0;
	hdr.riff_sz = 44 - 8;

	if (write(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
		fprintf(stderr, "cannot write header\n");
		return -1;
	}
	flag = 0;
	return record_file(hdr.sample_rate, hdr.num_channels, fd, flag, device, running);
}

int set_agm_stream_metadata_type(struct mixer *mixer, int device, char *val, enum stream_type stype)
{
    char *stream = "PCM";
    char *control = "control";
    char *mixer_str;
    struct mixer_ctl *ctl;
    int ctl_len = 0,ret = 0;

    if (stype == STREAM_COMPRESS)
        stream = "COMPRESS";

    ctl_len = strlen(stream) + 4 + strlen(control) + 1;
    mixer_str = calloc(1, ctl_len);

    if (!mixer_str) {
        DALSYS_Log_Err(LOG_TAG, "%s: memory allocation failed.", __func__);
        return -ENOMEM;
    }

    snprintf(mixer_str, ctl_len, "%s%d %s", stream, device, control);

    ctl = mixer_get_ctl_by_name(mixer, mixer_str);
    if (!ctl) {
        printf("Invalid mixer control: %s\n", mixer_str);
        free(mixer_str);
        return ENOENT;
    }

    ret = mixer_ctl_set_enum_by_string(ctl, val);
    free(mixer_str);
    return ret;
}

int agm_mixer_get_miid(struct mixer *mixer, int device, char *intf_name,
                       enum stream_type stype, int tag_id, uint32_t *miid)
{
    char *stream = "PCM";
    char *control = "getTaggedInfo";
    char *mixer_str;
    struct mixer_ctl *ctl;
    int ctl_len = 0,ret = 0, i;
    void *payload;
    struct gsl_tag_module_info *tag_info;
    struct gsl_tag_module_info_entry *tag_entry;
    int offset = 0;

    ret = set_agm_stream_metadata_type(mixer, device, intf_name, stype);
    if (ret)
        return ret;

    if (stype == STREAM_COMPRESS)
        stream = "COMPRESS";

    ctl_len = strlen(stream) + 4 + strlen(control) + 1;
    mixer_str = calloc(1, ctl_len);
    if (!mixer_str)
        return -ENOMEM;

    snprintf(mixer_str, ctl_len, "%s%d %s", stream, device, control);

    ctl = mixer_get_ctl_by_name(mixer, mixer_str);
    if (!ctl) {
        printf("Invalid mixer control: %s\n", mixer_str);
        free(mixer_str);
        return ENOENT;
    }

    payload = calloc(1024, sizeof(char));
    if (!payload) {
        free(mixer_str);
        return -ENOMEM;
    }

    ret = mixer_ctl_get_array(ctl, payload, 1024);
    if (ret < 0) {
        printf("Failed to mixer_ctl_get_array\n");
        free(payload);
        free(mixer_str);
        return ret;
    }
    tag_info = payload;
    ret = -1;
    tag_entry = &tag_info->tag_module_entry[0];
    offset = 0;
    for (i = 0; i < tag_info->num_tags; i++) {
        tag_entry += offset/sizeof(struct gsl_tag_module_info_entry);

        offset = sizeof(struct gsl_tag_module_info_entry) + (tag_entry->num_modules * sizeof(struct gsl_module_id_info_entry));
        if (tag_entry->tag_id == tag_id) {
            struct gsl_module_id_info_entry *mod_info_entry;

            if (tag_entry->num_modules) {
                 mod_info_entry = &tag_entry->module_entry[0];
                 *miid = mod_info_entry->module_iid;
                 ret = 0;
                 break;
            }
        }
    }

    free(payload);
    free(mixer_str);
    return ret;
}

int agm_mixer_set_param(struct mixer *mixer, int device,
                        enum stream_type stype, void *payload, int size)
{
    char *stream = "PCM";
    char *control = "setParam";
    char *mixer_str;
    struct mixer_ctl *ctl;
    int ctl_len = 0,ret = 0;

    if (stype == STREAM_COMPRESS)
        stream = "COMPRESS";


    ctl_len = strlen(stream) + 4 + strlen(control) + 1;
    mixer_str = calloc(1, ctl_len);
    if (!mixer_str) {
        free(payload);
        return -ENOMEM;
    }
    snprintf(mixer_str, ctl_len, "%s%d %s", stream, device, control);

    ctl = mixer_get_ctl_by_name(mixer, mixer_str);
    if (!ctl) {
        printf("Invalid mixer control: %s\n", mixer_str);
        free(mixer_str);
        return ENOENT;
    }


    ret = mixer_ctl_set_array(ctl, payload, size);

    free(mixer_str);
    return ret;
}

void populateChannelMap(uint16_t *pcmChannel, uint8_t numChannel)
{
    if (numChannel == 1) {
        pcmChannel[0] = PCM_CHANNEL_C;
    } else if (numChannel == 2) {
        pcmChannel[0] = PCM_CHANNEL_L;
        pcmChannel[1] = PCM_CHANNEL_R;
    } else if (numChannel == 3) {
        pcmChannel[0] = PCM_CHANNEL_L;
        pcmChannel[1] = PCM_CHANNEL_R;
        pcmChannel[2] = PCM_CHANNEL_C;
    } else if (numChannel == 4) {
        pcmChannel[0] = PCM_CHANNEL_L;
        pcmChannel[1] = PCM_CHANNEL_R;
        pcmChannel[2] = PCM_CHANNEL_LB;
        pcmChannel[3] = PCM_CHANNEL_RB;
    }
}

int configure_mfc(struct mixer *mixer, int device, char *intf_name, int tag,
                enum stream_type stype, unsigned int rate,
                unsigned int channels, unsigned int bits)
{
    int ret = 0;
    uint32_t miid = 0;
    struct apm_module_param_data_t* header = NULL;
    struct param_id_mfc_output_media_fmt_t *mfcConf;
    uint16_t* pcmChannel = NULL;
    uint8_t* payloadInfo = NULL;
    size_t payloadSize = 0, padBytes = 0, size;

    ret = agm_mixer_get_miid(mixer, device, intf_name, stype, tag, &miid);
    if (ret) {
        printf("%s Get MIID from tag data failed\n", __func__);
        return ret;
    }

    payloadSize = sizeof(struct apm_module_param_data_t) +
                sizeof(struct param_id_mfc_output_media_fmt_t) +
                sizeof(uint16_t)*channels;

    padBytes = PADDING_8BYTE_ALIGN(payloadSize);

    payloadInfo = (uint8_t*) calloc(1, payloadSize + padBytes);
    if (!payloadInfo) {
        return -ENOMEM;
    }

    header = (struct apm_module_param_data_t*)payloadInfo;
    mfcConf = (struct param_id_mfc_output_media_fmt_t*)(payloadInfo +
                 sizeof(struct apm_module_param_data_t));
    pcmChannel = (uint16_t*)(payloadInfo + sizeof(struct apm_module_param_data_t) +
                                     sizeof(struct param_id_mfc_output_media_fmt_t));

    header->module_instance_id = miid;
    header->param_id = PARAM_ID_MFC_OUTPUT_MEDIA_FORMAT;
    header->error_code = 0x0;
    header->param_size = payloadSize - sizeof(struct apm_module_param_data_t);

    mfcConf->sampling_rate = rate;
    mfcConf->bit_width = bits;
    mfcConf->num_channels = channels;
    populateChannelMap(pcmChannel, channels);
    size = payloadSize + padBytes;

    ret = agm_mixer_set_param(mixer, device, stype, (void *)payloadInfo, (int)size);
    printf("%s MFC config for miid:%x pcm dev:%d, backend:%s ret:%d\n",
           __func__, miid, device, intf_name, ret);
    return ret;
}

AUDIO_FTM_STS_T
audio_ftm_hw_open(Aud_FTM_DevCtxt_T  *pDevCtxt)
{
    AUDIO_FTM_STS_T  ret;
    int dev_fd;
    uint16  session_id;
    unsigned int period_size = 1024;
    unsigned int period_count = 4;
    struct mixer *mxr;
    char intf_name[] = "SLIM-DEV1-RX-0";
    char intf_name_one[] = "SLIM-DEV1-TX-0";
    unsigned int rate = 48000;
    unsigned int bits = 16;
    unsigned int channels = 1;
    unsigned int device = 100;
    unsigned int device_cap = 101;
    const char s[2] = "-";
    char *split_gkv[30];
    char gkv_dup[100];
    char gkv_rx_dup[100];
    char gkv_tx_dup[100];
    char *temp;

    mxr = mixer_open(SOUND_CARD_NUM);
    if (!mxr) {
        DALSYS_Log_Err("\nOpening mixer control failed");
        return -1;
    }
    if( pDevCtxt->m_state != AUDIO_FTM_HW_DRV_INITIALIZED)
    {
        DALSYS_Log_Err("Fail: Open must be run after initialization only\n");
        return AUDIO_FTM_ERROR;
    }

    if ((pDevCtxt->read_write_flag == PCM_OUT) && (pDevCtxt->bLoopbackCase != TRUE))
    {
        unsigned flags = pDevCtxt->read_write_flag;
        pb_config.rate = pDevCtxt->sampleRate;
        pb_config.channels = pDevCtxt->numChannels;
        pb_config.period_size = period_size;
        pb_config.period_count = period_count;
        pb_config.format = 0;
        pb_config.start_threshold = 0;
        pb_config.stop_threshold = INT_MAX;
        pb_config.silence_threshold = 0;
        pDevCtxt->playbackdevice = 100;
        int snd_card = SOUND_CARD_NUM;
        uint32_t val_usecase = 0;
        char gkv_use[100];

        if (pDevCtxt->period_size)
            pb_config.period_size = pDevCtxt->period_size;

        DALSYS_Log_Info("%s: flags %d rate %d channels %d period size %d playbackdevice: %d", __func__, flags, pb_config.rate,
        pb_config.channels, pb_config.period_size, pDevCtxt->playbackdevice);

        if (pDevCtxt->gkv == NULL || pDevCtxt->be_name == NULL)
        return -EINVAL;

        strlcpy(gkv_dup, pDevCtxt->gkv, strlen(pDevCtxt->gkv)+1);

        if (set_device_media_config(mxr, pDevCtxt->channels, rate, bits, pDevCtxt->be_name)) {
            DALSYS_Log_Err("Failed to set device media config\n");
            goto err_close_mixer;
        }

        /* set audio interface metadata mxr control */
        if (set_audio_intf_metadata(mxr, pDevCtxt->be_name, PLAYBACK, rate, bits, gkv_dup)) {
            DALSYS_Log_Err("Failed to set device metadata\n");
            goto err_close_mixer;
        }

        /* set audio interface metadata mixer control */
        if (set_stream_metadata(mxr, pDevCtxt->device_id, val_usecase, STREAM_PCM, NULL, pDevCtxt->gkv)) {
            DALSYS_Log_Err("Failed to set pcm metadata\n");
            goto err_close_mixer;
        }

        /* connect pcm stream to audio intf */
        if (connect_audio_intf_to_stream(mxr, pDevCtxt->device_id, pDevCtxt->be_name, STREAM_PCM, true)) {
            DALSYS_Log_Err("Failed to connect pcm to audio interface\n");
            goto err_close_mixer;
        }

        if (pDevCtxt->playbackdevice < 0) {
            pDevCtxt->pcm = pcm_open(snd_card, 0, flags, &pb_config);
        } else {
            pDevCtxt->pcm = pcm_open(snd_card, pDevCtxt->device_id, PCM_OUT, &pb_config);
        }
        if (pDevCtxt->pcm < 0) {
            DALSYS_Log_Err("\n %s: error opening device %s", __func__, strerror(errno));
            return AUDIO_FTM_ERROR;
        }
        g_pcm = pDevCtxt->pcm;

        if (!pcm_is_ready(pDevCtxt->pcm)) {
            DALSYS_Log_Err("pcm_ready(0x%p) failed\n",pDevCtxt->pcm);
            return AUDIO_FTM_ERROR;
        }

        configure_mfc(mxr, pDevCtxt->device_id, pDevCtxt->be_name, PER_STREAM_PER_DEVICE_MFC, STREAM_PCM, rate, pDevCtxt->channels, bits);

        pDevCtxt->rx_buf_size = pcm_get_buffer_size(pDevCtxt->pcm)/pb_config.period_count;

        if (pcm_start(pDevCtxt->pcm)) {
            DALSYS_Log_Err("pcm_prepare failed\n");
            pcm_close(pDevCtxt->pcm);
            return AUDIO_FTM_ERROR;
        }
    }

    if ((pDevCtxt->read_write_flag == PCM_IN) && (pDevCtxt->bLoopbackCase != TRUE))
    {
        if (pDevCtxt->gkv == NULL || pDevCtxt->be_name == NULL)
        return -EINVAL;

        strlcpy(gkv_dup, pDevCtxt->gkv, strlen(pDevCtxt->gkv)+1);

        unsigned flags = PCM_IN;

        if (set_device_media_config(mxr, pDevCtxt->channels, rate, bits, pDevCtxt->be_name)) {
            DALSYS_Log_Err("Failed to set device media config\n");
            goto err_close_mixer;
        }

        /* set audio interface metadata mxr control */

        if (set_audio_intf_metadata(mxr, pDevCtxt->be_name, CAPTURE, rate, bits, pDevCtxt->gkv)) {
            DALSYS_Log_Err("Failed to set device metadata\n");
            goto err_close_mixer;
        }

        /* set audio interface metadata mixer control */

        if (set_stream_metadata(mxr, pDevCtxt->device_id, PCM_RECORD, STREAM_PCM, NULL, gkv_dup)) {
            DALSYS_Log_Err("Failed to set pcm metadata\n");
            goto err_close_mixer;
        }

        /* connect pcm stream to audio intf */

        if (connect_audio_intf_to_stream(mxr, pDevCtxt->device_id, pDevCtxt->be_name, STREAM_PCM, true)) {
            DALSYS_Log_Err("Failed to connect pcm to audio interface\n");
            goto err_close_mixer;
        }

    }

    if(pDevCtxt->m_loopback_type == AUDIO_FTM_ADIE_LOOPBACK)
    {
        pDevCtxt->m_state=AUDIO_FTM_HW_DRV_OPENED;
        return AUDIO_FTM_SUCCESS;
    }

    if(pDevCtxt->m_loopback_type == AUDIO_FTM_EXT_LOOPBACK)
    {
        device = 100;
        device_cap = 101;
        unsigned int rate = 48000;
        unsigned int bits = 16;
        unsigned int channels = 1;
        unsigned int channels_cap = 1;

        if (pDevCtxt->gkv_rx == NULL || pDevCtxt->be_name_rx == NULL)
        return -EINVAL;

        if (pDevCtxt->gkv_tx == NULL || pDevCtxt->be_name_tx == NULL)
        return -EINVAL;

        strlcpy(gkv_rx_dup, pDevCtxt->gkv_rx, strlen(pDevCtxt->gkv_rx)+1);
        strlcpy(gkv_tx_dup, pDevCtxt->gkv_tx, strlen(pDevCtxt->gkv_tx)+1);

        pDevCtxt->device_id_rx = 100;
        pDevCtxt->device_id_tx = 101;


        if((pDevCtxt->tx_dev_id == -ENODEV) || (pDevCtxt->rx_dev_id == -ENODEV))
        {
            DALSYS_Log_Err("At least one device ID is invalid for loopback\n");
            return AUDIO_FTM_ERROR;
        }

        if (set_device_media_config(mxr, pDevCtxt->channels_rx, rate, bits, pDevCtxt->be_name_rx)) {
            DALSYS_Log_Err("Failed to set device media config\n");
            goto err_close_mixer;
        }

        if (set_device_media_config(mxr, pDevCtxt->channels_tx, rate, bits, pDevCtxt->be_name_tx)) {
            DALSYS_Log_Err("Failed to set device media config\n");
            goto err_close_mixer;
        }

        if (set_audio_intf_metadata(mxr, pDevCtxt->be_name_tx, CAPTURE, rate, bits, pDevCtxt->gkv_tx)) {
            DALSYS_Log_Err("Failed to set device metadata\n");
            goto err_close_mixer;
        }

        /* set audio interface metadata mxr control */
        if (set_audio_intf_metadata(mxr, pDevCtxt->be_name_rx, PLAYBACK, rate, bits, pDevCtxt->gkv_rx)) {
            DALSYS_Log_Err("Failed to set device metadata\n");
            goto err_close_mixer;
       }
        /* set audio interface metadata mixer control */
        if (set_stream_metadata(mxr, pDevCtxt->device_id_rx, PCM_LL_PLAYBACK, STREAM_PCM, NULL, gkv_rx_dup)) {
            DALSYS_Log_Err("Failed to set pcm metadata\n");
            goto err_close_mixer;
        }

        /* connect pcm stream to audio intf */
        if (connect_audio_intf_to_stream(mxr, pDevCtxt->device_id_rx, pDevCtxt->be_name_rx, STREAM_PCM, true)) {
            DALSYS_Log_Err("Failed to connect pcm to audio interface\n");
            goto err_close_mixer;
        }

        /* set audio interface metadata mixer control */

        if (set_stream_metadata(mxr, pDevCtxt->device_id_tx, PCM_RECORD, STREAM_PCM, NULL, gkv_tx_dup)) {
            DALSYS_Log_Err("Failed to set pcm metadata\n");
            goto err_close_mixer;
       }

        /* connect pcm stream to audio intf */

        if (connect_audio_intf_to_stream(mxr, pDevCtxt->device_id_tx, pDevCtxt->be_name_tx, STREAM_PCM, true)) {
            DALSYS_Log_Err("Failed to connect pcm to audio interface\n");
            goto err_close_mixer;
        }
        configure_mfc(mxr, pDevCtxt->device_id_rx, pDevCtxt->be_name_rx, PER_STREAM_PER_DEVICE_MFC, STREAM_PCM, 48000, pDevCtxt->channels_rx, 16);
    }

    if(pDevCtxt->m_loopback_type == AUDIO_FTM_AFE_LOOPBACK)
    {
        device = 100;
        device_cap = 101;
        unsigned int rate = 48000;
        unsigned int bits = 16;
        unsigned int channels = 2;
        unsigned int channels_cap = 1;

        if (pDevCtxt->gkv_rx == NULL || pDevCtxt->be_name_rx == NULL)
        return -EINVAL;

        if (pDevCtxt->gkv_tx == NULL || pDevCtxt->be_name_tx == NULL)
        return -EINVAL;

        strlcpy(gkv_rx_dup, pDevCtxt->gkv_rx, strlen(pDevCtxt->gkv_rx)+1);
        strlcpy(gkv_tx_dup, pDevCtxt->gkv_tx, strlen(pDevCtxt->gkv_tx)+1);

        pDevCtxt->device_id_rx = 103;
        pDevCtxt->device_id_tx = 104;

        if((pDevCtxt->tx_dev_id == -ENODEV) || (pDevCtxt->rx_dev_id == -ENODEV))
        {
            DALSYS_Log_Err("At least one device ID is invalid for loopback\n");
            return AUDIO_FTM_ERROR;
        }

        if (set_device_media_config(mxr, pDevCtxt->channels_rx, rate, bits, pDevCtxt->be_name_rx)) {
            DALSYS_Log_Err("Failed to set device media config\n");
            goto err_close_mixer;
        }

        if (set_device_media_config(mxr, pDevCtxt->channels_tx, rate, bits, pDevCtxt->be_name_tx)) {
            DALSYS_Log_Err("Failed to set device media config\n");
            goto err_close_mixer;
        }

        if (set_audio_intf_metadata(mxr, pDevCtxt->be_name_tx, CAPTURE, rate, bits, pDevCtxt->gkv_tx)) {
            DALSYS_Log_Err("Failed to set device metadata\n");
            goto err_close_mixer;
        }

        /* set audio interface metadata mxr control */
        if (set_audio_intf_metadata(mxr, pDevCtxt->be_name_rx, PLAYBACK, rate, bits, pDevCtxt->gkv_rx)) {
            DALSYS_Log_Err("Failed to set device metadata\n");
            goto err_close_mixer;
       }
        /* set audio interface metadata mixer control */
        if (set_stream_metadata(mxr, pDevCtxt->device_id_rx, PCM_RX_LOOPBACK, STREAM_PCM, NULL, gkv_rx_dup)) {
            DALSYS_Log_Err("Failed to set pcm metadata\n");
            goto err_close_mixer;
        }

        /* connect pcm stream to audio intf */
        if (connect_audio_intf_to_stream(mxr, pDevCtxt->device_id_rx, pDevCtxt->be_name_rx, STREAM_PCM, true)) {
            DALSYS_Log_Err("Failed to connect playback pcm to audio interface\n");
            goto err_close_mixer;
        }

        if (connect_audio_intf_to_stream(mxr, pDevCtxt->device_id_tx, pDevCtxt->be_name_tx, STREAM_PCM, true)) {
            DALSYS_Log_Err("Failed to connect capture pcm to audio interface\n");
            connect_audio_intf_to_stream(mxr, pDevCtxt->device_id_rx, pDevCtxt->be_name_rx, STREAM_PCM, false);
            goto err_close_mixer;
        }

        if (connect_play_pcm_to_cap_pcm(mxr, pDevCtxt->device_id_rx, pDevCtxt->device_id_tx)) {
            DALSYS_Log_Err("Failed to connect capture pcm to audio interface\n");
            goto err_close_mixer;
        }

        configure_mfc(mxr, pDevCtxt->device_id_rx, pDevCtxt->be_name_rx,
            PER_STREAM_PER_DEVICE_MFC, STREAM_PCM, rate,
            pDevCtxt->channels_rx, pDevCtxt->bitWidth);
    }
    pDevCtxt->m_state=AUDIO_FTM_HW_DRV_OPENED;
    //mixer_close(mxr);
    return AUDIO_FTM_SUCCESS;
fail :
    mixer_close(mxr);
err_close_mixer:
    mixer_close(mxr);
    return AUDIO_FTM_ERROR;
}

AUDIO_FTM_STS_T
audio_ftm_hw_close(Aud_FTM_DevCtxt_T  *pDevCtxt)
{
	AUDIO_FTM_STS_T  ret;
    struct mixer *mxr;

	ret = AUDIO_FTM_SUCCESS;

	if(pDevCtxt == NULL)  return AUDIO_FTM_ERR_INVALID_PARAM;

    mxr = mixer_open(SOUND_CARD_NUM);
    if ((pDevCtxt->m_loopback_type == AUDIO_FTM_AFE_LOOPBACK) ||
            (pDevCtxt->m_loopback_type == AUDIO_FTM_EXT_LOOPBACK))
        audio_ftm_hw_loopback_control_disable(mxr, pDevCtxt);

    if (((pDevCtxt->read_write_flag == PCM_OUT) ||
           (pDevCtxt->read_write_flag == PCM_IN)) &&
            (pDevCtxt->bLoopbackCase != TRUE))
        audio_ftm_hw_pcm_in_out_control_disable(mxr, pDevCtxt);

    mixer_close(mxr);
	pDevCtxt->m_state=AUDIO_FTM_HW_DRV_CLOSED;

	return ret;
}

AUDIO_FTM_STS_T
audio_ftm_hw_write(
    Aud_FTM_DevCtxt_T  *pDevCtxt,     /* Input: handle to hw driver */
    void *pBuf,                       /* Input: buffer pointer containing data for writing */
    uint32 nSamples                   /* Input: Samples */
)
{
	uint32  len;

	len = nSamples*(pDevCtxt->bitWidth/AUDIO_FTM_BIT_WIDTH_8) *
		(pDevCtxt->numChannels);
	int ret = 0;
	ret = pcm_write(g_pcm, pBuf, len);
	if (ret) {
	//if (pcm_write(pDevCtxt->pcm, pBuf, len)){
		DALSYS_Log_Err("\npcm_write() error\n %d error %s", ret, strerror(errno));
		return AUDIO_FTM_ERROR;
	}

	return AUDIO_FTM_SUCCESS;
}

AUDIO_FTM_STS_T
audio_ftm_hw_iocontrol(
    Aud_FTM_DevCtxt_T  *pDevCtxt,     /* Input: handle to hw driver */
    uint32 dwIOCode,                  /* Input: IOControl command */
    uint8 * pBufIn,                   /* Input: buffer pointer for input parameters */
    uint8 * pBufOut                   /* Output: buffer pointer for outputs */)
{
	struct mixer *mixer = 0;
	struct mixer_ctl *ctl = 0;
	AUDIO_FTM_STS_T   ret=AUDIO_FTM_SUCCESS;

	if((pDevCtxt->m_state == AUDIO_FTM_HW_DRV_OPENED) ||
		(pDevCtxt->m_state == AUDIO_FTM_HW_DRV_STARTED) ||
		(pDevCtxt->m_state == AUDIO_FTM_HW_DRV_STOPPED))
	{
		switch(dwIOCode)
		{
		case IOCTL_AUDIO_FTM_START:
		{
			AFEDevAudIfDirType dir;

			if(pBufIn == NULL)
			{
				DALSYS_Log_Err("pBufIn is NULL, IOCTL_AUDIO_FTM_START failed\n");
				ret=AUDIO_FTM_ERROR;
				break;
			}

			dir= *(AFEDevAudIfDirType*)pBufIn;
			g_playback_device = 5;
			g_capture_device = 4;
			pDevCtxt->capturedevice = 101;
			DALSYS_Log_Info("\n %s: capturedevice %d playback %d",
				   __func__, pDevCtxt->capturedevice, pDevCtxt->playbackdevice);
			if((pDevCtxt->bLoopbackCase == TRUE)
				&& (pDevCtxt->tx_dev_id != -ENODEV)
				&& (pDevCtxt->rx_dev_id != -ENODEV))
			{
				if(pDevCtxt->m_loopback_type == AUDIO_FTM_AFE_LOOPBACK)
				{
					audio_ftm_hw_loopback_en(1, 1);
				}
				else if(pDevCtxt->m_loopback_type == AUDIO_FTM_ADIE_LOOPBACK)
				{
					audio_ftm_hw_loopback_en(1, 0);
				}
			} else if (g_curr_rx_device == AUDIO_FTM_TC_RX_FM_STEREO_OUTPUT) {
				audio_ftm_fm_hostless_en(1);
			}

			if(ret == AUDIO_FTM_SUCCESS)
				pDevCtxt->m_state = AUDIO_FTM_HW_DRV_STARTED;
		}
		break;

		case IOCTL_AUDIO_FTM_STOP:
		{
			AFEDevAudIfDirType dir;

			if(pBufIn == NULL)
			{
				DALSYS_Log_Err("pBufIn is NULL, IOCTL_AUDIO_FTM_STOP failed\n");
				ret=AUDIO_FTM_ERROR;
				break;
			}

			if (ftm_tc_devices[g_curr_device].path_type == PATH_RX) {
				DALSYS_Log_Err("\n PCM Close called");
				pcm_close(pDevCtxt->pcm);
			} else {
				DALSYS_Log_Err("\n PCM Close not called");
			}

			dir= *(AFEDevAudIfDirType*)pBufIn;

			if((pDevCtxt->bLoopbackCase == TRUE)
				&& (pDevCtxt->tx_dev_id != -ENODEV)
				&& (pDevCtxt->rx_dev_id != -ENODEV)) {
				if(pDevCtxt->m_loopback_type == AUDIO_FTM_AFE_LOOPBACK)
				{
					audio_ftm_hw_loopback_en(0, 1);
					sleep(1);
				}
				else if(pDevCtxt->m_loopback_type == AUDIO_FTM_ADIE_LOOPBACK)
				{
					audio_ftm_hw_loopback_en(0, 0);
				}
			} else if (g_curr_rx_device == AUDIO_FTM_TC_RX_FM_STEREO_OUTPUT) {
				audio_ftm_fm_hostless_en(0);
			}

			if(ret == AUDIO_FTM_SUCCESS)
				pDevCtxt->m_state = AUDIO_FTM_HW_DRV_STOPPED;
		}
		break;

		case IOCTL_AUDIO_FTM_RX_DEV_BUF_SIZE:
		{
			if(pBufOut == NULL)
			{
				DALSYS_Log_Err("pBufOut is NULL, IOCTL_AUDIO_FTM_RX_DEV_BUF_SIZE failed\n");
				ret=AUDIO_FTM_ERROR;
				break;
			}
			// buffer size will be 8192 bytes
			*((uint32 *)pBufOut) = pDevCtxt->rx_buf_size;
		}
		break;

		case IOCTL_AUDIO_FTM_TX_DEV_BUF_SIZE:
		{
			if(pBufOut == NULL)
			{
				DALSYS_Log_Err("pBufOut is NULL, IOCTL_AUDIO_FTM_TX_DEV_BUF_SIZE failed\n");
				ret=AUDIO_FTM_ERROR;
				break;
			}
			*((uint32 *)pBufOut)=pDevCtxt->tx_buf_size;
		}
		break;

		default:
			DALSYS_Log_Err("this operation is not supportted\n");
			ret= AUDIO_FTM_ERROR;
			break;
		}
	}
	else
	{
		DALSYS_Log_Err("this operation cannot be done when the driver is not in active state\n");
		ret= AUDIO_FTM_ERROR;
	}

	return ret;
}

AUDIO_FTM_STS_T
aud_ftm_hw_deinit(Aud_FTM_DevCtxt_T  *pDevCtxt)
{
	g_bDriverInitialized = FALSE;

	pDevCtxt->m_state=AUDIO_FTM_HW_DRV_UN_INITIALIZED;

	return AUDIO_FTM_SUCCESS;
}

#ifdef __cplusplus
}
#endif

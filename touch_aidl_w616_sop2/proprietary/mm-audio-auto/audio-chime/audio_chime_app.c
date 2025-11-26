/**
@file audio_chime_app.c
@brief Early audio chime application
*/
/*
** Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
** All rights reserved.
** Confidential and Proprietary - Qualcomm Technologies, Inc.
**
** Copyright 2011, The Android Open Source Project
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of The Android Open Source Project nor the names of
**       its contributors may be used to endorse or promote products derived
**       from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY The Android Open Source Project ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL The Android Open Source Project BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
** DAMAGE.
**/

#define LOG_TAG "early-audio-chime"
//#define LOG_NDEBUG 0
//#define LOG_NDDEBUG 0

#include <tinyalsa/asoundlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <log/log.h>
#include <cutils/list.h>
#include <cutils/str_parms.h>
#include <cutils/properties.h>

#define LIB_ACDB_LOADER_CLIENT "libacdbloaderclient.so"
#define LIB_PLUGIN_CLIENT "libaudiohalpluginclient.so"

#define ENABLE_MIXER 1
#define DISABLE_MIXER 0

#define ACDB_DEV_TYPE_OUT 1
/*Following are derived as the default values for the MULTIMEDIA23 FEDAI*/
#define DEFAULT_APP_TYPE_RX_PATH  69943
#define DEFAULT_OUTPUT_SAMPLING_RATE 48000
#define DEFAULT_ACDB_ID 60
#define MULTIMEDIA_FEDAI_ID 22
#define DEFAULT_BIT_WIDTH 16
#define DEFAULT_BACKEND_ID 87

/*Mixer control for FEDAI MULTIMEDIA23
  This needs to be modified by customers if the mixer control
  is different for them*/
#define MIXER_AUDIO_ROUTE "TERT_TDM_RX_0 Audio Mixer MultiMedia23"
#define MIXER_AUDIO_STREAM_APP_TYPE_CONFIG "Audio Stream 55 App Type Cfg"
#define MIXER_TYPE_CONFIG "App Type Config"

#define AUDIO_CHIME_SLEEP(ms) usleep(( uint32_t )ms)
#define AUDIO_CHIME_WAIT_TIME 10   /* in ms */
#define MAX_SLEEP_RETRY 1000

/* bootkpi_writer creates a buffer of 40 char,
   including NUL termination */
#define MARKER_STRING_WIDTH 39

enum {
    ACDB_LOADER_INIT_V1 = 1,
    ACDB_LOADER_INIT_V2,
    ACDB_LOADER_INIT_V3,
    ACDB_LOADER_INIT_V4,
};

enum {
    CAL_MODE_SEND        = 0x1,
    CAL_MODE_PERSIST     = 0x2,
    CAL_MODE_RTAC        = 0x4
};

enum {
    CAL_OFFSET_NONE      = 0x0,
    CAL_OFFSET_ASM_TOP   = 0x1,
    CAL_OFFSET_INDEX_MAX
};

typedef enum snd_card_status_t {
    CARD_STATUS_OFFLINE,
    CARD_STATUS_ONLINE
} snd_card_status_t;

struct acdb_init_data_v4 {
    char                   *cvd_version;
    char                   *snd_card_name;
    struct listnode        *meta_key_list;
    bool                   *is_instance_id_supported;
};

typedef int  (*acdb_init_v4_t)(void *, int);
typedef void (*acdb_send_audio_cal_v4_t)(int, int, int, int, int, int);
typedef void (*acdb_send_audio_cal_v6_t)(int, int, int, int, int, int, int, int);
typedef int (*acdb_send_common_top_t) (void);
typedef void (*acdb_deallocate_cal_t) (int, int);

struct platform_data {
    void                       *acdb_handle;
    acdb_init_v4_t             acdb_init_v4;
    acdb_send_audio_cal_v4_t   acdb_send_audio_cal_v4;
    acdb_send_audio_cal_v6_t   acdb_send_audio_cal_v6;
    acdb_send_common_top_t     acdb_send_common_top;
    acdb_deallocate_cal_t      acdb_deallocate_cal;
    struct acdb_init_data_v4   acdb_init_data;
    struct listnode            acdb_meta_key_list;
};

typedef int32_t (*audio_hal_plugin_init_t) (void);

struct hal_plugin_data {
    void                   *plugin_handle;
    audio_hal_plugin_init_t  plugin_init;
};

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

struct riff_wave_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t wave_id;
};

struct chunk_header {
    uint32_t id;
    uint32_t sz;
};

struct chunk_fmt {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};

struct playback_sample_data {
    unsigned int acdb_id;
    struct platform_data *plt_data;
};

static int play_close = 0;
static bool is_playback_complete = true;
static bool is_acdb_init_done = false;
static bool restartPlayback = false;
static snd_card_status_t card_status;

//START: SND_MONITOR_FEATURE ================================================================
#ifdef __LP64__
#define SND_MONITOR_PATH  "/vendor/lib64/libsndmonitor.so"
#else
#define SND_MONITOR_PATH  "/vendor/lib/libsndmonitor.so"
#endif
static void *snd_mnt_lib_handle = NULL;

static int snd_mon_feature_init (bool is_feature_enabled);
typedef void (* snd_mon_cb)(void * stream, struct str_parms * parms);

static int audio_chime_snd_mon_init();
static int audio_chime_snd_mon_deinit();
static int audio_chime_snd_mon_register_listener(void *stream, snd_mon_cb cb);
static int audio_chime_snd_mon_unregister_listener(void *stream);

typedef int (*snd_mon_init_t)();
static snd_mon_init_t snd_mon_init;
typedef int (*snd_mon_deinit_t)();
static snd_mon_deinit_t snd_mon_deinit;
typedef int (*snd_mon_register_listener_t)(void *, snd_mon_cb);
static snd_mon_register_listener_t snd_mon_register_listener;
typedef int (*snd_mon_unregister_listener_t)(void *);
static snd_mon_unregister_listener_t snd_mon_unregister_listener;

static int snd_mon_feature_init (bool is_feature_enabled)
{
    int ret = -EINVAL;
    ALOGD("%s: Called with feature %s\n", __func__, is_feature_enabled?"Enabled":"NOT Enabled");
    if (is_feature_enabled) {
        //dlopen lib
        snd_mnt_lib_handle = dlopen(SND_MONITOR_PATH, RTLD_NOW);
        if (snd_mnt_lib_handle == NULL) {
            ALOGE("%s: dlopen failed\n", __func__);
            goto feature_disabled;
        }
        //map each function
        //on any faliure to map any function, disble feature
        if (((snd_mon_init = (snd_mon_init_t)dlsym(snd_mnt_lib_handle,"snd_mon_init")) == NULL) ||
            ((snd_mon_deinit = (snd_mon_deinit_t)dlsym(snd_mnt_lib_handle,"snd_mon_deinit")) == NULL) ||
            ((snd_mon_register_listener = (snd_mon_register_listener_t)dlsym(snd_mnt_lib_handle,"snd_mon_register_listener")) == NULL) ||
            ((snd_mon_unregister_listener = (snd_mon_unregister_listener_t)dlsym(snd_mnt_lib_handle,"snd_mon_unregister_listener")) == NULL)) {
            ALOGE("%s: dlsym failed\n", __func__);
            goto feature_disabled;
        }
        ALOGD("%s:: ---- Feature SND_MONITOR is Enabled ----\n", __func__);
        return 0;
    }

feature_disabled:
    if (snd_mnt_lib_handle) {
        dlclose(snd_mnt_lib_handle);
        snd_mnt_lib_handle = NULL;
    }

    snd_mon_init                = NULL;
    snd_mon_deinit              = NULL;
    snd_mon_register_listener   = NULL;
    snd_mon_unregister_listener = NULL;
    ALOGW(":: %s: ---- Feature SND_MONITOR is disabled ----\n", __func__);
    return ret;
}

static int audio_chime_snd_mon_init()
{
    int ret = -EINVAL;
    if (snd_mon_init != NULL)
        ret = snd_mon_init();

    return ret;
}

static int audio_chime_snd_mon_deinit()
{
    int ret = -EINVAL;
    if (snd_mon_deinit != NULL)
        ret = snd_mon_deinit();

    return ret;
}

static int audio_chime_snd_mon_register_listener(void *stream, snd_mon_cb cb)
{
    int ret = -EINVAL;
    if (snd_mon_register_listener != NULL)
        ret = snd_mon_register_listener(stream, cb);

    return ret;
}

static int audio_chime_snd_mon_unregister_listener(void *stream)
{
    int ret = -EINVAL;
    if (snd_mon_unregister_listener != NULL)
        ret = snd_mon_unregister_listener(stream);

    return ret;
}

//END: SND_MONITOR_FEATURE ================================================================

static void chime_snd_mon_cb(void *cookie, struct str_parms *parms);
int play_sample(FILE *file, unsigned int card, unsigned int device, unsigned int channels,
                 unsigned int rate, unsigned int bits, unsigned int period_size,
                 unsigned int period_count);

void stream_close(int sig)
{
    /* allow the stream to be closed gracefully */
    signal(sig, SIG_IGN);
    play_close = 1;
}

int check_param(struct pcm_params *params, unsigned int param, unsigned int value,
                 char *param_name, char *param_unit)
{
    unsigned int min;
    unsigned int max;
    int is_within_bounds = 1;

    min = pcm_params_get_min(params, param);
    if (value < min) {
        ALOGE("%s is %u%s, device only supports >= %u%s\n", param_name, value,
                param_unit, min, param_unit);
        is_within_bounds = 0;
    }

    max = pcm_params_get_max(params, param);
    if (value > max) {
        ALOGE("%s is %u%s, device only supports <= %u%s\n", param_name, value,
                param_unit, max, param_unit);
        is_within_bounds = 0;
    }

    return is_within_bounds;
}

int sample_is_playable(unsigned int card, unsigned int device, unsigned int channels,
                        unsigned int rate, unsigned int bits, unsigned int period_size,
                        unsigned int period_count)
{
    struct pcm_params *params;
    int can_play;

    params = pcm_params_get(card, device, PCM_OUT);
    if (params == NULL) {
        ALOGE("Unable to open PCM device %u.\n", device);
        return 0;
    }

    can_play = check_param(params, PCM_PARAM_RATE, rate, "Sample rate", "Hz");
    can_play &= check_param(params, PCM_PARAM_CHANNELS, channels, "Sample", " channels");
    can_play &= check_param(params, PCM_PARAM_SAMPLE_BITS, bits, "Bitrate", " bits");
    can_play &= check_param(params, PCM_PARAM_PERIOD_SIZE, period_size, "Period size", " frames");
    can_play &= check_param(params, PCM_PARAM_PERIODS, period_count, "Period count", " periods");

    pcm_params_free(params);

    return can_play;
}

void place_marker(char const *name)
{
   int fd=open("/sys/kernel/boot_kpi/kpi_values", O_WRONLY);
   if (fd > 0)
   {
       /* Only allow marker text shorter than MARKER_STRING_WIDTH */
       char earlyapp[MARKER_STRING_WIDTH] = {0};
       strlcpy(earlyapp, name, sizeof(earlyapp));
       write(fd, earlyapp, strlen(earlyapp));
       close(fd);
   }
}

int play_sample(FILE *file, unsigned int card, unsigned int device, unsigned int channels,
                 unsigned int rate, unsigned int bits, unsigned int period_size,
                 unsigned int period_count)
{
    place_marker("M - Audio_Chime writing audio samples");
    struct pcm_config config;
    struct pcm *pcm;
    char *buffer;
    int size;
    int num_read;

    memset(&config, 0, sizeof(config));
    config.channels = channels;
    config.rate = rate;
    config.period_size = period_size;
    config.period_count = period_count;
    if (bits == 32)
        config.format = PCM_FORMAT_S32_LE;
    else if (bits == 24)
        config.format = PCM_FORMAT_S24_3LE;
    else if (bits == 16)
        config.format = PCM_FORMAT_S16_LE;
    config.start_threshold = 0;
    config.stop_threshold = 0;
    config.silence_threshold = 0;

    if (!sample_is_playable(card, device, channels, rate, bits, period_size, period_count)) {
        return -EINVAL;
    }

    pcm = pcm_open(card, device, PCM_OUT, &config);
    if (!pcm) {
        ALOGE("Unable to open PCM device\n");
        return -ENODEV;
    }
    if (!pcm_is_ready(pcm)) {
        ALOGE("Unable to open PCM device %u (%s)\n",
                 device, pcm_get_error(pcm));
        pcm_close(pcm);
        return -ENODEV;
    }

    size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));
    buffer = malloc(size);
    if (!buffer) {
        ALOGE("Unable to allocate %d bytes\n", size);
        free(buffer);
        pcm_close(pcm);
        return -ENOMEM;
    }

    ALOGD("Playing sample: %u ch, %u hz, %u bit\n", channels, rate, bits);

    /* catch ctrl-c to shutdown cleanly */
    signal(SIGINT, stream_close);

    do {
        num_read = fread(buffer, 1, size, file);
        if (num_read > 0) {
            if (pcm_write(pcm, buffer, num_read)) {
                ALOGE("play_sample: pcm_write failed\n");
                fprintf(stderr, "Error playing sample\n");
                is_playback_complete = false;
                free(buffer);
                pcm_close(pcm);
                return -EFAULT;
            }
        }
    } while (!play_close && num_read > 0);

    free(buffer);
    pcm_close(pcm);
    is_playback_complete = true;

    return 0;
}

static int config_audio_route(struct mixer* mixer, int enable)
{
    struct mixer_ctl *ctl;
    char *mixer_str = MIXER_AUDIO_ROUTE;

    if(!mixer)
    {
        ALOGE("%s: Mixer is NULL\n", __func__);
        return -ENODEV;
    }
    ctl = mixer_get_ctl_by_name(mixer, mixer_str);
    if(!ctl) {
        ALOGE("%s: couldn't get ctl for mixer cmd: %s", __func__, mixer_str);
        return -EINVAL;
    }
    mixer_ctl_set_value(ctl, 0, enable);

    return 0;
}

static int config_app_type(struct mixer* mixer)
{
    struct mixer_ctl *ctl;
    char *mixer_str = MIXER_AUDIO_STREAM_APP_TYPE_CONFIG;
    long app_type_cfg[4];
    long app_type_config[4];

    if(!mixer)
    {
        ALOGE("%s: Mixer is NULL\n", __func__);
        return -ENODEV;
    }
    ctl = mixer_get_ctl_by_name(mixer, mixer_str);
    if(!ctl) {
        ALOGE("%s: couldn't get ctl for mixer cmd: %s", __func__, mixer_str);
        return -EINVAL;
    }

    /*Apply the following as default values for FEDAI MULTIMEDIA23*/
    app_type_cfg[0] = DEFAULT_APP_TYPE_RX_PATH;
    app_type_cfg[1] = DEFAULT_ACDB_ID;
    app_type_cfg[2] = DEFAULT_OUTPUT_SAMPLING_RATE;
    app_type_cfg[3] = DEFAULT_BACKEND_ID;
    mixer_ctl_set_array(ctl, app_type_cfg, sizeof(app_type_cfg)/sizeof(app_type_cfg[0]));

    mixer_str = MIXER_TYPE_CONFIG;
    ctl = mixer_get_ctl_by_name(mixer, mixer_str);
    if(!ctl) {
        ALOGE("%s: couldn't get ctl for mixer cmd: %s", __func__, mixer_str);
        return -EINVAL;
    }

    /*Apply the following as default values for the app_type 69936*/
    app_type_config[0] = 1;
    app_type_config[1] = DEFAULT_APP_TYPE_RX_PATH;
    app_type_config[2] = DEFAULT_OUTPUT_SAMPLING_RATE;
    app_type_config[3] = DEFAULT_BIT_WIDTH;

    mixer_ctl_set_array(ctl, app_type_config, sizeof(app_type_config)/sizeof(app_type_config[0]));

    return 0;
}

static int set_mixer_controls(int enable)
{
    struct mixer* mixer = NULL;
    int card = 0;
    unsigned int sleepRetry = 0;
    bool mixerOpenDone = false;
    int rc = 0;

    while((mixerOpenDone == false) && (sleepRetry < MAX_SLEEP_RETRY))
    {
        mixer = mixer_open(card);
        if (!mixer) {
           /*Failed to open mixer, wait 10ms, retry*/
            ALOGE("%s: Failed to open mixer, sleeping for 10ms\n", __func__);
            AUDIO_CHIME_SLEEP(AUDIO_CHIME_WAIT_TIME * 1000);
            sleepRetry++;
        }
        else
            mixerOpenDone = true;
    }

    if(mixerOpenDone == false)
    {
        ALOGE("%s: Mixer open failed\n", __func__);
        return -ENODEV;
    }

    rc = config_audio_route(mixer, enable);
    if (rc) {
        ALOGE("%s: config_audio_route failed\n", __func__);
        mixer_close(mixer);
        return rc;
    }

    if (enable) {
        rc = config_app_type(mixer);
        if (rc) {
            ALOGE("%s: config_app_type failed\n", __func__);
            config_audio_route(mixer, !enable);
            mixer_close(mixer);
            return rc;
        }
    }

    return rc;
}

static int parse_card_status(struct str_parms *parms, int *card, snd_card_status_t *status)
{
      char value[32]={0};
      char state[32]={0};

      int  ret = str_parms_get_str(parms, "SND_CARD_STATUS", value, sizeof(value));
      if (ret < 0)
          return -EINVAL;

      // sscanf should be okay as value is of max length 32.
      // same as sizeof state.
      if (sscanf(value, "%d,%s", card, state) < 2)
          return -EINVAL;

      *status = !strcmp(state, "ONLINE") ? CARD_STATUS_ONLINE :
                                           CARD_STATUS_OFFLINE;
      return 0;
}

static void chime_snd_mon_cb(void *stream, struct str_parms *parms)
{
    int rc;

    if (!stream || !parms)
        return;

    struct playback_sample_data *sample_data = (struct playback_sample_data *)stream;

    snd_card_status_t status;
    int card, path;
    if (parse_card_status(parms, &card, &status) < 0)
        return;

    if(card_status != status)
        card_status = status;
    else
        return;

    if(status == CARD_STATUS_ONLINE) {
        /*Configure mixer*/
        rc = set_mixer_controls(ENABLE_MIXER);
        if(rc != 0) {
            ALOGE("%s: failed to configure mixer post SSR\n", __func__);
            return;
        }
        if(is_acdb_init_done) {
            /*Send common custom topology to DSP post SSR*/
            if(sample_data->plt_data->acdb_send_common_top) {
                if(sample_data->plt_data->acdb_send_common_top() < 0)
                    ALOGE("%s: acdb did not set common topology\n", __func__);
            }

            //consider path as 0 since kernel considers RX as 0
            path = ACDB_DEV_TYPE_OUT-1;

            /*Send audio calibration data again post SSR completion*/
            if(sample_data->plt_data->acdb_send_audio_cal_v6) {
                sample_data->plt_data->acdb_send_audio_cal_v6(sample_data->acdb_id, ACDB_DEV_TYPE_OUT,
                                            DEFAULT_APP_TYPE_RX_PATH, DEFAULT_OUTPUT_SAMPLING_RATE, MULTIMEDIA_FEDAI_ID,
                                            DEFAULT_OUTPUT_SAMPLING_RATE, CAL_MODE_SEND, CAL_OFFSET_ASM_TOP);
            }
            else if(sample_data->plt_data->acdb_send_audio_cal_v4) {
                sample_data->plt_data->acdb_send_audio_cal_v4(sample_data->acdb_id, ACDB_DEV_TYPE_OUT, DEFAULT_APP_TYPE_RX_PATH,
                                                              DEFAULT_OUTPUT_SAMPLING_RATE, path, DEFAULT_OUTPUT_SAMPLING_RATE);
            }
        }
        if(!is_playback_complete) {
            /*Signal main thread to restart the playback*/
            ALOGD("%s: signaling main thread to restart the playback\n", __func__);
            restartPlayback = true;
        }
    }
    return;
}

/****************** audio_chime ***********************/

int main(int  __attribute__((unused))argc, char **argv)
{
    place_marker("M - Starting Audio_Chime App");
    struct platform_data *my_data = NULL;
    struct hal_plugin_data *my_plugin = NULL;
    struct playback_sample_data *sample_data = NULL;
    int card = 0, path;
    FILE *file = NULL;
    struct riff_wave_header riff_wave_header;
    struct chunk_header chunk_header;
    struct chunk_fmt chunk_fmt={
          .num_channels=2,
          .sample_rate=48000,
          .bits_per_sample=16
      };
    unsigned int device = 55;
    unsigned int period_size = 1024;
    unsigned int period_count = 4;
    unsigned int acdb_id = DEFAULT_ACDB_ID;
    const char *filename;
    int more_chunks = 1;
    static bool acdb_instance_id_support = false;
    const char* error;
    int32_t rc = 0;
    unsigned int sleepRetry = 0;


    rc = set_mixer_controls(ENABLE_MIXER);
    if(rc != 0)
        return -ENODEV;

    card_status = CARD_STATUS_ONLINE;

    filename = argv[1];
    ALOGD("file_open start\n");
    file = fopen(filename, "rb");
    if (!file) {
        ALOGE("Unable to open file '%s'\n", filename);
        return -EINVAL;
    } else {
        ALOGD("file_open success\n");
    }

    ALOGD("fread start\n");
    fread(&riff_wave_header, sizeof(riff_wave_header), 1, file);
    if ((riff_wave_header.riff_id != ID_RIFF) ||
        (riff_wave_header.wave_id != ID_WAVE)) {
        ALOGE("Error: '%s' is not a riff/wave file\n", filename);
        fclose(file);
        return -EINVAL;
    }

    do {
        fread(&chunk_header, sizeof(chunk_header), 1, file);

        switch (chunk_header.id) {
        case ID_FMT:
            fread(&chunk_fmt, sizeof(chunk_fmt), 1, file);
            /* If the format header is larger, skip the rest */
            if (chunk_header.sz > sizeof(chunk_fmt))
                fseek(file, chunk_header.sz - sizeof(chunk_fmt), SEEK_CUR);
            break;
        case ID_DATA:
            /* Stop looking for chunks */
            more_chunks = 0;
            break;
        default:
            /* Unknown chunk, skip bytes */
            fseek(file, chunk_header.sz, SEEK_CUR);
        }
    } while (more_chunks);

    /* parse command line arguments */
    ALOGD("parse args start\n");
    argv += 2;
    while (*argv) {
        if (strcmp(*argv, "-d") == 0) {
            argv++;
            if (*argv)
                device = atoi(*argv);
        }
       if(*argv) {
        if (strcmp(*argv, "-p") == 0) {
            argv++;
            if (*argv)
                period_size = atoi(*argv);
           }
       }
       if(*argv) {
        if (strcmp(*argv, "-n") == 0) {
            argv++;
            if (*argv)
                period_count = atoi(*argv);
          }
       }
       if(*argv)  {
        if (strcmp(*argv, "-D") == 0) {
            argv++;
            if (*argv)
                card = atoi(*argv);
          }
        }
        if(*argv) {
        if (strcmp(*argv, "-a") == 0) {
            argv++;
            if (*argv)
                acdb_id = atoi(*argv);
          }
        }
        if (*argv)
            argv++;
    }
    ALOGD("parse args end\n");

    sample_data = calloc(1, sizeof(struct playback_sample_data));
    if (!sample_data) {
        ALOGE("sample_data calloc failed\n");
        rc = -ENOMEM;
        fclose(file);
        return rc;
    }
    sample_data->acdb_id = acdb_id;

/************Init snd_monitor feature***********************/
    rc = snd_mon_feature_init(property_get_bool("vendor.audio.feature.snd_mon.enable", false));
    if (rc)
        ALOGE("%s: SND_MONITOR feature is not enabled, ADSP SSR will not be handled \n", __func__);
    else
        ALOGD("%s: SND_MONITOR feature init successful \n", __func__);

    rc = audio_chime_snd_mon_init();
    if (rc)
        ALOGE("%s: SND_MONITOR_INIT failed, ADSP SSR will not be handled \n", __func__);
    else
        ALOGD("%s: SND_MONITOR_INIT successful \n", __func__);

    rc = audio_chime_snd_mon_register_listener(sample_data, chime_snd_mon_cb);
    if (rc)
        ALOGE("%s: SND_MONITOR_LISTENER_REGISTER failed, ADSP SSR will not be handled \n", __func__);
    else
        ALOGD("%s: SND_MONITOR_LISTENER_REGISTER successful \n", __func__);


/****************** ACDB_LOADER ****************************/

    ALOGD("acdb_loader start\n");
    my_data = calloc(1, sizeof(struct platform_data));
    if (!my_data) {
        ALOGE("my_data calloc failed\n");
        rc = -ENOMEM;
        goto close_file;
    }
    list_init(&my_data->acdb_meta_key_list);
    error = dlerror();
    my_data->acdb_handle = dlopen(LIB_ACDB_LOADER_CLIENT, RTLD_LAZY);
    error = dlerror();
    if (error != NULL || my_data->acdb_handle == NULL) {
        ALOGE("%s: DLOPEN failed for %s with error %s\n", __func__, LIB_ACDB_LOADER_CLIENT, error);
        rc = 1;
        goto free_mydata;
    }

    ALOGD("DLOPEN success for %s\n", LIB_ACDB_LOADER_CLIENT);
    error = dlerror();
    my_data->acdb_init_v4 = (acdb_init_v4_t)dlsym(my_data->acdb_handle,
                                                 "acdb_loader_init_v4");
    error = dlerror();
    if (error != NULL || my_data->acdb_init_v4 == NULL) {
        ALOGE("%s: DLSYM failed for acdb_loader_init_v4 with error %s\n", __func__, error);
        rc = 1;
        goto close_acdbhandle;
    }

    my_data->acdb_send_audio_cal_v4 = (acdb_send_audio_cal_v4_t)dlsym(my_data->acdb_handle,
                                                 "acdb_loader_send_audio_cal_v4");
    error = dlerror();
    if (error != NULL || my_data->acdb_send_audio_cal_v4 == NULL) {
        ALOGE("%s: DLSYM failed for acdb_loader_send_audio_cal_v4 with error %s\n", __func__, error);
        rc = 1;
        goto close_acdbhandle;
    }

    my_data->acdb_send_audio_cal_v6 = (acdb_send_audio_cal_v6_t)dlsym(my_data->acdb_handle,
                                                 "acdb_loader_send_audio_cal_v6");
    error = dlerror();
    if (error != NULL || my_data->acdb_send_audio_cal_v6 == NULL) {
        ALOGE("%s: DLSYM failed for acdb_loader_send_audio_cal_v6 with error %s\n", __func__, error);
        rc = 1;
        goto close_acdbhandle;
    }


    my_data->acdb_send_common_top = (acdb_send_common_top_t)dlsym(my_data->acdb_handle,
                                                 "acdb_loader_send_common_custom_topology");
    error = dlerror();
    if (error != NULL || my_data->acdb_send_common_top == NULL) {
        ALOGE("%s: DLSYM failed for acdb_loader_send_common_custom_topology with error %s\n", __func__, error);
        rc = 1;
        goto close_acdbhandle;
    }

    my_data->acdb_deallocate_cal = (acdb_deallocate_cal_t)dlsym(my_data->acdb_handle,
                                                 "acdb_loader_deallocate_cal");
    error = dlerror();
    if (error != NULL || my_data->acdb_deallocate_cal == NULL) {
        ALOGE("%s: DLSYM failed for acdb_loader_deallocate_cal with error %s\n", __func__, error);
        rc = 1;
        goto close_acdbhandle;
    }

    sample_data->plt_data = my_data;

    if (my_data->acdb_init_v4) {
        ALOGD("acdb_loader_init\n");
        my_data->acdb_init_data.cvd_version = "2.4";
        my_data->acdb_init_data.snd_card_name = "sa6155-adp-star-snd-card";
        my_data->acdb_init_data.meta_key_list = &my_data->acdb_meta_key_list;
        my_data->acdb_init_data.is_instance_id_supported = &acdb_instance_id_support;
        rc = my_data->acdb_init_v4(&my_data->acdb_init_data, ACDB_LOADER_INIT_V4);
        if (rc) {
            ALOGE("acdb_init_v4 failed\n");
            goto close_acdbhandle;
        }
        is_acdb_init_done = true;
    }

    //consider path as 0 since kernel considers RX as 0
    path = ACDB_DEV_TYPE_OUT-1;

    if(my_data->acdb_send_audio_cal_v6) {
        ALOGD("acdb_loader_send_audio_cal \n");
        my_data->acdb_send_audio_cal_v6(sample_data->acdb_id, ACDB_DEV_TYPE_OUT,
                                            DEFAULT_APP_TYPE_RX_PATH, DEFAULT_OUTPUT_SAMPLING_RATE, MULTIMEDIA_FEDAI_ID,
                                            DEFAULT_OUTPUT_SAMPLING_RATE, CAL_MODE_SEND, CAL_OFFSET_ASM_TOP);
    }
    else if (my_data->acdb_send_audio_cal_v4) {
        ALOGD("acdb_loader_send_audio_cal \n");
        my_data->acdb_send_audio_cal_v4(sample_data->acdb_id, ACDB_DEV_TYPE_OUT, DEFAULT_APP_TYPE_RX_PATH,
                                        DEFAULT_OUTPUT_SAMPLING_RATE, path, DEFAULT_OUTPUT_SAMPLING_RATE);
    }

/*******************AUDIO_HAL_PLUGIN***********/

    ALOGD("audio_hal_plugin start\n");
    my_plugin = calloc(1, sizeof(struct hal_plugin_data));
    if (!my_plugin) {
        ALOGE("my_plugin calloc failed\n");
        rc = -ENOMEM;
        goto close_acdbhandle;
    }

    error = dlerror();
    my_plugin->plugin_handle = dlopen(LIB_PLUGIN_CLIENT, RTLD_LAZY);
    error = dlerror();
    if (error != NULL || my_plugin->plugin_handle == NULL) {
        ALOGE("%s: DLOPEN failed for %s with error %s\n", __func__, LIB_PLUGIN_CLIENT, error);
        rc = 1;
        goto free_myplugin;
    }

    ALOGE("DLOPEN success for %s\n", LIB_PLUGIN_CLIENT);
    error = dlerror();
    my_plugin->plugin_init = (audio_hal_plugin_init_t)dlsym(my_plugin->plugin_handle,
                                             "audio_hal_plugin_init");
    error = dlerror();
    if (error != NULL || my_plugin->plugin_init == NULL) {
        ALOGE("%s: DLSYM failed for audio_hal_plugin_init with error %s\n", __func__, error);
        rc = 1;
        goto close_pluginhandle;
    }

    if(my_plugin->plugin_init) {
        ALOGD("audio_hal_plugin_init \n");
        rc = my_plugin->plugin_init();
        if (rc) {
            ALOGE("plugin_init failed\n");
            goto close_pluginhandle;
        }
    }

/****************** Play sample ****************************/

    rc = play_sample(file, card, device, chunk_fmt.num_channels, chunk_fmt.sample_rate,
                chunk_fmt.bits_per_sample, period_size, period_count);


/*ToDo
  *Check for absolute system time
  *Try for restarting playback only if absolute system time
   is less than expected audio_chime KPI (add this condition in below if())
  * Calculate timeout value based on absolute system time and audio_chime KPI
*/
    if((rc != 0) && (card_status == CARD_STATUS_OFFLINE)) {
        /*Playback stopped due to ADSP SSR*/
        ALOGE("Playback stopped due to SSR, wait for recovery\n");
        sleepRetry = 0;
        while((restartPlayback == false) && (sleepRetry < MAX_SLEEP_RETRY))
        {
            AUDIO_CHIME_SLEEP(AUDIO_CHIME_WAIT_TIME * 1000);
            sleepRetry++;
        }
        if(restartPlayback == true) {
            ALOGD("restarting playback\n");
            rc = play_sample(file, card, device, chunk_fmt.num_channels, chunk_fmt.sample_rate,
                        chunk_fmt.bits_per_sample, period_size, period_count);
        }
        else
            ALOGE("Recovery from SSR timed out\n");
    }

    if (my_data->acdb_deallocate_cal) {
        ALOGD("acdb_loader_deallocate_cal \n");
        my_data->acdb_deallocate_cal(ACDB_DEV_TYPE_OUT, MULTIMEDIA_FEDAI_ID);
    }
close_pluginhandle:
    dlclose(my_plugin->plugin_handle);
free_myplugin:
    free(my_plugin);
close_acdbhandle:
    dlclose(my_data->acdb_handle);
free_mydata:
    free(my_data);
close_file:
    audio_chime_snd_mon_unregister_listener(sample_data);
    audio_chime_snd_mon_deinit();
    fclose(file);
    free(sample_data);
__attribute__((unused))disable_audio_route:
    set_mixer_controls(DISABLE_MIXER);

    return rc;
}

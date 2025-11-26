/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
#include <math.h>

#include <alsa/asoundlib.h>
#include <alsa/mixer.h>

#define LIB_ACDB_LOADER_CLIENT "libacdbloaderclient.so"

#define ACDB_DEV_TYPE_OUT 1
#define DEFAULT_APP_TYPE_RX_PATH 69936
#define DEFAULT_OUTPUT_SAMPLING_RATE 48000
#define DEFAULT_ACDB_ID 60
#define DEFAULT_FEDAI_ID 0
#define DEFAULT_BIT_WIDTH 16

#define AUDIO_CHIME_SLEEP(ms) usleep((uint32_t)ms)

struct audio_chime_open_param
{
    uint32_t acdb_id;  /*acdb device for synth */
    uint32_t app_type;  /*app type select for copp */
    uint32_t sample_rate; /*sample rate for copp, it could be 8k/16k/48k */
    uint32_t backend_id;  /*sample rate for copp, it could be 8k/16k/48k */
    uint32_t instance_id; /*for multi instance, default 0 */
    uint32_t count;   /*for white noise play, default 10000 */
};
typedef struct audio_chime_open_param audio_chime_open_param_t;

/*------------------------------acdb loader begin----------------------------*/
typedef void (*acdb_send_audio_cal_t)(int, int, int, int, int, int);
struct platform_data
{
    void *acdb_handle;
    acdb_send_audio_cal_t acdb_send_audio_cal;
};
/*------------------------------acdb loader end------------------------------*/

/*---------------------------------wav file begin----------------------------*/
#define ID_RIFF 0x46464952 //"RIFF"
#define ID_WAVE 0x45564157 //"WAVE"
#define ID_FMT 0x20746d66  //"fmt "
#define ID_DATA 0x61746164 //"data"
struct riff_wave_header
{
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t wave_type;
};
struct chunk_header
{
    uint32_t id;
    uint32_t sz;
};
struct chunk_fmt
{
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};
/*---------------------------------wav file end----------------------------*/

/*---------------------------------amixer begin----------------------------*/
enum
{
    MIXER_SLOTNUMBER = 0,
    MIXER_RX_CHANNEL,
    MIXER_RX_ROUTING,
    MIXER_RX_SLOTMAPPING,
    MIXER_APPCONF,
    MIXER_RX_STREAMAPPCONF,
    MIXER_MAX,
};

enum
{
    ROUTE_MIXER = 0,
    ROUTE_VALUE,
    ROUTE_MAX
};

static char instance_id_support[] = "Instance ID Support";
const char *g_audio_route[MIXER_MAX][ROUTE_MAX] = {
    {"TERT_TDM SlotNumber", "3"}, // TDM8
    {"TERT_TDM_RX_0 Channels", "1"}, // channel Two
    {"TERT_TDM_RX_0 Audio Mixer MultiMedia1", "1"},
    {"TERT_TDM_RX_0 SlotMapping", "NULL"},
    {"App Type Config", "NULL"},
    {"Audio Stream 0 App Type Cfg", "NULL"},
};
/*---------------------------------amixer end----------------------------*/

static char *device = "hw:0,0"; /* playback device */
static snd_pcm_format_t format = SND_PCM_FORMAT_S16;/* sample format */
static unsigned int sample_rate = DEFAULT_OUTPUT_SAMPLING_RATE; /* stream sample_rate */
static unsigned int channels = 2; /* count of channels */
static unsigned int buffer_time = 50000; /* ring buffer length in us */
static unsigned int period_time = 10000; /* period time in us */

static audio_chime_open_param_t open_args = {
    // Defaults
    .acdb_id = 60,
    .app_type = 0x11130,
    .sample_rate = 48000,
    .backend_id = 87,
    .count = 10000,
};
static int resample = 1; /* enable alsa-lib resampling */
static int period_event = 0; /* produce poll event after each period */

static snd_pcm_sframes_t buffer_size;
static snd_pcm_sframes_t period_size;

static int32_t mixer_ctl_set_value(int mixer_value);
static int32_t mixer_ctl_set_instance_support(const char *ctl_name, int ctl_value);
static int set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *params, snd_pcm_access_t access);
static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams);
static int play_sample(snd_pcm_t *handle, FILE *file);

static int play_sample(snd_pcm_t *handle, FILE *file)
{
    signed short *ptr;
    int err, cptr;
    int num_read, size = 0;
    int cnt = 0;
    short *samples = NULL;
    if (file == NULL)
    {
        ALOGE("Unable to open file, play noise!!!\n");
        samples = malloc((period_size * channels * snd_pcm_format_physical_width(format)) / 8);
        if (samples == NULL)
        {
            ALOGE("malloc samples failed, no enough memory\n");
            return -ENODEV;
        }
        memset(samples, 0x5A, sizeof(samples));
        cnt = open_args.count;
        while (cnt > 0)
        {
            err = snd_pcm_mmap_writei(handle, samples, period_size);
            if (err == -EAGAIN)
                continue;
            cnt--;
        }
        free(samples);
        ALOGD("playback end!!!");
    }
    else
    {
        size = (period_size * channels * snd_pcm_format_physical_width(format)) / 8;

        samples = malloc(size);
        if (samples == NULL)
        {
            ALOGE("No enough memory\n");
            return -ENODEV;
        }

        memset(samples, 0, size);
        ALOGD("%s : period_size = %d, channels = %d, tot_size = %d\n", __func__, period_size, channels, size);
        do
        {
            //get period size file from wav file
            num_read = fread(samples, 1, size, file);
            if (num_read > 0)
            {
                //start to wirte ring buffer
                ptr = samples;
                cptr = period_size;
                while (cptr > 0)
                {
                    err = snd_pcm_mmap_writei(handle, ptr, cptr);
                    if (err == -EAGAIN)
                        continue;
                    ptr += err * channels;
                    cptr -= err;
                }
            }
            cnt++;
        } while (num_read > 0);
        free(samples);
        fclose(file);
        ALOGD("%s : loop cnt = %d \n", __func__, cnt);
    }
    return 0;
}

static int set_hwparams(snd_pcm_t *handle,
                        snd_pcm_hw_params_t *params,
                        snd_pcm_access_t access)
{
    unsigned int rrate;
    snd_pcm_uframes_t size;
    int err, dir;

    /* choose all parameters */
    err = snd_pcm_hw_params_any(handle, params);
    if (err < 0)
    {
        ALOGE("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
        return err;
    }
    /* set hardware resampling */
    err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
    if (err < 0)
    {
        ALOGE("Resampling setup failed for playback: %s\n", snd_strerror(err));
        return err;
    }
    ALOGD("set resample = %d\n", resample);
    /* set the interleaved read/write format */
    err = snd_pcm_hw_params_set_access(handle, params, access);
    if (err < 0)
    {
        ALOGE("Access type not available for playback: %s\n", snd_strerror(err));
        return err;
    }
    ALOGD("set access = %d\n", access);
    /* set the sample format */
    err = snd_pcm_hw_params_set_format(handle, params, format);
    if (err < 0)
    {
        ALOGE("Sample format not available for playback: %s\n", snd_strerror(err));
        return err;
    }
    ALOGD("set format = %d\n", format);
    /* set the count of channels */
    err = snd_pcm_hw_params_set_channels(handle, params, channels);
    if (err < 0)
    {
        ALOGE("Channels count (%u) not available for playbacks: %s\n", channels, snd_strerror(err));
        return err;
    }
    ALOGD("set channels = %d\n", channels);
    /* set the stream sample_rate */
    rrate = sample_rate;
    err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
    if (err < 0)
    {
        ALOGE("sample_rate %uHz not available for playback: %s\n", sample_rate, snd_strerror(err));
        return err;
    }
    if (rrate != sample_rate)
    {
        ALOGE("sample_rate doesn't match (requested %uHz, get %iHz)\n", sample_rate, err);
        return -EINVAL;
    }
    ALOGD("set rrate = %d\n", rrate);
    /* set the buffer time */
    err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, &dir);
    if (err < 0)
    {
        ALOGE("Unable to set buffer time %u for playback: %s\n", buffer_time, snd_strerror(err));
        return err;
    }
    ALOGD("set buffer_time = %d\n", buffer_time);
    err = snd_pcm_hw_params_get_buffer_size(params, &size);
    if (err < 0)
    {
        ALOGE("Unable to get buffer size for playback: %s\n", snd_strerror(err));
        return err;
    }
    buffer_size = size;
    ALOGD("get buffer_size = %d\n", buffer_size);
    /* set the period time */
    err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, &dir);
    if (err < 0)
    {
        ALOGE("Unable to set period time %u for playback: %s\n", period_time, snd_strerror(err));
        return err;
    }
    ALOGD("set period_time = %d\n", period_time);
    err = snd_pcm_hw_params_get_period_size(params, &size, &dir);
    if (err < 0)
    {
        ALOGE("Unable to get period size for playback: %s\n", snd_strerror(err));
        return err;
    }
    period_size = size;
    ALOGD("get period_size = %d\n", period_size);
    /* write the parameters to device */
    err = snd_pcm_hw_params(handle, params);
    if (err < 0)
    {
        ALOGE("Unable to set hw params for playback: %s\n", snd_strerror(err));
        return err;
    }
    return 0;
}

static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
    int err;

    /* get the current swparams */
    err = snd_pcm_sw_params_current(handle, swparams);
    if (err < 0)
    {
        ALOGE("Unable to determine current swparams for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* start the transfer when the buffer is almost full: */
    /* (buffer_size / avail_min) * avail_min */
    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size);
    if (err < 0)
    {
        ALOGE("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* allow the transfer when at least period_size samples can be processed */
    /* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
    err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_event ? buffer_size : period_size);
    if (err < 0)
    {
        ALOGE("Unable to set avail min for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* enable period events when requested */
    if (period_event)
    {
        err = snd_pcm_sw_params_set_period_event(handle, swparams, 1);
        if (err < 0)
        {
            ALOGE("Unable to set period event: %s\n", snd_strerror(err));
            return err;
        }
    }
    /* write the parameters to the playback device */
    err = snd_pcm_sw_params(handle, swparams);
    if (err < 0)
    {
        ALOGE("Unable to set sw params for playback: %s\n", snd_strerror(err));
        return err;
    }
    return 0;
}

static int32_t mixer_ctl_set_value(int mixer_value)
{
    int32_t ret = 0;
    int32_t card_index = -1;
    char dev[32];
    static snd_ctl_t *handle = NULL;
    snd_ctl_elem_id_t *id = NULL;
    snd_ctl_elem_value_t *control = NULL;
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_value_alloca(&control);

    if (snd_card_next(&card_index) < 0 || card_index < 0)
    {
        ALOGE("%s: Failed to find sound card", __func__);
        return EINVAL;
    }
    snprintf(dev, sizeof(dev), "hw:%d", card_index);
    ALOGD("%s: mixer ctl dev = %s, mixer_value = %d\n", __func__, dev, mixer_value);

    if ((ret = snd_ctl_open(&handle, dev, 0)) < 0)
    {
        ALOGE("%s: Control %s open error: %s", __func__, dev, snd_strerror(ret));
        goto exit;
    }

    snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
    snd_ctl_elem_id_set_name(id, g_audio_route[mixer_value][ROUTE_MIXER]);

    snd_ctl_elem_value_set_id(control, id);
    if ((ret = snd_ctl_elem_read(handle, control)) < 0)
    {
        ALOGE("%s: Cannot read the given element from control", __func__);
        goto exit;
    }

    if (mixer_value < MIXER_RX_SLOTMAPPING)
        snd_ctl_elem_value_set_integer(control, 0, atoi(g_audio_route[mixer_value][ROUTE_VALUE]));
    else if (mixer_value == MIXER_RX_SLOTMAPPING)
    {
        snd_ctl_elem_value_set_integer(control, 0, 0);
        snd_ctl_elem_value_set_integer(control, 1, 4);
        snd_ctl_elem_value_set_integer(control, 2, 65535);
        snd_ctl_elem_value_set_integer(control, 3, 65535);
        snd_ctl_elem_value_set_integer(control, 4, 65535);
        snd_ctl_elem_value_set_integer(control, 5, 65535);
    }
    else if (mixer_value == MIXER_APPCONF)
    {
        snd_ctl_elem_value_set_integer(control, 0, 1);
        snd_ctl_elem_value_set_integer(control, 1, DEFAULT_APP_TYPE_RX_PATH);
        snd_ctl_elem_value_set_integer(control, 2, DEFAULT_OUTPUT_SAMPLING_RATE);
        snd_ctl_elem_value_set_integer(control, 3, 16);
    }
    else if (mixer_value == MIXER_RX_STREAMAPPCONF)
    {
        snd_ctl_elem_value_set_integer(control, 0, DEFAULT_APP_TYPE_RX_PATH);
        snd_ctl_elem_value_set_integer(control, 1, open_args.acdb_id);
        snd_ctl_elem_value_set_integer(control, 2, DEFAULT_OUTPUT_SAMPLING_RATE);
        snd_ctl_elem_value_set_integer(control, 3, open_args.backend_id);
    }

    if ((ret = snd_ctl_elem_write(handle, control)) < 0)
    {
        ALOGE("%s: Cannot write the given element from control", __func__);
        goto exit;
    }
    ALOGD("%s: Exit!\n", __func__);

    exit:
    if (handle)
        snd_ctl_close(handle);
    return ret;
}

int main(int argc, char *argv[])
{
    int c, rc = 0;
    uint32_t i = 0, read_bytes;
    int option_index;
    char *samples = NULL;
    char filename[64];
    struct platform_data *my_data = NULL;
    const char *error = NULL;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *hwparams;
    snd_pcm_sw_params_t *swparams;
    struct riff_wave_header riff_wave_header;
    struct chunk_header chunk_header;
    struct chunk_fmt chunk_fmt = {0};
    int more_chunks = 1;
    FILE *wav_handle = NULL;

    struct option long_opts[] = {
        {"file_name", required_argument, 0, 0},
        {"acdb_id", required_argument, 0, 0},
        {"apptype", required_argument, 0, 0},
        {"sample-rate", required_argument, 0, 0},
        {"backend-id", required_argument, 0, 0},
        {"count", required_argument, 0, 0},
        {0, 0, 0, 0},
    };

    /****************** Parsing parameter info *********************/
    ALOGD("...Starting audio chime...");
    while ((c = getopt_long(argc, argv, "", long_opts, &option_index)) != -1)
    {
        switch (c)
        {
        case 0:
        {
            if (!strcmp("file_name", long_opts[option_index].name) && optarg)
            {
                strlcpy(filename, optarg, sizeof(filename));
                ALOGD("option \"%s\" set to %s", long_opts[option_index].name, filename);
            }
            else if (!strcmp("acdb_id", long_opts[option_index].name) && optarg)
            {
                open_args.acdb_id = strtol(optarg, NULL, 0);
                ALOGD("option \"%s\" set to %d", long_opts[option_index].name, open_args.acdb_id);
            }
            else if (!strcmp("apptype", long_opts[option_index].name) && optarg)
            {
                open_args.app_type = strtol(optarg, NULL, 0);
                ALOGD("option \"%s\" set to 0x%x", long_opts[option_index].name, open_args.app_type);
            }
            else if (!strcmp("sample-rate", long_opts[option_index].name) && optarg)
            {
                open_args.sample_rate = strtol(optarg, NULL, 0);
                ALOGD("option \"%s\" set to %d", long_opts[option_index].name, open_args.sample_rate);
            }
            else if (!strcmp("backend-id", long_opts[option_index].name) && optarg)
            {
                open_args.backend_id = strtol(optarg, NULL, 0);
                ALOGD("option \"%s\" set to %d", long_opts[option_index].name, open_args.backend_id);
            }
            else if (!strcmp("count", long_opts[option_index].name) && optarg)
            {
                open_args.count = strtol(optarg, NULL, 0);
                ALOGD("option \"%s\" set to %d", long_opts[option_index].name, open_args.count);
            }
            break;
        }
        default:
        {
            ALOGD("invalid command line argument");
            break;
        }
        }
    }
    ALOGD("device = %s, acdb_id = %d, backend_id = %d\n", device, open_args.acdb_id, open_args.backend_id);

    /****************** Parsing media file info *********************/
    wav_handle = fopen(filename, "rb");
    if (wav_handle != NULL)
    {
        ALOGD("file_open success\n");
        fread(&riff_wave_header, sizeof(riff_wave_header), 1, wav_handle);
        if ((riff_wave_header.riff_id != ID_RIFF) ||
            (riff_wave_header.wave_type != ID_WAVE))
        {
            ALOGE("Error: '%s' is not a riff/wave file\n", filename);
            fclose(wav_handle);
            return -EINVAL;
        }

        do
        {
            fread(&chunk_header, sizeof(chunk_header), 1, wav_handle);

            switch (chunk_header.id)
            {
            case ID_FMT:
                fread(&chunk_fmt, sizeof(chunk_fmt), 1, wav_handle);
                /* If the format header is larger, skip the rest */
                if (chunk_header.sz > sizeof(chunk_fmt))
                    fseek(wav_handle, chunk_header.sz - sizeof(chunk_fmt), SEEK_CUR);
                break;
            case ID_DATA:
                /* Stop looking for chunks */
                more_chunks = 0;
                break;
            default:
                /* Unknown chunk, skip bytes */
                fseek(wav_handle, chunk_header.sz, SEEK_CUR);
            }
        } while (more_chunks);
        ALOGD("audio_format = %d, num_channels=%d, sample_rate =%d, byte_rate = %d, block_align= %d, bits_per_sample = %d\n",
            chunk_fmt.audio_format, chunk_fmt.num_channels, chunk_fmt.sample_rate,
            chunk_fmt.byte_rate, chunk_fmt.block_align, chunk_fmt.bits_per_sample);

        channels = chunk_fmt.num_channels;
        sample_rate = chunk_fmt.sample_rate;
    }
    /****************** Mixer Control INIT**************************/
    ALOGD("Starting Audio_Chime App!");
    for (i = 0; i < MIXER_MAX; i++)
    {
        rc = mixer_ctl_set_value(i);
        if (rc)
        {
            ALOGE("%s: mixer_ctl_set_value failed: %d", __func__, rc);
            return -EINVAL;
        }
        ALOGD("%s: mixer[%d] is successfully set.", __func__, i);
    }

    /****************** ACDB_LOADER INIT****************************/
    ALOGD("acdb_loader start\n");
    my_data = calloc(1, sizeof(struct platform_data));
    if (!my_data)
    {
        ALOGE("my_data calloc failed\n");
        rc = -ENOMEM;
        return rc;
    }
    error = dlerror();
    my_data->acdb_handle = dlopen(LIB_ACDB_LOADER_CLIENT, RTLD_LAZY);
    error = dlerror();
    if (error != NULL || my_data->acdb_handle == NULL)
    {
        ALOGE("%s: DLOPEN failed for %s with error %s\n", __func__, LIB_ACDB_LOADER_CLIENT, error);
        rc = -ENOMEM;
        goto free_mydata;
    }

    my_data->acdb_send_audio_cal = (acdb_send_audio_cal_t)dlsym(my_data->acdb_handle,
                    "acdb_loader_send_audio_cal_v4");
    error = dlerror();
    if (error != NULL || my_data->acdb_send_audio_cal == NULL)
    {
        ALOGE("%s: DLSYM failed for acdb_loader_send_audio_cal_v4 with error %s\n", __func__, error);
        rc = -ENOMEM;
        goto close_acdbhandle;
    }

    if (my_data->acdb_send_audio_cal)
    {
        ALOGD("acdb_loader_send_audio_cal \n");
        my_data->acdb_send_audio_cal(open_args.acdb_id, ACDB_DEV_TYPE_OUT, DEFAULT_APP_TYPE_RX_PATH,
                     DEFAULT_OUTPUT_SAMPLING_RATE, DEFAULT_FEDAI_ID, DEFAULT_OUTPUT_SAMPLING_RATE);
    }
    AUDIO_CHIME_SLEEP(500000);

    /****************** Set sw/hw Parameter **************************/
    snd_pcm_hw_params_alloca(&hwparams);
    snd_pcm_sw_params_alloca(&swparams);

    ALOGD("%s: snd_pcm_open start!\n", __func__);
    if ((rc = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
        ALOGE("Playback open error: %s\n", snd_strerror(rc));
        goto close_pcm;
    }
    ALOGD("%s: set_hwparams start!\n", __func__);
    if ((rc = set_hwparams(handle, hwparams, SND_PCM_ACCESS_MMAP_INTERLEAVED)) < 0)
    {
        ALOGE("Setting of hwparams failed: %s\n", snd_strerror(rc));
        goto close_pcm;
    }
    ALOGD("%s: set_swparams start!\n", __func__);
    if ((rc = set_swparams(handle, swparams)) < 0)
    {
        ALOGE("Setting of swparams failed: %s\n", snd_strerror(rc));
        goto close_pcm;
    }

    /****************** Play sample ********************************/
    ALOGD("%s: wave_play start!\n", __func__);
    rc = play_sample(handle, wav_handle);
    if (rc < 0)
    {
        ALOGE("wave play failed! \n");
        goto close_pcm;
    }

close_pcm:
    ALOGD("%s: exit,close_pcm!\n", __func__);
    snd_pcm_close(handle);
close_acdbhandle:
    ALOGD("%s: exit,close_acdbhandle!\n", __func__);
    dlclose(my_data->acdb_handle);
free_mydata:
    ALOGD("%s: exit,free_mydata!\n", __func__);
    free(my_data);
    return rc;
}

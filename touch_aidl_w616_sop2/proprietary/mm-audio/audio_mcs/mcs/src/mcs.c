/*
 * Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define LOG_TAG "MCS"

#include <tinyalsa/asoundlib.h>
#include <sound/asound.h>
#include "mcs.h"
#include "mcs_api.h"
#include <unistd.h>
#include <cutils/properties.h>
#include <errno.h>
#include <agm/agm_api.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <audio_route/audio_route.h>

#define NO_OF_BUFS 4
#define REC_BUF_SIZE 1920
#define BUFFER_COUNT 32
#define SNDCARD 100
#define MAX_SND_CARD 10
#define MAX_RETRY_CNT 20
#define SND_CARD_VIRTUAL 100
#define MIXER_FILE_DELIMITER "_"
#define MIXER_FILE_EXT ".xml"
#define MIXER_PATH_MAX_LENGTH 100
#define HW_INFO_ARRAY_MAX_SIZE 32
#define VENDOR_CONFIG_PATH_MAX_LENGTH 128
#define XML_PATH_EXTN_MAX_SIZE 80

#define AUDIO_INTERFACE_RX "CODEC_DMA-LPAIF_WSA-RX-0"
#define AUDIO_INTERFACE_TX "CODEC_DMA-LPAIF_RXTX-TX-3"

#if defined (LINUX_ENABLED)
    #define MIXER_XML_BASE_STRING_NAME "mixer_paths_wsa"
#elif defined (FEATURE_IPQ_OPENWRT)
    #define MIXER_XML_BASE_STRING_NAME "mixer_paths"
#else
    #define MIXER_XML_BASE_STRING_NAME "mixer_paths"
#endif

struct mcs_play_ctxt {
    unsigned int cur_state;
    mcs_play_t cur_param;
    ar_fhandle file_handle;
    ar_osal_thread_t tid;
    ar_osal_thread_attr_t tattr;
    void *stream_handle;
    size_t buffer_size;
    unsigned int device;
    char intf_name[100];
    char snd_device_name[100];
    struct pcm *pcm;
};

struct mcs_rec_ctxt {
    unsigned int cur_state;
    mcs_record_t cur_param;
    ar_fhandle file_handle;
    ar_osal_thread_t tid;
    ar_osal_thread_attr_t tattr;
    void *stream_handle;
    size_t buffer_size;
    unsigned int device;
    char intf_name[100];
    char snd_device_name[100];
    struct pcm *pcm;
};

struct acdb_mcs {
    struct mcs_play_ctxt* pb_ctxt;
    struct mcs_rec_ctxt* rec_ctxt;
    ar_osal_mutex_t lock;
    struct mixer *mixer;
    struct audio_route* audio_route;
};

static struct acdb_mcs *mcs_info;

struct audio_mixer* audio_mixer = NULL;

enum {
    MCS_STATE_IDLE,
    MCS_STATE_RUNNING,
    MCS_STATE_STOPPING
};

enum dir{
    PLAYBACK,
    CAPTURE,
    LOOPBACK,
};

enum stream_type {
    STREAM_PCM,
    STREAM_COMPRESS,
};

static int set_device_media_config(struct mixer *mixer, unsigned int channels,
                unsigned int rate, unsigned int bits, char *intf_name);
static int set_metadata(struct mixer *mixer, int device, uint32_t val,
                enum stream_type stype, char *intf_name,
                mcs_play_t *mcs_param);

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

static void getFileNameExtn(const char* in_snd_card_name, const char* file_name_extn)
{
    /* Sound card name follows below mentioned convention:
       <target name>-<form factor>-<variant>-snd-card.
    */
    char *snd_card_name = NULL;
    char *tmp = NULL;
    char *card_sub_str = NULL;

    snd_card_name = strdup(in_snd_card_name);

    if (snd_card_name == NULL) {
        AR_LOG_ERR(LOG_TAG,"snd_card_name passed is NULL");
        goto err;
    }

   card_sub_str = strtok_r(snd_card_name, "-", &tmp);
    if (card_sub_str == NULL) {
        AR_LOG_ERR(LOG_TAG,"called on invalid snd card name");
        goto err;
    }
    strlcat(file_name_extn,card_sub_str, XML_PATH_EXTN_MAX_SIZE);

    while ((card_sub_str = strtok_r(NULL, "-", &tmp))) {
           if (strncmp(card_sub_str, "snd", strlen("snd"))) {
               strlcat(file_name_extn, MIXER_FILE_DELIMITER, XML_PATH_EXTN_MAX_SIZE);
               strlcat(file_name_extn, card_sub_str, XML_PATH_EXTN_MAX_SIZE);
           }
           else
               break;
    }

    AR_LOG_INFO(LOG_TAG,"file path extension(%s)", file_name_extn);

err:
    if (snd_card_name)
        free(snd_card_name);

}

static int set_agm_stream_metadata_type(int device, char *val,
                                       enum stream_type stype)
{
    char *stream = "PCM";
    char *control = "control";
    char *mixer_str;
    struct mixer_ctl *ctl;
    int ctl_len = 0,ret = 0;

    ctl_len = strlen(stream) + 4 + strlen(control) + 1;
    mixer_str = calloc(1, ctl_len);
    if (!mixer_str) {
        AR_LOG_ERR(LOG_TAG, "%s: memory allocation failed.", __func__);
        return -ENOMEM;
    }
    snprintf(mixer_str, ctl_len, "%s%d %s", stream, device, control);

    AR_LOG_VERBOSE(LOG_TAG, "%s - mixer -%s-\n", __func__, mixer_str);
    ctl = mixer_get_ctl_by_name(mcs_info->mixer, mixer_str);
    if (!ctl) {
        AR_LOG_ERR(LOG_TAG, "Invalid mixer control: %s\n", mixer_str);
        free(mixer_str);
        return -ENOENT;
    }

    ret = mixer_ctl_set_enum_by_string(ctl, val);
    free(mixer_str);
    return ret;
}

static unsigned int mcs_get_device_id(uint32_t device_id)
{
    uint32_t playback_device_id = 110, record_device_id = 111;
    if (device_id == DEVICERX)
         return playback_device_id;
    else if (device_id == DEVICETX)
         return record_device_id;
    return -1;

}

static int mcs_get_snd_device_name(uint32_t key_value, char *intf_name)
{
    if (key_value == SPEAKER) {
        strlcpy(intf_name, "speaker", 8);
        return 0;
    } else if (key_value == HANDSETMIC) {
        strlcpy(intf_name, "handset-mic", 12);
        return 0;
    } else if (key_value == HEADPHONES ) {
        strlcpy(intf_name, "headphones", 11);
        return 0;
    } else if (key_value == SPEAKER_MIC ) {
        strlcpy(intf_name, "speaker-mic", 12);
        return 0;
    } else if (key_value == VI_TX ) {
        strlcpy(intf_name, "vi-feedback", 12);
        return 0;
    } else if (key_value == HEADPHONE_MIC ) {
        strlcpy(intf_name, "headset-mic", 12);
        return 0;
    }
    return -1;
}

static int mcs_get_intf_name(uint32_t key_value, char *intf_name)
{
    if (key_value == SPEAKER) {
        strlcpy(intf_name, AUDIO_INTERFACE_RX, strlen(AUDIO_INTERFACE_RX) + 1);
        return 0;
    } else if (key_value == HANDSETMIC) {
        strlcpy(intf_name, AUDIO_INTERFACE_TX, strlen(AUDIO_INTERFACE_TX) + 1);
        return 0;
    } else if (key_value == VI_TX) {
        strlcpy(intf_name, "CODEC_DMA-LPAIF_WSA-TX-0",
                        strlen("CODEC_DMA-LPAIF_WSA-TX-0") + 1);
        return 0;
    } else if (key_value == HEADPHONES) {
        strlcpy(intf_name, "CODEC_DMA-LPAIF_RXTX-RX-0",
                        strlen("CODEC_DMA-LPAIF_RXTX-RX-0") + 1);
        return 0;
    } else if (key_value == SPEAKER_MIC) {
        strlcpy(intf_name, AUDIO_INTERFACE_TX, strlen(AUDIO_INTERFACE_TX) + 1);
        return 0;
    } else if (key_value == HEADPHONE_MIC) {
        strlcpy(intf_name, AUDIO_INTERFACE_TX, strlen(AUDIO_INTERFACE_TX) + 1);
        return 0;
    }
    AR_LOG_ERR(LOG_TAG,"Invalid Key_value:%d\n", key_value);
    return -1;
}

static int32_t mcs_record_prepare(struct mcs_rec_ctxt* ctxt)
{
    AR_LOG_VERBOSE(LOG_TAG,"enter: ctxt - %pK", ctxt);
    int ret = 0;
    size_t size;
    struct pcm_config config;
    unsigned int period_size = 1024;
    unsigned int period_count = 4;
    int bits = 0;
    int card = SND_CARD_VIRTUAL;

    memset(&config, 0, sizeof(config));
    if (ctxt == NULL) {
        AR_LOG_ERR(LOG_TAG, " null pointer");
        ret = AR_EBADPARAM;
        return ret;
    }
    config.channels = ctxt->cur_param.stream_properties.num_channels;
    config.rate = ctxt->cur_param.stream_properties.sample_rate;
    bits = ctxt->cur_param.stream_properties.bit_width;
    config.period_size = period_size;
    config.period_count = period_count;

    if (bits == 32)
        config.format = PCM_FORMAT_S32_LE;
    else if (bits == 24)
        config.format = PCM_FORMAT_S24_3LE;
    else if (bits == 16)
        config.format = PCM_FORMAT_S16_LE;

    config.start_threshold = 0;
    config.stop_threshold = INT_MAX;
    config.silence_threshold = 0;

    size = ctxt->cur_param.stream_properties.num_channels *
           ctxt->cur_param.stream_properties.bit_width * BUFFER_COUNT;

    if (ctxt->cur_param.write_to_file == 1) {
        ret = ar_fopen(&ctxt->file_handle, ctxt->cur_param.filename,
              AR_FOPEN_WRITE_ONLY);
        AR_LOG_VERBOSE(LOG_TAG,"ar_fread file handle %pK file name %s",
                         ctxt->file_handle, ctxt->cur_param.filename);
        if (ret != 0) {
            AR_LOG_ERR(LOG_TAG,"file open error");
            return ret;
        }
    }

    ctxt->pcm = pcm_open(card, ctxt->device, PCM_IN, &config);
    if (!ctxt->pcm || !pcm_is_ready(ctxt->pcm)) {
       AR_LOG_ERR(LOG_TAG, "Unable to open PCM device (%s)\n",
                pcm_get_error(ctxt->pcm));
        ret = -EINVAL;
        goto exit;
    }

    size = pcm_frames_to_bytes(ctxt->pcm, pcm_get_buffer_size(ctxt->pcm));
    ctxt->buffer_size = size;

    AR_LOG_VERBOSE(LOG_TAG, "Capturing sample: %u ch, %u hz, %u bit\n",
                      config.channels, config.rate,
                      pcm_format_to_bits(config.format));

    audio_route_apply_and_update_path(mcs_info->audio_route,
                                          ctxt->snd_device_name);
    ret = pcm_start(ctxt->pcm);
    if (ret < 0) {
        AR_LOG_ERR(LOG_TAG, "start error\n");
        goto err_pcm_close;
    }

    AR_LOG_VERBOSE(LOG_TAG, "exit");
    return ret;

err_pcm_close:
    pcm_close(ctxt->pcm);
exit:
    if (ctxt->file_handle != NULL){
        AR_LOG_INFO(LOG_TAG,"file closed");
        ar_fclose(ctxt->file_handle);
    }
    return ret;
}

static int32_t mcs_play_prepare(struct mcs_play_ctxt* ctxt)
{
    AR_LOG_VERBOSE(LOG_TAG,"enter: ctxt - %pK", ctxt);
    int ret = 0;
    size_t size;
    size_t in_buf_size = 0;
    struct pcm_config config;
    unsigned int period_size = 1024;
    unsigned int period_count = 4;
    unsigned int bits = 0;
    unsigned int card = SND_CARD_VIRTUAL;

    memset(&config, 0, sizeof(config));
    if (ctxt == NULL) {
        AR_LOG_ERR(LOG_TAG, " null pointer");
        ret = AR_EBADPARAM;
        return ret;
    }
    config.channels = ctxt->cur_param.stream_properties.num_channels;
    config.rate = ctxt->cur_param.stream_properties.sample_rate;
    bits = ctxt->cur_param.stream_properties.bit_width;
    config.period_size = period_size;
    config.period_count = period_count;

    if (bits == 32)
        config.format = PCM_FORMAT_S32_LE;
    else if (bits == 24)
        config.format = PCM_FORMAT_S24_3LE;
    else if (bits == 16)
        config.format = PCM_FORMAT_S16_LE;
    config.start_threshold = 0;
    config.stop_threshold = INT_MAX;
    config.silence_threshold = 0;


    size = ctxt->cur_param.stream_properties.num_channels *
           ctxt->cur_param.stream_properties.bit_width * BUFFER_COUNT;

    ret = ar_fopen(&ctxt->file_handle, ctxt->cur_param.filename,
          AR_FOPEN_READ_ONLY);
    AR_LOG_ERR(LOG_TAG, "%s:%d ar_fread file handle %pK",
                 __func__, __LINE__, ctxt->file_handle);
    if (ret != 0) {
        AR_LOG_ERR(LOG_TAG, "file open error");
        return ret;
    }

    ctxt->pcm = pcm_open(card, ctxt->device, PCM_OUT, &config);
    if (!ctxt->pcm || !pcm_is_ready(ctxt->pcm)) {
       AR_LOG_ERR(LOG_TAG, "Unable to open PCM device %u (%s)\n",
                ctxt->device, pcm_get_error(ctxt->pcm));
        ret -EINVAL;
        goto exit;
    }

    size = pcm_frames_to_bytes(ctxt->pcm, pcm_get_buffer_size(ctxt->pcm));
    ctxt->buffer_size = size;

    audio_route_apply_and_update_path(mcs_info->audio_route,
                                          ctxt->snd_device_name);
    ret = pcm_start(ctxt->pcm);
    if (ret < 0) {
        AR_LOG_ERR(LOG_TAG, "start error\n");
        goto err_pcm_close;
    }

    AR_LOG_VERBOSE(LOG_TAG, "exit");
    return ret;

err_pcm_close:
    pcm_close(ctxt->pcm);
exit:
    if (ctxt->file_handle != NULL)
        ar_fclose(ctxt->file_handle);
    return ret;
}

static int32_t mcs_play_close(struct mcs_play_ctxt* ctxt)
{
    AR_LOG_VERBOSE(LOG_TAG, "enter: ctxt - %pK", ctxt);
    int status = 0;
    if (ctxt == NULL) {
        AR_LOG_ERR(LOG_TAG, "null pointer");
        return AR_EBADPARAM;
    }

    status = pcm_stop(ctxt->pcm);
    if (status != 0) {
        AR_LOG_ERR(LOG_TAG, "stream stop failed");
    }

    status = pcm_close(ctxt->pcm);
    if (status != 0) {
        AR_LOG_ERR(LOG_TAG, "stream close failed");
    }
    if (ctxt->file_handle != NULL) {
        status = ar_fclose(ctxt->file_handle);
    }
    audio_route_reset_and_update_path(mcs_info->audio_route,
                                            ctxt->snd_device_name);
    AR_LOG_INFO(LOG_TAG, "exit");
    return status;
}

static void mcs_play_sample(void *txt)
{
    AR_LOG_VERBOSE(LOG_TAG, "enter: txt - %pK", txt);
    if (txt == NULL) {
        AR_LOG_ERR(LOG_TAG, "null pointer");
        return;
    }
    struct mcs_play_ctxt *ctxt = (struct mcs_play_ctxt *)txt;
    char * buffer = NULL;
    int status;
    size_t file_size = 0;
    size_t size, num_read, num_read1;
    int total_bytes_to_play = 0;
    int bytes_count = 0;
    int replay = 0;
    ar_fhandle file;
    if (ctxt->cur_param.playback_duration_sec > 0) {
        total_bytes_to_play = ctxt->cur_param.playback_duration_sec *
                              ctxt->cur_param.stream_properties.sample_rate *
                              ctxt->cur_param.stream_properties.num_channels *
                              ctxt->cur_param.stream_properties.bit_width / 8;
        replay = 1;
    }
    else
        return;
    size = ctxt->buffer_size;
    buffer = malloc(size);
    if (!buffer) {
        AR_LOG_ERR(LOG_TAG, "Unable to allocate %d bytes\n", size);
        goto fileclose;
    }
    file_size = ar_fsize(ctxt->file_handle);
    if (file_size == AR_EFAILED || file_size == AR_EBADPARAM) {
        AR_LOG_ERR(LOG_TAG, "file size error %d", file_size);
        if (replay == 1)
            goto err;
    }

    AR_LOG_INFO(LOG_TAG, "buffer address %pK", buffer);
    AR_LOG_VERBOSE(LOG_TAG, "playing sample\n");
    do {
        status = ar_fread(ctxt->file_handle, (void*)buffer, size, &num_read);
        AR_LOG_INFO(LOG_TAG, "ar_fread file handle %pK", ctxt->file_handle);
        if (status < 0) {
            AR_LOG_ERR(LOG_TAG, "ar read failed");
            break;
        }
        if (num_read > 0) {
            AR_LOG_INFO(LOG_TAG, "Before pcm_write\n");
            if (pcm_write(ctxt->pcm, buffer, num_read)) {
                AR_LOG_ERR(LOG_TAG, "Error playing sample\n");
                break;
            }

            bytes_count += num_read;
            AR_LOG_INFO(LOG_TAG, "%d bytes played", bytes_count);
            if (bytes_count >= total_bytes_to_play) {
                AR_LOG_INFO(LOG_TAG, "%d total bytes played", bytes_count);
                break;
            }
        } else if (num_read == 0) {
            if ((bytes_count % file_size) == 0) {
                status = pcm_stop(ctxt->pcm);
                AR_LOG_INFO(LOG_TAG, " pause playing sample");
                status = ar_fseek(ctxt->file_handle, 0, AR_FSEEK_BEGIN);
                AR_LOG_INFO(LOG_TAG, " fseek status = %d", status);
                num_read = size;
                status = pcm_start(ctxt->pcm);
                if (status < 0) {
                    AR_LOG_ERR(LOG_TAG, "start error\n");
                    goto err;
                }
                AR_LOG_INFO(LOG_TAG, " resume playing sample");
            }
        }
    } while ((ctxt->cur_state == MCS_STATE_RUNNING) && num_read > 0);
    AR_LOG_VERBOSE(LOG_TAG, "exit");
    free(buffer);
    return;
err:
    if (buffer)
        free(buffer);
fileclose:
    if (ctxt->file_handle)
        ar_fclose(ctxt->file_handle);
}

static int connect_audio_intf_to_stream(struct mixer *mixer, unsigned int device,
                    char *intf_name, enum stream_type stype, bool connect)
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

    ctl_len = strlen(stream) + 4 + strlen(control) + 1;
    mixer_str = calloc(1, ctl_len);
    if (!mixer_str) {
        AR_LOG_ERR(LOG_TAG, "%s: memory allocation failed.", __func__);
        return -ENOMEM;
    }
    snprintf(mixer_str, ctl_len, "%s%d %s", stream, device, control);

    AR_LOG_VERBOSE(LOG_TAG, "%s - mixer -%s-\n", __func__, mixer_str);
    ctl = mixer_get_ctl_by_name(mixer, mixer_str);
    if (!ctl) {
        AR_LOG_ERR(LOG_TAG, "Invalid mixer control: %s\n", mixer_str);
        free(mixer_str);
        return -ENOENT;
    }

    ret = mixer_ctl_set_enum_by_string(ctl, intf_name);
    free(mixer_str);
    return ret;
}
int32_t process_playback_request(uint8_t cmd,
                                struct mcs_play_ctxt* ctxt,
                                mcs_play_t *param)
{
    AR_LOG_INFO(LOG_TAG, "enter cmd = %x", cmd);
    int ret = 0;
    int no_of_devices = 0;
    int i = 0;
    int j = 0;
    void *tid1 = NULL;
    void *tid2 = NULL;
    int no_of_kv_pairs = 0;
    unsigned int sample_rate = 44100;
    unsigned int channels = 1;
    unsigned int bit_width = 16;

    if (ctxt == NULL) {
        AR_LOG_ERR(LOG_TAG, "%s:%d null pointer", __func__, __LINE__);
        return AR_EBADPARAM;
    }

    switch(cmd) {
        case MCS_START:
            AR_LOG_INFO(LOG_TAG, "MCS Playback start\n");
            if (param == NULL || param->graph_key_vector.kvp == NULL ) {
                AR_LOG_ERR(LOG_TAG, "%s:%d null pointer", __func__, __LINE__);
                return AR_EBADPARAM;
            }
            if (ctxt->cur_state == MCS_STATE_IDLE) {
                memcpy(&ctxt->cur_param, param, sizeof(ctxt->cur_param));
                no_of_kv_pairs = param->graph_key_vector.num_kvps;
                for (i = 0; i < no_of_kv_pairs; i++) {
                    if (param->graph_key_vector.kvp[i].key == STREAMRX) {
                        sample_rate = param->device_properties.sample_rate;
                        bit_width = param->device_properties.bit_width;
                        channels = param->device_properties.num_channels;
                    }
                    else if (param->graph_key_vector.kvp[i].key == DEVICERX) {
                        no_of_devices++;
                        ctxt->device = mcs_get_device_id(param->graph_key_vector.kvp[i].key);
                        if (ctxt->device < 0) {
                            AR_LOG_ERR(LOG_TAG, "error in getting device_id");
                            return ctxt->device;
                        }
                        ret = mcs_get_intf_name(param->graph_key_vector.kvp[i].value,
                                                              ctxt->intf_name);
                        if (ret) {
                            AR_LOG_ERR(LOG_TAG, "error in getting audio_intf name");
                            return ret;
                        }
                        ret = mcs_get_snd_device_name(param->graph_key_vector.kvp[i].value,
                                                       ctxt->snd_device_name);
                        if (ret) {
                            AR_LOG_ERR(LOG_TAG, "error in getting snd device name");
                            return ret;
                        }
                    }
                }
            }
            else {
                AR_LOG_ERR(LOG_TAG, " current state is not proper");
                ret = -EINVAL;
                return ret;
            }
            AR_LOG_INFO(LOG_TAG, "no of devices %d %pK", no_of_devices, ctxt);

           /* set device/audio_intf media config mixer control */
            if (set_device_media_config(mcs_info->mixer, channels, sample_rate,
                                                bit_width, ctxt->intf_name)) {
                AR_LOG_INFO(LOG_TAG, "Failed to set device media config\n");
                return AR_EFAILED;
            }
          /* set audio interface metadata mixer control */
            if (set_metadata(mcs_info->mixer, ctxt->device, PCM_LL_PLAYBACK,
                                                  STREAM_PCM, NULL, param)) {
                AR_LOG_INFO(LOG_TAG, "Failed to set pcm metadata\n");
                return AR_EFAILED;
            }

            if (connect_audio_intf_to_stream(mcs_info->mixer, ctxt->device,
                                        ctxt->intf_name, STREAM_PCM, true)) {
                AR_LOG_ERR(LOG_TAG, "Failed to connect pcm to audio interface\n");
                return AR_EFAILED;
            }

            ret = mcs_play_prepare(ctxt);
            if (ret == 0) {
                ctxt->cur_state = MCS_STATE_RUNNING;
                ret = ar_osal_thread_create(&mcs_info->pb_ctxt->tid,
                                  &mcs_info->pb_ctxt->tattr, mcs_play_sample,
                                                               (void *)ctxt);
                if (ret != 0) {
                    ctxt->cur_state = MCS_STATE_IDLE;
                    AR_LOG_ERR(LOG_TAG, "MCS Playback start thread failed\n");
                }
            }
            else {
                AR_LOG_ERR(LOG_TAG, "MCS Playback start failed\n");
            }
            break;
        case MCS_STOP:
            AR_LOG_INFO(LOG_TAG, "MCS Playback stop\n");
            if (ctxt->cur_state != MCS_STATE_IDLE) {
                AR_LOG_INFO(LOG_TAG, "%s:%d mcs_stop called", __func__, __LINE__);
                ctxt->cur_state = MCS_STATE_STOPPING;
                ret = ar_osal_thread_join_destroy(mcs_info->pb_ctxt->tid);
                ret = mcs_play_close(ctxt);
                ctxt->cur_state = MCS_STATE_IDLE;
                AR_LOG_INFO(LOG_TAG, "%s:%d Play stopped", __func__, __LINE__);
            }
            break;
        default:
            AR_LOG_ERR(LOG_TAG, "cmd not found  %x ", cmd);
            ret = AR_EFAILED;
            break;
    }
    AR_LOG_VERBOSE(LOG_TAG, "exit");
    return ret;
}

static int set_device_media_config(struct mixer *mixer, unsigned int channels,
                 unsigned int rate, unsigned int bits, char *intf_name)
{
    char *control = "rate ch fmt";
    char *mixer_str;
    struct mixer_ctl *ctl;
    long media_config[4];
    int ctl_len = 0;
    int ret = 0;

    ctl_len = strlen(intf_name)+1+strlen(control)+1;

    mixer_str = calloc(1, ctl_len);
    if (!mixer_str) {
        AR_LOG_ERR(LOG_TAG, "%s: memory allocation failed.", __func__);
        return -ENOMEM;
    }
    snprintf(mixer_str, ctl_len, "%s %s", intf_name, control);

    AR_LOG_VERBOSE(LOG_TAG, "%s - mixer -%s-\n", __func__, mixer_str);
    ctl = mixer_get_ctl_by_name(mixer, mixer_str);
    if (!ctl) {
        AR_LOG_ERR(LOG_TAG, "Invalid mixer control: %s\n", mixer_str);
        free(mixer_str);
        return -ENOENT;
    }

    media_config[0] = rate;
    media_config[1] = channels;
    media_config[2] = bits_to_alsa_format(bits);
    media_config[3] = AGM_DATA_FORMAT_FIXED_POINT;

    AR_LOG_VERBOSE(LOG_TAG, "%s - %d - %d - %d\n", __func__, media_config[0],
                                             media_config[1], media_config[2]);
    ret = mixer_ctl_set_array(ctl, &media_config,
                                sizeof(media_config)/sizeof(media_config[0]));
    free(mixer_str);
    return ret;
}


static int set_metadata(struct mixer *mixer, int device, uint32_t val,
                 enum stream_type stype, char *intf_name,
                 mcs_play_t *mcs_param)
{
    char *stream = "PCM";
    char *control = "metadata";
    char *mixer_str;
    struct mixer_ctl *ctl;
    uint8_t *metadata = NULL;
    struct agm_key_value *gkv = NULL, *ckv = NULL;
    struct prop_data *prop = NULL;
    uint32_t num_gkv = 1, num_ckv = 1, num_props = 0;
    uint32_t gkv_size, ckv_size, prop_size, index = 0;
    int ctl_len = 0, ret = 0, offset = 0;
    char *type = "ZERO";

    if (intf_name)
        type = intf_name;

    ret = set_agm_stream_metadata_type(device, type, stype);
    if (ret)
        return ret;

    num_gkv = mcs_param->graph_key_vector.num_kvps;
    gkv_size = num_gkv * sizeof(struct agm_key_value);
    ckv_size = num_ckv * sizeof(struct agm_key_value);
    prop_size = sizeof(struct prop_data) + (num_props * sizeof(uint32_t));

    metadata = calloc(1, sizeof(num_gkv) + sizeof(num_ckv) + gkv_size +
                                                 ckv_size + prop_size);
    if (!metadata) {
        AR_LOG_ERR(LOG_TAG, "%s: memory allocation failed.", __func__);
        ret = -ENOMEM;
        goto err_ret;
    }

    ckv = calloc(num_ckv, sizeof(struct agm_key_value));
    if (!ckv) {
        AR_LOG_ERR(LOG_TAG, "%s: memory allocation failed.", __func__);
        ret = -ENOMEM;
        goto free_metadata;
    }

    prop = calloc(1, prop_size);
    if (!prop) {
        AR_LOG_ERR(LOG_TAG, "%s: memory allocation failed.", __func__);
        ret = -ENOMEM;
        goto free_ckv;
    }

    index = 0;
    ckv[index].key = VOLUME;
    ckv[index].value = LEVEL_0;

    prop->prop_id = 0;  //Update prop_id here
    prop->num_values = num_props;

    memcpy(metadata, &num_gkv, sizeof(num_gkv));
    offset += sizeof(num_gkv);
    memcpy(metadata + offset, mcs_param->graph_key_vector.kvp, gkv_size);
    offset += gkv_size;
    memcpy(metadata + offset, &num_ckv, sizeof(num_ckv));

    offset += sizeof(num_ckv);
    memcpy(metadata + offset, ckv, ckv_size);
    offset += ckv_size;
    memcpy(metadata + offset, prop, prop_size);

    ctl_len = strlen(stream) + 4 + strlen(control) + 1;
    /* Here the value 4 represents the lenght of the device_id passed*/

    mixer_str = calloc(1, ctl_len);
    if (!mixer_str) {
        AR_LOG_ERR(LOG_TAG, "%s: memory allocation failed.", __func__);
        ret = -ENOMEM;
        goto free_prop;
    }
    snprintf(mixer_str, ctl_len, "%s%d %s", stream, device, control);

    AR_LOG_VERBOSE(LOG_TAG, "%s - mixer -%s-\n", __func__, mixer_str);
    ctl = mixer_get_ctl_by_name(mixer, mixer_str);
    if (!ctl) {
        AR_LOG_ERR(LOG_TAG, "Invalid mixer control: %s\n", mixer_str);
        ret = -ENOENT;
        goto free_mixer_str;
    }

    ret = mixer_ctl_set_array(ctl, metadata, sizeof(num_gkv) + sizeof(num_ckv) +
                                             gkv_size + ckv_size + prop_size);

free_mixer_str:
    free(mixer_str);
free_prop:
    free(prop);
free_ckv:
    free(ckv);
free_metadata:
    free(metadata);
err_ret:
    return ret;
}


static void mcs_record_sample(void *txt)
{
    AR_LOG_VERBOSE(LOG_TAG, "enter : txt %pK", txt);
    struct mcs_rec_ctxt* ctxt = (struct mcs_rec_ctxt*)txt;
    char * buffer;
    int status;
    size_t size, num_read;
    int total_bytes_to_record = 0;
    int bytes_count = 0;
    ssize_t bytes_read = 0;
    int replay = 0;

    if (ctxt->cur_param.record_duration_sec > 0) {
        total_bytes_to_record = ctxt->cur_param.record_duration_sec *
                    ctxt->cur_param.stream_properties.sample_rate *
                    ctxt->cur_param.stream_properties.num_channels *
                    ctxt->cur_param.stream_properties.bit_width / 8;
        replay = 1;
    }

    size = REC_BUF_SIZE * NO_OF_BUFS;
    buffer = malloc(size);
    if (!buffer) {
        AR_LOG_ERR(LOG_TAG, "Unable to allocate %d bytes\n",size);
        goto fileclose;
    }

    while (ctxt->cur_state == MCS_STATE_RUNNING) {
        bytes_read = pcm_read(ctxt->pcm, buffer, size);
        if (ctxt->cur_param.write_to_file == 1) {
            AR_LOG_INFO(LOG_TAG, "bytes recorded =%d ", bytes_read);
            status = ar_fwrite(ctxt->file_handle, buffer, size, &num_read);
            AR_LOG_INFO(LOG_TAG, "ar_fread file handle %pK", ctxt->file_handle);
            bytes_count += num_read;
            if ((ctxt->cur_param.record_duration_sec > 0) &&
                (bytes_count >= total_bytes_to_record)) {
                AR_LOG_INFO(LOG_TAG, "total bytes recorded =%d ", bytes_count);
                break;
            }
        }
    }

    AR_LOG_VERBOSE(LOG_TAG, "exit");
    if (buffer)
        free(buffer);
    return;
fileclose:
    if (ctxt->file_handle)
        ar_fclose(ctxt->file_handle);

}

static int32_t mcs_record_close(struct mcs_rec_ctxt* ctxt)
{
    AR_LOG_VERBOSE(LOG_TAG, "enter - ctxt %pK", ctxt);
    int status = 0;
    if (ctxt == NULL) {
        AR_LOG_ERR(LOG_TAG, " null pointer");
        return AR_EBADPARAM;
    }
    status = pcm_stop(ctxt->pcm);
    if (status != 0) {
        AR_LOG_ERR(LOG_TAG, "stream stop failed");
    }
    status = pcm_close(ctxt->pcm);
    if (status != 0) {
        AR_LOG_ERR(LOG_TAG, "stream close failed");
    }
    if (ctxt->cur_param.write_to_file == 1) {
        if (ctxt->file_handle != NULL) {
            status = ar_fclose(ctxt->file_handle);
        }
    }

    audio_route_reset_and_update_path(mcs_info->audio_route,
                                            ctxt->snd_device_name);
    AR_LOG_INFO(LOG_TAG, "exit");
    return status;
}

int32_t process_record_request(uint8_t cmd,
                                struct mcs_rec_ctxt* ctxt,
                                mcs_record_t *param)
{
    AR_LOG_INFO(LOG_TAG, "enter - ctxt %pK", ctxt);
    AR_LOG_INFO(LOG_TAG, "enter cmd = %x", cmd);
    int ret = 0;
    int no_of_devices = 0;
    int i = 0;
    int j = 0;
    void *tid1 = NULL;
    void *tid2 = NULL;
    int no_of_kv_pairs = 0;
    unsigned int sample_rate = 44100;
    unsigned int channels = 1;
    unsigned int bit_width = 16;

    if (ctxt == NULL) {
        AR_LOG_ERR(LOG_TAG, "%s:%d null pointer", __func__, __LINE__);
        return AR_EBADPARAM;
    }

    switch(cmd) {
        case MCS_START:
            AR_LOG_INFO(LOG_TAG, "MCS Record Start");
            if (param == NULL || param->graph_key_vector.kvp == NULL) {
                AR_LOG_ERR(LOG_TAG, "%s:%d null pointer", __func__, __LINE__);
                return AR_EBADPARAM;
            }
            if (ctxt->cur_state == MCS_STATE_IDLE) {
                memcpy(&ctxt->cur_param, param, sizeof(ctxt->cur_param));
                no_of_kv_pairs = param->graph_key_vector.num_kvps;
                for (i = 0; i < no_of_kv_pairs; i++) {
                    if (param->graph_key_vector.kvp[i].key == STREAMTX) {
                        sample_rate = param->device_properties.sample_rate;
                        bit_width = param->device_properties.bit_width;
                        channels = param->device_properties.num_channels;
                    }
                    else if (param->graph_key_vector.kvp[i].key == DEVICETX) {
                        no_of_devices++;
                        ctxt->device = mcs_get_device_id(param->graph_key_vector.kvp[i].key);
                        if (ctxt->device < 0) {
                            AR_LOG_ERR(LOG_TAG, "error in getting device_id");
                            return ctxt->device;
                        }
                        ret = mcs_get_intf_name(param->graph_key_vector.kvp[i].value,
                                                              ctxt->intf_name);
                        if (ret) {
                            AR_LOG_ERR(LOG_TAG, "error in getting audio_intf name");
                            return ret;
                        }
                        ret = mcs_get_snd_device_name(param->graph_key_vector.kvp[i].value,
                                                       ctxt->snd_device_name);
                        if (ret) {
                            AR_LOG_ERR(LOG_TAG, "error in getting snd device name");
                            return AR_EFAILED;
                        }
                    }
                }
            }
            else {
                AR_LOG_ERR(LOG_TAG, " current state is not proper");
                ret = -EINVAL;
                return ret;
            }
            if (set_device_media_config(mcs_info->mixer, channels, sample_rate,
                                               bit_width, ctxt->intf_name)) {
                AR_LOG_INFO(LOG_TAG, "Failed to set device media config\n");
                return AR_EFAILED;
            }

          /* set audio interface metadata mixer control */
            if (set_metadata(mcs_info->mixer, ctxt->device, PCM_RECORD,
                                                 STREAM_PCM, NULL, param)) {
                AR_LOG_INFO(LOG_TAG, "Failed to set pcm metadata\n");
                return AR_EFAILED;
            }

            if (connect_audio_intf_to_stream(mcs_info->mixer, ctxt->device,
                                       ctxt->intf_name, STREAM_PCM, true)) {
                AR_LOG_ERR(LOG_TAG, "Failed to connect pcm to audio interface\n");
                return AR_EFAILED;
            }
            ret = mcs_record_prepare(ctxt);
            if (ret == 0) {
                ctxt->cur_state = MCS_STATE_RUNNING;
                ret = ar_osal_thread_create(&mcs_info->rec_ctxt->tid,
                                              &mcs_info->rec_ctxt->tattr,
                                              mcs_record_sample, (void *)ctxt);
                if (ret != 0) {
                    ctxt->cur_state = MCS_STATE_IDLE;
                    AR_LOG_ERR(LOG_TAG, "Unable to create thread for record");
                }
            }
            else {
                AR_LOG_ERR(LOG_TAG, "Record Start failed\n");
            }
            break;

        case MCS_STOP:
            AR_LOG_INFO(LOG_TAG, "MCS Record Stop");
            if (ctxt->cur_state != MCS_STATE_IDLE) {
                AR_LOG_INFO(LOG_TAG, "%s:%d", __func__, __LINE__);
                ctxt->cur_state = MCS_STATE_STOPPING;
                ret = mcs_record_close(ctxt);
                ret = ar_osal_thread_join_destroy(mcs_info->rec_ctxt->tid);
                ctxt->cur_state = MCS_STATE_IDLE;
                AR_LOG_INFO(LOG_TAG, "%s:%d Record stopped", __func__, __LINE__);
                return ret;
            }
            break;
        default:
            AR_LOG_ERR(LOG_TAG, "Invalid command");
            ret = AR_EFAILED;
            break;
    }
    AR_LOG_VERBOSE(LOG_TAG, "exit");
    return ret;
}

int32_t mcs_stream_cmd(uint32_t cmd, uint8_t *cmd_buf,
                     uint32_t cmd_buf_size, uint8_t*rsp_buf,
                     uint32_t rsp_buf_size, uint32_t *rsp_buf_bytes_filled)
{
    AR_LOG_INFO(LOG_TAG, "Enter %x cmd", cmd);
    int ret = 0;
    mcs_play_rec_t *pdata;
    ar_osal_mutex_lock(mcs_info->lock);

    switch(cmd) {
        case MCS_CMD_PLAY:
            AR_LOG_INFO(LOG_TAG, "MCS_CMD_PLAY");
            ret = process_playback_request(MCS_START, mcs_info->pb_ctxt,
                                    (mcs_play_t *) cmd_buf);
            break;
        case MCS_CMD_REC:
            AR_LOG_INFO(LOG_TAG, "MCS_CMD_REC");
            ret = process_record_request(MCS_START, mcs_info->rec_ctxt,
                                    (struct mcs_record_t*) cmd_buf);
            break;
        case MCS_CMD_PLAY_REC:
            pdata = (struct mcs_play_rec_t *)cmd_buf;
            ret = process_playback_request(MCS_START, mcs_info->pb_ctxt,
                                            &pdata->playback_session);
            if (ret == 0) {
                ret = process_record_request(MCS_START, mcs_info->rec_ctxt,
                                             &pdata->record_session);
                if (ret != 0) {
                    process_playback_request(MCS_STOP, mcs_info->pb_ctxt,
                                              &pdata->playback_session);
                }
            }
            break;
        case MCS_CMD_STOP:
            AR_LOG_INFO(LOG_TAG, "MCS_CMD_STOP");
            ret = process_playback_request(MCS_STOP, mcs_info->pb_ctxt,
                                           NULL);
            if (ret == 0)
                ret = process_record_request(MCS_STOP, mcs_info->rec_ctxt,
                                                NULL);
            break;

        default:
            AR_LOG_ERR(LOG_TAG, "%s: invalid command ID from ATS: 0x%x\n",
                                                            __func__, cmd);
            ret = AR_EFAILED;
            break;
    }
    ar_osal_mutex_unlock(mcs_info->lock);
    AR_LOG_INFO(LOG_TAG, "exit");
    return ret;
}

int32_t mcs_init(void)
{
    int ret = 0;
    int retry = 0;
    bool snd_card_found = false;
    char snd_macro[] = "snd";
    char *snd_card_name = NULL, *snd_card_name_t = NULL;
    char *snd_internal_name = NULL;
    char *tmp = NULL;
    char mixer_xml_file[MIXER_PATH_MAX_LENGTH] = {0};
    char vendor_config_path[VENDOR_CONFIG_PATH_MAX_LENGTH] = {0};
    char file_name_extn[XML_PATH_EXTN_MAX_SIZE] = {0};
    int snd_card;
    struct audio_mixer* tmp_mixer = NULL;

    do {
        /* Look for only default codec sound card */
        /* Ignore USB sound card if detected */
        snd_card = 0;
        while (snd_card < MAX_SND_CARD) {
            tmp_mixer = NULL;
            tmp_mixer = mixer_open(snd_card);
            if (tmp_mixer) {
                snd_card_name = strdup(mixer_get_name(tmp_mixer));
                if (!snd_card_name) {
                    AR_LOG_ERR(LOG_TAG, "failed to allocate memory for snd_card_name");
                    mixer_close(tmp_mixer);
                    ret = -EINVAL;
                    goto err_mcs;
                }
                AR_LOG_INFO(LOG_TAG, "mixer_open success. snd_card_num = %d, snd_card_name %s",
                snd_card, snd_card_name);

                /* TODO: Needs to extend for new targets */
                if (strstr(snd_card_name, "kona") ||
                    strstr(snd_card_name, "sm8150") ||
                    strstr(snd_card_name, "lahaina") ||
                    strstr(snd_card_name, "waipio") ||
                    strstr(snd_card_name, "diwali") ||
                    strstr(snd_card_name, "bengal") ||
                    strstr(snd_card_name, "monaco")) {
                    AR_LOG_VERBOSE(LOG_TAG, "Found Codec sound card");
                    snd_card_found = true;
                    audio_mixer = tmp_mixer;
                    break;
                } else {
                    if (snd_card_name) {
                        free(snd_card_name);
                        snd_card_name = NULL;
                    }
                    mixer_close(tmp_mixer);
                }
            }
            snd_card++;
        }

        if (!snd_card_found) {
            AR_LOG_INFO(LOG_TAG, "No audio mixer, retry %d", retry++);
            sleep(1);
        }
    } while (!snd_card_found && retry <= MAX_RETRY_CNT);


    if (snd_card >= MAX_SND_CARD || !audio_mixer) {
        AR_LOG_ERR(LOG_TAG, "audio mixer open failure");
        ret = -EINVAL;
        goto err_snd_name;
    }

    AR_LOG_INFO(LOG_TAG, "enter");
    mcs_info = calloc(1, sizeof(struct acdb_mcs));
    if (mcs_info == NULL) {
        AR_LOG_ERR(LOG_TAG, "%s: memory allocation failed.", __func__);
        ret = -AR_ENOMEMORY;
        goto err_snd_name;
    }

    mcs_info->pb_ctxt = calloc(1, sizeof(struct mcs_play_ctxt));
    if (mcs_info->pb_ctxt == NULL) {
        AR_LOG_ERR(LOG_TAG, "%s: memory allocation failed.", __func__);
        ret = -AR_ENOMEMORY;
        goto err_pb_ctxt;
    }
    mcs_info->pb_ctxt->cur_state = MCS_STATE_IDLE;

    mcs_info->rec_ctxt = calloc(1, sizeof(struct mcs_rec_ctxt));
    if (mcs_info->rec_ctxt == NULL) {
        AR_LOG_ERR(LOG_TAG, "%s: memory allocation failed.", __func__);
        ret = -AR_ENOMEMORY;
        goto err_rec_ctxt;
    }
    mcs_info->rec_ctxt->cur_state = MCS_STATE_IDLE;

    mcs_info->mixer = mixer_open(SND_CARD_VIRTUAL);
    if (!(mcs_info->mixer)) {
        AR_LOG_INFO(LOG_TAG, "Failed to open mixer\n");
        ret = -EINVAL;
        goto err_ret;
    }

    if (snd_card_name) {
        snd_card_name_t = strdup(snd_card_name);
    }
    if (!snd_card_name_t) {
        AR_LOG_ERR(LOG_TAG, "failed to allocate memory for snd_card_name_t");
        ret = -EINVAL;
        goto err_mixer_close;
    }
    snd_internal_name = strtok_r(snd_card_name_t, "-", &tmp);

    if (snd_internal_name != NULL)
        snd_internal_name = strtok_r(NULL, "-", &tmp);

    getFileNameExtn(snd_card_name, file_name_extn);

    getVendorConfigPath(vendor_config_path, sizeof(vendor_config_path));

    /* Get path for mixer_xml_path_name in vendor */
    snprintf(mixer_xml_file, sizeof(mixer_xml_file),
             "%s/%s", vendor_config_path, MIXER_XML_BASE_STRING_NAME);

     strlcat(mixer_xml_file, MIXER_FILE_DELIMITER, MIXER_PATH_MAX_LENGTH);
     strlcat(mixer_xml_file, file_name_extn, MIXER_PATH_MAX_LENGTH);

    strlcat(mixer_xml_file, MIXER_FILE_EXT, MIXER_PATH_MAX_LENGTH);

    mcs_info->audio_route = audio_route_init(snd_card, mixer_xml_file);

    AR_LOG_INFO(LOG_TAG, "audio route %pK, mixer path %s",
                     mcs_info->audio_route, mixer_xml_file);
    if (!mcs_info->audio_route) {
        AR_LOG_ERR(LOG_TAG, "audio route init failed");
        ret = -EINVAL;
        goto err_audio_route;
    }
    ar_osal_mutex_create(&mcs_info->lock);

    ar_osal_thread_attr_init(&mcs_info->pb_ctxt->tattr);

    ar_osal_thread_attr_init(&mcs_info->rec_ctxt->tattr);

    mcs_info->pb_ctxt->tid = NULL;
    mcs_info->rec_ctxt->tid = NULL;
    if (snd_card_name_t) {
        free(snd_card_name_t);
        snd_card_name_t = NULL;
    }
    if (snd_card_name) {
        free(snd_card_name);
        snd_card_name = NULL;
    }
    AR_LOG_INFO(LOG_TAG, "exit");
    return 0;

err_audio_route:
    if (snd_card_name_t) {
        free(snd_card_name_t);
        snd_card_name_t = NULL;
    }
err_mixer_close:
    if(mcs_info->mixer) {
        mixer_close(mcs_info->mixer);
        mcs_info->mixer = NULL;
    }
err_ret:
    if(mcs_info->rec_ctxt) {
        free(mcs_info->rec_ctxt);
        mcs_info->rec_ctxt = NULL;
    }
err_rec_ctxt:
    if(mcs_info->pb_ctxt) {
        free(mcs_info->pb_ctxt);
        mcs_info->pb_ctxt = NULL;
    }
err_pb_ctxt:
    if(mcs_info) {
        free(mcs_info);
        mcs_info = NULL;
    }
err_snd_name:
    if (snd_card_name) {
        free(snd_card_name);
        snd_card_name = NULL;
    }
    if (audio_mixer) {
        mixer_close(audio_mixer);
        audio_mixer = NULL;
    }
err_mcs:
    return ret;
}

int32_t ats_mcs_deinit(void)
{
    int ret = 0;

    AR_LOG_INFO(LOG_TAG, "Enter");

    if(!mcs_info) {
        ret = AR_EBADPARAM;
        return ret;
    }
    audio_route_free(mcs_info->audio_route);

    ar_osal_mutex_destroy(mcs_info->lock);

    mixer_close(mcs_info->mixer);

    if (audio_mixer)
        mixer_close(audio_mixer);

    if (mcs_info->rec_ctxt) {
        free(mcs_info->rec_ctxt);
        mcs_info->rec_ctxt = NULL;
    }

    if (mcs_info->pb_ctxt) {
        free(mcs_info->pb_ctxt);
        mcs_info->pb_ctxt = NULL;
    }

    if (mcs_info) {
        free(mcs_info);
        mcs_info = NULL;
    }

    AR_LOG_INFO(LOG_TAG, "Exit");
    return ret;
}

/* Function to get audio vendor configs path */
void getVendorConfigPath (char* config_file_path, int path_size)
{
    char vendor_sku[PROPERTY_VALUE_MAX] = {'\0'};

    if (property_get("ro.boot.product.vendor.sku", vendor_sku, "") <= 0) {
#if defined(FEATURE_IPQ_OPENWRT) || defined(LINUX_ENABLED)
       /*Audio configs are stored in /etc */
       snprintf(config_file_path, path_size, "%s", "/etc");
#else
       /* Audio configs are stored in /vendor/etc */
       snprintf(config_file_path, path_size, "%s", "/vendor/etc");
#endif
    } else {
       /* Audio configs are stored in /vendor/etc/audio/sku_${vendor_sku} */
       snprintf(config_file_path, path_size,
                       "%s%s", "/vendor/etc/audio/sku_", vendor_sku);
    }
}

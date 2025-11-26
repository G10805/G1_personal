/* Copyright (c) 2020, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _ACDB_LOADER_OPT_H
#define _ACDB_LOADER_OPT_H

#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <cutils/list.h>

#if defined(__KERNEL__) || defined(__linux__)
#include <linux/types.h>
#else
#include <sys/ioctl.h>
#endif
#include <linux/msm_audio_calibration.h>


#ifdef __KERNEL__
#include <stdlib.h>
#include <time.h>
#endif

#ifdef _ANDROID_
#include <cutils/properties.h>
/* definitions for Android logging */
#include <utils/Log.h>
#include "common_log.h"
#else
#undef ALOGI
#undef ALOGE
#undef ALOGW
#undef ALOGV
#undef ALOGD

#define ALOGI(...)     fprintf(stdout,__VA_ARGS__);\
                       fprintf(stdout, "\n")
#define ALOGE(...)     fprintf(stderr,__VA_ARGS__);\
                       fprintf(stderr, "\n")
#define ALOGW(...)     do { } while(0)
#ifdef ACDB_LOADER_DEBUG
#define ALOGV(...)     fprintf(stderr,__VA_ARGS__);\
                       fprintf(stderr, "\n")
#define ALOGD(...)     fprintf(stderr,__VA_ARGS__);\
                       fprintf(stderr, "\n")
#else
#define ALOGV(...)     ((void)0)
#define ALOGD(...)     ((void)0)
#endif /* ACDB_LOADER_DEBUG */
#endif /* _ANDROID_ */

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef memscpy
#define memscpy(dst, dst_size, src, bytes_to_copy) (void) memcpy(dst, src, MIN(dst_size, bytes_to_copy))
#endif

typedef struct acdb_data_buf {
    int          msgID;
    unsigned int hdrSize;
    unsigned int bufPos;
    unsigned int readPos;
    unsigned int bufSize;
    char         data[2048];
}acdb_data_buf_t;

struct acdb_init_data_v2 {
    char        *cvd_version;
    char        *snd_card_name;
    struct listnode    *meta_key_list;
    bool        *is_instance_id_supported;
};

struct param_data {
    int         use_case;
    int         acdb_id;
    int         get_size;
    int         buff_size;
    int         data_size;
    void        *buff;
};

typedef struct acdb_audio_cal_cfg {
    uint32_t    persist;
    uint32_t    snd_dev_id;
    uint32_t    dev_id;
    int32_t     acdb_dev_id;
    uint32_t    app_type;
    uint32_t    topo_id;
    uint32_t    sampling_rate;
    uint32_t    cal_type;
    uint32_t    module_id;
    uint16_t    instance_id;
    uint16_t    reserved;
    uint32_t    param_id;
} acdb_audio_cal_cfg_t;

struct meta_key_list {
    struct listnode     list;
    struct audio_cal_info_metainfo  cal_info;
};

struct acdb_init_data_v4 {
    char        *cvd_version;
    char        *snd_card_name;
    struct listnode     *meta_key_list;
    bool        *is_instance_id_supported;
};

enum {
    ACDB_LOADER_INIT_V1 = 1,
    ACDB_LOADER_INIT_V2,
    ACDB_LOADER_INIT_V3,
    ACDB_LOADER_INIT_V4,
};

enum{
    MSG_INIT_V2 = 50,
    MSG_INIT_V3,
    MSG_INIT_V4,
    MSG_INIT_ACDB,
    MSG_GET_DEF_APP_TYPE,
    MSG_SEND_COM_CUSTOM_TOP,
    MSG_DEALLOCATE_ACDB,
    MSG_DEALLOCATE_CAL,
    MSG_SEND_VOICE_CAL_V2,
    MSG_SEND_VOICE_CAL,
    MSG_RELOAD_VOICE_VOL_TABLE,
    MSG_SEND_AUDIO_CAL,
    MSG_SEND_AUDIO_CAL_V2,
    MSG_SEND_AUDIO_CAL_V3,
    MSG_SEND_AUDIO_CAL_V4,
    MSG_SEND_AUDIO_CAL_V6,
    MSG_SEND_LISTEN_DEV_CAL,
    MSG_SEND_LISTEN_LSM_CAL,
    MSG_SEND_LISTEN_LSM_CAL_V1,
    MSG_SEND_ANC_CAL,
    MSG_SEND_ANC_DATA,
    MSG_GET_AUD_VOL_STEPS,
    MSG_SEND_GAIN_DEP_CAL,
    MSG_GET_REMOTE_ACDB_ID,
    MSG_GET_ECRX_DEVICE,
    MSG_GET_CALIBRATION,
    MSG_SET_AUDIO_CAL_V2,
    MSG_GET_AUDIO_CAL_V2,
    MSG_SEND_META_INFO,
    MSG_SEND_META_INFO_LIST,
    MSG_SET_CODEC_DATA,
    MSG_IS_INITIALIZED,
    MSG_RELOAD_ACDB_FILES,
    MSG_RELOAD_ACDB_FILES_V2
};

typedef void (*acdb_loader_callback)(void* context);

typedef int  (*acdb_init_v2_t)(const char *, char *, int);
typedef int  (*acdb_init_v3_t)(const char *, char *, struct listnode *);
typedef int  (*acdb_init_v4_t)(void *, int);
typedef int  (*acdb_init_t)(void);
typedef int  (*acdb_get_default_app_type_t)(void);
typedef int  (*acdb_send_common_custom_topology_t)(void);
typedef void (*acdb_deallocate_t)(void);
typedef void (*acdb_deallocate_cal_t)(int, int);
typedef void (*acdb_send_voice_cal_v2_t)(int, int, int);
typedef void (*acdb_send_voice_cal_t)(int, int);
typedef int  (*acdb_reload_vocvoltable_t)(int);
typedef void (*acdb_send_audio_cal_t)(int, int);
typedef void (*acdb_send_audio_cal_v2_t)(int, int, int, int);
typedef void (*acdb_send_audio_cal_v3_t)(int, int, int, int, int);
typedef void (*acdb_send_audio_cal_v4_t)(int, int, int, int, int, int);
typedef void (*acdb_send_audio_cal_v6_t)(int, int, int, int, int, int, int, int);
typedef void (*acdb_send_listen_device_cal_t)(int, int, int, int);
typedef int  (*acdb_send_listen_lsm_cal_t)(int, int, int, int);
typedef int  (*acdb_send_listen_lsm_cal_v1_t)(int, int, int, int, int);
typedef int  (*acdb_send_anc_cal_t)(int);
typedef void (*acdb_send_tabla_anc_data_t)(void);
typedef int (*acdb_get_aud_volume_steps_t)(void);
typedef int (*acdb_send_gain_dep_cal_t)(int, int, int, int, int);
typedef int (*acdb_get_remote_acdb_id_t)(unsigned int);
typedef int (*acdb_get_ecrx_device_t)(int);
typedef int (*acdb_get_calibration_t)(char *, int, void *);
typedef int (*acdb_set_audio_cal_v2_t)(void *, void*, unsigned int);
typedef int (*acdb_get_audio_cal_v2_t)(void *, void*, unsigned int *);
typedef int (*acdb_send_meta_info_t)(int, int);
typedef int (*acdb_send_meta_info_list_t)(struct listnode *);
typedef int (*acdb_set_codec_data_t)(void *, char *);
typedef bool (*acdb_is_initialized_t)(void);
typedef int (*acdb_reload_acdb_files_t)(char *, char *, char *, int);
typedef int (*acdb_reload_acdb_files_v2_t)(char *, char *, char *, struct listnode *);

typedef struct acdb_service_data {
    void                               *acdb_handle;
    acdb_init_v2_t                     acdb_init_v2;
    acdb_init_v3_t                     acdb_init_v3;
    acdb_init_v4_t                     acdb_init_v4;
    acdb_init_t                        acdb_init;
    acdb_get_default_app_type_t        acdb_get_default_app_type;
    acdb_send_common_custom_topology_t acdb_send_common_custom_topology;
    acdb_deallocate_t                  acdb_deallocate;
    acdb_deallocate_cal_t              acdb_deallocate_cal;
    acdb_send_voice_cal_v2_t           acdb_send_voice_cal_v2;
    acdb_send_voice_cal_t              acdb_send_voice_cal;
    acdb_reload_vocvoltable_t          acdb_reload_vocvoltable;
    acdb_send_audio_cal_t              acdb_send_audio_cal;
    acdb_send_audio_cal_v2_t           acdb_send_audio_cal_v2;
    acdb_send_audio_cal_v3_t           acdb_send_audio_cal_v3;
    acdb_send_audio_cal_v4_t           acdb_send_audio_cal_v4;
    acdb_send_audio_cal_v6_t           acdb_send_audio_cal_v6;
    acdb_send_listen_device_cal_t      acdb_send_listen_device_cal;
    acdb_send_listen_lsm_cal_t         acdb_send_listen_lsm_cal;
    acdb_send_listen_lsm_cal_v1_t      acdb_send_listen_lsm_cal_v1;
    acdb_send_anc_cal_t                acdb_send_anc_cal;
    acdb_send_tabla_anc_data_t         acdb_send_tabla_anc_data;
    acdb_get_aud_volume_steps_t        acdb_get_aud_volume_steps;
    acdb_send_gain_dep_cal_t           acdb_send_gain_dep_cal;
    acdb_get_remote_acdb_id_t          acdb_get_remote_acdb_id;
    acdb_get_ecrx_device_t             acdb_get_ecrx_device;
    acdb_get_calibration_t             acdb_get_calibration;
    acdb_set_audio_cal_v2_t            acdb_set_audio_cal_v2;
    acdb_get_audio_cal_v2_t            acdb_get_audio_cal_v2;
    acdb_send_meta_info_t              acdb_send_meta_info;
    acdb_send_meta_info_list_t         acdb_send_meta_info_list;
    acdb_set_codec_data_t              acdb_set_codec_data;
    acdb_is_initialized_t              acdb_is_initialized;
    acdb_reload_acdb_files_t           acdb_reload_acdb_files;
    acdb_reload_acdb_files_v2_t        acdb_reload_acdb_files_v2;
}acdb_service_data;

int writeInt(int32_t val, acdb_data_buf_t* data_buf)
{
    int ret = -1;
    char * pbuf = NULL;
    if(data_buf == NULL)
        return ret;

    pbuf = data_buf->data;
    if ((data_buf->bufPos+sizeof(val)) <= data_buf->bufSize) {
        *(int32_t *)(pbuf + data_buf->bufPos) = val;
        data_buf->bufPos += sizeof(int32_t);
        //printf("writeInt new pos=%d, size=%d val %d\n", data_buf->bufPos, data_buf->bufSize, val);
        return 0;
    }
    return ret;
}

int writeBool(bool val, acdb_data_buf_t* data_buf)
{
    return writeInt(val, data_buf);
}

int writeString(const char* str, size_t len, acdb_data_buf_t* data_buf)
{
    if (data_buf == NULL) return -1;
    if (str == NULL) return writeInt(0, data_buf);

    //printf("writeString str %s len %d\n", str, len);

    int ret = writeInt(len, data_buf);
    if (ret == 0) {
        len *= sizeof(char);
        uint8_t* data = (uint8_t*)data_buf->data + data_buf->bufPos;
        if (data) {
            memscpy(data, data_buf->bufSize - data_buf->bufPos, str, len);
            data_buf->bufPos += MIN((data_buf->bufSize - data_buf->bufPos), len);
            //printf("writeString new pos %d len %d\n", data_buf->bufPos, len);
            return 0;
        }
        ret = -1;
    }
    return ret;
}

int readInt(int32_t *pArg, acdb_data_buf_t * data_buf)
{
    if (data_buf == NULL || pArg == NULL)
        return -1;
    if ((data_buf->readPos+sizeof(int32_t)) <= data_buf->bufPos) {
        const void* data = data_buf->data + data_buf->readPos;
        data_buf->readPos += sizeof(int32_t);
        *pArg =  *(int32_t *)data;
        //printf("recvDataPos %d *pArg %d\n", data_buf->readPos, *pArg);
        return 0;
    } else
        return -1;
}

int readBool(bool *pArg, acdb_data_buf_t * data_buf)
{
    int32_t tmp = 0;
    int ret = readInt(&tmp, data_buf);
    *pArg = (tmp != 0);
    return ret;
}

int readString(char* outData, size_t dst_size, int* len, acdb_data_buf_t * data_buf)
{
    if (data_buf == NULL || len == NULL || outData == NULL) return -1;
    int ret = readInt(len, data_buf);
    //printf("readString len %d \n", *len);

    if (ret == 0) {
        const size_t avail = data_buf->bufPos-data_buf->readPos;
        if(avail < 0 || *len > avail)
            return -1;
        if(*len > 0)
            memscpy(outData, dst_size, data_buf->data+data_buf->readPos, *len);
        data_buf->readPos += *len;
        *len = MIN(dst_size, *len);
        return 0;
    } else
        return -1;
}
#endif

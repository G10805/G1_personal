/* Copyright (c) 2020, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

/*
 * acdb_loader_server
 *
 * Manages multiple acdb loader clients
 * This is implemented as a Singleton Thread !
 * First client that requests to be initialized, instantiates acdb_loader_server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <linux/un.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stringl.h>
#include <dlfcn.h>
#include <ctype.h>
#include <fcntl.h>
#ifdef ANDROID
#include <cutils/sockets.h>
#endif

#include "acdb-loader-server-client-v2.h"
#include "acdb.h"
#include "acdb-loader.h"
#include "anc_map_api.h"
#include "vbat_map_api.h"

#ifndef _ANDROID_
#define strlcat g_strlcat
#define strlcpy g_strlcpy
#endif

#define SV_ACDB_SOCK_PATH "/var/lib/shared/acdb"
#define SV_SOCK_PATH_HOST "/var/lib/shared/acdb/acdb"
#define LIB_ACDB_LOADER "libacdbloader.so"

#ifndef FD_SETSIZE
#define FD_SETSIZE 1024
#endif

#define BUF_SIZE 2048
#define NUM_LISTEN_QUEUE 5

acdb_service_data * mydata = NULL;
static bool init_status;
static int sockfd_server_host = -1;
static int refcount = 0;

void acdb_loader_send_reply(int fd, struct sockaddr_un claddr, int length, acdb_data_buf_t * reply_buf) {
    ALOGV("%s: length %d sun_path %s", __func__, length, claddr.sun_path);

    struct sockaddr_un svaddr;
    memset(&svaddr, 0, sizeof(struct sockaddr_un));
    svaddr.sun_family = AF_LOCAL;

    if (strncmp(claddr.sun_path, "/vendor/", 8) == 0) {
        strlcpy(svaddr.sun_path, claddr.sun_path +7 , sizeof(svaddr.sun_path) - 8);
        ALOGD("socket from android: svaddr.sun_path %s", svaddr.sun_path);
    } else {
        strlcpy(svaddr.sun_path, claddr.sun_path , sizeof(svaddr.sun_path) - 1);
        ALOGD("socket from linux: svaddr.sun_path %s", svaddr.sun_path);
    }

    if (fd < 0 || length < 0) {
        ALOGE("%s: Invalid socket or buffer length!", __func__);
        return;
    }

    if (sendto(fd, (void *)reply_buf, length, 0, (struct sockaddr *)&svaddr, sizeof(struct sockaddr_un)) != length) {
        ALOGE("%s: send fail error %s!!!", __func__, (char *)strerror(errno));
        return;
    }

    ALOGV("%s: msgID %d bufPos %d done", __func__, reply_buf->msgID, reply_buf->bufPos);
    return;
}

int acdb_loader_handle_message(int fd, struct sockaddr_un claddr, acdb_data_buf_t * data_buf, acdb_data_buf_t * reply_buf) {
    int ret = -1, msgID;
    if (data_buf == NULL || reply_buf == NULL) {
        ALOGE("%s: Invalid parameter!!", __func__);
        return ret;
    }
    msgID = data_buf->msgID;
    reply_buf->msgID = msgID;
    reply_buf->bufSize = BUF_SIZE;
    data_buf->readPos = 0;
    ALOGV("%s: handle message (%d)", __func__, msgID);

    switch(msgID) {
        case MSG_INIT_V2: {
            if (mydata->acdb_init_v2 != NULL) {
                char snd_card_name[64] = "";
                int  name_length = 0;
                char cvd_version[64] = "";
                int  cvd_length = 0;

                readString((char *)snd_card_name, 64, &name_length, data_buf);
                readString((char *)cvd_version, 64, &cvd_length, data_buf);
                if (name_length == 0 || cvd_length == 0) {
                    ALOGE("Bad socket transaction: MSG_INIT_V2 !!");
                    writeInt(-EINVAL, reply_buf);
                    acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                    return 0;
                }

                int metaInfoKey;
                readInt(&metaInfoKey, data_buf);
                ALOGD("%s INIT_V2: snd(%s), cvd(%s), metaInfoKey(%d)", __func__, snd_card_name, cvd_version, metaInfoKey);

                if (strcmp("null", snd_card_name) == 0) {
                    name_length = 0;
                }
                if (strcmp("null", cvd_version) == 0) {
                    cvd_length = 0;
                }
                ret = mydata->acdb_init_v2((name_length ? (char *)snd_card_name : NULL), (cvd_length ? (char*)cvd_version : NULL), metaInfoKey);
                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_INIT_V3: {
            if (mydata->acdb_init_v3 != NULL) {
                char snd_card_name[64] = "";
                int  name_length = 0;
                char cvd_version[64] = "";
                int  cvd_length = 0;

                readString((char *)snd_card_name, 64, &name_length, data_buf);
                readString((char *)cvd_version, 64, &cvd_length, data_buf);
                if (name_length == 0 || cvd_length == 0) {
                    ALOGE("Bad socket transaction: MSG_INIT_V3");
                    writeInt(-EINVAL, reply_buf);
                    acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                    return 0;
                }

                struct listnode key_list;
                struct meta_key_list key_info;
                readInt(&key_info.cal_info.nKey, data_buf);
                list_init(&key_list);
                list_add_tail(&key_list, &key_info.list);

                ALOGD("%s INIT_V3: snd(%s), cvd(%s), key_info = %u", __func__, snd_card_name, cvd_version, key_info.cal_info.nKey);
                if (strcmp("null", snd_card_name) == 0) {
                    name_length = 0;
                }
                if (strcmp("null", cvd_version) == 0) {
                    cvd_length = 0;
                }
                ret = mydata->acdb_init_v3((name_length ? (char *)snd_card_name : NULL), (cvd_length ? (char*)cvd_version : NULL), &key_list);
                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_INIT_V4: {
            if (mydata->acdb_init_v4 != NULL) {
                int32_t init_version = ACDB_LOADER_INIT_V4;
                char snd_card_name[64] = "";
                int  name_length = 0;
                char cvd_version[64] = "";
                int  cvd_length = 0;
                bool is_instance_id_supported;
                struct listnode key_list;
                struct meta_key_list key_info;
                struct acdb_init_data_v4   acdb_init_data;

                readString((char *)snd_card_name, 64, &name_length, data_buf);
                readString((char *)cvd_version, 64, &cvd_length, data_buf);
                if (name_length == 0 || cvd_length == 0) {
                    ALOGE("Bad socket transaction: MSG_INIT_V4");
                    writeInt(-EINVAL, reply_buf);
                    acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                    return 0;
                }
                ALOGD("%s INIT_V4: acdb_loader_init_version(%d), cvd_version = %s, snd_card_name = %s", __func__, init_version, cvd_version, snd_card_name);

                if (strcmp("null", snd_card_name) == 0) {
                    name_length = 0;
                }
                if (strcmp("null", cvd_version) == 0) {
                    cvd_length = 0;
                }
                readInt(&is_instance_id_supported, data_buf);

                readInt(&key_info.cal_info.nKey, data_buf);
                list_init(&key_list);
                list_add_tail(&key_list, &key_info.list);

                acdb_init_data.cvd_version = (cvd_length ? strdup(cvd_version) : NULL);
                acdb_init_data.snd_card_name  = (name_length ? strdup(snd_card_name) : NULL);
                acdb_init_data.meta_key_list = &key_list;
                acdb_init_data.is_instance_id_supported = &is_instance_id_supported;

                ret = mydata->acdb_init_v4((void*)&acdb_init_data, init_version);
                writeInt(ret, reply_buf);
                writeBool(is_instance_id_supported, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_INIT_ACDB: {
            if (mydata->acdb_init != NULL) {
                ALOGD("%s MSG_INIT_ACDB", __func__);
                ret = mydata->acdb_init();
                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_GET_DEF_APP_TYPE: {
            if (mydata->acdb_get_default_app_type != NULL) {
                ALOGD("%s MSG_GET_DEF_APP_TYPE", __func__);
                ret = mydata->acdb_get_default_app_type();
                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_SEND_COM_CUSTOM_TOP: {
            if (mydata->acdb_send_common_custom_topology != NULL) {
                ALOGD("%s SEND_COM_CUSTOM_TOP", __func__);
                ret = mydata->acdb_send_common_custom_topology();
                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_DEALLOCATE_ACDB: {
            if (mydata->acdb_deallocate != NULL) {
                ALOGD("%s DEALLOCATE_ACDB", __func__);
                mydata->acdb_deallocate();
                return 0;
            }
        }
        break;

        case MSG_DEALLOCATE_CAL: {
            if (mydata->acdb_deallocate_cal != NULL) {
                int capability;
                readInt(&capability, data_buf);
                int use_case;
                readInt(&use_case, data_buf);
                ALOGD("%s DEALLOCATE_CAL: capability(%d), use_case(%d)", __func__, capability, use_case);
                mydata->acdb_deallocate_cal(capability, use_case);
                return 0;
            }
        }
        break;

        case MSG_SEND_VOICE_CAL_V2: {
            if (mydata->acdb_send_voice_cal_v2 != NULL) {
                int rxacdb_id;
                readInt(&rxacdb_id, data_buf);
                int txacdb_id;
                readInt(&txacdb_id, data_buf);
                int feature_set;
                readInt(&feature_set, data_buf);
                ALOGD("%s SEND_VOICE_CAL_V2: rxacdb_id(%d), txacdb_id(%d), feature_set(%d)", __func__, rxacdb_id, txacdb_id, feature_set);
                mydata->acdb_send_voice_cal_v2(rxacdb_id, txacdb_id, feature_set);
                return 0;
            }
        }
        break;

        case MSG_SEND_VOICE_CAL: {
            if (mydata->acdb_send_voice_cal != NULL) {
                int rxacdb_id;
                readInt(&rxacdb_id, data_buf);
                int txacdb_id;
                readInt(&txacdb_id, data_buf);
                ALOGD("%s SEND_VOICE_CAL: rxacdb_id(%d), txacdb_id(%d)", __func__, rxacdb_id, txacdb_id);
                mydata->acdb_send_voice_cal(rxacdb_id, txacdb_id);
                return 0;
            }
        }
        break;

        case MSG_RELOAD_VOICE_VOL_TABLE: {
            if (mydata->acdb_reload_vocvoltable != NULL) {
                int feature_set;
                readInt(&feature_set, data_buf);
                ALOGD("%s RELOAD_VOICE_VOL_TABLE: feature_set(%d)", __func__, feature_set);
                ret = mydata->acdb_reload_vocvoltable(feature_set);
                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_SEND_AUDIO_CAL: {
            if (mydata->acdb_send_audio_cal != NULL) {
                int acdb_id;
                readInt(&acdb_id, data_buf);
                int capability;
                readInt(&capability, data_buf);
                ALOGD("%s SEND_AUDIO_CAL: acdb_id(%d), capability(%d)", __func__, acdb_id, capability);
                mydata->acdb_send_audio_cal(acdb_id, capability);
                return 0;
            }
        }
        break;

        case MSG_SEND_AUDIO_CAL_V2: {
            if (mydata->acdb_send_audio_cal_v2 != NULL) {
                int acdb_id;
                readInt(&acdb_id, data_buf);
                int capability;
                readInt(&capability, data_buf);
                int app_id;
                readInt(&app_id, data_buf);
                int sample_rate;
                readInt(&sample_rate, data_buf);
                ALOGD("%s SEND_AUDIO_CAL_V2: acdb_id(%d), capability(%d), app_id(%d), sample_rate(%d)", __func__, acdb_id, capability, app_id, sample_rate);
                mydata->acdb_send_audio_cal_v2(acdb_id, capability, app_id, sample_rate);
                return 0;
            }
        }
        break;

        case MSG_SEND_AUDIO_CAL_V3: {
            if (mydata->acdb_send_audio_cal_v3 != NULL) {
                int index = 0;
                if (strncmp(claddr.sun_path, "/vendor/", 8) == 0) {
                    refcount ++;
                    index = 2 + (refcount % 2) * 2; // only send_cal from Android AHAL, need to add index.
                    ALOGD("socket from android: claddr.sun_path %s, index %d", claddr.sun_path, index);
                } else {
                    ALOGD("socket from linux: claddr.sun_path %s", claddr.sun_path);
                }
                int acdb_id;
                readInt(&acdb_id, data_buf);
                int capability;
                readInt(&capability, data_buf);
                int app_id;
                readInt(&app_id, data_buf);
                int sample_rate;
                readInt(&sample_rate, data_buf);
                int use_case;
                readInt(&use_case, data_buf);
                if (use_case < 2) // if use_case > 2, send_cal is from Android Early Chime.
                    use_case += index;
                ALOGD("%s SEND_AUDIO_CAL_V3: acdb_id(%d), capability(%d), app_id(%d), sample_rate(%d), use_case(%d)", __func__, acdb_id, capability, app_id, sample_rate, use_case);
                mydata->acdb_send_audio_cal_v3(acdb_id, capability, app_id, sample_rate, use_case);
                return 0;
            }
        }
        break;

        case MSG_SEND_AUDIO_CAL_V4: {
            if (mydata->acdb_send_audio_cal_v4 != NULL) {
                int index = 0;
                if (strncmp(claddr.sun_path, "/vendor/", 8) == 0) {
                    refcount ++;
                    index = 2 + (refcount % 2) * 2;  // only send_cal from Android AHAL, need to add index.
                    ALOGD("socket from android: claddr.sun_path %s", claddr.sun_path);
                } else {
                    ALOGD("socket from linux: claddr.sun_path %s", claddr.sun_path);
                }
                int acdb_id;
                readInt(&acdb_id, data_buf);
                int capability;
                readInt(&capability, data_buf);
                int app_id;
                readInt(&app_id, data_buf);
                int sample_rate;
                readInt(&sample_rate, data_buf);
                int use_case;
                readInt(&use_case, data_buf);
                if (use_case < 2) // if use_case > 2, send_cal is from Android Early Chime.
                    use_case += index;
                int afe_sample_rate;
                readInt(&afe_sample_rate, data_buf);
                ALOGD("%s SEND_AUDIO_CAL_V4: acdb_id(%d), capability(%d), app_id(%d), sample_rate(%d), use_case(%d), afe_sample_rate(%d)", __func__, acdb_id, capability, app_id, sample_rate, use_case, afe_sample_rate);
                mydata->acdb_send_audio_cal_v4(acdb_id, capability, app_id, sample_rate, use_case, afe_sample_rate);
                return 0;
            }
        }
        break;

        case MSG_SEND_AUDIO_CAL_V6: {
            if (mydata->acdb_send_audio_cal_v6 != NULL) {
                int index = 0;
                if (strncmp(claddr.sun_path, "/vendor/", 8) == 0) {
                    refcount ++;
                    index = 2 + (refcount % 2) * 2;  // only send_cal from Android AHAL, need to add index.
                    ALOGD("socket from android: claddr.sun_path %s", claddr.sun_path);
                } else {
                    ALOGD("socket from linux: claddr.sun_path %s", claddr.sun_path);
                }
                int acdb_id;
                readInt(&acdb_id, data_buf);
                int capability;
                readInt(&capability, data_buf);
                int app_id;
                readInt(&app_id, data_buf);
                int sample_rate;
                readInt(&sample_rate, data_buf);
                int use_case;
                readInt(&use_case, data_buf);
                if (use_case < 2) // if use_case > 2, send_cal is from Android Early Chime.
                    use_case += index;
                int afe_sample_rate;
                readInt(&afe_sample_rate, data_buf);
                int cal_mode;
                readInt(&cal_mode, data_buf);
                int offset_index;
                readInt(&offset_index, data_buf);
                ALOGD("%s SEND_AUDIO_CAL_V6: acdb_id(%d), capability(%d), app_id(%d), sample_rate(%d), use_case(%d), afe_sample_rate(%d), cal_mode(%d), offset_index(%d)",\
                         __func__, acdb_id, capability, app_id, sample_rate, use_case, afe_sample_rate, cal_mode, offset_index);
                mydata->acdb_send_audio_cal_v6(acdb_id, capability, app_id, sample_rate, use_case, afe_sample_rate, cal_mode, offset_index);
                return 0;
            }
        }
        break;

        case MSG_SEND_LISTEN_DEV_CAL: {
            if (mydata->acdb_send_listen_device_cal != NULL) {
                int acdb_id;
                readInt(&acdb_id, data_buf);
                int type;
                readInt(&type, data_buf);
                int app_id;
                readInt(&app_id, data_buf);
                int sample_rate;
                readInt(&sample_rate, data_buf);
                ALOGD("%s SEND_LISTEN_DEV_CAL: acdb_id(%d), type(%d), app_id(%d), sample_rate(%d)", __func__, acdb_id, type, app_id, sample_rate);
                mydata->acdb_send_listen_device_cal(acdb_id, type, app_id, sample_rate);
                return 0;
            }
        }
        break;

        case MSG_SEND_LISTEN_LSM_CAL: {
            if (mydata->acdb_send_listen_lsm_cal != NULL) {
                int acdb_id;
                readInt(&acdb_id, data_buf);
                int app_id;
                readInt(&app_id, data_buf);
                int mode;
                readInt(&mode, data_buf);
                int type;
                readInt(&type, data_buf);
                ALOGD("%s SEND_LISTEN_LSM_CAL: acdb_id(%d), app_id(%d), mode(%d), type(%d)", __func__, acdb_id, app_id, mode, type);
                ret = mydata->acdb_send_listen_lsm_cal(acdb_id, app_id, mode, type);
                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_SEND_LISTEN_LSM_CAL_V1: {
            if (mydata->acdb_send_listen_lsm_cal_v1 != NULL) {
                int acdb_id;
                readInt(&acdb_id, data_buf);
                int app_id;
                readInt(&app_id, data_buf);
                int mode;
                readInt(&mode, data_buf);
                int type;
                readInt(&type, data_buf);
                int buff_idx;
                readInt(&buff_idx, data_buf);
                ALOGD("%s SEND_LISTEN_LSM_CAL_V1: acdb_id(%d), app_id(%d), mode(%d), type(%d), buff_idx(%d)", __func__, acdb_id, app_id, mode, type, buff_idx);
                ret = mydata->acdb_send_listen_lsm_cal_v1(acdb_id, app_id, mode, type, buff_idx);
                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_SEND_ANC_CAL: {
            if (mydata->acdb_send_anc_cal != NULL) {
                int acdb_id;
                readInt(&acdb_id, data_buf);
                ALOGD("%s SEND_ANC_CAL: acdb_id(%d)", __func__, acdb_id);
                ret = mydata->acdb_send_anc_cal(acdb_id);
                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_SEND_ANC_DATA: {
            if (mydata->acdb_send_tabla_anc_data != NULL) {
                ALOGD("%s SEND_ANC_DATA", __func__);
                mydata->acdb_send_tabla_anc_data();
                return 0;
            }
        }
        break;

        case MSG_GET_AUD_VOL_STEPS: {
            if (mydata->acdb_get_aud_volume_steps != NULL) {
                ALOGD("%s GET_AUD_VOL_STEPS:", __func__);
                ret = mydata->acdb_get_aud_volume_steps();
                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_SEND_GAIN_DEP_CAL: {
            if (mydata->acdb_send_gain_dep_cal != NULL) {
                int acdb_id;
                readInt(&acdb_id, data_buf);
                int app_id;
                readInt(&app_id, data_buf);
                int capability;
                readInt(&capability, data_buf);
                int mode;
                readInt(&mode, data_buf);
                int vol_index;
                readInt(&vol_index, data_buf);
                ALOGD("%s SEND_GAIN_DEP_CAL: acdb_id(%d), app_id(%d), capability(%d), mode(%d), vol_index(%d)", __func__, acdb_id, app_id, capability, mode, vol_index);
                ret = mydata->acdb_send_gain_dep_cal(acdb_id, app_id, capability, mode, vol_index);
                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_GET_REMOTE_ACDB_ID: {
            if (mydata->acdb_get_remote_acdb_id != NULL) {
                unsigned int native_acdb_id;
                readInt(&native_acdb_id, data_buf);
                ALOGD("%s GET_REMOTE_ACDB_ID: native_acdb_id(%d)", __func__, native_acdb_id);
                ret = mydata->acdb_get_remote_acdb_id(native_acdb_id);
                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_GET_ECRX_DEVICE: {
            if (mydata->acdb_get_ecrx_device != NULL) {
                int acdb_id;
                readInt(&acdb_id, data_buf);
                ALOGD("%s GET_ECRX_DEVICE: acdb_id(%d)", __func__, acdb_id);
                ret = mydata->acdb_get_ecrx_device(acdb_id);
                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_GET_CALIBRATION: {
            if (mydata->acdb_get_calibration != NULL) {
                const char attr[128] = "";
                int attrlen, size, paramsize;

                readString((char *)attr, sizeof(attr), &attrlen, data_buf);
                if (attrlen == 0) {
                    ALOGE("Bad socket transaction: GET_CALIBRATION");
                    writeInt(-EINVAL, reply_buf);
                    acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                    return 0;
                }
                if (strcmp("null", attr) == 0) {
                    attrlen = 0;
                }

                readInt(&size, data_buf);
                ALOGD("%s GET_CALIBRATION: attr(%s), size(%d)", __func__, attr, size);

                struct param_data params;
                void *pdata = (void *)&params;
                readString((char *)pdata, sizeof(struct param_data), &paramsize, data_buf);

                if (params.get_size == 0) {
                    params.buff = calloc(1, params.buff_size);
                    if (params.buff == NULL) {
                        writeInt(-ENOMEM, reply_buf);
                        acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                        return 0;
                    }
                }
                int ret = mydata->acdb_get_calibration((attrlen ? (char *)attr : NULL), size, pdata);
                writeInt(ret, reply_buf);
                writeString(pdata, sizeof(struct param_data), reply_buf);
                if (params.get_size == 0) {
                    writeString(params.buff, params.buff_size, reply_buf);
                }

                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                if (params.get_size == 0) {
                    free(params.buff);
                }
                return 0;
            }
        }
        break;

        case MSG_SET_AUDIO_CAL_V2: {
            if (mydata->acdb_set_audio_cal_v2 != NULL) {
                unsigned int datalen, cfgsize;
                acdb_audio_cal_cfg_t cfg;
                void *caldata = (void *)&cfg;

                readString((char *)caldata, sizeof(acdb_audio_cal_cfg_t), &cfgsize, data_buf);
                if (cfgsize != sizeof(acdb_audio_cal_cfg_t)) {
                    ALOGE("Bad socket transaction: MSG_SET_AUDIO_CAL_V2");
                    writeInt(-EINVAL, reply_buf);
                    acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                    return 0;
                }

                readInt(&datalen, data_buf);
                void *pdata = calloc(1, datalen);
                if (pdata == NULL) {
                    ALOGE("calloc fail MSG_SET_AUDIO_CAL_V2");
                    writeInt(-ENOMEM, reply_buf);
                    acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                    return 0;
                }
                readString((char *)pdata, datalen, &datalen, data_buf);
                ALOGD("%s SET_AUDIO_CAL_V2: datalen(%d)", __func__, datalen);

                ret = mydata->acdb_set_audio_cal_v2(caldata, pdata, datalen);
                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                free(pdata);
                return 0;
            }
        }
        break;

        case MSG_GET_AUDIO_CAL_V2: {
            if (mydata->acdb_get_audio_cal_v2 != NULL) {
                ALOGD("%s GET_AUDIO_CAL_V2", __func__);
                unsigned int datalen, cfgsize;
                acdb_audio_cal_cfg_t cfg;
                void *caldata = (void *)&cfg;

                readString((char *)caldata, sizeof(acdb_audio_cal_cfg_t), &cfgsize, data_buf);
                if (cfgsize != sizeof(acdb_audio_cal_cfg_t)) {
                    ALOGE("Bad socket transaction: MSG_GET_AUDIO_CAL_V2");
                    writeInt(-EINVAL, reply_buf);
                    acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                    return 0;
                }

                readInt(&datalen, data_buf);
                void *pdata = calloc(1, datalen);
                if (pdata == NULL) {
                    writeInt(-ENOMEM, reply_buf);
                    acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                    return 0;
                }
                ALOGD("%s: provided datalen(%d)", __func__, datalen);

                int ret = mydata->acdb_get_audio_cal_v2(caldata, pdata, &datalen);
                ALOGD("%s: return datalen(%d)", __func__, datalen);
                writeInt(ret, reply_buf);
                writeString(pdata, datalen, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                free(pdata);
                return 0;
            }
        }
        break;

        case MSG_SEND_META_INFO: {
            if (mydata->acdb_send_meta_info != NULL) {
                int metaInfoKey;
                readInt(&metaInfoKey, data_buf);
                int buf_idx;
                readInt(&buf_idx, data_buf);
                ALOGD("%s SEND_META_INFO: metaInfoKey(%d), buf_idx(%d)", __func__, metaInfoKey, buf_idx);
                ret = mydata->acdb_send_meta_info(metaInfoKey, buf_idx);
                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_SEND_META_INFO_LIST: {
            if (mydata->acdb_send_meta_info_list != NULL) {
                struct listnode key_list;
                struct meta_key_list key_info;
                readInt(&key_info.cal_info.nKey, data_buf);
                list_init(&key_list);
                list_add_tail(&key_list, &key_info.list);

                ALOGD("%s SEND_META_INFO_LIST key_info = %u", __func__, key_info.cal_info.nKey);
                ret = mydata->acdb_send_meta_info_list(&key_list);
                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_SET_CODEC_DATA: {
            if (mydata->acdb_set_codec_data != NULL) {
                vbat_adc_data_t params;
                const char attr[128] = "";
                int attrlen, adclen;
                void *pdata = (void *)&params;

                readString((char *)pdata, sizeof(vbat_adc_data_t), &adclen, data_buf);
                ALOGD("%s: SET_CODEC_DATA dcp1 %d, dcp2 %d", params.dcp1, params.dcp2);
                if (adclen != sizeof(vbat_adc_data_t)) {
                    ALOGE("Bad socket transaction: MSG_SET_CODEC_DATA");
                    writeInt(-EINVAL, reply_buf);
                    acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                    return 0;
                }

                readString((char *)attr, sizeof(attr), &attrlen, data_buf);
                if (attrlen == 0) {
                    ALOGE("Bad socket transaction: SET_CODEC_DATA");
                    writeInt(-EINVAL, reply_buf);
                    acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                    return 0;
                }
                if (strcmp("null", attr) == 0) {
                    attrlen = 0;
                }
                ALOGD("%s SET_CODEC_DATA: attr(%s)", __func__, attr);

                ret = mydata->acdb_set_codec_data(pdata, (attrlen ? (char *)attr : NULL));
                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_IS_INITIALIZED: {
            if (mydata->acdb_is_initialized != NULL) {
                ALOGD("%s IS_INITIALIZED", __func__);
                bool retval = mydata->acdb_is_initialized();
                writeBool(retval, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_RELOAD_ACDB_FILES: {
            if (mydata->acdb_reload_acdb_files != NULL) {
                int pathlen = 0, namelen = 0, verlen = 0;
                const char new_acdb_file_path[128] = "";
                const char snd_card_name[64] = "";
                const char cvd_version[64] = "";
                readString((char *)new_acdb_file_path, 128, &pathlen, data_buf);
                readString((char *)snd_card_name, 64, &namelen, data_buf);
                readString((char *)cvd_version, 64, &verlen, data_buf);
                if (pathlen == 0 || namelen == 0 || verlen == 0) {
                    ALOGE("Bad socket transaction: RELOAD_ACDB_FILES");
                    writeInt(-EINVAL, reply_buf);
                    acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                    return 0;
                }
                int metaInfoKey;
                readInt(&metaInfoKey, data_buf);
                ALOGD("%s RELOAD_ACDB_FILES: new_acdb_file_path(%s), snd(%s), cvd(%s), metaInfoKey(%d)", __func__, new_acdb_file_path, snd_card_name, cvd_version, metaInfoKey);

                if (strcmp("null", new_acdb_file_path) == 0) {
                    pathlen = 0;
                }
                if (strcmp("null", snd_card_name) == 0) {
                    namelen = 0;
                }
                if (strcmp("null", cvd_version) == 0) {
                    verlen = 0;
                }
                ret = mydata->acdb_reload_acdb_files((pathlen ? (char *)new_acdb_file_path : NULL),
                    (namelen ? (char *)snd_card_name : NULL),
                    (verlen ? (char *)cvd_version : NULL),
                    metaInfoKey);

                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        case MSG_RELOAD_ACDB_FILES_V2: {
            if (mydata->acdb_reload_acdb_files_v2 != NULL) {
                int pathlen = 0, namelen = 0, verlen = 0;
                const char new_acdb_file_path[128] = "";
                const char snd_card_name[64] = "";
                const char cvd_version[64] = "";
                readString((char *)new_acdb_file_path, 128, &pathlen, data_buf);
                readString((char *)snd_card_name, 64, &namelen, data_buf);
                readString((char *)cvd_version, 64, &verlen, data_buf);

                if (pathlen == 0 || namelen == 0 || verlen == 0) {
                    ALOGE("Bad socket transaction: RELOAD_ACDB_FILES");
                    writeInt(-EINVAL, reply_buf);
                    acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                    return 0;
                }

                struct listnode key_list;
                struct meta_key_list key_info;
                readInt(&key_info.cal_info.nKey, data_buf);
                list_init(&key_list);
                list_add_tail(&key_list, &key_info.list);

                ALOGD("%s RELOAD_ACDB_FILES_V2: new_acdb_file_path(%s), snd(%s), cvd(%s), metaInfoKey(%d)", __func__, new_acdb_file_path, snd_card_name, cvd_version, key_info.cal_info.nKey);
                if (strcmp("null", new_acdb_file_path) == 0) {
                    pathlen = 0;
                }
                if (strcmp("null", snd_card_name) == 0) {
                    namelen = 0;
                }
                if (strcmp("null", cvd_version) == 0) {
                    verlen = 0;
                }
                ret = mydata->acdb_reload_acdb_files_v2((pathlen ? (char *)new_acdb_file_path : NULL),
                    (namelen ? (char *)snd_card_name : NULL),
                    (verlen ? (char *)cvd_version : NULL),
                    &key_list);
                writeInt(ret, reply_buf);
                acdb_loader_send_reply(fd, claddr, sizeof(acdb_data_buf_t), reply_buf);
                return 0;
            }
        }
        break;

        default:
            return 0;
    }
    return 0;
}

int acdb_loader_server_init(void)
{
    ALOGD("acdb_loader_server_init enter");

    mydata = malloc(sizeof(acdb_service_data));

    if(mydata == NULL) {
        init_status = FALSE;
        return -1;
    }
    mydata->acdb_handle = dlopen(LIB_ACDB_LOADER, RTLD_NOW);
    if (mydata->acdb_handle == NULL) {
        ALOGE("%s: DLOPEN failed for %s", __func__, LIB_ACDB_LOADER);
        init_status = FALSE;
        free(mydata);
        mydata = NULL;
    } else {
        ALOGD("%s: DLOPEN successful for %s", __func__, LIB_ACDB_LOADER);
        init_status = TRUE;

        mydata->acdb_init_v2 = (acdb_init_v2_t)dlsym(mydata->acdb_handle, "acdb_loader_init_v2");
        if (mydata->acdb_init_v2 == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_init_v2", __func__, dlerror());

        mydata->acdb_init_v3 = (acdb_init_v3_t)dlsym(mydata->acdb_handle, "acdb_loader_init_v3");
        if (mydata->acdb_init_v3 == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_init_v3", __func__, dlerror());

        mydata->acdb_init_v4 = (acdb_init_v4_t)dlsym(mydata->acdb_handle, "acdb_loader_init_v4");
        if (mydata->acdb_init_v4 == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_init_v4", __func__, dlerror());

        mydata->acdb_init = (acdb_init_t)dlsym(mydata->acdb_handle, "acdb_loader_init_ACDB");
        if (mydata->acdb_init == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_init_ACDB", __func__, dlerror());

        mydata->acdb_get_default_app_type = (acdb_get_default_app_type_t)dlsym(mydata->acdb_handle, "acdb_loader_get_default_app_type");
        if (mydata->acdb_get_default_app_type == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_get_default_app_type", __func__, dlerror());

        mydata->acdb_send_common_custom_topology = (acdb_send_common_custom_topology_t)dlsym(mydata->acdb_handle, "acdb_loader_send_common_custom_topology");
        if (mydata->acdb_send_common_custom_topology == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_send_common_custom_topology", __func__, dlerror());

        mydata->acdb_deallocate = (acdb_deallocate_t)dlsym(mydata->acdb_handle, "acdb_loader_deallocate_ACDB");
        if (mydata->acdb_deallocate == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_deallocate_ACDB", __func__, dlerror());

        mydata->acdb_deallocate_cal = (acdb_deallocate_cal_t)dlsym(mydata->acdb_handle, "acdb_loader_deallocate_cal");
        if (mydata->acdb_deallocate_cal == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_deallocate_cal", __func__, dlerror());

        mydata->acdb_send_voice_cal_v2 = (acdb_send_voice_cal_v2_t)dlsym(mydata->acdb_handle, "acdb_loader_send_voice_cal_v2");
        if (mydata->acdb_send_voice_cal_v2 == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_send_voice_cal_v2", __func__, dlerror());

        mydata->acdb_send_voice_cal = (acdb_send_voice_cal_t)dlsym(mydata->acdb_handle, "acdb_loader_send_voice_cal");
        if (mydata->acdb_send_voice_cal == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_send_voice_cal", __func__, dlerror());

        mydata->acdb_reload_vocvoltable = (acdb_reload_vocvoltable_t)dlsym(mydata->acdb_handle, "acdb_loader_reload_vocvoltable");
        if (mydata->acdb_reload_vocvoltable == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_reload_vocvoltable", __func__, dlerror());

        mydata->acdb_send_audio_cal = (acdb_send_audio_cal_t)dlsym(mydata->acdb_handle, "acdb_loader_send_audio_cal");
        if (mydata->acdb_send_audio_cal == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_send_audio_cal", __func__, dlerror());

        mydata->acdb_send_audio_cal_v2 = (acdb_send_audio_cal_v2_t)dlsym(mydata->acdb_handle, "acdb_loader_send_audio_cal_v2");
        if (mydata->acdb_send_audio_cal_v2 == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_send_audio_cal_v2", __func__, dlerror());

        mydata->acdb_send_audio_cal_v3 = (acdb_send_audio_cal_v3_t)dlsym(mydata->acdb_handle, "acdb_loader_send_audio_cal_v3");
        if (mydata->acdb_send_audio_cal_v3 == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_send_audio_cal_v3", __func__, dlerror());

        mydata->acdb_send_audio_cal_v4 = (acdb_send_audio_cal_v4_t)dlsym(mydata->acdb_handle, "acdb_loader_send_audio_cal_v4");
        if (mydata->acdb_send_audio_cal_v4 == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_send_audio_cal_v4", __func__, dlerror());

        mydata->acdb_send_audio_cal_v6 = (acdb_send_audio_cal_v6_t)dlsym(mydata->acdb_handle, "acdb_loader_send_audio_cal_v6");
        if (mydata->acdb_send_audio_cal_v6 == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_send_audio_cal_v6", __func__, dlerror());

        mydata->acdb_send_listen_device_cal = (acdb_send_listen_device_cal_t)dlsym(mydata->acdb_handle, "acdb_loader_send_listen_device_cal");
        if (mydata->acdb_send_listen_device_cal == NULL)
        ALOGE("%s: dlsym error %s for acdb_loader_send_listen_device_cal", __func__, dlerror());

        mydata->acdb_send_listen_lsm_cal = (acdb_send_listen_lsm_cal_t)dlsym(mydata->acdb_handle, "acdb_loader_send_listen_lsm_cal");
        if (mydata->acdb_send_listen_lsm_cal == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_send_listen_lsm_cal", __func__, dlerror());

        mydata->acdb_send_listen_lsm_cal_v1 = (acdb_send_listen_lsm_cal_v1_t)dlsym(mydata->acdb_handle, "acdb_loader_send_listen_lsm_cal_v1");
        if (mydata->acdb_send_listen_lsm_cal_v1 == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_send_listen_lsm_cal_v1", __func__, dlerror());

        mydata->acdb_send_anc_cal = (acdb_send_anc_cal_t)dlsym(mydata->acdb_handle, "acdb_loader_send_anc_cal");
        if (mydata->acdb_send_anc_cal == NULL)
            ALOGW("%s: dlsym error %s for acdb_loader_send_anc_cal", __func__, dlerror());

        mydata->acdb_send_tabla_anc_data = (acdb_send_tabla_anc_data_t)dlsym(mydata->acdb_handle, "send_tabla_anc_data");
        if (mydata->acdb_send_tabla_anc_data == NULL)
            ALOGW("%s: dlsym error %s for send_tabla_anc_data", __func__, dlerror());

        mydata->acdb_get_aud_volume_steps = (acdb_get_aud_volume_steps_t)dlsym(mydata->acdb_handle, "acdb_loader_get_aud_volume_steps");
        if (mydata->acdb_get_aud_volume_steps == NULL)
            ALOGW("%s: dlsym error %s for acdb_loader_get_aud_volume_steps", __func__, dlerror());

        mydata->acdb_send_gain_dep_cal = (acdb_send_gain_dep_cal_t)dlsym(mydata->acdb_handle, "acdb_loader_send_gain_dep_cal");
        if (mydata->acdb_send_gain_dep_cal == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_send_gain_dep_cal", __func__, dlerror());

        mydata->acdb_get_remote_acdb_id = (acdb_get_remote_acdb_id_t)dlsym(mydata->acdb_handle, "acdb_loader_get_remote_acdb_id");
        if (mydata->acdb_get_remote_acdb_id == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_get_remote_acdb_id", __func__, dlerror());

        mydata->acdb_get_ecrx_device = (acdb_get_ecrx_device_t)dlsym(mydata->acdb_handle, "acdb_loader_get_ecrx_device");
        if (mydata->acdb_get_ecrx_device == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_get_ecrx_device", __func__, dlerror());

        mydata->acdb_get_calibration = (acdb_get_calibration_t)dlsym(mydata->acdb_handle, "acdb_loader_get_calibration");
        if (mydata->acdb_get_calibration == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_get_calibration", __func__, dlerror());

        mydata->acdb_set_audio_cal_v2 = (acdb_set_audio_cal_v2_t)dlsym(mydata->acdb_handle, "acdb_loader_set_audio_cal_v2");
        if (mydata->acdb_set_audio_cal_v2 == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_set_audio_cal_v2", __func__, dlerror());

        mydata->acdb_get_audio_cal_v2 = (acdb_get_audio_cal_v2_t)dlsym(mydata->acdb_handle, "acdb_loader_get_audio_cal_v2");
        if (mydata->acdb_get_audio_cal_v2 == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_get_audio_cal_v2", __func__, dlerror());

        mydata->acdb_send_meta_info = (acdb_send_meta_info_t)dlsym(mydata->acdb_handle, "send_meta_info");
        if (mydata->acdb_send_meta_info == NULL)
            ALOGE("%s: dlsym error %s for send_meta_info", __func__, dlerror());

        mydata->acdb_send_meta_info_list = (acdb_send_meta_info_list_t)dlsym(mydata->acdb_handle, "send_meta_info_list");
        if (mydata->acdb_send_meta_info_list == NULL)
            ALOGE("%s: dlsym error %s for send_meta_info_list", __func__, dlerror());

        mydata->acdb_set_codec_data = (acdb_set_codec_data_t)dlsym(mydata->acdb_handle, "acdb_loader_set_codec_data");
        if (mydata->acdb_set_codec_data == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_set_codec_data", __func__, dlerror());

        mydata->acdb_is_initialized = (acdb_is_initialized_t)dlsym(mydata->acdb_handle, "acdb_loader_is_initialized");
        if (mydata->acdb_is_initialized == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_is_initialized", __func__, dlerror());

        mydata->acdb_reload_acdb_files = (acdb_reload_acdb_files_t)dlsym(mydata->acdb_handle, "acdb_loader_reload_acdb_files");
        if (mydata->acdb_reload_acdb_files == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_reload_acdb_files", __func__, dlerror());

        mydata->acdb_reload_acdb_files_v2 = (acdb_reload_acdb_files_v2_t)dlsym(mydata->acdb_handle, "acdb_loader_reload_acdb_files_v2");
        if (mydata->acdb_reload_acdb_files_v2 == NULL)
            ALOGE("%s: dlsym error %s for acdb_loader_reload_acdb_files_v2", __func__, dlerror());
    }
    return 0;
}

/*========================================================================================
FUNCTION acdb_loader_socket_init

acdbloader socket initialization function for different clients
This function setup a server for container acdb client socket

ARGUMENTS
none

RETURN VALUE
0 on success, negative on failure.
=========================================================================================*/
int acdb_loader_socket_init(void)
{
    int ret = -1;
    static struct sockaddr_un sockfd_host_addr;
    ALOGD("%s::socket init()", __func__);

    if (sockfd_server_host != -1) {
        sockfd_server_host = -1;
    }

    sockfd_server_host = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sockfd_server_host < 0) {
        ALOGE("%s: create sock fail !!!", __func__);
        goto error;
    }
    /* Delete existing socket file if necessary */
    if (access(SV_SOCK_PATH_HOST,F_OK) == 0) {
        if (remove(SV_SOCK_PATH_HOST) == -1) {
            ALOGE("%s: remove sock path %s maybe exist!!", __func__, SV_SOCK_PATH_HOST);
            goto error;
        }
    } else {
        if (access(SV_ACDB_SOCK_PATH, F_OK) != 0) {
            if (mkdir(SV_ACDB_SOCK_PATH, 0777) < 0) {
                ALOGE("%s: mkdir %s failed!", __func__, SV_ACDB_SOCK_PATH);
                goto error;
            }
            chmod(SV_ACDB_SOCK_PATH, 0777);
        }
    }
    memset(&sockfd_host_addr, 0, sizeof(struct sockaddr_un));
    snprintf(sockfd_host_addr.sun_path, UNIX_PATH_MAX, SV_SOCK_PATH_HOST);
    sockfd_host_addr.sun_family = AF_LOCAL;

    ret = bind(sockfd_server_host,(struct sockaddr  const *)&sockfd_host_addr, sizeof(struct sockaddr_un));
    if (ret != 0) {
        ALOGE("%s: sockfd_server_container4: bind error - %s", __func__, strerror(errno));
        goto error;
    }

    /* Allow any client to connect to send socket.
    Allow any client to connect to passive receive socket.
    Only root group clients can connect recieve socket. */
    chmod(sockfd_host_addr.sun_path, 0666);

    ALOGD("%s::socket init() done!", __func__);
    return ret;
error:
    ALOGE("%s: Unable to create socket server for clients", __func__);
    if (sockfd_server_host != -1) {
        close(sockfd_server_host);
        sockfd_server_host = -1;
    }
    return ret;
}

/*========================================================================================
FUNCTION acdb_loader_server_release

Function to close socket and release fd

ARGUMENTS
none

RETURN VALUE
void
========================================================================================*/
void acdb_loader_socket_release(void)
{
    ALOGD("acdb_loader_socket_release()");
    close(sockfd_server_host);
    sockfd_server_host = -1;
}

/*========================================================================================
FUNCTION acdb_loader_server_deinit

Function to release mydata structure and fd

ARGUMENTS
none

RETURN VALUE
void
========================================================================================*/
void acdb_loader_server_deinit(void)
{
    ALOGD("acdb_loader_server_deinit()");
    dlclose(mydata->acdb_handle);
    free(mydata);
}

int main(int argc, char * argv[]) {
    int ret = -1;
    int fd;
    acdb_data_buf_t * data_buf = NULL;
    acdb_data_buf_t * reply_buf = NULL;
    struct sockaddr_un client_addr;
    fd_set readfds;
    ssize_t numBytes;
    socklen_t len;
    ALOGD("acdb_loader_server::acdb_loader_server()");

    ret = acdb_loader_server_init();
    if (ret != 0) {
        ALOGE("acdb_loader_server::acdb_loader_server_init fail");
        return ret;
    }

    ret = acdb_loader_socket_init();
    if (ret != 0) {
        ALOGE("acdb_loader_server::acdb_loader_server_socket_init fail");
        return ret;
    }

    data_buf = calloc(1, sizeof(acdb_data_buf_t));
    if (data_buf == NULL) {
        ALOGE("acdb_loader_server::calloc data buffer fail");
        goto FAIL1;
    }
    reply_buf = calloc(1, sizeof(acdb_data_buf_t));
    if (reply_buf == NULL) {
        ALOGE("acdb_loader_server::calloc data buffer fail");
        goto FAIL2;
    }
    data_buf->readPos = 0;
    data_buf->bufSize = sizeof(acdb_data_buf_t);
    reply_buf->bufPos = 0;
    reply_buf->bufSize = sizeof(acdb_data_buf_t);

    FD_ZERO(&readfds);
    FD_SET(sockfd_server_host, &readfds);

    while(init_status) {
        ALOGV("acdb-server: %s select begin.", __func__);
        ret = select(sockfd_server_host + 1, &readfds, (fd_set *)0,
           (fd_set *)0, (struct timeval *) 0);
        if (ret  ==-1 && errno == EINTR) {
            ALOGE("acdb-server: %s interupt occur ret %d", __func__, ret);
            continue;
        }

        if (ret == -1) {
            ALOGE("acdb-server: %s select error ret %d, need reboot", __func__, ret);
            break;
        }

        if (ret == 0) {
            ALOGE("acdb-server: %s select timeout ret %d.", __func__, ret);
            continue;
        }

        if (FD_ISSET(sockfd_server_host, &readfds)) {
            len = sizeof(struct sockaddr_un);
            numBytes = recvfrom(sockfd_server_host, (void *)data_buf, data_buf->bufSize, 0, (struct sockaddr *)&client_addr, &len);
            if (numBytes == -1) {
                ALOGE("%s: recv fail error %s!!", __func__, (char *)strerror(errno));
                continue;
            }
            memset(reply_buf, 0, sizeof(acdb_data_buf_t));
            ret = acdb_loader_handle_message(sockfd_server_host, client_addr, data_buf, reply_buf);
            if (ret != 0) {
                ALOGE("%s: acdb_loader_handle_message fail", __func__);
                continue;
            }
        }
    }

    free(reply_buf);
FAIL2:
    free(data_buf);
FAIL1:
    acdb_loader_socket_release();
    acdb_loader_server_deinit();
    ALOGD("acdb_loader_server::exit()!!");
    return ret;
}


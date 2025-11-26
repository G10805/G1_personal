/* Copyright (c) 2020, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <cutils/list.h>
#include "acdb-loader-server-client-v2.h"
#include "acdb.h"
#include "acdb-loader.h"
#include "anc_map_api.h"
#include "vbat_map_api.h"

#ifndef _ANDROID_
#define strlcat g_strlcat
#define strlcpy g_strlcpy
#endif

#ifdef LINUX_ENABLED
#define SV_SOCK_PATH "/var/lib/shared/acdb/acdb"
#define CL_SOCK_PATH "/var/lib/shared/acdb/acdb_cl"
#else
#define SV_SOCK_PATH "/vendor/var/lib/shared/acdb/acdb"
#define CL_SOCK_PATH "/vendor/var/lib/shared/acdb/acdb_cl"
#endif
#define BUF_SIZE 2048

static int  acdb_socket_client = -1;
static acdb_data_buf_t socket_data_buf;

int getNameByPid(pid_t pid, char *task_name)
{
    FILE* fp = NULL;
    char proc_pid_path[BUF_SIZE];
    char buf[BUF_SIZE];

    snprintf(proc_pid_path, sizeof(proc_pid_path), "/proc/%d/status", pid);
    fp = fopen(proc_pid_path, "r");
    if(fp != NULL) {
        if(fgets(buf, BUF_SIZE - 1, fp) == NULL) {
            fclose(fp);
            return -1;
        }
        fclose(fp);
        sscanf(buf, "%*s %s", task_name);
    }
    return 0;
}

int acdb_loader_dispatch_message(int sockfd, int length, acdb_data_buf_t * data_buf) {
    struct sockaddr_un svaddr;
    if (sockfd < 0 || length < 0) {
        ALOGE("%s: Invalid socket or buffer length!", __func__);
        return -1;
    }

    memset(&svaddr, 0, sizeof(struct sockaddr_un));
    svaddr.sun_family = AF_LOCAL;
    strlcpy(svaddr.sun_path, SV_SOCK_PATH, sizeof(svaddr.sun_path) - 1);

    if (sendto(sockfd, (void *)data_buf, length, 0, (struct sockaddr *)&svaddr, sizeof(struct sockaddr_un)) != length) {
        ALOGE("%s: send fail error %s!!", __func__, (char *)strerror(errno));
        return -1;
    }

    ALOGV("acdb_loader_dispatch_message done");
    return 0;
}

int acdb_loader_handle_reply(int sockfd, acdb_data_buf_t * data_buf) {
    ssize_t numBytes;
    int ret = -1;

    numBytes = recvfrom(sockfd, (void *)data_buf, sizeof(acdb_data_buf_t), 0, NULL, NULL);
    if (numBytes == -1) {
        ALOGE("%s: recv fail error %s!!", __func__, (char *)strerror(errno));
        return ret;
    }
    readInt(&ret, data_buf);

    ALOGV("%s: %d msgID %d bufPos %d ret %d done", __func__, (int)numBytes, data_buf->msgID, data_buf->bufPos, ret);
    return ret;
}

int get_acdb_loader_client_socket(void) {
    char task_name[50] = "";
    struct sockaddr_un claddr;

    if (acdb_socket_client == -1) {
        acdb_socket_client = socket(AF_LOCAL, SOCK_DGRAM, 0);
        if (acdb_socket_client < 0) {
            ALOGE("create socket fail error: %s", (char *)strerror(errno));//TODO whether need retry ?
            return -1;
        }

        memset(&claddr, 0, sizeof(struct sockaddr_un));
        claddr.sun_family = AF_LOCAL;
        getNameByPid(getpid(), task_name);
        snprintf(claddr.sun_path, sizeof(claddr.sun_path), "%s.%s", CL_SOCK_PATH, task_name);

        /* Delete existing socket file if necessary */
        if (access(claddr.sun_path, F_OK) == 0) {
            if (remove(claddr.sun_path) == -1) {
                ALOGE("remove sock path %s", claddr.sun_path);
            }
        }
        int rc = bind(acdb_socket_client, (struct sockaddr *)&claddr, sizeof(struct sockaddr_un));
        if (rc != 0) {
            ALOGE("bind error - %s", (char *)strerror(errno));
            goto error;
        }

        struct timeval tv = {3, 0};
        if (setsockopt(acdb_socket_client, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval)) != 0) {
            ALOGE("setsocket fail error: %s", (char *)strerror(errno));
            goto error;
        }
    }
    return acdb_socket_client;

error:
    close(acdb_socket_client);
    acdb_socket_client = -1;
    /* Delete existing socket file if necessary */
    if (access(claddr.sun_path, F_OK) == 0) {
        if (remove(claddr.sun_path) == -1) {
            ALOGE("remove sock path %s failed", claddr.sun_path);
        }
    }
    return -1;
}

void release_acdb_loader_client_socket(void) {
    char task_name[50] = "";
    struct sockaddr_un claddr;

    if (acdb_socket_client != -1) {
        getNameByPid(getpid(), task_name);
        snprintf(claddr.sun_path, sizeof(claddr.sun_path), "%s.%s", CL_SOCK_PATH, task_name);
        ALOGD("remove sock path %s ", claddr.sun_path);

        close(acdb_socket_client);
        acdb_socket_client = -1;

        /* Delete existing socket file if necessary */
        if (access(claddr.sun_path, F_OK) == 0) {
            if (remove(claddr.sun_path) == -1) {
                ALOGE("remove sock path %s failed", claddr.sun_path);
            }
        }
    }
}

int acdb_loader_init_v2(char *snd_card_name, char *cvd_version, int metaInfoKey) {
    int ret = -1, socketfd;
    const char dummy[] = "null";
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: snd(%s), cvd(%s), metaInfoKey(%d)", __func__, snd_card_name, cvd_version, metaInfoKey);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%s: invalid acdb loader client socket ", __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_INIT_V2;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    if (snd_card_name && strlen(snd_card_name)) {
        writeString(snd_card_name, strlen(snd_card_name), data_buf);
    } else {
        writeString(dummy, strlen(dummy), data_buf);
    }
    if (cvd_version && strlen(cvd_version)) {
        writeString(cvd_version, strlen(cvd_version), data_buf);
    } else {
        writeString(dummy, strlen(dummy), data_buf);
    }
    writeInt(metaInfoKey, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }
    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

int acdb_loader_init_v3(char *snd_card_name, char *cvd_version, struct listnode *metaKey_list) {
    int ret = -1, socketfd;
    const char dummy[] = "null";
    struct listnode *node;
    struct meta_key_list *key_info;
    int count = 0;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: snd(%s), cvd(%s)", __func__, snd_card_name, cvd_version);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_INIT_V3;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    if (snd_card_name && strlen(snd_card_name)) {
        writeString(snd_card_name, strlen(snd_card_name), data_buf);
    } else {
        writeString(dummy, strlen(dummy), data_buf);
    }
    if (cvd_version && strlen(cvd_version)) {
        writeString(cvd_version, strlen(cvd_version), data_buf);
    } else {
        writeString(dummy, strlen(dummy), data_buf);
    }

    list_for_each(node, metaKey_list) {
        if (count > 0) {
            ALOGE("%s: Only a single value for key_list is supported in the current implementation", __func__);
            return -EINVAL;
        }
        key_info = node_to_item(node, struct meta_key_list, list);
        ALOGV("%s: acdb key_info = %u", __func__, key_info->cal_info.nKey);
        writeInt(key_info->cal_info.nKey, data_buf);
        count++;
    }

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }
    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

int acdb_loader_init_v4(void* acdb_init_data, int acdb_loader_init_version) {
    int ret = -1, socketfd;
    const char dummy[] = "null";
    struct listnode *node;
    struct meta_key_list *key_info;
    int count = 0;
    struct acdb_init_data_v4 *init_data = (struct acdb_init_data_v4*) acdb_init_data;
    char *cvd_version = init_data->cvd_version;;
    char *snd_card_name = init_data->snd_card_name;
    bool *is_instance_id_supported = init_data->is_instance_id_supported;
    struct listnode *metaKey_list = init_data->meta_key_list;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: acdb_loader_init_version(%d)", __func__, acdb_loader_init_version);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ",__LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_INIT_V4;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    if (snd_card_name && strlen(snd_card_name)) {
        writeString(snd_card_name, strlen(snd_card_name), data_buf);
    } else {
        writeString(dummy, strlen(dummy), data_buf);
    }
    if (cvd_version && strlen(cvd_version)) {
        writeString(cvd_version, strlen(cvd_version), data_buf);
    } else {
        writeString(dummy, strlen(dummy), data_buf);
    }
    writeBool(*is_instance_id_supported, data_buf);

    list_for_each(node, metaKey_list) {
        if (count > 0) {
            ALOGE("%s: Only a single value for key_list is supported in the current implementation", __func__);
            return -EINVAL;
        }
        key_info = node_to_item(node, struct meta_key_list, list);
        ALOGV("%s: acdb key_info = %u", __func__, key_info->cal_info.nKey);
        writeInt(key_info->cal_info.nKey, data_buf);
        count++;
    }

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    if (ret == 0) {
        readBool(is_instance_id_supported, data_buf);
    }
    return ret;
}

int acdb_loader_init_ACDB(void) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s", __func__);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_INIT_ACDB;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

int acdb_loader_get_default_app_type(void) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s", __func__);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket.", __LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_GET_DEF_APP_TYPE;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

int acdb_loader_send_common_custom_topology(void) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s", __func__);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket.", __LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_SEND_COM_CUSTOM_TOP;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

void acdb_loader_deallocate_ACDB(void) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s", __func__);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_DEALLOCATE_ACDB;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    release_acdb_loader_client_socket();
    return;
}

void acdb_loader_deallocate_cal(int capability, int use_case) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s", __func__);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_DEALLOCATE_CAL;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeInt(capability, data_buf);
    writeInt(use_case, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    return;
}

void acdb_loader_send_voice_cal_v2(int rxacdb_id, int txacdb_id, int feature_set) {
    int socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s", __func__);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_SEND_VOICE_CAL_V2;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeInt(rxacdb_id, data_buf);
    writeInt(txacdb_id, data_buf);
    writeInt(feature_set, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    return;
}

void acdb_loader_send_voice_cal(int rxacdb_id, int txacdb_id) {
    int socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: rxacdb_id(%d), txacdb_id(%d)", __func__, rxacdb_id, txacdb_id);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_SEND_VOICE_CAL;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeInt(rxacdb_id, data_buf);
    writeInt(txacdb_id, data_buf);

    ALOGD("%s: msgID %d, length %d.", __func__, data_buf->msgID, data_buf->bufPos);
    acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    return;
}

int acdb_loader_reload_vocvoltable(int feature_set) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: feature_set(%d)", __func__, feature_set);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket.", __LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_RELOAD_VOICE_VOL_TABLE;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeInt(feature_set, data_buf);

    ALOGD("%s: msgID %d, length %d.", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

void acdb_loader_send_audio_cal(int acdb_id, int capability) {
    int socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: acdb_id(%d), capability(%d)", __func__, acdb_id, capability);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_SEND_AUDIO_CAL;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeInt(acdb_id, data_buf);
    writeInt(capability, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    return;
}

void acdb_loader_send_audio_cal_v2(int acdb_id, int capability, int app_id, int sample_rate) {
    int socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: acdb_id(%d), capability(%d), app_id(%d), sample_rate(%d)", __func__, acdb_id, capability, app_id, sample_rate);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_SEND_AUDIO_CAL_V2;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeInt(acdb_id, data_buf);
    writeInt(capability,data_buf);
    writeInt(app_id, data_buf);
    writeInt(sample_rate, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    return;
}

void acdb_loader_send_audio_cal_v3(int acdb_id, int capability, int app_id, int sample_rate, int use_case) {
    int socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: acdb_id(%d), capability(%d), app_id(%d), sample_rate(%d), use_case(%d)", __func__, acdb_id, capability, app_id, sample_rate, use_case);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_SEND_AUDIO_CAL_V3;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeInt(acdb_id, data_buf);
    writeInt(capability, data_buf);
    writeInt(app_id, data_buf);
    writeInt(sample_rate, data_buf);
    writeInt(use_case, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    return;
}

void acdb_loader_send_audio_cal_v4(int acdb_id, int capability, int app_id,
    int sample_rate, int use_case, int afe_sample_rate) {
    int socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: acdb_id(%d), capability(%d), app_id(%d), sample_rate(%d), use_case(%d), afe_sample_rate(%d)", __func__, acdb_id, capability, app_id, sample_rate, use_case, afe_sample_rate);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_SEND_AUDIO_CAL_V4;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeInt(acdb_id, data_buf);
    writeInt(capability, data_buf);
    writeInt(app_id, data_buf);
    writeInt(sample_rate, data_buf);
    writeInt(use_case, data_buf);
    writeInt(afe_sample_rate, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    return;
}

void acdb_loader_send_audio_cal_v6(int acdb_id, int capability, int app_id,
    int sample_rate, int use_case, int afe_sample_rate, int cal_mode, int offset_index) {
    int socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: acdb_id(%d), capability(%d), app_id(%d), sample_rate(%d), use_case(%d), afe_sample_rate(%d), cal_mode(%d), offset_index(%d)",\
        __func__, acdb_id, capability, app_id, sample_rate, use_case, afe_sample_rate, cal_mode, offset_index);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_SEND_AUDIO_CAL_V6;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeInt(acdb_id, data_buf);
    writeInt(capability, data_buf);
    writeInt(app_id, data_buf);
    writeInt(sample_rate, data_buf);
    writeInt(use_case, data_buf);
    writeInt(afe_sample_rate, data_buf);
    writeInt(cal_mode, data_buf);
    writeInt(offset_index, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    return;
}

void acdb_loader_send_listen_device_cal(int acdb_id, int type, int app_id, int sample_rate) {
    int socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: acdb_id(%d), type(%d), app_id(%d), sample_rate(%d)", __func__, acdb_id, type, app_id, sample_rate);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_SEND_LISTEN_DEV_CAL;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeInt(acdb_id, data_buf);
    writeInt(type, data_buf);
    writeInt(app_id, data_buf);
    writeInt(sample_rate, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    return;
}

int acdb_loader_send_listen_lsm_cal(int acdb_id, int app_id, int mode, int type) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: acdb_id(%d), app_id(%d), mode(%d), type(%d)", __func__, acdb_id, app_id, mode, type);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_SEND_LISTEN_LSM_CAL;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeInt(acdb_id, data_buf);
    writeInt(app_id, data_buf);
    writeInt(mode, data_buf);
    writeInt(type, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

int acdb_loader_send_listen_lsm_cal_v1(int acdb_id, int app_id, int mode, int type, int buff_idx) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: acdb_id(%d), app_id(%d), mode(%d), type(%d), buff_idx(%d)", __func__, acdb_id, app_id, mode, type, buff_idx);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == 0) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_SEND_LISTEN_LSM_CAL_V1;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeInt(acdb_id, data_buf);
    writeInt(app_id, data_buf);
    writeInt(mode, data_buf);
    writeInt(type, data_buf);
    writeInt(buff_idx, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

int acdb_loader_send_anc_cal(int acdb_id) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: acdb_id(%d)", __func__, acdb_id);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_SEND_ANC_CAL;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeInt(acdb_id, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

void send_tabla_anc_data(void) {
    int socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s", __func__);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_SEND_ANC_DATA;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    return;
}

int acdb_loader_get_aud_volume_steps(void) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s", __func__);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_GET_AUD_VOL_STEPS;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

int acdb_loader_send_gain_dep_cal(int acdb_id, int app_id,
    int capability, int mode, int vol_index) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: acdb_id(%d), app_id(%d), capability(%d), mode(%d), vol_index(%d)", __func__, acdb_id, app_id, capability, mode, vol_index);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_SEND_GAIN_DEP_CAL;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeInt(acdb_id, data_buf);
    writeInt(app_id, data_buf);
    writeInt(capability, data_buf);
    writeInt(mode, data_buf);
    writeInt(vol_index, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

int acdb_loader_get_remote_acdb_id(unsigned int native_acdb_id) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: native_acdb_id(%d)", __func__, native_acdb_id);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_GET_REMOTE_ACDB_ID;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeInt(native_acdb_id, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

int acdb_loader_get_ecrx_device(int acdb_id) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: acdb_id(%d)", __func__, acdb_id);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ",__LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_GET_ECRX_DEVICE;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeInt(acdb_id, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

int acdb_loader_get_calibration(char *attr, int size, void *pdata) {
    int ret = -1, socketfd;
    const char dummy[] = "null";
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: attr(%s), size(%d)", __func__, attr, size);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ",__LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_GET_CALIBRATION;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    if (attr != NULL) {
        writeString(attr, strlen(attr), data_buf);
    } else {
        writeString(dummy, strlen(dummy), data_buf);
    }
    writeInt(size, data_buf);
    writeString(pdata, sizeof(struct param_data), data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    if (ret == 0) {
        struct param_data * params = (struct param_data *)pdata;
        int paramsize;
        void * pbuff = params->buff;
        readString(pdata, sizeof(struct param_data), &paramsize, data_buf);
        params->buff = pbuff;
        if (params->get_size == 0) {
            readString(params->buff, params->buff_size, &paramsize, data_buf);
        }
    }
    return ret;
}

int acdb_loader_set_audio_cal_v2(void *caldata, void* pdata,
    unsigned int datalen) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: datalen(%d)", __func__, datalen);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_SET_AUDIO_CAL_V2;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeString(caldata, sizeof(acdb_audio_cal_cfg_t), data_buf);
    writeInt(datalen, data_buf);
    writeString(pdata, datalen, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

int acdb_loader_get_audio_cal_v2(void *caldata, void* pdata,
    unsigned int *datalen) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: provided datalen(%d)", __func__, *datalen);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_GET_AUDIO_CAL_V2;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeString(caldata, sizeof(acdb_audio_cal_cfg_t), data_buf);
    writeInt(*datalen, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    if (ret == 0) {
        readString(pdata, *datalen, datalen, data_buf);
    }
    return ret;
}

int send_meta_info(int metaInfoKey, int buf_idx) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s: metaInfoKey(%d), buf_idx(%d)", __func__, metaInfoKey, buf_idx);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_SEND_META_INFO;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeInt(metaInfoKey, data_buf);
    writeInt(buf_idx, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

int send_meta_info_list(struct listnode *metaKey_list) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s", __func__);

    struct listnode *node;
    struct meta_key_list *key_info;
    int count = 0;

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ",__LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_SEND_META_INFO_LIST;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    list_for_each(node, metaKey_list) {
        if (count > 0) {
            ALOGE("%s: Only a single value for key_list is supported in the current implementation", __func__);
            return -EINVAL;
        }
        key_info = node_to_item(node, struct meta_key_list, list);
        ALOGV("%s: acdb key_info = %u", __func__, key_info->cal_info.nKey);
        writeInt(key_info->cal_info.nKey, data_buf);
        count++;
    }

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

int acdb_loader_set_codec_data(void *pdata, char *attr) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    const char dummy[] = "null";
    struct listnode *node;
    struct meta_key_list *key_info;
    int count = 0;
    ALOGD("%s: attr(%s)", __func__, attr);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_SET_CODEC_DATA;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    writeString(pdata, sizeof(vbat_adc_data_t), data_buf);
    if (attr != NULL) {
        writeString(attr, strlen(attr), data_buf);;
    } else {
        writeString(dummy, strlen(dummy), data_buf);
    }

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

bool acdb_loader_is_initialized(void) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    ALOGD("%s", __func__);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_IS_INITIALIZED;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

int acdb_loader_reload_acdb_files(char *new_acdb_file_path, char *snd_card_name, char *cvd_version, int metaInfoKey) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    const char dummy[] = "null";
    ALOGD("%s: new_acdb_file_path(%s), snd(%s), cvd(%s), metaInfoKey(%d)", __func__, new_acdb_file_path, snd_card_name, cvd_version, metaInfoKey);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_RELOAD_ACDB_FILES;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    if (new_acdb_file_path != NULL) {
        writeString(new_acdb_file_path, strlen(new_acdb_file_path), data_buf);
    } else {
        writeString(dummy, strlen(dummy), data_buf);
    }
    if (snd_card_name != NULL) {
        writeString(snd_card_name, strlen(snd_card_name), data_buf);
    } else {
        writeString(dummy, strlen(dummy), data_buf);
    }
    if (cvd_version != NULL) {
        writeString(cvd_version, strlen(cvd_version), data_buf);
    } else {
        writeString(dummy, strlen(dummy), data_buf);
    }
    writeInt(metaInfoKey, data_buf);

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

int acdb_loader_reload_acdb_files_v2(char *new_acdb_file_path, char *snd_card_name, char *cvd_version, struct listnode *metaKey_list) {
    int ret = -1, socketfd;
    acdb_data_buf_t *data_buf = &socket_data_buf;
    const char dummy[] = "null";
    struct listnode *node;
    struct meta_key_list *key_info;
    int count = 0;
    ALOGD("%s: new_acdb_file_path(%s), snd(%s), cvd(%s)", __func__, new_acdb_file_path, snd_card_name, cvd_version);

    socketfd = get_acdb_loader_client_socket();
    if (socketfd == -1) {
       ALOGE("%d:%s: invalid acdb loader client socket ", __LINE__, __func__);
       return -ENODEV;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    data_buf->msgID = MSG_RELOAD_ACDB_FILES_V2;
    data_buf->hdrSize = 5 * sizeof(int);
    data_buf->bufSize = BUF_SIZE;

    if (new_acdb_file_path != NULL) {
        writeString(new_acdb_file_path, strlen(new_acdb_file_path), data_buf);
    } else {
        writeString(dummy,strlen(dummy), data_buf);
    }
    if (snd_card_name != NULL) {
        writeString(snd_card_name, strlen(snd_card_name), data_buf);
    } else {
        writeString(dummy, strlen(dummy), data_buf);
    }
    if (cvd_version != NULL) {
        writeString(cvd_version, strlen(cvd_version), data_buf);
    } else {
        writeString(dummy, strlen(dummy), data_buf);
    }

    list_for_each(node, metaKey_list) {
        if (count > 0) {
            ALOGE("%s: Only a single value for key_list is supported in the current implementation", __func__);
            return -EINVAL;
        }
        key_info = node_to_item(node, struct meta_key_list, list);
        ALOGV("%s: acdb key_info = %u", __func__, key_info->cal_info.nKey);
        writeInt(key_info->cal_info.nKey, data_buf);
        count++;
    }

    ALOGD("%s: msgID %d, length %d", __func__, data_buf->msgID, data_buf->bufPos);
    ret = acdb_loader_dispatch_message(socketfd, data_buf->bufSize + data_buf->hdrSize, data_buf);
    if (ret < 0) {
        ALOGE("%s: acdb_loader_dispatch_message fail", __func__);
        return ret;
    }

    memset(data_buf, 0, sizeof(acdb_data_buf_t));
    ret = acdb_loader_handle_reply(socketfd, data_buf);
    return ret;
}

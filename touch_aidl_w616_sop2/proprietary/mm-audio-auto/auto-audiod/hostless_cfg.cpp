/**
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#define LOG_TAG "auto_hostless_cfg"
#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0

#include <log/log.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "hostless_cfg.h"

/* cfg file shares from AMS */
#define CFG_PATH "/vendor/etc/ams_core.cfg"
#define AUDIOD_CONF_KEYS_NUM 1
#define AUDIOD_CONF_SUBKEYS_NUM 1
static const char *audiod_cfg_key[AUDIOD_CONF_KEYS_NUM] = {"[audiod_hostless_ctrl]"};

#define AUDIOD_CONF_GET_KEY_VAL(keys, keysnum, s, ki, v)    \
    {                                                       \
        int i;                                              \
        ki = 0;                                             \
        v = NULL;                                           \
        for (i = 0; i < keysnum; i++)                       \
        {                                                   \
            int klen = strlen(keys[i]);                     \
            if (keys[i] && strncmp(s, keys[i], klen) == 0)  \
            {                                               \
                ki = i;                                     \
                v = s;                                      \
                break;                                      \
            }                                               \
        }                                                   \
    }

#define AUDIOD_CONF_GET_SUBKEY_VAL(s, skey, v)      \
    {                                               \
        v = NULL;                                   \
        int sklen = strlen(skey);                   \
        if (strncmp(s, skey, sklen) == 0)           \
        {                                           \
            if (s[sklen] == ':' || s[sklen] == '=') \
            {                                       \
                v = &s[sklen + 1];                  \
            }                                       \
        }                                           \
    }

static void audio_get_disabled_tdm(char *disabled_cfg, FILE *fp)
{
    int cnt = 0;
    char *sv = NULL;
    char buffer[128];
    while (cnt < AUDIOD_CONF_SUBKEYS_NUM && fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        AUDIOD_CONF_GET_SUBKEY_VAL(buffer, "tdm_disabled", sv);
        if (sv)
        {
            strlcpy(disabled_cfg, sv, strlen(sv));
        }
    }
}

static inline int hostless_cfg_init_key(char *disabled_cfg, const char *name)
{
    FILE *fp = NULL;
    char buffer[32] = {0};
    int errvalue = 0;
    errno = 0;
    fp = fopen(name, "r");

    if (fp == NULL)
    {
        errvalue = errno;
        ALOGE("File %s cannot be opened(err:%s)!", name, strerror(errvalue));
        return errvalue;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        int ki = 0;
        char *v = NULL;
        AUDIOD_CONF_GET_KEY_VAL(audiod_cfg_key, AUDIOD_CONF_KEYS_NUM, buffer, ki, v);
        if (v)
        {
            ALOGD("Found key = %s", v);
            audio_get_disabled_tdm(disabled_cfg, fp);
        }
    }

    fclose(fp);

    return 0;
}

int32_t audiod_hostless_disbaled_cfg(char *disabled_cfg)
{
    int ret = 0;
    if (disabled_cfg == NULL){
        return -EFAULT;
    }

    ret = hostless_cfg_init_key(disabled_cfg, CFG_PATH);

    return ret;
}


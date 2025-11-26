/**
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#define LOG_TAG "ams_core_cfg"
#include <log/log.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "ams_core_cfg.h"
#include "ams_osal_error.h"

static unsigned int ams_core_cfg_log_dbg_lvl = 1;

#define AMS_CORE_CFG_PATH "/vendor/etc/ams_core.cfg"
#define AMS_CORE_CONF_KEYS_NUM 1
#define AMS_CORE_CONF_SUBKEYS_NUM 5
static const char *ams_core_cfg_key[AMS_CORE_CONF_KEYS_NUM] = {"[sh_res_ctrl]"};

#define AMS_CORE_CONF_GET_KEY_VAL(keys, keysnum, s, ki, v) \
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

#define AMS_CORE_CONF_GET_SUBKEY_VAL(s, skey, v)   \
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

#define AMS_CORE_CONF_GET_VAL_INT(sv, v, num)     \
    {                                              \
        if (sv != NULL)                            \
        {                                          \
            while (sv[0] != '\0')                  \
            {                                      \
                if (isdigit(*sv))                  \
                {                                  \
                    int val = strtol(sv, &sv, 10); \
                    *v++ = val;                    \
                    ++num;                         \
                }                                  \
                else                               \
                {                                  \
                    sv++;                          \
                }                                  \
            }                                      \
        }                                          \
    }

#define AMS_CORE_CONF_INIT_ARR(cfg, sv, field, num)                \
    {                                                               \
        int i, field[AMS_CORE_CONF_CFG_MAX_NUM] = {0}, *v = field; \
        num = 0; \
        AMS_CORE_CONF_GET_VAL_INT(sv, v, num);                     \
        for (i = 0; i < num; i++)                                   \
        {                                                           \
            cfg.field[i] = field[i];                                \
        }                                                           \
    }

typedef void (*ams_core_cfg_set_key_val_func_t)(ams_core_cfg_t *, FILE *fp);

static void ams_core_cfg_set_sh_res_ctrl(ams_core_cfg_t *cfg, FILE *fp)
{
    int num = 0, cnt = 0;
    char *sv = NULL;
    char buffer[128];
    while (cnt < 3 && fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        AMS_CORE_CONF_GET_SUBKEY_VAL(buffer, "dma_types", sv);
        if (sv)
        {
            AMS_CORE_CONF_INIT_ARR(cfg->sh_res_ctrl, sv, dma_types, num);
            cfg->sh_res_ctrl.dma_types_num = num;
            cnt++;
            continue;
        }
        AMS_CORE_CONF_GET_SUBKEY_VAL(buffer, "dma_rd_num", sv);
        if (sv)
        {
            AMS_CORE_CONF_INIT_ARR(cfg->sh_res_ctrl, sv, dma_rd_num, num);
            cnt++;
            continue;
        }
        AMS_CORE_CONF_GET_SUBKEY_VAL(buffer, "dma_wr_num", sv);
        if (sv)
        {
            AMS_CORE_CONF_INIT_ARR(cfg->sh_res_ctrl, sv, dma_wr_num, num);
            cnt++;
            continue;
        }
    }
}

static void ams_core_cfg_show_cfg(ams_core_cfg_t *cfg)
{
    int i;
    if (ams_core_cfg_log_dbg_lvl) {
        ALOGD("AMS_CORE_CONF->sh_res_ctrl.dma_types_num:%d", cfg->sh_res_ctrl.dma_types_num);
        for (i = 0; i < cfg->sh_res_ctrl.dma_types_num; i++)
        {
            ALOGD("AMS_CORE_CONF->sh_res_ctrl.dma_types:%d", cfg->sh_res_ctrl.dma_types[i]);
            ALOGD("AMS_CORE_CONF->sh_res_ctrl.dma_rd_num:%d", cfg->sh_res_ctrl.dma_rd_num[i]);
            ALOGD("AMS_CORE_CONF->sh_res_ctrl.dma_wr_num:%d", cfg->sh_res_ctrl.dma_wr_num[i]);
        }
    };
}

static ams_core_cfg_set_key_val_func_t ams_core_cfg_set_keys_val_func[AMS_CORE_CONF_KEYS_NUM] = {
    ams_core_cfg_set_sh_res_ctrl,
};

static int ams_core_cfg_reset(ams_core_cfg_t *cfg);

static inline int ams_core_cfg_init_key(ams_core_cfg_t *cfg, const char *name);

static inline int ams_core_cfg_init_key(ams_core_cfg_t *cfg, const char *name)
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
        return AMS_EFAILED;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        int ki = 0;
        char *v = NULL;
        AMS_CORE_CONF_GET_KEY_VAL(ams_core_cfg_key, AMS_CORE_CONF_KEYS_NUM, buffer, ki, v);
        if (v)
        {
            ALOGD("Found key =%s", v);
            if (ams_core_cfg_set_keys_val_func[ki])
                ams_core_cfg_set_keys_val_func[ki](cfg, fp);
        }
    }

    fclose(fp);

    return AMS_EOK;
}

static int ams_core_cfg_reset(ams_core_cfg_t *cfg)
{
    memset(cfg, 0, sizeof(*cfg));
    return 0;
}

int32_t ams_core_cfg_init(ams_core_cfg_t *cfg)
{
    int r = 0;
    const char *name;
    if (cfg==NULL){
        return AMS_EFAILED;
    }
    if (cfg->init){
        // skip
        goto exit;
    }
    name = AMS_CORE_CFG_PATH;
    ams_core_cfg_reset(cfg);
    r = ams_core_cfg_init_key(cfg, name);
    if (r)
    {
        return AMS_EFAILED;
    }
    ams_core_cfg_show_cfg(cfg);
    //
    cfg->init = 1;
exit:

    return AMS_EOK;
}


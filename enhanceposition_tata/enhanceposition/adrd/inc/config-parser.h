/*
* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2017. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/


#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include "adr_util.h"

#ifdef  __cplusplus
extern "C" {
#endif

#if defined(__ANDROID_OS__)
#define ADR_DEFAULT_CONFIG_FILE "/system/vendor/etc/adr/adr_alt.conf"
#elif defined(__LINUX_OS__)
#define ADR_DEFAULT_CONFIG_FILE "/etc/adr/adr_alt.conf"
#endif

enum config_key_type {
    CONFIG_KEY_INTEGER,        /* typeof data = int */
    CONFIG_KEY_UNSIGNED_INTEGER,    /* typeof data = unsigned int */
    CONFIG_KEY_STRING,        /* typeof data = char* */
    CONFIG_KEY_BOOLEAN        /* typeof data = int */
};

struct config_key {
    const char *name;
    enum config_key_type type;
    void *data;
};

struct config_section {
    const char *name;
    const struct config_key *keys;
    int num_keys;
    void (*done)(void *data);
};

enum adr_option_type {
    WESTON_OPTION_INTEGER,
    WESTON_OPTION_UNSIGNED_INTEGER,
    WESTON_OPTION_STRING,
    WESTON_OPTION_BOOLEAN
};

struct adr_option {
    enum adr_option_type type;
    const char *name;
    int short_name;
    void *data;
};

struct adr_config_entry {
    char *key;
    char *value;
    struct module_list link;
};

struct adr_config_section {
    char *name;
    struct module_list entry_list;
    struct module_list link;
};

struct adr_config {
    struct module_list section_list;
    char path[PATH_LEN];
};

int
parse_options(const struct adr_option *options,
          int count, int *argc, char *argv[]);

struct adr_config_section *
adr_config_get_section(struct adr_config *config, const char *section,
              const char *key, const char *value);
int
adr_config_section_get_int32_t(struct adr_config_section *section,
                  const char *key,
                  int32_t *value, int32_t default_value);
int
adr_config_section_get_int64_t(struct adr_config_section *section,
                  const char *key,
                  int64_t *value, int64_t default_value);
int
adr_config_section_get_uint(struct adr_config_section *section,
                   const char *key,
                   uint32_t *value, uint32_t default_value);
int
adr_config_section_get_double(struct adr_config_section *section,
                 const char *key,
                 double *value, double default_value);
int 
adr_config_section_get_char(struct adr_config_section *section,
                const char *key,
                char *value, const char default_value);
int
adr_config_section_get_string(struct adr_config_section *section,
                 const char *key,
                 char **value,
                 const char *default_value);
int
adr_config_section_get_bool(struct adr_config_section *section,
                   const char *key,
                   int *value, int default_value);

struct adr_config *
adr_config_parse(const char *name);

const char *
adr_config_get_full_path(struct adr_config *config);

void
adr_config_destroy(struct adr_config *config);

int adr_config_next_section(struct adr_config *config,
                   struct adr_config_section **section,
                   const char **name);


#ifdef  __cplusplus
}
#endif

#endif /* CONFIGPARSER_H */

